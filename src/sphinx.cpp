//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxstem.h"
#include "sphinxquery.h"
#include "sphinxutils.h"
#include "sphinxsort.h"
#include "fileutils.h"
#include "sphinxexpr.h"
#include "sphinxfilter.h"
#include "sphinxint.h"
#include "sphinxsearch.h"
#include "searchnode.h"
#include "sphinxjson.h"
#include "sphinxplugin.h"
#include "sphinxqcache.h"
#include "icu.h"
#include "attribute.h"
#include "secondaryindex.h"
#include "histogram.h"
#include "killlist.h"
#include "docstore.h"
#include "global_idf.h"
#include "indexformat.h"
#include "indexcheck.h"
#include "coroutine.h"
#include "columnarlib.h"
#include "columnarmisc.h"
#include "columnarfilter.h"
#include "mini_timer.h"
#include "sphinx_alter.h"
#include "conversion.h"
#include "binlog.h"
#include "task_info.h"
#include "chunksearchctx.h"

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <time.h>
#include <math.h>
#include <float.h>
#include <algorithm>

#if WITH_STEMMER
#include <libstemmer.h>
#endif

#if WITH_RE2
#include <string>
#include <re2/re2.h>
#endif

#if _WIN32
	#define snprintf	_snprintf
#else
	#include <unistd.h>
	#include <sys/time.h>
#endif

/////////////////////////////////////////////////////////////////////////////

// logf() is not there sometimes (eg. Solaris 9)
#if !_WIN32 && !HAVE_LOGF
static inline float logf ( float v )
{
	return (float) log ( v );
}
#endif

#if _WIN32
void localtime_r ( const time_t * clock, struct tm * res )
{
	*res = *localtime ( clock );
}

void gmtime_r ( const time_t * clock, struct tm * res )
{
	*res = *gmtime ( clock );
}
#endif

#include <boost/preprocessor/repetition/repeat.hpp>

// forward decl
void sphWarn ( const char * sTemplate, ... ) __attribute__ ( ( format ( printf, 1, 2 ) ) );

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////

const char *		SPHINX_DEFAULT_UTF8_TABLE	= "0..9, A..Z->a..z, _, a..z, U+410..U+42F->U+430..U+44F, U+430..U+44F, U+401->U+451, U+451";

const char *		MAGIC_WORD_SENTENCE		= "\3sentence";		// emitted from source on sentence boundary, stored in dictionary
const char *		MAGIC_WORD_PARAGRAPH	= "\3paragraph";	// emitted from source on paragraph boundary, stored in dictionary

bool				g_bJsonStrict				= false;
bool				g_bJsonAutoconvNumbers		= false;
bool				g_bJsonKeynamesToLowercase	= false;

static const int	MIN_READ_BUFFER			= 8192;
static const int	MIN_READ_UNHINTED		= 1024;

static int 			g_iReadUnhinted 		= DEFAULT_READ_UNHINTED;

CSphString			g_sLemmatizerBase		= GET_FULL_SHARE_DIR ();

// quick hack for indexer crash reporting
// one day, these might turn into a callback or something
int64_t		g_iIndexerCurrentDocID		= 0;
int64_t		g_iIndexerCurrentHits		= 0;
int64_t		g_iIndexerCurrentRangeMin	= 0;
int64_t		g_iIndexerCurrentRangeMax	= 0;
int64_t		g_iIndexerPoolStartDocID	= 0;
int64_t		g_iIndexerPoolStartHit		= 0;

static bool IndexBuildDone ( const BuildHeader_t & tBuildHeader, const WriteHeader_t & tWriteHeader, const CSphString & sFileName, CSphString & sError );

/////////////////////////////////////////////////////////////////////////////
// COMPILE-TIME CHECKS
/////////////////////////////////////////////////////////////////////////////

STATIC_SIZE_ASSERT ( SphOffset_t, 8 );

/////////////////////////////////////////////////////////////////////////////
// INTERNAL SPHINX CLASSES DECLARATIONS
/////////////////////////////////////////////////////////////////////////////

/// possible bin states
enum ESphBinState
{
	BIN_ERR_READ	= -2,	///< bin read error
	BIN_ERR_END		= -1,	///< bin end
	BIN_POS			= 0,	///< bin is in "expects pos delta" state
	BIN_DOC			= 1,	///< bin is in "expects doc delta" state
	BIN_WORD		= 2		///< bin is in "expects word delta" state
};


enum ESphBinRead
{
	BIN_READ_OK,			///< bin read ok
	BIN_READ_EOF,			///< bin end
	BIN_READ_ERROR,			///< bin read error
	BIN_PRECACHE_OK,		///< precache ok
	BIN_PRECACHE_ERROR		///< precache failed
};


/// aggregated hit info
struct CSphAggregateHit
{
	RowID_t			m_tRowID {INVALID_ROWID};	///< document ID
	SphWordID_t		m_uWordID {0};				///< word ID in current dictionary
	const BYTE *	m_sKeyword {nullptr};		///< word itself (in keywords dictionary case only)
	Hitpos_t		m_iWordPos {0};				///< word position in current document, or hit count in case of aggregate hit
	FieldMask_t		m_dFieldMask;				///< mask of fields containing this word, 0 for regular hits, non-0 for aggregate hits

	int GetAggrCount () const
	{
		assert ( !m_dFieldMask.TestAll ( false ) );
		return m_iWordPos;
	}

	void SetAggrCount ( int iVal )
	{
		m_iWordPos = iVal;
	}
};


/// bin, block input buffer
struct CSphBin
{
	static const int	MIN_SIZE	= 8192;
	static const int	WARN_SIZE	= 262144;

protected:
	ESphHitless			m_eMode;
	int					m_iSize = 0;

	BYTE *				m_dBuffer = nullptr;
	BYTE *				m_pCurrent = nullptr;
	int					m_iLeft = 0;
	int					m_iDone = 0;
	ESphBinState		m_eState { BIN_POS };
	bool				m_bWordDict;
	bool				m_bError = false;	// FIXME? sort of redundant, but states are a mess

	CSphAggregateHit	m_tHit;									///< currently decoded hit
	BYTE				m_sKeyword [ MAX_KEYWORD_BYTES ];	///< currently decoded hit keyword (in keywords dict mode)

#ifndef NDEBUG
	SphWordID_t			m_iLastWordID = 0;
	BYTE				m_sLastKeyword [ MAX_KEYWORD_BYTES ];
#endif

	int					m_iFile = -1;	///< my file
	SphOffset_t *		m_pFilePos = nullptr;	///< shared current offset in file

public:
	SphOffset_t			m_iFilePos = 0;		///< my current offset in file
	int					m_iFileLeft = 0;	///< how much data is still unread from the file

public:
	explicit 			CSphBin ( ESphHitless eMode = SPH_HITLESS_NONE, bool bWordDict = false );
						~CSphBin ();

	static int			CalcBinSize ( int iMemoryLimit, int iBlocks, const char * sPhase, bool bWarn = true );
	void				Init ( int iFD, SphOffset_t * pSharedOffset, const int iBinSize );

	SphWordID_t			ReadVLB ();
	int					ReadByte ();
	ESphBinRead			ReadBytes ( void * pDest, int iBytes );
	int					ReadHit ( CSphAggregateHit * pOut );

	DWORD				UnzipInt ();
	SphOffset_t			UnzipOffset ();

	bool				IsEOF () const;
	bool				IsDone () const;
	bool				IsError () const { return m_bError; }
	ESphBinRead			Precache ();
};

//////////////////////////////////////////////////////////////////////////

static const char * g_dRankerNames[] =
{
	"proximity_bm25",
	"bm25",
	"none",
	"wordcount",
	"proximity",
	"matchany",
	"fieldmask",
	"sph04",
	"expr",
	"export",
	NULL
};


const char * sphGetRankerName ( ESphRankMode eRanker )
{
	if ( eRanker<SPH_RANK_PROXIMITY_BM25 || eRanker>=SPH_RANK_TOTAL )
		return NULL;

	return g_dRankerNames[eRanker];
}

//////////////////////////////////////////////////////////////////////////

/// everything required to setup search term
class DiskIndexQwordSetup_c : public ISphQwordSetup
{
public:
	DiskIndexQwordSetup_c ( DataReaderFactory_c * pDoclist, DataReaderFactory_c * pHitlist, const BYTE * pSkips, int iSkiplistBlockSize, bool bSetupReaders, RowID_t iRowsCount )
		: m_pDoclist ( pDoclist )
		, m_pHitlist ( pHitlist )
		, m_pSkips ( pSkips )
		, m_iSkiplistBlockSize ( iSkiplistBlockSize )
		, m_bSetupReaders ( bSetupReaders )
		, m_iRowsCount ( iRowsCount )
	{
		SafeAddRef(pDoclist);
		SafeAddRef(pHitlist);
	}

	ISphQword *			QwordSpawn ( const XQKeyword_t & tWord ) const final;
	bool				QwordSetup ( ISphQword * ) const final;
	bool				Setup ( ISphQword * ) const;
	ISphQword *			ScanSpawn() const override;

private:
	DataReaderFactoryPtr_c		m_pDoclist;
	DataReaderFactoryPtr_c		m_pHitlist;
	const BYTE *				m_pSkips;
	int							m_iSkiplistBlockSize = 0;
	bool						m_bSetupReaders = false;
	RowID_t						m_iRowsCount = INVALID_ROWID;

private:
	bool SetupWithWrd ( const DiskIndexQwordTraits_c& tWord, CSphDictEntry& tRes ) const;
	bool SetupWithCrc ( const DiskIndexQwordTraits_c& tWord, CSphDictEntry& tRes ) const;
};

/// query word from the searcher's point of view
template < bool INLINE_HITS, bool DISABLE_HITLIST_SEEK >
class DiskIndexQword_c : public DiskIndexQwordTraits_c
{
public:
	DiskIndexQword_c ( bool bUseMinibuffer, bool bExcluded )
		: DiskIndexQwordTraits_c ( bUseMinibuffer, bExcluded )
	{}

	void Reset () final
	{
		if ( m_rdDoclist )
			m_rdDoclist->Reset ();
		if ( m_rdHitlist )
			m_rdHitlist->Reset ();
		ResetDecoderState();
	}

	void GetHitlistEntry ()
	{
		assert ( !m_bHitlistOver );
		DWORD iDelta = m_rdHitlist->UnzipInt ();
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

		while ( m_tDoc.m_tRowID < tRowID )
			ReadNext();

		return m_tDoc.m_tRowID;
	}


	bool HintRowID ( RowID_t tRowID ) final
	{
		// tricky bit
		// FindSpan() will match a block where tBaseRowIDPlus1[i] <= tRowID < tBaseRowIDPlus1[i+1]
		// meaning that the subsequent ids decoded will be strictly > RefValue
		// meaning that if previous (!) blocks end with tRowID exactly,
		// and we use tRowID itself as RefValue, that document gets lost!

		// first check if we're still inside the last block
		if ( m_iSkipListBlock==-1 )
		{
			m_iSkipListBlock = FindSpan ( m_dSkiplist, tRowID );
			if ( m_iSkipListBlock<0 )
				return false;
		}
		else
		{
			if ( m_iSkipListBlock<m_dSkiplist.GetLength()-1 )
			{
				int iNextBlock = m_iSkipListBlock+1;
				RowID_t tNextBlockRowID = m_dSkiplist[iNextBlock].m_tBaseRowIDPlus1;
				if ( tRowID>=tNextBlockRowID )
				{
					auto dSkips = VecTraits_T<SkiplistEntry_t> ( &m_dSkiplist[iNextBlock], m_dSkiplist.GetLength()-iNextBlock );
					m_iSkipListBlock = FindSpan ( dSkips, tRowID );
					if ( m_iSkipListBlock<0 )
						return false;

					m_iSkipListBlock += iNextBlock;
				}
			}
			else // we're already at our last block, no need to search
				return false;
		}

		const SkiplistEntry_t & t = m_dSkiplist[m_iSkipListBlock];
		if ( t.m_iOffset<=m_rdDoclist->GetPos() )
			return false;

		m_rdDoclist->SeekTo ( t.m_iOffset, -1 );
		m_tDoc.m_tRowID = t.m_tBaseRowIDPlus1-1;
		m_uHitPosition = m_iHitlistPos = t.m_iBaseHitlistPos;

		return true;
	}

	const CSphMatch & GetNextDoc() override
	{
		ReadNext();
		return m_tDoc;
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
			if_const ( DISABLE_HITLIST_SEEK )
				assert ( m_rdHitlist->GetPos()==uOff ); // make sure we're where caller thinks we are.
			else
				m_rdHitlist->SeekTo ( uOff, READ_NO_SIZE_HINT );
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

	inline void ReadNext()
	{
		RowID_t uDelta = m_rdDoclist->UnzipRowid();
		if ( uDelta )
		{
			m_bAllFieldsKnown = false;
			m_tDoc.m_tRowID += uDelta;

			if_const ( INLINE_HITS )
			{
				m_uMatchHits = m_rdDoclist->UnzipInt();
				const DWORD uFirst = m_rdDoclist->UnzipInt();
				if ( m_uMatchHits==1 && m_bHasHitlist )
				{
					DWORD uField = m_rdDoclist->UnzipInt(); // field and end marker
					m_iHitlistPos = uFirst | ( uField << 23 ) | ( U64C(1)<<63 );
					m_dQwordFields.UnsetAll();
					// want to make sure bad field data not cause crash
					m_dQwordFields.Set ( ( uField >> 1 ) & ( (DWORD)SPH_MAX_FIELDS-1 ) );
					m_bAllFieldsKnown = true;
				} else
				{
					m_dQwordFields.Assign32 ( uFirst );
					m_uHitPosition += m_rdDoclist->UnzipOffset();
					m_iHitlistPos = m_uHitPosition;
				}
			} else
			{
				SphOffset_t iDeltaPos = m_rdDoclist->UnzipOffset();
				assert ( iDeltaPos>=0 );

				m_iHitlistPos += iDeltaPos;

				m_dQwordFields.Assign32 ( m_rdDoclist->UnzipInt() );
				m_uMatchHits = m_rdDoclist->UnzipInt();
			}
		} else
			m_tDoc.m_tRowID = INVALID_ROWID;
	}
};


DiskIndexQwordTraits_c * sphCreateDiskIndexQword ( bool bInlineHits )
{
	if ( bInlineHits )
		return new DiskIndexQword_c<true,false> ( false, false );

	return new DiskIndexQword_c<false,false> ( false, false );
}

/////////////////////////////////////////////////////////////////////////////

#define WITH_QWORD(INDEX, NO_SEEK, NAME, ACTION)										\
do if ( (( const CSphIndex_VLN *)INDEX)->m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE )	\
		{ using NAME = DiskIndexQword_c < true, NO_SEEK >; ACTION; }					\
	else																				\
		{ using NAME = DiskIndexQword_c < false, NO_SEEK >; ACTION; }					\
while(0)

/////////////////////////////////////////////////////////////////////////////

#define HITLESS_DOC_MASK 0x7FFFFFFF
#define HITLESS_DOC_FLAG 0x80000000


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


template < bool INLINE_HITS >
class DiskPayloadQword_c : public DiskIndexQword_c<INLINE_HITS, false>
{
	typedef DiskIndexQword_c<INLINE_HITS, false> BASE;

public:
	explicit DiskPayloadQword_c ( const DiskSubstringPayload_t * pPayload, bool bExcluded,
		DataReaderFactory_c * pDoclist, DataReaderFactory_c * pHitlist )
		: BASE ( true, bExcluded )
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

		this->m_rdDoclist->SeekTo ( uDocOff, iHint );
	}

	const DiskSubstringPayload_t *	m_pPayload;
	int								m_iDoclist;
};

//////////////////////////////////////////////////////////////////////////
// dirty hack for some build systems which not has LLONG_MAX
#ifndef LLONG_MAX
#define LLONG_MAX (((unsigned long long)(-1))>>1)
#endif

#ifndef LLONG_MIN
#define LLONG_MIN (-LLONG_MAX-1)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX	(LLONG_MAX * 2ULL + 1)
#endif


AttrIndexBuilder_c::AttrIndexBuilder_c ( const CSphSchema & tSchema )
{
	Init ( tSchema );
}

void AttrIndexBuilder_c::Init ( const CSphSchema& tSchema )
{
	m_uStride = tSchema.GetRowSize();
	for ( int i = 0; i < tSchema.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo& tCol = tSchema.GetAttr ( i );
		if ( tCol.IsColumnar() )
			continue;

		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_BOOL:
		case SPH_ATTR_BIGINT:
		case SPH_ATTR_TOKENCOUNT:
			m_dIntAttrs.Add ( tCol.m_tLocator );
			break;

		case SPH_ATTR_FLOAT:
			m_dFloatAttrs.Add ( tCol.m_tLocator );
			break;

		default:
			break;
		}
	}

	m_dIntMin.Resize ( m_dIntAttrs.GetLength() );
	m_dIntMax.Resize ( m_dIntAttrs.GetLength() );
	m_dFloatMin.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatMax.Resize ( m_dFloatAttrs.GetLength() );

	m_dIntIndexMin.Resize ( m_dIntAttrs.GetLength() );
	m_dIntIndexMax.Resize ( m_dIntAttrs.GetLength() );
	m_dFloatIndexMin.Resize ( m_dFloatAttrs.GetLength() );
	m_dFloatIndexMax.Resize ( m_dFloatAttrs.GetLength() );

	m_dIntIndexMin.Fill ( LLONG_MAX );
	m_dIntIndexMax.Fill ( LLONG_MIN );
	m_dFloatIndexMin.Fill ( FLT_MAX );
	m_dFloatIndexMax.Fill ( FLT_MIN );

	ResetLocal();
}


void AttrIndexBuilder_c::Collect ( const CSphRowitem * pRow )
{
	// check if it is time to flush already collected values
	if ( m_nLocalCollected>=DOCINFO_INDEX_FREQ )
		FlushComputed();

	m_nLocalCollected++;

	// ints
	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		SphAttr_t tVal = sphGetRowAttr ( pRow, m_dIntAttrs[i] );
		m_dIntMin[i] = Min ( m_dIntMin[i], tVal );
		m_dIntMax[i] = Max ( m_dIntMax[i], tVal );
	}

	// floats
	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		float fVal = sphDW2F ( (DWORD)sphGetRowAttr ( pRow, m_dFloatAttrs[i] ) );
		m_dFloatMin[i] = Min ( m_dFloatMin[i], fVal );
		m_dFloatMax[i] = Max ( m_dFloatMax[i], fVal );
	}
}


void AttrIndexBuilder_c::FinishCollect()
{
	if ( m_nLocalCollected )
		FlushComputed();

	CSphRowitem * pMinAttrs = m_dMinMaxRows.AddN ( m_uStride*2 );
	CSphRowitem * pMaxAttrs = pMinAttrs+m_uStride;

	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		sphSetRowAttr ( pMinAttrs, m_dIntAttrs[i], m_dIntIndexMin[i] );
		sphSetRowAttr ( pMaxAttrs, m_dIntAttrs[i], m_dIntIndexMax[i] );
	}

	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		sphSetRowAttr ( pMinAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatIndexMin[i] ) );
		sphSetRowAttr ( pMaxAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatIndexMax[i] ) );
	}
}


const CSphTightVector<CSphRowitem> & AttrIndexBuilder_c::GetCollected() const
{
	return m_dMinMaxRows;
}


void AttrIndexBuilder_c::ResetLocal()
{
	for ( auto & i : m_dIntMin )
		i = LLONG_MAX;

	for ( auto & i : m_dIntMax )
		i = LLONG_MIN;

	for ( auto & i : m_dFloatMin )
		i = FLT_MAX;

	for ( auto & i : m_dFloatMax )
		i = FLT_MIN;

	m_nLocalCollected = 0;
}


void AttrIndexBuilder_c::FlushComputed ()
{
	CSphRowitem * pMinAttrs = m_dMinMaxRows.AddN ( m_uStride*2 );
	CSphRowitem * pMaxAttrs = pMinAttrs+m_uStride;

	ARRAY_FOREACH ( i, m_dIntAttrs )
	{
		m_dIntIndexMin[i] = Min ( m_dIntIndexMin[i], m_dIntMin[i] );
		m_dIntIndexMax[i] = Max ( m_dIntIndexMax[i], m_dIntMax[i] );
		sphSetRowAttr ( pMinAttrs, m_dIntAttrs[i], m_dIntMin[i] );
		sphSetRowAttr ( pMaxAttrs, m_dIntAttrs[i], m_dIntMax[i] );
	}

	ARRAY_FOREACH ( i, m_dFloatAttrs )
	{
		m_dFloatIndexMin[i] = Min ( m_dFloatIndexMin[i], m_dFloatMin[i] );
		m_dFloatIndexMax[i] = Max ( m_dFloatIndexMax[i], m_dFloatMax[i] );
		sphSetRowAttr ( pMinAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatMin[i] ) );
		sphSetRowAttr ( pMaxAttrs, m_dFloatAttrs[i], sphF2DW ( m_dFloatMax[i] ) );
	}

	ResetLocal();
}


//////////////////////////////////////////////////////////////////////////

class CSphHitBuilder;

const char* CheckFmtMagic ( DWORD uHeader )
{
	if ( uHeader!=INDEX_MAGIC_HEADER )
	{
		FlipEndianess ( &uHeader );
		if ( uHeader==INDEX_MAGIC_HEADER )
#if USE_LITTLE_ENDIAN
			return "This instance is working on little-endian platform, but %s seems built on big-endian host.";
#else
			return "This instance is working on big-endian platform, but %s seems built on little-endian host.";
#endif
		else
			return "%s is invalid header file (too old index version?)";
	}
	return nullptr;
}


static IndexFileExt_t g_dIndexFilesExts[SPH_EXT_TOTAL] =
{
	{ SPH_EXT_SPH,	".sph",		1,	false,	true,	"header file" },
	{ SPH_EXT_SPA,	".spa",		1,	true,	true,	"attribute values" },
	{ SPH_EXT_SPB,	".spb",		50,	true,	true,	"var-length attrs: strings, jsons, mva" },
	{ SPH_EXT_SPC,	".spc",		61,	true,	true,	"columnar storage" },
	{ SPH_EXT_SPI,	".spi",		1,	false,	true,	"dictionary (aka wordlist)" },
	{ SPH_EXT_SPD,	".spd",		1,	false,	true,	"document lists (aka doclists)"},
	{ SPH_EXT_SPP,	".spp",		3,	false,	true,	"keyword positions lists (aka hitlists)" },
	{ SPH_EXT_SPK,	".spk",		10, true,	true,	"kill list (aka klist)" },
	{ SPH_EXT_SPE,	".spe",		31,	false,	true,	"skip-lists to speed up doc-list filtering" },
	{ SPH_EXT_SPM,	".spm",		4,	false,	true,	"dead row map" },
	{ SPH_EXT_SPT,	".spt",		53,	true,	true,	"docid lookup table" },
	{ SPH_EXT_SPHI,	".sphi",	53,	true,	true,	"secondary index histograms" },
	{ SPH_EXT_SPDS, ".spds",	57, true,	true,	"document storage" },
	{ SPH_EXT_SPL,	".spl",		1,	true,	false,	"file lock for the index" },
	{ SPH_EXT_SETTINGS,	".settings", 1,	true,	false,	"index runtime settings" }
};


const char* sphGetExt ( ESphExt eExt )
{
	if ( eExt<SPH_EXT_SPH || eExt>=SPH_EXT_TOTAL )
		return "";

	return g_dIndexFilesExts[eExt].m_szExt;
}


CSphVector<IndexFileExt_t> sphGetExts()
{
	// we may add support for older index versions in the future
	CSphVector<IndexFileExt_t> dResult;
	for ( const auto& tExt : g_dIndexFilesExts )
		dResult.Add ( tExt );

	return dResult;
}


/// this pseudo-index used to store and manage the tokenizer
/// without any footprint in real files
//////////////////////////////////////////////////////////////////////////
class CSphTokenizerIndex : public CSphIndexStub
{
public:
						CSphTokenizerIndex ( const char * szIndexName ) : CSphIndexStub ( szIndexName, nullptr ) {}
	bool				GetKeywords ( CSphVector <CSphKeywordInfo> & , const char * , const GetKeywordsSettings_t & tSettings, CSphString * ) const final ;
	Bson_t				ExplainQuery ( const CSphString & sQuery ) const final;
};


struct CSphTemplateQueryFilter : public ISphQueryFilter
{
	void AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords ) final
	{
		SphWordID_t iWord = m_pDict->GetWordID ( sWord );
		if ( !iWord )
			return;

		CSphKeywordInfo & tInfo = dKeywords.Add();
		tInfo.m_sTokenized = (const char *)sTokenized;
		tInfo.m_sNormalized = (const char*)sWord;
		tInfo.m_iDocs = 0;
		tInfo.m_iHits = 0;
		tInfo.m_iQpos = iQpos;

		RemoveDictSpecials ( tInfo.m_sNormalized );
	}
};


bool CSphTokenizerIndex::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * ) const
{
	// short-cut if no query or keywords to fill
	if ( !szQuery || !szQuery[0] )
		return true;

	TokenizerRefPtr_c pTokenizer { m_pTokenizer->Clone ( SPH_CLONE_INDEX ) };
	pTokenizer->EnableTokenizedMultiformTracking ();

	// need to support '*' and '=' but not the other specials
	// so m_pQueryTokenizer does not work for us, gotta clone and setup one manually

	DictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };

	if ( IsStarDict ( pDict->GetSettings().m_bWordDict ) )
	{
		pTokenizer->AddPlainChar ( '*' );
		pDict = new CSphDictStar ( pDict );
	}

	if ( m_tSettings.m_bIndexExactWords )
	{
		pTokenizer->AddPlainChar ( '=' );
		pDict = new CSphDictExact ( pDict );
	}

	dKeywords.Resize ( 0 );

	CSphVector<BYTE> dFiltered;
	FieldFilterRefPtr_c pFieldFilter;
	const BYTE * sModifiedQuery = (const BYTE *)szQuery;
	if ( m_pFieldFilter && szQuery )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter && pFieldFilter->Apply ( sModifiedQuery, dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	pTokenizer->SetBuffer ( sModifiedQuery, (int) strlen ( (const char*)sModifiedQuery) );

	CSphTemplateQueryFilter tAotFilter;
	tAotFilter.m_pTokenizer = pTokenizer;
	tAotFilter.m_pDict = pDict;
	tAotFilter.m_pSettings = &m_tSettings;
	tAotFilter.m_tFoldSettings = tSettings;
	tAotFilter.m_tFoldSettings.m_bStats = false;
	tAotFilter.m_tFoldSettings.m_bFoldWildcards = true;

	ExpansionContext_t tExpCtx;

	tAotFilter.GetKeywords ( dKeywords, tExpCtx );

	return true;
}


CSphIndex * sphCreateIndexTemplate ( const char * szIndexName )
{
	return new CSphTokenizerIndex(szIndexName);
}

Bson_t CSphTokenizerIndex::ExplainQuery ( const CSphString & sQuery ) const
{
	bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	WordlistStub_c tWordlist;
	ExplainQueryArgs_t tArgs ( sQuery );
	tArgs.m_pDict = GetStatelessDict ( m_pDict );
	if ( IsStarDict ( bWordDict ) )
		tArgs.m_pDict = new CSphDictStarV8 ( tArgs.m_pDict, m_tSettings.m_iMinInfixLen>0 );
	if ( m_tSettings.m_bIndexExactWords )
		tArgs.m_pDict = new CSphDictExact ( tArgs.m_pDict );
	if ( m_pFieldFilter )
		tArgs.m_pFieldFilter = m_pFieldFilter->Clone();
	tArgs.m_pSettings = &m_tSettings;
	tArgs.m_pWordlist = &tWordlist;
	tArgs.m_pQueryTokenizer = m_pQueryTokenizer;
	tArgs.m_iExpandKeywords = m_tMutableSettings.m_iExpandKeywords;
	tArgs.m_iExpansionLimit = m_iExpansionLimit;
	tArgs.m_bExpandPrefix = ( bWordDict && IsStarDict ( bWordDict ) );

	return Explain ( tArgs );
}

//////////////////////////////////////////////////////////////////////////

UpdateContext_t::UpdateContext_t ( AttrUpdateInc_t & tUpd, const ISphSchema & tSchema )
	: m_tUpd ( tUpd )
	, m_tSchema ( tSchema )
	, m_dUpdatedAttrs ( tUpd.m_pUpdate->m_dAttributes.GetLength() )
	, m_dSchemaUpdateMask ( tSchema.GetAttrsCount() )
{}

//////////////////////////////////////////////////////////////////////////

bool IndexUpdateHelper_c::Update_CheckAttributes ( const CSphAttrUpdate & tUpd, const ISphSchema & tSchema, CSphString & sError )
{
	for ( const auto & tUpdAttr : tUpd.m_dAttributes )
	{
		const CSphString & sUpdAttrName = tUpdAttr.m_sName;
		int iUpdAttrId = tSchema.GetAttrIndex ( sUpdAttrName.cstr() );

		// try to find JSON attribute with a field
		if ( iUpdAttrId<0 )
		{
			CSphString sJsonCol;
			if ( sphJsonNameSplit ( sUpdAttrName.cstr(), &sJsonCol ) )
				iUpdAttrId = tSchema.GetAttrIndex ( sJsonCol.cstr() );
		}

		if ( iUpdAttrId<0 )
		{
			if ( tUpd.m_bIgnoreNonexistent )
				continue;

			sError.SetSprintf ( "attribute '%s' not found", sUpdAttrName.cstr() );
			return false;
		}

		// forbid updates on non-int columns
		const CSphColumnInfo & tCol = tSchema.GetAttr ( iUpdAttrId );
		if ( !( tCol.m_eAttrType==SPH_ATTR_BOOL || tCol.m_eAttrType==SPH_ATTR_INTEGER || tCol.m_eAttrType==SPH_ATTR_TIMESTAMP
			|| tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET || tCol.m_eAttrType==SPH_ATTR_STRING
			|| tCol.m_eAttrType==SPH_ATTR_BIGINT || tCol.m_eAttrType==SPH_ATTR_FLOAT || tCol.m_eAttrType==SPH_ATTR_JSON ))
		{
			sError.SetSprintf ( "attribute '%s' can not be updated (must be boolean, integer, bigint, float, timestamp, string, MVA or JSON)", sUpdAttrName.cstr() );
			return false;
		}

		bool bSrcMva = ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET );
		bool bDstMva = ( tUpdAttr.m_eType==SPH_ATTR_UINT32SET || tUpdAttr.m_eType==SPH_ATTR_INT64SET );
		if ( bSrcMva!=bDstMva )
		{
			sError.SetSprintf ( "attribute '%s' MVA flag mismatch", sUpdAttrName.cstr() );
			return false;
		}

		if( tCol.m_eAttrType==SPH_ATTR_UINT32SET && tUpdAttr.m_eType==SPH_ATTR_INT64SET )
		{
			sError.SetSprintf ( "attribute '%s' MVA bits (dst=%d, src=%d) mismatch", sUpdAttrName.cstr(), tCol.m_eAttrType, tUpdAttr.m_eType );
			return false;
		}

		if ( tCol.IsColumnar() )
		{
			sError.SetSprintf ( "unable to update columnar attribute '%s'", sUpdAttrName.cstr() );
			return false;
		}

	}

	return true;
}


void IndexUpdateHelper_c::Update_PrepareListOfUpdatedAttributes ( UpdateContext_t & tCtx, CSphString & sError )
{
	const auto & tUpd = *tCtx.m_tUpd.m_pUpdate;
	ARRAY_FOREACH ( i, tUpd.m_dAttributes )
	{
		const CSphString & sUpdAttrName = tUpd.m_dAttributes[i].m_sName;
		ESphAttr eUpdAttrType = tUpd.m_dAttributes[i].m_eType;
		UpdatedAttribute_t & tUpdAttr = tCtx.m_dUpdatedAttrs[i];

		int iUpdAttrId = tCtx.m_tSchema.GetAttrIndex ( sUpdAttrName.cstr() );

		if ( iUpdAttrId<0 )
		{
			CSphString sJsonCol;
			if ( sphJsonNameSplit ( sUpdAttrName.cstr(), &sJsonCol ) )
			{
				iUpdAttrId = tCtx.m_tSchema.GetAttrIndex ( sJsonCol.cstr() );
				if ( iUpdAttrId>=0 )
				{
					ExprParseArgs_t tExprArgs;
					tUpdAttr.m_pExpr = sphExprParse ( sUpdAttrName.cstr(), tCtx.m_tSchema, sError, tExprArgs );
				}
			}
		}

		if ( iUpdAttrId>=0 )
		{
			const CSphColumnInfo & tCol = tCtx.m_tSchema.GetAttr(iUpdAttrId);

			switch ( tCol.m_eAttrType )
			{
			case SPH_ATTR_FLOAT:
				if ( eUpdAttrType==SPH_ATTR_BIGINT )
					tUpdAttr.m_eConversion = CONVERSION_BIGINT2FLOAT;
				break;

			case SPH_ATTR_BIGINT:
				if ( eUpdAttrType==SPH_ATTR_FLOAT )
					tUpdAttr.m_eConversion = CONVERSION_FLOAT2BIGINT;
				break;

			default:
				break;
			}

			tUpdAttr.m_eAttrType = tCol.m_eAttrType;
			tUpdAttr.m_tLocator = tCol.m_tLocator;
			tUpdAttr.m_pHistogram = tCtx.m_pHistograms ? tCtx.m_pHistograms->Get(tCol.m_sName) : nullptr;
			tUpdAttr.m_bExisting = true;
			tUpdAttr.m_iSchemaAttr = iUpdAttrId;

			tCtx.m_dSchemaUpdateMask.BitSet(iUpdAttrId);
			tCtx.m_bBlobUpdate |= sphIsBlobAttr(tCol);
		}
		else
		{
			assert ( tUpd.m_bIgnoreNonexistent ); // should be handled by Update_CheckAttributes
			continue;
		}

		// this is a hack
		// Query parser tries to detect an attribute type. And this is wrong because, we should
		// take attribute type from schema. Probably we'll rewrite updates in future but
		// for now this fix just works.
		// Fixes cases like UPDATE float_attr=1 WHERE id=1;
		assert ( iUpdAttrId>=0 );
		if ( eUpdAttrType==SPH_ATTR_INTEGER && tCtx.m_tSchema.GetAttr(iUpdAttrId).m_eAttrType==SPH_ATTR_FLOAT )
		{
			const_cast<CSphAttrUpdate &>(tUpd).m_dAttributes[i].m_eType = SPH_ATTR_FLOAT;
			const_cast<CSphAttrUpdate &>(tUpd).m_dPool[i] = sphF2DW ( (float)tUpd.m_dPool[i] );
		}
	}
}


static void IncUpdatePoolPos ( UpdateContext_t & tCtx, int iAttr, int & iPos )
{
	switch ( tCtx.m_tUpd.m_pUpdate->m_dAttributes[iAttr].m_eType )
	{
	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
		iPos += tCtx.m_tUpd.m_pUpdate->m_dPool[iPos] + 1;
		break;

	case SPH_ATTR_STRING:
	case SPH_ATTR_BIGINT:
		iPos += 2;
		break;

	default:
		iPos += 1;
		break;
	}
}


static bool FitsInplaceJsonUpdate ( const UpdateContext_t & tCtx, int iAttr )
{
	// only json fields and no strings (strings go as full json updates)
	return tCtx.m_dUpdatedAttrs[iAttr].m_eAttrType==SPH_ATTR_JSON && tCtx.m_tUpd.m_pUpdate->m_dAttributes[iAttr].m_eType!=SPH_ATTR_STRING;
}


bool IndexUpdateHelper_c::Update_InplaceJson ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx, CSphString & sError, bool bDryRun )
{
	const auto& tUpd = *tCtx.m_tUpd.m_pUpdate;
	for ( const auto & tRow : dRows )
	{
		int iUpd = tRow.m_iIdx;
		int iPos = tUpd.GetRowOffset ( iUpd );
		ARRAY_CONSTFOREACH ( i, tUpd.m_dAttributes )
		{
			if ( !FitsInplaceJsonUpdate ( tCtx, i ) || !tCtx.m_dUpdatedAttrs[i].m_bExisting )
			{
				IncUpdatePoolPos ( tCtx, i, iPos );
				continue;
			}

			ESphAttr eAttr = tUpd.m_dAttributes[i].m_eType;

			bool bBigint = eAttr==SPH_ATTR_BIGINT;
			bool bDouble = eAttr==SPH_ATTR_FLOAT;

			ESphJsonType eType = bDouble ? JSON_DOUBLE : ( bBigint ? JSON_INT64 : JSON_INT32 );
			SphAttr_t uValue = bDouble
					? sphD2QW ( (double)sphDW2F ( tUpd.m_dPool[iPos] ) )
					: ( bBigint ? MVA_UPSIZE ( &tUpd.m_dPool[iPos] ) : tUpd.m_dPool[iPos] );

			if ( sphJsonInplaceUpdate ( eType, uValue, tCtx.m_dUpdatedAttrs[i].m_pExpr, tCtx.m_pBlobPool, tRow.m_pRow, !bDryRun ) )
			{
				assert ( tCtx.m_dUpdatedAttrs[i].m_iSchemaAttr>=0 );
				tCtx.m_tUpd.MarkUpdated ( iUpd );
				tCtx.m_uUpdateMask |= ATTRS_BLOB_UPDATED;
				// reset update bit to copy partial updated JSON into new blob
				tCtx.m_dSchemaUpdateMask.BitClear ( tCtx.m_dUpdatedAttrs[i].m_iSchemaAttr );
			} else
			{
				if ( bDryRun )
				{
					sError.SetSprintf ( "attribute '%s' can not be updated (not found or incompatible types)", tUpd.m_dAttributes[i].m_sName.cstr() );
					return false;
				} else
					++tCtx.m_iJsonWarnings;
			}

			IncUpdatePoolPos ( tCtx, i, iPos );
		}
	}

	return true;
}


bool IndexUpdateHelper_c::Update_Blobs ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx, bool & bCritical, CSphString & sError )
{
	const auto & tUpd = *tCtx.m_tUpd.m_pUpdate;

	// any blobs supplied in the update?
	if ( !tCtx.m_bBlobUpdate )
		return true;

	//	create a remap from attribute id in UPDATE to blob attr id
	CSphVector<int> dRemap ( tUpd.m_dAttributes.GetLength() );
	dRemap.Fill(-1);

	CSphVector<int> dBlobAttrIds;

	bool bNeedBlobBuilder = false;
	int iBlobAttrId = 0;
	for ( int i = 0, iAttrs = tCtx.m_tSchema.GetAttrsCount (); i < iAttrs; ++i )
	{
		const CSphColumnInfo & tAttr = tCtx.m_tSchema.GetAttr(i);
		if ( sphIsBlobAttr(tAttr) )
		{
			dBlobAttrIds.Add(i);
			ARRAY_CONSTFOREACH ( iUpd, tUpd.m_dAttributes )
			{
				const TypedAttribute_t & tTypedAttr = tUpd.m_dAttributes[iUpd];

				if ( sphIsBlobAttr ( tTypedAttr.m_eType ) && tAttr.m_sName==tTypedAttr.m_sName )
				{
					dRemap[iUpd] = iBlobAttrId;
					bNeedBlobBuilder = true;
				}
			}

			++iBlobAttrId;
		}
	}

	if ( !bNeedBlobBuilder )
		return true;

	CSphTightVector<BYTE> tBlobPool;
	CSphScopedPtr<BlobRowBuilder_i> pBlobRowBuilder ( sphCreateBlobRowBuilderUpdate ( tCtx.m_tSchema, tBlobPool, tCtx.m_dSchemaUpdateMask ) );

	const CSphColumnInfo * pBlobLocator = tCtx.m_tSchema.GetAttr ( sphGetBlobLocatorName() );

	for ( const auto & tRow : dRows )
	{
		int iUpd = tRow.m_iIdx;
		tBlobPool.Resize(0);
		ARRAY_CONSTFOREACH ( iBlobId, dBlobAttrIds )
		{
			int iCol = dBlobAttrIds[iBlobId];
			if ( tCtx.m_dSchemaUpdateMask.BitGet(iCol) )
				continue;

			const CSphColumnInfo & tAttr = tCtx.m_tSchema.GetAttr(iCol);
			int iLengthBytes = 0;
			const BYTE * pData = sphGetBlobAttr ( tRow.m_pRow, tAttr.m_tLocator, tCtx.m_pBlobPool, iLengthBytes );
			if ( !pBlobRowBuilder->SetAttr ( iBlobId, pData, iLengthBytes, sError ) )
				return false;
		}

		int iPos = tUpd.GetRowOffset ( iUpd );
		ARRAY_CONSTFOREACH ( iCol, tUpd.m_dAttributes )
		{
			ESphAttr eAttr = tUpd.m_dAttributes[iCol].m_eType;

			if ( !sphIsBlobAttr(eAttr) || FitsInplaceJsonUpdate ( tCtx, iCol ) || !tCtx.m_dUpdatedAttrs[iCol].m_bExisting )
			{
				IncUpdatePoolPos ( tCtx, iCol, iPos );
				continue;
			}

			int iBlobId = dRemap[iCol];

			switch ( eAttr )
			{
			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
				{
					DWORD uLength = tUpd.m_dPool[iPos++];
					if ( iBlobId!=-1 )
					{
						pBlobRowBuilder->SetAttr ( iBlobId, (const BYTE *)(tUpd.m_dPool.Begin()+iPos), uLength*sizeof(DWORD), sError );
						tCtx.m_tUpd.MarkUpdated ( iUpd );
						tCtx.m_uUpdateMask |= ATTRS_BLOB_UPDATED;
					}

					iPos += uLength;
				}
				break;

			case SPH_ATTR_STRING:
				{
					DWORD uOffset = tUpd.m_dPool[iPos++];
					DWORD uLength = tUpd.m_dPool[iPos++];
					if ( iBlobId!=-1 )
					{

						pBlobRowBuilder->SetAttr ( iBlobId, &tUpd.m_dBlobs[uOffset], uLength, sError );
						tCtx.m_tUpd.MarkUpdated ( iUpd );
						tCtx.m_uUpdateMask |= ATTRS_BLOB_UPDATED;
					}
				}
				break;

			default:
				IncUpdatePoolPos ( tCtx, iCol, iPos );
				break;
			}
		}

		pBlobRowBuilder->Flush();

		assert(pBlobLocator);
		if ( !Update_WriteBlobRow ( tCtx, const_cast<CSphRowitem*> (tRow.m_pRow), tBlobPool.Begin(), tBlobPool.GetLength(), iBlobAttrId, pBlobLocator->m_tLocator, bCritical, sError ) )
			return false;
	}

	return true;
}


bool IndexUpdateHelper_c::Update_HandleJsonWarnings ( UpdateContext_t & tCtx, int iUpdated, CSphString & sWarning, CSphString & sError )
{
	if ( !tCtx.m_iJsonWarnings )
		return true;

	sWarning.SetSprintf ( "%d attribute(s) can not be updated (not found or incompatible types)", tCtx.m_iJsonWarnings );
	if ( !iUpdated )
		sError = sWarning;

	return !!iUpdated;
}


void IndexUpdateHelper_c::Update_Plain ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx )
{
	const auto & tUpd = *tCtx.m_tUpd.m_pUpdate;

	for ( const auto & tRow : dRows )
	{
		int iPos = tUpd.GetRowOffset ( tRow.m_iIdx );
		ARRAY_CONSTFOREACH ( iCol, tUpd.m_dAttributes )
		{
			ESphAttr eAttr = tUpd.m_dAttributes[iCol].m_eType;
			const UpdatedAttribute_t & tUpdAttr = tCtx.m_dUpdatedAttrs[iCol];

			// already updated?
			if ( sphIsBlobAttr(eAttr) || tUpdAttr.m_eAttrType==SPH_ATTR_JSON || !tUpdAttr.m_bExisting )
			{
				IncUpdatePoolPos ( tCtx, iCol, iPos );
				continue;
			}

			const CSphAttrLocator & tLoc = tUpdAttr.m_tLocator;

			bool bBigint = eAttr==SPH_ATTR_BIGINT;
			SphAttr_t uValue = bBigint ? MVA_UPSIZE ( &tUpd.m_dPool[iPos] ) : tUpd.m_dPool[iPos];

			if ( tUpdAttr.m_eConversion==CONVERSION_BIGINT2FLOAT ) // handle bigint(-1) -> float attr updates
				uValue = sphF2DW ( float((int64_t)uValue) );
			else if ( tUpdAttr.m_eConversion==CONVERSION_FLOAT2BIGINT ) // handle float(1.0) -> bigint attr updates
				uValue = (int64_t)sphDW2F((DWORD)uValue);

			Histogram_i * pHistogram = tUpdAttr.m_pHistogram;
			if ( pHistogram )
			{
				SphAttr_t tOldValue = sphGetRowAttr ( tRow.m_pRow, tLoc );
				pHistogram->Delete ( tOldValue );
				pHistogram->UpdateCounter ( uValue );
			}

			sphSetRowAttr ( const_cast<CSphRowitem*> ( tRow.m_pRow ), tLoc, uValue );
			tCtx.m_tUpd.MarkUpdated ( tRow.m_iIdx );
			tCtx.m_uUpdateMask |= ATTRS_UPDATED;

			// next
			IncUpdatePoolPos ( tCtx, iCol, iPos );
		}
	}
}

//////////////////////////////////////////////////////////////////////////

struct FileDebugCheckReader_c final : public DebugCheckReader_i
{
	FileDebugCheckReader_c ( CSphAutoreader * pReader )
		: m_pReader ( pReader )
	{}
	~FileDebugCheckReader_c () final
	{}
	int64_t GetLengthBytes () final
	{
		return ( m_pReader ? m_pReader->GetFilesize() : 0 );
	}
	bool GetBytes ( void * pData, int iSize ) final
	{
		if ( !m_pReader )
			return false;

		m_pReader->GetBytes ( pData, iSize );
		return !m_pReader->GetErrorFlag();
	}

	bool SeekTo ( int64_t iOff, int iHint ) final
	{
		if ( !m_pReader )
			return false;

		m_pReader->SeekTo ( iOff, iHint );
		return !m_pReader->GetErrorFlag();
	}

	CSphAutoreader * m_pReader = nullptr;
};


class QueryMvaContainer_c
{
public:
	CSphVector<OpenHash_T<CSphVector<int64_t>, int64_t, HashFunc_Int64_t>*> m_tContainer;

	~QueryMvaContainer_c()
	{
		for ( auto i : m_tContainer )
			delete i;
	}
};


/// this is my actual VLN-compressed phrase index implementation
class CSphIndex_VLN : public CSphIndex, public IndexUpdateHelper_c, public IndexAlterHelper_c, public DebugCheckHelper_c
{
	friend class DiskIndexQwordSetup_c;
	friend class CSphMerger;
	friend class AttrIndexBuilder_c;
	friend struct SphFinalMatchCalc_t;
	friend class KeepAttrs_c;
	friend class AttrMerger_c;

public:
	explicit			CSphIndex_VLN ( const char * szIndexName, const char * szFilename );
						~CSphIndex_VLN() override;

	int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer, CSphIndexProgress & tProgress ) final;

	bool				LoadHeader ( const char * sHeaderName, bool bStripPath, CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder, CSphString & sWarning );

	void				DebugDumpHeader ( FILE * fp, const char * sHeaderName, bool bConfig ) final;
	void				DebugDumpDocids ( FILE * fp ) final;
	void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID ) final;
	void				DebugDumpDict ( FILE * fp ) final;
	void				SetDebugCheck ( bool bCheckIdDups, int iCheckChunk ) final;
	int					DebugCheck ( FILE * fp ) final;
	template <class Qword> void		DumpHitlist ( FILE * fp, const char * sKeyword, bool bID );

	bool				Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings ) final;
	void				Dealloc () final;
	void				Preread () final;

	void				SetBase ( const char * sNewBase ) final;
	bool				Rename ( const char * sNewBase ) final;

	bool				Lock () final;
	void				Unlock () final;
	void				PostSetup() final {}

	bool				MultiQuery ( CSphQueryResult & pResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs ) const final;
	bool				MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult* pResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const final;
	bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const final;
	template <class Qword> bool		DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, bool bFillOnly, CSphString * pError ) const;
	bool 				FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const final;
	void				GetSuggest ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes ) const final;

	bool				Merge ( CSphIndex * pSource, const VecTraits_T<CSphFilterSettings> & dFilters, bool bSupressDstDocids, CSphIndexProgress & tProgress ) final;

	template <class QWORDDST, class QWORDSRC>
	static bool			MergeWords ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, VecTraits_T<RowID_t> dDstRows, VecTraits_T<RowID_t> dSrcRows, CSphHitBuilder * pHitBuilder, CSphString & sError, CSphIndexProgress & tProgress);
	static bool			DoMerge ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, ISphFilter * pFilter, CSphString & sError, CSphIndexProgress & tProgress, bool bSrcSettings, bool bSupressDstDocids );
	ISphFilter *		CreateMergeFilters ( const VecTraits_T<CSphFilterSettings> & dSettings ) const;
	template <class QWORD>
	static bool			DeleteField ( const CSphIndex_VLN * pIndex, CSphHitBuilder * pHitBuilder, CSphString & sError, CSphSourceStats & tStat, int iKillField );

	int					UpdateAttributes ( AttrUpdateInc_t & tUpd, bool & bCritical, CSphString & sError, CSphString & sWarning ) final;
	void				UpdateAttributesOffline ( VecTraits_T<PostponedUpdate_t> & dUpdates, IndexSegment_c * /*pSeg*/) final;

	// the only txn we can replay is 'update attributes', but it is processed by dedicated branch in binlog, so we have nothing to do here.
	Binlog::CheckTnxResult_t ReplayTxn (Binlog::Blop_e, CSphReader&, CSphString & , Binlog::CheckTxn_fn&&) final { return {}; }
	bool				SaveAttributes ( CSphString & sError ) const final;
	DWORD				GetAttributeStatus () const final;

	bool				AddRemoveAttribute ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, AttrEngine_e eEngine, CSphString & sError ) final;
	bool				AddRemoveField ( bool bAdd, const CSphString & sFieldName, DWORD uFieldFlags, CSphString & sError ) final;

	void				FlushDeadRowMap ( bool bWaitComplete ) const final;
	bool				LoadKillList ( CSphFixedVector<DocID_t> * pKillList, KillListTargets_c & tTargets, CSphString & sError ) const final;
	bool				AlterKillListTarget ( KillListTargets_c & tTargets, CSphString & sError ) final;
	void				KillExistingDocids ( CSphIndex * pTarget ) final;

	bool				EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const final;

	void				SetKeepAttrs ( const CSphString & sKeepAttrs, const StrVec_t & dAttrs ) final { m_sKeepAttrs = sKeepAttrs; m_dKeepAttrs = dAttrs; }
	RowID_t				GetRowidByDocid ( DocID_t tDocID ) const;
	int					Kill ( DocID_t tDocID ) final;
	int					KillMulti ( const VecTraits_T<DocID_t> & dKlist ) final;
	bool				IsAlive ( DocID_t tDocID ) const final;

	const CSphSourceStats &		GetStats () const final { return m_tStats; }
	int64_t *			GetFieldLens() const final { return m_tSettings.m_bIndexFieldLens ? m_dFieldLens.begin() : nullptr; }
	void				GetStatus ( CSphIndexStatus* ) const final;

	bool				PreallocWordlist();
	bool				PreallocAttributes();
	bool				PreallocDocidLookup();
	bool				PreallocKilllist();
	bool				PreallocHistograms ( StrVec_t & dWarnings );
	bool				PreallocDocstore();
	bool				PreallocColumnar();
	bool				PreallocSkiplist();

	CSphVector<SphAttr_t> 	BuildDocList () const final;

	// docstore-related section
	void				CreateReader ( int64_t iSessionId ) const final;
	bool				GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const final;
	int					GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const final;
	Bson_t				ExplainQuery ( const CSphString & sQuery ) const final;

	bool				CopyExternalFiles ( int iPostfix, StrVec_t & dCopied ) final;
	void				CollectFiles ( StrVec_t & dFiles, StrVec_t & dExt ) const final;

	bool				CheckEarlyReject ( const CSphVector<CSphFilterSettings> & dFilters, ISphFilter * pFilter, ESphCollation eCollation, const ISphSchema & tSchema ) const;

private:
	static const int			MIN_WRITE_BUFFER		= 262144;	///< min write buffer size
	static const int			DEFAULT_WRITE_BUFFER	= 1048576;	///< default write buffer size

private:
	// common stuff
	int								m_iLockFD;
	CSphSourceStats					m_tStats;			///< my stats
	CSphFixedVector<int64_t>		m_dFieldLens;	///< total per-field lengths summed over entire indexed data, in tokens
	CSphString						m_sKeepAttrs;			///< retain attributes of that index reindexing
	StrVec_t						m_dKeepAttrs;

private:
	int64_t						m_iDocinfo;				///< my docinfo cache size
	int64_t						m_iDocinfoIndex;		///< docinfo "index" entries count (each entry is 2x docinfo rows, for min/max)
	DWORD *						m_pDocinfoIndex;		///< docinfo "index", to accelerate filtering during full-scan (2x rows for each block, and 2x rows for the whole index, 1+m_uDocinfoIndex entries)
	int64_t						m_iMinMaxIndex;			///< stored min/max cache offset (counted in DWORDs)

	// !COMMIT slow setup data
	CSphMappedBuffer<DWORD>		m_tAttr;
	CSphMappedBuffer<BYTE>		m_tBlobAttrs;
	CSphMappedBuffer<BYTE>		m_tSkiplists;		///< (compressed) skiplists data
	CWordlist					m_tWordlist;		///< my wordlist

	DeadRowMap_Disk_c 			m_tDeadRowMap;

	CSphMappedBuffer<BYTE>		m_tDocidLookup;		///< speeds up docid-rowid lookups + used for applying killlist on startup
	LookupReader_c				m_tLookupReader;	///< used by getrowidbydocid

	CSphScopedPtr<Docstore_i> 	m_pDocstore {nullptr};
	CSphScopedPtr<columnar::Columnar_i> m_pColumnar {nullptr};

	DWORD						m_uVersion;				///< data files version
	volatile bool				m_bPassedRead;
	volatile bool				m_bPassedAlloc;
	bool						m_bIsEmpty;				///< do we have actually indexed documents (m_iTotalDocuments is just fetched documents, not indexed!)
	bool						m_bDebugCheck;
	bool						m_bCheckIdDups = false;

	mutable DWORD				m_uAttrsStatus = 0;

	DataReaderFactoryPtr_c		m_pDoclistFile;			///< doclist file
	DataReaderFactoryPtr_c		m_pHitlistFile;			///< hitlist file
	DataReaderFactoryPtr_c		m_pColumnarFile;			///< columnar file

	HistogramContainer_c *		m_pHistograms {nullptr};

private:
	CSphString					GetIndexFileName ( ESphExt eExt, bool bTemp=false ) const;
	CSphString					GetIndexFileName ( const char * szExt ) const;
	void						GetIndexFiles ( CSphVector<CSphString> & dFiles, const FilenameBuilder_i * pFilenameBuilder ) const override;

	bool						ParsedMultiQuery ( const CSphQuery & tQuery, CSphQueryResult & tResult, const VecTraits_T<ISphMatchSorter*> & dSorters, const XQQuery_t & tXQ, CSphDict * pDict, const CSphMultiQueryArgs & tArgs, CSphQueryNodeCache * pNodeCache, int64_t tmMaxTimer ) const;

	template <bool ROWID_LIMITS>
	void						ScanByBlocks ( const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer, const RowIdBoundaries_t * pBoundaries = nullptr ) const;
	void						RunFullscanOnAttrs ( const RowIdBoundaries_t & tBoundaries, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer, bool & bStop ) const;
	void						RunFullscanOnIterator ( RowidIterator_i * pIterator, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer ) const;
	bool						MultiScan ( CSphQueryResult & tResult, const CSphQuery & dQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const;

	template<bool USE_KLIST, bool RANDOMIZE, bool USE_FACTORS>
	void						MatchExtended ( CSphQueryContext & tCtx, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *>& dSorters, ISphRanker * pRanker, int iTag, int iIndexWeight ) const;

	const CSphRowitem *			FindDocinfo ( DocID_t tDocID ) const;

	const DWORD *				GetDocinfoByRowID ( RowID_t tRowID ) const;
	RowID_t						GetRowIDByDocinfo ( const CSphRowitem * pDocinfo ) const;

	void						SetupStarDict ( DictRefPtr_c &pDict ) const;
	void						SetupExactDict ( DictRefPtr_c &pDict ) const;

	bool						RelocateBlock ( int iFile, BYTE * pBuffer, int iRelocationSize, SphOffset_t * pFileSize, CSphBin & dMinBin, SphOffset_t * pSharedOffset );

	bool						SortDocidLookup ( int iFD, int nBlocks, int iMemoryLimit, int nLookupsInBlock, int nLookupsInLastBlock, CSphIndexProgress& tProgress );

	bool						CollectQueryMvas ( const CSphVector<CSphSource*> & dSources, QueryMvaContainer_c & tMvaContainer );

private:
	mutable HashCollection_c	m_dHashes;

	bool						JuggleFile ( ESphExt eExt, CSphString & sError, bool bNeedSrc=true, bool bNeedDst=true ) const;
	XQNode_t *					ExpandPrefix ( XQNode_t * pNode, CSphQueryResultMeta & tMeta, CSphScopedPayload * pPayloads, DWORD uQueryDebugFlags ) const;

	static std::pair<DWORD,DWORD>		CreateRowMapsAndCountTotalDocs ( const CSphIndex_VLN* pSrcIndex, const CSphIndex_VLN* pDstIndex, CSphFixedVector<RowID_t>& dSrcRowMap, CSphFixedVector<RowID_t>& dDstRowMap, const ISphFilter* pFilter, bool bSupressDstDocids, MergeCb_c& tMonitor );
	RowsToUpdateData_t			Update_CollectRowPtrs ( const UpdateContext_t & tCtx );
	RowsToUpdate_t				Update_PrepareGatheredRowPtrs ( RowsToUpdate_t & dWRows, const VecTraits_T<DocID_t>& dDocids );
	bool						Update_WriteBlobRow ( UpdateContext_t & tCtx, CSphRowitem * pDocinfo, const BYTE * pBlob,
										int iLength, int nBlobAttrs, const CSphAttrLocator & tBlobRowLoc, bool & bCritical, CSphString & sError ) override;
	void						Update_MinMax ( const RowsToUpdate_t& dRows, const UpdateContext_t & tCtx );
	bool						DoUpdateAttributes ( const RowsToUpdate_t& dRows, UpdateContext_t& tCtx, bool & bCritical, CSphString & sError );

	bool						Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const override;
	bool						AddRemoveColumnarAttr ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, const ISphSchema & tOldSchema, const ISphSchema & tNewSchema, CSphString & sError );
	bool						DeleteFieldFromDict ( int iFieldId, BuildHeader_t & tBuildHeader, CSphString & sError );
	bool						AddRemoveFieldFromDocstore ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, CSphString & sError );

	bool						Build_SetupInplace ( SphOffset_t & iHitsGap, int iHitsMax, int iFdHits ) const;
	bool						Build_SetupDocstore ( CSphScopedPtr<DocstoreBuilder_i> & pDocstore );
	bool						Build_SetupBlobBuilder ( CSphScopedPtr<BlobRowBuilder_i> & pBuilder );
	bool						Build_SetupColumnar ( CSphScopedPtr<columnar::Builder_i> & pBuilder );
	bool						Build_SetupHistograms ( CSphScopedPtr<HistogramContainer_c> & pContainer, CSphVector<std::pair<Histogram_i*,int>> & dHistograms );

	void						Build_AddToDocstore ( DocstoreBuilder_i * pDocstoreBuilder, CSphSource * pSource );
	bool						Build_StoreBlobAttrs ( DocID_t tDocId, SphOffset_t & tOffset, BlobRowBuilder_i & tBlobRowBuilderconst, QueryMvaContainer_c & tMvaContainer, AttrSource_i & tSource, bool bForceSource );
	void						Build_StoreColumnarAttrs ( DocID_t tDocId, columnar::Builder_i & tBuilder, CSphSource & tSource, QueryMvaContainer_c & tMvaContainer );
	void						Build_StoreHistograms ( CSphVector<std::pair<Histogram_i*,int>> dHistograms, CSphSource & tSource );

	bool						SpawnReader ( DataReaderFactoryPtr_c & m_pFile, ESphExt eExt, DataReaderFactory_c::Kind_e eKind, int iBuffer, FileAccess_e eAccess );
	bool						SpawnReaders();

	RowidIterator_i *			CreateColumnarAnalyzerOrPrefilter ( const CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged, const CSphVector<FilterTreeItem_t> & dFilterTree, const ISphFilter * pFilter, ESphCollation eCollation, const ISphSchema & tSchema, CSphString & sWarning ) const;

	bool						SplitQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const;
	RowidIterator_i *			SpawnIterators ( const CSphQuery & tQuery, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged, const ISphFilter * pFilter, const ISphSchema & tMaxSorterSchema, CSphString & sWarning ) const;
};

class AttrMerger_c
{
	AttrIndexBuilder_c				m_tMinMax;
	HistogramContainer_c			m_tHistograms;
	CSphVector<PlainOrColumnar_t>	m_dAttrsForHistogram;
	CSphFixedVector<DocidRowidPair_t> m_dDocidLookup {0};
	CSphWriter						m_tWriterSPA;
	BlobRowBuilder_i * 				m_pBlobRowBuilder = nullptr;
	DocstoreBuilder_i * 			m_pDocstoreBuilder = nullptr;
	columnar::Builder_i *			m_pColumnarBuilder = nullptr;
	RowID_t							m_tResultRowID = 0;
	int64_t							m_iTotalBytes = 0;

	MergeCb_c & 					m_tMonitor;
	CSphString &					m_sError;
	int64_t							m_iTotalDocs;

private:
	bool CopyPureColumnarAttributes ( const CSphIndex_VLN& tIndex, const VecTraits_T<RowID_t>& dRowMap );
	bool CopyMixedAttributes ( const CSphIndex_VLN& tIndex, const VecTraits_T<RowID_t>& dRowMap );

public:
	AttrMerger_c ( MergeCb_c & tMonitor, CSphString & sError, int64_t iTotalDocs )
		: m_tMonitor ( tMonitor )
		, m_sError ( sError)
		, m_iTotalDocs ( iTotalDocs )
	{
	}

	~AttrMerger_c()
	{
		SafeDelete ( m_pColumnarBuilder );
		SafeDelete ( m_pDocstoreBuilder );
		SafeDelete ( m_pBlobRowBuilder );
	}

	bool Prepare ( const CSphIndex_VLN* pSrcIndex, const CSphIndex_VLN* pDstIndex );
	bool CopyAttributes ( const CSphIndex_VLN& tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive );
	bool FinishMergeAttributes ( const CSphIndex_VLN* pDstIndex, BuildHeader_t& tBuildHeader );
};


/////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

/// indexer warning
void sphWarn ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	fprintf ( stdout, "WARNING: " );
	vfprintf ( stdout, sTemplate, ap );
	fprintf ( stdout, "\n" );
	va_end ( ap );
}

//////////////////////////////////////////////////////////////////////////

static int 		g_iIOpsDelay = 0;
static int 		g_iMaxIOSize = 0;
static std::atomic<int64_t> g_tmNextIOTime {0};

void sphSetThrottling ( int iMaxIOps, int iMaxIOSize )
{
	g_iIOpsDelay = iMaxIOps ? 1000000/iMaxIOps : iMaxIOps;
	g_iMaxIOSize = iMaxIOSize;
}


static inline void ThrottleSleep ()
{
	if ( !g_iIOpsDelay )
		return;

	auto tmTimer = sphMicroTimer ();
	while ( tmTimer < g_tmNextIOTime.load(std::memory_order_relaxed) ) // m.b. >1 sleeps if another thread more lucky
	{
		sphSleepMsec ( ( int ) ( g_tmNextIOTime.load(std::memory_order_relaxed) - tmTimer ) / 1000 );
		tmTimer = sphMicroTimer();
	}
	g_tmNextIOTime.store ( tmTimer+g_iIOpsDelay, std::memory_order_relaxed );
}


bool sphWriteThrottled ( int iFD, const void * pBuf, int64_t iCount, const char * sName, CSphString & sError )
{
	if ( iCount<=0 )
		return true;

	// by default, slice ios by at most 1 GB
	int iChunkSize = ( 1UL<<30 );

	// when there's a sane max_iosize (4K to 1GB), use it
	if ( g_iMaxIOSize>=4096 )
		iChunkSize = Min ( iChunkSize, g_iMaxIOSize );

	CSphIOStats * pIOStats = GetIOStats();

	// while there's data, write it chunk by chunk
	auto * p = (const BYTE*) pBuf;
	while ( iCount )
	{
		// wait for a timely occasion
		ThrottleSleep ();

		// write (and maybe time)
		int64_t tmTimer = 0;
		if ( pIOStats )
			tmTimer = sphMicroTimer();

		auto iToWrite = (int) Min ( iCount, iChunkSize );
		int iWritten = ::write ( iFD, p, iToWrite );

		if ( pIOStats )
		{
			pIOStats->m_iWriteTime += sphMicroTimer() - tmTimer;
			pIOStats->m_iWriteOps++;
			pIOStats->m_iWriteBytes += iWritten;
		}
		if ( sphInterrupted () && iWritten!=iToWrite )
		{
			sError.SetSprintf ( "%s: write interrupted: %d of %d bytes written", sName, iWritten, iToWrite );
			return false;
		}

		// success? rinse, repeat
		if ( iWritten==iToWrite )
		{
			iCount -= iToWrite;
			p += iToWrite;
			continue;
		}

		// failure? report, bailout
		if ( iWritten<0 )
			sError.SetSprintf ( "%s: write error: %s", sName, strerrorm(errno) );
		else
			sError.SetSprintf ( "%s: write error: %d of %d bytes written", sName, iWritten, iToWrite );
		return false;
	}
	return true;
}


size_t sphReadThrottled ( int iFD, void * pBuf, size_t iCount )
{
	if ( iCount<=0 )
		return iCount;

	auto iStep = g_iMaxIOSize ? Min ( iCount, (size_t)g_iMaxIOSize ) : iCount;
	auto * p = ( BYTE * ) pBuf;
	size_t nBytesToRead = iCount;
	while ( iCount && !sphInterrupted() )
	{
		ThrottleSleep();
		auto iChunk = (long) Min ( iCount, iStep );
		auto iRead = sphRead ( iFD, p, iChunk );
		p += iRead;
		iCount -= iRead;
		if ( iRead!=iChunk )
			break;
	}
	return nBytesToRead-iCount; // FIXME? we sure this is under 2gb?
}


void SafeClose ( int & iFD )
{
	if ( iFD>=0 )
		::close ( iFD );
	iFD = -1;
}

//////////////////////////////////////////////////////////////////////////

#if !_WIN32
char * strlwr ( char * s )
{
	while ( *s )
	{
		*s = tolower ( *s );
		s++;
	}
	return s;
}
#endif


#if _WIN32

bool sphLockEx ( int iFile, bool bWait )
{
	HANDLE hHandle = (HANDLE) _get_osfhandle ( iFile );
	if ( hHandle!=INVALID_HANDLE_VALUE )
	{
		OVERLAPPED tOverlapped;
		memset ( &tOverlapped, 0, sizeof ( tOverlapped ) );
		return !!LockFileEx ( hHandle, LOCKFILE_EXCLUSIVE_LOCK | ( bWait ? 0 : LOCKFILE_FAIL_IMMEDIATELY ), 0, 1, 0, &tOverlapped );
	}

	return false;
}

void sphLockUn ( int iFile )
{
	HANDLE hHandle = (HANDLE) _get_osfhandle ( iFile );
	if ( hHandle!=INVALID_HANDLE_VALUE )
	{
		OVERLAPPED tOverlapped;
		memset ( &tOverlapped, 0, sizeof ( tOverlapped ) );
		UnlockFileEx ( hHandle, 0, 1, 0, &tOverlapped );
	}
}

#else

bool sphLockEx ( int iFile, bool bWait )
{
	struct flock tLock;
	tLock.l_type = F_WRLCK;
	tLock.l_whence = SEEK_SET;
	tLock.l_start = 0;
	tLock.l_len = 0;
	if ( !bWait )
		return ( fcntl ( iFile, F_SETLK, &tLock )!=-1 );

#if HAVE_F_SETLKW
	return ( fcntl ( iFile, F_SETLKW, &tLock )!=-1 );
#else
	for (;;)
	{
		int iResult = fcntl ( iFile, F_SETLK, &tLock )
		if ( iResult!=-1 )
			return true;
		if ( errno==EACCES || errno==EAGAIN )
			sphSleepMsec(10);
		else
			return false;
	}
#endif

}


void sphLockUn ( int iFile )
{
	struct flock tLock;
	tLock.l_type = F_UNLCK;
	tLock.l_whence = SEEK_SET;
	tLock.l_start = 0;
	tLock.l_len = 0;

	if ( fcntl ( iFile, F_SETLK, &tLock )==-1 )
		sphWarning ( "sphLockUn: failed fcntl. Error: %d '%s'", errno, strerror ( errno ) );

}
#endif


void sphSleepMsec ( int iMsec )
{
	if ( iMsec<0 )
		return;

#if _WIN32
	Sleep ( iMsec );

#else
	struct timeval tvTimeout;
	tvTimeout.tv_sec = iMsec / 1000; // full seconds
	tvTimeout.tv_usec = ( iMsec % 1000 ) * 1000; // remainder is msec, so *1000 for usec

	select ( 0, NULL, NULL, NULL, &tvTimeout ); // FIXME? could handle EINTR
#endif
}


void SetUnhintedBuffer ( int iReadUnhinted )
{
	if ( iReadUnhinted<=0 )
		iReadUnhinted = DEFAULT_READ_UNHINTED;
	g_iReadUnhinted = Max ( iReadUnhinted, MIN_READ_UNHINTED );
}


int GetUnhintedBuffer()
{
	return g_iReadUnhinted;
}


// returns correct size even if iBuf is 0
int GetReadBuffer ( int iBuf )
{
	if ( !iBuf )
		return DEFAULT_READ_BUFFER;
	return Max ( iBuf, MIN_READ_BUFFER );
}

bool IsMlock ( FileAccess_e eType ) { return eType==FileAccess_e::MLOCK; }
bool IsOndisk ( FileAccess_e eType ) { return eType==FileAccess_e::FILE || eType==FileAccess_e::MMAP; }

bool FileAccessSettings_t::operator== ( const FileAccessSettings_t & tOther ) const
{
	return ( m_eAttr==tOther.m_eAttr && m_eBlob==tOther.m_eBlob && m_eDoclist==tOther.m_eDoclist && m_eHitlist==tOther.m_eHitlist &&
		m_iReadBufferDocList==tOther.m_iReadBufferDocList && m_iReadBufferHitList==tOther.m_iReadBufferHitList );
}

bool FileAccessSettings_t::operator!= ( const FileAccessSettings_t & tOther ) const
{
	return !operator==( tOther );
}


//////////////////////////////////////////////////////////////////////////

float CSphMatch::GetAttrFloat ( const CSphAttrLocator & tLoc ) const
{
	return sphDW2F ( (DWORD)sphGetRowAttr ( tLoc.m_bDynamic ? m_pDynamic : m_pStatic, tLoc ) );
}


void CSphMatch::SetAttrFloat ( const CSphAttrLocator & tLoc, float fValue ) const
{
	assert ( tLoc.m_bDynamic );
	assert ( tLoc.GetMaxRowitem() < (int)m_pDynamic[-1] );
	sphSetRowAttr ( m_pDynamic, tLoc, sphF2DW ( fValue ) );
}


ByteBlob_t CSphMatch::FetchAttrData ( const CSphAttrLocator & tLoc, const BYTE * pPool ) const
{
	if ( tLoc.IsBlobAttr ())
		return sphGetBlobAttr ( *this, tLoc, pPool );
	return sphUnpackPtrAttr ((const BYTE *) GetAttr ( tLoc ));
}

/////////////////////////////////////////////////////////////////////////////
// TOKENIZING EXCEPTIONS
/////////////////////////////////////////////////////////////////////////////

/// exceptions trie, stored in a tidy simple blob
/// we serialize each trie node as follows:
///
/// int result_offset, 0 if no output mapping
/// BYTE num_bytes, 0 if no further valid bytes can be accepted
/// BYTE values[num_bytes], known accepted byte values
/// BYTE offsets[num_bytes], and the respective next node offsets
///
/// output mappings themselves are serialized just after the nodes,
/// as plain old ASCIIZ strings
class ExceptionsTrie_c
{
	friend class		ExceptionsTrieGen_c;

protected:
	int					m_dFirst[256];	///< table to speedup 1st byte lookup
	CSphVector<BYTE>	m_dData;		///< data blob
	int					m_iCount;		///< number of exceptions
	int					m_iMappings;	///< offset where the nodes end, and output mappings start

public:
	const BYTE * GetMapping ( int i ) const
	{
		assert ( i>=0 && i<m_iMappings );
		int p = *(int*)&m_dData[i];
		if ( !p )
			return NULL;
		assert ( p>=m_iMappings && p<m_dData.GetLength() );
		return &m_dData[p];
	}

	int GetFirst ( BYTE v ) const
	{
		return m_dFirst[v];
	}

	int GetNext ( int i, BYTE v ) const
	{
		assert ( i>=0 && i<m_iMappings );
		if ( i==0 )
			return m_dFirst[v];
		BYTE * p = &m_dData[i];
		int n = p[4];
		p += 5;
		for ( i=0; i<n; i++ )
			if ( p[i]==v )
				return *(int*)&p [ n + 4*i ]; // FIXME? unaligned
		return -1;
	}

public:
	void Export ( CSphWriter & w ) const
	{
		CSphVector<BYTE> dPrefix;
		int iCount = 0;

		w.PutDword ( m_iCount );
		Export ( [&w] (const char* szLine) { w.PutString ( szLine ); }, dPrefix, 0, &iCount);
		assert ( iCount==m_iCount );
	}

	void Export ( JsonEscapedBuilder & tOut ) const
	{
		CSphVector<BYTE> dPrefix;
		int iCount = 0;

		Export ( [&tOut] (const char* szLine) { tOut.FixupSpacedAndAppendEscaped ( szLine ); }, dPrefix, 0, &iCount);
		assert ( iCount==m_iCount );
	}

protected:
	template<typename WRITER>
	void Export ( WRITER&& W, CSphVector<BYTE>& dPrefix, int iNode, int* pCount ) const
	{
		assert ( iNode>=0 && iNode<m_iMappings );
		BYTE * p = &m_dData[iNode];

		int iTo = *(int*)const_cast<BYTE*>(p);
		if ( iTo>0 )
		{
			CSphString s;
			const char * sTo = (char*)&m_dData[iTo];
			s.SetBinary ( (char*)dPrefix.Begin(), dPrefix.GetLength() );
			s.SetSprintf ( "%s => %s\n", s.cstr(), sTo );
			W ( s.cstr() );
			(*pCount)++;
		}

		int n = p[4];
		if ( n==0 )
			return;

		p += 5;
		for ( int i=0; i<n; i++ )
		{
			dPrefix.Add ( p[i] );
			Export ( W, dPrefix, *(int*)&p[n+4*i], pCount );
			dPrefix.Pop();
		}
	}
};


/// intermediate exceptions trie node
/// only used by ExceptionsTrieGen_c, while building a blob
class ExceptionsTrieNode_c
{
	friend class						ExceptionsTrieGen_c;

protected:
	struct Entry_t
	{
		BYTE					m_uValue;
		ExceptionsTrieNode_c *	m_pKid;
	};

	CSphString					m_sTo;		///< output mapping for current prefix, if any
	CSphVector<Entry_t>			m_dKids;	///< known and accepted incoming byte values

public:
	~ExceptionsTrieNode_c()
	{
		ARRAY_FOREACH ( i, m_dKids )
			SafeDelete ( m_dKids[i].m_pKid );
	}

	/// returns false on a duplicate "from" part, or true on success
	bool AddMapping ( const BYTE * sFrom, const BYTE * sTo )
	{
		// no more bytes to consume? this is our output mapping then
		if ( !*sFrom )
		{
			if ( !m_sTo.IsEmpty() )
				return false;
			m_sTo = (const char*)sTo;
			return true;
		}

		int i;
		for ( i=0; i<m_dKids.GetLength(); i++ )
			if ( m_dKids[i].m_uValue==*sFrom )
				break;
		if ( i==m_dKids.GetLength() )
		{
			Entry_t & t = m_dKids.Add();
			t.m_uValue = *sFrom;
			t.m_pKid = new ExceptionsTrieNode_c();
		}
		return m_dKids[i].m_pKid->AddMapping ( sFrom+1, sTo );
	}
};


/// exceptions trie builder
/// plain old text mappings in, nice useful trie out
class ExceptionsTrieGen_c
{
protected:
	ExceptionsTrieNode_c *	m_pRoot;
	int						m_iCount;

public:
	ExceptionsTrieGen_c()
	{
		m_pRoot = new ExceptionsTrieNode_c();
		m_iCount = 0;
	}

	~ExceptionsTrieGen_c()
	{
		SafeDelete ( m_pRoot );
	}

	/// trims left/right whitespace, folds inner whitespace
	void FoldSpace ( char * s ) const
	{
		// skip leading spaces
		char * d = s;
		while ( *s && sphIsSpace(*s) )
			s++;

		// handle degenerate (empty string) case
		if ( !*s )
		{
			*d = '\0';
			return;
		}

		while ( *s )
		{
			// copy another token, add exactly 1 space after it, and skip whitespace
			while ( *s && !sphIsSpace(*s) )
				*d++ = *s++;
			*d++ = ' ';
			while ( sphIsSpace(*s) )
				s++;
		}

		// replace that last space that we added
		d[-1] = '\0';
	}

	bool ParseLine ( char * sBuffer, CSphString & sError )
	{
		#define LOC_ERR(_arg) { sError = _arg; return false; }
		assert ( m_pRoot );

		// extract map-from and map-to parts
		char * sSplit = strstr ( sBuffer, "=>" );
		if ( !sSplit )
			LOC_ERR ( "mapping token (=>) not found" );

		char * sFrom = sBuffer;
		char * sTo = sSplit + 2; // skip "=>"
		*sSplit = '\0';

		// trim map-from, map-to
		FoldSpace ( sFrom );
		FoldSpace ( sTo );
		if ( !*sFrom )
			LOC_ERR ( "empty map-from part" );
		if ( !*sTo )
			LOC_ERR ( "empty map-to part" );
		if ( (int)strlen(sFrom) > MAX_KEYWORD_BYTES )
			LOC_ERR ( "map-from part too long" );
		if ( (int)strlen(sTo)>MAX_KEYWORD_BYTES )
			LOC_ERR ( "map-from part too long" );

		// all parsed ok; add it!
		if ( m_pRoot->AddMapping ( (BYTE*)sFrom, (BYTE*)sTo ) )
			m_iCount++;
		else
			LOC_ERR ( "duplicate map-from part" );

		return true;
		#undef LOC_ERR
	}

	ExceptionsTrie_c * Build()
	{
		if ( !m_pRoot || !m_pRoot->m_sTo.IsEmpty() || m_pRoot->m_dKids.GetLength()==0 )
			return NULL;

		ExceptionsTrie_c * pRes = new ExceptionsTrie_c();
		pRes->m_iCount = m_iCount;

		// save the nodes themselves
		CSphVector<BYTE> dMappings;
		SaveNode ( pRes, m_pRoot, dMappings );

		// append and fixup output mappings
		CSphVector<BYTE> & d = pRes->m_dData;
		pRes->m_iMappings = d.GetLength();
		d.Append ( dMappings );

		BYTE * p = d.Begin();
		BYTE * pMax = p + pRes->m_iMappings;
		while ( p<pMax )
		{
			// fixup offset in the current node, if needed
			int * pOff = (int*)p; // FIXME? unaligned
			if ( (*pOff)<0 )
				*pOff = 0; // convert -1 to 0 for non-outputs
			else
				(*pOff) += pRes->m_iMappings; // fixup offsets for outputs

			// proceed to the next node
			int n = p[4];
			p += 5 + 5*n;
		}
		assert ( p==pMax );

		// build the speedup table for the very 1st byte
		for ( int i=0; i<256; i++ )
			pRes->m_dFirst[i] = -1;
		int n = d[4];
		for ( int i=0; i<n; i++ )
			pRes->m_dFirst [ d[5+i] ] = *(int*)&pRes->m_dData [ 5+n+4*i ];

		SafeDelete ( m_pRoot );
		m_pRoot = new ExceptionsTrieNode_c();
		m_iCount = 0;
		return pRes;
	}

protected:
	void SaveInt ( CSphVector<BYTE> & v, int p, int x )
	{
#if USE_LITTLE_ENDIAN
		v[p] = x & 0xff;
		v[p+1] = (x>>8) & 0xff;
		v[p+2] = (x>>16) & 0xff;
		v[p+3] = (x>>24) & 0xff;
#else
		v[p] = (x>>24) & 0xff;
		v[p+1] = (x>>16) & 0xff;
		v[p+2] = (x>>8) & 0xff;
		v[p+3] = x & 0xff;
#endif
	}

	int SaveNode ( ExceptionsTrie_c * pRes, ExceptionsTrieNode_c * pNode, CSphVector<BYTE> & dMappings )
	{
		CSphVector<BYTE> & d = pRes->m_dData; // shortcut

		// remember the start node offset
		int iRes = d.GetLength();
		int n = pNode->m_dKids.GetLength();
		assert (!( pNode->m_sTo.IsEmpty() && n==0 ));

		// save offset into dMappings, or temporary (!) save -1 if there is no output mapping
		// note that we will fixup those -1's to 0's afterwards
		int iOff = -1;
		if ( !pNode->m_sTo.IsEmpty() )
		{
			iOff = dMappings.GetLength();
			int iLen = pNode->m_sTo.Length();
			memcpy ( dMappings.AddN ( iLen+1 ), pNode->m_sTo.cstr(), iLen+1 );
		}
		d.AddN(4);
		SaveInt ( d, d.GetLength()-4, iOff );

		// sort children nodes by value
		pNode->m_dKids.Sort ( bind ( &ExceptionsTrieNode_c::Entry_t::m_uValue ) );

		// save num_values, and values[]
		d.Add ( (BYTE)n );
		ARRAY_FOREACH ( i, pNode->m_dKids )
			d.Add ( pNode->m_dKids[i].m_uValue );

		// save offsets[], and the respective child nodes
		int p = d.GetLength();
		d.AddN ( 4*n );
		for ( int i=0; i<n; i++, p+=4 )
			SaveInt ( d, p, SaveNode ( pRes, pNode->m_dKids[i].m_pKid, dMappings ) );
		assert ( p==iRes+5+5*n );

		// done!
		return iRes;
	}
};

/////////////////////////////////////////////////////////////////////////////
// TOKENIZERS
/////////////////////////////////////////////////////////////////////////////

inline int sphUTF8Decode ( const BYTE * & pBuf ); // forward ref for GCC
inline int sphUTF8Encode ( BYTE * pBuf, int iCode ); // forward ref for GCC


class CSphTokenizerBase : public ISphTokenizer
{
public:
					CSphTokenizerBase();

	bool			SetCaseFolding ( const char * sConfig, CSphString & sError ) final;
	bool			LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, StrVec_t & dWarnings, CSphString & sError ) final;
	void			WriteSynonyms ( CSphWriter & tWriter ) const final;
	void 			WriteSynonyms ( JsonEscapedBuilder & tOut ) const final;

	void			CloneBase ( const CSphTokenizerBase * pFrom, ESphTokenizerClone eMode );

	const char *	GetTokenStart () const final	{ return (const char *) m_pTokenStart; }
	const char *	GetTokenEnd () const final		{ return (const char *) m_pTokenEnd; }
	const char *	GetBufferPtr () const final		{ return (const char *) m_pCur; }
	const char *	GetBufferEnd () const final		{ return (const char *) m_pBufferMax; }
	void			SetBufferPtr ( const char * sNewPtr ) final;
	uint64_t		GetSettingsFNV () const final;

	bool			SetBlendChars ( const char * sConfig, CSphString & sError ) final;
	bool			WasTokenMultiformDestination ( bool &, int & ) const final { return false; }

public:
	// lightweight clones must impose a lockdown on some methods
	// (specifically those that change the lowercaser data table)

	void AddPlainChar ( char c ) final
	{
		assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
		ISphTokenizer::AddPlainChar ( c );
	}

	void AddSpecials ( const char * sSpecials ) final
	{
		assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
		ISphTokenizer::AddSpecials ( sSpecials );
	}

	void Setup ( const CSphTokenizerSettings & tSettings ) final
	{
		assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
		ISphTokenizer::Setup ( tSettings );
	}

	bool RemapCharacters ( const char * sConfig, DWORD uFlags, const char * sSource, bool bCanRemap, CSphString & sError ) final
	{
		assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
		return ISphTokenizer::RemapCharacters ( sConfig, uFlags, sSource, bCanRemap, sError );
	}

protected:
			~CSphTokenizerBase() override;

	bool	BlendAdjust ( const BYTE * pPosition );
	int		CodepointArbitrationI ( int iCodepoint );
	int		CodepointArbitrationQ ( int iCodepoint, bool bWasEscaped, BYTE uNextByte );

protected:
	const BYTE *		m_pBuffer = nullptr;				///< my buffer
	const BYTE *		m_pBufferMax = nullptr;				///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	const BYTE *		m_pCur = nullptr;					///< current position
	const BYTE *		m_pTokenStart = nullptr;			///< last token start point
	const BYTE *		m_pTokenEnd = nullptr;				///< last token end point

	BYTE				m_sAccum [ 3*SPH_MAX_WORD_LEN+3 ];	///< folded token accumulator
	BYTE *				m_pAccum = nullptr;					///< current accumulator position
	int					m_iAccum = 0;						///< boundary token size

	BYTE				m_sAccumBlend [ 3*SPH_MAX_WORD_LEN+3 ];	///< blend-acc, an accumulator copy for additional blended variants
	int					m_iBlendNormalStart = 0;			///< points to first normal char in the accumulators (might be NULL)
	int					m_iBlendNormalEnd = 0;				///< points just past (!) last normal char in the accumulators (might be NULL)

	ExceptionsTrie_c *	m_pExc = nullptr;					///< exceptions trie, if any

	bool				m_bHasBlend = false;
	const BYTE *		m_pBlendStart = nullptr;
	const BYTE *		m_pBlendEnd = nullptr;

	ESphTokenizerClone m_eMode { SPH_CLONE_INDEX };
};


/// methods that get specialized with regards to charset type
/// aka GetCodepoint() decoder and everything that depends on it
class CSphTokenizerBase2 : public CSphTokenizerBase
{
protected:
	/// get codepoint
	inline int GetCodepoint ()
	{
		while ( m_pCur<m_pBufferMax )
		{
			int iCode = sphUTF8Decode ( m_pCur );
			if ( iCode>=0 )
				return iCode; // successful decode
		}
		return -1; // eof
	}

	/// accum codepoint
	inline void AccumCodepoint ( int iCode )
	{
		assert ( iCode>0 );
		assert ( m_iAccum>=0 );

		// throw away everything which is over the token size
		bool bFit = ( m_iAccum<SPH_MAX_WORD_LEN );
		bFit &= ( m_pAccum-m_sAccum+SPH_MAX_UTF8_BYTES<=(int)sizeof(m_sAccum));

		if ( bFit )
		{
			m_pAccum += sphUTF8Encode ( m_pAccum, iCode );
			assert ( m_pAccum>=m_sAccum && m_pAccum<m_sAccum+sizeof(m_sAccum) );
			m_iAccum++;
		}
	}

protected:
	BYTE *	GetBlendedVariant ();
	bool	CheckException ( const BYTE * pStart, const BYTE * pCur, bool bQueryMode );

	template < bool IS_QUERY, bool IS_BLEND >
	BYTE *	DoGetToken();

	void	FlushAccum ();

public:
	int		SkipBlended () final;
};


/// UTF-8 tokenizer
template < bool IS_QUERY >
class CSphTokenizer_UTF8 : public CSphTokenizerBase2
{
public:
						CSphTokenizer_UTF8 ();
	void				SetBuffer ( const BYTE * sBuffer, int iLength ) final;
	BYTE *				GetToken () override;
	ISphTokenizer *		Clone ( ESphTokenizerClone eMode ) const final;
	int					GetCodepointLength ( int iCode ) const final;
	int					GetMaxCodepointLength () const final { return m_tLC.GetMaxCodepointLength(); }
};


/// UTF-8 tokenizer with n-grams
template < bool IS_QUERY >
class CSphTokenizer_UTF8Ngram : public CSphTokenizer_UTF8<IS_QUERY>
{
public:
	bool		SetNgramChars ( const char * sConfig, CSphString & sError ) final;
	void		SetNgramLen ( int iLen ) final;
	BYTE *		GetToken () final;

protected:
	int			m_iNgramLen = 1;
};


struct CSphNormalForm
{
	CSphString				m_sForm;
	int						m_iLengthCP;
};

struct CSphMultiform
{
	int								m_iFileId;
	CSphTightVector<CSphNormalForm>	m_dNormalForm;
	CSphTightVector<CSphString>		m_dTokens;
};


struct CSphMultiforms
{
	int							m_iMinTokens;
	int							m_iMaxTokens;
	CSphVector<CSphMultiform*>	m_pForms;		// OPTIMIZE? blobify?
};


struct CSphMultiformContainer
{
							CSphMultiformContainer () : m_iMaxTokens ( 0 ) {}

	int						m_iMaxTokens;
	using CSphMultiformHash = CSphOrderedHash < CSphMultiforms *, CSphString, CSphStrHashFunc, 131072 >;
	CSphMultiformHash	m_Hash;
};


/// token filter for multiforms support
class CSphMultiformTokenizer : public CSphTokenFilter
{
	using Base = CSphTokenFilter;
public:
	CSphMultiformTokenizer ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer );


public:
	void	SetBuffer ( const BYTE * sBuffer, int iLength ) final;
	BYTE *	GetToken () final;
	void	EnableTokenizedMultiformTracking () final	{ m_bBuildMultiform = true; }
	int		GetLastTokenLen () const final				{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_iTokenLen : Base::GetLastTokenLen(); }
	bool	GetBoundary () final						{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBoundary : Base::GetBoundary(); }
	bool	WasTokenSpecial () final					{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bSpecial : Base::WasTokenSpecial(); }
	int		GetOvershortCount () final					{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_iOvershortCount : Base::GetOvershortCount(); }
	BYTE *	GetTokenizedMultiform () final				{ return m_sTokenizedMultiform[0] ? m_sTokenizedMultiform : nullptr; }
	bool	TokenIsBlended () const final				{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBlended : Base::TokenIsBlended(); }
	bool	TokenIsBlendedPart () const final			{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_bBlendedPart : Base::TokenIsBlendedPart(); }
	int		SkipBlended () final;

public:
	ISphTokenizer *			Clone ( ESphTokenizerClone eMode ) const final;
	const char *			GetTokenStart () const final	{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_szTokenStart : Base::GetTokenStart(); }
	const char *			GetTokenEnd () const final		{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_szTokenEnd : Base::GetTokenEnd(); }
	const char *			GetBufferPtr () const final		{ return m_iStart<m_dStoredTokens.GetLength() ? m_dStoredTokens[m_iStart].m_pBufferPtr : Base::GetBufferPtr(); }
	void					SetBufferPtr ( const char * sNewPtr ) final;
	uint64_t				GetSettingsFNV () const final;
	bool					WasTokenMultiformDestination ( bool & bHead, int & iDestCount ) const final;

private:
	const CSphMultiformContainer *	m_pMultiWordforms;
	int								m_iStart = 0;
	int								m_iOutputPending = -1;
	const CSphMultiform *			m_pCurrentForm = nullptr;

	bool				m_bBuildMultiform = false;
	BYTE				m_sTokenizedMultiform [ 3*SPH_MAX_WORD_LEN+4 ];

	CSphVector<StoredToken_t>		m_dStoredTokens;
};


/// token filter for bigram indexing
///
/// passes tokens through until an eligible pair is found
/// then buffers and returns that pair as a blended token
/// then returns the first token as a regular one
/// then pops the first one and cycles again
///
/// pair (aka bigram) eligibility depends on bigram_index value
/// "all" means that all token pairs gets indexed
/// "first_freq" means that 1st token must be from bigram_freq_words
/// "both_freq" means that both tokens must be from bigram_freq_words
class CSphBigramTokenizer : public CSphTokenFilter
{
protected:
	enum
	{
		BIGRAM_CLEAN,	///< clean slate, nothing accumulated
		BIGRAM_PAIR,	///< just returned a pair from m_sBuf, and m_iFirst/m_pSecond are correct
		BIGRAM_FIRST	///< just returned a first token from m_sBuf, so m_iFirst/m_pSecond are still good
	}		m_eState = BIGRAM_CLEAN;
	BYTE	m_sBuf [ MAX_KEYWORD_BYTES ];	///< pair buffer
	BYTE *	m_pSecond = nullptr;			///< second token pointer
	int		m_iFirst = 0;					///< first token length, bytes
	CSphRefcountedPtr<const CSphBigramTokenizer>	m_pFather; ///< used by clones to share state

	// unchanged state (not need to copy on clone)
	ESphBigram			m_eMode;			///< bigram indexing mode
	BYTE				m_uMaxLen = 0;		///< max bigram_freq_words length
	CSphFixedVector<int> m_dWordsHash {0};	///< offsets into m_dWords hashed by 1st byte
	CSphVector<BYTE>	m_dWords;			///< case-folded, sorted bigram_freq_words

public:
	CSphBigramTokenizer ( ISphTokenizer * pTok, ESphBigram eMode, StrVec_t & dWords )
		: CSphTokenFilter ( pTok )
	{
		assert ( pTok );
		assert ( eMode!=SPH_BIGRAM_NONE );
		assert ( eMode==SPH_BIGRAM_ALL || dWords.GetLength() );

		m_sBuf[0] = 0;
		m_dWordsHash.Reset(256);
		m_dWordsHash.ZeroVec();

		m_eMode = eMode;

		// only keep unique, real, short enough words
		dWords.Uniq();
		for ( const auto & sWord : dWords )
		{
			int iLen = Min ( sWord.Length(), (int)255 );
			if ( !iLen )
				continue;

			m_uMaxLen = Max ( m_uMaxLen, (BYTE)iLen );

			// hash word blocks by the first letter
			BYTE uFirst = *(BYTE*)const_cast<char*>( sWord.cstr() );
			if ( !m_dWordsHash [ uFirst ] )
			{
				m_dWords.Add ( 0 ); // end marker for the previous block
				m_dWordsHash [ uFirst ] = m_dWords.GetLength(); // hash new block
			}

			// store that word
			m_dWords.ReserveGap ( iLen+1 );
			m_dWords.Add ( BYTE(iLen) );
			m_dWords.Append ( sWord.cstr(), iLen);
		}
		m_dWords.Add ( 0 );
	}

	CSphBigramTokenizer ( ISphTokenizer * pTok, const CSphBigramTokenizer * pBase )
		: CSphTokenFilter ( pTok )
	{
		m_sBuf[0] = 0;
		m_eMode = pBase->m_eMode;
		m_uMaxLen = pBase->m_uMaxLen;
		if ( pBase->m_pFather )
			m_pFather = pBase->m_pFather;
		else
		{
			pBase->AddRef ();
			m_pFather = pBase;
		}
	}

	ISphTokenizer * Clone ( ESphTokenizerClone eMode ) const final
	{
		return new CSphBigramTokenizer ( TokenizerRefPtr_c ( m_pTokenizer->Clone ( eMode ) ), this );
	}


	bool TokenIsBlended() const final
	{
		if ( m_eState==BIGRAM_PAIR )
			return true;
		if ( m_eState==BIGRAM_FIRST )
			return false;
		return m_pTokenizer->TokenIsBlended();
	}

	bool IsFreq ( int iLen, BYTE * sWord ) const
	{
		// early check
		if ( iLen>m_uMaxLen )
			return false;

		if ( m_pFather )
			return m_pFather->IsFreq ( iLen, sWord );

		// hash lookup, then linear scan
		int iPos = m_dWordsHash [ *sWord ];
		if ( !iPos )
			return false;
		while ( m_dWords[iPos] )
		{
			if ( m_dWords[iPos]==iLen && !memcmp ( sWord, &m_dWords[iPos+1], iLen ) )
				break;
			iPos += 1+m_dWords[iPos];
		}
		return m_dWords[iPos]!=0;
	}

	BYTE * GetToken() final
	{
		if ( m_eState==BIGRAM_FIRST || m_eState==BIGRAM_CLEAN )
		{
			BYTE * pFirst;
			if ( m_eState==BIGRAM_FIRST )
			{
				// first out, clean slate again, actually
				// and second will now become our next first
				assert ( m_pSecond );
				m_eState = BIGRAM_CLEAN;
				pFirst = m_pSecond;
				m_pSecond = nullptr;
			} else
			{
				// just clean slate
				// assure we're, well, clean
				assert ( !m_pSecond );
				pFirst = CSphTokenFilter::GetToken();
			}

			// clean slate
			// get first non-blended token
			if ( !pFirst )
				return nullptr;

			// pass through blended
			// could handle them as first too, but.. cumbersome
			if ( CSphTokenFilter::TokenIsBlended() )
				return pFirst;

			// check pair
			// in first_freq and both_freq modes, 1st token must be listed
			m_iFirst = (int) strlen ( (const char*)pFirst );
			if ( m_eMode!=SPH_BIGRAM_ALL && !IsFreq ( m_iFirst, pFirst ) )
				return pFirst;

			// copy it
			// subsequent calls can and will override token accumulator
			memcpy ( m_sBuf, pFirst, m_iFirst+1 );

			// grow a pair!
			// get a second one (lookahead, in a sense)
			BYTE * pSecond = CSphTokenFilter::GetToken();

			// eof? oi
			if ( !pSecond )
				return m_sBuf;

			// got a pair!
			// check combined length
			m_pSecond = pSecond;
			auto iSecond = (int) strlen ( (const char*)pSecond );
			if ( m_iFirst+iSecond+1 > SPH_MAX_WORD_LEN )
			{
				// too long pair
				// return first token as is
				m_eState = BIGRAM_FIRST;
				return m_sBuf;
			}

			// check pair
			// in freq2 mode, both tokens must be listed
			if ( m_eMode==SPH_BIGRAM_BOTHFREQ && !IsFreq ( iSecond, m_pSecond ) )
			{
				m_eState = BIGRAM_FIRST;
				return m_sBuf;
			}

			// ok, this is a eligible pair
			// begin with returning first+second pair (as blended)
			m_eState = BIGRAM_PAIR;
			m_sBuf [ m_iFirst ] = MAGIC_WORD_BIGRAM;
			assert ( m_iFirst + strlen ( (const char*)pSecond ) < sizeof(m_sBuf) );
			strcpy ( (char*)m_sBuf+m_iFirst+1, (const char*)pSecond ); //NOLINT
			return m_sBuf;

		} else if ( m_eState==BIGRAM_PAIR )
		{
			// pair (aka bigram) out, return first token as a regular token
			m_eState = BIGRAM_FIRST;
			m_sBuf [ m_iFirst ] = 0;
			return m_sBuf;
		}

		assert ( 0 && "unhandled bigram tokenizer internal state" );
		return nullptr;
	}

	uint64_t GetSettingsFNV () const final
	{
		if ( m_pFather )
			return m_pFather->GetSettingsFNV ();
		uint64_t uHash = CSphTokenFilter::GetSettingsFNV();
		uHash = sphFNV64 ( m_dWords.Begin(), m_dWords.GetLength(), uHash );
		return uHash;
	}
};

/////////////////////////////////////////////////////////////////////////////

void FillStoredTokenInfo ( StoredToken_t & tToken, const BYTE * sToken, ISphTokenizer * pTokenizer )
{
	assert ( sToken && pTokenizer );
	strncpy ( (char *)tToken.m_sToken, (const char *)sToken, sizeof(tToken.m_sToken)-1 );
	tToken.m_szTokenStart = pTokenizer->GetTokenStart ();
	tToken.m_szTokenEnd = pTokenizer->GetTokenEnd ();
	tToken.m_iOvershortCount = pTokenizer->GetOvershortCount ();
	tToken.m_iTokenLen = pTokenizer->GetLastTokenLen ();
	tToken.m_pBufferPtr = pTokenizer->GetBufferPtr ();
	tToken.m_pBufferEnd = pTokenizer->GetBufferEnd();
	tToken.m_bBoundary = pTokenizer->GetBoundary ();
	tToken.m_bSpecial = pTokenizer->WasTokenSpecial ();
	tToken.m_bBlended = pTokenizer->TokenIsBlended();
	tToken.m_bBlendedPart = pTokenizer->TokenIsBlendedPart();
}


//////////////////////////////////////////////////////////////////////////

ISphTokenizer * sphCreateUTF8Tokenizer ()
{
	return new CSphTokenizer_UTF8<false> ();
}

ISphTokenizer * sphCreateUTF8NgramTokenizer ()
{
	return new CSphTokenizer_UTF8Ngram<false> ();
}

/////////////////////////////////////////////////////////////////////////////

enum : DWORD
{
	MASK_CODEPOINT			= 0x00ffffffUL,	// mask off codepoint flags
	MASK_FLAGS				= 0xff000000UL, // mask off codepoint value
	FLAG_CODEPOINT_SPECIAL	= 0x01000000UL,	// this codepoint is special
	FLAG_CODEPOINT_DUAL		= 0x02000000UL,	// this codepoint is special but also a valid word part
	FLAG_CODEPOINT_NGRAM	= 0x04000000UL,	// this codepoint is n-gram indexed
	FLAG_CODEPOINT_BOUNDARY	= 0x10000000UL,	// this codepoint is phrase boundary
	FLAG_CODEPOINT_IGNORE	= 0x20000000UL,	// this codepoint is ignored
	FLAG_CODEPOINT_BLEND	= 0x40000000UL	// this codepoint is "blended" (indexed both as a character, and as a separator)
};

void CSphLowercaser::Reset()
{
	SafeDeleteArray ( m_pData );
	m_pData = new int [ CHUNK_SIZE ];
	memset ( m_pData, 0, CHUNK_SIZE*sizeof(int) ); // NOLINT sizeof(int)
	m_iChunks = 1;
	m_pChunk[0] = m_pData; // chunk 0 must always be allocated, for utf-8 tokenizer shortcut to work
	for ( int i=1; i<CHUNK_COUNT; i++ )
		m_pChunk[i] = NULL;
}


CSphLowercaser::~CSphLowercaser ()
{
	SafeDeleteArray ( m_pData );
}


void CSphLowercaser::SetRemap ( const CSphLowercaser * pLC )
{
	if ( !pLC )
		return;

	SafeDeleteArray ( m_pData );

	m_iChunks = pLC->m_iChunks;
	m_pData = new int [ m_iChunks*CHUNK_SIZE ];
	memcpy ( m_pData, pLC->m_pData, sizeof(int)*m_iChunks*CHUNK_SIZE ); // NOLINT sizeof(int)

	for ( int i=0; i<CHUNK_COUNT; i++ )
		m_pChunk[i] = pLC->m_pChunk[i]
			? pLC->m_pChunk[i] - pLC->m_pData + m_pData
			: NULL;
}


void CSphLowercaser::AddRemaps ( const CSphVector<CSphRemapRange> & dRemaps, DWORD uFlags )
{
	if ( !dRemaps.GetLength() )
		return;

	// build new chunks map
	// 0 means "was unused"
	// 1 means "was used"
	// 2 means "is used now"
	int dUsed [ CHUNK_COUNT ];
	for ( int i=0; i<CHUNK_COUNT; i++ )
		dUsed[i] = m_pChunk[i] ? 1 : 0;

	int iNewChunks = m_iChunks;

	ARRAY_FOREACH ( i, dRemaps )
	{
		const CSphRemapRange & tRemap = dRemaps[i];

#define LOC_CHECK_RANGE(_a) assert ( (_a)>=0 && (_a)<MAX_CODE );
		LOC_CHECK_RANGE ( tRemap.m_iStart );
		LOC_CHECK_RANGE ( tRemap.m_iEnd );
		LOC_CHECK_RANGE ( tRemap.m_iRemapStart );
		LOC_CHECK_RANGE ( tRemap.m_iRemapStart + tRemap.m_iEnd - tRemap.m_iStart );
#undef LOC_CHECK_RANGE

		for ( int iChunk=( tRemap.m_iStart >> CHUNK_BITS ); iChunk<=( tRemap.m_iEnd >> CHUNK_BITS ); iChunk++ )
			if ( dUsed[iChunk]==0 )
			{
				dUsed[iChunk] = 2;
				iNewChunks++;
			}
	}

	// alloc new tables and copy, if necessary
	if ( iNewChunks>m_iChunks )
	{
		int * pData = new int [ iNewChunks*CHUNK_SIZE ];
		memset ( pData, 0, sizeof(int)*iNewChunks*CHUNK_SIZE ); // NOLINT sizeof(int)

		int * pChunk = pData;
		for ( int i=0; i<CHUNK_COUNT; i++ )
		{
			int * pOldChunk = m_pChunk[i];

			// build new ptr
			if ( dUsed[i] )
			{
				m_pChunk[i] = pChunk;
				pChunk += CHUNK_SIZE;
			}

			// copy old data
			if ( dUsed[i]==1 )
				memcpy ( m_pChunk[i], pOldChunk, sizeof(int)*CHUNK_SIZE ); // NOLINT sizeof(int)
		}
		assert ( pChunk-pData==iNewChunks*CHUNK_SIZE );

		SafeDeleteArray ( m_pData );
		m_pData = pData;
		m_iChunks = iNewChunks;
	}

	// fill new stuff
	ARRAY_FOREACH ( i, dRemaps )
	{
		const CSphRemapRange & tRemap = dRemaps[i];

		DWORD iRemapped = tRemap.m_iRemapStart;
		for ( int j=tRemap.m_iStart; j<=tRemap.m_iEnd; j++, iRemapped++ )
		{
			assert ( m_pChunk [ j >> CHUNK_BITS ] );
			int & iCodepoint = m_pChunk [ j >> CHUNK_BITS ] [ j & CHUNK_MASK ];
			bool bWordPart = ( iCodepoint & MASK_CODEPOINT )!=0;
			int iNew = iRemapped | uFlags | ( iCodepoint & MASK_FLAGS );
			if ( bWordPart && ( uFlags & FLAG_CODEPOINT_SPECIAL ) )
				iNew |= FLAG_CODEPOINT_DUAL;
			iCodepoint = iNew;
		}
	}
}


void CSphLowercaser::AddSpecials ( const char * sSpecials )
{
	assert ( sSpecials );
	auto iSpecials = (int) strlen(sSpecials);

	CSphVector<CSphRemapRange> dRemaps;
	dRemaps.Resize ( iSpecials );
	ARRAY_FOREACH ( i, dRemaps )
		dRemaps[i].m_iStart = dRemaps[i].m_iEnd = dRemaps[i].m_iRemapStart = sSpecials[i];

	AddRemaps ( dRemaps, FLAG_CODEPOINT_SPECIAL );
}

CSphLowercaser & CSphLowercaser::operator = ( const CSphLowercaser & rhs )
{
	SetRemap ( &rhs );
	return * this;
}

uint64_t CSphLowercaser::GetFNV () const
{
	int iLen = ( sizeof(int) * m_iChunks * CHUNK_SIZE ) / sizeof(BYTE); // NOLINT
	return sphFNV64 ( m_pData, iLen );
}

int CSphLowercaser::GetMaxCodepointLength () const
{
	int iMax = 0;
	for ( int iChunk=0; iChunk<CHUNK_COUNT; iChunk++ )
	{
		int * pChunk = m_pChunk[iChunk];
		if ( !pChunk )
			continue;

		int * pMax = pChunk + CHUNK_SIZE;
		while ( pChunk<pMax )
		{
			int iCode = *pChunk++ & MASK_CODEPOINT;
			iMax = Max ( iMax, iCode );
		}
	}
	if ( iMax<0x80 )
		return 1;
	if ( iMax<0x800 )
		return 2;
	return 3; // actually, 4 once we hit 0x10000
}

/////////////////////////////////////////////////////////////////////////////

const char * CSphCharsetDefinitionParser::GetLastError ()
{
	return m_bError ? m_sError : NULL;
}


bool CSphCharsetDefinitionParser::IsEof ()
{
	return ( *m_pCurrent )==0;
}


bool CSphCharsetDefinitionParser::CheckEof ()
{
	if ( IsEof() )
	{
		Error ( "unexpected end of line" );
		return true;
	} else
	{
		return false;
	}
}


bool CSphCharsetDefinitionParser::Error ( const char * sMessage )
{
	char sErrorBuffer[32];
	strncpy ( sErrorBuffer, m_pCurrent, sizeof(sErrorBuffer) );
	sErrorBuffer [ sizeof(sErrorBuffer)-1 ] = '\0';

	snprintf ( m_sError, sizeof(m_sError), "%s near '%s'",
		sMessage, sErrorBuffer );
	m_sError [ sizeof(m_sError)-1 ] = '\0';

	m_bError = true;
	return false;
}


int CSphCharsetDefinitionParser::HexDigit ( int c )
{
	if ( c>='0' && c<='9' ) return c-'0';
	if ( c>='a' && c<='f' ) return c-'a'+10;
	if ( c>='A' && c<='F' ) return c-'A'+10;
	return 0;
}


void CSphCharsetDefinitionParser::SkipSpaces ()
{
	while ( ( *m_pCurrent ) && isspace ( (BYTE)*m_pCurrent ) )
		m_pCurrent++;
}


int CSphCharsetDefinitionParser::ParseCharsetCode ()
{
	const char * p = m_pCurrent;
	int iCode = 0;

	if ( p[0]=='U' && p[1]=='+' )
	{
		p += 2;
		while ( isxdigit(*p) )
		{
			iCode = iCode*16 + HexDigit ( *p++ );
		}
		while ( isspace(*p) )
			p++;

	} else
	{
		if ( (*(const BYTE*)p)<32 || (*(const BYTE*)p)>127 )
		{
			Error ( "non-ASCII characters not allowed, use 'U+00AB' syntax" );
			return -1;
		}

		iCode = *p++;
		while ( isspace(*p) )
			p++;
	}

	m_pCurrent = p;
	return iCode;
}

bool CSphCharsetDefinitionParser::AddRange ( const CSphRemapRange & tRange, CSphVector<CSphRemapRange> & dRanges )
{
	if ( tRange.m_iRemapStart>=0x20 )
	{
		dRanges.Add ( tRange );
		return true;
	}

	CSphString sError;
	sError.SetSprintf ( "dest range (U+%x) below U+20, not allowed", tRange.m_iRemapStart );
	Error ( sError.cstr() );
	return false;
}


// Charsets relocated to folder 'charsets', each one in separate .txt file.
// When you change the content of the folder,
// reconfigure the project with cmake in order to pick the changes.

#include "globalaliases.h"

static CSphVector<CharsetAlias_t> g_dCharsetAliases;

const CSphVector<CharsetAlias_t> & GetCharsetAliases()
{
	return g_dCharsetAliases;
}


bool sphInitCharsetAliasTable ( CSphString & sError ) // FIXME!!! move alias generation to config common section
{
	g_dCharsetAliases.Reset();
	CSphCharsetDefinitionParser tParser;
	CSphVector<CharsetAlias_t> dAliases;
	CSphVector<char> dConcat;

	const int iTotalChunks = sizeof(globalaliases)/sizeof(globalaliases[0]);
	int iCurAliasChunk = 0;
	for ( int i=0; i<CHARSET_TOTAL; ++i )
	{
		CharsetAlias_t & tCur = dAliases.Add();
		tCur.m_sName = globalaliases_names[i];
		tCur.m_iNameLen = tCur.m_sName.Length();

		dConcat.Resize(0);
		while ( iCurAliasChunk<iTotalChunks && globalaliases[iCurAliasChunk] )
		{
			auto iChunkLen = (int) strlen(globalaliases[iCurAliasChunk]);
			char * szChunk = dConcat.AddN(iChunkLen);
			memcpy ( szChunk, globalaliases[iCurAliasChunk], iChunkLen );
			iCurAliasChunk++;
		}

		dConcat.Add(0);
		iCurAliasChunk++;

		if ( !tParser.Parse ( dConcat.Begin(), tCur.m_dRemaps ) )
		{
			sError = tParser.GetLastError();
			return false;
		}
	}

	g_dCharsetAliases.SwapData ( dAliases );
	return true;
}


bool CSphCharsetDefinitionParser::Parse ( const char * sConfig, CSphVector<CSphRemapRange> & dRanges )
{
	m_pCurrent = sConfig;
	dRanges.Reset ();

	// do parse
	while ( *m_pCurrent )
	{
		SkipSpaces ();
		if ( IsEof () )
			break;

		// check for stray comma
		if ( *m_pCurrent==',' )
			return Error ( "stray ',' not allowed, use 'U+002C' instead" );

		// alias
		bool bGotAlias = false;
		ARRAY_FOREACH_COND ( i, g_dCharsetAliases, !bGotAlias )
		{
			const CharsetAlias_t & tCur = g_dCharsetAliases[i];
			bGotAlias = ( strncmp ( tCur.m_sName.cstr(), m_pCurrent, tCur.m_iNameLen )==0 && ( !m_pCurrent[tCur.m_iNameLen] || m_pCurrent[tCur.m_iNameLen]==',' ) );
			if ( !bGotAlias )
				continue;

			// skip to next definition
			m_pCurrent += tCur.m_iNameLen;
			if ( *m_pCurrent && *m_pCurrent==',' )
				m_pCurrent++;

			ARRAY_FOREACH ( iDef, tCur.m_dRemaps )
			{
				if ( !AddRange ( tCur.m_dRemaps[iDef], dRanges ) )
					return false;
			}
		}
		if ( bGotAlias )
			continue;

		// parse char code
		const char * pStart = m_pCurrent;
		int iStart = ParseCharsetCode();
		if ( iStart<0 )
			return false;

		// stray char?
		if ( !*m_pCurrent || *m_pCurrent==',' )
		{
			// stray char
			if ( !AddRange ( CSphRemapRange ( iStart, iStart, iStart ), dRanges ) )
				return false;

			if ( IsEof () )
				break;
			m_pCurrent++;
			continue;
		}

		// stray remap?
		if ( m_pCurrent[0]=='-' && m_pCurrent[1]=='>' )
		{
			// parse and add
			m_pCurrent += 2;
			int iDest = ParseCharsetCode ();
			if ( iDest<0 )
				return false;
			if ( !AddRange ( CSphRemapRange ( iStart, iStart, iDest ), dRanges ) )
				return false;

			// it's either end of line now, or must be followed by comma
			if ( *m_pCurrent )
				if ( *m_pCurrent++!=',' )
					return Error ( "syntax error" );
			continue;
		}

		// range start?
		if (!( m_pCurrent[0]=='.' && m_pCurrent[1]=='.' ))
			return Error ( "syntax error" );
		m_pCurrent += 2;

		SkipSpaces ();
		if ( CheckEof () )
			return false;

		// parse range end char code
		int iEnd = ParseCharsetCode ();
		if ( iEnd<0 )
			return false;
		if ( iStart>iEnd )
		{
			m_pCurrent = pStart;
			return Error ( "range end less than range start" );
		}

		// stray range?
		if ( !*m_pCurrent || *m_pCurrent==',' )
		{
			if ( !AddRange ( CSphRemapRange ( iStart, iEnd, iStart ), dRanges ) )
				return false;

			if ( IsEof () )
				break;
			m_pCurrent++;
			continue;
		}

		// "checkerboard" range?
		if ( m_pCurrent[0]=='/' && m_pCurrent[1]=='2' )
		{
			for ( int i=iStart; i<iEnd; i+=2 )
			{
				if ( !AddRange ( CSphRemapRange ( i, i, i+1 ), dRanges ) )
					return false;
				if ( !AddRange ( CSphRemapRange ( i+1, i+1, i+1 ), dRanges ) )
					return false;
			}

			// skip "/2", expect ","
			m_pCurrent += 2;
			SkipSpaces ();
			if ( *m_pCurrent )
				if ( *m_pCurrent++!=',' )
					return Error ( "expected end of line or ','" );
			continue;
		}

		// remapped range?
		if (!( m_pCurrent[0]=='-' && m_pCurrent[1]=='>' ))
			return Error ( "expected end of line, ',' or '-><char>'" );
		m_pCurrent += 2;

		SkipSpaces ();
		if ( CheckEof () )
			return false;

		// parse dest start
		const char * pRemapStart = m_pCurrent;
		int iRemapStart = ParseCharsetCode ();
		if ( iRemapStart<0 )
			return false;

		// expect '..'
		if ( CheckEof () )
			return false;
		if (!( m_pCurrent[0]=='.' && m_pCurrent[1]=='.' ))
			return Error ( "expected '..'" );
		m_pCurrent += 2;

		// parse dest end
		int iRemapEnd = ParseCharsetCode ();
		if ( iRemapEnd<0 )
			return false;

		// check dest range
		if ( iRemapStart>iRemapEnd )
		{
			m_pCurrent = pRemapStart;
			return Error ( "dest range end less than dest range start" );
		}

		// check for length mismatch
		if ( ( iRemapEnd-iRemapStart )!=( iEnd-iStart ) )
		{
			m_pCurrent = pStart;
			return Error ( "dest range length must match src range length" );
		}

		// remapped ok
		if ( !AddRange ( CSphRemapRange ( iStart, iEnd, iRemapStart ), dRanges ) )
			return false;

		if ( IsEof () )
			break;
		if ( *m_pCurrent!=',' )
			return Error ( "expected ','" );
		m_pCurrent++;
	}

	dRanges.Sort ();
	for ( int i=0; i<dRanges.GetLength()-1; i++ )
	{
		if ( dRanges[i].m_iEnd>=dRanges[i+1].m_iStart )
		{
			// FIXME! add an ambiguity check
			dRanges[i].m_iEnd = Max ( dRanges[i].m_iEnd, dRanges[i+1].m_iEnd );
			dRanges.Remove ( i+1 );
			i--;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////

bool sphParseCharset ( const char * sCharset, CSphVector<CSphRemapRange> & dRemaps )
{
	CSphCharsetDefinitionParser tParser;
	return tParser.Parse ( sCharset, dRemaps );
}

/////////////////////////////////////////////////////////////////////////////
void CSphEmbeddedFiles::Reset()
{
	m_dSynonyms.Reset();
	m_dStopwordFiles.Reset();
	m_dStopwords.Reset();
	m_dWordforms.Reset();
	m_dWordformFiles.Reset();
}


static inline bool ShortTokenFilter ( BYTE * pToken, int iLen )
{
	return pToken[0]=='*' || ( iLen > 0 && pToken[iLen-1]=='*' );
}

/////////////////////////////////////////////////////////////////////////////

bool ISphTokenizer::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	const int MIN_CODE = 0x21;
	ARRAY_FOREACH ( i, dRemaps )
	{
		CSphRemapRange & tMap = dRemaps[i];

		if ( tMap.m_iStart<MIN_CODE || tMap.m_iStart>=m_tLC.MAX_CODE )
		{
			sphWarning ( "wrong character mapping start specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED",
				tMap.m_iStart, MIN_CODE, m_tLC.MAX_CODE-1 );
			tMap.m_iStart = Min ( Max ( tMap.m_iStart, MIN_CODE ), m_tLC.MAX_CODE-1 );
		}

		if ( tMap.m_iEnd<MIN_CODE || tMap.m_iEnd>=m_tLC.MAX_CODE )
		{
			sphWarning ( "wrong character mapping end specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED",
				tMap.m_iEnd, MIN_CODE, m_tLC.MAX_CODE-1 );
			tMap.m_iEnd = Min ( Max ( tMap.m_iEnd, MIN_CODE ), m_tLC.MAX_CODE-1 );
		}

		if ( tMap.m_iRemapStart<MIN_CODE || tMap.m_iRemapStart>=m_tLC.MAX_CODE )
		{
			sphWarning ( "wrong character remapping start specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED",
				tMap.m_iRemapStart, MIN_CODE, m_tLC.MAX_CODE-1 );
			tMap.m_iRemapStart = Min ( Max ( tMap.m_iRemapStart, MIN_CODE ), m_tLC.MAX_CODE-1 );
		}

		int iRemapEnd = tMap.m_iRemapStart+tMap.m_iEnd-tMap.m_iStart;
		if ( iRemapEnd<MIN_CODE || iRemapEnd>=m_tLC.MAX_CODE )
		{
			sphWarning ( "wrong character remapping end specified: U+%x, should be between U+%x and U+%x (inclusive); IGNORED",
				iRemapEnd, MIN_CODE, m_tLC.MAX_CODE-1 );
			dRemaps.Remove(i);
			i--;
		}
	}

	m_tLC.Reset ();
	m_tLC.AddRemaps ( dRemaps, 0 );
	return true;
}


void ISphTokenizer::AddPlainChar ( char c )
{
	CSphVector<CSphRemapRange> dTmp ( 1 );
	dTmp[0].m_iStart = dTmp[0].m_iEnd = dTmp[0].m_iRemapStart = c;
	m_tLC.AddRemaps ( dTmp, 0 );
}


void ISphTokenizer::AddSpecials ( const char * sSpecials )
{
	m_tLC.AddSpecials ( sSpecials );
}


void ISphTokenizer::Setup ( const CSphTokenizerSettings & tSettings )
{
	m_tSettings = tSettings;
}


ISphTokenizer * ISphTokenizer::Create ( const CSphTokenizerSettings & tSettings, const CSphEmbeddedFiles * pFiles, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings, CSphString & sError )
{
	TokenizerRefPtr_c pTokenizer;

	switch ( tSettings.m_iType )
	{
	case TOKENIZER_UTF8:	pTokenizer = sphCreateUTF8Tokenizer (); break;
	case TOKENIZER_NGRAM:	pTokenizer = sphCreateUTF8NgramTokenizer (); break;
	default:
		sError.SetSprintf ( "failed to create tokenizer (unknown charset type '%d')", tSettings.m_iType );
		return nullptr;
	}

	pTokenizer->Setup ( tSettings );

	if ( !tSettings.m_sCaseFolding.IsEmpty () && !pTokenizer->SetCaseFolding ( tSettings.m_sCaseFolding.cstr (), sError ) )
	{
		sError.SetSprintf ( "'charset_table': %s", sError.cstr() );
		return nullptr;
	}

	CSphString sSynonymsFile = tSettings.m_sSynonymsFile;
	if ( !sSynonymsFile.IsEmpty() )
	{
		if ( pFilenameBuilder )
			sSynonymsFile = pFilenameBuilder->GetFullPath(sSynonymsFile);

		if ( !pTokenizer->LoadSynonyms ( sSynonymsFile.cstr(), pFiles && pFiles->m_bEmbeddedSynonyms ? pFiles : nullptr, dWarnings, sError ) )
		{
			sError.SetSprintf ( "'synonyms': %s", sError.cstr() );
			return nullptr;
		}
	}

	if ( !tSettings.m_sBoundary.IsEmpty () && !pTokenizer->SetBoundary ( tSettings.m_sBoundary.cstr (), sError ) )
	{
		sError.SetSprintf ( "'phrase_boundary': %s", sError.cstr() );
		return nullptr;
	}

	if ( !tSettings.m_sIgnoreChars.IsEmpty () && !pTokenizer->SetIgnoreChars ( tSettings.m_sIgnoreChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'ignore_chars': %s", sError.cstr() );
		return nullptr;
	}

	if ( !tSettings.m_sBlendChars.IsEmpty () && !pTokenizer->SetBlendChars ( tSettings.m_sBlendChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'blend_chars': %s", sError.cstr() );
		return nullptr;
	}

	if ( !pTokenizer->SetBlendMode ( tSettings.m_sBlendMode.cstr (), sError ) )
	{
		sError.SetSprintf ( "'blend_mode': %s", sError.cstr() );
		return nullptr;
	}

	pTokenizer->SetNgramLen ( tSettings.m_iNgramLen );

	if ( !tSettings.m_sNgramChars.IsEmpty () && !pTokenizer->SetNgramChars ( tSettings.m_sNgramChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'ngram_chars': %s", sError.cstr() );
		return nullptr;
	}

	return pTokenizer.Leak ();
}


ISphTokenizer * ISphTokenizer::CreateMultiformFilter ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer )
{
	if ( !pContainer )
	{
		SafeAddRef ( pTokenizer );
		return pTokenizer;
	}
	return new CSphMultiformTokenizer ( pTokenizer, pContainer );
}


ISphTokenizer * ISphTokenizer::CreateBigramFilter ( ISphTokenizer * pTokenizer, ESphBigram eBigramIndex, const CSphString & sBigramWords, CSphString & sError )
{
	assert ( pTokenizer );

	if ( eBigramIndex==SPH_BIGRAM_NONE )
	{
		SafeAddRef ( pTokenizer );
		return pTokenizer;
	}

	StrVec_t dFreq;
	if ( eBigramIndex!=SPH_BIGRAM_ALL )
	{
		const BYTE * pTok = NULL;
		pTokenizer->SetBuffer ( (const BYTE*)sBigramWords.cstr(), sBigramWords.Length() );
		while ( ( pTok = pTokenizer->GetToken() )!=NULL )
			dFreq.Add ( (const char*)pTok );

		if ( !dFreq.GetLength() )
		{
			SafeDelete ( pTokenizer );
			sError.SetSprintf ( "bigram_freq_words does not contain any valid words" );
			return NULL;
		}
	}

	return new CSphBigramTokenizer ( pTokenizer, eBigramIndex, dFreq );
}


class PluginFilterTokenizer_c final : public CSphTokenFilter
{
protected:
	const PluginTokenFilter_c *	m_pFilter;		///< plugin descriptor
	CSphString					m_sOptions;		///< options string for the plugin init()
	void *						m_pUserdata = nullptr;	///< userdata returned from by the plugin init()
	bool						m_bGotExtra = false;	///< are we looping through extra tokens?
	int							m_iPosDelta = 0;		///< position delta for the current token, see comments in GetToken()
	bool						m_bWasBlended = false;	///< whether the last raw token was blended

	~PluginFilterTokenizer_c () final
	{
		if ( m_pFilter->m_fnDeinit )
			m_pFilter->m_fnDeinit ( m_pUserdata );
		m_pFilter->Release ();
	}

public:
	PluginFilterTokenizer_c ( ISphTokenizer * pTok, const PluginTokenFilter_c * pFilter, const char * sOptions )
		: CSphTokenFilter ( pTok )
		, m_pFilter ( pFilter )
		, m_sOptions ( sOptions )
	{
		assert ( pTok );
		assert ( m_pFilter );
		m_pFilter->AddRef();
		// FIXME!!! handle error in constructor \ move to setup?
		CSphString sError;
		SetFilterSchema ( CSphSchema(), sError );
	}

	ISphTokenizer * Clone ( ESphTokenizerClone eMode ) const final
	{
		TokenizerRefPtr_c pTok { m_pTokenizer->Clone ( eMode ) };
		return new PluginFilterTokenizer_c ( pTok, m_pFilter, m_sOptions.cstr() );
	}

	bool SetFilterSchema ( const CSphSchema & s, CSphString & sError ) final
	{
		if ( m_pUserdata && m_pFilter->m_fnDeinit )
			m_pFilter->m_fnDeinit ( m_pUserdata );

		CSphVector<const char*> dFields;
		for ( int i = 0; i < s.GetFieldsCount(); i++ )
			dFields.Add ( s.GetFieldName(i) );

		char sErrBuf[SPH_UDF_ERROR_LEN+1];
		if ( m_pFilter->m_fnInit ( &m_pUserdata, dFields.GetLength(), dFields.Begin(), m_sOptions.cstr(), sErrBuf )==0 )
			return true;
		sError = sErrBuf;
		return false;
	}

	bool SetFilterOptions ( const char * sOptions, CSphString & sError ) final
	{
		char sErrBuf[SPH_UDF_ERROR_LEN+1];
		if ( m_pFilter->m_fnBeginDocument ( m_pUserdata, sOptions, sErrBuf )==0 )
			return true;
		sError = sErrBuf;
		return false;
	}

	void BeginField ( int iField ) final
	{
		if ( m_pFilter->m_fnBeginField )
			m_pFilter->m_fnBeginField ( m_pUserdata, iField );
	}

	BYTE * GetToken () final
	{
		// we have two principal states here
		// a) have pending extra tokens, keep looping and returning those
		// b) no extras, keep pushing until plugin returns anything
		//
		// we also have to handle position deltas, and that story is a little tricky
		// positions are not assigned in the tokenizer itself (we might wanna refactor that)
		// however, tokenizer has some (partial) control over the keyword positions, too
		// when it skips some too-short tokens, it returns a non-zero value via GetOvershortCount()
		// when it returns a blended token, it returns true via TokenIsBlended()
		// so while the default position delta is 1, overshorts can increase it by N,
		// and blended flag can decrease it by 1, and that's under tokenizer's control
		//
		// so for the plugins, we simplify (well i hope!) this complexity a little
		// we compute a proper position delta here, pass it, and let the plugin modify it
		// we report all tokens as regular, and return the delta via GetOvershortCount()

		// state (a), just loop the pending extras
		if ( m_bGotExtra )
		{
			m_iPosDelta = 1; // default delta is 1
			BYTE * pTok = (BYTE*) m_pFilter->m_fnGetExtraToken ( m_pUserdata, &m_iPosDelta );
			GetBlended();
			if ( pTok )
				return pTok;
			m_bGotExtra = false;
		}

		// state (b), push raw tokens, return results
		for ( ;; )
		{
			// get next raw token, handle field end
			BYTE * pRaw = CSphTokenFilter::GetToken();
			if ( !pRaw )
			{
				// no more hits? notify plugin of a field end,
				// and check if there are pending tokens
				m_bGotExtra = 0;
				if ( m_pFilter->m_fnEndField )
					if ( !m_pFilter->m_fnEndField ( m_pUserdata ) )
					{
						m_bBlended = false;
						m_bBlendedPart = false;
						return NULL;
					}

				// got them, start fetching
				m_bGotExtra = true;
				BYTE * pTok = (BYTE*)m_pFilter->m_fnGetExtraToken ( m_pUserdata, &m_iPosDelta );
				GetBlended();
				return pTok;
			}

			// compute proper position delta
			m_iPosDelta = ( m_bWasBlended ? 0 : 1 ) + CSphTokenFilter::GetOvershortCount();
			m_bWasBlended = CSphTokenFilter::TokenIsBlended();

			// push raw token to plugin, return a processed one, if any
			int iExtra = 0;
			BYTE * pTok = (BYTE*)m_pFilter->m_fnPushToken ( m_pUserdata, (char*)pRaw, &iExtra, &m_iPosDelta );
			m_bGotExtra = ( iExtra!=0 );
			GetBlended();
			if ( pTok )
				return pTok;
		}
	}

	int GetOvershortCount() final
	{
		return m_iPosDelta-1;
	}

private:
	void GetBlended()
	{
		if ( m_pFilter->m_fnTokenIsBlended )
			m_bBlended = ( !!m_pFilter->m_fnTokenIsBlended ( m_pUserdata ) );
		if ( m_pFilter->m_fnTokenIsBlendedPart )
			m_bBlendedPart = ( !!m_pFilter->m_fnTokenIsBlendedPart ( m_pUserdata ) );
	}
};


ISphTokenizer * ISphTokenizer::CreatePluginFilter ( ISphTokenizer * pTokenizer, const CSphString & sSpec, CSphString & sError )
{
	StrVec_t dPlugin; // dll, filtername, options
	if ( !sphPluginParseSpec ( sSpec, dPlugin, sError ) )
		return nullptr;

	if ( !dPlugin.GetLength() )
	{
		SafeAddRef ( pTokenizer );
		return pTokenizer;
	}

	CSphRefcountedPtr<PluginTokenFilter_c> p { ( PluginTokenFilter_c *) sphPluginAcquire ( dPlugin[0].cstr(), PLUGIN_INDEX_TOKEN_FILTER, dPlugin[1].cstr(), sError ) };
	if ( !p )
	{
		sError.SetSprintf ( "INTERNAL ERROR: plugin %s:%s loaded ok but lookup fails, error: %s", dPlugin[0].cstr(), dPlugin[1].cstr(), sError.cstr() );
		return nullptr;
	}
	return new PluginFilterTokenizer_c ( pTokenizer, p, dPlugin[2].cstr () );
}


bool ISphTokenizer::AddSpecialsSPZ ( const char * sSpecials, const char * sDirective, CSphString & sError )
{
	for ( int i=0; sSpecials[i]; i++ )
	{
		int iCode = m_tLC.ToLower ( sSpecials[i] );
		if ( iCode & ( FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_BOUNDARY | FLAG_CODEPOINT_IGNORE ) )
		{
			sError.SetSprintf ( "%s requires that character '%c' is not in ngram_chars, phrase_boundary, or ignore_chars",
				sDirective, sSpecials[i] );
			return false;
		}
	}

	AddSpecials ( sSpecials );
	return true;
}


bool ISphTokenizer::EnableSentenceIndexing ( CSphString & sError )
{
	const char sSpecials[] = { '.', '?', '!', MAGIC_CODE_PARAGRAPH, 0 };

	if ( !AddSpecialsSPZ ( sSpecials, "index_sp", sError ) )
		return false;

	m_bDetectSentences = true;
	return true;
}


bool ISphTokenizer::EnableZoneIndexing ( CSphString & sError )
{
	static const char sSpecials[] = { MAGIC_CODE_ZONE, 0 };
	return AddSpecialsSPZ ( sSpecials, "index_zones", sError );
}

uint64_t ISphTokenizer::GetSettingsFNV () const
{
	uint64_t uHash = m_tLC.GetFNV();

	DWORD uFlags = 0;
	if ( m_bBlendSkipPure )
		uFlags |= 1<<1;
	if ( m_bShortTokenFilter )
		uFlags |= 1<<2;
	uHash = sphFNV64 ( &uFlags, sizeof(uFlags), uHash );
	uHash = sphFNV64 ( &m_uBlendVariants, sizeof(m_uBlendVariants), uHash );

	uHash = sphFNV64 ( &m_tSettings.m_iType, sizeof(m_tSettings.m_iType), uHash );
	uHash = sphFNV64 ( &m_tSettings.m_iMinWordLen, sizeof(m_tSettings.m_iMinWordLen), uHash );
	uHash = sphFNV64 ( &m_tSettings.m_iNgramLen, sizeof(m_tSettings.m_iNgramLen), uHash );

	return uHash;
}

//////////////////////////////////////////////////////////////////////////

CSphTokenizerBase::CSphTokenizerBase ()
{
	m_pAccum = m_sAccum;
}


CSphTokenizerBase::~CSphTokenizerBase()
{
	SafeDelete ( m_pExc );
}


bool CSphTokenizerBase::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
	if ( m_pExc )
	{
		sError = "SetCaseFolding() must not be called after LoadSynonyms()";
		return false;
	}
	m_bHasBlend = false;
	return ISphTokenizer::SetCaseFolding ( sConfig, sError );
}


bool CSphTokenizerBase::SetBlendChars ( const char * sConfig, CSphString & sError )
{
	assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
	m_bHasBlend = ISphTokenizer::SetBlendChars ( sConfig, sError );
	return m_bHasBlend;
}


bool CSphTokenizerBase::LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, StrVec_t & dWarnings, CSphString & sError )
{
	assert ( m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );

	CSphString sWarning;
	ExceptionsTrieGen_c g;
	if ( pFiles )
	{
		m_tSynFileInfo = pFiles->m_tSynonymFile;
		ARRAY_FOREACH ( i, pFiles->m_dSynonyms )
		{
			if ( !g.ParseLine ( const_cast<char*>(pFiles->m_dSynonyms[i].cstr()), sWarning ) )
			{
				sWarning.SetSprintf ( "%s line %d: %s", pFiles->m_tSynonymFile.m_sFilename.cstr(), i, sWarning.cstr() );
				dWarnings.Add(sWarning);
				sphWarning ( "%s", sWarning.cstr() );
			}
		}
	} else
	{
		if ( !sFilename || !*sFilename )
			return true;

		m_tSynFileInfo.Collect(sFilename);

		CSphAutoreader tReader;
		if ( !tReader.Open ( sFilename, sError ) )
			return false;

		char sBuffer[1024];
		int iLine = 0;
		while ( tReader.GetLine ( sBuffer, sizeof(sBuffer) )>=0 )
		{
			iLine++;
			if ( !g.ParseLine ( sBuffer, sWarning ) )
			{
				sWarning.SetSprintf ( "%s line %d: %s", sFilename, iLine, sWarning.cstr() );
				dWarnings.Add(sWarning);
				sphWarning ( "%s", sWarning.cstr() );
			}
		}
	}

	m_pExc = g.Build();
	return true;
}


void CSphTokenizerBase::WriteSynonyms ( CSphWriter & tWriter ) const
{
	if ( m_pExc )
		m_pExc->Export ( tWriter );
	else
		tWriter.PutDword ( 0 );
}

void CSphTokenizerBase::WriteSynonyms ( JsonEscapedBuilder & tOut ) const
{
	if ( !m_pExc )
		return;

	tOut.Named ( "synonyms" );
	auto _ = tOut.ArrayW();
	m_pExc->Export ( tOut );
}


void CSphTokenizerBase::CloneBase ( const CSphTokenizerBase * pFrom, ESphTokenizerClone eMode )
{
	m_eMode = eMode;
	m_pExc = nullptr;
	if ( pFrom->m_pExc )
	{
		m_pExc = new ExceptionsTrie_c();
		*m_pExc = *pFrom->m_pExc;
	}
	m_tSettings = pFrom->m_tSettings;
	m_bHasBlend = pFrom->m_bHasBlend;
	m_uBlendVariants = pFrom->m_uBlendVariants;
	m_bBlendSkipPure = pFrom->m_bBlendSkipPure;
	m_bShortTokenFilter = ( eMode!=SPH_CLONE_INDEX );
	m_bDetectSentences = pFrom->m_bDetectSentences;

	switch ( eMode )
	{
		case SPH_CLONE_INDEX:
			assert ( pFrom->m_eMode==SPH_CLONE_INDEX );
			m_tLC = pFrom->m_tLC;
			break;

		case SPH_CLONE_QUERY:
		{
			assert ( pFrom->m_eMode==SPH_CLONE_INDEX || pFrom->m_eMode==SPH_CLONE_QUERY );
			m_tLC = pFrom->m_tLC;

			CSphVector<CSphRemapRange> dRemaps;
			CSphRemapRange Range;
			Range.m_iStart = Range.m_iEnd = Range.m_iRemapStart = '\\';
			dRemaps.Add ( Range );
			m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_SPECIAL );

			m_uBlendVariants = BLEND_TRIM_NONE;
			break;
		}

		case SPH_CLONE_QUERY_LIGHTWEIGHT:
		{
			// FIXME? avoid double lightweight clones, too?
			assert ( pFrom->m_eMode!=SPH_CLONE_INDEX );
			assert ( pFrom->m_tLC.ToLower ( '\\' ) & FLAG_CODEPOINT_SPECIAL );

			// lightweight tokenizer clone
			// copy 3 KB of lowercaser chunk pointers, but do NOT copy the table data
			SafeDeleteArray ( m_tLC.m_pData );
			m_tLC.m_iChunks = 0;
			m_tLC.m_pData = nullptr;
			for ( int i=0; i<CSphLowercaser::CHUNK_COUNT; ++i )
				m_tLC.m_pChunk[i] = pFrom->m_tLC.m_pChunk[i];
			break;
		}
	}
}

uint64_t CSphTokenizerBase::GetSettingsFNV () const
{
	uint64_t uHash = ISphTokenizer::GetSettingsFNV();

	DWORD uFlags = 0;
	if ( m_bHasBlend )
		uFlags |= 1<<0;
	uHash = sphFNV64 ( &uFlags, sizeof(uFlags), uHash );

	return uHash;
}


void CSphTokenizerBase::SetBufferPtr ( const char * sNewPtr )
{
	assert ( (const BYTE*)sNewPtr>=m_pBuffer && (const BYTE*)sNewPtr<=m_pBufferMax );
	m_pCur = Min ( m_pBufferMax, Max ( m_pBuffer, (const BYTE*)sNewPtr ) );
	m_iAccum = 0;
	m_pAccum = m_sAccum;
	m_pTokenStart = m_pTokenEnd = nullptr;
	m_pBlendStart = m_pBlendEnd = nullptr;
}


int CSphTokenizerBase2::SkipBlended()
{
	if ( !m_pBlendEnd )
		return 0;

	const BYTE * pMax = m_pBufferMax;
	m_pBufferMax = m_pBlendEnd;

	// loop until the blended token end
	int iBlended = 0; // how many blended subtokens we have seen so far
	int iAccum = 0; // how many non-blended chars in a row we have seen so far
	while ( m_pCur < m_pBufferMax )
	{
		int iCode = GetCodepoint();
		if ( iCode=='\\' )
			iCode = GetCodepoint(); // no boundary check, GetCP does it
		iCode = m_tLC.ToLower ( iCode ); // no -1 check, ToLower does it
		if ( iCode<0 )
			iCode = 0;
		if ( iCode & FLAG_CODEPOINT_BLEND )
			iCode = 0;
		if ( iCode & MASK_CODEPOINT )
		{
			iAccum++;
			continue;
		}
		if ( iAccum>=m_tSettings.m_iMinWordLen )
			iBlended++;
		iAccum = 0;
	}
	if ( iAccum>=m_tSettings.m_iMinWordLen )
		iBlended++;

	m_pBufferMax = pMax;
	return iBlended;
}


/// adjusts blending magic when we're about to return a token (any token)
/// returns false if current token should be skipped, true otherwise
bool CSphTokenizerBase::BlendAdjust ( const BYTE * pCur )
{
	// check if all we got is a bunch of blended characters (pure-blended case)
	if ( m_bBlended && !m_bNonBlended )
	{
		// we either skip this token, or pretend it was normal
		// in both cases, clear the flag
		m_bBlended = false;

		// do we need to skip it?
		if ( m_bBlendSkipPure )
		{
			m_pBlendStart = NULL;
			return false;
		}
	}
	m_bNonBlended = false;

	// adjust buffer pointers
	if ( m_bBlended && m_pBlendStart )
	{
		// called once per blended token, on processing start
		// at this point, full blended token is in the accumulator
		// and we're about to return it
		m_pCur = m_pBlendStart;
		m_pBlendEnd = pCur;
		m_pBlendStart = nullptr;
		m_bBlendedPart = true;
	} else if ( pCur>=m_pBlendEnd )
	{
		// tricky bit, as at this point, token we're about to return
		// can either be a blended subtoken, or the next one
		m_bBlendedPart = m_pTokenStart && ( m_pTokenStart<m_pBlendEnd );
		m_pBlendEnd = nullptr;
		m_pBlendStart = nullptr;
	} else if ( !m_pBlendEnd )
	{
		// we aren't re-parsing blended; so clear the "blended subtoken" flag
		m_bBlendedPart = false;
	}
	return true;
}


static inline void CopySubstring ( BYTE * pDst, const BYTE * pSrc, int iLen )
{
	while ( iLen-->0 && *pSrc )
		*pDst++ = *pSrc++;
	*pDst++ = '\0';
}


BYTE * CSphTokenizerBase2::GetBlendedVariant ()
{
	// we can get called on several occasions
	// case 1, a new blended token was just accumulated
	if ( m_bBlended && !m_bBlendAdd )
	{
		// fast path for the default case (trim_none)
		if ( m_uBlendVariants==BLEND_TRIM_NONE )
			return m_sAccum;

		// analyze the full token, find non-blended bounds
		m_iBlendNormalStart = -1;
		m_iBlendNormalEnd = -1;

		// OPTIMIZE? we can skip this based on non-blended flag from adjust
		const BYTE * p = m_sAccum;
		while ( *p )
		{
			int iLast = (int)( p-m_sAccum );
			int iCode = sphUTF8Decode(p);
			if (!( m_tLC.ToLower ( iCode ) & FLAG_CODEPOINT_BLEND ))
			{
				m_iBlendNormalEnd = (int)( p-m_sAccum );
				if ( m_iBlendNormalStart<0 )
					m_iBlendNormalStart = iLast;
			}
		}

		// build todo mask
		// check and revert a few degenerate cases
		m_uBlendVariantsPending = m_uBlendVariants;
		if ( m_uBlendVariantsPending & BLEND_TRIM_BOTH )
		{
			if ( m_iBlendNormalStart<0 )
			{
				// no heading blended; revert BOTH to TAIL
				m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
				m_uBlendVariantsPending |= BLEND_TRIM_TAIL;
			} else if ( m_iBlendNormalEnd<0 )
			{
				// no trailing blended; revert BOTH to HEAD
				m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
				m_uBlendVariantsPending |= BLEND_TRIM_HEAD;
			}
		}
		if ( m_uBlendVariantsPending & BLEND_TRIM_HEAD )
		{
			// either no heading blended, or pure blended; revert HEAD to NONE
			if ( m_iBlendNormalStart<=0 )
			{
				m_uBlendVariantsPending &= ~BLEND_TRIM_HEAD;
				m_uBlendVariantsPending |= BLEND_TRIM_NONE;
			}
		}
		if ( m_uBlendVariantsPending & BLEND_TRIM_TAIL )
		{
			// either no trailing blended, or pure blended; revert TAIL to NONE
			if ( m_iBlendNormalEnd<=0 || m_sAccum[m_iBlendNormalEnd]==0 )
			{
				m_uBlendVariantsPending &= ~BLEND_TRIM_TAIL;
				m_uBlendVariantsPending |= BLEND_TRIM_NONE;
			}
		}

		// ok, we are going to return a few variants after all, flag that
		// OPTIMIZE? add fast path for "single" variants?
		m_bBlendAdd = true;
		assert ( m_uBlendVariantsPending );

		// we also have to stash the original blended token
		// because accumulator contents may get trashed by caller (say, when stemming)
		strncpy ( (char*)m_sAccumBlend, (char*)m_sAccum, sizeof(m_sAccumBlend)-1 );
	}

	// case 2, caller is checking for pending variants, have we even got any?
	if ( !m_bBlendAdd )
		return NULL;

	// handle trim_none
	// this MUST be the first handler, so that we could avoid copying below, and just return the original accumulator
	if ( m_uBlendVariantsPending & BLEND_TRIM_NONE )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_NONE;
		m_bBlended = true;
		return m_sAccum;
	}

	// handle trim_all
	if ( m_uBlendVariantsPending & BLEND_TRIM_ALL )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_ALL;
		m_bBlended = true;
		const BYTE * pSrc = m_sAccumBlend;
		BYTE * pDst = m_sAccum;
		while ( *pSrc )
		{
			int iCode = sphUTF8Decode ( pSrc );
			if ( !( m_tLC.ToLower ( iCode ) & FLAG_CODEPOINT_BLEND ) )
				pDst += sphUTF8Encode ( pDst, ( iCode & MASK_CODEPOINT ) );
		}
		*pDst = '\0';

		return m_sAccum;
	}

	// handle trim_both
	if ( m_uBlendVariantsPending & BLEND_TRIM_BOTH )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_BOTH;
		if ( m_iBlendNormalStart<0 )
			m_uBlendVariantsPending |= BLEND_TRIM_TAIL; // no heading blended; revert BOTH to TAIL
		else if ( m_iBlendNormalEnd<0 )
			m_uBlendVariantsPending |= BLEND_TRIM_HEAD; // no trailing blended; revert BOTH to HEAD
		else
		{
			assert ( m_iBlendNormalStart<m_iBlendNormalEnd );
			CopySubstring ( m_sAccum, m_sAccumBlend+m_iBlendNormalStart, m_iBlendNormalEnd-m_iBlendNormalStart );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// handle TRIM_HEAD
	if ( m_uBlendVariantsPending & BLEND_TRIM_HEAD )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_HEAD;
		if ( m_iBlendNormalStart>=0 )
		{
			// FIXME! need we check for overshorts?
			CopySubstring ( m_sAccum, m_sAccumBlend+m_iBlendNormalStart, sizeof(m_sAccum) );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// handle TRIM_TAIL
	if ( m_uBlendVariantsPending & BLEND_TRIM_TAIL )
	{
		m_uBlendVariantsPending &= ~BLEND_TRIM_TAIL;
		if ( m_iBlendNormalEnd>0 )
		{
			// FIXME! need we check for overshorts?
			CopySubstring ( m_sAccum, m_sAccumBlend, m_iBlendNormalEnd );
			m_bBlended = true;
			return m_sAccum;
		}
	}

	// all clear, no more variants to go
	m_bBlendAdd = false;
	return NULL;
}


static inline bool IsCapital ( int iCh )
{
	return iCh>='A' && iCh<='Z';
}


static inline bool IsWhitespace ( BYTE c )
{
	return ( c=='\0' || c==' ' || c=='\t' || c=='\r' || c=='\n' );
}


static inline bool IsWhitespace ( int c )
{
	return ( c=='\0' || c==' ' || c=='\t' || c=='\r' || c=='\n' );
}


static inline bool IsBoundary ( BYTE c, bool bPhrase )
{
	// FIXME? sorta intersects with specials
	// then again, a shortened-down list (more strict syntax) is reasonble here too
	return IsWhitespace(c) || c=='"' || ( !bPhrase && ( c=='(' || c==')' || c=='|' ) );
}


static inline bool IsPunctuation ( int c )
{
	return ( c>=33 && c<=47 ) || ( c>=58 && c<=64 ) || ( c>=91 && c<=96 ) || ( c>=123 && c<=126 );
}


int CSphTokenizerBase::CodepointArbitrationI ( int iCode )
{
	if ( !m_bDetectSentences )
		return iCode;

	// detect sentence boundaries
	// FIXME! should use charset_table (or add a new directive) and support languages other than English
	int iSymbol = iCode & MASK_CODEPOINT;
	if ( iSymbol=='?' || iSymbol=='!' )
	{
		// definitely a sentence boundary
		return MAGIC_CODE_SENTENCE | FLAG_CODEPOINT_SPECIAL;
	}

	if ( iSymbol=='.' )
	{
		// inline dot ("in the U.K and"), not a boundary
		bool bInwordDot = ( sphIsAlpha ( m_pCur[0] ) || ( m_pCur[0] & 0x80 )==0x80 // IsAlpha to consider UTF8 chars
			|| m_pCur[0]==',' );

		// followed by a small letter or an opening paren, not a boundary
		// FIXME? might want to scan for more than one space
		// Yoyodine Inc. exists ...
		// Yoyodine Inc. (the company) ..
		bool bInphraseDot = ( sphIsSpace ( m_pCur[0] )
			&& ( ( 'a'<=m_pCur[1] && m_pCur[1]<='z' )
				|| ( m_pCur[1]=='(' && 'a'<=m_pCur[2] && m_pCur[2]<='z' ) ) );

		// preceded by something that looks like a middle name, opening first name, salutation
		bool bMiddleName = false;
		switch ( m_iAccum )
		{
			case 1:
				// 1-char capital letter
				// example: J. R. R. Tolkien, who wrote Hobbit ...
				// example: John D. Doe ...
				bMiddleName = IsCapital ( m_pCur[-2] );
				break;
			case 2:
				// 2-char token starting with a capital
				if ( IsCapital ( m_pCur[-3] ) )
				{
					// capital+small
					// example: Known as Mr. Doe ...
					if ( !IsCapital ( m_pCur[-2] ) )
						bMiddleName = true;

					// known capital+capital (MR, DR, MS)
					if (
						( m_pCur[-3]=='M' && m_pCur[-2]=='R' ) ||
						( m_pCur[-3]=='M' && m_pCur[-2]=='S' ) ||
						( m_pCur[-3]=='D' && m_pCur[-2]=='R' ) )
							bMiddleName = true;
				}
				break;
			case 3:
				// preceded by a known 3-byte token (MRS, DRS)
				// example: Survived by Mrs. Doe ...
				if ( ( m_sAccum[0]=='m' || m_sAccum[0]=='d' ) && m_sAccum[1]=='r' && m_sAccum[2]=='s' )
					bMiddleName = true;
				break;
		}

		if ( !bInwordDot && !bInphraseDot && !bMiddleName )
		{
			// sentence boundary
			return MAGIC_CODE_SENTENCE | FLAG_CODEPOINT_SPECIAL;
		} else
		{
			// just a character
			if ( ( iCode & MASK_FLAGS )==FLAG_CODEPOINT_SPECIAL )
				return 0; // special only, not dual? then in this context, it is a separator
			else
				return iCode & ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL ); // perhaps it was blended, so return the original code
		}
	}

	// pass-through
	return iCode;
}


int CSphTokenizerBase::CodepointArbitrationQ ( int iCode, bool bWasEscaped, BYTE uNextByte )
{
	if ( iCode & FLAG_CODEPOINT_NGRAM )
		return iCode; // ngrams are handled elsewhere

	int iSymbol = iCode & MASK_CODEPOINT;

	// codepoints can't be blended and special at the same time
	if ( ( iCode & FLAG_CODEPOINT_BLEND ) && ( iCode & FLAG_CODEPOINT_SPECIAL ) )
	{
		bool bBlend =
			bWasEscaped || // escaped characters should always act as blended
			( m_bPhrase && !sphIsModifier ( iSymbol ) && iSymbol!='"' ) || // non-modifier special inside phrase
			( m_iAccum && ( iSymbol=='@' || iSymbol=='/' || iSymbol=='-' ) ); // some specials in the middle of a token

		// clear special or blend flags
		iCode &= bBlend
			? ~( FLAG_CODEPOINT_DUAL | FLAG_CODEPOINT_SPECIAL )
			: ~( FLAG_CODEPOINT_DUAL | FLAG_CODEPOINT_BLEND );
	}

	// escaped specials are not special
	// dash and dollar inside the word are not special (however, single opening modifier is not a word!)
	// non-modifier specials within phrase are not special
	bool bDashInside = ( m_iAccum && iSymbol=='-' && !( m_iAccum==1 && sphIsModifier ( m_sAccum[0] ) ));
	if ( iCode & FLAG_CODEPOINT_SPECIAL )
		if ( bWasEscaped
			|| bDashInside
			|| ( m_iAccum && iSymbol=='$' && !IsBoundary ( uNextByte, m_bPhrase ) )
			|| ( m_bPhrase && iSymbol!='"' && !sphIsModifier ( iSymbol ) ) )
	{
		if ( iCode & FLAG_CODEPOINT_DUAL )
			iCode &= ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL );
		else
			iCode = 0;
	}

	// if we didn't remove special by now, it must win
	if ( iCode & FLAG_CODEPOINT_DUAL )
	{
		assert ( iCode & FLAG_CODEPOINT_SPECIAL );
		iCode = iSymbol | FLAG_CODEPOINT_SPECIAL;
	}

	// ideally, all conflicts must be resolved here
	// well, at least most
	assert ( sphBitCount ( iCode & MASK_FLAGS )<=1 );
	return iCode;
}


static FORCE_INLINE bool IsSeparator ( int iFolded, bool bFirst )
{
	// eternal separator
	if ( iFolded<0 || ( iFolded & MASK_CODEPOINT )==0 )
		return true;

	// just a codepoint
	if (!( iFolded & MASK_FLAGS ))
		return false;

	// any magic flag, besides dual
	if (!( iFolded & FLAG_CODEPOINT_DUAL ))
		return true;

	// FIXME? n-grams currently also set dual
	if ( iFolded & FLAG_CODEPOINT_NGRAM )
		return true;

	// dual depends on position
	return bFirst;
}

// handles escaped specials that are not in the character set
// returns true if the codepoint should be processed as a simple codepoint,
// returns false if it should be processed as a whitespace
// for example: aaa\!bbb => aaa bbb
static inline bool Special2Simple ( int & iCodepoint )
{
	if ( ( iCodepoint & FLAG_CODEPOINT_DUAL ) || !( iCodepoint & FLAG_CODEPOINT_SPECIAL ) )
	{
		iCodepoint &= ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL );
		return true;
	}

	return false;
}

static bool CheckRemap ( CSphString & sError, const CSphVector<CSphRemapRange> & dRemaps, const char * sSource, bool bCanRemap, const CSphLowercaser & tLC )
{
	// check
	ARRAY_FOREACH ( i, dRemaps )
	{
		const CSphRemapRange & r = dRemaps[i];

		for ( int j=r.m_iStart; j<=r.m_iEnd; j++ )
		{
			if ( tLC.ToLower ( j ) )
			{
				sError.SetSprintf ( "%s characters must not be referenced anywhere else (code=U+%x)", sSource, j );
				return false;
			}
		}

		if ( bCanRemap )
		{
			for ( int j=r.m_iRemapStart; j<=r.m_iRemapStart + r.m_iEnd - r.m_iStart; j++ )
			{
				if ( tLC.ToLower ( j ) )
				{
					sError.SetSprintf ( "%s characters must not be referenced anywhere else (code=U+%x)", sSource, j );
					return false;
				}
			}
		}
	}

	return true;
}


bool ISphTokenizer::RemapCharacters ( const char * sConfig, DWORD uFlags, const char * sSource, bool bCanRemap, CSphString & sError )
{
	// parse
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	if ( !CheckRemap ( sError, dRemaps, sSource, bCanRemap, m_tLC ) )
		return false;

	// add mapping
	m_tLC.AddRemaps ( dRemaps, uFlags );
	return true;
}

bool ISphTokenizer::SetBoundary ( const char * sConfig, CSphString & sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_BOUNDARY, "phrase boundary", false, sError );
}

bool ISphTokenizer::SetIgnoreChars ( const char * sConfig, CSphString & sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_IGNORE, "ignored", false, sError );
}

bool ISphTokenizer::SetBlendChars ( const char * sConfig, CSphString & sError )
{
	return sConfig ? RemapCharacters ( sConfig, FLAG_CODEPOINT_BLEND, "blend", true, sError ) : false;
}


static bool sphStrncmp ( const char * sCheck, int iCheck, const char * sRef )
{
	return ( iCheck==(int)strlen(sRef) && memcmp ( sCheck, sRef, iCheck )==0 );
}


bool ISphTokenizer::SetBlendMode ( const char * sMode, CSphString & sError )
{
	if ( !sMode || !*sMode )
	{
		m_uBlendVariants = BLEND_TRIM_NONE;
		m_bBlendSkipPure = false;
		return true;
	}

	m_uBlendVariants = 0;
	const char * p = sMode;
	while ( *p )
	{
		while ( !sphIsAlpha(*p) )
			p++;
		if ( !*p )
			break;

		const char * sTok = p;
		while ( sphIsAlpha(*p) )
			p++;

		int iLen = int ( p-sTok );
		if ( sphStrncmp ( sTok, iLen, "trim_none" ) )
			m_uBlendVariants |= BLEND_TRIM_NONE;
		else if ( sphStrncmp ( sTok, iLen, "trim_head" ) )
			m_uBlendVariants |= BLEND_TRIM_HEAD;
		else if ( sphStrncmp ( sTok, iLen, "trim_tail" ) )
			m_uBlendVariants |= BLEND_TRIM_TAIL;
		else if ( sphStrncmp ( sTok, iLen, "trim_both" ) )
			m_uBlendVariants |= BLEND_TRIM_BOTH;
		else if ( sphStrncmp ( sTok, iLen, "trim_all" ) )
			m_uBlendVariants |= BLEND_TRIM_ALL;
		else if ( sphStrncmp ( sTok, iLen, "skip_pure" ) )
			m_bBlendSkipPure = true;
		else
		{
			sError.SetSprintf ( "unknown blend_mode option near '%s'", sTok );
			return false;
		}
	}

	if ( !m_uBlendVariants )
	{
		sError.SetSprintf ( "blend_mode must define at least one variant to index" );
		m_uBlendVariants = BLEND_TRIM_NONE;
		m_bBlendSkipPure = false;
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////

template < bool IS_QUERY >
CSphTokenizer_UTF8<IS_QUERY>::CSphTokenizer_UTF8 ()
{
	CSphString sTmp;
	SetCaseFolding ( SPHINX_DEFAULT_UTF8_TABLE, sTmp );
	m_bHasBlend = false;
}


template < bool IS_QUERY >
void CSphTokenizer_UTF8<IS_QUERY>::SetBuffer ( const BYTE * sBuffer, int iLength )
{
	// check that old one is over and that new length is sane
	assert ( iLength>=0 );

	// set buffer
	m_pBuffer = sBuffer;
	m_pBufferMax = sBuffer + iLength;
	m_pCur = sBuffer;
	m_pTokenStart = m_pTokenEnd = nullptr;
	m_pBlendStart = m_pBlendEnd = nullptr;

	m_iOvershortCount = 0;
	m_bBoundary = m_bTokenBoundary = false;
}


template < bool IS_QUERY >
BYTE * CSphTokenizer_UTF8<IS_QUERY>::GetToken ()
{
	m_bWasSpecial = false;
	m_bBlended = false;
	m_iOvershortCount = 0;
	m_bTokenBoundary = false;
	m_bWasSynonym = false;

	return m_bHasBlend
		? DoGetToken<IS_QUERY,true>()
		: DoGetToken<IS_QUERY,false>();
}


bool CSphTokenizerBase2::CheckException ( const BYTE * pStart, const BYTE * pCur, bool bQueryMode )
{
	assert ( m_pExc );
	assert ( pStart );

	// at this point [pStart,pCur) is our regular tokenization candidate,
	// and pCur is pointing at what normally is considered separtor
	//
	// however, it might be either a full exception (if we're really lucky)
	// or (more likely) an exception prefix, so lets check for that
	//
	// interestingly enough, note that our token might contain a full exception
	// as a prefix, for instance [USAF] token vs [USA] exception; but in that case
	// we still need to tokenize regularly, because even exceptions need to honor
	// word boundaries

	// lets begin with a special (hopefully fast) check for the 1st byte
	const BYTE * p = pStart;
	if ( m_pExc->GetFirst ( *p )<0 )
		return false;

	// consume all the (character data) bytes until the first separator
	int iNode = 0;
	while ( p<pCur )
	{
		if ( bQueryMode && *p=='\\' )
		{
			p++;
			continue;;
		}
		iNode = m_pExc->GetNext ( iNode, *p++ );
		if ( iNode<0 )
			return false;
	}

	const BYTE * pMapEnd = nullptr; // the longest exception found so far is [pStart,pMapEnd)
	const BYTE * pMapTo = nullptr; // the destination mapping

	// now, we got ourselves a valid exception prefix, so lets keep consuming more bytes,
	// ie. until further separators, and keep looking for a full exception match
	while ( iNode>=0 )
	{
		// in query mode, ignore quoting slashes
		if ( bQueryMode && *p=='\\' )
		{
			p++;
			continue;
		}

		// decode one more codepoint, check if it is a separator
		bool bSep = true;
		bool bSpace = sphIsSpace(*p); // okay despite utf-8, cause hard whitespace is all ascii-7

		const BYTE * q = p;
		if ( p<m_pBufferMax )
			bSep = IsSeparator ( m_tLC.ToLower ( sphUTF8Decode(q) ), false ); // FIXME? sometimes they ARE first

		// there is a separator ahead, so check if we have a full match
		if ( bSep && m_pExc->GetMapping(iNode) )
		{
			pMapEnd = p;
			pMapTo = m_pExc->GetMapping(iNode);
		}

		// eof? bail
		if ( p>=m_pBufferMax )
			break;

		// not eof? consume those bytes
		if ( bSpace )
		{
			// and fold (hard) whitespace while we're at it!
			while ( sphIsSpace(*p) )
				p++;
			iNode = m_pExc->GetNext ( iNode, ' ' );
		} else
		{
			// just consume the codepoint, byte-by-byte
			while ( p<q && iNode>=0 )
				iNode = m_pExc->GetNext ( iNode, *p++ );
		}

		// we just consumed a separator, so check for a full match again
		if ( iNode>=0 && bSep && m_pExc->GetMapping(iNode) )
		{
			pMapEnd = p;
			pMapTo = m_pExc->GetMapping(iNode);
		}
	}

	// found anything?
	if ( !pMapTo )
		return false;

	strncpy ( (char*)m_sAccum, (char*)const_cast<BYTE*>(pMapTo), sizeof(m_sAccum)-1 );
	m_pCur = pMapEnd;
	m_pTokenStart = pStart;
	m_pTokenEnd = pMapEnd;
	m_iLastTokenLen = (int) strlen ( (char*)m_sAccum );

	m_bWasSynonym = true;
	return true;
}

template < bool IS_QUERY, bool IS_BLEND >
BYTE * CSphTokenizerBase2::DoGetToken ()
{
	// return pending blending variants
	if_const ( IS_BLEND )
	{
		BYTE * pVar = GetBlendedVariant ();
		if ( pVar )
			return pVar;
		m_bBlendedPart = ( m_pBlendEnd!=nullptr );
	}

	// in query mode, lets capture (soft-whitespace hard-whitespace) sequences and adjust overshort counter
	// sample queries would be (one NEAR $$$) or (one | $$$ two) where $ is not a valid character
	bool bGotNonToken = ( !IS_QUERY || m_bPhrase ); // only do this in query mode, never in indexing mode, never within phrases
	bool bGotSoft = false; // hey Beavis he said soft huh huhhuh

	m_pTokenStart = nullptr;
	for ( ;; )
	{
		// get next codepoint
		const BYTE * const pCur = m_pCur; // to redo special char, if there's a token already

		int iCodePoint;
		int iCode;
		if ( pCur<m_pBufferMax && *pCur<128 )
		{
			iCodePoint = *m_pCur++;
			iCode = m_tLC.m_pChunk[0][iCodePoint];
		} else
		{
			iCodePoint = GetCodepoint(); // advances m_pCur
			iCode = m_tLC.ToLower ( iCodePoint );
		}

		// handle escaping
		const bool bWasEscaped = ( IS_QUERY && iCodePoint=='\\' ); // whether current codepoint was escaped
		if ( bWasEscaped )
		{
			iCodePoint = GetCodepoint();
			iCode = m_tLC.ToLower ( iCodePoint );
			if ( !Special2Simple ( iCode ) )
				iCode = 0;
		}

		// handle eof
		if ( iCode<0 )
		{
			FlushAccum ();

			// suddenly, exceptions
			if ( m_pExc && m_pTokenStart && CheckException ( m_pTokenStart, pCur, IS_QUERY ) )
				return m_sAccum;

			// skip trailing short word
			if ( m_iLastTokenLen<m_tSettings.m_iMinWordLen )
			{
				if ( !m_bShortTokenFilter || !ShortTokenFilter ( m_sAccum, m_iLastTokenLen ) )
				{
					if ( m_iLastTokenLen )
						m_iOvershortCount++;
					m_iLastTokenLen = 0;
					if_const ( IS_BLEND )
						BlendAdjust ( pCur );
					return nullptr;
				}
			}

			// keep token end here as BlendAdjust might change m_pCur
			m_pTokenEnd = m_pCur;

			// return trailing word
			if_const ( IS_BLEND && !BlendAdjust ( pCur ) )
				return nullptr;
			if_const ( IS_BLEND && m_bBlended )
				return GetBlendedVariant();
			return m_sAccum;
		}

		// handle all the flags..
		if_const ( IS_QUERY )
			iCode = CodepointArbitrationQ ( iCode, bWasEscaped, *m_pCur );
		else if ( m_bDetectSentences )
			iCode = CodepointArbitrationI ( iCode );

		// handle ignored chars
		if ( iCode & FLAG_CODEPOINT_IGNORE )
			continue;

		// handle blended characters
		if_const ( IS_BLEND && ( iCode & FLAG_CODEPOINT_BLEND ) )
		{
			if ( m_pBlendEnd )
				iCode = 0;
			else
			{
				m_bBlended = true;
				m_pBlendStart = m_iAccum ? m_pTokenStart : pCur;
			}
		}

		// handle soft-whitespace-only tokens
		if ( !bGotNonToken && !m_iAccum )
		{
			if ( !bGotSoft )
			{
				// detect opening soft whitespace
				if ( ( iCode==0 && !IsWhitespace ( iCodePoint ) && !IsPunctuation ( iCodePoint ) )
					|| ( ( iCode & FLAG_CODEPOINT_BLEND ) && !m_iAccum ) )
				{
					bGotSoft = true;
				}
			} else
			{
				// detect closing hard whitespace or special
				// (if there was anything meaningful in the meantime, we must never get past the outer if!)
				if ( IsWhitespace ( iCodePoint ) || ( iCode & FLAG_CODEPOINT_SPECIAL ) )
				{
					m_iOvershortCount++;
					bGotNonToken = true;
				}
			}
		}

		// handle whitespace and boundary
		if ( m_bBoundary && ( iCode==0 ) )
		{
			m_bTokenBoundary = true;
			m_iBoundaryOffset = int ( pCur - m_pBuffer - 1 );
		}
		m_bBoundary = ( iCode & FLAG_CODEPOINT_BOUNDARY )!=0;

		// handle separator (aka, most likely a token!)
		if ( iCode==0 || m_bBoundary )
		{
			FlushAccum ();

			// suddenly, exceptions
			if ( m_pExc && CheckException ( m_pTokenStart ? m_pTokenStart : pCur, pCur, IS_QUERY ) )
				return m_sAccum;

			if_const ( IS_BLEND && !BlendAdjust ( pCur ) )
				continue;

			if ( m_iLastTokenLen<m_tSettings.m_iMinWordLen
				&& !( m_bShortTokenFilter && ShortTokenFilter ( m_sAccum, m_iLastTokenLen ) ) )
			{
				if ( m_iLastTokenLen )
					m_iOvershortCount++;
				continue;
			} else
			{
				m_pTokenEnd = pCur;
				if_const ( IS_BLEND && m_bBlended )
					return GetBlendedVariant();
				return m_sAccum;
			}
		}

		// handle specials
		if ( iCode & FLAG_CODEPOINT_SPECIAL )
		{
			// skip short words preceding specials
			if ( m_iAccum<m_tSettings.m_iMinWordLen )
			{
				m_sAccum[m_iAccum] = '\0';

				if ( !m_bShortTokenFilter || !ShortTokenFilter ( m_sAccum, m_iAccum ) )
				{
					if ( m_iAccum )
						m_iOvershortCount++;

					FlushAccum ();
				}
			}

			if ( m_iAccum==0 )
			{
				m_bNonBlended = m_bNonBlended || ( !( iCode & FLAG_CODEPOINT_BLEND ) && !( iCode & FLAG_CODEPOINT_SPECIAL ) );
				m_bWasSpecial = !( iCode & FLAG_CODEPOINT_NGRAM );
				m_pTokenStart = pCur;
				m_pTokenEnd = m_pCur;
				AccumCodepoint ( iCode & MASK_CODEPOINT ); // handle special as a standalone token
			} else
			{
				m_pCur = pCur; // we need to flush current accum and then redo special char again
				m_pTokenEnd = pCur;
			}

			FlushAccum ();

			// suddenly, exceptions
			if ( m_pExc && m_pTokenStart && CheckException ( m_pTokenStart, pCur, IS_QUERY ) )
				return m_sAccum;

			if_const ( IS_BLEND )
			{
				if ( !BlendAdjust ( pCur ) )
					continue;
				if ( m_bBlended )
					return GetBlendedVariant();
			}
			return m_sAccum;
		}

		if ( m_iAccum==0 )
			m_pTokenStart = pCur;

		// tricky bit
		// heading modifiers must not (!) affected blended status
		// eg. we want stuff like '=-' (w/o apostrophes) thrown away when pure_blend is on
		if_const ( IS_BLEND )
			if_const (!( IS_QUERY && !m_iAccum && sphIsModifier ( iCode & MASK_CODEPOINT ) ) )
				m_bNonBlended = m_bNonBlended || !( iCode & FLAG_CODEPOINT_BLEND );

		// just accumulate
		// manual inlining of utf8 encoder gives us a few extra percent
		// which is important here, this is a hotspot
		if ( m_iAccum<SPH_MAX_WORD_LEN && ( m_pAccum-m_sAccum+SPH_MAX_UTF8_BYTES<=(int)sizeof(m_sAccum) ) )
		{
			iCode &= MASK_CODEPOINT;
			m_iAccum++;
			SPH_UTF8_ENCODE ( m_pAccum, iCode );
		}
	}
}


void CSphTokenizerBase2::FlushAccum ()
{
	assert ( m_pAccum-m_sAccum < (int)sizeof(m_sAccum) );
	m_iLastTokenLen = m_iAccum;
	*m_pAccum = 0;
	m_iAccum = 0;
	m_pAccum = m_sAccum;
}


template < bool IS_QUERY >
ISphTokenizer * CSphTokenizer_UTF8<IS_QUERY>::Clone ( ESphTokenizerClone eMode ) const
{
	CSphTokenizerBase * pClone;
	if ( eMode!=SPH_CLONE_INDEX )
		pClone = new CSphTokenizer_UTF8<true>();
	else
		pClone = new CSphTokenizer_UTF8<false>();
	pClone->CloneBase ( this, eMode );
	return pClone;
}


template < bool IS_QUERY >
int CSphTokenizer_UTF8<IS_QUERY>::GetCodepointLength ( int iCode ) const
{
	if ( iCode<128 )
		return 1;

	int iBytes = 0;
	while ( iCode & 0x80 )
	{
		iBytes++;
		iCode <<= 1;
	}

	assert ( iBytes>=2 && iBytes<=4 );
	return iBytes;
}

/////////////////////////////////////////////////////////////////////////////

template < bool IS_QUERY >
bool CSphTokenizer_UTF8Ngram<IS_QUERY>::SetNgramChars ( const char * sConfig, CSphString & sError )
{
	assert ( this->m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
	CSphVector<CSphRemapRange> dRemaps;
	CSphCharsetDefinitionParser tParser;
	if ( !tParser.Parse ( sConfig, dRemaps ) )
	{
		sError = tParser.GetLastError();
		return false;
	}

	if ( !CheckRemap ( sError, dRemaps, "ngram", true, this->m_tLC ) )
		return false;

	// gcc braindamage requires this
	this->m_tLC.AddRemaps ( dRemaps, FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_SPECIAL ); // !COMMIT support other n-gram lengths than 1
	return true;
}


template < bool IS_QUERY >
void CSphTokenizer_UTF8Ngram<IS_QUERY>::SetNgramLen ( int iLen )
{
	assert ( this->m_eMode!=SPH_CLONE_QUERY_LIGHTWEIGHT );
	assert ( iLen>0 );
	m_iNgramLen = iLen;
}


template < bool IS_QUERY >
BYTE * CSphTokenizer_UTF8Ngram<IS_QUERY>::GetToken ()
{
	// !COMMIT support other n-gram lengths than 1
	assert ( m_iNgramLen==1 );
	return CSphTokenizer_UTF8<IS_QUERY>::GetToken ();
}

//////////////////////////////////////////////////////////////////////////

CSphMultiformTokenizer::CSphMultiformTokenizer ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer )
	: CSphTokenFilter ( pTokenizer )
	, m_pMultiWordforms ( pContainer )
{
	assert ( pTokenizer && pContainer );
	m_dStoredTokens.Reserve ( pContainer->m_iMaxTokens + 6 ); // max form tokens + some blended tokens
	m_sTokenizedMultiform[0] = '\0';
}

BYTE * CSphMultiformTokenizer::GetToken ()
{
	if ( m_iOutputPending > -1 && m_pCurrentForm )
	{
		if ( ++m_iOutputPending>=m_pCurrentForm->m_dNormalForm.GetLength() )
		{
			m_iOutputPending = -1;
			m_pCurrentForm = nullptr;
		} else
		{
			StoredToken_t & tStart = m_dStoredTokens[m_iStart];
			strncpy ( (char *)tStart.m_sToken, m_pCurrentForm->m_dNormalForm[m_iOutputPending].m_sForm.cstr(), sizeof(tStart.m_sToken)-1 );

			tStart.m_iTokenLen = m_pCurrentForm->m_dNormalForm[m_iOutputPending].m_iLengthCP;
			tStart.m_bBoundary = false;
			tStart.m_bSpecial = false;
			tStart.m_bBlended = false;
			tStart.m_bBlendedPart = false;
			return tStart.m_sToken;
		}
	}

	m_sTokenizedMultiform[0] = '\0';
	m_iStart++;

	if ( m_iStart>=m_dStoredTokens.GetLength() )
	{
		m_iStart = 0;
		m_dStoredTokens.Resize ( 0 );
		const BYTE * pToken = CSphTokenFilter::GetToken();
		if ( !pToken )
			return NULL;

		FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
		while ( m_dStoredTokens.Last().m_bBlended || m_dStoredTokens.Last().m_bBlendedPart )
		{
			pToken = CSphTokenFilter::GetToken ();
			if ( !pToken )
				break;

			FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
		}
	}

	CSphMultiforms ** pWordforms = NULL;
	int iTokensGot = 1;
	bool bBlended = false;

	// check multi-form
	// only blended parts checked for multi-form with blended
	// in case ALL blended parts got transformed primary blended got replaced by normal form
	// otherwise blended tokens provided as is
	if ( m_dStoredTokens[m_iStart].m_bBlended || m_dStoredTokens[m_iStart].m_bBlendedPart )
	{
		if ( m_dStoredTokens[m_iStart].m_bBlended && m_iStart+1<m_dStoredTokens.GetLength() && m_dStoredTokens[m_iStart+1].m_bBlendedPart )
		{
			pWordforms = m_pMultiWordforms->m_Hash ( (const char *)m_dStoredTokens[m_iStart+1].m_sToken );
			if ( pWordforms )
			{
				bBlended = true;
				for ( int i=m_iStart+2; i<m_dStoredTokens.GetLength(); i++ )
				{
					// break out on blended over or got completely different blended
					if ( m_dStoredTokens[i].m_bBlended || !m_dStoredTokens[i].m_bBlendedPart )
						break;

					iTokensGot++;
				}
			}
		}
	} else
	{
		pWordforms = m_pMultiWordforms->m_Hash ( (const char *)m_dStoredTokens[m_iStart].m_sToken );
		if ( pWordforms )
		{
			int iTokensNeed = (*pWordforms)->m_iMaxTokens + 1;
			int iCur = m_iStart;
			bool bGotBlended = false;

			// collect up ahead to multi-form tokens or all blended tokens
			while ( iTokensGot<iTokensNeed || bGotBlended )
			{
				iCur++;
				if ( iCur>=m_dStoredTokens.GetLength() )
				{
					// fetch next token
					const BYTE* pToken = CSphTokenFilter::GetToken ();
					if ( !pToken )
						break;

					FillStoredTokenInfo ( m_dStoredTokens.Add(), pToken, m_pTokenizer );
				}

				bool bCurBleneded = ( m_dStoredTokens[iCur].m_bBlended || m_dStoredTokens[iCur].m_bBlendedPart );
				if ( bGotBlended && !bCurBleneded )
					break;

				bGotBlended = bCurBleneded;
				// count only regular tokens; can not fold mixed (regular+blended) tokens to form
				iTokensGot += ( bGotBlended ? 0 : 1 );
			}
		}
	}

	if ( !pWordforms || iTokensGot<(*pWordforms)->m_iMinTokens+1 )
		return m_dStoredTokens[m_iStart].m_sToken;

	int iStartToken = m_iStart + ( bBlended ? 1 : 0 );
	ARRAY_FOREACH ( i, (*pWordforms)->m_pForms )
	{
		const CSphMultiform * pCurForm = (*pWordforms)->m_pForms[i];
		int iFormTokCount = pCurForm->m_dTokens.GetLength();

		if ( iTokensGot<iFormTokCount+1 || ( bBlended && iTokensGot!=iFormTokCount+1 ) )
			continue;

		int iForm = 0;
		for ( ; iForm<iFormTokCount; iForm++ )
		{
			const StoredToken_t & tTok = m_dStoredTokens[iStartToken + 1 + iForm];
			const char * szStored = (const char*)tTok.m_sToken;
			const char * szNormal = pCurForm->m_dTokens[iForm].cstr ();

			if ( *szNormal!=*szStored || strcasecmp ( szNormal, szStored ) )
				break;
		}

		// early out - no destination form detected
		if ( iForm!=iFormTokCount )
			continue;

		// tokens after folded form are valid tail that should be processed next time
		if ( m_bBuildMultiform )
		{
			BYTE * pOut = m_sTokenizedMultiform;
			BYTE * pMax = pOut + sizeof(m_sTokenizedMultiform);
			for ( int j=0; j<iFormTokCount+1 && pOut<pMax; j++ )
			{
				const StoredToken_t & tTok = m_dStoredTokens[iStartToken+j];
				const BYTE * sTok = tTok.m_sToken;
				if ( j && pOut<pMax )
					*pOut++ = ' ';
				while ( *sTok && pOut<pMax )
					*pOut++ = *sTok++;
			}
			*pOut = '\0';
			*(pMax-1) = '\0';
		}

		if ( !bBlended )
		{
			// fold regular tokens to form
			const StoredToken_t & tStart = m_dStoredTokens[m_iStart];
			StoredToken_t & tEnd = m_dStoredTokens[m_iStart+iFormTokCount];
			m_iStart += iFormTokCount;

			strncpy ( (char *)tEnd.m_sToken, pCurForm->m_dNormalForm[0].m_sForm.cstr(), sizeof(tEnd.m_sToken)-1 );
			tEnd.m_szTokenStart = tStart.m_szTokenStart;
			tEnd.m_iTokenLen = pCurForm->m_dNormalForm[0].m_iLengthCP;

			tEnd.m_bBoundary = false;
			tEnd.m_bSpecial = false;
			tEnd.m_bBlended = false;
			tEnd.m_bBlendedPart = false;

			if ( pCurForm->m_dNormalForm.GetLength()>1 )
			{
				m_iOutputPending = 0;
				m_pCurrentForm = pCurForm;
			}
		} else
		{
			// replace blended by form
			// FIXME: add multiple destination token support here (if needed)
			assert ( pCurForm->m_dNormalForm.GetLength()==1 );
			StoredToken_t & tDst = m_dStoredTokens[m_iStart];
			strncpy ( (char *)tDst.m_sToken, pCurForm->m_dNormalForm[0].m_sForm.cstr(), sizeof(tDst.m_sToken)-1 );
			tDst.m_iTokenLen = pCurForm->m_dNormalForm[0].m_iLengthCP;
		}
		break;
	}

	return m_dStoredTokens[m_iStart].m_sToken;
}


ISphTokenizer * CSphMultiformTokenizer::Clone ( ESphTokenizerClone eMode ) const
{
	TokenizerRefPtr_c pClone { m_pTokenizer->Clone ( eMode ) };
	return CreateMultiformFilter ( pClone, m_pMultiWordforms );
}


void CSphMultiformTokenizer::SetBufferPtr ( const char * sNewPtr )
{
	m_iStart = 0;
	m_iOutputPending = -1;
	m_pCurrentForm = nullptr;
	m_dStoredTokens.Resize ( 0 );
	CSphTokenFilter::SetBufferPtr ( sNewPtr );
}

void CSphMultiformTokenizer::SetBuffer ( const BYTE * sBuffer, int iLength )
{
	CSphTokenFilter::SetBuffer ( sBuffer, iLength );
	SetBufferPtr ( (const char *)sBuffer );
}

uint64_t CSphMultiformTokenizer::GetSettingsFNV () const
{
	uint64_t uHash = CSphTokenFilter::GetSettingsFNV();
	uHash ^= (uint64_t)m_pMultiWordforms;
	return uHash;
}

int CSphMultiformTokenizer::SkipBlended ()
{
	bool bGotBlended = ( m_iStart<m_dStoredTokens.GetLength() &&
		( m_dStoredTokens[m_iStart].m_bBlended || m_dStoredTokens[m_iStart].m_bBlendedPart ) );
	if ( !bGotBlended )
		return 0;

	int iWasStart = m_iStart;
	for ( int iTok=m_iStart+1; iTok<m_dStoredTokens.GetLength() && m_dStoredTokens[iTok].m_bBlendedPart && !m_dStoredTokens[iTok].m_bBlended; iTok++ )
		m_iStart = iTok;

	return m_iStart-iWasStart;
}

bool CSphMultiformTokenizer::WasTokenMultiformDestination ( bool & bHead, int & iDestCount ) const
{
	if ( m_iOutputPending>-1 && m_pCurrentForm && m_pCurrentForm->m_dNormalForm.GetLength()>1 && m_iOutputPending<m_pCurrentForm->m_dNormalForm.GetLength() )
	{
		bHead = ( m_iOutputPending==0 );
		iDestCount = m_pCurrentForm->m_dNormalForm.GetLength();
		return true;
	} else
	{
		return false;
	}
}

/////////////////////////////////////////////////////////////////////////////
// FILTER
/////////////////////////////////////////////////////////////////////////////
void CSphFilterSettings::SetExternalValues ( const SphAttr_t * pValues, int nValues )
{
	m_pValues = pValues;
	m_nValues = nValues;
}


bool CSphFilterSettings::operator == ( const CSphFilterSettings & rhs ) const
{
	// check name, mode, type
	if ( m_sAttrName!=rhs.m_sAttrName || m_bExclude!=rhs.m_bExclude || m_eType!=rhs.m_eType )
		return false;

	switch ( m_eType )
	{
		case SPH_FILTER_RANGE:
			return m_iMinValue==rhs.m_iMinValue && m_iMaxValue==rhs.m_iMaxValue;

		case SPH_FILTER_FLOATRANGE:
			return m_fMinValue==rhs.m_fMinValue && m_fMaxValue==rhs.m_fMaxValue;

		case SPH_FILTER_VALUES:
			if ( m_dValues.GetLength()!=rhs.m_dValues.GetLength() )
				return false;

			ARRAY_FOREACH ( i, m_dValues )
				if ( m_dValues[i]!=rhs.m_dValues[i] )
					return false;

			return true;

		case SPH_FILTER_STRING:
		case SPH_FILTER_USERVAR:
		case SPH_FILTER_STRING_LIST:
			if ( m_dStrings.GetLength ()!=rhs.m_dStrings.GetLength () )
				return false;
			ARRAY_FOREACH ( i, m_dStrings )
				if ( m_dStrings[i]!=rhs.m_dStrings[i] )
					return false;
			return ( m_eMvaFunc==rhs.m_eMvaFunc );

		default:
			assert ( 0 && "internal error: unhandled filter type in comparison" );
			return false;
	}
}


uint64_t CSphFilterSettings::GetHash() const
{
	uint64_t h = sphFNV64 ( &m_eType, sizeof(m_eType) );
	h = sphFNV64 ( &m_bExclude, sizeof(m_bExclude), h );
	switch ( m_eType )
	{
		case SPH_FILTER_VALUES:
			{
				int t = m_dValues.GetLength();
				h = sphFNV64 ( &t, sizeof(t), h );
				h = sphFNV64 ( m_dValues.Begin(), t*sizeof(SphAttr_t), h );
				break;
			}
		case SPH_FILTER_RANGE:
			h = sphFNV64 ( &m_iMaxValue, sizeof(m_iMaxValue), sphFNV64 ( &m_iMinValue, sizeof(m_iMinValue), h ) );
			break;
		case SPH_FILTER_FLOATRANGE:
			h = sphFNV64 ( &m_fMaxValue, sizeof(m_fMaxValue), sphFNV64 ( &m_fMinValue, sizeof(m_fMinValue), h ) );
			break;
		case SPH_FILTER_STRING:
		case SPH_FILTER_USERVAR:
		case SPH_FILTER_STRING_LIST:
			ARRAY_FOREACH ( iString, m_dStrings )
				h = sphFNV64cont ( m_dStrings[iString].cstr(), h );
			if ( m_eMvaFunc!=SPH_MVAFUNC_NONE )
				h = sphFNV64 ( &m_eMvaFunc, sizeof ( m_eMvaFunc ), h );
		break;
		case SPH_FILTER_NULL:
			break;
		default:
			assert ( 0 && "internal error: unhandled filter type in GetHash()" );
	}
	return h;
}


bool FilterTreeItem_t::operator == ( const FilterTreeItem_t & rhs ) const
{
	return ( m_iLeft==rhs.m_iLeft && m_iRight==rhs.m_iRight && m_iFilterItem==rhs.m_iFilterItem && m_bOr==rhs.m_bOr );
}


uint64_t FilterTreeItem_t::GetHash() const
{
	uint64_t uHash = sphFNV64 ( &m_iLeft, sizeof(m_iLeft) );
	uHash = sphFNV64 ( &m_iRight, sizeof(m_iRight), uHash );
	uHash = sphFNV64 ( &m_iFilterItem, sizeof(m_iFilterItem), uHash );
	uHash = sphFNV64 ( &m_bOr, sizeof(m_bOr), uHash );
	return uHash;
}


/////////////////////////////////////////////////////////////////////////////
// QUERY
/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

struct SelectBounds_t
{
	int		m_iStart;
	int		m_iEnd;
};
#define YYSTYPE SelectBounds_t
class SelectParser_t;

#include "bissphinxselect.h"

class SelectParser_t
{
public:
	int				GetToken ( YYSTYPE * lvalp );
	void			AddItem ( YYSTYPE * pExpr, ESphAggrFunc eAggrFunc=SPH_AGGR_NONE, YYSTYPE * pStart=NULL, YYSTYPE * pEnd=NULL );
	void			AddItem ( const char * pToken, YYSTYPE * pStart=NULL, YYSTYPE * pEnd=NULL );
	void			AliasLastItem ( YYSTYPE * pAlias );
	void			AddOption ( YYSTYPE * pOpt, YYSTYPE * pVal );

private:
	void			AutoAlias ( CSphQueryItem & tItem, YYSTYPE * pStart, YYSTYPE * pEnd );
	bool			IsTokenEqual ( YYSTYPE * pTok, const char * sRef );

public:
	CSphString		m_sParserError;
	const char *	m_pLastTokenStart;

	const char *	m_pStart;
	const char *	m_pCur;

	CSphQuery *		m_pQuery;
};

static int yylex ( YYSTYPE * lvalp, SelectParser_t * pParser )
{
	return pParser->GetToken ( lvalp );
}

static void yyerror ( SelectParser_t * pParser, const char * sMessage )
{
	pParser->m_sParserError.SetSprintf ( "%s near '%s'", sMessage, pParser->m_pLastTokenStart );
}

#include "bissphinxselect.c"

int SelectParser_t::GetToken ( YYSTYPE * lvalp )
{
	// skip whitespace, check eof
	while ( isspace ( *m_pCur ) )
		m_pCur++;
	if ( !*m_pCur )
		return 0;

	// begin working that token
	m_pLastTokenStart = m_pCur;
	lvalp->m_iStart = int ( m_pCur-m_pStart );

	// check for constant
	if ( isdigit ( *m_pCur ) )
	{
		char * pEnd = NULL;
		double VARIABLE_IS_NOT_USED fDummy = strtod ( m_pCur, &pEnd );
		m_pCur = pEnd;
		lvalp->m_iEnd = int ( m_pCur-m_pStart );
		return SEL_TOKEN;
	}

	// check for token
	if ( sphIsAttr ( m_pCur[0] ) || ( m_pCur[0]=='@' && sphIsAttr ( m_pCur[1] ) && !isdigit ( m_pCur[1] ) ) )
	{
		m_pCur++;
		while ( sphIsAttr ( *m_pCur ) ) m_pCur++;
		lvalp->m_iEnd = int ( m_pCur-m_pStart );

		#define LOC_CHECK(_str,_len,_ret) \
			if ( lvalp->m_iEnd==_len+lvalp->m_iStart && strncasecmp ( m_pStart+lvalp->m_iStart, _str, _len )==0 ) return _ret;

		LOC_CHECK ( "ID", 2, SEL_ID );
		LOC_CHECK ( "AS", 2, SEL_AS );
		LOC_CHECK ( "OR", 2, TOK_OR );
		LOC_CHECK ( "AND", 3, TOK_AND );
		LOC_CHECK ( "NOT", 3, TOK_NOT );
		LOC_CHECK ( "DIV", 3, TOK_DIV );
		LOC_CHECK ( "MOD", 3, TOK_MOD );
		LOC_CHECK ( "AVG", 3, SEL_AVG );
		LOC_CHECK ( "MIN", 3, SEL_MIN );
		LOC_CHECK ( "MAX", 3, SEL_MAX );
		LOC_CHECK ( "SUM", 3, SEL_SUM );
		LOC_CHECK ( "GROUP_CONCAT", 12, SEL_GROUP_CONCAT );
		LOC_CHECK ( "GROUPBY", 7, SEL_GROUPBY );
		LOC_CHECK ( "COUNT", 5, SEL_COUNT );
		LOC_CHECK ( "DISTINCT", 8, SEL_DISTINCT );
		LOC_CHECK ( "WEIGHT", 6, SEL_WEIGHT );
		LOC_CHECK ( "OPTION", 6, SEL_OPTION );
		LOC_CHECK ( "IS", 2, TOK_IS );
		LOC_CHECK ( "NULL", 4, TOK_NULL );
		LOC_CHECK ( "FOR", 3, TOK_FOR );
		LOC_CHECK ( "IN", 2, TOK_FUNC_IN );
		LOC_CHECK ( "RAND", 4, TOK_FUNC_RAND );

		#undef LOC_CHECK

		return SEL_TOKEN;
	}

	// check for equality checks
	lvalp->m_iEnd = 1+lvalp->m_iStart;
	switch ( *m_pCur )
	{
		case '<':
			m_pCur++;
			if ( *m_pCur=='>' ) { m_pCur++; lvalp->m_iEnd++; return TOK_NE; }
			if ( *m_pCur=='=' ) { m_pCur++; lvalp->m_iEnd++; return TOK_LTE; }
			return '<';

		case '>':
			m_pCur++;
			if ( *m_pCur=='=' ) { m_pCur++; lvalp->m_iEnd++; return TOK_GTE; }
			return '>';

		case '=':
			m_pCur++;
			if ( *m_pCur=='=' ) { m_pCur++; lvalp->m_iEnd++; }
			return TOK_EQ;

		case '\'':
		{
			const char cEnd = *m_pCur;
			for ( const char * s = m_pCur+1; *s; s++ )
			{
				if ( *s==cEnd && s-1>=m_pCur && *(s-1)!='\\' )
				{
					m_pCur = s+1;
					return TOK_CONST_STRING;
				}
			}
			return -1;
		}
	}

	// check for comment begin/end
	if ( m_pCur[0]=='/' && m_pCur[1]=='*' )
	{
		m_pCur += 2;
		lvalp->m_iEnd += 1;
		return SEL_COMMENT_OPEN;
	}
	if ( m_pCur[0]=='*' && m_pCur[1]=='/' )
	{
		m_pCur += 2;
		lvalp->m_iEnd += 1;
		return SEL_COMMENT_CLOSE;
	}

	// return char as a token
	return *m_pCur++;
}

void SelectParser_t::AutoAlias ( CSphQueryItem & tItem, YYSTYPE * pStart, YYSTYPE * pEnd )
{
	if ( pStart && pEnd )
	{
		tItem.m_sAlias.SetBinary ( m_pStart + pStart->m_iStart, pEnd->m_iEnd - pStart->m_iStart );
		sphColumnToLowercase ( const_cast<char *>( tItem.m_sAlias.cstr() ) ); // as in SqlParser_c
	} else
		tItem.m_sAlias = tItem.m_sExpr;
}

void SelectParser_t::AddItem ( YYSTYPE * pExpr, ESphAggrFunc eAggrFunc, YYSTYPE * pStart, YYSTYPE * pEnd )
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr.SetBinary ( m_pStart + pExpr->m_iStart, pExpr->m_iEnd - pExpr->m_iStart );
	sphColumnToLowercase ( const_cast<char *>( tItem.m_sExpr.cstr() ) );
	tItem.m_eAggrFunc = eAggrFunc;
	AutoAlias ( tItem, pStart, pEnd );
}

void SelectParser_t::AddItem ( const char * pToken, YYSTYPE * pStart, YYSTYPE * pEnd )
{
	CSphQueryItem & tItem = m_pQuery->m_dItems.Add();
	tItem.m_sExpr = pToken;
	tItem.m_eAggrFunc = SPH_AGGR_NONE;
	sphColumnToLowercase ( const_cast<char *>( tItem.m_sExpr.cstr() ) );
	AutoAlias ( tItem, pStart, pEnd );
}

void SelectParser_t::AliasLastItem ( YYSTYPE * pAlias )
{
	if ( pAlias )
	{
		CSphQueryItem & tItem = m_pQuery->m_dItems.Last();
		tItem.m_sAlias.SetBinary ( m_pStart + pAlias->m_iStart, pAlias->m_iEnd - pAlias->m_iStart );
		tItem.m_sAlias.ToLower();
	}
}

bool SelectParser_t::IsTokenEqual ( YYSTYPE * pTok, const char * sRef )
{
	auto iLen = (int) strlen(sRef);
	if ( iLen!=( pTok->m_iEnd - pTok->m_iStart ) )
		return false;
	return strncasecmp ( m_pStart + pTok->m_iStart, sRef, iLen )==0;
}

void SelectParser_t::AddOption ( YYSTYPE * pOpt, YYSTYPE * pVal )
{
	if ( IsTokenEqual ( pOpt, "sort_method" ) )
	{
		if ( IsTokenEqual ( pVal, "kbuffer" ) )
			m_pQuery->m_bSortKbuffer = true;
	} else if ( IsTokenEqual ( pOpt, "max_predicted_time" ) )
	{
		char szNumber[256];
		int iLen = pVal->m_iEnd-pVal->m_iStart;
		assert ( iLen < (int)sizeof(szNumber) );
		strncpy ( szNumber, m_pStart+pVal->m_iStart, iLen );
		int64_t iMaxPredicted = strtoull ( szNumber, NULL, 10 );
		m_pQuery->m_iMaxPredictedMsec = int(iMaxPredicted > INT_MAX ? INT_MAX : iMaxPredicted );
	}
}

bool ParseSelectList ( CSphString & sError, CSphQuery & tQuery )
{
	tQuery.m_dItems.Reset();
	if ( tQuery.m_sSelect.IsEmpty() )
		return true; // empty is ok; will just return everything

	SelectParser_t tParser;
	tParser.m_pStart = tQuery.m_sSelect.cstr();
	tParser.m_pCur = tParser.m_pStart;
	tParser.m_pQuery = &tQuery;

	yyparse ( &tParser );

	sError = tParser.m_sParserError;
	return sError.IsEmpty ();
}


int ExpandKeywords ( int iIndexOpt, QueryOption_e eQueryOpt, const CSphIndexSettings & tSettings, bool bWordDict )
{
	if ( tSettings.m_iMinInfixLen<=0 && tSettings.GetMinPrefixLen ( bWordDict )<=0 && !tSettings.m_bIndexExactWords )
		return KWE_DISABLED;

	int iOpt = KWE_DISABLED;
	if ( eQueryOpt==QUERY_OPT_DEFAULT )
		iOpt = iIndexOpt;
	else if ( eQueryOpt==QUERY_OPT_MORPH_NONE )
		iOpt = KWE_MORPH_NONE;
	 else
		iOpt = ( eQueryOpt==QUERY_OPT_ENABLED ? KWE_ENABLED : KWE_DISABLED );

	if ( ( iOpt & KWE_STAR )==KWE_STAR && tSettings.m_iMinInfixLen<=0 && tSettings.GetMinPrefixLen ( bWordDict )<=0 )
		iOpt ^= KWE_STAR;

	if ( ( iOpt & KWE_EXACT )==KWE_EXACT && !tSettings.m_bIndexExactWords )
		iOpt ^= KWE_EXACT;

	if ( ( iOpt & KWE_MORPH_NONE )==KWE_MORPH_NONE && !tSettings.m_bIndexExactWords )
		iOpt ^= KWE_MORPH_NONE;

	return iOpt;
}


/////////////////////////////////////////////////////////////////////////////
// QUERY STATS
/////////////////////////////////////////////////////////////////////////////
void CSphQueryStats::Add ( const CSphQueryStats & tStats )
{
	m_iFetchedDocs += tStats.m_iFetchedDocs;
	m_iFetchedHits += tStats.m_iFetchedHits;
	m_iSkips += tStats.m_iSkips;
}


/////////////////////////////////////////////////////////////////////////////
// SCHEMAS
/////////////////////////////////////////////////////////////////////////////

static CSphString sphDumpAttr ( const CSphColumnInfo & tAttr )
{
	CSphString sRes;
	sRes.SetSprintf ( "%s %s:%d@%d", sphTypeName ( tAttr.m_eAttrType ), tAttr.m_sName.cstr(),
		tAttr.m_tLocator.m_iBitCount, tAttr.m_tLocator.m_iBitOffset );
	return sRes;
}


/// make string lowercase but keep case of JSON.field
void sphColumnToLowercase ( char * sVal )
{
	if ( !sVal || !*sVal )
		return;

	// make all chars lowercase but only prior to '.', ',', and '[' delimiters
	// leave quoted values unchanged
	for ( bool bQuoted=false; *sVal && *sVal!='.' && *sVal!=',' && *sVal!='['; sVal++ )
	{
		if ( !bQuoted )
			*sVal = (char) tolower ( *sVal );
		if ( *sVal=='\'' )
			bQuoted = !bQuoted;
	}
}

//////////////////////////////////////////////////////////////////////////

CSphColumnInfo::CSphColumnInfo ( const char * sName, ESphAttr eType )
	: m_sName ( sName )
	, m_eAttrType ( eType )
{
	sphColumnToLowercase ( const_cast<char *>( m_sName.cstr() ) );
}


bool CSphColumnInfo::IsDataPtr() const
{
	return sphIsDataPtrAttr ( m_eAttrType );
}


bool CSphColumnInfo::IsColumnar() const
{
	return m_uAttrFlags & ATTR_COLUMNAR;
}


bool CSphColumnInfo::HasStringHashes() const
{
	return m_uAttrFlags & ATTR_COLUMNAR_HASHES;
}


bool CSphColumnInfo::IsColumnarExpr() const
{
	return m_pExpr.Ptr() && m_pExpr->IsColumnar();
}

//////////////////////////////////////////////////////////////////////////

void CSphSchemaHelper::InsertAttr ( CSphVector<CSphColumnInfo> & dAttrs, CSphVector<int> & dUsed, int iPos, const CSphColumnInfo & tCol, bool bDynamic )
{
	assert ( 0<=iPos && iPos<=dAttrs.GetLength() );
	assert ( tCol.m_eAttrType!=SPH_ATTR_NONE );
	if ( tCol.m_eAttrType==SPH_ATTR_NONE )
		return;

	dAttrs.Insert ( iPos, tCol );
	CSphAttrLocator & tLoc = dAttrs [ iPos ].m_tLocator;

	int iBits = ROWITEM_BITS;
	if ( tLoc.m_iBitCount>0 )
		iBits = tLoc.m_iBitCount;
	if ( tCol.m_eAttrType==SPH_ATTR_BOOL )
		iBits = 1;
	if ( tCol.m_eAttrType==SPH_ATTR_BIGINT || tCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
		iBits = 64;

	if ( tCol.IsDataPtr() )
	{
		assert ( bDynamic );
		iBits = ROWITEMPTR_BITS;
		m_dDataPtrAttrs.Add ( dUsed.GetLength() );
	}

	tLoc.m_iBitCount = iBits;
	tLoc.m_bDynamic = bDynamic;

	if ( tCol.IsColumnar() )
	{
		// can't have an attribute that is stored in columnar and common storage at the same time
		// FIXME: maybe add extra flags here?
		tLoc.m_iBitOffset = 0;
	}
	else if ( sphIsBlobAttr(tCol) )
	{
		bool bColumnarID = !dAttrs.IsEmpty() && dAttrs[0].m_sName==sphGetDocidName() && dAttrs[0].IsColumnar();

		tLoc.m_iBitCount = 0;
		tLoc.m_iBitOffset = 0;

		int iBlobAttrId = 0;
		for ( auto & i : dAttrs )
			if ( sphIsBlobAttr(i) )
				i.m_tLocator.m_iBlobAttrId = iBlobAttrId++;

		for ( auto & i : dAttrs )
			if ( sphIsBlobAttr(i) )
			{
				i.m_tLocator.m_nBlobAttrs = iBlobAttrId;
				i.m_tLocator.m_iBlobRowOffset = bColumnarID ? 0 : 1;
			}
	}
	else if ( iBits>=ROWITEM_BITS )
	{
		tLoc.m_iBitOffset = dUsed.GetLength()*ROWITEM_BITS;
		int iItems = (iBits+ROWITEM_BITS-1) / ROWITEM_BITS;
		for ( int i=0; i<iItems; i++ )
			dUsed.Add ( ROWITEM_BITS );
	} else
	{
		int iItem;
		for ( iItem=0; iItem<dUsed.GetLength(); iItem++ )
			if ( dUsed[iItem]+iBits<=ROWITEM_BITS )
				break;
		if ( iItem==dUsed.GetLength() )
			dUsed.Add ( 0 );
		tLoc.m_iBitOffset = iItem*ROWITEM_BITS + dUsed[iItem];
		dUsed[iItem] += iBits;
	}
}

void CSphSchemaHelper::Swap ( CSphSchemaHelper & rhs ) noexcept
{
	rhs.m_dDataPtrAttrs.SwapData ( m_dDataPtrAttrs );
	rhs.m_dDynamicUsed.SwapData ( m_dDynamicUsed );
}

void CSphSchemaHelper::ResetSchemaHelper()
{
	m_dDataPtrAttrs.Reset();
	m_dDynamicUsed.Reset ();
}

void CSphSchemaHelper::CloneMatch ( CSphMatch & tDst, const CSphMatch & rhs ) const
{
	CloneMatchSpecial ( tDst, rhs, m_dDataPtrAttrs );
}

void CSphSchemaHelper::FreeDataPtrs ( CSphMatch & tMatch ) const
{
	FreeDataSpecial ( tMatch, m_dDataPtrAttrs );
}

void CSphSchemaHelper::CopyPtrs ( CSphMatch & tDst, const CSphMatch & rhs ) const
{
	// notes on PACKEDFACTORS
	// not immediately obvious: this is not needed while pushing matches to sorters; factors are held in an outer hash table
	// but it is necessary to copy factors when combining results from several indexes via a sorter because at this moment matches are the owners of factor data
	CopyPtrsSpecial ( tDst, rhs, m_dDataPtrAttrs );
}

CSphVector<int> CSphSchemaHelper::SubsetPtrs ( CSphVector<int> &dDiscarded ) const
{
	CSphVector<int> dFiltered;
	dDiscarded.Uniq ();
	for ( int iPtr : m_dDataPtrAttrs )
		if ( !dDiscarded.BinarySearch ( iPtr ) )
			dFiltered.Add ( iPtr );
	dFiltered.Uniq ();
	return dFiltered;
}

// in destination free listed attrs, then copy new from rhs
void CSphSchemaHelper::CloneMatchSpecial ( CSphMatch & tDst, const CSphMatch & rhs,	const VecTraits_T<int> & dSpecials ) const
{
	FreeDataSpecial ( tDst, dSpecials );
	tDst.Combine ( rhs, GetDynamicSize () );
	for ( auto i : m_dDataPtrAttrs )
		*(BYTE **) ( tDst.m_pDynamic+i ) = nullptr;
	CopyPtrsSpecial ( tDst, rhs, dSpecials );
}

// declared in sphinxstd.h
void sphDeallocatePacked ( BYTE * pBlob )
{
	if ( !pBlob )
		return;
	//const BYTE * pFoo = pBlob;
	//sphDeallocateSmall ( pBlob, sphCalcPackedLength ( sphUnzipInt ( pFoo ) ) );
	sphDeallocateSmall ( pBlob );
}

// fixme! direct reinterpreting rows is not good idea. Use sphGetAttr/sphSetAttr!
/*
 * wide (64bit) attributes occupies 2 rows and placed order lo,high
 * On LE arch (intel) it is ok to reinterpret them back as 64-bit pointer
 * However on BE (mips) you have problems since such cast gives garbage.
 */
void CSphSchemaHelper::FreeDataSpecial ( CSphMatch& tMatch, const VecTraits_T<int> &dSpecials )
{
	if ( !tMatch.m_pDynamic )
		return;

	for ( auto iOffset : dSpecials )
	{
		BYTE * &pData = *( BYTE ** ) ( tMatch.m_pDynamic + iOffset );
		sphDeallocatePacked ( pData );
		pData = nullptr;
	}
}

inline int CSphRsetSchema::ActualLen() const
{
	if ( m_pIndexSchema )
		return m_pIndexSchema->GetAttrsCount () - m_dRemoved.GetLength ();
	return 0;
}

void CSphSchemaHelper::CopyPtrsSpecial ( CSphMatch & tDst, const CSphMatch& tSrc, const VecTraits_T<int> &dSpecials )
{
	auto pSrc = tSrc.m_pDynamic;
	assert ( pSrc || dSpecials.IsEmpty() );
	for ( auto i : dSpecials )
	{
		const BYTE * pData = *( BYTE ** ) ( pSrc + i );
		if ( pData )
			*( BYTE ** ) ( tDst.m_pDynamic + i ) = sph::CopyPackedAttr ( pData );
	}
}

void CSphSchemaHelper::MovePtrsSpecial ( CSphMatch & tDst, CSphMatch & tSrc, const VecTraits_T<int> &dSpecials )
{
	auto pSrc = tSrc.m_pDynamic;
	assert ( pSrc || dSpecials.IsEmpty () );
	for ( auto i : dSpecials ) {
		memcpy ( tDst.m_pDynamic+i, pSrc+i, sizeof ( BYTE * ) );
		*(BYTE **) ( pSrc+i ) = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////

CSphSchema::CSphSchema ( CSphString sName )
	: m_sName ( std::move(sName) )
{
	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ) );
}


CSphSchema::CSphSchema ( const CSphSchema & rhs )
{
	m_dDataPtrAttrs = rhs.m_dDataPtrAttrs;
	m_iFirstFieldLenAttr = rhs.m_iFirstFieldLenAttr;
	m_iLastFieldLenAttr = rhs.m_iLastFieldLenAttr;
	m_iRowSize = rhs.m_iRowSize;
	m_sName = rhs.m_sName;
	m_dFields = rhs.m_dFields;
	m_dAttrs = rhs.m_dAttrs;
	m_dStaticUsed = rhs.m_dStaticUsed;
	m_dDynamicUsed = rhs.m_dDynamicUsed;
	memcpy ( m_dBuckets, rhs.m_dBuckets, sizeof ( m_dBuckets ));
}

CSphSchema::CSphSchema ( CSphSchema && rhs ) noexcept
	: CSphSchema ( nullptr )
{
	Swap(rhs);
}

void CSphSchema::Swap ( CSphSchema& rhs ) noexcept
{
	::Swap ( m_dDataPtrAttrs, rhs.m_dDataPtrAttrs );
	::Swap ( m_iFirstFieldLenAttr, rhs.m_iFirstFieldLenAttr );
	::Swap ( m_iLastFieldLenAttr, rhs.m_iLastFieldLenAttr );
	::Swap ( m_iRowSize, rhs.m_iRowSize );
	::Swap ( m_sName, rhs.m_sName );
	::Swap ( m_dFields, rhs.m_dFields );
	::Swap ( m_dAttrs, rhs.m_dAttrs );
	::Swap ( m_dStaticUsed, rhs.m_dStaticUsed );
	::Swap ( m_dDynamicUsed, rhs.m_dDynamicUsed );
	std::swap ( m_dBuckets, rhs.m_dBuckets );
}


CSphSchema & CSphSchema::operator = ( const ISphSchema & rhs )
{
	Reset();

	m_sName = rhs.GetName();

	for ( int i = 0; i < rhs.GetAttrsCount(); ++i )
	{
		const CSphColumnInfo & tAttr = rhs.GetAttr(i);

		if ( tAttr.m_tLocator.m_bDynamic )
			AddAttr ( tAttr, true );
		else // static attr, keep previous storage
			m_dAttrs.Add ( tAttr );
	}

	for ( int i = 0 ; i < rhs.GetFieldsCount(); ++i )
		AddField ( rhs.GetField(i) );

	RebuildHash();

	return *this;
}



CSphSchema & CSphSchema::operator = ( CSphSchema rhs )
{
	Swap ( rhs );
	return *this;
}

const char * CSphSchema::GetName() const
{
	return m_sName.cstr();
}


bool CSphSchema::CompareTo ( const CSphSchema & rhs, CSphString & sError, bool bFullComparison ) const
{
	// check attr count
	if ( GetAttrsCount()!=rhs.GetAttrsCount() )
	{
		sError.SetSprintf ( "attribute count mismatch (me=%s, in=%s, myattrs=%d, inattrs=%d)", m_sName.cstr(), rhs.m_sName.cstr(), GetAttrsCount(), rhs.GetAttrsCount() );
		return false;
	}

	// check attrs
	ARRAY_FOREACH ( i, m_dAttrs )
	{
		const CSphColumnInfo & tAttr1 = rhs.m_dAttrs[i];
		const CSphColumnInfo & tAttr2 = m_dAttrs[i];

		bool bMismatch;
		if ( bFullComparison )
			bMismatch = !(tAttr1==tAttr2);
		else
		{
			ESphAttr eAttr1 = tAttr1.m_eAttrType;
			ESphAttr eAttr2 = tAttr2.m_eAttrType;

			bMismatch = tAttr1.m_sName!=tAttr2.m_sName || eAttr1!=eAttr2 || tAttr1.m_eWordpart!=tAttr2.m_eWordpart ||
				tAttr1.m_tLocator.m_iBitCount!=tAttr2.m_tLocator.m_iBitCount ||
				tAttr1.m_tLocator.m_iBitOffset!=tAttr2.m_tLocator.m_iBitOffset;
		}

		if ( bMismatch )
		{
			sError.SetSprintf ( "attribute mismatch (me=%s, in=%s, idx=%d, myattr=%s, inattr=%s)",
				m_sName.cstr(), rhs.m_sName.cstr(), i, sphDumpAttr ( m_dAttrs[i] ).cstr(), sphDumpAttr ( rhs.m_dAttrs[i] ).cstr() );
			return false;
		}
	}

	// check field count
	if ( rhs.m_dFields.GetLength()!=m_dFields.GetLength() )
	{
		sError.SetSprintf ( "fulltext fields count mismatch (me=%s, in=%s, myfields=%d, infields=%d)",
			m_sName.cstr(), rhs.m_sName.cstr(),
			m_dFields.GetLength(), rhs.m_dFields.GetLength() );
		return false;
	}

	// check fulltext field names
	ARRAY_FOREACH ( i, rhs.m_dFields )
		if ( rhs.m_dFields[i].m_sName!=m_dFields[i].m_sName )
	{
		sError.SetSprintf ( "fulltext field mismatch (me=%s, myfield=%s, idx=%d, in=%s, infield=%s)",
			m_sName.cstr(), rhs.m_sName.cstr(),
			i, m_dFields[i].m_sName.cstr(), rhs.m_dFields[i].m_sName.cstr() );
		return false;
	}

	return true;
}


int CSphSchema::GetFieldIndex ( const char * sName ) const
{
	if ( !sName )
		return -1;
	ARRAY_FOREACH ( i, m_dFields )
		if ( strcasecmp ( m_dFields[i].m_sName.cstr(), sName )==0 )
			return i;
	return -1;
}


const CSphColumnInfo * CSphSchema::GetField ( const char * szName ) const
{
	int iField = GetFieldIndex(szName);
	if ( iField==-1 )
		return nullptr;

	return &m_dFields[iField];
}


int CSphSchema::GetAttrIndex ( const char * sName ) const
{
	if ( !sName )
		return -1;

	if ( m_dAttrs.GetLength()>=HASH_THRESH )
	{
		DWORD uCrc = sphCRC32 ( sName );
		DWORD uPos = m_dBuckets [ uCrc%BUCKET_COUNT ];
		while ( uPos!=0xffff && m_dAttrs [ uPos ].m_sName!=sName )
			uPos = m_dAttrs [ uPos ].m_uNext;

		return (short)uPos; // 0xffff == -1 is our "end of list" marker
	}

	ARRAY_FOREACH ( i, m_dAttrs )
		if ( m_dAttrs[i].m_sName==sName )
			return i;

	return -1;
}


const CSphColumnInfo * CSphSchema::GetAttr ( const char * sName ) const
{
	int iIndex = GetAttrIndex ( sName );
	if ( iIndex>=0 )
		return &m_dAttrs[iIndex];
	return NULL;
}


void CSphSchema::Reset ()
{
	CSphSchemaHelper::ResetSchemaHelper();

	m_dFields.Reset();
	m_dAttrs.Reset();
	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ));
	m_dStaticUsed.Reset();
	m_iRowSize = 0;
}


void CSphSchema::InsertAttr ( int iPos, const CSphColumnInfo & tCol, bool bDynamic )
{
	// it's redundant in case of AddAttr
	if ( iPos!=m_dAttrs.GetLength() )
		UpdateHash ( iPos-1, 1 );

	CSphSchemaHelper::InsertAttr ( m_dAttrs, bDynamic ? m_dDynamicUsed : m_dStaticUsed, iPos, tCol, bDynamic );

	// update field length locators
	if ( tCol.m_eAttrType==SPH_ATTR_TOKENCOUNT )
	{
		m_iFirstFieldLenAttr = m_iFirstFieldLenAttr==-1 ? iPos : Min ( m_iFirstFieldLenAttr, iPos );
		m_iLastFieldLenAttr = Max ( m_iLastFieldLenAttr, iPos );
	}

	// do hash add
	if ( m_dAttrs.GetLength()==HASH_THRESH )
		RebuildHash();
	else if ( m_dAttrs.GetLength()>HASH_THRESH )
	{
		WORD & uPos = GetBucketPos ( m_dAttrs [ iPos ].m_sName.cstr() );
		m_dAttrs [ iPos ].m_uNext = uPos;
		uPos = (WORD)iPos;
	}

	m_iRowSize = GetRowSize();
}


void CSphSchema::RemoveAttr ( const char * szAttr, bool bDynamic )
{
	int iIndex = GetAttrIndex ( szAttr );
	if ( iIndex<0 )
		return;

	if ( bDynamic )
		m_dDynamicUsed.Reset();
	else
		m_dStaticUsed.Reset();

	CSphSchemaHelper::ResetSchemaHelper();
	m_iFirstFieldLenAttr = -1;
	m_iLastFieldLenAttr = -1;

	CSphVector<CSphColumnInfo> dBackup;
	dBackup.Reserve ( m_dAttrs.GetLength() );
	dBackup.SwapData ( m_dAttrs );
	ARRAY_FOREACH ( i, dBackup )
		if ( i!=iIndex )
			AddAttr ( dBackup[i], bDynamic );
}


void CSphSchema::AddAttr ( const CSphColumnInfo & tCol, bool bDynamic )
{
	InsertAttr ( m_dAttrs.GetLength(), tCol, bDynamic );
}


void CSphSchema::AddField ( const char * szFieldName )
{
	CSphColumnInfo & tField = m_dFields.Add();
	tField.m_sName = szFieldName;
}


void CSphSchema::AddField ( const CSphColumnInfo & tField )
{
	m_dFields.Add ( tField );
}

void CSphSchema::RemoveField ( const char * szFieldName )
{
	auto iIdx = GetFieldIndex ( szFieldName );
	RemoveField ( iIdx );
}

void CSphSchema::RemoveField ( int iIdx )
{
	if (iIdx>=0)
		m_dFields.Remove ( iIdx );
}

int CSphSchema::GetAttrId_FirstFieldLen() const
{
	return m_iFirstFieldLenAttr;
}


int CSphSchema::GetAttrId_LastFieldLen() const
{
	return m_iLastFieldLenAttr;
}


bool CSphSchema::IsReserved ( const char * szToken )
{
	static const char * dReserved[] =
	{
		"AND", "AS", "BY", "DISTINCT", "DIV", "EXPLAIN", "FACET",
		"FALSE", "FORCE", "FROM", "IGNORE", "IN", "INDEXES", "IS",
		"LIMIT", "LOGS", "MOD", "NOT", "NULL", "OFFSET", "OR",
		"ORDER", "REGEX", "RELOAD", "SELECT", "SYSFILTERS",
		"TRUE", "USE", NULL
	};

	const char ** p = dReserved;
	while ( *p )
		if ( strcasecmp ( szToken, *p++ )==0 )
			return true;
	return false;
}


WORD & CSphSchema::GetBucketPos ( const char * sName )
{
	DWORD uCrc = sphCRC32 ( sName );
	return m_dBuckets [ uCrc % BUCKET_COUNT ];
}


void CSphSchema::RebuildHash ()
{
	if ( m_dAttrs.GetLength()<HASH_THRESH )
		return;

	memset ( m_dBuckets, 0xFF, sizeof ( m_dBuckets ));

	ARRAY_FOREACH ( i, m_dAttrs )
	{
		WORD & uPos = GetBucketPos ( m_dAttrs[i].m_sName.cstr() );
		m_dAttrs[i].m_uNext = uPos;
		uPos = WORD(i);
	}
}


void CSphSchema::RebuildLocators()
{
	CSphVector<CSphColumnInfo> dBackup = m_dAttrs;

	m_dDynamicUsed.Reset();
	m_dStaticUsed.Reset();

	CSphSchemaHelper::ResetSchemaHelper();
	m_dAttrs.Reset();
	m_iFirstFieldLenAttr = -1;
	m_iLastFieldLenAttr = -1;

	for ( auto & i : dBackup )
		AddAttr ( i, i.m_tLocator.m_bDynamic );
}


void CSphSchema::UpdateHash ( int iStartIndex, int iAddVal )
{
	if ( m_dAttrs.GetLength()<HASH_THRESH )
		return;

	for ( auto &dAttr : m_dAttrs )
	{
		WORD &uPos = dAttr.m_uNext;
		if ( uPos!=0xffff && uPos>iStartIndex )
			uPos = (WORD)( uPos + iAddVal );
	}
	for ( WORD &uPos : m_dBuckets )
		if ( uPos!=0xffff && uPos>iStartIndex )
			uPos = (WORD)( uPos + iAddVal );
}


void CSphSchema::AssignTo ( CSphRsetSchema & lhs ) const
{
	lhs = *this;
}


void CSphSchema::CloneWholeMatch ( CSphMatch & tDst, const CSphMatch & rhs ) const
{
	FreeDataPtrs ( tDst );
	tDst.Combine ( rhs, GetRowSize() );
	CopyPtrs ( tDst, rhs );
}


void CSphSchema::SetFieldWordpart ( int iField, ESphWordpart eWordpart )
{
	m_dFields[iField].m_eWordpart = eWordpart;
}


void CSphSchema::SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs )
{
	m_dAttrs.SwapData(dAttrs);
	RebuildHash();
	m_iRowSize = GetRowSize();
}


ISphSchema * CSphSchema::CloneMe () const
{
	return new CSphSchema ( *this );
}


bool CSphSchema::HasBlobAttrs() const
{
	return m_dAttrs.any_of ([] (const CSphColumnInfo& i) { return sphIsBlobAttr(i);});
}


inline int CSphSchema::GetCachedRowSize() const
{
	return m_iRowSize;
}


void CSphSchema::SetupColumnarFlags ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings )
{
	SmallStringHash_T<int> hColumnar, hRowwise, hNoHashes;
	for ( const auto & i : tSettings.m_dColumnarAttrs )
		hColumnar.Add ( 0, i );

	for ( const auto & i : tSettings.m_dRowwiseAttrs )
		hRowwise.Add ( 0, i );

	for ( const auto & i : tSettings.m_dColumnarStringsNoHash )
		hNoHashes.Add ( 0, i );

	bool bHaveColumnar = false;
	for ( auto & tAttr : m_dAttrs )
	{
		if ( hColumnar.Exists ( tAttr.m_sName ) && hRowwise.Exists ( tAttr.m_sName ) )
		{
			if ( pWarnings )
			{
				CSphString sWarning;
				sWarning.SetSprintf ( "both columnar and rowwise storages specified for '%s'; using rowwise", tAttr.m_sName.cstr() );
				pWarnings->Add(sWarning);
			}
		}

		if ( sphIsInternalAttr(tAttr) )
			tAttr.m_eEngine = AttrEngine_e::ROWWISE;
		else
		{
			tAttr.m_eEngine = AttrEngine_e::DEFAULT;
			if ( hColumnar.Exists ( tAttr.m_sName ) )
				tAttr.m_eEngine = AttrEngine_e::COLUMNAR;

			if ( hRowwise.Exists ( tAttr.m_sName ) )
				tAttr.m_eEngine = AttrEngine_e::ROWWISE;
		}

		// combine per-index and per-attribute engine settings
		AttrEngine_e eEngine = tSettings.m_eEngine;
		if ( tAttr.m_eEngine!=AttrEngine_e::DEFAULT )
			eEngine = tAttr.m_eEngine;

		if ( eEngine!=AttrEngine_e::COLUMNAR )
			continue;

		if ( tAttr.m_eAttrType==SPH_ATTR_JSON && pWarnings )
		{
			if ( pWarnings )
			{
				CSphString sWarning;
				sWarning.SetSprintf ( "columnar storage does not support json type ('%s' attribute specified as columnar)", tAttr.m_sName.cstr() );
				pWarnings->Add(sWarning);
			}

			continue;
		}

		tAttr.m_uAttrFlags |= CSphColumnInfo::ATTR_COLUMNAR;

		// set strings to have pre-generated hashes by default
		if ( tAttr.m_eAttrType==SPH_ATTR_STRING && !hNoHashes.Exists ( tAttr.m_sName ) )
			tAttr.m_uAttrFlags |= CSphColumnInfo::ATTR_COLUMNAR_HASHES;

		bHaveColumnar = true;
	}

	// if all blob attrs turned columnar, there's no point in keeping the blob locator
	bool bHaveBlobs = m_dAttrs.any_of ( []( const CSphColumnInfo & tAttr ){ return sphIsBlobAttr(tAttr); } );
	const CSphColumnInfo * pBlobLocator = GetAttr ( sphGetBlobLocatorName() );
	if ( !bHaveBlobs && pBlobLocator )
		RemoveAttr ( sphGetBlobLocatorName(), pBlobLocator->m_tLocator.m_bDynamic );

	if ( bHaveColumnar )
		RebuildLocators();	// remove columnar attrs from row storage
}


void CSphSchema::SetupFlags ( const CSphSourceSettings & tSettings, bool bPQ, StrVec_t * pWarnings )
{
	for ( auto & tSchemaField : m_dFields )
	{
		for ( const auto & sStored : tSettings.m_dStoredFields )
			if ( tSchemaField.m_sName==sStored )
				tSchemaField.m_uFieldFlags |= CSphColumnInfo::FIELD_STORED;

		for ( const auto & sStoredOnly : tSettings.m_dStoredOnlyFields )
			if ( tSchemaField.m_sName==sStoredOnly )
				tSchemaField.m_uFieldFlags = CSphColumnInfo::FIELD_STORED;
	}

	if ( !bPQ )
		SetupColumnarFlags ( tSettings, pWarnings );
}


bool CSphSchema::HasStoredFields() const
{
	return m_dFields.any_of ( []( const CSphColumnInfo & tField ){ return tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED; } );
}


bool CSphSchema::HasColumnarAttrs() const
{
	return m_dAttrs.any_of ( []( const CSphColumnInfo & tAttr ){ return tAttr.IsColumnar(); } );
}


bool CSphSchema::HasNonColumnarAttrs() const
{
	return m_dAttrs.any_of ( []( const CSphColumnInfo & tAttr ){ return !tAttr.IsColumnar(); } );
}


bool CSphSchema::IsFieldStored ( int iField ) const
{
	return !!( m_dFields[iField].m_uFieldFlags & CSphColumnInfo::FIELD_STORED );
}

//////////////////////////////////////////////////////////////////////////

void CSphRsetSchema::ResetRsetSchema ()
{
	CSphSchemaHelper::ResetSchemaHelper();

	m_pIndexSchema = nullptr;
	m_dExtraAttrs.Reset();
}


void CSphRsetSchema::AddAttr ( const CSphColumnInfo & tCol, bool bDynamic )
{
	assert ( bDynamic );
	InsertAttr ( m_dExtraAttrs, m_dDynamicUsed, m_dExtraAttrs.GetLength(), tCol, true );
}


const char * CSphRsetSchema::GetName() const
{
	return m_pIndexSchema ? m_pIndexSchema->GetName() : nullptr;
}


int CSphRsetSchema::GetRowSize() const
{
	// we copy over dynamic map in case index schema has dynamic attributes
	// (that happens in case of inline attributes, or RAM segments in RT indexes)
	// so there is no need to add GetDynamicSize() here
	return GetDynamicSize () + ( m_pIndexSchema ? m_pIndexSchema->GetStaticSize() : 0 );
}


int CSphRsetSchema::GetStaticSize() const
{
	// result set schemas additions are always dynamic
	return m_pIndexSchema ? m_pIndexSchema->GetStaticSize() : 0;
}


int CSphRsetSchema::GetAttrsCount() const
{
	return m_dExtraAttrs.GetLength () + ActualLen();
}


int CSphRsetSchema::GetFieldsCount() const
{
	return m_pIndexSchema ? m_pIndexSchema->GetFieldsCount() : 0;
}


int CSphRsetSchema::GetAttrIndex ( const char * sName ) const
{
	ARRAY_FOREACH ( i, m_dExtraAttrs )
		if ( m_dExtraAttrs[i].m_sName==sName )
			return i + ActualLen();

	if ( !m_pIndexSchema )
		return -1;

	int iRes = m_pIndexSchema->GetAttrIndex(sName);
	if ( iRes>=0 )
	{
		if ( m_dRemoved.Contains ( iRes ) )
			return -1;
		int iSub = 0;
		ARRAY_FOREACH_COND ( i, m_dRemoved, iRes>=m_dRemoved[i] )
			iSub++;
		return iRes - iSub;
	}
	return -1;
}


int CSphRsetSchema::GetFieldIndex ( const char * sName ) const
{
	if ( !m_pIndexSchema )
		return -1;

	return m_pIndexSchema->GetFieldIndex(sName);
}


const CSphColumnInfo & CSphRsetSchema::GetField ( int iIndex ) const
{
	assert ( m_pIndexSchema );
	return m_pIndexSchema->GetField(iIndex);
}


const CSphVector<CSphColumnInfo> & CSphRsetSchema::GetFields() const
{
	assert ( m_pIndexSchema );
	return m_pIndexSchema->GetFields();
}


const CSphColumnInfo & CSphRsetSchema::GetAttr ( int iIndex ) const
{
	if ( !m_pIndexSchema )
		return m_dExtraAttrs[iIndex];

	if ( iIndex<ActualLen () )
	{
		ARRAY_FOREACH_COND ( i, m_dRemoved, iIndex>=m_dRemoved[i] )
			++iIndex;

		return m_pIndexSchema->GetAttr(iIndex);
	}

	return m_dExtraAttrs [ iIndex - ActualLen() ];
}


const CSphColumnInfo * CSphRsetSchema::GetAttr ( const char * sName ) const
{
	for ( auto & tExtraAttr : m_dExtraAttrs )
		if ( tExtraAttr.m_sName==sName )
			return &tExtraAttr;

	if ( m_pIndexSchema )
		return m_pIndexSchema->GetAttr ( sName );

	return nullptr;
}


int CSphRsetSchema::GetAttrId_FirstFieldLen() const
{
	// we assume that field_lens are in the base schema
	return m_pIndexSchema->GetAttrId_FirstFieldLen();
}


int CSphRsetSchema::GetAttrId_LastFieldLen() const
{
	// we assume that field_lens are in the base schema
	return m_pIndexSchema->GetAttrId_LastFieldLen();
}


CSphRsetSchema & CSphRsetSchema::operator = ( const ISphSchema & rhs )
{
	rhs.AssignTo ( *this );
	return *this;
}


CSphRsetSchema & CSphRsetSchema::operator = ( const CSphSchema & rhs )
{
	ResetRsetSchema();
	m_pIndexSchema = &rhs;

	// copy over dynamic rowitems map
	// so that the new attributes we might add would not overlap
	m_dDynamicUsed = rhs.m_dDynamicUsed;

	// copy data ptr map. we might want to add proper access via virtual funcs later
	m_dDataPtrAttrs = rhs.m_dDataPtrAttrs;

	return *this;
}

void CSphRsetSchema::Swap ( CSphRsetSchema & rhs ) noexcept
{
	CSphSchemaHelper::Swap ( rhs );
	::Swap ( m_pIndexSchema, rhs.m_pIndexSchema );
	::Swap ( m_dExtraAttrs, rhs.m_dExtraAttrs );
	::Swap ( m_dRemoved, rhs.m_dRemoved );
}

CSphRsetSchema::CSphRsetSchema ( const CSphRsetSchema & rhs )
	: CSphSchemaHelper ( rhs )
{
	m_pIndexSchema = rhs.m_pIndexSchema;
	m_dExtraAttrs = rhs.m_dExtraAttrs;
	m_dRemoved = rhs.m_dRemoved;
}


void CSphRsetSchema::RemoveStaticAttr ( int iAttr )
{
	assert ( m_pIndexSchema );
	assert ( iAttr>=0 );
	assert ( iAttr<ActualLen() );

	// map from rset indexes (adjusted for removal) to index schema indexes (the original ones)
	ARRAY_FOREACH_COND ( i, m_dRemoved, iAttr>=m_dRemoved[i] )
		iAttr++;

	// we may be removing our static attribute after expressions have been created and that invalidates their dependent cols (if any)
	// we need to update those
	int iExtraAttrId = ActualLen();
	for ( auto & i : m_dExtraAttrs )
		if ( i.m_pExpr )
			i.m_pExpr->Command ( SPH_EXPR_UPDATE_DEPENDENT_COLS, &iExtraAttrId );

	m_dRemoved.Add(iAttr);
	m_dRemoved.Uniq();
}


void CSphRsetSchema::SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs )
{
#ifndef NDEBUG
	// ensure that every incoming column has a matching original column
	// only check locators and attribute types, because at this stage,
	// names that are used in dAttrs are already overwritten by the aliases
	// (example: SELECT col1 a, col2 b, count(*) c FROM test)
	//
	// FIXME? maybe also lockdown the schema from further swaps, adds etc from here?

	for ( auto &dAttr : dAttrs )
	{
		auto fComparer = [&dAttr] ( const CSphColumnInfo &dInfo ) {
			return dInfo.m_tLocator==dAttr.m_tLocator
				&& dInfo.m_eAttrType==dAttr.m_eAttrType;
		};
		bool bFound1 = m_pIndexSchema ? m_pIndexSchema->m_dAttrs.any_of ( fComparer ) : false;
		bool bFound2 = m_dExtraAttrs.any_of ( fComparer );
		assert ( bFound1 || bFound2 );
	}
#endif
	m_dExtraAttrs.SwapData ( dAttrs );
	m_pIndexSchema = nullptr;
}


ISphSchema * CSphRsetSchema::CloneMe () const
{
	auto pClone = new CSphRsetSchema;
	AssignTo ( *pClone );

	ARRAY_FOREACH ( i, m_dExtraAttrs )
		if ( m_dExtraAttrs[i].m_pExpr )
			pClone->m_dExtraAttrs[i].m_pExpr = SafeClone ( m_dExtraAttrs[i].m_pExpr );

	return pClone;
}



/////////////////////////////////////////////////////////////////////////////
// CHUNK READER
/////////////////////////////////////////////////////////////////////////////

CSphBin::CSphBin ( ESphHitless eMode, bool bWordDict )
	: m_eMode ( eMode )
	, m_bWordDict ( bWordDict )
{
	m_tHit.m_sKeyword = bWordDict ? m_sKeyword : nullptr;
	m_sKeyword[0] = '\0';

#ifndef NDEBUG
	m_sLastKeyword[0] = '\0';
#endif
}


int CSphBin::CalcBinSize ( int iMemoryLimit, int iBlocks, const char * sPhase, bool bWarn )
{
	if ( iBlocks<=0 )
		return CSphBin::MIN_SIZE;

	int iBinSize = ( ( iMemoryLimit/iBlocks + 2048 ) >> 12 ) << 12; // round to 4k

	if ( iBinSize<CSphBin::MIN_SIZE )
	{
		iBinSize = CSphBin::MIN_SIZE;
		sphWarn ( "%s: mem_limit=%d kb extremely low, increasing to %d kb",
			sPhase, iMemoryLimit/1024, iBinSize*iBlocks/1024 );
	}

	if ( iBinSize<CSphBin::WARN_SIZE && bWarn )
	{
		sphWarn ( "%s: merge_block_size=%d kb too low, increasing mem_limit may improve performance",
			sPhase, iBinSize/1024 );
	}

	return iBinSize;
}


void CSphBin::Init ( int iFD, SphOffset_t * pSharedOffset, const int iBinSize )
{
	assert ( !m_dBuffer );
	assert ( iBinSize>=MIN_SIZE );
	assert ( pSharedOffset );

	m_iFile = iFD;
	m_pFilePos = pSharedOffset;

	m_iSize = iBinSize;
	m_dBuffer = new BYTE [ iBinSize ];
	m_pCurrent = m_dBuffer;

	m_tHit.m_tRowID = INVALID_ROWID;
	m_tHit.m_uWordID = 0;
	m_tHit.m_iWordPos = EMPTY_HIT;
	m_tHit.m_dFieldMask.UnsetAll();

	m_bError = false;
}


CSphBin::~CSphBin ()
{
	SafeDeleteArray ( m_dBuffer );
}


int CSphBin::ReadByte ()
{
	BYTE r;

	if ( !m_iLeft )
	{
		if ( *m_pFilePos!=m_iFilePos )
		{
			if ( !SeekAndWarn ( m_iFile, m_iFilePos, "CSphBin::ReadBytes" ) )
			{
				m_bError = true;
				return BIN_READ_ERROR;
			}
			*m_pFilePos = m_iFilePos;
		}

		int n = m_iFileLeft > m_iSize
			? m_iSize
			: (int)m_iFileLeft;
		if ( n==0 )
		{
			m_iDone = 1;
			m_iLeft = 1;
		} else
		{
			assert ( m_dBuffer );

			if ( sphReadThrottled ( m_iFile, m_dBuffer, n )!=(size_t)n )
			{
				m_bError = true;
				return -2;
			}
			m_iLeft = n;

			m_iFilePos += n;
			m_iFileLeft -= n;
			m_pCurrent = m_dBuffer;
			*m_pFilePos += n;
		}
	}
	if ( m_iDone )
	{
		m_bError = true; // unexpected (!) eof
		return -1;
	}

	m_iLeft--;
	r = *(m_pCurrent);
	m_pCurrent++;
	return r;
}


ESphBinRead CSphBin::ReadBytes ( void * pDest, int iBytes )
{
	assert ( iBytes>0 );
	assert ( iBytes<=m_iSize );

	if ( m_iDone )
		return BIN_READ_EOF;

	if ( m_iLeft<iBytes )
	{
		if ( *m_pFilePos!=m_iFilePos )
		{
			if ( !SeekAndWarn ( m_iFile, m_iFilePos, "CSphBin::ReadBytes" ))
			{
				m_bError = true;
				return BIN_READ_ERROR;
			}
			*m_pFilePos = m_iFilePos;
		}

		int n = Min ( m_iFileLeft, m_iSize - m_iLeft );
		if ( n==0 )
		{
			m_iDone = 1;
			m_bError = true; // unexpected (!) eof
			return BIN_READ_EOF;
		}

		assert ( m_dBuffer );
		memmove ( m_dBuffer, m_pCurrent, m_iLeft );

		if ( sphReadThrottled ( m_iFile, m_dBuffer + m_iLeft, n )!=(size_t)n )
		{
			m_bError = true;
			return BIN_READ_ERROR;
		}

		m_iLeft += n;
		m_iFilePos += n;
		m_iFileLeft -= n;
		m_pCurrent = m_dBuffer;
		*m_pFilePos += n;
	}

	assert ( m_iLeft>=iBytes );
	m_iLeft -= iBytes;

	memcpy ( pDest, m_pCurrent, iBytes );
	m_pCurrent += iBytes;

	return BIN_READ_OK;
}


SphWordID_t CSphBin::ReadVLB ()
{
	SphWordID_t uValue = 0;
	int iByte, iOffset = 0;
	do
	{
		if ( ( iByte = ReadByte() )<0 )
			return 0;
		uValue += ( ( SphWordID_t ( iByte & 0x7f ) ) << iOffset );
		iOffset += 7;
	}
	while ( iByte & 0x80 );
	return uValue;
}

DWORD CSphBin::UnzipInt ()
{
	register int b = 0;
	register DWORD v = 0;
	do
	{
		b = ReadByte();
		if ( b<0 )
			b = 0;
		v = ( v<<7 ) + ( b & 0x7f );
	} while ( b & 0x80 );
	return v;
}

SphOffset_t CSphBin::UnzipOffset ()
{
	register int b = 0;
	register SphOffset_t v = 0;
	do
	{
		b = ReadByte();
		if ( b<0 )
			b = 0;
		v = ( v<<7 ) + ( b & 0x7f );
	} while ( b & 0x80 );
	return v;
}

int CSphBin::ReadHit ( CSphAggregateHit * pOut )
{
	// expected EOB
	if ( m_iDone )
	{
		pOut->m_uWordID = 0;
		return 1;
	}

	CSphAggregateHit & tHit = m_tHit; // shortcut
	while (true)
	{
		// SPH_MAX_WORD_LEN is now 42 only to keep ReadVLB() below
		// technically, we can just use different functions on different paths, if ever needed
		STATIC_ASSERT ( SPH_MAX_WORD_LEN*3<=127, KEYWORD_TOO_LONG );
		SphWordID_t uDelta = ReadVLB();

		if ( uDelta )
		{
			switch ( m_eState )
			{
				case BIN_WORD:
					if ( m_bWordDict )
					{
#ifdef NDEBUG
						// FIXME?! move this under PARANOID or something?
						// or just introduce an assert() checked release build?
						if ( uDelta>=sizeof(m_sKeyword) )
							sphDie ( "INTERNAL ERROR: corrupted keyword length (len=" UINT64_FMT ", deltapos=" UINT64_FMT ")",
								(uint64_t)uDelta, (uint64_t)(m_iFilePos-m_iLeft) );
#else
						assert ( uDelta>0 && uDelta<sizeof(m_sKeyword)-1 );
#endif

						ReadBytes ( m_sKeyword, (int)uDelta );
						m_sKeyword[uDelta] = '\0';
						tHit.m_uWordID = sphCRC32 ( m_sKeyword ); // must be in sync with dict!

#ifndef NDEBUG
						assert ( ( m_iLastWordID<tHit.m_uWordID )
							|| ( m_iLastWordID==tHit.m_uWordID && strcmp ( (char*)m_sLastKeyword, (char*)m_sKeyword )<0 ) );
						strncpy ( (char*)m_sLastKeyword, (char*)m_sKeyword, sizeof(m_sLastKeyword) );
#endif

					} else
						tHit.m_uWordID += uDelta;

					tHit.m_tRowID = INVALID_ROWID;
					tHit.m_iWordPos = EMPTY_HIT;
					tHit.m_dFieldMask.UnsetAll();
					m_eState = BIN_DOC;
					break;

				case BIN_DOC:
					// doc id
					m_eState = BIN_POS;
					tHit.m_tRowID += uDelta;
					tHit.m_iWordPos = EMPTY_HIT;
					break;

				case BIN_POS:
					if ( m_eMode==SPH_HITLESS_ALL )
					{
						tHit.m_dFieldMask.Assign32 ( (DWORD)ReadVLB() );
						m_eState = BIN_DOC;

					} else if ( m_eMode==SPH_HITLESS_SOME )
					{
						if ( uDelta & 1 )
						{
							tHit.m_dFieldMask.Assign32 ( (DWORD)ReadVLB() );
							m_eState = BIN_DOC;
						}
						uDelta >>= 1;
					}
					tHit.m_iWordPos += (DWORD)uDelta;
					*pOut = tHit;
					return 1;

				default:
					sphDie ( "INTERNAL ERROR: unknown bin state (state=%d)", m_eState );
			}
		} else
		{
			switch ( m_eState )
			{
				case BIN_POS:	m_eState = BIN_DOC; break;
				case BIN_DOC:	m_eState = BIN_WORD; break;
				case BIN_WORD:	m_iDone = 1; pOut->m_uWordID = 0; return 1;
				default:		sphDie ( "INTERNAL ERROR: unknown bin state (state=%d)", m_eState );
			}
		}
	}
}


bool CSphBin::IsEOF () const
{
	return m_iDone!=0 || m_iFileLeft<=0;
}


bool CSphBin::IsDone () const
{
	return m_iDone!=0 || ( m_iFileLeft<=0 && m_iLeft<=0 );
}


ESphBinRead CSphBin::Precache ()
{
	if ( m_iFileLeft > m_iSize-m_iLeft )
	{
		m_bError = true;
		return BIN_PRECACHE_ERROR;
	}

	if ( !m_iFileLeft )
		return BIN_PRECACHE_OK;

	if ( *m_pFilePos!=m_iFilePos )
	{
		if ( !SeekAndWarn ( m_iFile, m_iFilePos, "CSphBin::Precache" ))
		{
			m_bError = true;
			return BIN_PRECACHE_ERROR;
		}

		*m_pFilePos = m_iFilePos;
	}

	assert ( m_dBuffer );
	memmove ( m_dBuffer, m_pCurrent, m_iLeft );

	if ( sphReadThrottled ( m_iFile, m_dBuffer+m_iLeft, m_iFileLeft )!=(size_t)m_iFileLeft )
	{
		m_bError = true;
		return BIN_READ_ERROR;
	}

	m_iLeft += m_iFileLeft;
	m_iFilePos += m_iFileLeft;
	m_iFileLeft -= m_iFileLeft;
	m_pCurrent = m_dBuffer;
	*m_pFilePos += m_iFileLeft;

	return BIN_PRECACHE_OK;
}


CSphMultiQueryArgs::CSphMultiQueryArgs ( int iIndexWeight )
	: m_iIndexWeight ( iIndexWeight )
{
	assert ( iIndexWeight>0 );
}


/////////////////////////////////////////////////////////////////////////////
// INDEX
/////////////////////////////////////////////////////////////////////////////

std::atomic<long> CSphIndex::m_tIdGenerator {0};

CSphIndex::CSphIndex ( const char * sIndexName, const char * sFilename )
	: m_tSchema ( sFilename )
	, m_sIndexName ( sIndexName )
	, m_sFilename ( sFilename )
{
	m_iIndexId = m_tIdGenerator.fetch_add ( 1, std::memory_order_relaxed );
	m_tMutableSettings = MutableIndexSettings_c::GetDefaults();
}


CSphIndex::~CSphIndex ()
{
	QcacheDeleteIndex ( m_iIndexId );
}


void CSphIndex::SetInplaceSettings ( int iHitGap, float fRelocFactor, float fWriteFactor )
{
	m_iHitGap = iHitGap;
	m_fRelocFactor = fRelocFactor;
	m_fWriteFactor = fWriteFactor;
	m_bInplaceSettings = true;
}


void CSphIndex::SetFieldFilter ( ISphFieldFilter * pFieldFilter )
{
	SafeAddRef ( pFieldFilter );
	m_pFieldFilter = pFieldFilter;
}


void CSphIndex::SetTokenizer ( ISphTokenizer * pTokenizer )
{
	SafeAddRef ( pTokenizer );
	m_pTokenizer = pTokenizer;
}


void CSphIndex::SetupQueryTokenizer()
{
	bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	// create and setup a master copy of query time tokenizer
	// that we can then use to create lightweight clones
	m_pQueryTokenizer = m_pTokenizer->Clone ( SPH_CLONE_QUERY );
	sphSetupQueryTokenizer ( m_pQueryTokenizer, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, false );

	m_pQueryTokenizerJson = m_pTokenizer->Clone ( SPH_CLONE_QUERY );
	sphSetupQueryTokenizer ( m_pQueryTokenizerJson, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, true );
}


ISphTokenizer *	CSphIndex::LeakTokenizer ()
{
	return m_pTokenizer.Leak();
}


void CSphIndex::SetDictionary ( CSphDict * pDict )
{
	SafeAddRef ( pDict );
	m_pDict = pDict;
}


CSphDict * CSphIndex::LeakDictionary ()
{
	return m_pDict.Leak();
}


void CSphIndex::Setup ( const CSphIndexSettings & tSettings )
{
	m_bStripperInited = true;
	m_tSettings = tSettings;
}

void CSphIndex::SetCacheSize ( int iMaxCachedDocs, int iMaxCachedHits )
{
	m_iMaxCachedDocs = iMaxCachedDocs;
	m_iMaxCachedHits = iMaxCachedHits;
}


float CSphIndex::GetGlobalIDF ( const CSphString & sWord, int64_t iDocsLocal, bool bPlainIDF ) const
{
	auto pIDFer = sph::GetIDFer ( m_sGlobalIDFPath );
	if ( !pIDFer )
		return 0.0;
	return pIDFer->GetIDF ( sWord, iDocsLocal, bPlainIDF );
}


int CSphIndex::UpdateAttributes ( AttrUpdateSharedPtr_t pUpd, bool & bCritical, CSphString & sError, CSphString & sWarning )
{
	AttrUpdateInc_t tUpdInc { std::move (pUpd) };
	return UpdateAttributes ( tUpdInc, bCritical, sError, sWarning );
}

CSphVector<SphAttr_t> CSphIndex::BuildDocList () const
{
	TlsMsg::Err(); // reset error
	return CSphVector<SphAttr_t>();
}

void CSphIndex::GetFieldFilterSettings ( CSphFieldFilterSettings & tSettings ) const
{
	if ( m_pFieldFilter )
		m_pFieldFilter->GetSettings ( tSettings );
}


void CSphIndex::SetMutableSettings ( const MutableIndexSettings_c & tSettings )
{
	m_tMutableSettings = tSettings;
}

//////////////////////////////////////////////////////////////////////////

static void PooledAttrsToPtrAttrs ( const VecTraits_T<ISphMatchSorter *> & dSorters, const BYTE * pBlobPool, columnar::Columnar_i * pColumnar )
{
	dSorters.Apply ( [&] ( ISphMatchSorter * p )
	{
		if ( p )
			p->TransformPooled2StandalonePtrs ( [pBlobPool] ( const CSphMatch * ) { return pBlobPool; }, [pColumnar] ( const CSphMatch * ) { return pColumnar; } );
	});
}


CSphIndex * sphCreateIndexPhrase ( const char* szIndexName, const char * sFilename )
{
	return new CSphIndex_VLN ( szIndexName, sFilename );
}

//////////////////////////////////////////////////////////////////////////


CSphIndex_VLN::CSphIndex_VLN ( const char * szIndexName, const char * szFilename )
	: CSphIndex ( szIndexName, szFilename )
	, m_iLockFD ( -1 )
	, m_dFieldLens ( SPH_MAX_FIELDS )
{
	m_sFilename = szFilename;

	m_iDocinfo = 0;
	m_iDocinfoIndex = 0;
	m_pDocinfoIndex = NULL;

	m_uVersion = INDEX_FORMAT_VERSION;
	m_bPassedRead = false;
	m_bPassedAlloc = false;
	m_bIsEmpty = true;
	m_bDebugCheck = false;
	m_uAttrsStatus = 0;

	m_iMinMaxIndex = 0;

	m_dFieldLens.ZeroVec();
}


CSphIndex_VLN::~CSphIndex_VLN ()
{
	SafeDelete ( m_pHistograms );
	Unlock();
}

class DocIdIndexReader_c
{
public:
	DocIdIndexReader_c ( const VecTraits_T<int>& dIdx, const VecTraits_T<DocID_t>& dDocids )
		: m_pCur ( dIdx.begin() )
		, m_pEnd ( dIdx.end() )
		, m_dDocids ( dDocids )
	{}

	inline bool ReadDocID ( DocID_t & tDocID )
	{
		if ( m_pCur>=m_pEnd )
			return false;

		tDocID = m_dDocids[*m_pCur];
		++m_pCur;
		return true;
	}

	int GetIndex() const
	{
		return *( m_pCur-1 );
	}

private:
	const int * m_pCur;
	const int * m_pEnd;
	const VecTraits_T<DocID_t> & m_dDocids;
};

template <typename FUNCTOR>
void Intersect ( LookupReaderIterator_c& tReader1, DocIdIndexReader_c & tReader2, FUNCTOR && tFunctor )
{
	RowID_t tRowID1 = INVALID_ROWID;
	DocID_t tDocID1 = 0, tDocID2 = 0;
	bool bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
	bool bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );

	while ( bHaveDocs1 && bHaveDocs2 )
	{
		if ( tDocID1 < tDocID2 )
		{
			tReader1.HintDocID ( tDocID2 );
			bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
		}
		else if (  tDocID1 > tDocID2 )
			bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );
		else
		{
			tFunctor ( tRowID1, tReader2.GetIndex () );
			bHaveDocs1 = tReader1.Read ( tDocID1, tRowID1 );
			bHaveDocs2 = tReader2.ReadDocID ( tDocID2 );
		}
	}
}

// fill collect rows which will be updated in this index
RowsToUpdateData_t CSphIndex_VLN::Update_CollectRowPtrs ( const UpdateContext_t & tCtx )
{
	RowsToUpdateData_t dRowsToUpdate;
	const auto & dDocids = tCtx.m_tUpd.m_pUpdate->m_dDocids;

	// collect idxes of alive (not-yet-updated) rows
	CSphVector<int> dSorted;
	dSorted.Reserve ( dDocids.GetLength() - tCtx.m_tUpd.m_iAffected );
	ARRAY_CONSTFOREACH (i, dDocids)
		if ( !tCtx.m_tUpd.m_dUpdated.BitGet ( i ) )
			dSorted.Add ( i );

	if ( dSorted.IsEmpty () )
		return dRowsToUpdate;

	dSorted.Sort ( Lesser ( [&dDocids] ( int a, int b ) { return dDocids[a]<dDocids[b]; } ) );
	DocIdIndexReader_c tSortedReader ( dSorted, dDocids );
	LookupReaderIterator_c tLookupReader ( m_tDocidLookup.GetReadPtr() );
	Intersect ( tLookupReader, tSortedReader, [&dRowsToUpdate, this] ( RowID_t tRowID, int iIdx )
	{
		auto& dUpd = dRowsToUpdate.Add();
		dUpd.m_pRow = GetDocinfoByRowID ( tRowID );
		dUpd.m_iIdx = iIdx;
		assert ( dUpd.m_pRow );
	} );
	return dRowsToUpdate;
}

// We fill docinfo ptr for actual rows, and move out non-actual (the ones which doesn't point to existing document)
// note, it actually changes (rearranges) rows!
RowsToUpdate_t CSphIndex_VLN::Update_PrepareGatheredRowPtrs ( RowsToUpdate_t & dWRows, const VecTraits_T<DocID_t>& dDocids )
{
	RowsToUpdate_t dRows = dWRows; // that is actually to indicate that we CHANGE contents inside dWRows, so it should be passed by non-const reference.

	dRows.Sort ( Lesser ( [&dDocids] ( auto& a, auto& b ) { return dDocids[a.m_iIdx]<dDocids[b.m_iIdx]; } ) );
	LookupReaderIterator_c tLookupReader ( m_tDocidLookup.GetReadPtr() );

	RowID_t tRowID = INVALID_ROWID;
	DocID_t tDocID = 0;
	bool bHaveDocs = tLookupReader.Read ( tDocID, tRowID );
	bool bHaveDocsToUpdate = !dRows.IsEmpty();
	DocID_t tDocIDPrepared = bHaveDocsToUpdate ? dDocids[dRows[0].m_iIdx] : 0;
	int iReadIdx = 0;
	int iWriteIdx = 0;

	while ( bHaveDocs && bHaveDocsToUpdate )
	{
		if ( tDocID < tDocIDPrepared )
		{
			tLookupReader.HintDocID ( tDocIDPrepared );
			bHaveDocs = tLookupReader.Read ( tDocID, tRowID );
			continue;
		} else if ( tDocID == tDocIDPrepared )
		{
			dRows[iWriteIdx].m_pRow = GetDocinfoByRowID ( tRowID );
			assert ( dRows[iWriteIdx].m_pRow );
			Swap ( dRows[iWriteIdx].m_iIdx, dRows[iReadIdx].m_iIdx );
			bHaveDocs = tLookupReader.Read ( tDocID, tRowID );
			++iWriteIdx;
		}

		++iReadIdx;
		bHaveDocsToUpdate = iReadIdx < dRows.GetLength();
		if ( bHaveDocsToUpdate )
			tDocIDPrepared = dDocids[dRows[iReadIdx].m_iIdx];
	}
	return dWRows.Slice ( 0, iWriteIdx );
}


bool CSphIndex_VLN::Update_WriteBlobRow ( UpdateContext_t & tCtx, CSphRowitem * pDocinfo, const BYTE * pBlob,
		int iLength, int nBlobAttrs, const CSphAttrLocator & tBlobRowLoc, bool & bCritical, CSphString & sError )
{
	BYTE * pExistingBlob = m_tBlobAttrs.GetWritePtr() + sphGetRowAttr ( pDocinfo, tBlobRowLoc );
	DWORD uExistingBlobLen = sphGetBlobTotalLen ( pExistingBlob, nBlobAttrs );

	bCritical = false;

	// overwrite old record (because we have the write lock)
	if ( (DWORD)iLength<=uExistingBlobLen )
	{
		memcpy ( pExistingBlob, pBlob, iLength );
		return true;
	}

	BYTE * pOldBlobPool = m_tBlobAttrs.GetWritePtr();
	SphOffset_t tBlobSpaceUsed = *(SphOffset_t*)pOldBlobPool;
	SphOffset_t tSpaceLeft = m_tBlobAttrs.GetLengthBytes()-tBlobSpaceUsed;

	// not great: we have to resize our .spb file and create new memory maps
	if ( (SphOffset_t)iLength > tSpaceLeft )
	{
		SphOffset_t tSizeDelta = Max ( (SphOffset_t)iLength-tSpaceLeft, m_tSettings.m_tBlobUpdateSpace );
		CSphString sWarning;
		size_t tOldSize = m_tBlobAttrs.GetLengthBytes();
		if ( !m_tBlobAttrs.Resize ( tOldSize + tSizeDelta, sWarning, sError ) )
		{
			// try to map again, using old size
			if ( !m_tBlobAttrs.Resize ( tOldSize, sWarning, sError ) )
				bCritical = true;	// real bad, index unusable

			sError = "unable to resize .SPB file";
			return false;
		}

		// update blob pool ptr since it might be changed after resize
		tCtx.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	}

	BYTE * pEnd = m_tBlobAttrs.GetWritePtr() + tBlobSpaceUsed;
	memcpy ( pEnd, pBlob, iLength );

	sphSetRowAttr ( pDocinfo, tBlobRowLoc, tBlobSpaceUsed );

	tBlobSpaceUsed += iLength;

	*(SphOffset_t*)m_tBlobAttrs.GetWritePtr() = tBlobSpaceUsed;

	return true;
}


void CSphIndex_VLN::Update_MinMax ( const RowsToUpdate_t& dRows, const UpdateContext_t & tCtx )
{
	int iRowStride = tCtx.m_tSchema.GetRowSize();

	for ( const auto & tRow : dRows )
	{
		int64_t iBlock = int64_t ( tRow.m_pRow-tCtx.m_pAttrPool ) / ( iRowStride*DOCINFO_INDEX_FREQ );
		DWORD * pBlockRanges = m_pDocinfoIndex + ( iBlock * iRowStride * 2 );
		DWORD * pIndexRanges = m_pDocinfoIndex + ( m_iDocinfoIndex * iRowStride * 2 );
		assert ( iBlock>=0 && iBlock<m_iDocinfoIndex );

		ARRAY_CONSTFOREACH ( iCol, tCtx.m_tUpd.m_pUpdate->m_dAttributes )
		{
			const UpdatedAttribute_t & tUpdAttr = tCtx.m_dUpdatedAttrs[iCol];
			if ( !tUpdAttr.m_bExisting )
				continue;

			const CSphAttrLocator & tLoc = tUpdAttr.m_tLocator;
			if ( tLoc.IsBlobAttr() )
				continue;

			SphAttr_t uValue = sphGetRowAttr ( tRow.m_pRow, tLoc );

			// update block and index ranges
			for ( int i=0; i<2; i++ )
			{
				DWORD * pBlock = i ? pBlockRanges : pIndexRanges;
				SphAttr_t uMin = sphGetRowAttr ( pBlock, tLoc );
				SphAttr_t uMax = sphGetRowAttr ( pBlock+iRowStride, tLoc );
				if ( tUpdAttr.m_eAttrType==SPH_ATTR_FLOAT ) // update float's indexes assumes float comparision
				{
					float fValue = sphDW2F ( (DWORD) uValue );
					float fMin = sphDW2F ( (DWORD) uMin );
					float fMax = sphDW2F ( (DWORD) uMax );
					if ( fValue<fMin )
						sphSetRowAttr ( pBlock, tLoc, sphF2DW ( fValue ) );
					if ( fValue>fMax )
						sphSetRowAttr ( pBlock+iRowStride, tLoc, sphF2DW ( fValue ) );
				} else // update usual integers
				{
					if ( uValue<uMin )
						sphSetRowAttr ( pBlock, tLoc, uValue );
					if ( uValue>uMax )
						sphSetRowAttr ( pBlock+iRowStride, tLoc, uValue );
				}
			}
		}
	}
}

bool CSphIndex_VLN::DoUpdateAttributes ( const RowsToUpdate_t& dRows, UpdateContext_t& tCtx, bool& bCritical, CSphString& sError )
{
	if ( dRows.IsEmpty() )
		return true;

	tCtx.m_pHistograms = m_pHistograms;
	tCtx.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	tCtx.m_pAttrPool = m_tAttr.GetWritePtr ();
	tCtx.m_pSegment = this;

	if ( !Update_CheckAttributes ( *tCtx.m_tUpd.m_pUpdate, tCtx.m_tSchema, sError ) )
		return false;

	Update_PrepareListOfUpdatedAttributes ( tCtx, sError );

	// FIXME! FIXME! FIXME! overwriting just-freed blocks might hurt concurrent searchers;
	// should implement a simplistic MVCC-style delayed-free to avoid that

	// first pass, if needed
	if ( tCtx.m_tUpd.m_pUpdate->m_bStrict )
		if ( !Update_InplaceJson ( dRows, tCtx, sError, true ) )
			return false;

	// second pass
	tCtx.m_iJsonWarnings = 0;
	Update_InplaceJson ( dRows, tCtx, sError, false );

	if ( !Update_Blobs ( dRows, tCtx, bCritical, sError ) )
		return false;

	Update_Plain ( dRows, tCtx );
	Update_MinMax ( dRows, tCtx );
	return true;
}

int CSphIndex_VLN::UpdateAttributes ( AttrUpdateInc_t & tUpd, bool & bCritical, CSphString & sError, CSphString & sWarning )
{
	assert ( tUpd.m_pUpdate->m_dRowOffset.IsEmpty() || tUpd.m_pUpdate->m_dDocids.GetLength()==tUpd.m_pUpdate->m_dRowOffset.GetLength() );

	// check if we have to
	if ( !m_iDocinfo || tUpd.m_pUpdate->m_dDocids.IsEmpty() )
		return 0;

	UpdateContext_t tCtx ( tUpd, m_tSchema );
	int iUpdated = tUpd.m_iAffected;

	auto dRowsToUpdate = Update_CollectRowPtrs ( tCtx );
	if ( !DoUpdateAttributes ( dRowsToUpdate, tCtx, bCritical, sError ))
		return -1;

	iUpdated = tUpd.m_iAffected - iUpdated;

	if ( !Update_HandleJsonWarnings ( tCtx, iUpdated, sWarning, sError ) )
		return -1;

	if ( tCtx.m_uUpdateMask && m_bBinlog )
		Binlog::CommitUpdateAttributes ( &m_iTID, m_sIndexName.cstr(), *tUpd.m_pUpdate );

	m_uAttrsStatus |= tCtx.m_uUpdateMask; // FIXME! add lock/atomic?

	if ( m_bAttrsBusy.load ( std::memory_order_acquire ) )
	{
		auto& tNewUpdate = m_dPostponedUpdates.Add();
		tNewUpdate.m_pUpdate = MakeReusableUpdate(tUpd.m_pUpdate);
		tNewUpdate.m_dRowsToUpdate.SwapData ( dRowsToUpdate );
	}

	return iUpdated;
}

void CSphIndex_VLN::UpdateAttributesOffline ( VecTraits_T<PostponedUpdate_t> & dUpdates, IndexSegment_c *)
{
	if ( dUpdates.IsEmpty () )
		return;

	CSphString sError;
	bool bCritical;

	for ( auto & tUpdate : dUpdates )
	{
		AttrUpdateInc_t tUpdInc { tUpdate.m_pUpdate }; // dont move, keep update (need twice when split chunks)
		UpdateContext_t tCtx ( tUpdInc, m_tSchema );
		RowsToUpdate_t dRows = Update_PrepareGatheredRowPtrs ( tUpdate.m_dRowsToUpdate, tCtx.m_tUpd.m_pUpdate->m_dDocids );
		if ( !DoUpdateAttributes ( dRows, tCtx, bCritical, sError ) )
		{
			sphWarning ("UpdateAttributesOffline: %s", sError.cstr() );
			break;
		}
		m_uAttrsStatus |= tCtx.m_uUpdateMask; // FIXME! add lock/atomic?
	}
}

// safely rename an index file
bool CSphIndex_VLN::JuggleFile ( ESphExt eExt, CSphString & sError, bool bNeedSrc, bool bNeedDst ) const
{
	CSphString sExt = GetIndexFileName ( eExt );
	CSphString sExtNew = GetIndexFileName ( eExt, true );
	CSphString sExtOld;
	sExtOld.SetSprintf ( "%s.old", sExt.cstr() );

	if ( sph::rename ( sExt.cstr(), sExtOld.cstr() ) )
	{
		if ( bNeedSrc )
		{
			sError.SetSprintf ( "rename '%s' to '%s' failed: %s", sExt.cstr(), sExtOld.cstr(), strerror(errno) );
			return false;
		}
	}

	if ( sph::rename ( sExtNew.cstr(), sExt.cstr() ) )
	{
		if ( bNeedDst )
		{
			if ( bNeedSrc && !sph::rename ( sExtOld.cstr(), sExt.cstr() ) )
			{
				// rollback failed too!
				sError.SetSprintf ( "rollback rename to '%s' failed: %s; INDEX UNUSABLE; FIX FILE NAMES MANUALLY", sExt.cstr(), strerror(errno) );
			} else
			{
				// rollback went ok
				sError.SetSprintf ( "rename '%s' to '%s' failed: %s", sExtNew.cstr(), sExt.cstr(), strerror(errno) );
			}
			return false;
		}
	}

	// all done
	::unlink ( sExtOld.cstr() );
	return true;
}

bool CSphIndex_VLN::SaveAttributes ( CSphString & sError ) const
{
	if ( !m_uAttrsStatus || !m_iDocinfo )
		return true;

	DWORD uAttrStatus = m_uAttrsStatus;

	sphLogDebugvv ( "index '%s' attrs (%u) saving...", m_sIndexName.cstr(), uAttrStatus );

	if ( uAttrStatus & IndexUpdateHelper_c::ATTRS_UPDATED )
	{
		if ( !m_tAttr.Flush ( true, sError ) )
			return false;

		if ( m_pHistograms && !m_pHistograms->Save ( GetIndexFileName(SPH_EXT_SPHI), sError ) )
			return false;
	}

	if ( uAttrStatus & IndexUpdateHelper_c::ATTRS_BLOB_UPDATED )
	{
		if ( !m_tBlobAttrs.Flush ( true, sError ) )
			return false;
	}

	if ( uAttrStatus & IndexUpdateHelper_c::ATTRS_ROWMAP_UPDATED )
	{
		if ( !m_tDeadRowMap.Flush ( true, sError ) )
			return false;
	}

	if ( m_bBinlog )
		Binlog::NotifyIndexFlush ( m_sIndexName.cstr(), m_iTID, false );

	if ( m_uAttrsStatus==uAttrStatus )
		m_uAttrsStatus = 0;

	sphLogDebugvv ( "index '%s' attrs (%u) saved", m_sIndexName.cstr(), m_uAttrsStatus );

	return true;
}

DWORD CSphIndex_VLN::GetAttributeStatus () const
{
	return m_uAttrsStatus;
}


//////////////////////////////////////////////////////////////////////////

struct CmpDocidLookup_fn
{
	static inline bool IsLess ( const DocidRowidPair_t & a, const DocidRowidPair_t & b )
	{
		if ( a.m_tDocID==b.m_tDocID )
			return a.m_tRowID < b.m_tRowID;

		return a.m_tDocID < b.m_tDocID;
	}
};


struct CmpQueuedLookup_fn
{
	static DocidRowidPair_t * m_pStorage;

	static inline bool IsLess ( const int a, const int b )
	{
		if ( m_pStorage[a].m_tDocID==m_pStorage[b].m_tDocID )
			return m_pStorage[a].m_tRowID < m_pStorage[b].m_tRowID;

		return m_pStorage[a].m_tDocID < m_pStorage[b].m_tDocID;
	}
};

DocidRowidPair_t * CmpQueuedLookup_fn::m_pStorage = nullptr;

bool CSphIndex_VLN::Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const
{
	return pDocinfo-m_tAttr.GetWritePtr() >= m_iDocinfo*iStride;
}


bool CSphIndex_VLN::AddRemoveColumnarAttr ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, const ISphSchema & tOldSchema, const ISphSchema & tNewSchema, CSphString & sError )
{
	CSphScopedPtr<columnar::Builder_i> pBuilder ( CreateColumnarBuilder ( tNewSchema, m_tSettings, GetIndexFileName ( SPH_EXT_SPC, true ), sError ) );
	if ( !pBuilder )
		return false;

	if ( !Alter_AddRemoveColumnar ( bAddAttr, m_tSchema, tNewSchema, m_pColumnar.Ptr(), pBuilder.Ptr(), (DWORD)m_iDocinfo, m_sIndexName, sError ) )
		return false;

	return true;
}


bool CSphIndex_VLN::AddRemoveAttribute ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, AttrEngine_e eEngine, CSphString & sError )
{
	// combine per-index and per-attribute engine settings
	AttrEngine_e eAttrEngine = m_tSettings.m_eEngine;
	if ( eEngine!=AttrEngine_e::DEFAULT )
		eAttrEngine = eEngine;

	CSphSchema tNewSchema = m_tSchema;
	if ( !Alter_AddRemoveFromSchema ( tNewSchema, sAttrName, eAttrType, eAttrEngine, bAddAttr, sError ) )
		return false;

	int iNewStride = tNewSchema.GetRowSize();

	int64_t iNewMinMaxIndex = m_iDocinfo * iNewStride;

	BuildHeader_t tBuildHeader ( m_tStats );
	tBuildHeader.m_iMinMaxIndex = iNewMinMaxIndex;

	*(DictHeader_t*)&tBuildHeader = *(DictHeader_t*)&m_tWordlist;

	CSphString sHeaderName = GetIndexFileName ( SPH_EXT_SPH, true );
	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &m_tSettings;
	tWriteHeader.m_pSchema = &tNewSchema;
	tWriteHeader.m_pTokenizer = m_pTokenizer;
	tWriteHeader.m_pDict = m_pDict;
	tWriteHeader.m_pFieldFilter = m_pFieldFilter;
	tWriteHeader.m_pFieldLens = m_dFieldLens.Begin();

	// save the header
	if ( !IndexBuildDone ( tBuildHeader, tWriteHeader, sHeaderName, sError ) )
		return false;

	// generate new .SPA, .SPB files
	WriterWithHash_c tSPAWriter ( "spa", &m_dHashes );
	WriterWithHash_c tSPBWriter ( "spb", &m_dHashes );
	tSPAWriter.SetBufferSize ( 524288 );
	tSPBWriter.SetBufferSize ( 524288 );

	CSphScopedPtr<WriteWrapper_c> pSPAWriteWrapper ( CreateWriteWrapperDisk(tSPAWriter) );
	CSphScopedPtr<WriteWrapper_c> pSPBWriteWrapper ( CreateWriteWrapperDisk(tSPBWriter) );

	CSphString sSPAfile = GetIndexFileName ( SPH_EXT_SPA, true );
	CSphString sSPBfile = GetIndexFileName ( SPH_EXT_SPB, true );
	CSphString sSPHIfile = GetIndexFileName ( SPH_EXT_SPHI, true );
	if ( !tSPAWriter.OpenFile ( sSPAfile, sError ) )
		return false;

	bool bHadBlobs = false;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		bHadBlobs |= sphIsBlobAttr ( m_tSchema.GetAttr(i) );

	bool bHaveBlobs = false;
	for ( int i = 0; i < tNewSchema.GetAttrsCount(); i++ )
		bHaveBlobs |= sphIsBlobAttr ( tNewSchema.GetAttr(i) );

	bool bBlob = sphIsBlobAttr ( eAttrType );
	bool bBlobsModified = bBlob && ( bAddAttr || bHaveBlobs==bHadBlobs );
	if ( bBlobsModified )
	{
		if ( !tSPBWriter.OpenFile ( sSPBfile, sError ) )
			return false;

		tSPBWriter.PutOffset(0);
	}

	if ( !tNewSchema.GetAttrsCount() )
	{
		sError = "index must have at least one attribute";
		return false;
	}

	bool bColumnar = bAddAttr ? tNewSchema.GetAttr ( sAttrName.cstr() )->IsColumnar() : m_tSchema.GetAttr ( sAttrName.cstr() )->IsColumnar();
	if ( bColumnar )
		AddRemoveColumnarAttr ( bAddAttr, sAttrName, eAttrType, m_tSchema, tNewSchema, sError );
	else
	{
		int64_t iTotalRows = m_iDocinfo + (m_iDocinfoIndex+1)*2;
		Alter_AddRemoveRowwiseAttr ( m_tSchema, tNewSchema, m_tAttr.GetWritePtr(), (DWORD)iTotalRows, m_tBlobAttrs.GetWritePtr(), *pSPAWriteWrapper, *pSPBWriteWrapper, bAddAttr, sAttrName );
	}

	if ( m_pHistograms )
	{
		if ( bAddAttr )
		{
			Histogram_i * pNewHistogram = CreateHistogram ( sAttrName, eAttrType );
			if ( pNewHistogram )
			{
				for ( DWORD i = 0; i < m_iDocinfo; i++ )
					pNewHistogram->Insert(0);

				m_pHistograms->Add ( pNewHistogram );
			}
		}
		else
			m_pHistograms->Remove ( sAttrName );

		if ( !m_pHistograms->Save ( sSPHIfile, sError ) )
			return false;
	}

	if ( tSPAWriter.IsError() )
	{
		sError.SetSprintf ( "error writing to %s", sSPAfile.cstr() );
		return false;
	}

	tSPAWriter.CloseFile();
	m_dHashes.SaveSHA();

	bool bHadColumnar = m_tSchema.HasColumnarAttrs();
	bool bHaveColumnar = tNewSchema.HasColumnarAttrs();

	bool bHadNonColumnar = m_tSchema.HasNonColumnarAttrs();
	bool bHaveNonColumnar = tNewSchema.HasNonColumnarAttrs();

	m_tAttr.Reset();

	if ( bColumnar )
	{
		if ( !JuggleFile ( SPH_EXT_SPC, sError, bHadColumnar, bHaveColumnar ) )
			return false;

		if ( tNewSchema.HasColumnarAttrs() )
		{
			m_pColumnar = CreateColumnarStorageReader ( GetIndexFileName(SPH_EXT_SPC).cstr(), (DWORD)m_iDocinfo, sError );
			if ( !m_pColumnar.Ptr() )
				return false;
		}
		else
			m_pColumnar.Reset();
	}
	else
	{
		if ( !JuggleFile ( SPH_EXT_SPA, sError, bHadNonColumnar, bHaveNonColumnar ) )
			return false;
	}

	if ( !JuggleFile ( SPH_EXT_SPH, sError ) )
		return false;

	if ( !JuggleFile ( SPH_EXT_SPHI, sError ) )
		return false;

	if ( bHaveNonColumnar && !m_tAttr.Setup ( GetIndexFileName(SPH_EXT_SPA), sError, true ) )
		return false;

	if ( bBlob )
	{
		m_tBlobAttrs.Reset();

		if ( bAddAttr || bHaveBlobs==bHadBlobs )
		{
			if ( tSPBWriter.IsError() )
			{
				sError.SetSprintf ( "error writing to %s", sSPBfile.cstr() );
				return false;
			}

			SphOffset_t tPos = tSPBWriter.GetPos();
			// FIXME!!! made single function from this mess as order matters here
			tSPBWriter.Flush(); // store collected data as SeekTo might got rid of buffer collected so far
			tSPBWriter.SeekTo ( 0 );
			tSPBWriter.PutOffset ( tPos );
			tSPBWriter.SeekTo ( tPos + m_tSettings.m_tBlobUpdateSpace, true );
			tSPBWriter.CloseFile();

			if ( !JuggleFile ( SPH_EXT_SPB, sError, bHadBlobs ) )
				return false;

			if ( !m_tBlobAttrs.Setup ( GetIndexFileName(SPH_EXT_SPB), sError, true ) )
				return false;
		} else
			::unlink ( GetIndexFileName(SPH_EXT_SPB).cstr() );
	}

	m_tSchema = tNewSchema;
	m_iMinMaxIndex = iNewMinMaxIndex;
	m_pDocinfoIndex = m_tAttr.GetWritePtr() + m_iMinMaxIndex;

	PrereadMapping ( m_sIndexName.cstr(), "attributes", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eAttr ), m_tAttr );

	if ( bBlobsModified )
		PrereadMapping ( m_sIndexName.cstr(), "blob attributes", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eBlob ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eBlob ), m_tBlobAttrs );

	return true;
}


void CSphIndex_VLN::FlushDeadRowMap ( bool bWaitComplete ) const
{
	// FIXME! handle errors
	CSphString sError;
	m_tDeadRowMap.Flush ( bWaitComplete, sError );
}


bool CSphIndex_VLN::LoadKillList ( CSphFixedVector<DocID_t> *pKillList, KillListTargets_c & tTargets, CSphString & sError ) const
{
	CSphString sSPK = GetIndexFileName(SPH_EXT_SPK);
	if ( !sphIsReadable ( sSPK.cstr() ) )
		return true;

	CSphAutoreader tReader;
	if ( !tReader.Open ( sSPK, sError ) )
		return false;

	DWORD nIndexes = tReader.GetDword();
	tTargets.m_dTargets.Resize ( nIndexes );
	for ( auto & tIndex : tTargets.m_dTargets )
	{
		tIndex.m_sIndex = tReader.GetString();
		tIndex.m_uFlags = tReader.GetDword();
	}

	if ( pKillList )
	{
		DWORD nKills = tReader.GetDword();

		pKillList->Reset(nKills);
		DocID_t tDocID = 0;
		for ( int i = 0; i < (int)nKills; i++ )
		{
			DocID_t tDelta = tReader.UnzipOffset();
			assert ( tDelta>0 );
			tDocID += tDelta;
			(*pKillList)[i] = tDocID;
		}
	}

	if ( tReader.GetErrorFlag() )
	{
		sError = tReader.GetErrorMessage();
		return false;
	}

	return true;
}


bool WriteKillList ( const CSphString & sFilename, const DocID_t * pKlist, int nEntries, const KillListTargets_c & tTargets, CSphString & sError )
{
	if ( !nEntries && !tTargets.m_dTargets.GetLength() )
		return true;

	CSphWriter tKillList;
	if ( !tKillList.OpenFile ( sFilename, sError ) )
		return false;

	tKillList.PutDword ( tTargets.m_dTargets.GetLength() );
	for ( const auto & tTarget : tTargets.m_dTargets )
	{
		tKillList.PutString ( tTarget.m_sIndex );
		tKillList.PutDword ( tTarget.m_uFlags );
	}

	tKillList.PutDword ( nEntries );

	if ( pKlist )
	{
		DocID_t tPrevDocID = 0;
		for ( int i = 0; i < nEntries; i++ )
		{
			DocID_t tDocID = pKlist[i];
			tKillList.ZipOffset ( tDocID-tPrevDocID );
			tPrevDocID = tDocID;
		}
	}

	tKillList.CloseFile();
	if ( tKillList.IsError() )
	{
		sError.SetSprintf ( "error writing kill list to %s", sFilename.cstr() );
		return false;
	}

	return true;
}


bool CSphIndex_VLN::AlterKillListTarget ( KillListTargets_c & tTargets, CSphString & sError )
{
	CSphFixedVector<DocID_t> dKillList(0);
	KillListTargets_c tOldTargets;
	if ( !LoadKillList ( &dKillList, tOldTargets, sError ) )
		return false;

	if ( !WriteKillList ( GetIndexFileName ( SPH_EXT_SPK, true ), dKillList.Begin(), dKillList.GetLength(), tTargets, sError ) )
		return false;

	if ( !JuggleFile ( SPH_EXT_SPK, sError, false ) )
		return false;

	return true;
}


void CSphIndex_VLN::KillExistingDocids ( CSphIndex * pTarget )
{
	// FIXME! collecting all docids is a waste of memory
	LookupReaderIterator_c tLookup ( m_tDocidLookup.GetWritePtr() );
	CSphFixedVector<DocID_t> dKillList ( m_iDocinfo );
	DocID_t tDocID;
	DWORD uDocidIndex = 0;
	while ( tLookup.ReadDocID(tDocID) )
		dKillList[uDocidIndex++] = tDocID;

	pTarget->KillMulti ( dKillList );
}


int CSphIndex_VLN::KillMulti ( const VecTraits_T<DocID_t> & dKlist )
{
	LookupReaderIterator_c tTargetReader ( m_tDocidLookup.GetWritePtr() );
	DocidListReader_c tKillerReader ( dKlist );

	int iTotalKilled;
	if ( !m_pKillHook )
		iTotalKilled = KillByLookup ( tTargetReader, tKillerReader, m_tDeadRowMap, [] ( DocID_t ) {} );
	else
		iTotalKilled = KillByLookup ( tTargetReader, tKillerReader, m_tDeadRowMap,
				[this] ( DocID_t tDoc ) { m_pKillHook->Kill ( tDoc ); } );

	if ( iTotalKilled )
		m_uAttrsStatus |= IndexUpdateHelper_c::ATTRS_ROWMAP_UPDATED;

	return iTotalKilled;
}


/////////////////////////////////////////////////////////////////////////////

struct CmpHit_fn
{
	inline bool IsLess ( const CSphWordHit & a, const CSphWordHit & b ) const
	{
		return ( a.m_uWordID<b.m_uWordID ) ||
				( a.m_uWordID==b.m_uWordID && a.m_tRowID<b.m_tRowID ) ||
				( a.m_uWordID==b.m_uWordID && a.m_tRowID==b.m_tRowID && HITMAN::GetPosWithField ( a.m_uWordPos )<HITMAN::GetPosWithField ( b.m_uWordPos ) );
	}
};


CSphString CSphIndex_VLN::GetIndexFileName ( ESphExt eExt, bool bTemp ) const
{
	CSphString sRes;
	sRes.SetSprintf ( bTemp ? "%s.tmp%s" : "%s%s", m_sFilename.cstr(), sphGetExt(eExt) );
	return sRes;
}


CSphString CSphIndex_VLN::GetIndexFileName ( const char * szExt ) const
{
	CSphString sRes;
	sRes.SetSprintf ( "%s.%s", m_sFilename.cstr(), szExt );
	return sRes;
}

void CSphIndex_VLN::GetIndexFiles ( CSphVector<CSphString> & dFiles, const FilenameBuilder_i * pFilenameBuilder ) const
{
	for ( int iExt=0; iExt<SPH_EXT_TOTAL; iExt++ )
	{
		if ( iExt==SPH_EXT_SPL )
			continue;

		CSphString sName = GetIndexFileName ( (ESphExt)iExt, false );
		if ( !sphIsReadable ( sName.cstr() ) )
			continue;

		dFiles.Add ( sName );
	}

	// might be pFilenameBuilder from parent RT index
	CSphScopedPtr<const FilenameBuilder_i> pFilename ( nullptr );
	if ( !pFilenameBuilder && GetIndexFilenameBuilder() )
	{
		pFilename = GetIndexFilenameBuilder() ( m_sIndexName.cstr() );
		pFilenameBuilder = pFilename.Ptr();
	}

	GetSettingsFiles ( m_pTokenizer, m_pDict, GetSettings(), pFilenameBuilder, dFiles );
}

void GetSettingsFiles ( const ISphTokenizer * pTok, const CSphDict * pDict, const CSphIndexSettings & tSettings, const FilenameBuilder_i * pFilenameBuilder, StrVec_t & dFiles )
{
	assert ( pTok );
	assert ( pDict );

	StringBuilder_c sFiles ( "," );
	sFiles += pDict->GetSettings().m_sStopwords.cstr();
	sFiles += pTok->GetSettings().m_sSynonymsFile.cstr();
	sFiles += tSettings.m_sHitlessFiles.cstr();

	for ( const CSphString & sName : pDict->GetSettings().m_dWordforms )
		dFiles.Add ( pFilenameBuilder ? pFilenameBuilder->GetFullPath ( sName ) : sName );

	StrVec_t dFileNames = sphSplit ( sFiles.cstr(), ", " );
	for ( const CSphString & sName : dFileNames )
		dFiles.Add ( pFilenameBuilder ? pFilenameBuilder->GetFullPath ( sName ) : sName );
}


class CSphHitBuilder
{
public:
	CSphHitBuilder ( const CSphIndexSettings & tSettings, const CSphVector<SphWordID_t> & dHitless, bool bMerging, int iBufSize, CSphDict * pDict, CSphString * sError );
	~CSphHitBuilder () {}

	bool	CreateIndexFiles ( const char * sDocName, const char * sHitName, const char * sSkipName, bool bInplace, int iWriteBuffer, CSphAutofile & tHit, SphOffset_t * pSharedOffset );
	void	HitReset ();

	void	cidxHit ( CSphAggregateHit * pHit );
	bool	cidxDone ( int iMemLimit, int & iMinInfixLen, int iMaxCodepointLen, DictHeader_t * pDictHeader );
	int		cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits );

	SphOffset_t		GetHitfilePos () const { return m_wrHitlist.GetPos (); }
	void			CloseHitlist () { m_wrHitlist.CloseFile (); }
	bool			IsError () const { return ( m_pDict->DictIsError() || m_wrDoclist.IsError() || m_wrHitlist.IsError() ); }
	void			HitblockBegin () { m_pDict->HitblockBegin(); }
	bool			IsWordDict () const { return m_pDict->GetSettings().m_bWordDict; }

private:
	void	DoclistBeginEntry ( RowID_t tDocid );
	void	DoclistEndEntry ( Hitpos_t uLastPos );
	void	DoclistEndList ();

	CSphWriter					m_wrDoclist;			///< wordlist writer
	CSphWriter					m_wrHitlist;			///< hitlist writer
	CSphWriter					m_wrSkiplist;			///< skiplist writer
	CSphFixedVector<BYTE>		m_dWriteBuffer;			///< my write buffer (for temp files)

	CSphAggregateHit			m_tLastHit;				///< hitlist entry
	Hitpos_t					m_iPrevHitPos {0};		///< previous hit position
	bool						m_bGotFieldEnd = false;
	BYTE						m_sLastKeyword [ MAX_KEYWORD_BYTES ];

	const CSphVector<SphWordID_t> &	m_dHitlessWords;
	DictRefPtr_c				m_pDict;
	CSphString *				m_pLastError;

	int							m_iSkiplistBlockSize = 0;
	SphOffset_t					m_iLastHitlistPos = 0;		///< doclist entry
	SphOffset_t					m_iLastHitlistDelta = 0;	///< doclist entry
	FieldMask_t					m_dLastDocFields;		///< doclist entry
	DWORD						m_uLastDocHits = 0;			///< doclist entry

	CSphDictEntry				m_tWord;				///< dictionary entry

	ESphHitFormat				m_eHitFormat;
	ESphHitless					m_eHitless;

	CSphVector<SkiplistEntry_t>	m_dSkiplist;
#ifndef NDEBUG
	bool m_bMerging;
#endif
};


CSphHitBuilder::CSphHitBuilder ( const CSphIndexSettings & tSettings,
	const CSphVector<SphWordID_t> & dHitless, bool bMerging, int iBufSize,
	CSphDict * pDict, CSphString * sError )
	: m_dWriteBuffer ( iBufSize )
	, m_dHitlessWords ( dHitless )
	, m_pDict ( pDict )
	, m_pLastError ( sError )
	, m_iSkiplistBlockSize ( tSettings.m_iSkiplistBlockSize )
	, m_eHitFormat ( tSettings.m_eHitFormat )
	, m_eHitless ( tSettings.m_eHitless )
#ifndef NDEBUG
	, m_bMerging ( bMerging )
#endif
{
	m_sLastKeyword[0] = '\0';
	HitReset();
	m_dLastDocFields.UnsetAll();
	SafeAddRef ( pDict );
	assert ( m_pDict );
	assert ( m_pLastError );

	m_pDict->SetSkiplistBlockSize ( m_iSkiplistBlockSize );
}


bool CSphHitBuilder::CreateIndexFiles ( const char * sDocName, const char * sHitName, const char * sSkipName, bool bInplace, int iWriteBuffer, CSphAutofile & tHit, SphOffset_t * pSharedOffset )
{
	// doclist and hitlist files
	m_wrDoclist.CloseFile();
	m_wrHitlist.CloseFile();
	m_wrSkiplist.CloseFile();

	m_wrDoclist.SetBufferSize ( m_dWriteBuffer.GetLength() );
	m_wrHitlist.SetBufferSize ( bInplace ? iWriteBuffer : m_dWriteBuffer.GetLength() );

	if ( !m_wrDoclist.OpenFile ( sDocName, *m_pLastError ) )
		return false;

	if ( bInplace )
	{
		sphSeek ( tHit.GetFD(), 0, SEEK_SET );
		m_wrHitlist.SetFile ( tHit, pSharedOffset, *m_pLastError );
	} else
	{
		if ( !m_wrHitlist.OpenFile ( sHitName, *m_pLastError ) )
			return false;
	}

	if ( !m_wrSkiplist.OpenFile ( sSkipName, *m_pLastError ) )
		return false;

	// put dummy byte (otherwise offset would start from 0, first delta would be 0
	// and VLB encoding of offsets would fuckup)
	BYTE bDummy = 1;
	m_wrDoclist.PutBytes ( &bDummy, 1 );
	m_wrHitlist.PutBytes ( &bDummy, 1 );
	m_wrSkiplist.PutBytes ( &bDummy, 1 );
	return true;
}


void CSphHitBuilder::HitReset()
{
	m_tLastHit.m_tRowID = INVALID_ROWID;
	m_tLastHit.m_uWordID = 0;
	m_tLastHit.m_iWordPos = EMPTY_HIT;
	m_tLastHit.m_sKeyword = m_sLastKeyword;
	m_iPrevHitPos = 0;
	m_bGotFieldEnd = false;
}


// doclist entry format
// (with the new and shiny "inline hit" format, that is)
//
// zint docid_delta
// zint doc_hits
// if doc_hits==1:
// 		zint field_pos
// 		zint field_no
// else:
// 		zint field_mask
// 		zint hlist_offset_delta
//
// so 4 bytes/doc minimum
// avg 4-6 bytes/doc according to our tests


void CSphHitBuilder::DoclistBeginEntry ( RowID_t tRowid )
{
	assert ( m_iSkiplistBlockSize>0 );

	// build skiplist
	// that is, save decoder state and doclist position per every 128 documents
	if ( ( m_tWord.m_iDocs & ( m_iSkiplistBlockSize-1 ) )==0 )
	{
		SkiplistEntry_t & tBlock = m_dSkiplist.Add();
		tBlock.m_tBaseRowIDPlus1 = m_tLastHit.m_tRowID+1;
		tBlock.m_iOffset = m_wrDoclist.GetPos();
		tBlock.m_iBaseHitlistPos = m_iLastHitlistPos;
	}

	// begin doclist entry
	m_wrDoclist.ZipInt ( tRowid - m_tLastHit.m_tRowID );
}


void CSphHitBuilder::DoclistEndEntry ( Hitpos_t uLastPos )
{
	// end doclist entry
	if ( m_eHitFormat==SPH_HIT_FORMAT_INLINE )
	{
		bool bIgnoreHits =
			( m_eHitless==SPH_HITLESS_ALL ) ||
			( m_eHitless==SPH_HITLESS_SOME && ( m_tWord.m_iDocs & HITLESS_DOC_FLAG ) );

		// inline the only hit into doclist (unless it is completely discarded)
		// and finish doclist entry
		m_wrDoclist.ZipInt ( m_uLastDocHits );
		if ( m_uLastDocHits==1 && !bIgnoreHits )
		{
			m_wrHitlist.SeekTo ( m_iLastHitlistPos );
			m_wrDoclist.ZipInt ( uLastPos & 0x7FFFFF );
			m_wrDoclist.ZipInt ( uLastPos >> 23 );
			m_iLastHitlistPos -= m_iLastHitlistDelta;
			assert ( m_iLastHitlistPos>=0 );

		} else
		{
			m_wrDoclist.ZipInt ( m_dLastDocFields.GetMask32() );
			m_wrDoclist.ZipOffset ( m_iLastHitlistDelta );
		}
	} else // plain format - finish doclist entry
	{
		assert ( m_eHitFormat==SPH_HIT_FORMAT_PLAIN );
		m_wrDoclist.ZipOffset ( m_iLastHitlistDelta );
		m_wrDoclist.ZipInt ( m_dLastDocFields.GetMask32() );
		m_wrDoclist.ZipInt ( m_uLastDocHits );
	}
	m_dLastDocFields.UnsetAll();
	m_uLastDocHits = 0;

	// update keyword stats
	m_tWord.m_iDocs++;
}


void CSphHitBuilder::DoclistEndList ()
{
	assert ( m_iSkiplistBlockSize>0 );

	// emit eof marker
	m_wrDoclist.ZipInt ( 0 );

	// emit skiplist
	// OPTIMIZE? placing it after doclist means an extra seek on searching
	// however placing it before means some (longer) doclist data moves while indexing
	if ( m_tWord.m_iDocs>m_iSkiplistBlockSize )
	{
		assert ( m_dSkiplist.GetLength() );
		assert ( m_dSkiplist[0].m_iOffset==m_tWord.m_iDoclistOffset );
		assert ( m_dSkiplist[0].m_tBaseRowIDPlus1==0 );
		assert ( m_dSkiplist[0].m_iBaseHitlistPos==0 );

		m_tWord.m_iSkiplistOffset = m_wrSkiplist.GetPos();

		// delta coding, but with a couple of skiplist specific tricks
		// 1) first entry is omitted, it gets reconstructed from dict itself
		// both base values are zero, and offset equals doclist offset
		// 2) docids are at least SKIPLIST_BLOCK apart
		// doclist entries are at least 4*SKIPLIST_BLOCK bytes apart
		// so we additionally subtract that to improve delta coding
		// 3) zero deltas are allowed and *not* used as any markers,
		// as we know the exact skiplist entry count anyway
		SkiplistEntry_t tLast = m_dSkiplist[0];
		for ( int i=1; i<m_dSkiplist.GetLength(); i++ )
		{
			const SkiplistEntry_t & t = m_dSkiplist[i];
			assert ( t.m_tBaseRowIDPlus1 - tLast.m_tBaseRowIDPlus1>=(DWORD)m_iSkiplistBlockSize );
			assert ( t.m_iOffset - tLast.m_iOffset>=4*m_iSkiplistBlockSize );
			m_wrSkiplist.ZipInt ( t.m_tBaseRowIDPlus1 - tLast.m_tBaseRowIDPlus1 - m_iSkiplistBlockSize );
			m_wrSkiplist.ZipOffset ( t.m_iOffset - tLast.m_iOffset - 4*m_iSkiplistBlockSize );
			m_wrSkiplist.ZipOffset ( t.m_iBaseHitlistPos - tLast.m_iBaseHitlistPos );
			tLast = t;
		}
	}

	// in any event, reset skiplist
	m_dSkiplist.Resize ( 0 );
}

static int strcmpp (const char* l, const char* r)
{
	const char* szEmpty = "";
	if ( !l )
		l = szEmpty;
	if ( !r )
		r = szEmpty;
	return strcmp ( l, r );
}

void CSphHitBuilder::cidxHit ( CSphAggregateHit * pHit )
{
	assert (
		( pHit->m_uWordID!=0 && pHit->m_iWordPos!=EMPTY_HIT && pHit->m_tRowID!=INVALID_ROWID ) || // it's either ok hit
		( pHit->m_uWordID==0 && pHit->m_iWordPos==EMPTY_HIT ) ); // or "flush-hit"

	/////////////
	// next word
	/////////////

	const bool bNextWord = ( m_tLastHit.m_uWordID!=pHit->m_uWordID ||	( m_pDict->GetSettings().m_bWordDict && strcmpp ( (const char*)m_tLastHit.m_sKeyword, (const char*)pHit->m_sKeyword ) ) ); // OPTIMIZE?
	const bool bNextDoc = bNextWord || ( m_tLastHit.m_tRowID!=pHit->m_tRowID );

	if ( m_bGotFieldEnd && ( bNextWord || bNextDoc ) )
	{
		// writing hits only without duplicates
		assert ( HITMAN::GetPosWithField ( m_iPrevHitPos )!=HITMAN::GetPosWithField ( m_tLastHit.m_iWordPos ) );
		HITMAN::SetEndMarker ( &m_tLastHit.m_iWordPos );
		m_wrHitlist.ZipInt ( m_tLastHit.m_iWordPos - m_iPrevHitPos );
		m_bGotFieldEnd = false;
	}


	if ( bNextDoc )
	{
		// finish hitlist, if any
		Hitpos_t uLastPos = m_tLastHit.m_iWordPos;
		if ( m_tLastHit.m_iWordPos!=EMPTY_HIT )
		{
			m_wrHitlist.ZipInt ( 0 );
			m_tLastHit.m_iWordPos = EMPTY_HIT;
			m_iPrevHitPos = EMPTY_HIT;
		}

		// finish doclist entry, if any
		if ( m_tLastHit.m_tRowID!=INVALID_ROWID )
			DoclistEndEntry ( uLastPos );
	}

	if ( bNextWord )
	{
		// finish doclist, if any
		if ( m_tLastHit.m_tRowID!=INVALID_ROWID )
		{
			// emit end-of-doclist marker
			DoclistEndList ();

			// emit dict entry
			m_tWord.m_uWordID = m_tLastHit.m_uWordID;
			m_tWord.m_sKeyword = m_tLastHit.m_sKeyword;
			m_tWord.m_iDoclistLength = m_wrDoclist.GetPos() - m_tWord.m_iDoclistOffset;
			m_pDict->DictEntry ( m_tWord );

			// reset trackers
			m_tWord.m_iDocs = 0;
			m_tWord.m_iHits = 0;

			m_tLastHit.m_tRowID = INVALID_ROWID;
			m_iLastHitlistPos = 0;
		}

		// flush wordlist, if this is the end
		if ( pHit->m_iWordPos==EMPTY_HIT )
		{
			m_pDict->DictEndEntries ( m_wrDoclist.GetPos() );
			return;
		}
#ifndef NDEBUG
		assert ( pHit->m_uWordID > m_tLastHit.m_uWordID
			|| ( m_pDict->GetSettings().m_bWordDict &&
				pHit->m_uWordID==m_tLastHit.m_uWordID && strcmp ( (const char*)pHit->m_sKeyword, (const char*)m_tLastHit.m_sKeyword )>0 )
			|| m_bMerging );
#endif // usually assert excluded in release, but this is 'paranoid' clause
		m_tWord.m_iDoclistOffset = m_wrDoclist.GetPos();
		m_tLastHit.m_uWordID = pHit->m_uWordID;
		if ( m_pDict->GetSettings().m_bWordDict )
		{
			assert ( strlen ( (const char *)pHit->m_sKeyword )<sizeof(m_sLastKeyword)-1 );
			strncpy ( (char*)const_cast<BYTE*>(m_tLastHit.m_sKeyword), (const char*)pHit->m_sKeyword, sizeof(m_sLastKeyword) ); // OPTIMIZE?
		}
	}

	if ( bNextDoc )
	{
		// begin new doclist entry for new doc id
		assert ( m_tLastHit.m_tRowID==INVALID_ROWID || pHit->m_tRowID>m_tLastHit.m_tRowID );
		assert ( m_wrHitlist.GetPos()>=m_iLastHitlistPos );

		DoclistBeginEntry ( pHit->m_tRowID );
		m_iLastHitlistDelta = m_wrHitlist.GetPos() - m_iLastHitlistPos;

		m_tLastHit.m_tRowID = pHit->m_tRowID;
		m_iLastHitlistPos = m_wrHitlist.GetPos();
	}

	///////////
	// the hit
	///////////

	if ( !pHit->m_dFieldMask.TestAll(false) ) // merge aggregate hits into the current hit
	{
		int iHitCount = pHit->GetAggrCount();
		assert ( m_eHitless );
		assert ( iHitCount );
		assert ( !pHit->m_dFieldMask.TestAll(false) );

		m_uLastDocHits += iHitCount;
		for ( int i=0; i<FieldMask_t::SIZE; i++ )
			m_dLastDocFields[i] |= pHit->m_dFieldMask[i];
		m_tWord.m_iHits += iHitCount;

		if ( m_eHitless==SPH_HITLESS_SOME )
			m_tWord.m_iDocs |= HITLESS_DOC_FLAG;

	} else // handle normal hits
	{
		Hitpos_t iHitPosPure = HITMAN::GetPosWithField ( pHit->m_iWordPos );

		// skip any duplicates and keep only 1st position in place
		// duplicates are hit with same position: [N, N] [N, N | FIELDEND_MASK] [N | FIELDEND_MASK, N] [N | FIELDEND_MASK, N | FIELDEND_MASK]
		if ( iHitPosPure==m_tLastHit.m_iWordPos )
			return;

		// storing previous hit that might have a field end flag
		if ( m_bGotFieldEnd )
		{
			if ( HITMAN::GetField ( pHit->m_iWordPos )!=HITMAN::GetField ( m_tLastHit.m_iWordPos ) ) // is field end flag real?
				HITMAN::SetEndMarker ( &m_tLastHit.m_iWordPos );

			m_wrHitlist.ZipInt ( m_tLastHit.m_iWordPos - m_iPrevHitPos );
			m_bGotFieldEnd = false;
		}

		/* duplicate hits from duplicated documents
		... 0x03, 0x03 ... 
		... 0x8003, 0x8003 ... 
		... 1, 0x8003, 0x03 ... 
		... 1, 0x03, 0x8003 ... 
		... 1, 0x8003, 0x04 ... 
		... 1, 0x03, 0x8003, 0x8003 ... 
		... 1, 0x03, 0x8003, 0x03 ... 
		*/

		assert ( m_tLastHit.m_iWordPos < pHit->m_iWordPos );

		// add hit delta without field end marker
		// or postpone adding to hitlist till got another uniq hit
		if ( iHitPosPure==pHit->m_iWordPos )
		{
			m_wrHitlist.ZipInt ( pHit->m_iWordPos - m_tLastHit.m_iWordPos );
			m_tLastHit.m_iWordPos = pHit->m_iWordPos;
		} else
		{
			assert ( HITMAN::IsEnd ( pHit->m_iWordPos ) );
			m_bGotFieldEnd = true;
			m_iPrevHitPos = m_tLastHit.m_iWordPos;
			m_tLastHit.m_iWordPos = HITMAN::GetPosWithField ( pHit->m_iWordPos );
		}

		// update matched fields mask
		m_dLastDocFields.Set ( HITMAN::GetField ( pHit->m_iWordPos ) );

		m_uLastDocHits++;
		m_tWord.m_iHits++;
	}
}


static void ReadSchemaColumn ( CSphReader & rdInfo, CSphColumnInfo & tCol, DWORD uVersion )
{
	tCol.m_sName = rdInfo.GetString ();
	if ( tCol.m_sName.IsEmpty () )
		tCol.m_sName = "@emptyname";

	tCol.m_sName.ToLower ();
	tCol.m_eAttrType = (ESphAttr) rdInfo.GetDword (); // FIXME? check/fixup?

	rdInfo.GetDword (); // ignore rowitem
	tCol.m_tLocator.m_iBitOffset = rdInfo.GetDword ();
	tCol.m_tLocator.m_iBitCount = rdInfo.GetDword ();
	tCol.m_bPayload = ( rdInfo.GetByte()!=0 );

	if ( uVersion>=61 )
		tCol.m_uAttrFlags = rdInfo.GetDword();

	if ( uVersion>=63 )
		tCol.m_eEngine = (AttrEngine_e)rdInfo.GetDword();

	// WARNING! max version used here must be in sync with RtIndex_c::Prealloc
}


static void ReadSchemaField ( CSphReader & rdInfo, CSphColumnInfo & tCol, DWORD uVersion )
{
	if ( uVersion>=57  )
	{
		tCol.m_sName = rdInfo.GetString();
		tCol.m_uFieldFlags = rdInfo.GetDword();
		tCol.m_bPayload = !!rdInfo.GetByte();
	}
	else
		ReadSchemaColumn ( rdInfo, tCol, uVersion );

	if ( uVersion<59 )
		tCol.m_uFieldFlags |= CSphColumnInfo::FIELD_INDEXED;
}


void ReadSchema ( CSphReader & rdInfo, CSphSchema & m_tSchema, DWORD uVersion )
{
	m_tSchema.Reset ();

	int iNumFields = rdInfo.GetDword();
	for ( int i=0; i<iNumFields; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaField ( rdInfo, tCol, uVersion );
		m_tSchema.AddField ( tCol );
	}

	int iNumAttrs = rdInfo.GetDword();
	for ( int i=0; i<iNumAttrs; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumn ( rdInfo, tCol, uVersion );
		m_tSchema.AddAttr ( tCol, false );
	}
}


static void WriteSchemaField ( CSphWriter & fdInfo, const CSphColumnInfo & tCol )
{
	fdInfo.PutString ( tCol.m_sName );
	fdInfo.PutDword ( tCol.m_uFieldFlags );
	fdInfo.PutByte ( tCol.m_bPayload );
}


static void WriteSchemaColumn ( CSphWriter & fdInfo, const CSphColumnInfo & tCol )
{
	fdInfo.PutString( tCol.m_sName );
	fdInfo.PutDword ( tCol.m_eAttrType );

	fdInfo.PutDword ( tCol.m_tLocator.CalcRowitem() ); // for backwards compatibility
	fdInfo.PutDword ( tCol.m_tLocator.m_iBitOffset );
	fdInfo.PutDword ( tCol.m_tLocator.m_iBitCount );

	fdInfo.PutByte ( tCol.m_bPayload );
	fdInfo.PutDword ( tCol.m_uAttrFlags );
	fdInfo.PutDword ( (DWORD)tCol.m_eEngine );
}


void WriteSchema ( CSphWriter & fdInfo, const CSphSchema & tSchema )
{
	fdInfo.PutDword ( tSchema.GetFieldsCount() );
	for ( int i=0; i<tSchema.GetFieldsCount(); i++ )
		WriteSchemaField ( fdInfo, tSchema.GetField(i) );

	fdInfo.PutDword ( tSchema.GetAttrsCount() );
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
		WriteSchemaColumn ( fdInfo, tSchema.GetAttr(i) );
}

void operator<< ( JsonEscapedBuilder& tOut, const CSphAttrLocator& tLoc )
{
	auto _ = tOut.Object();
	tOut.NamedVal ( "pos", tLoc.m_iBitOffset );
	tOut.NamedVal ( "bits", tLoc.m_iBitCount );
}

namespace {

void DumpFieldToJson ( JsonEscapedBuilder& tOut, const CSphColumnInfo& tCol )
{
	auto _ = tOut.Object();
	tOut.NamedString ( "name", tCol.m_sName );
	tOut.NamedValNonDefault ( "flags", tCol.m_uFieldFlags, (DWORD)CSphColumnInfo::FIELD_INDEXED );
	tOut.NamedValNonDefault ( "payload", tCol.m_bPayload, false );
}

void DumpAttrToJson ( JsonEscapedBuilder& tOut, const CSphColumnInfo& tCol )
{
	auto _ = tOut.Object();
	tOut.NamedString ( "name", tCol.m_sName );
	tOut.NamedValNonDefault ( "flags", tCol.m_uAttrFlags, (DWORD)CSphColumnInfo::ATTR_NONE );
	tOut.NamedValNonDefault ( "payload", tCol.m_bPayload, false );
	tOut.NamedValNonDefault ( "engine", (DWORD)tCol.m_eEngine, (DWORD)AttrEngine_e::DEFAULT );
	tOut.NamedVal ( "type", tCol.m_eAttrType );
	tOut.NamedVal ( "locator", tCol.m_tLocator );
}
} // namespace

void operator<< ( JsonEscapedBuilder& tOut, const CSphSchema& tSchema )
{
	auto _ = tOut.ObjectW();
	if ( tSchema.GetFieldsCount() > 0 )
	{
		tOut.Named ( "fields" );
		auto _ = tOut.ArrayW();
		for ( int i = 0; i < tSchema.GetFieldsCount(); ++i )
			DumpFieldToJson ( tOut, tSchema.GetField ( i ) );
	}
	if ( tSchema.GetAttrsCount() > 0 )
	{
		tOut.Named ( "attributes" );
		auto _ = tOut.ArrayW();
		for ( int i = 0; i < tSchema.GetAttrsCount(); ++i )
			DumpAttrToJson ( tOut, tSchema.GetAttr ( i ) );
	}
}


void SaveIndexSettings ( CSphWriter & tWriter, const CSphIndexSettings & tSettings )
{
	tWriter.PutDword ( tSettings.RawMinPrefixLen() );
	tWriter.PutDword ( tSettings.m_iMinInfixLen );
	tWriter.PutDword ( tSettings.m_iMaxSubstringLen );
	tWriter.PutByte ( tSettings.m_bHtmlStrip ? 1 : 0 );
	tWriter.PutString ( tSettings.m_sHtmlIndexAttrs.cstr () );
	tWriter.PutString ( tSettings.m_sHtmlRemoveElements.cstr () );
	tWriter.PutByte ( tSettings.m_bIndexExactWords ? 1 : 0 );
	tWriter.PutDword ( tSettings.m_eHitless );
	tWriter.PutDword ( tSettings.m_eHitFormat );
	tWriter.PutByte ( tSettings.m_bIndexSP );
	tWriter.PutString ( tSettings.m_sZones );
	tWriter.PutDword ( tSettings.m_iBoundaryStep );
	tWriter.PutDword ( tSettings.m_iStopwordStep );
	tWriter.PutDword ( tSettings.m_iOvershortStep );
	tWriter.PutDword ( tSettings.m_iEmbeddedLimit );
	tWriter.PutByte ( tSettings.m_eBigramIndex );
	tWriter.PutString ( tSettings.m_sBigramWords );
	tWriter.PutByte ( tSettings.m_bIndexFieldLens );
	tWriter.PutByte ( tSettings.m_ePreprocessor==Preprocessor_e::ICU ? 1 : 0 );
	tWriter.PutString("");	// was: RLP context
	tWriter.PutString ( tSettings.m_sIndexTokenFilter );
	tWriter.PutOffset ( tSettings.m_tBlobUpdateSpace );
	tWriter.PutDword ( tSettings.m_iSkiplistBlockSize );
	tWriter.PutString ( tSettings.m_sHitlessFiles );
	tWriter.PutDword ( (DWORD)tSettings.m_eEngine );
}

void operator<< ( JsonEscapedBuilder& tOut, const CSphIndexSettings& tSettings )
{
	auto _ = tOut.ObjectW();
	tOut.NamedValNonDefault ( "min_prefix_len", tSettings.RawMinPrefixLen() );
	tOut.NamedValNonDefault ( "min_infix_len", tSettings.m_iMinInfixLen );
	tOut.NamedValNonDefault ( "max_substring_len", tSettings.m_iMaxSubstringLen );
	tOut.NamedValNonDefault ( "strip_html", tSettings.m_bHtmlStrip, false );
	tOut.NamedStringNonEmpty ( "html_index_attrs", tSettings.m_sHtmlIndexAttrs );
	tOut.NamedStringNonEmpty ( "html_remove_elements", tSettings.m_sHtmlRemoveElements );
	tOut.NamedValNonDefault ( "index_exact_words", tSettings.m_bIndexExactWords, false );
	tOut.NamedValNonDefault ( "hitless", tSettings.m_eHitless, SPH_HITLESS_NONE );
	tOut.NamedValNonDefault ( "hit_format", tSettings.m_eHitFormat, SPH_HIT_FORMAT_PLAIN );
	tOut.NamedValNonDefault ( "index_sp", tSettings.m_bIndexSP, false );
	tOut.NamedStringNonEmpty ( "zones", tSettings.m_sZones );
	tOut.NamedValNonDefault ( "boundary_step", tSettings.m_iBoundaryStep );
	tOut.NamedValNonDefault ( "stopword_step", tSettings.m_iStopwordStep, 1 );
	tOut.NamedValNonDefault ( "overshort_step", tSettings.m_iOvershortStep, 1 );
	tOut.NamedValNonDefault ( "embedded_limit", tSettings.m_iEmbeddedLimit );
	tOut.NamedValNonDefault ( "bigram_index", tSettings.m_eBigramIndex, SPH_BIGRAM_NONE );
	tOut.NamedStringNonEmpty ( "bigram_words", tSettings.m_sBigramWords );
	tOut.NamedValNonDefault ( "index_field_lens", tSettings.m_bIndexFieldLens, false );
	tOut.NamedValNonDefault ( "icu", (DWORD)tSettings.m_ePreprocessor, (DWORD)Preprocessor_e::NONE );
	tOut.NamedStringNonEmpty ( "index_token_filter", tSettings.m_sIndexTokenFilter );
	tOut.NamedValNonDefault ( "blob_update_space", tSettings.m_tBlobUpdateSpace );
	tOut.NamedValNonDefault ( "skiplist_block_size", tSettings.m_iSkiplistBlockSize, 32 );
	tOut.NamedStringNonEmpty ( "hitless_files", tSettings.m_sHitlessFiles );
	tOut.NamedValNonDefault ( "engine", (DWORD)tSettings.m_eEngine, (DWORD)AttrEngine_e::DEFAULT );
}

void IndexWriteHeader ( const BuildHeader_t & tBuildHeader, const WriteHeader_t & tWriteHeader, CSphWriter & fdInfo, bool bForceWordDict, bool SkipEmbeddDict )
{
	// version
	fdInfo.PutDword ( INDEX_MAGIC_HEADER );
	fdInfo.PutDword ( INDEX_FORMAT_VERSION );

	// schema
	WriteSchema ( fdInfo, *tWriteHeader.m_pSchema );

	// wordlist checkpoints
	fdInfo.PutOffset ( tBuildHeader.m_iDictCheckpointsOffset );
	fdInfo.PutDword ( tBuildHeader.m_iDictCheckpoints );
	fdInfo.PutByte ( (BYTE)tBuildHeader.m_iInfixCodepointBytes );
	fdInfo.PutDword ( (DWORD)tBuildHeader.m_iInfixBlocksOffset );
	fdInfo.PutDword ( tBuildHeader.m_iInfixBlocksWordsSize );

	// index stats
	fdInfo.PutDword ( (DWORD)tBuildHeader.m_iTotalDocuments ); // FIXME? we don't expect over 4G docs per just 1 local index
	fdInfo.PutOffset ( tBuildHeader.m_iTotalBytes );

	// index settings
	SaveIndexSettings ( fdInfo, *tWriteHeader.m_pSettings );

	// tokenizer info
	assert ( tWriteHeader.m_pTokenizer );
	SaveTokenizerSettings ( fdInfo, tWriteHeader.m_pTokenizer, tWriteHeader.m_pSettings->m_iEmbeddedLimit );

	// dictionary info
	assert ( tWriteHeader.m_pDict );
	SaveDictionarySettings ( fdInfo, tWriteHeader.m_pDict, bForceWordDict, SkipEmbeddDict ? 0 : tWriteHeader.m_pSettings->m_iEmbeddedLimit );

	fdInfo.PutOffset ( tBuildHeader.m_iDocinfo );
	fdInfo.PutOffset ( tBuildHeader.m_iDocinfoIndex );
	fdInfo.PutOffset ( tBuildHeader.m_iMinMaxIndex );

	// field filter info
	CSphFieldFilterSettings tFieldFilterSettings;
	if ( tWriteHeader.m_pFieldFilter.Ptr() )
		tWriteHeader.m_pFieldFilter->GetSettings(tFieldFilterSettings);
	tFieldFilterSettings.Save(fdInfo);

	// average field lengths
	if ( tWriteHeader.m_pSettings->m_bIndexFieldLens )
		for ( int i=0; i < tWriteHeader.m_pSchema->GetFieldsCount(); ++i )
			fdInfo.PutOffset ( tWriteHeader.m_pFieldLens[i] );
}


bool IndexBuildDone ( const BuildHeader_t & tBuildHeader, const WriteHeader_t & tWriteHeader, const CSphString & sFileName, CSphString & sError )
{
	CSphWriter fdInfo;
	if ( !fdInfo.OpenFile ( sFileName, sError ) )
		return false;

	IndexWriteHeader ( tBuildHeader, tWriteHeader, fdInfo, false );
	return true;
}


bool CSphHitBuilder::cidxDone ( int iMemLimit, int & iMinInfixLen, int iMaxCodepointLen, DictHeader_t * pDictHeader )
{
	assert ( pDictHeader );

	if ( m_bGotFieldEnd )
	{
		HITMAN::SetEndMarker ( &m_tLastHit.m_iWordPos );
		m_wrHitlist.ZipInt ( m_tLastHit.m_iWordPos - m_iPrevHitPos );
		m_bGotFieldEnd = false;
	}

	// finalize dictionary
	// in dict=crc mode, just flushes wordlist checkpoints
	// in dict=keyword mode, also creates infix index, if needed

	if ( iMinInfixLen>0 && m_pDict->GetSettings().m_bWordDict )
	{
		pDictHeader->m_iInfixCodepointBytes = iMaxCodepointLen;
		if ( iMinInfixLen==1 )
		{
			sphWarn ( "min_infix_len must be greater than 1, changed to 2" );
			iMinInfixLen = 2;
		}
	}

	if ( !m_pDict->DictEnd ( pDictHeader, iMemLimit, *m_pLastError ) )
		return false;

	// close all data files
	m_wrDoclist.CloseFile ();
	m_wrHitlist.CloseFile ( true );
	m_wrSkiplist.CloseFile ();
	return !IsError();
}


inline int encodeVLB ( BYTE * buf, DWORD v )
{
	register BYTE b;
	register int n = 0;

	do
	{
		b = (BYTE)(v & 0x7f);
		v >>= 7;
		if ( v )
			b |= 0x80;
		*buf++ = b;
		n++;
	} while ( v );
	return n;
}


inline int encodeKeyword ( BYTE * pBuf, const char * pKeyword )
{
	auto iLen = (int) strlen ( pKeyword ); // OPTIMIZE! remove this and memcpy and check if thats faster
	assert ( iLen>0 && iLen<128 ); // so that ReadVLB()

	*pBuf = (BYTE) iLen;
	memcpy ( pBuf+1, pKeyword, iLen );
	return 1+iLen;
}


int CSphHitBuilder::cidxWriteRawVLB ( int fd, CSphWordHit * pHit, int iHits )
{
	assert ( pHit );
	assert ( iHits>0 );

	///////////////////////////////////////
	// encode through a small write buffer
	///////////////////////////////////////

	BYTE *pBuf, *maxP;
	int n = 0, w;
	SphWordID_t d1, l1 = 0;
	RowID_t d2, l2 = (RowID_t)-1; // rowids start from 0 and we can't have delta=0
	DWORD d3, l3 = 0; // !COMMIT must be wide enough

	int iGap = (int)Max ( 16*sizeof(DWORD) + ( m_pDict->GetSettings().m_bWordDict ? MAX_KEYWORD_BYTES : 0 ), 128u );
	pBuf = m_dWriteBuffer.Begin();
	maxP = m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() - iGap;

	// hit aggregation state
	DWORD uHitCount = 0;
	DWORD uHitFieldMask = 0;

	const int iPositionShift = m_eHitless==SPH_HITLESS_SOME ? 1 : 0;

	while ( iHits-- )
	{
		// calc deltas
		d1 = pHit->m_uWordID - l1;
		d2 = pHit->m_tRowID - l2;
		d3 = pHit->m_uWordPos - l3;

		// ignore duplicate hits
		if ( d1==0 && d2==0 && d3==0 ) // OPTIMIZE? check if ( 0==(d1|d2|d3) ) is faster
		{
			pHit++;
			continue;
		}

		// checks below are intended handle several "fun" cases
		//
		// case 1, duplicate documents (same docid), different field contents, but ending with
		// the same keyword, ending up in multiple field end markers within the same keyword
		// eg. [foo] in positions {1, 0x800005} in 1st dupe, {3, 0x800007} in 2nd dupe
		//
		// case 2, blended token in the field end, duplicate parts, different positions (as expected)
		// for those parts but still multiple field end markers, eg. [U.S.S.R.] in the end of field

		// replacement of hit itself by field-end form
		if ( d1==0 && d2==0 && HITMAN::GetPosWithField ( pHit->m_uWordPos )==HITMAN::GetPosWithField ( l3 ) )
		{
			l3 = pHit->m_uWordPos;
			pHit++;
			continue;
		}

		// reset field-end inside token stream due of document duplicates
		if ( d1==0 && d2==0 && HITMAN::IsEnd ( l3 ) && HITMAN::GetField ( pHit->m_uWordPos )==HITMAN::GetField ( l3 ) )
		{
			l3 = HITMAN::GetPosWithField ( l3 );
			d3 = HITMAN::GetPosWithField ( pHit->m_uWordPos ) - l3;

			if ( d3==0 )
			{
				pHit++;
				continue;
			}
		}

		// non-zero delta restarts all the fields after it
		// because their deltas might now be negative
		if ( d1 ) d2 = pHit->m_tRowID+1;
		if ( d2 ) d3 = pHit->m_uWordPos;

		// when we moved to the next word or document
		bool bFlushed = false;
		if ( d1 || d2 )
		{
			// flush previous aggregate hit
			if ( uHitCount )
			{
				// we either skip all hits or the high bit must be available for marking
				// failing that, we can't produce a consistent index
				assert ( m_eHitless!=SPH_HITLESS_NONE );
				assert ( m_eHitless==SPH_HITLESS_ALL || !( uHitCount & 0x80000000UL ) );

				if ( m_eHitless!=SPH_HITLESS_ALL )
					uHitCount = ( uHitCount << 1 ) | 1;
				pBuf += encodeVLB ( pBuf, uHitCount );
				pBuf += encodeVLB ( pBuf, uHitFieldMask );
				assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );

				uHitCount = 0;
				uHitFieldMask = 0;

				bFlushed = true;
			}

			// start aggregating if we're skipping all hits or this word is in a list of ignored words
			if ( ( m_eHitless==SPH_HITLESS_ALL ) ||
				( m_eHitless==SPH_HITLESS_SOME && m_dHitlessWords.BinarySearch ( pHit->m_uWordID ) ) )
			{
				uHitCount = 1;
				uHitFieldMask |= 1 << HITMAN::GetField ( pHit->m_uWordPos );
			}

		} else if ( uHitCount ) // next hit for the same word/doc pair, update state if we need it
		{
			uHitCount++;
			uHitFieldMask |= 1 << HITMAN::GetField ( pHit->m_uWordPos );
		}

		// encode enough restart markers
		if ( d1 ) pBuf += encodeVLB ( pBuf, 0 );
		if ( d2 && !bFlushed ) pBuf += encodeVLB ( pBuf, 0 );

		assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );

		// encode deltas

		// encode keyword
		if ( d1 )
		{
			if ( m_pDict->GetSettings().m_bWordDict )
				pBuf += encodeKeyword ( pBuf, m_pDict->HitblockGetKeyword ( pHit->m_uWordID ) ); // keyword itself in case of keywords dict
			else
				pBuf += sphEncodeVLB8 ( pBuf, d1 ); // delta in case of CRC dict

			assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
		}

		// encode docid delta
		if ( d2 )
		{
			pBuf += sphEncodeVLB8 ( pBuf, d2 );
			assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
		}

		assert ( d3 );
		if ( !uHitCount ) // encode position delta, unless accumulating hits
		{
			pBuf += encodeVLB ( pBuf, d3 << iPositionShift );
			assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
		}

		// update current state
		l1 = pHit->m_uWordID;
		l2 = pHit->m_tRowID;
		l3 = pHit->m_uWordPos;

		pHit++;

		if ( pBuf>maxP )
		{
			w = (int)(pBuf - m_dWriteBuffer.Begin());
			assert ( w<m_dWriteBuffer.GetLength() );
			if ( !sphWriteThrottled ( fd, m_dWriteBuffer.Begin(), w, "raw_hits", *m_pLastError ) )
				return -1;
			n += w;
			pBuf = m_dWriteBuffer.Begin();
		}
	}

	// flush last aggregate
	if ( uHitCount )
	{
		assert ( m_eHitless!=SPH_HITLESS_NONE );
		assert ( m_eHitless==SPH_HITLESS_ALL || !( uHitCount & 0x80000000UL ) );

		if ( m_eHitless!=SPH_HITLESS_ALL )
			uHitCount = ( uHitCount << 1 ) | 1;
		pBuf += encodeVLB ( pBuf, uHitCount );
		pBuf += encodeVLB ( pBuf, uHitFieldMask );

		assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
	}

	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	pBuf += encodeVLB ( pBuf, 0 );
	assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
	w = (int)(pBuf - m_dWriteBuffer.Begin());
	assert ( w<m_dWriteBuffer.GetLength() );
	if ( !sphWriteThrottled ( fd, m_dWriteBuffer.Begin(), w, "raw_hits", *m_pLastError ) )
		return -1;
	n += w;

	return n;
}

/////////////////////////////////////////////////////////////////////////////

// OPTIMIZE?
inline bool SPH_CMPAGGRHIT_LESS ( const CSphAggregateHit & a, const CSphAggregateHit & b )
{
	if ( a.m_uWordID < b.m_uWordID )
		return true;

	if ( a.m_uWordID > b.m_uWordID )
		return false;

	if ( a.m_sKeyword )
	{
		int iCmp = strcmp ( (const char*)a.m_sKeyword, (const char*)b.m_sKeyword ); // OPTIMIZE?
		if ( iCmp!=0 )
			return ( iCmp<0 );
	}

	return
		( a.m_tRowID < b.m_tRowID ) ||
		( a.m_tRowID==b.m_tRowID && HITMAN::GetPosWithField ( a.m_iWordPos )<HITMAN::GetPosWithField ( b.m_iWordPos ) );
}


/// hit priority queue entry
struct CSphHitQueueEntry : public CSphAggregateHit
{
	int m_iBin;
};


/// hit priority queue
struct CSphHitQueue
{
public:
	CSphHitQueueEntry *		m_pData;
	int						m_iSize;
	int						m_iUsed;

public:
	/// create queue
	explicit CSphHitQueue ( int iSize )
	{
		assert ( iSize>0 );
		m_iSize = iSize;
		m_iUsed = 0;
		m_pData = new CSphHitQueueEntry [ iSize ];
	}

	/// destroy queue
	~CSphHitQueue ()
	{
		SafeDeleteArray ( m_pData );
	}

	/// add entry to the queue
	void Push ( CSphAggregateHit & tHit, int iBin )
	{
		// check for overflow and do add
		assert ( m_iUsed<m_iSize );
		auto & tEntry = m_pData[m_iUsed];

		tEntry.m_tRowID = tHit.m_tRowID;
		tEntry.m_uWordID = tHit.m_uWordID;
		tEntry.m_sKeyword = tHit.m_sKeyword; // bin must hold the actual data for the queue
		tEntry.m_iWordPos = tHit.m_iWordPos;
		tEntry.m_dFieldMask = tHit.m_dFieldMask;
		tEntry.m_iBin = iBin;

		int iEntry = m_iUsed++;

		// sift up if needed
		while ( iEntry )
		{
			int iParent = ( iEntry-1 ) >> 1;
			if ( SPH_CMPAGGRHIT_LESS ( m_pData[iEntry], m_pData[iParent] ) )
			{
				// entry is less than parent, should float to the top
				Swap ( m_pData[iEntry], m_pData[iParent] );
				iEntry = iParent;
			} else
				break;
		}
	}

	/// remove root (ie. top priority) entry
	void Pop ()
	{
		assert ( m_iUsed );
		if ( !(--m_iUsed) ) // empty queue? just return
			return;

		// make the last entry my new root
		m_pData[0] = m_pData[m_iUsed];

		// sift down if needed
		int iEntry = 0;
		while (true)
		{
			// select child
			int iChild = (iEntry<<1) + 1;
			if ( iChild>=m_iUsed )
				break;

			// select smallest child
			if ( iChild+1<m_iUsed )
				if ( SPH_CMPAGGRHIT_LESS ( m_pData[iChild+1], m_pData[iChild] ) )
					iChild++;

			// if smallest child is less than entry, do float it to the top
			if ( SPH_CMPAGGRHIT_LESS ( m_pData[iChild], m_pData[iEntry] ) )
			{
				Swap ( m_pData[iChild], m_pData[iEntry] );
				iEntry = iChild;
				continue;
			}

			break;
		}
	}
};


#define MAX_SOURCE_HITS	32768
static const int MIN_KEYWORDS_DICT	= 4*1048576;	// FIXME! ideally must be in sync with impl (ENTRY_CHUNKS, KEYWORD_CHUNKS)

/////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::RelocateBlock ( int iFile, BYTE * pBuffer, int iRelocationSize,
	SphOffset_t * pFileSize, CSphBin & dMinBin, SphOffset_t * pSharedOffset )
{
	assert ( pBuffer && pFileSize && pSharedOffset );

	SphOffset_t iBlockStart = dMinBin.m_iFilePos;
	SphOffset_t iBlockLeft = dMinBin.m_iFileLeft;

	ESphBinRead eRes = dMinBin.Precache ();
	switch ( eRes )
	{
	case BIN_PRECACHE_OK:
		return true;
	case BIN_READ_ERROR:
		m_sLastError = "block relocation: preread error";
		return false;
	default:
		break;
	}

	int nTransfers = (int)( ( iBlockLeft+iRelocationSize-1) / iRelocationSize );

	SphOffset_t uTotalRead = 0;
	SphOffset_t uNewBlockStart = *pFileSize;

	for ( int i = 0; i < nTransfers; i++ )
	{
		if ( !SeekAndWarn ( iFile, iBlockStart + uTotalRead, "block relocation" ))
			return false;

		int iToRead = i==nTransfers-1 ? (int)( iBlockLeft % iRelocationSize ) : iRelocationSize;
		size_t iRead = sphReadThrottled ( iFile, pBuffer, iToRead );
		if ( iRead!=size_t(iToRead) )
		{
			m_sLastError.SetSprintf ( "block relocation: read error (%d of %d bytes read): %s", (int)iRead, iToRead, strerrorm(errno) );
			return false;
		}

		if ( !SeekAndWarn ( iFile, *pFileSize, "block relocation" ))
			return false;

		uTotalRead += iToRead;

		if ( !sphWriteThrottled ( iFile, pBuffer, iToRead, "block relocation", m_sLastError ) )
			return false;

		*pFileSize += iToRead;
	}

	assert ( uTotalRead==iBlockLeft );

	// update block pointers
	dMinBin.m_iFilePos = uNewBlockStart;
	*pSharedOffset = *pFileSize;

	return true;
}


bool LoadHitlessWords ( const CSphString & sHitlessFiles, ISphTokenizer * pTok, CSphDict * pDict, CSphVector<SphWordID_t> & dHitlessWords, CSphString & sError )
{
	assert ( dHitlessWords.GetLength()==0 );

	if ( sHitlessFiles.IsEmpty() )
		return true;

	StrVec_t dFiles;
	sphSplit ( dFiles, sHitlessFiles.cstr(), ", " );

	for ( const CSphString & sFilename : dFiles )
	{
		CSphAutofile tFile ( sFilename.cstr(), SPH_O_READ, sError );
		if ( tFile.GetFD()==-1 )
			return false;

		CSphVector<BYTE> dBuffer ( (int)tFile.GetSize() );
		if ( !tFile.Read ( &dBuffer[0], dBuffer.GetLength(), sError ) )
			return false;

		// FIXME!!! dict=keywords + hitless_words=some
		pTok->SetBuffer ( &dBuffer[0], dBuffer.GetLength() );
		while ( BYTE * sToken = pTok->GetToken() )
			dHitlessWords.Add ( pDict->GetWordID ( sToken ) );
	}

	dHitlessWords.Uniq();
	return true;
}


class DeleteOnFail_c : public ISphNoncopyable
{
public:
	DeleteOnFail_c() : m_bShitHappened ( true )
	{}
	~DeleteOnFail_c()
	{
		if ( m_bShitHappened )
		{
			ARRAY_FOREACH ( i, m_dWriters )
				m_dWriters[i]->UnlinkFile();

			ARRAY_FOREACH ( i, m_dAutofiles )
				m_dAutofiles[i]->SetTemporary();
		}
	}
	void AddWriter ( CSphWriter * pWr )
	{
		if ( pWr )
			m_dWriters.Add ( pWr );
	}
	void AddAutofile ( CSphAutofile * pAf )
	{
		if ( pAf )
			m_dAutofiles.Add ( pAf );
	}
	void AllIsDone()
	{
		m_bShitHappened = false;
	}
private:
	bool	m_bShitHappened;
	CSphVector<CSphWriter*> m_dWriters;
	CSphVector<CSphAutofile*> m_dAutofiles;
};


bool CSphIndex_VLN::CollectQueryMvas ( const CSphVector<CSphSource*> & dSources, QueryMvaContainer_c & tMvaContainer )
{
	CSphBitvec dQueryMvas ( m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		if ( tAttr.m_eSrc!=SPH_ATTRSRC_FIELD && ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) )
			dQueryMvas.BitSet(i);
	}

	if ( !dQueryMvas.BitCount() )
		return true;

	assert ( !tMvaContainer.m_tContainer.GetLength() );
	tMvaContainer.m_tContainer.Resize ( m_tSchema.GetAttrsCount() );
	for ( auto & i : tMvaContainer.m_tContainer )
		i = nullptr;

	for ( auto & pSource : dSources )
	{
		assert ( pSource );
		if ( !pSource->Connect ( m_sLastError ) )
			return false;

		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			if ( !dQueryMvas.BitGet(i) )
				continue;

			auto * & pHash = tMvaContainer.m_tContainer[i];
			if ( !pHash )
				pHash = new OpenHash_T<CSphVector<int64_t>, int64_t, HashFunc_Int64_t>;

			if ( !pSource->IterateMultivaluedStart ( i, m_sLastError ) )
				return false;

			int64_t iDocID;
			int64_t iMvaValue;
			while ( pSource->IterateMultivaluedNext ( iDocID, iMvaValue) )
			{
				auto & tMva = pHash->Acquire ( iDocID );
				tMva.Add ( iMvaValue );
			}
		}

		pSource->Disconnect ();
	}

	return true;
}

struct Mva32Uniq_fn
{
	bool IsLess ( const uint64_t & iA, const uint64_t & iB ) const
	{
		DWORD uA = (DWORD)iA;
		DWORD uB = (DWORD)iB;

		return uA<uB;
	}

	bool IsEq ( const uint64_t & iA, const uint64_t & iB ) const
	{
		DWORD uA = (DWORD)iA;
		DWORD uB = (DWORD)iB;
		return uA==uB;
	}
};


static void SortMva ( CSphVector<int64_t> & tMva, bool bMva32 )
{
	if ( !bMva32 )
	{
		tMva.Uniq();
		return;
	}

	tMva.Sort ( Mva32Uniq_fn() );
	int iLeft = sphUniq ( tMva.Begin(), tMva.GetLength(), Mva32Uniq_fn() );
	tMva.Resize ( iLeft );
}


static const CSphVector<int64_t> * FetchMVA ( DocID_t tDocId, int iAttr, const CSphColumnInfo & tAttr, QueryMvaContainer_c & tMvaContainer, AttrSource_i & tSource, bool bForceSource )
{
	CSphVector<int64_t> * pMva = nullptr;

	if ( tAttr.m_eSrc==SPH_ATTRSRC_FIELD || bForceSource )
		pMva = tSource.GetFieldMVA(iAttr);
	else
	{
		assert ( tMvaContainer.m_tContainer[iAttr] );
		pMva = tMvaContainer.m_tContainer[iAttr]->Find(tDocId);
	}

	if ( pMva )
		SortMva ( *pMva, ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET ) );

	return pMva;
}


bool CSphIndex_VLN::Build_StoreBlobAttrs ( DocID_t tDocId, SphOffset_t & tOffset, BlobRowBuilder_i & tBlobRowBuilder, QueryMvaContainer_c & tMvaContainer, AttrSource_i & tSource, bool bForceSource )
{
	CSphString sError;
	int iBlobAttr = 0;

	for ( int i=0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);

		if ( !sphIsBlobAttr(tAttr) )
			continue;

		bool bOk = true;
		bool bFatal = false;

		switch ( tAttr.m_eAttrType )
		{
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
			{
				const CSphVector<int64_t> * pMva = FetchMVA ( tDocId, i, tAttr, tMvaContainer, tSource, bForceSource );
				bOk = tBlobRowBuilder.SetAttr ( iBlobAttr++, pMva ? (const BYTE*)(pMva->Begin()) : nullptr, pMva ? pMva->GetLength()*sizeof(int64_t) : 0, sError );
			}
			break;

		case SPH_ATTR_STRING:
		case SPH_ATTR_JSON:
			{
				const CSphString & sStrAttr = tSource.GetStrAttr(i);
				bOk = tBlobRowBuilder.SetAttr ( iBlobAttr++, (const BYTE*)sStrAttr.cstr(), sStrAttr.Length(), sError );
				if ( !bOk )
					bFatal = tAttr.m_eAttrType==SPH_ATTR_JSON && g_bJsonStrict;
			}
			break;

		default:
			break;
		}

		if ( !bOk )
		{
			sError.SetSprintf ( "document " INT64_FMT ", attribute %s: %s", tDocId, tAttr.m_sName.cstr(), sError.cstr() );
			if ( bFatal )
			{
				m_sLastError = sError;
				return false;
			}
			else
				sphWarning ( "%s", sError.cstr() );
		}
	}

	tOffset = tBlobRowBuilder.Flush();
	return true;
}


void CSphIndex_VLN::Build_StoreColumnarAttrs ( DocID_t tDocId, columnar::Builder_i & tBuilder, CSphSource & tSource, QueryMvaContainer_c & tMvaContainer )
{
	int iColumnarAttrId = 0;
	for ( int i=0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		if ( !tAttr.IsColumnar() )
			continue;

		switch ( tAttr.m_eAttrType )
		{
		case SPH_ATTR_STRING:
			{
				const CSphString & sStrAttr = tSource.GetStrAttr(i);
				tBuilder.SetAttr ( iColumnarAttrId, (const BYTE*)sStrAttr.cstr(), sStrAttr.Length() );
			}
			break;

		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
			{
				const CSphVector<int64_t> * pMva = FetchMVA ( tDocId, i, tAttr, tMvaContainer, tSource, false );
				tBuilder.SetAttr ( iColumnarAttrId, pMva ? pMva->Begin() : nullptr, pMva ? pMva->GetLength() : 0 );
			}
			break;

		default:
			tBuilder.SetAttr ( iColumnarAttrId, tSource.GetAttr(i) );
			break;
		}

		iColumnarAttrId++;
	}
}


void CSphIndex_VLN::Build_StoreHistograms ( CSphVector<std::pair<Histogram_i*,int>> dHistograms, CSphSource & tSource )
{
	for ( auto & i : dHistograms )
		i.first->Insert ( tSource.GetAttr ( i.second ) );
}

template <typename T>
void SourceCopyMva ( const BYTE * pData, int iLenBytes, CSphVector<int64_t> & dDst )
{
	const T * pSrc = (const T *)pData;
	int iValues = iLenBytes / sizeof(T);

	dDst.Resize ( iValues );
	int64_t * pDst = dDst.Begin();
	const int64_t * pDstEnd = pDst + iValues;
	while ( pDst<pDstEnd )
	{
		*pDst = sphUnalignedRead ( *pSrc );
		pSrc++;
		pDst++;
	}
}


static void ResetFileAccess ( CSphIndex * pIndex )
{
	MutableIndexSettings_c tMutable = pIndex->GetMutableSettings();
	tMutable.m_tFileAccess = FileAccessSettings_t();
	pIndex->SetMutableSettings ( tMutable );
}

class KeepAttrs_c : public AttrSource_i
{
public:
	explicit KeepAttrs_c ( QueryMvaContainer_c & tMvaContainer )
		: m_pIndex ( nullptr )
		, m_tMvaContainer ( tMvaContainer )
	{}

	void SetBlobSource ( AttrSource_i * pSource )
	{
		m_pBlobSource = pSource;
	}

	bool Init ( const CSphString & sKeepAttrs, const StrVec_t & dKeepAttrs, const CSphSchema & tSchema )
	{
		if ( sKeepAttrs.IsEmpty() && !dKeepAttrs.GetLength() )
			return false;

		m_pBlobRowLocator = tSchema.GetAttr ( sphGetBlobLocatorName() );
		m_bHasBlobAttrs = tSchema.HasBlobAttrs();
		m_iStride = tSchema.GetRowSize();

		CSphString sError;
		StrVec_t dWarnings;

		m_pIndex = (CSphIndex_VLN *)sphCreateIndexPhrase ( "keep-attrs", sKeepAttrs.cstr() );
		ResetFileAccess ( m_pIndex.Ptr() );

		if ( !m_pIndex->Prealloc ( false, nullptr, dWarnings ) )
		{
			if ( !m_pIndex->GetLastError().IsEmpty() )
				sError.SetSprintf ( "%s error: '%s'", sError.scstr(), m_pIndex->GetLastError().cstr() );

			sphWarn ( "unable to load 'keep-attrs' index (%s); ignoring --keep-attrs", sError.cstr() );

			m_pIndex.Reset();
		} else
		{
			// check schema
			if ( !tSchema.CompareTo ( m_pIndex->GetMatchSchema(), sError, false ) )
			{
				sphWarn ( "schemas are different (%s); ignoring --keep-attrs", sError.cstr() );
				m_pIndex.Reset();
			}
		}

		for ( const auto & i : dWarnings )
			sphWarn ( "%s", i.cstr() );

		if ( m_pIndex.Ptr() )
		{
			if ( dKeepAttrs.GetLength() )
			{
				m_dLocMva.Init ( tSchema.GetAttrsCount() );
				m_dLocString.Init ( tSchema.GetAttrsCount() );
				m_bKeepSomeAttrs = true;

				ARRAY_FOREACH ( i, dKeepAttrs )
				{
					int iCol = tSchema.GetAttrIndex ( dKeepAttrs[i].cstr() );
					if ( iCol==-1 )
					{
						sphWarn ( "no attribute found '%s'; ignoring --keep-attrs", dKeepAttrs[i].cstr() );
						m_pIndex.Reset();
						break;
					}

					const CSphColumnInfo & tCol = tSchema.GetAttr ( iCol );
					if ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET )
					{
						m_dLocMva.BitSet ( iCol );
						m_bHasMva = true;
					} else if ( tCol.m_eAttrType==SPH_ATTR_STRING || tCol.m_eAttrType==SPH_ATTR_JSON )
					{
						m_dLocString.BitSet ( iCol );
						m_bHasString = true;
					} else
					{
						m_dLocPlain.Add ( tCol.m_tLocator );
					}
				}
			}

			m_dMvaField.Init ( tSchema.GetAttrsCount() );
			for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
			{
				const CSphColumnInfo & tCol = tSchema.GetAttr ( i );
				if ( ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) && tCol.m_eSrc==SPH_ATTRSRC_FIELD )
				{
					m_dMvaField.BitSet ( i );
				}
			}
		}

		if ( m_pIndex.Ptr() )
			m_pIndex->Preread();

		return ( m_pIndex.Ptr()!=nullptr );
	}

	bool Keep ( DocID_t tDocid )
	{
		if ( !m_pIndex.Ptr() )
			return false;

		m_tDocid = tDocid;
		m_pRow = m_pIndex->FindDocinfo ( tDocid );
		return ( m_pRow!=nullptr );
	}

	SphAttr_t GetAttr ( int iAttr ) override
	{
		assert ( 0 && "internal error" );
		return 0;
	}

	CSphVector<int64_t> * GetFieldMVA ( int iAttr ) override
	{
		if ( !m_pIndex.Ptr() || !m_pRow )
			return nullptr;

		// fallback to indexed data
		if ( m_bKeepSomeAttrs && ( !m_bHasMva || !m_dLocMva.BitGet ( iAttr ) ) )
		{
			if ( m_dMvaField.BitGet ( iAttr ) )
			{
				return m_pBlobSource->GetFieldMVA ( iAttr );
			} else
			{
				assert ( m_tMvaContainer.m_tContainer[iAttr] );
				return m_tMvaContainer.m_tContainer[iAttr]->Find ( m_tDocid );
			}
		}

		int iLen = 0;
		ESphAttr eAttr = SPH_ATTR_NONE;
		const BYTE * pData = GetBlobData ( iAttr, iLen, eAttr );
		assert ( eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET );

		if ( eAttr==SPH_ATTR_INT64SET )
		{
			SourceCopyMva<int64_t> ( pData, iLen, m_dDataMva );
		} else
		{
			SourceCopyMva<DWORD> ( pData, iLen, m_dDataMva );
		}

		return &m_dDataMva;
	}

	/// returns string attributes for a given attribute
	virtual const CSphString & GetStrAttr ( int iAttr ) override
	{
		if ( !m_pIndex.Ptr() || !m_pRow )
			return m_sEmpty;

		// fallback to indexed data
		if ( m_bKeepSomeAttrs && ( !m_bHasString || !m_dLocString.BitGet ( iAttr ) ) )
		{
			assert ( m_pBlobSource );
			return m_pBlobSource->GetStrAttr ( iAttr );
		}

		int iLen = 0;
		ESphAttr eAttr = SPH_ATTR_NONE;
		const BYTE * pData = GetBlobData ( iAttr, iLen, eAttr );
		assert ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_JSON );

		if ( !iLen )
			return m_sEmpty;

		m_sDataString.SetBinary ( (const char *)pData, iLen );
		return m_sDataString;
	}

	const CSphRowitem * GetRow ( CSphRowitem * pSrc )
	{
		if ( !m_pIndex.Ptr() || !m_pRow )
			return pSrc;

		// keep only blob attributes
		if ( m_bKeepSomeAttrs && !m_dLocPlain.GetLength() )
			return pSrc;

		if ( m_bKeepSomeAttrs )
		{
			// keep only some plain attributes
			ARRAY_FOREACH ( i, m_dLocPlain )
			{
				const CSphAttrLocator & tLoc = m_dLocPlain[i];
				SphAttr_t tAtrr = sphGetRowAttr ( m_pRow, tLoc );
				sphSetRowAttr ( pSrc, tLoc, tAtrr );
			}
			return pSrc;
		}
		else if ( m_bHasBlobAttrs )
		{
			// copy whole row except blob row offset
			assert(m_pBlobRowLocator);
			SphOffset_t tOffset = sphGetRowAttr ( pSrc, m_pBlobRowLocator->m_tLocator );
			memcpy ( pSrc, m_pRow, m_iStride*sizeof(CSphRowitem) );
			sphSetRowAttr ( pSrc, m_pBlobRowLocator->m_tLocator, tOffset );
			return pSrc;
		}

		// keep whole row
		return m_pRow;
	}

	void Reset()
	{
		m_pIndex.Reset();
	}

private:
	CSphScopedPtr<CSphIndex_VLN>	m_pIndex;
	CSphVector<CSphAttrLocator>		m_dLocPlain;
	CSphBitvec						m_dLocMva;
	CSphBitvec						m_dMvaField;
	CSphBitvec						m_dLocString;
	const CSphColumnInfo *			m_pBlobRowLocator = nullptr;

	bool							m_bKeepSomeAttrs = false;
	bool							m_bHasMva = false;
	bool							m_bHasString = false;
	bool							m_bHasBlobAttrs = false;

	int								m_iStride = 0;
	const CSphRowitem *				m_pRow = nullptr;
	CSphVector<int64_t>				m_dDataMva;
	CSphString						m_sDataString;
	const CSphString				m_sEmpty = "";

	AttrSource_i *					m_pBlobSource = nullptr;
	DocID_t							m_tDocid = 0;
	QueryMvaContainer_c &			m_tMvaContainer;


	const BYTE * GetBlobData ( int iAttr, int & iLen, ESphAttr & eAttr )
	{
		const BYTE * pPool = m_pIndex->m_tBlobAttrs.GetWritePtr();
		const CSphColumnInfo & tCol = m_pIndex->GetMatchSchema().GetAttr ( iAttr );
		assert ( tCol.m_tLocator.IsBlobAttr() );
		eAttr = tCol.m_eAttrType;

		return sphGetBlobAttr ( m_pRow, tCol.m_tLocator, pPool, iLen );
	}
};


void WarnAboutKillList ( const CSphVector<DocID_t> & dKillList, const KillListTargets_c & tTargets )
{
	if ( dKillList.GetLength() && !tTargets.m_dTargets.GetLength() )
		sphWarn ( "kill-list not empty but no killlist_target specified" );

	if ( !dKillList.GetLength() )
	{
		for ( const auto & tTarget : tTargets.m_dTargets )
			if ( tTarget.m_uFlags==KillListTarget_t::USE_KLIST )
			{
				sphWarn ( "killlist_target is specified, but kill-list is empty" );
				break;
			}
	}
}


bool CSphIndex_VLN::SortDocidLookup ( int iFD, int nBlocks, int iMemoryLimit, int nLookupsInBlock, int nLookupsInLastBlock, CSphIndexProgress & tProgress )
{
	tProgress.PhaseBegin ( CSphIndexProgress::PHASE_LOOKUP );
	assert (!tProgress.m_iDocids);
	tProgress.m_iDocidsTotal = m_tStats.m_iTotalDocuments;

	if ( !nBlocks )
		return true;

	DocidLookupWriter_c tWriter ( (DWORD)m_tStats.m_iTotalDocuments );
	if ( !tWriter.Open ( GetIndexFileName(SPH_EXT_SPT), m_sLastError ) )
		return false;

	DeleteOnFail_c tWatchdog;
	tWatchdog.AddWriter ( &tWriter.GetWriter() );

	RawVector_T<CSphBin> dBins;
	SphOffset_t iSharedOffset = -1;

	dBins.Reserve ( nBlocks );

	int iBinSize = CSphBin::CalcBinSize ( iMemoryLimit, nBlocks, "sort_lookup" );

	dBins.Reserve_static (nBlocks);
	for ( int i=0; i<nBlocks; ++i )
	{
		auto& dBin = dBins.Add();
		dBin.m_iFileLeft = ( ( i==nBlocks-1 ) ? nLookupsInLastBlock : nLookupsInBlock )*sizeof(DocidRowidPair_t);
		dBin.m_iFilePos = ( i==0 ) ? 0 : dBins[i-1].m_iFilePos + dBins[i-1].m_iFileLeft;
		dBin.Init ( iFD, &iSharedOffset, iBinSize );
	}

	CSphFixedVector<DocidRowidPair_t> dTopDocIDs ( nBlocks );
	CSphQueue<int, CmpQueuedLookup_fn> tLookupQueue ( nBlocks );

	CmpQueuedLookup_fn::m_pStorage = dTopDocIDs.Begin();

	for ( int i=0; i<nBlocks; ++i )
	{
		if ( dBins[i].ReadBytes ( &dTopDocIDs[i], sizeof(DocidRowidPair_t) )!=BIN_READ_OK )
		{
			m_sLastError.SetSprintf ( "sort_lookup: warmup failed (io error?)" );
			return false;
		}

		tLookupQueue.Push(i);
	}

	DWORD tProcessed = 0;
	while ( tLookupQueue.GetLength() )
	{
		int iBin = tLookupQueue.Root();

		tWriter.AddPair ( dTopDocIDs[iBin] );

		tLookupQueue.Pop();
		ESphBinRead eRes = dBins[iBin].ReadBytes ( &dTopDocIDs[iBin], sizeof(DocidRowidPair_t) );
		if ( eRes==BIN_READ_ERROR )
		{
			m_sLastError.SetSprintf ( "sort_lookup: failed to read entry" );
			return false;
		}

		if ( eRes==BIN_READ_OK )
			tLookupQueue.Push(iBin);

		tProcessed++;
		if ( ( tProcessed % 10000 )==0 )
		{
			tProgress.m_iDocids = tProcessed;
			tProgress.Show ( );
		}
	}

	if ( !tWriter.Finalize ( m_sLastError ) )
		return false;

	// clean up readers
	dBins.Reset();

	tWatchdog.AllIsDone();
	tProgress.m_iDocids = tProgress.m_iDocidsTotal;
	tProgress.PhaseEnd();
	return true;
}


bool CSphIndex_VLN::Build_SetupInplace ( SphOffset_t & iHitsGap, int iHitsMax, int iFdHits ) const
{
	if ( !m_bInplaceSettings )
		return true;

	const int HIT_SIZE_AVG = 4;
	const float HIT_BLOCK_FACTOR = 1.0f;

	if ( m_iHitGap )
		iHitsGap = (SphOffset_t) m_iHitGap;
	else
		iHitsGap = (SphOffset_t)( iHitsMax*HIT_BLOCK_FACTOR*HIT_SIZE_AVG );

	iHitsGap = Max ( iHitsGap, 1 );
	if ( !SeekAndWarn ( iFdHits, iHitsGap, "CSphIndex_VLN::Build" ))
		return false;

	return true;
}


void SetupDocstoreFields ( DocstoreAddField_i & tFields, const CSphSchema & tSchema )
{
	int iStored = 0;
	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
		if ( tSchema.IsFieldStored(i) )
		{
			tFields.AddField ( tSchema.GetFieldName(i), DOCSTORE_TEXT );
			iStored++;
		}

	assert(iStored);
}


bool CheckStoredFields ( const CSphSchema & tSchema, const CSphIndexSettings & tSettings, CSphString & sError )
{
	for ( const auto & i : tSettings.m_dStoredFields )
		if ( tSchema.GetAttr ( i.cstr() ) )
		{
			sError.SetSprintf ( "existing attribute specified in stored_fields: '%s'\n", i.cstr() );
			return false;
		}

	for ( const auto & i : tSettings.m_dStoredOnlyFields )
		if ( tSchema.GetAttr ( i.cstr() ) )
		{
			sError.SetSprintf ( "existing attribute specified in stored_fields: '%s'\n", i.cstr() );
			return false;
		}

	return true;
}


bool CSphIndex_VLN::Build_SetupDocstore ( CSphScopedPtr<DocstoreBuilder_i> & pDocstore )
{
	if ( !m_tSchema.HasStoredFields() )
		return true;

	DocstoreBuilder_i * pBuilder = CreateDocstoreBuilder ( GetIndexFileName(SPH_EXT_SPDS), GetSettings(), m_sLastError );
	if ( !pBuilder )
		return false;

	SetupDocstoreFields ( *pBuilder, m_tSchema );

	pDocstore = pBuilder;
	return true;
}


bool CSphIndex_VLN::Build_SetupBlobBuilder ( CSphScopedPtr<BlobRowBuilder_i> & pBuilder )
{
	if ( !m_tSchema.HasBlobAttrs() )
		return true;

	pBuilder = sphCreateBlobRowBuilder ( m_tSchema, GetIndexFileName ( SPH_EXT_SPB, true ), m_tSettings.m_tBlobUpdateSpace, m_sLastError );
	return !!pBuilder.Ptr();
}


bool CSphIndex_VLN::Build_SetupColumnar ( CSphScopedPtr<columnar::Builder_i> & pBuilder )
{
	if ( !m_tSchema.HasColumnarAttrs() )
		return true;

	pBuilder = CreateColumnarBuilder ( m_tSchema, m_tSettings, GetIndexFileName ( SPH_EXT_SPC, true ), m_sLastError );
	return !!pBuilder.Ptr();
}


bool CSphIndex_VLN::Build_SetupHistograms ( CSphScopedPtr<HistogramContainer_c> & pContainer, CSphVector<std::pair<Histogram_i*,int>> & dHistograms )
{
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		Histogram_i * pHistogram = CreateHistogram ( tAttr.m_sName, tAttr.m_eAttrType );
		if ( pHistogram )
			dHistograms.Add ( { pHistogram, i } );
	}

	if ( !dHistograms.GetLength() )
		return true;

	pContainer = new HistogramContainer_c;
	for ( const auto & i : dHistograms )
		Verify ( pContainer->Add ( i.first ) );

	return true;
}


void CSphIndex_VLN::Build_AddToDocstore ( DocstoreBuilder_i * pDocstoreBuilder, CSphSource * pSource )
{
	if ( !pDocstoreBuilder )
		return;

	DocstoreBuilder_i::Doc_t tDoc;
	pSource->GetDocFields ( tDoc.m_dFields );

	assert ( tDoc.m_dFields.GetLength()==m_tSchema.GetFieldsCount() );

	// filter out non-hl fields (should already be null)
	int iField = 0;
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
	{
		if ( !m_tSchema.IsFieldStored(i) )
			tDoc.m_dFields.Remove(iField);
		else
			iField++;
	}

	pDocstoreBuilder->AddDoc ( pSource->m_tDocInfo.m_tRowID, tDoc );
}


int CSphIndex_VLN::Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer, CSphIndexProgress& tProgress )
{
	assert ( dSources.GetLength() );

	CSphVector<SphWordID_t> dHitlessWords;

	if ( !LoadHitlessWords ( m_tSettings.m_sHitlessFiles, m_pTokenizer, m_pDict, dHitlessWords, m_sLastError ) )
		return 0;

	// vars shared between phases
	RawVector_T<CSphBin> dBins;
	SphOffset_t iSharedOffset = -1;

	m_pDict->HitblockBegin();

	// setup sources
	ARRAY_FOREACH ( iSource, dSources )
	{
		CSphSource * pSource = dSources[iSource];
		assert ( pSource );

		pSource->SetDict ( m_pDict );
		pSource->Setup ( m_tSettings, nullptr );
	}

	// connect 1st source and fetch its schema
	// and don't disconnect it because some sources can't survive connect/disconnect
	CSphSource * pSource0 = dSources[0];
	if ( !pSource0->Connect ( m_sLastError )
		|| !pSource0->IterateStart ( m_sLastError )
		|| !pSource0->UpdateSchema ( &m_tSchema, m_sLastError )
		|| !pSource0->SetupMorphFields ( m_sLastError ) )
	{
		return 0;
	}

	if ( !CheckStoredFields ( m_tSchema, m_tSettings, m_sLastError ) )
		return 0;

	bool bHaveQueryMVAs = false;
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		if ( ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET || tAttr.m_eAttrType==SPH_ATTR_INT64SET ) && tAttr.m_eSrc!=SPH_ATTRSRC_FIELD )
		{
			bHaveQueryMVAs = true;
			break;
		}
	}

	// non-SQL sources don't survive connect+disconnect+reconnect
	// and if we don't disconnect now we won't be able to fetch query MVAs from SQL sources
	QueryMvaContainer_c tQueryMvaContainer;
	if ( bHaveQueryMVAs )
	{
		pSource0->Disconnect();

		// temporary storage for MVAs that we fetch from queries
		// we might want to dump that to file later
		if ( !CollectQueryMvas ( dSources, tQueryMvaContainer ) )
			return 0;
	}

	if ( m_tSchema.GetFieldsCount()==0 )
	{
		m_sLastError.SetSprintf ( "No fields in schema - will not index" );
		return 0;
	}

	int iFieldLens = m_tSchema.GetAttrId_FirstFieldLen();

	const CSphColumnInfo * pBlobLocatorAttr = m_tSchema.GetAttr ( sphGetBlobLocatorName() );

	if ( !m_pTokenizer->SetFilterSchema ( m_tSchema, m_sLastError ) )
		return 0;

	int iHitBuilderBufferSize = ( iWriteBuffer>0 )
		? Max ( iWriteBuffer, MIN_WRITE_BUFFER )
		: DEFAULT_WRITE_BUFFER;

	CSphHitBuilder tHitBuilder ( m_tSettings, dHitlessWords, false, iHitBuilderBufferSize, m_pDict, &m_sLastError );

	////////////////////////////////////////////////
	// collect and partially sort hits
	////////////////////////////////////////////////

	CSphVector <DocID_t> dKillList;

	// adjust memory requirements
	int iOldLimit = iMemoryLimit;

	// book at least 2 MB for keywords dict, if needed
	int iDictSize = 0;
	if ( m_pDict->GetSettings().m_bWordDict )
		iDictSize = Max ( MIN_KEYWORDS_DICT, iMemoryLimit/8 );

	// reserve for sorting docid-rowid pairs
	int iDocidLookupSize = Max ( 32768, iMemoryLimit/16 );

	iMemoryLimit -= iDocidLookupSize+iDictSize;

	// do we have enough left for hits?
	int iHitsMax = 1048576;
	if ( iMemoryLimit < iHitsMax*(int)sizeof(CSphWordHit) )
	{
		iMemoryLimit = iOldLimit + iHitsMax*sizeof(CSphWordHit) - iMemoryLimit;
		sphWarn ( "collect_hits: mem_limit=%d kb too low, increasing to %d kb",	iOldLimit/1024, iMemoryLimit/1024 );
	} else
		iHitsMax = iMemoryLimit / sizeof(CSphWordHit);

	// allocate raw hits block
	CSphFixedVector<CSphWordHit> dHits ( iHitsMax + MAX_SOURCE_HITS );
	CSphWordHit * pHits = dHits.Begin();
	CSphWordHit * pHitsMax = dHits.Begin() + iHitsMax;

	int nDocidLookupsPerBlock = iDocidLookupSize/sizeof(DocidRowidPair_t);
	int nDocidLookup = 0;
	int nDocidLookupBlocks = 0;
	CSphFixedVector<DocidRowidPair_t> dDocidLookup ( nDocidLookupsPerBlock );

	// fallback blob source (for mva)
	KeepAttrs_c tPrevAttrs ( tQueryMvaContainer );
	const bool bGotPrevIndex = tPrevAttrs.Init ( m_sKeepAttrs, m_dKeepAttrs, m_tSchema );

	// create temp files
	CSphString sFileSPP = m_bInplaceSettings ? GetIndexFileName(SPH_EXT_SPP) : GetIndexFileName("tmp1");

	CSphAutofile fdLock ( GetIndexFileName("tmp0"), SPH_O_NEW, m_sLastError, true );
	CSphAutofile fdHits ( sFileSPP, SPH_O_NEW, m_sLastError, !m_bInplaceSettings );
	CSphAutofile fdTmpLookup ( GetIndexFileName("tmp2"), SPH_O_NEW, m_sLastError, true );

	CSphWriter tWriterSPA;
	bool bHaveNonColumnarAttrs = m_tSchema.HasNonColumnarAttrs();

	// write to temp file because of possible --keep-attrs option which loads prev index
	if ( bHaveNonColumnarAttrs && !tWriterSPA.OpenFile ( GetIndexFileName ( SPH_EXT_SPA, true ), m_sLastError ) )
		return 0;

	DeleteOnFail_c dFileWatchdog;

	if ( m_bInplaceSettings )
		dFileWatchdog.AddAutofile ( &fdHits );

	if ( fdLock.GetFD()<0 || fdHits.GetFD()<0 )
		return 0;

	CSphScopedPtr<BlobRowBuilder_i> pBlobRowBuilder(nullptr);
	if ( !Build_SetupBlobBuilder(pBlobRowBuilder) )
		return 0;

	CSphScopedPtr<columnar::Builder_i> pColumnarBuilder(nullptr);
	if ( !Build_SetupColumnar(pColumnarBuilder) )
		return 0;

	CSphScopedPtr<DocstoreBuilder_i> pDocstoreBuilder(nullptr);
	if ( !Build_SetupDocstore(pDocstoreBuilder) )
		return 0;

	CSphScopedPtr<HistogramContainer_c> pHistogramContainer(nullptr);
	CSphVector<std::pair<Histogram_i*,int>> dHistograms;
	if ( !Build_SetupHistograms ( pHistogramContainer, dHistograms ) )
		return 0;

	SphOffset_t iHitsGap = 0;
	if ( !Build_SetupInplace ( iHitsGap, iHitsMax, fdHits.GetFD() ) )
		return 0;

	if ( !sphLockEx ( fdLock.GetFD(), false ) )
	{
		m_sLastError.SetSprintf ( "failed to lock '%s': another indexer running?", fdLock.GetFilename() );
		return 0;
	}

	m_tStats.Reset ();
	tProgress.PhaseBegin ( CSphIndexProgress::PHASE_COLLECT );

	CSphVector<int> dHitBlocks;
	dHitBlocks.Reserve ( 1024 );

	AttrIndexBuilder_c tMinMax(m_tSchema);
	RowID_t tRowID = 0;
	int64_t iHitsTotal = 0;

	ARRAY_FOREACH ( iSource, dSources )
	{
		// connect and check schema
		CSphSource * pSource = dSources[iSource];

		bool bNeedToConnect = iSource>0 || bHaveQueryMVAs;
		if ( bNeedToConnect )
		{
			if ( !pSource->Connect ( m_sLastError )
				|| !pSource->IterateStart ( m_sLastError )
				|| !pSource->UpdateSchema ( &m_tSchema, m_sLastError )
				|| !pSource->SetupMorphFields ( m_sLastError ) )
			{
				return 0;
			}
		}

		// joined filter
		bool bGotJoined = pSource->HasJoinedFields();

		// fallback blob source (for string and json )
		if ( bGotPrevIndex )
			tPrevAttrs.SetBlobSource ( pSource );

		// fetch documents
		for ( ;; )
		{
			// get next doc, and handle errors
			bool bEOF = false;
			if ( !pSource->IterateDocument ( bEOF, m_sLastError ) )
				return 0;

			// check if we have no more documents
			if ( bEOF )
				break;

			pSource->m_tDocInfo.m_tRowID = tRowID++;
			DocID_t tDocID = pSource->GetAttr(0);

			pSource->RowIDAssigned ( tDocID, tRowID-1 );
			bool bKeepRow = ( bGotPrevIndex && tPrevAttrs.Keep ( tDocID ) );

			// show progress bar
			if ( ( pSource->GetStats().m_iTotalDocuments % 1000 )==0 )
			{
				tProgress.m_iDocuments = m_tStats.m_iTotalDocuments + pSource->GetStats().m_iTotalDocuments;
				tProgress.m_iBytes = m_tStats.m_iTotalBytes + pSource->GetStats().m_iTotalBytes;
				tProgress.Show();
			}

			// update crashdump
			g_iIndexerCurrentDocID = pSource->m_tDocInfo.m_tRowID;
			g_iIndexerCurrentHits = pHits-dHits.Begin();

			// store mva, strings and JSON blobs
			if ( pBlobRowBuilder.Ptr() )
			{
				AttrSource_i * pBlobSource = pSource;
				if ( bKeepRow )
					pBlobSource = &tPrevAttrs;
				SphOffset_t tBlobOffset = 0;
				if ( !Build_StoreBlobAttrs ( tDocID, tBlobOffset, *pBlobRowBuilder.Ptr(), tQueryMvaContainer, *pBlobSource, bKeepRow ) )
					return 0;

				pSource->m_tDocInfo.SetAttr ( pBlobLocatorAttr->m_tLocator, tBlobOffset );
			}

			// store anything columnar
			if ( pColumnarBuilder.Ptr() )
				Build_StoreColumnarAttrs ( tDocID, *pColumnarBuilder.Ptr(), *pSource, tQueryMvaContainer );

			Build_StoreHistograms ( dHistograms, *pSource );

			// store hits
			while ( const ISphHits * pDocHits = pSource->IterateHits ( m_sLastWarning ) )
			{
				int iDocHits = pDocHits->GetLength();
#if PARANOID
				for ( int i=0; i<iDocHits; i++ )
				{
					assert ( pDocHits->m_dData[i].m_uDocID==pSource->m_tDocInfo.m_uDocID );
					assert ( pDocHits->m_dData[i].m_uWordID );
					assert ( pDocHits->m_dData[i].m_iWordPos );
				}
#endif

				assert ( ( pHits+iDocHits )<=( pHitsMax+MAX_SOURCE_HITS ) );

				memcpy ( pHits, pDocHits->Begin(), iDocHits*sizeof(CSphWordHit) );
				pHits += iDocHits;

				// check if we need to flush
				if ( pHits<pHitsMax	&& !( iDictSize && m_pDict->HitblockGetMemUse() > iDictSize ) )
					continue;

				// update crashdump
				g_iIndexerPoolStartDocID = tDocID;
				g_iIndexerPoolStartHit = pHits-dHits.Begin();

				// sort hits
				int iHits = int ( pHits - dHits.Begin() );
				{
					sphSort ( dHits.Begin(), iHits, CmpHit_fn() );
					m_pDict->HitblockPatch ( dHits.Begin(), iHits );
				}
				pHits = dHits.Begin();

				// flush hits, docs are flushed independently
				dHitBlocks.Add ( tHitBuilder.cidxWriteRawVLB ( fdHits.GetFD(), dHits.Begin(), iHits ) );

				m_pDict->HitblockReset ();

				if ( dHitBlocks.Last()<0 )
					return 0;

				// progress bar
				iHitsTotal += iHits;
				tProgress.m_iDocuments = m_tStats.m_iTotalDocuments + pSource->GetStats().m_iTotalDocuments;
				tProgress.m_iBytes = m_tStats.m_iTotalBytes + pSource->GetStats().m_iTotalBytes;
				tProgress.Show();
			}

			// update total field lengths
			if ( iFieldLens>=0 )
			{
				for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
					m_dFieldLens[i] += pSource->m_tDocInfo.GetAttr ( m_tSchema.GetAttr ( i+iFieldLens ).m_tLocator );
			}

			if ( bHaveNonColumnarAttrs )
			{
				// store docinfo
				// with the advent of SPH_ATTR_TOKENCOUNT, now MUST be done AFTER iterating the hits
				// because field lengths are computed during that iterating
				const CSphRowitem * pRow = pSource->m_tDocInfo.m_pDynamic;
				if ( bKeepRow )
					pRow = tPrevAttrs.GetRow ( pSource->m_tDocInfo.m_pDynamic );
				tMinMax.Collect ( pRow );
				tWriterSPA.PutBytes ( pRow, sizeof(CSphRowitem)*m_tSchema.GetRowSize() );
			}

			dDocidLookup[nDocidLookup].m_tDocID = tDocID;
			dDocidLookup[nDocidLookup].m_tRowID = pSource->m_tDocInfo.m_tRowID;
			nDocidLookup++;

			if ( nDocidLookup==dDocidLookup.GetLength() )
			{
				dDocidLookup.Sort ( CmpDocidLookup_fn() );
				if ( !sphWriteThrottled ( fdTmpLookup.GetFD (), &dDocidLookup[0], nDocidLookup*sizeof(DocidRowidPair_t), "temp_docid_lookup", m_sLastError ) )
					return 0;

				nDocidLookup = 0;
				nDocidLookupBlocks++;
			}

			Build_AddToDocstore ( pDocstoreBuilder.Ptr(), pSource );

			// go on, loop next document
		}

		// FIXME! uncontrolled memory usage; add checks and/or diskbased sort in the future?
		if ( pSource->IterateKillListStart ( m_sLastError ) )
		{
			DocID_t tKilllistDocID;
			while ( pSource->IterateKillListNext ( tKilllistDocID ) )
				dKillList.Add ( tKilllistDocID );
		} else if ( !m_sLastError.IsEmpty() )
			return 0;

		// fetch joined fields
		if ( bGotJoined )
		{
			// flush tail of regular hits
			int iHits = int ( pHits - dHits.Begin() );
			if ( iDictSize && m_pDict->HitblockGetMemUse() && iHits )
			{
				sphSort ( dHits.Begin(), iHits, CmpHit_fn() );
				m_pDict->HitblockPatch ( dHits.Begin(), iHits );
				pHits = dHits.Begin();
				iHitsTotal += iHits;
				dHitBlocks.Add ( tHitBuilder.cidxWriteRawVLB ( fdHits.GetFD(), dHits.Begin(), iHits ) );
				if ( dHitBlocks.Last()<0 )
					return 0;
				m_pDict->HitblockReset ();
			}

			while (true)
			{
				// get next doc, and handle errors
				ISphHits * pJoinedHits = pSource->IterateJoinedHits ( m_sLastError );
				if ( !pJoinedHits )
					return 0;

				// check for eof
				if ( pSource->m_tDocInfo.m_tRowID==INVALID_ROWID )
					break;

				int iJoinedHits = pJoinedHits->GetLength();
				memcpy ( pHits, pJoinedHits->Begin(), iJoinedHits*sizeof(CSphWordHit) );
				pHits += iJoinedHits;

				// check if we need to flush
				if ( pHits<pHitsMax && !( iDictSize && m_pDict->HitblockGetMemUse() > iDictSize ) )
					continue;

				// store hits
				int iStoredHits = int ( pHits - dHits.Begin() );
				sphSort ( dHits.Begin(), iStoredHits, CmpHit_fn() );
				m_pDict->HitblockPatch ( dHits.Begin(), iStoredHits );

				pHits = dHits.Begin();
				iHitsTotal += iStoredHits;

				dHitBlocks.Add ( tHitBuilder.cidxWriteRawVLB ( fdHits.GetFD(), dHits.Begin(), iStoredHits ) );
				if ( dHitBlocks.Last()<0 )
					return 0;
				m_pDict->HitblockReset ();
			}
		}

		// this source is over, disconnect and update stats
		pSource->Disconnect ();

		m_tStats.m_iTotalDocuments += pSource->GetStats().m_iTotalDocuments;
		m_tStats.m_iTotalBytes += pSource->GetStats().m_iTotalBytes;
	}

	if ( m_tStats.m_iTotalDocuments>=INT_MAX )
	{
		m_sLastError.SetSprintf ( "index over %d documents not supported (got documents count=" INT64_FMT ")", INT_MAX, m_tStats.m_iTotalDocuments );
		return 0;
	}

	// flush last hit block
	if ( pHits>dHits.Begin() )
	{
		int iHits = int ( pHits - dHits.Begin() );
		{
			sphSort ( dHits.Begin(), iHits, CmpHit_fn() );
			m_pDict->HitblockPatch ( dHits.Begin(), iHits );
		}
		iHitsTotal += iHits;

		dHitBlocks.Add ( tHitBuilder.cidxWriteRawVLB ( fdHits.GetFD(), dHits.Begin(), iHits ) );

		m_pDict->HitblockReset ();

		if ( dHitBlocks.Last()<0 )
			return 0;
	}

	// reset hits pool
	dHits.Reset(0);

	if ( nDocidLookup )
	{
		dDocidLookup.Sort ( CmpDocidLookup_fn(), 0, nDocidLookup-1 );
		if ( !sphWriteThrottled ( fdTmpLookup.GetFD(), &dDocidLookup[0], nDocidLookup*sizeof(DocidRowidPair_t), "temp_docid_lookup", m_sLastError ) )
			return 0;

		nDocidLookupBlocks++;
	}

	dDocidLookup.Reset(0);

	tProgress.m_iDocuments = m_tStats.m_iTotalDocuments;
	tProgress.m_iBytes = m_tStats.m_iTotalBytes;
	tProgress.PhaseEnd();

	if ( bHaveNonColumnarAttrs )
	{
		if ( m_tStats.m_iTotalDocuments  )
		{
			tMinMax.FinishCollect();
			const CSphTightVector<CSphRowitem> & dMinMaxRows = tMinMax.GetCollected();
			tWriterSPA.PutBytes ( dMinMaxRows.Begin(), dMinMaxRows.GetLength()*sizeof(CSphRowitem) );

			m_iDocinfoIndex = ( dMinMaxRows.GetLength() / m_tSchema.GetRowSize() / 2 ) - 1;
		}

		tWriterSPA.CloseFile();
		if ( tWriterSPA.IsError() )
		{
			m_sLastError = "error writing .SPA";
			return false;
		}
	}

	if ( pBlobRowBuilder.Ptr() && !pBlobRowBuilder->Done(m_sLastError) )
		return 0;

	std::string sError;
	if ( pColumnarBuilder.Ptr() && !pColumnarBuilder->Done(sError) )
	{
		m_sLastError = sError.c_str();
		return 0;
	}

	if ( pHistogramContainer.Ptr() && !pHistogramContainer->Save ( GetIndexFileName(SPH_EXT_SPHI), m_sLastError ) )
		return 0;

	if ( bGotPrevIndex )
		tPrevAttrs.Reset();

	CSphString sSPA = GetIndexFileName(SPH_EXT_SPA);
	CSphString sSPATmp = GetIndexFileName ( SPH_EXT_SPA, true );
	if ( bHaveNonColumnarAttrs && sph::rename ( sSPATmp.cstr(), sSPA.cstr() )!=0 )
	{
		m_sLastError.SetSprintf ( "failed to rename %s to %s", sSPATmp.cstr(), sSPA.cstr() );
		return false;
	}

	CSphString sSPB = GetIndexFileName(SPH_EXT_SPB);
	CSphString sSPBTmp = GetIndexFileName ( SPH_EXT_SPB, true );
	if ( m_tSchema.HasBlobAttrs() && sph::rename ( sSPBTmp.cstr(), sSPB.cstr() )!=0 )
	{
		m_sLastError.SetSprintf ( "failed to rename %s to %s", sSPBTmp.cstr(), sSPB.cstr() );
		return false;
	}

	CSphString sSPC = GetIndexFileName(SPH_EXT_SPC);
	CSphString sSPCTmp = GetIndexFileName ( SPH_EXT_SPC, true );
	if ( m_tSchema.HasColumnarAttrs() && sph::rename ( sSPCTmp.cstr(), sSPC.cstr() )!=0 )
	{
		m_sLastError.SetSprintf ( "failed to rename %s to %s", sSPCTmp.cstr(), sSPC.cstr() );
		return false;
	}

	if ( !WriteDeadRowMap ( GetIndexFileName(SPH_EXT_SPM), (DWORD)m_tStats.m_iTotalDocuments, m_sLastError ) )
		return 0;

	dKillList.Uniq();
	if ( !WriteKillList ( GetIndexFileName(SPH_EXT_SPK), dKillList.Begin(), dKillList.GetLength(), m_tSettings.m_tKlistTargets, m_sLastError ) )
		return 0;

	if ( pDocstoreBuilder.Ptr() )
		pDocstoreBuilder->Finalize();

	WarnAboutKillList ( dKillList, m_tSettings.m_tKlistTargets );

	m_iMinMaxIndex = m_tStats.m_iTotalDocuments*m_tSchema.GetRowSize();
	if ( !SortDocidLookup ( fdTmpLookup.GetFD(), nDocidLookupBlocks, iMemoryLimit, nDocidLookupsPerBlock, nDocidLookup, tProgress ) )
		return 0;

	///////////////////////////////////
	// sort and write compressed index
	///////////////////////////////////

	// initialize readers
	assert ( dBins.IsEmpty() );
	dBins.Reserve_static ( dHitBlocks.GetLength() );

	iSharedOffset = -1;

	float fReadFactor = 1.0f;
	int iRelocationSize = 0;
	iWriteBuffer = iHitBuilderBufferSize;

	if ( m_bInplaceSettings )
	{
		assert ( m_fRelocFactor > 0.005f && m_fRelocFactor < 0.95f );
		assert ( m_fWriteFactor > 0.005f && m_fWriteFactor < 0.95f );
		assert ( m_fWriteFactor+m_fRelocFactor < 1.0f );

		fReadFactor -= m_fRelocFactor + m_fWriteFactor;

		iRelocationSize = int ( iMemoryLimit * m_fRelocFactor );
		iWriteBuffer = int ( iMemoryLimit * m_fWriteFactor );
	}

	int iBinSize = CSphBin::CalcBinSize ( int ( iMemoryLimit * fReadFactor ),
		dHitBlocks.GetLength() + m_pDict->GetSettings().m_bWordDict, "sort_hits" );

	CSphFixedVector <BYTE> dRelocationBuffer ( iRelocationSize );
	iSharedOffset = -1;

	ARRAY_FOREACH ( i, dHitBlocks )
	{
		dBins.Emplace_back ( m_tSettings.m_eHitless, m_pDict->GetSettings().m_bWordDict );
		dBins[i].m_iFileLeft = dHitBlocks[i];
		dBins[i].m_iFilePos = ( i==0 ) ? iHitsGap : dBins[i-1].m_iFilePos + dBins[i-1].m_iFileLeft;
		dBins[i].Init ( fdHits.GetFD(), &iSharedOffset, iBinSize );
	}

	// if there were no hits, create zero-length index files
	int iRawBlocks = dBins.GetLength();

	//////////////////////////////
	// create new index files set
	//////////////////////////////

	tHitBuilder.CreateIndexFiles ( GetIndexFileName(SPH_EXT_SPD).cstr(), GetIndexFileName(SPH_EXT_SPP).cstr(), GetIndexFileName(SPH_EXT_SPE).cstr(), m_bInplaceSettings, iWriteBuffer, fdHits, &iSharedOffset );

	// dict files
	CSphAutofile fdTmpDict ( GetIndexFileName("tmp8"), SPH_O_NEW, m_sLastError, true );
	CSphAutofile fdDict ( GetIndexFileName(SPH_EXT_SPI), SPH_O_NEW, m_sLastError, false );
	if ( fdTmpDict.GetFD()<0 || fdDict.GetFD()<0 )
		return 0;

	m_pDict->DictBegin ( fdTmpDict, fdDict, iBinSize );

	//////////////
	// final sort
	//////////////

	if ( iRawBlocks )
	{
		int iLastBin = dBins.GetLength () - 1;
		SphOffset_t iHitFileSize = dBins[iLastBin].m_iFilePos + dBins [iLastBin].m_iFileLeft;

		CSphHitQueue tQueue ( iRawBlocks );
		CSphAggregateHit tHit;

		// initialize hitlist encoder state
		tHitBuilder.HitReset();

		// initial fill
		CSphFixedVector<BYTE> dActive ( iRawBlocks );
		for ( int i=0; i<iRawBlocks; i++ )
		{
			if ( !dBins[i].ReadHit ( &tHit ) )
			{
				m_sLastError.SetSprintf ( "sort_hits: warmup failed (io error?)" );
				return 0;
			}
			dActive[i] = ( tHit.m_uWordID!=0 );
			if ( dActive[i] )
				tQueue.Push ( tHit, i );
		}

		// init progress meter
		tProgress.PhaseBegin ( CSphIndexProgress::PHASE_SORT );
		assert ( !tProgress.m_iHits );
		tProgress.m_iHitsTotal = iHitsTotal;

		// while the queue has data for us
		// FIXME! analyze binsRead return code
		int iHitsSorted = 0;
		int iMinBlock = -1;
		while ( tQueue.m_iUsed )
		{
			int iBin = tQueue.m_pData->m_iBin;

			// pack and emit queue root
			if ( m_bInplaceSettings )
			{
				if ( iMinBlock==-1 || dBins[iMinBlock].IsEOF () || !dActive[iMinBlock] )
				{
					iMinBlock = -1;
					ARRAY_FOREACH ( i, dBins )
						if ( !dBins[i].IsEOF () && dActive[i] && ( iMinBlock==-1 || dBins[i].m_iFilePos < dBins[iMinBlock].m_iFilePos ) )
							iMinBlock = i;
				}

				int iToWriteMax = 3*sizeof(DWORD);
				if ( iMinBlock!=-1 && ( tHitBuilder.GetHitfilePos() + iToWriteMax ) > dBins[iMinBlock].m_iFilePos )
				{
					if ( !RelocateBlock ( fdHits.GetFD (), dRelocationBuffer.Begin(), iRelocationSize, &iHitFileSize, dBins[iMinBlock], &iSharedOffset ) )
						return 0;

					iMinBlock = (iMinBlock+1) % dBins.GetLength ();
				}
			}

			tHitBuilder.cidxHit ( tQueue.m_pData );
			if ( tHitBuilder.IsError() )
				return 0;

			// pop queue root and push next hit from popped bin
			tQueue.Pop ();
			if ( dActive[iBin] )
			{
				dBins[iBin].ReadHit ( &tHit );
				dActive[iBin] = ( tHit.m_uWordID!=0 );
				if ( dActive[iBin] )
					tQueue.Push ( tHit, iBin );
			}

			// progress
			if ( ++iHitsSorted==1000000 )
			{
				tProgress.m_iHits += iHitsSorted;
				tProgress.Show();
				iHitsSorted = 0;
			}
		}

		tProgress.m_iHits = tProgress.m_iHitsTotal; // sum might be less than total because of dupes!
		tProgress.PhaseEnd();
		dBins.Reset ();

		CSphAggregateHit tFlush;
		tFlush.m_tRowID = INVALID_ROWID;
		tFlush.m_uWordID = 0;
		tFlush.m_sKeyword = NULL;
		tFlush.m_iWordPos = EMPTY_HIT;
		tFlush.m_dFieldMask.UnsetAll();
		tHitBuilder.cidxHit ( &tFlush );

		if ( m_bInplaceSettings )
		{
			tHitBuilder.CloseHitlist();
			if ( !sphTruncate ( fdHits.GetFD () ) )
				sphWarn ( "failed to truncate %s", fdHits.GetFilename() );
		}
	}

	BuildHeader_t tBuildHeader ( m_tStats );
	if ( !tHitBuilder.cidxDone ( iMemoryLimit, m_tSettings.m_iMinInfixLen, m_pTokenizer->GetMaxCodepointLength(), &tBuildHeader ) )
		return 0;

	dRelocationBuffer.Reset(0);

	tBuildHeader.m_iDocinfo = m_tStats.m_iTotalDocuments;
	tBuildHeader.m_iDocinfoIndex = m_iDocinfoIndex;
	tBuildHeader.m_iMinMaxIndex = m_iMinMaxIndex;

	CSphString sHeaderName = GetIndexFileName(SPH_EXT_SPH);
	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &m_tSettings;
	tWriteHeader.m_pSchema = &m_tSchema;
	tWriteHeader.m_pTokenizer = m_pTokenizer;
	tWriteHeader.m_pDict = m_pDict;
	tWriteHeader.m_pFieldFilter = m_pFieldFilter;
	tWriteHeader.m_pFieldLens = m_dFieldLens.Begin();

	// we're done
	if ( !IndexBuildDone ( tBuildHeader, tWriteHeader, sHeaderName, m_sLastError ) )
		return 0;

	// when the party's over..
	ARRAY_FOREACH ( i, dSources )
		dSources[i]->PostIndex ();

	dFileWatchdog.AllIsDone();
	return 1;
} // NOLINT function length


/////////////////////////////////////////////////////////////////////////////
// MERGER HELPERS
/////////////////////////////////////////////////////////////////////////////

BYTE sphDoclistHintPack ( SphOffset_t iDocs, SphOffset_t iLen )
{
	// we won't really store a hint for small lists
	if ( iDocs<DOCLIST_HINT_THRESH )
		return 0;

	// for bigger lists len/docs varies 4x-6x on test indexes
	// so lets assume that 4x-8x should be enough for everybody
	SphOffset_t iDelta = Min ( Max ( iLen-4*iDocs, 0 ), 4*iDocs-1 ); // len delta over 4x, clamped to [0x..4x) range
	BYTE uHint = (BYTE)( 64*iDelta/iDocs ); // hint now must be in [0..256) range
	while ( uHint<255 && ( iDocs*uHint/64 )<iDelta ) // roundoff (suddenly, my guru math skillz failed me)
		uHint++;

	return uHint;
}

// !COMMIT eliminate this, move to dict (or at least couple with CWordlist)
template<bool WORDDICT>
class CSphDictReader
{
public:
	// current word
	SphWordID_t		m_uWordID = 0;
	SphOffset_t		m_iDoclistOffset = 0;
	int				m_iDocs = 0;
	int				m_iHits = 0;
	bool			m_bHasHitlist = true;
	int				m_iHint = 0;

private:
	ESphHitless		m_eHitless { SPH_HITLESS_NONE };
	CSphAutoreader	m_tMyReader;
	CSphReader *	m_pReader = nullptr;
	SphOffset_t		m_iMaxPos = 0;
	int				m_iSkiplistBlockSize = 0;
	char			m_sWord[MAX_KEYWORD_BYTES];
	int				m_iCheckpoint = 1;

public:
	CSphDictReader ( int iSkiplistBlockSize )
		: m_iSkiplistBlockSize ( iSkiplistBlockSize )
	{
		m_sWord[0] = '\0';
	}

	bool Setup ( const CSphString & sFilename, SphOffset_t iMaxPos, ESphHitless eHitless, CSphString & sError )
	{
		if ( !m_tMyReader.Open ( sFilename, sError ) )
			return false;

		Setup ( &m_tMyReader, iMaxPos, eHitless );
		return true;
	}

	void Setup ( CSphReader * pReader, SphOffset_t iMaxPos, ESphHitless eHitless )
	{
		m_pReader = pReader;
		m_pReader->SeekTo ( 1, READ_NO_SIZE_HINT );

		m_iMaxPos = iMaxPos;
		m_eHitless = eHitless;
		m_sWord[0] = '\0';
		m_iCheckpoint = 1;
	}

	bool Read()
	{
		assert ( m_iSkiplistBlockSize>0 );

		if ( m_pReader->GetPos()>=m_iMaxPos )
			return false;

		// get leading value
		SphWordID_t iWord0 = WORDDICT ? m_pReader->GetByte() : m_pReader->UnzipWordid();
		if ( !iWord0 )
		{
			// handle checkpoint
			m_iCheckpoint++;
			m_pReader->UnzipOffset();

			m_uWordID = 0;
			m_iDoclistOffset = 0;
			m_sWord[0] = '\0';

			if ( m_pReader->GetPos()>=m_iMaxPos )
				return false;

			iWord0 = WORDDICT ? m_pReader->GetByte() : m_pReader->UnzipWordid(); // get next word
		}
		if ( !iWord0 )
			return false; // some failure

		// get word entry
		if_const ( WORDDICT )
		{
			// unpack next word
			// must be in sync with DictEnd()!
			assert ( iWord0<=255 );
			auto uPack = (BYTE) iWord0;

			int iMatch, iDelta;
			if ( uPack & 0x80 )
			{
				iDelta = ( ( uPack>>4 ) & 7 ) + 1;
				iMatch = uPack & 15;
			} else
			{
				iDelta = uPack & 127;
				iMatch = m_pReader->GetByte();
			}
			assert ( iMatch+iDelta<(int)sizeof(m_sWord)-1 );
			assert ( iMatch<=(int)strlen(m_sWord) );

			m_pReader->GetBytes ( m_sWord + iMatch, iDelta );
			m_sWord [ iMatch+iDelta ] = '\0';

			m_iDoclistOffset = m_pReader->UnzipOffset();
			m_iDocs = m_pReader->UnzipInt();
			m_iHits = m_pReader->UnzipInt();
			m_iHint = 0;
			if ( m_iDocs>=DOCLIST_HINT_THRESH )
				m_iHint = m_pReader->GetByte();
			if ( m_iDocs > m_iSkiplistBlockSize )
				m_pReader->UnzipInt();

			m_uWordID = (SphWordID_t) sphCRC32 ( GetWord() ); // set wordID for indexing

		} else
		{
			m_uWordID += iWord0;
			m_iDoclistOffset += m_pReader->UnzipOffset();
			m_iDocs = m_pReader->UnzipInt();
			m_iHits = m_pReader->UnzipInt();
			if ( m_iDocs > m_iSkiplistBlockSize )
				m_pReader->UnzipOffset();
		}

		m_bHasHitlist =
			( m_eHitless==SPH_HITLESS_NONE ) ||
			( m_eHitless==SPH_HITLESS_SOME && !( m_iDocs & HITLESS_DOC_FLAG ) );
		m_iDocs = m_eHitless==SPH_HITLESS_SOME ? ( m_iDocs & HITLESS_DOC_MASK ) : m_iDocs;

		return true; // FIXME? errorflag?
	}

	int CmpWord ( const CSphDictReader & tOther ) const
	{
		if_const ( WORDDICT )
			return strcmp ( m_sWord, tOther.m_sWord );
		return ( m_uWordID < tOther.m_uWordID ) ? -1 : ( m_uWordID == tOther.m_uWordID ? 0 : 1 );
	}

	BYTE * GetWord() const { return (BYTE *)const_cast<char*>(m_sWord); }

	int GetCheckpoint() const { return m_iCheckpoint; }
};


ISphFilter * CSphIndex_VLN::CreateMergeFilters ( const VecTraits_T<CSphFilterSettings> & dSettings ) const
{
	CSphString sError, sWarning;
	ISphFilter * pResult = nullptr;
	CreateFilterContext_t tCtx;
	tCtx.m_pSchema = &m_tSchema;
	tCtx.m_pBlobPool = m_tBlobAttrs.GetReadPtr();

	for ( const auto& dSetting : dSettings )
		pResult = sphJoinFilters ( pResult, sphCreateFilter ( dSetting, tCtx, sError, sWarning ) );

	if ( pResult )
		pResult->SetColumnar(m_pColumnar.Ptr());

	return pResult;
}

static bool CheckDocsCount ( int64_t iDocs, CSphString & sError )
{
	if ( iDocs<INT_MAX )
		return true;

	sError.SetSprintf ( "index over %d documents not supported (got " INT64_FMT " documents)", INT_MAX, iDocs );
	return false;
}

namespace QwordIteration
{
	template<typename QWORD>
	inline void PrepareQword ( QWORD & tQword, const CSphDictReader<QWORD::is_worddict::value> & tReader ) //NOLINT
	{
		tQword.m_tDoc.m_tRowID = INVALID_ROWID;

		tQword.m_iDocs = tReader.m_iDocs;
		tQword.m_iHits = tReader.m_iHits;
		tQword.m_bHasHitlist = tReader.m_bHasHitlist;

		tQword.m_uHitPosition = 0;
		tQword.m_iHitlistPos = 0;

		if_const ( QWORD::is_worddict::value )
			tQword.m_rdDoclist->SeekTo ( tReader.m_iDoclistOffset, tReader.m_iHint );
	}

	template<typename QWORD>
	inline bool NextDocument ( QWORD & tQword, const CSphIndex_VLN * pSourceIndex )
	{
		while (true)
		{
			tQword.GetNextDoc();
			if ( tQword.m_tDoc.m_tRowID==INVALID_ROWID )
				return false;

			tQword.SeekHitlist ( tQword.m_iHitlistPos );
			return true;
		}
	}

	template < typename QWORD >
	inline bool NextDocument ( QWORD & tQword, const CSphIndex_VLN * pSourceIndex, const VecTraits_T<RowID_t> & dRows )
	{
		while (true)
		{
			tQword.GetNextDoc();
			if ( tQword.m_tDoc.m_tRowID==INVALID_ROWID )
				return false;

			if ( dRows[tQword.m_tDoc.m_tRowID]==INVALID_ROWID )
				continue;

			tQword.SeekHitlist ( tQword.m_iHitlistPos );
			return true;
		}
	}

	template<typename QWORD>
	inline void ConfigureQword ( QWORD & tQword, DataReaderFactory_c * pHits, DataReaderFactory_c * pDocs, int iDynamic )
	{
		tQword.SetHitReader ( pHits );
		tQword.m_rdHitlist->SeekTo ( 1, READ_NO_SIZE_HINT );

		tQword.SetDocReader ( pDocs );
		tQword.m_rdDoclist->SeekTo ( 1, READ_NO_SIZE_HINT );

		tQword.m_tDoc.Reset ( iDynamic );
	}
}; // namespace QwordIteration

class CSphMerger
{
public:
	explicit CSphMerger ( CSphHitBuilder * pHitBuilder )
		: m_pHitBuilder ( pHitBuilder )
	{}

	template < typename QWORD >
	inline void TransferData ( QWORD & tQword, SphWordID_t iWordID, const BYTE * sWord,
							const CSphIndex_VLN * pSourceIndex, const VecTraits_T<RowID_t>& dRows,
							MergeCb_c & tMonitor )
	{
		CSphAggregateHit tHit;
		tHit.m_uWordID = iWordID;
		tHit.m_sKeyword = sWord;
		tHit.m_dFieldMask.UnsetAll();

		while ( QwordIteration::NextDocument ( tQword, pSourceIndex, dRows ) && !tMonitor.NeedStop() )
		{
			if ( tQword.m_bHasHitlist )
				TransferHits ( tQword, tHit, dRows );
			else
			{
				// convert to aggregate if there is no hit-list
				tHit.m_tRowID = dRows[tQword.m_tDoc.m_tRowID];
				tHit.m_dFieldMask = tQword.m_dQwordFields;
				tHit.SetAggrCount ( tQword.m_uMatchHits );
				m_pHitBuilder->cidxHit ( &tHit );
			}
		}
	}

	template < typename QWORD >
	inline void TransferHits ( QWORD & tQword, CSphAggregateHit & tHit, const VecTraits_T<RowID_t> & dRows )
	{
		assert ( tQword.m_bHasHitlist );
		tHit.m_tRowID = dRows[tQword.m_tDoc.m_tRowID];
		for ( Hitpos_t uHit = tQword.GetNextHit(); uHit!=EMPTY_HIT; uHit = tQword.GetNextHit() )
		{
			tHit.m_iWordPos = uHit;
			m_pHitBuilder->cidxHit ( &tHit );
		}
	}

private:
	CSphHitBuilder *	m_pHitBuilder;
};


template < typename QWORDDST, typename QWORDSRC >
bool CSphIndex_VLN::MergeWords ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, VecTraits_T<RowID_t> dDstRows, VecTraits_T<RowID_t> dSrcRows,
	CSphHitBuilder * pHitBuilder, CSphString & sError, CSphIndexProgress & tProgress )
{
	auto& tMonitor = tProgress.GetMergeCb();
	CSphAutofile tDummy;
	pHitBuilder->CreateIndexFiles ( pDstIndex->GetIndexFileName("tmp.spd").cstr(), pDstIndex->GetIndexFileName("tmp.spp").cstr(), pDstIndex->GetIndexFileName("tmp.spe").cstr(), false, 0, tDummy, nullptr );

	static_assert ( QWORDDST::is_worddict::value == QWORDDST::is_worddict::value, "can't merge worddict with non-worddict" );

	CSphDictReader<QWORDDST::is_worddict::value> tDstReader ( pDstIndex->GetSettings().m_iSkiplistBlockSize );
	CSphDictReader<QWORDSRC::is_worddict::value> tSrcReader ( pSrcIndex->GetSettings().m_iSkiplistBlockSize );

	/// compress means: I don't want true merge, I just want to apply deadrows and filter
	bool bCompress = pDstIndex==pSrcIndex;

	if ( !tDstReader.Setup ( pDstIndex->GetIndexFileName(SPH_EXT_SPI), pDstIndex->m_tWordlist.GetWordsEnd(), pDstIndex->m_tSettings.m_eHitless, sError ) )
		return false;
	if ( !bCompress && !tSrcReader.Setup ( pSrcIndex->GetIndexFileName(SPH_EXT_SPI), pSrcIndex->m_tWordlist.GetWordsEnd(), pSrcIndex->m_tSettings.m_eHitless, sError ) )
		return false;

	/// prepare for indexing
	pHitBuilder->HitblockBegin();
	pHitBuilder->HitReset();

	/// setup qwords

	QWORDDST tDstQword ( false, false );
	QWORDSRC tSrcQword ( false, false );

	DataReaderFactoryPtr_c tSrcDocs {
		NewProxyReader ( pSrcIndex->GetIndexFileName ( SPH_EXT_SPD ), sError,
			DataReaderFactory_c::DOCS, pSrcIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE )
	};
	if ( !tSrcDocs )
		return false;

	DataReaderFactoryPtr_c tSrcHits {
		NewProxyReader ( pSrcIndex->GetIndexFileName ( SPH_EXT_SPP ), sError,
			DataReaderFactory_c::HITS, pSrcIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE  )
	};
	if ( !tSrcHits )
		return false;

	if ( !sError.IsEmpty () || tMonitor.NeedStop () )
		return false;

	DataReaderFactoryPtr_c tDstDocs {
		NewProxyReader ( pDstIndex->GetIndexFileName ( SPH_EXT_SPD ), sError,
			DataReaderFactory_c::DOCS, pDstIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE )
	};
	if ( !tDstDocs )
		return false;

	DataReaderFactoryPtr_c tDstHits {
		NewProxyReader ( pDstIndex->GetIndexFileName ( SPH_EXT_SPP ), sError,
			DataReaderFactory_c::HITS, pDstIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE )
	};
	if ( !tDstHits )
		return false;

	if ( !sError.IsEmpty() || tMonitor.NeedStop () )
		return false;

	CSphMerger tMerger(pHitBuilder);

	QwordIteration::ConfigureQword<QWORDDST> ( tDstQword, tDstHits, tDstDocs, pDstIndex->m_tSchema.GetDynamicSize() );
	QwordIteration::ConfigureQword<QWORDSRC> ( tSrcQword, tSrcHits, tSrcDocs, pSrcIndex->m_tSchema.GetDynamicSize() );

	/// merge

	bool bDstWord = tDstReader.Read();
	bool bSrcWord = !bCompress && tSrcReader.Read();

	tProgress.PhaseBegin ( CSphIndexProgress::PHASE_MERGE );
	tProgress.Show();

	int iWords = 0;
	int iHitlistsDiscarded = 0;
	for ( ; bDstWord || bSrcWord; ++iWords )
	{
		if ( iWords==1000 )
		{
			tProgress.m_iWords += 1000;
			tProgress.Show();
			iWords = 0;
		}

		if ( tMonitor.NeedStop () )
			return false;

		const int iCmp = bCompress ? -1 : tDstReader.CmpWord ( tSrcReader );

		if ( !bSrcWord || ( bDstWord && iCmp<0 ) )
		{
			// transfer documents and hits from destination
			QwordIteration::PrepareQword<QWORDDST> ( tDstQword, tDstReader );
			tMerger.TransferData<QWORDDST> ( tDstQword, tDstReader.m_uWordID, tDstReader.GetWord(), pDstIndex, dDstRows, tMonitor );
			bDstWord = tDstReader.Read();

		} else if ( !bDstWord || ( bSrcWord && iCmp>0 ) )
		{
			// transfer documents and hits from source
			QwordIteration::PrepareQword<QWORDSRC> ( tSrcQword, tSrcReader );
			tMerger.TransferData<QWORDSRC> ( tSrcQword, tSrcReader.m_uWordID, tSrcReader.GetWord(), pSrcIndex, dSrcRows, tMonitor );
			bSrcWord = tSrcReader.Read();

		} else // merge documents and hits inside the word
		{
			assert ( iCmp==0 );

			bool bHitless = !tDstReader.m_bHasHitlist;
			if ( tDstReader.m_bHasHitlist!=tSrcReader.m_bHasHitlist )
			{
				++iHitlistsDiscarded;
				bHitless = true;
			}

			QwordIteration::PrepareQword<QWORDDST> ( tDstQword, tDstReader );
			QwordIteration::PrepareQword<QWORDSRC> ( tSrcQword, tSrcReader );

			CSphAggregateHit tHit;
			tHit.m_uWordID = tDstReader.m_uWordID; // !COMMIT m_sKeyword anyone?
			tHit.m_sKeyword = tDstReader.GetWord();
			tHit.m_dFieldMask.UnsetAll();

			// we assume that all the duplicates have been removed
			// and we don't need to merge hits from the same document

			// transfer hits from destination
			while ( QwordIteration::NextDocument ( tDstQword, pDstIndex, dDstRows ) )
			{
				if ( tMonitor.NeedStop () )
					return false;

				if ( bHitless )
				{
					while ( tDstQword.m_bHasHitlist && tDstQword.GetNextHit()!=EMPTY_HIT );

					tHit.m_tRowID = dDstRows[tDstQword.m_tDoc.m_tRowID];
					tHit.m_dFieldMask = tDstQword.m_dQwordFields;
					tHit.SetAggrCount ( tDstQword.m_uMatchHits );
					pHitBuilder->cidxHit ( &tHit );
				} else
					tMerger.TransferHits ( tDstQword, tHit, dDstRows );
			}

			// transfer hits from source
			while ( QwordIteration::NextDocument ( tSrcQword, pSrcIndex, dSrcRows ) )
			{
				if ( tMonitor.NeedStop () )
					return false;

				if ( bHitless )
				{
					while ( tSrcQword.m_bHasHitlist && tSrcQword.GetNextHit()!=EMPTY_HIT );

					tHit.m_tRowID = dSrcRows[tSrcQword.m_tDoc.m_tRowID];
					tHit.m_dFieldMask = tSrcQword.m_dQwordFields;
					tHit.SetAggrCount ( tSrcQword.m_uMatchHits );
					pHitBuilder->cidxHit ( &tHit );
				} else
					tMerger.TransferHits ( tSrcQword, tHit, dSrcRows );
			}

			// next word
			bDstWord = tDstReader.Read();
			bSrcWord = tSrcReader.Read();
		}
	}

	tProgress.m_iWords += iWords;
	tProgress.Show();

	if ( iHitlistsDiscarded )
		sphWarning ( "discarded hitlists for %u words", iHitlistsDiscarded );

	return true;
}


bool CSphIndex_VLN::Merge ( CSphIndex * pSource, const VecTraits_T<CSphFilterSettings> & dFilters, bool bSupressDstDocids, CSphIndexProgress& tProgress )
{
	// if no source provided - special pass. No preload/preread, just merge with filters
	if ( pSource )
	{
		{
			ResetFileAccess(this);

			StrVec_t dWarnings;
			if ( !Prealloc ( false, nullptr, dWarnings ) )
				return false;

			for ( const auto & i : dWarnings )
				sphWarn ( "%s", i.cstr() );

			Preread();
		}

		{
			ResetFileAccess(pSource);

			StrVec_t dWarnings;
			if ( !pSource->Prealloc ( false, nullptr, dWarnings ) )
			{
				m_sLastError.SetSprintf ( "source index preload failed: %s", pSource->GetLastError().cstr() );
				return false;
			}

			for ( const auto & i : dWarnings )
				sphWarn ( "%s", i.cstr() );

			pSource->Preread();
		}
	} else
	{
		if ( dFilters.IsEmpty() )
		{
			m_sLastError.SetSprintf ( "no source, no filters - nothing to merge" );
			return false;
		}
		// prepare for self merging - no supress dst, source same as destination. Will apply klists/filters only.
		bSupressDstDocids = false;
		pSource = this;
	}

	// create filters
	CSphScopedPtr<ISphFilter> pFilter(nullptr);
	pFilter = CreateMergeFilters ( dFilters );

	return CSphIndex_VLN::DoMerge ( this, (const CSphIndex_VLN *)pSource, pFilter.Ptr(), m_sLastError, tProgress, false, bSupressDstDocids );
}


std::pair<DWORD,DWORD> CSphIndex_VLN::CreateRowMapsAndCountTotalDocs ( const CSphIndex_VLN* pSrcIndex, const CSphIndex_VLN* pDstIndex, CSphFixedVector<RowID_t>& dSrcRowMap, CSphFixedVector<RowID_t>& dDstRowMap, const ISphFilter* pFilter, bool bSupressDstDocids, MergeCb_c& tMonitor )
{
	if ( pSrcIndex!=pDstIndex )
		dSrcRowMap.Reset ( pSrcIndex->m_iDocinfo );
	dDstRowMap.Reset ( pDstIndex->m_iDocinfo );

	int iStride = pDstIndex->m_tSchema.GetRowSize();

	DeadRowMap_Ram_c tExtraDeadMap(0);
	if ( bSupressDstDocids ) // skip monitoring supressions as they're used _only_ from indexer call and never from flavours of optimize.
	{
		tExtraDeadMap.Reset ( dDstRowMap.GetLength() );

		LookupReaderIterator_c tDstLookupReader ( pDstIndex->m_tDocidLookup.GetWritePtr() );
		LookupReaderIterator_c tSrcLookupReader ( pSrcIndex->m_tDocidLookup.GetWritePtr() );

		KillByLookup ( tDstLookupReader, tSrcLookupReader, tExtraDeadMap, [] (DocID_t) {} );
	}

	dSrcRowMap.Fill ( INVALID_ROWID );
	dDstRowMap.Fill ( INVALID_ROWID );

	const DWORD * pRow = pDstIndex->m_tAttr.GetWritePtr();
	int64_t iTotalDocs = 0;
	std::pair<DWORD, DWORD> tPerIndexDocs {0,0};

	// say to observer we're going to collect alive rows from dst index
	// (kills directed to that index must be collected to reapply at the finish)
	tMonitor.SetEvent ( MergeCb_c::E_COLLECT_START, pDstIndex->m_iChunk );
	for ( int i = 0; i < dDstRowMap.GetLength(); ++i, pRow+=iStride )
	{
		if ( pDstIndex->m_tDeadRowMap.IsSet(i) )
			continue;

		if ( bSupressDstDocids && tExtraDeadMap.IsSet(i) )
			continue;

		if ( pFilter )
		{
			CSphMatch tFakeMatch;
			tFakeMatch.m_tRowID = i;
			tFakeMatch.m_pStatic = pRow;
			if ( !pFilter->Eval ( tFakeMatch ) )
				continue;
		}

		dDstRowMap[i] = (RowID_t)iTotalDocs++;
	}
	tMonitor.SetEvent ( MergeCb_c::E_COLLECT_FINISHED, pDstIndex->m_iChunk );
	tPerIndexDocs.first = (DWORD)iTotalDocs;
	if ( dSrcRowMap.IsEmpty() )
		return tPerIndexDocs;

	// say to observer we're going to collect alive rows from src index (again, issue to kills).
	tMonitor.SetEvent ( MergeCb_c::E_COLLECT_START, pSrcIndex->m_iChunk );
	for ( int i = 0; i < dSrcRowMap.GetLength(); ++i )
	{
		if ( pSrcIndex->m_tDeadRowMap.IsSet(i) )
			continue;

		dSrcRowMap[i] = (RowID_t)iTotalDocs++;
	}
	tMonitor.SetEvent ( MergeCb_c::E_COLLECT_FINISHED, pSrcIndex->m_iChunk );
	tPerIndexDocs.second = DWORD ( iTotalDocs - tPerIndexDocs.first );
	return tPerIndexDocs;
}


bool AttrMerger_c::Prepare ( const CSphIndex_VLN* pSrcIndex, const CSphIndex_VLN* pDstIndex )
{
	auto sSPA = pDstIndex->GetIndexFileName ( SPH_EXT_SPA, true );
	if ( pDstIndex->m_tSchema.HasNonColumnarAttrs() && !m_tWriterSPA.OpenFile ( sSPA, m_sError ) )
		return false;

	if ( pDstIndex->m_tSchema.HasBlobAttrs() )
	{
		m_pBlobRowBuilder = sphCreateBlobRowBuilder ( pSrcIndex->m_tSchema, pDstIndex->GetIndexFileName ( SPH_EXT_SPB, true ), pSrcIndex->GetSettings().m_tBlobUpdateSpace, m_sError );
		if ( !m_pBlobRowBuilder )
			return false;
	}

	if ( pDstIndex->m_pDocstore )
	{
		m_pDocstoreBuilder = CreateDocstoreBuilder ( pDstIndex->GetIndexFileName ( SPH_EXT_SPDS, true ), pDstIndex->m_pDocstore->GetDocstoreSettings(), m_sError );
		if ( !m_pDocstoreBuilder )
			return false;

		for ( int i = 0; i < pDstIndex->m_tSchema.GetFieldsCount(); ++i )
			if ( pDstIndex->m_tSchema.IsFieldStored ( i ) )
				m_pDocstoreBuilder->AddField ( pDstIndex->m_tSchema.GetFieldName ( i ), DOCSTORE_TEXT );
	}

	if ( pDstIndex->m_tSchema.HasColumnarAttrs() )
	{
		m_pColumnarBuilder = CreateColumnarBuilder ( pDstIndex->m_tSchema, pDstIndex->m_tSettings, pDstIndex->GetIndexFileName ( SPH_EXT_SPC, true ), m_sError );
		if ( !m_pColumnarBuilder )
			return false;
	}

	m_tMinMax.Init ( pDstIndex->m_tSchema );

	m_dDocidLookup.Reset ( m_iTotalDocs );
	CreateHistograms ( m_tHistograms, m_dAttrsForHistogram, pDstIndex->m_tSchema );

	m_tResultRowID = 0;
	return true;
}


bool AttrMerger_c::CopyPureColumnarAttributes ( const CSphIndex_VLN& tIndex, const VecTraits_T<RowID_t>& dRowMap )
{
	assert ( !tIndex.m_tAttr.GetWritePtr() );
	assert ( tIndex.m_tSchema.GetAttr ( 0 ).IsColumnar() );

	auto dColumnarIterators = CreateAllColumnarIterators ( tIndex.m_pColumnar.Ptr(), tIndex.m_tSchema );
	CSphVector<int64_t> dTmp;

	int iChunk = tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	auto _ = AtScopeExit ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );
	for ( RowID_t tRowID = 0, tRows = (RowID_t)dRowMap.GetLength64(); tRowID < tRows; ++tRowID )
	{
		if ( dRowMap[tRowID] == INVALID_ROWID )
			continue;

		if ( m_tMonitor.NeedStop() )
			return false;

		// limit granted by caller code
		assert ( m_tResultRowID != INVALID_ROWID );

		ARRAY_FOREACH ( i, dColumnarIterators )
		{
			auto& tIt = dColumnarIterators[i];
			Verify ( AdvanceIterator ( tIt.first.get(), tRowID ) );
			SetColumnarAttr ( i, tIt.second, m_pColumnarBuilder, tIt.first.get(), dTmp );
		}

		ARRAY_FOREACH ( i, m_dAttrsForHistogram )
			m_tHistograms.Insert ( i, m_dAttrsForHistogram[i].Get ( nullptr, dColumnarIterators ) );

		if ( m_pDocstoreBuilder )
			m_pDocstoreBuilder->AddDoc ( m_tResultRowID, tIndex.m_pDocstore->GetDoc ( tRowID, nullptr, -1, false ) );

		m_dDocidLookup[m_tResultRowID] = { dColumnarIterators[0].first->Get(), m_tResultRowID };
		++m_tResultRowID;
	}
	return true;
}

bool AttrMerger_c::CopyMixedAttributes ( const CSphIndex_VLN& tIndex, const VecTraits_T<RowID_t>& dRowMap )
{
	auto dColumnarIterators = CreateAllColumnarIterators ( tIndex.m_pColumnar.Ptr(), tIndex.m_tSchema );
	CSphVector<int64_t> dTmp;

	int iColumnarIdLoc = tIndex.m_tSchema.GetAttr ( 0 ).IsColumnar() ? 0 : - 1;
	const CSphRowitem* pRow = tIndex.m_tAttr.GetWritePtr();
	assert ( pRow );
	int iStride = tIndex.m_tSchema.GetRowSize();
	CSphFixedVector<CSphRowitem> dTmpRow ( iStride );
	auto iStrideBytes = dTmpRow.GetLengthBytes();
	const CSphColumnInfo* pBlobLocator = tIndex.m_tSchema.GetAttr ( sphGetBlobLocatorName() );

	int iChunk = tIndex.m_iChunk;
	m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_START, iChunk );
	auto _ = AtScopeExit ( [this, iChunk] { m_tMonitor.SetEvent ( MergeCb_c::E_MERGEATTRS_FINISHED, iChunk ); } );
	for ( RowID_t tRowID = 0, tRows = (RowID_t)dRowMap.GetLength64(); tRowID < tRows; ++tRowID, pRow += iStride )
	{
		if ( dRowMap[tRowID] == INVALID_ROWID )
			continue;

		if ( m_tMonitor.NeedStop() )
			return false;

		// limit granted by caller code
		assert ( m_tResultRowID != INVALID_ROWID );

		m_tMinMax.Collect ( pRow );

		if ( m_pBlobRowBuilder )
		{
			const BYTE* pOldBlobRow = tIndex.m_tBlobAttrs.GetWritePtr() + sphGetRowAttr ( pRow, pBlobLocator->m_tLocator );
			uint64_t	uNewOffset	= m_pBlobRowBuilder->Flush ( pOldBlobRow );

			memcpy ( dTmpRow.Begin(), pRow, iStrideBytes );
			sphSetRowAttr ( dTmpRow.Begin(), pBlobLocator->m_tLocator, uNewOffset );

			m_tWriterSPA.PutBytes ( dTmpRow.Begin(), iStrideBytes );
		} else if ( iStrideBytes )
			m_tWriterSPA.PutBytes ( pRow, iStrideBytes );

		ARRAY_FOREACH ( i, dColumnarIterators )
		{
			auto& tIt = dColumnarIterators[i];
			Verify ( AdvanceIterator ( tIt.first.get(), tRowID ) );
			SetColumnarAttr ( i, tIt.second, m_pColumnarBuilder, tIt.first.get(), dTmp );
		}

		DocID_t tDocID = iColumnarIdLoc >= 0 ? dColumnarIterators[iColumnarIdLoc].first->Get() : sphGetDocID ( pRow );

		ARRAY_FOREACH ( i, m_dAttrsForHistogram )
			m_tHistograms.Insert ( i, m_dAttrsForHistogram[i].Get ( pRow, dColumnarIterators ) );

		if ( m_pDocstoreBuilder )
			m_pDocstoreBuilder->AddDoc ( m_tResultRowID, tIndex.m_pDocstore->GetDoc ( tRowID, nullptr, -1, false ) );

		m_dDocidLookup[m_tResultRowID] = { tDocID, m_tResultRowID };
		++m_tResultRowID;
	}
	return true;
}


bool AttrMerger_c::CopyAttributes ( const CSphIndex_VLN& tIndex, const VecTraits_T<RowID_t>& dRowMap, DWORD uAlive )
{
	if ( !uAlive )
		return true;

	// that is very empyric, however is better than nothing.
	m_iTotalBytes += tIndex.m_tStats.m_iTotalBytes * ( (float)uAlive / (float)dRowMap.GetLength64() );

	if ( !tIndex.m_tAttr.GetWritePtr() )
		return CopyPureColumnarAttributes( tIndex, dRowMap );
	return CopyMixedAttributes ( tIndex, dRowMap );
}


bool AttrMerger_c::FinishMergeAttributes ( const CSphIndex_VLN* pDstIndex, BuildHeader_t& tBuildHeader )
{
	m_tMinMax.FinishCollect();
	assert ( m_tResultRowID==m_iTotalDocs );
	tBuildHeader.m_iDocinfo = m_iTotalDocs;
	tBuildHeader.m_iTotalDocuments = m_iTotalDocs;
	tBuildHeader.m_iTotalBytes = m_iTotalBytes;

	m_dDocidLookup.Sort ( CmpDocidLookup_fn() );
	if ( !WriteDocidLookup ( pDstIndex->GetIndexFileName ( SPH_EXT_SPT, true ), m_dDocidLookup, m_sError ) )
		return false;

	if ( pDstIndex->m_tSchema.HasNonColumnarAttrs() )
	{
		if ( m_iTotalDocs )
		{
			tBuildHeader.m_iMinMaxIndex = m_tWriterSPA.GetPos() / sizeof(CSphRowitem);
			const auto& dMinMaxRows		 = m_tMinMax.GetCollected();
			m_tWriterSPA.PutBytes ( dMinMaxRows.Begin(), dMinMaxRows.GetLengthBytes64() );
			tBuildHeader.m_iDocinfoIndex = ( dMinMaxRows.GetLength() / pDstIndex->m_tSchema.GetRowSize() / 2 ) - 1;
		}

		m_tWriterSPA.CloseFile();
		if ( m_tWriterSPA.IsError() )
			return false;
	}

	if ( m_pBlobRowBuilder && !m_pBlobRowBuilder->Done(m_sError) )
		return false;

	std::string sErrorSTL;
	if ( m_pColumnarBuilder && !m_pColumnarBuilder->Done(sErrorSTL) )
	{
		m_sError = sErrorSTL.c_str();
		return false;
	}

	if ( !m_tHistograms.Save ( pDstIndex->GetIndexFileName ( SPH_EXT_SPHI, true ), m_sError ) )
		return false;

	if ( !CheckDocsCount ( m_tResultRowID, m_sError ) )
		return false;

	if ( m_pDocstoreBuilder )
		m_pDocstoreBuilder->Finalize();

	return WriteDeadRowMap ( pDstIndex->GetIndexFileName ( SPH_EXT_SPM, true ), m_tResultRowID, m_sError );
}


bool CSphIndex_VLN::DoMerge ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, ISphFilter * pFilter, CSphString & sError, CSphIndexProgress & tProgress,
	bool bSrcSettings, bool bSupressDstDocids )
{
	auto & tMonitor = tProgress.GetMergeCb();
	assert ( pDstIndex && pSrcIndex );

	/// 'merge with self' - only apply filters/kill-lists, no real merge
	bool bCompress = pSrcIndex==pDstIndex;

	const CSphSchema & tDstSchema = pDstIndex->m_tSchema;
	const CSphSchema & tSrcSchema = pSrcIndex->m_tSchema;
	if ( !bCompress && !tDstSchema.CompareTo ( tSrcSchema, sError ) )
		return false;

	if ( !bCompress && pDstIndex->m_tSettings.m_eHitless!=pSrcIndex->m_tSettings.m_eHitless )
	{
		sError = "hitless settings must be the same on merged indices";
		return false;
	}

	if ( !bCompress && pDstIndex->m_pDict->GetSettings().m_bWordDict!=pSrcIndex->m_pDict->GetSettings().m_bWordDict )
	{
		sError.SetSprintf ( "dictionary types must be the same (dst dict=%s, src dict=%s )",
			pDstIndex->m_pDict->GetSettings().m_bWordDict ? "keywords" : "crc",
			pSrcIndex->m_pDict->GetSettings().m_bWordDict ? "keywords" : "crc" );
		return false;
	}

	// Create global list of documents to be merged from both sources
	// here we can also quickly consider to reject whole merge if N of final docs is >4G
	CSphFixedVector<RowID_t> dSrcRows{0}, dDstRows{0};
	auto tTotalDocs = CreateRowMapsAndCountTotalDocs ( pSrcIndex, pDstIndex, dSrcRows, dDstRows, pFilter, bSupressDstDocids, tMonitor );
	int64_t iTotalDocs = (int64_t)tTotalDocs.first + (int64_t)tTotalDocs.second;
	if ( iTotalDocs >= INVALID_ROWID )
		return false; // too many docs in merged segment (>4G even with filtered/killed), abort.

	BuildHeader_t tBuildHeader ( pDstIndex->m_tStats );

	// merging attributes
	{
		AttrMerger_c tAttrMerger ( tMonitor, sError, iTotalDocs );
		if ( !tAttrMerger.Prepare ( pSrcIndex, pDstIndex ) )
			return false;

		if ( !tAttrMerger.CopyAttributes ( *pDstIndex, dDstRows, tTotalDocs.first ) )
			return false;

		if ( !bCompress && !tAttrMerger.CopyAttributes ( *pSrcIndex, dSrcRows, tTotalDocs.second ) )
			return false;

		if ( !tAttrMerger.FinishMergeAttributes ( pDstIndex, tBuildHeader ) )
			return false;
	}

	const CSphIndex_VLN* pSettings = ( bSrcSettings ? pSrcIndex : pDstIndex );
	CSphAutofile tTmpDict ( pDstIndex->GetIndexFileName("spi.tmp"), SPH_O_NEW, sError, true );
	CSphAutofile tDict ( pDstIndex->GetIndexFileName ( SPH_EXT_SPI, true ), SPH_O_NEW, sError );

	if ( !sError.IsEmpty() || tTmpDict.GetFD()<0 || tDict.GetFD()<0 || tMonitor.NeedStop() )
		return false;

	DictRefPtr_c pDict { pSettings->m_pDict->Clone() };

	int iHitBufferSize = 8 * 1024 * 1024;
	CSphVector<SphWordID_t> dDummy;
	CSphHitBuilder tHitBuilder ( pSettings->m_tSettings, dDummy, true, iHitBufferSize, pDict, &sError );

	// FIXME? is this magic dict block constant any good?..
	pDict->DictBegin ( tTmpDict, tDict, iHitBufferSize );

	// merge dictionaries, doclists and hitlists
	if ( pDict->GetSettings().m_bWordDict )
	{
		WITH_QWORD ( pDstIndex, false, QwordDst,
			WITH_QWORD ( pSrcIndex, false, QwordSrc,
				if ( !CSphIndex_VLN::MergeWords < QwordDst, QwordSrc > ( pDstIndex, pSrcIndex, dDstRows, dSrcRows, &tHitBuilder, sError, tProgress ) )
					return false;
		));
	} else
	{
		WITH_QWORD ( pDstIndex, true, QwordDst,
			WITH_QWORD ( pSrcIndex, true, QwordSrc,
				if ( !CSphIndex_VLN::MergeWords < QwordDst, QwordSrc > ( pDstIndex, pSrcIndex, dDstRows, dSrcRows, &tHitBuilder, sError, tProgress ) )
					return false;
		));
	}

	if ( tMonitor.NeedStop () )
		return false;

	// finalize
	CSphAggregateHit tFlush;
	tFlush.m_tRowID = INVALID_ROWID;
	tFlush.m_uWordID = 0;
	tFlush.m_sKeyword = (const BYTE*)""; // tricky: assertion in cidxHit calls strcmp on this in case of empty index!
	tFlush.m_iWordPos = EMPTY_HIT;
	tFlush.m_dFieldMask.UnsetAll();
	tHitBuilder.cidxHit ( &tFlush );

	int iMinInfixLen = pSettings->m_tSettings.m_iMinInfixLen;
	if ( !tHitBuilder.cidxDone ( iHitBufferSize, iMinInfixLen, pSettings->m_pTokenizer->GetMaxCodepointLength(), &tBuildHeader ) )
		return false;

	CSphString sHeaderName = pDstIndex->GetIndexFileName ( SPH_EXT_SPH, true );

	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &pSettings->m_tSettings;
	tWriteHeader.m_pSchema = &pSettings->m_tSchema;
	tWriteHeader.m_pTokenizer = pSettings->m_pTokenizer;
	tWriteHeader.m_pDict = pSettings->m_pDict;
	tWriteHeader.m_pFieldFilter = pSettings->m_pFieldFilter;
	tWriteHeader.m_pFieldLens = pSettings->m_dFieldLens.Begin();

	IndexBuildDone ( tBuildHeader, tWriteHeader, sHeaderName, sError );

	return true;
}


bool sphMerge ( const CSphIndex * pDst, const CSphIndex * pSrc, VecTraits_T<CSphFilterSettings> dFilters, CSphIndexProgress & tProgress, CSphString& sError )
{
	auto pDstIndex = (const CSphIndex_VLN*) pDst;
	auto pSrcIndex = (const CSphIndex_VLN*) pSrc;

//	CSphScopedPtr<ISphFilter> pFilter { pDstIndex->CreateMergeFilters ( dFilters ) };
	std::unique_ptr<ISphFilter> pFilter { pDstIndex->CreateMergeFilters ( dFilters ) };
	return CSphIndex_VLN::DoMerge ( pDstIndex, pSrcIndex, pFilter.get(), sError, tProgress, dFilters.IsEmpty(), false );
}

template < typename QWORD >
bool CSphIndex_VLN::DeleteField ( const CSphIndex_VLN * pIndex, CSphHitBuilder * pHitBuilder, CSphString & sError, CSphSourceStats & tStat, int iKillField )
{
	assert ( iKillField>=0 );

	CSphAutofile tDummy;
	pHitBuilder->CreateIndexFiles ( pIndex->GetIndexFileName("tmp.spd").cstr(), pIndex->GetIndexFileName("tmp.spp").cstr(), pIndex->GetIndexFileName("tmp.spe").cstr(), false, 0, tDummy, nullptr );

	CSphDictReader<QWORD::is_worddict::value> tWordsReader ( pIndex->GetSettings().m_iSkiplistBlockSize );
	if ( !tWordsReader.Setup ( pIndex->GetIndexFileName(SPH_EXT_SPI), pIndex->m_tWordlist.GetWordsEnd(), pIndex->m_tSettings.m_eHitless, sError ) )
		return false;

	/// prepare for indexing
	pHitBuilder->HitblockBegin();
	pHitBuilder->HitReset();

	/// setup qword
	QWORD tQword ( false, false );
	DataReaderFactoryPtr_c tDocs {
		NewProxyReader ( pIndex->GetIndexFileName ( SPH_EXT_SPD ), sError,
			DataReaderFactory_c::DOCS, pIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE )
	};
	if ( !tDocs )
		return false;

	DataReaderFactoryPtr_c tHits {
		NewProxyReader ( pIndex->GetIndexFileName ( SPH_EXT_SPP ), sError,
			DataReaderFactory_c::HITS, pIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE  )
	};
	if ( !tHits )
		return false;

	if ( !sError.IsEmpty () || sphInterrupted () )
		return false;

	QwordIteration::ConfigureQword ( tQword, tHits, tDocs, pIndex->m_tSchema.GetDynamicSize() );

	/// process
	while ( tWordsReader.Read () )
	{
		if ( sphInterrupted () )
			return false;

		bool bHitless = !tWordsReader.m_bHasHitlist;
		QwordIteration::PrepareQword ( tQword, tWordsReader );

		CSphAggregateHit tHit;
		tHit.m_uWordID = tWordsReader.m_uWordID; // !COMMIT m_sKeyword anyone?
		tHit.m_sKeyword = tWordsReader.GetWord();
		tHit.m_dFieldMask.UnsetAll();

		// transfer hits
		while ( QwordIteration::NextDocument ( tQword, pIndex ) )
		{
			if ( sphInterrupted () )
				return false;

			if ( bHitless )
			{
				tHit.m_tRowID = tQword.m_tDoc.m_tRowID;
				tHit.m_dFieldMask = tQword.m_dQwordFields; // fixme! what field mask on hitless? m.b. write 0 here?
				tHit.m_dFieldMask.DeleteBit (iKillField);
				if ( tHit.m_dFieldMask.TestAll ( false ) )
					continue;
				tHit.SetAggrCount ( tQword.m_uMatchHits );
				pHitBuilder->cidxHit ( &tHit );
			} else
			{
				assert ( tQword.m_bHasHitlist );
				tHit.m_tRowID = tQword.m_tDoc.m_tRowID;
				for ( Hitpos_t uHit = tQword.GetNextHit(); uHit!=EMPTY_HIT; uHit = tQword.GetNextHit() )
				{
					int iField = HITMAN::GetField ( uHit );
					if ( iKillField==iField )
						continue;

					if ( iField>iKillField )
						HITMAN::DecrementField ( uHit );

					tHit.m_iWordPos = uHit;
					pHitBuilder->cidxHit ( &tHit );
				}
			}
		}
	}

	return true;
}


bool CSphIndex_VLN::DeleteFieldFromDict ( int iFieldId, BuildHeader_t & tBuildHeader, CSphString & sError )
{
	CSphAutofile tTmpDict ( GetIndexFileName ( "spi.tmp" ), SPH_O_NEW, sError, true );
	CSphAutofile tNewDict ( GetIndexFileName ( SPH_EXT_SPI, true ), SPH_O_NEW, sError );

	if ( !sError.IsEmpty () || tTmpDict.GetFD ()<0 || tNewDict.GetFD ()<0 || sphInterrupted () )
		return false;

	DictRefPtr_c pDict { m_pDict->Clone () };

	int iHitBufferSize = 8 * 1024 * 1024;
	CSphVector<SphWordID_t> dDummy;
	CSphHitBuilder tHitBuilder ( m_tSettings, dDummy, true, iHitBufferSize, pDict, &sError );

	// FIXME? is this magic dict block constant any good?..
	pDict->DictBegin ( tTmpDict, tNewDict, iHitBufferSize );

	// merge dictionaries, doclists and hitlists
	if ( pDict->GetSettings().m_bWordDict )
	{
		WITH_QWORD ( this, false, Qword,
			if ( !CSphIndex_VLN::DeleteField <Qword> ( this, &tHitBuilder, sError, tBuildHeader, iFieldId ) )
				return false;
		);
	} else
	{
		WITH_QWORD ( this, true, Qword,
			if ( !CSphIndex_VLN::DeleteField <Qword> ( this, &tHitBuilder, sError, tBuildHeader, iFieldId ) )
				return false;
		);
	}

	if ( sphInterrupted() )
		return false;

	// finalize
	CSphAggregateHit tFlush;
	tFlush.m_tRowID = INVALID_ROWID;
	tFlush.m_uWordID = 0;
	tFlush.m_sKeyword = (const BYTE*)""; // tricky: assertion in cidxHit calls strcmp on this in case of empty index!
	tFlush.m_iWordPos = EMPTY_HIT;
	tFlush.m_dFieldMask.UnsetAll();
	tHitBuilder.cidxHit ( &tFlush );

	int iMinInfixLen = m_tSettings.m_iMinInfixLen;
	if ( !tHitBuilder.cidxDone ( iHitBufferSize, iMinInfixLen, m_pTokenizer->GetMaxCodepointLength(), &tBuildHeader ) )
		return false;

	/// as index is w-locked, we can also detach doclist/hitlist/dictionary and juggle them.
	tTmpDict.Close();
	tNewDict.Close();

	m_tWordlist.Reset();
	if ( !JuggleFile ( SPH_EXT_SPI, sError ) )	return false;
	m_tWordlist.m_iDictCheckpointsOffset= tBuildHeader.m_iDictCheckpointsOffset;
	m_tWordlist.m_iDictCheckpoints		= tBuildHeader.m_iDictCheckpoints;
	m_tWordlist.m_iInfixCodepointBytes	= tBuildHeader.m_iInfixCodepointBytes;
	m_tWordlist.m_iInfixBlocksOffset	= tBuildHeader.m_iInfixBlocksOffset;
	m_tWordlist.m_iInfixBlocksWordsSize = tBuildHeader.m_iInfixBlocksWordsSize;
	m_tWordlist.m_dCheckpoints.Reset ( m_tWordlist.m_iDictCheckpoints );
	if ( !PreallocWordlist() )					return false;

	m_tSkiplists.Reset ();
	if ( !JuggleFile ( SPH_EXT_SPE, sError ) )	return false;
	if ( !PreallocSkiplist() )					return false;

	m_pDoclistFile = nullptr;
	m_pHitlistFile = nullptr;
	if ( !JuggleFile ( SPH_EXT_SPD, sError ) )	return false;
	if ( !JuggleFile ( SPH_EXT_SPP, sError ) )	return false;
	if ( !SpawnReaders() )						return false;

	return true;
}


bool CSphIndex_VLN::AddRemoveFieldFromDocstore ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, CSphString & sError )
{
	int iOldNumStored = 0;
	for ( int i = 0; i < tOldSchema.GetFieldsCount(); i++ )
		if ( tOldSchema.IsFieldStored(i) )
			iOldNumStored++;

	int iNewNumStored = 0;
	for ( int i = 0; i < tNewSchema.GetFieldsCount(); i++ )
		if ( tNewSchema.IsFieldStored(i) )
			iNewNumStored++;

	if ( iOldNumStored==iNewNumStored )
		return true;

	CSphScopedPtr<DocstoreBuilder_i> pDocstoreBuilder(nullptr);
	if ( iNewNumStored )
	{
		pDocstoreBuilder = CreateDocstoreBuilder ( GetIndexFileName ( SPH_EXT_SPDS, true ), m_pDocstore->GetDocstoreSettings(), sError );
		if ( !pDocstoreBuilder.Ptr() )
			return false;

		Alter_AddRemoveFieldFromDocstore ( *pDocstoreBuilder, m_pDocstore.Ptr(), (DWORD)m_iDocinfo, tNewSchema );
	}

	if ( !JuggleFile ( SPH_EXT_SPDS, sError, !!iOldNumStored, !!iNewNumStored ) )
		return false;

	m_pDocstore.Reset();
	PreallocDocstore();

	return true;
}


bool CSphIndex_VLN::AddRemoveField ( bool bAddField, const CSphString & sFieldName, DWORD uFieldFlags, CSphString & sError )
{
	CSphSchema tOldSchema = m_tSchema;
	CSphSchema tNewSchema = m_tSchema;
	if ( !Alter_AddRemoveFieldFromSchema ( bAddField, tNewSchema, sFieldName, uFieldFlags, sError ) )
		return false;

	auto iRemoveIdx = m_tSchema.GetFieldIndex ( sFieldName.cstr () );
	m_tSchema = tNewSchema;

	BuildHeader_t tBuildHeader ( m_tStats );
	tBuildHeader.m_iDocinfo = m_iDocinfo;
	tBuildHeader.m_iDocinfoIndex = m_iDocinfoIndex;
	tBuildHeader.m_iMinMaxIndex = m_iMinMaxIndex;

	*(DictHeader_t *) &tBuildHeader = *(DictHeader_t *) &m_tWordlist;

	if ( !bAddField && !DeleteFieldFromDict ( iRemoveIdx, tBuildHeader, sError ) )
		return false;

	if ( !AddRemoveFieldFromDocstore ( tOldSchema, tNewSchema, sError ) )
		return false;

	CSphString sHeaderName = GetIndexFileName ( SPH_EXT_SPH, true );
	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &m_tSettings;
	tWriteHeader.m_pSchema = &tNewSchema;
	tWriteHeader.m_pTokenizer = m_pTokenizer;
	tWriteHeader.m_pDict = m_pDict;
	tWriteHeader.m_pFieldFilter = m_pFieldFilter;
	tWriteHeader.m_pFieldLens = m_dFieldLens.Begin ();

	// save the header
	if ( !IndexBuildDone ( tBuildHeader, tWriteHeader, sHeaderName, sError ) ) 	return false;
	if ( !JuggleFile ( SPH_EXT_SPH, sError ) )		return false;

	return true;
}


/////////////////////////////////////////////////////////////////////////////
// THE SEARCHER
/////////////////////////////////////////////////////////////////////////////

SphWordID_t CSphDictTraits::GetWordID ( BYTE * )
{
	assert ( 0 && "not implemented" );
	return 0;
}


SphWordID_t CSphDictStar::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];
	assert ( strlen ( (const char*)pWord ) < 16+3*SPH_MAX_WORD_LEN );

	if ( m_pDict->GetSettings().m_bStopwordsUnstemmed && m_pDict->IsStopWord ( pWord ) )
		return 0;

	m_pDict->ApplyStemmers ( pWord );

	auto iLen = (int) strlen ( (const char*)pWord );
	assert ( iLen < 16+3*SPH_MAX_WORD_LEN - 1 );
	// stemmer might squeeze out the word
	if ( iLen && !pWord[0] )
		return 0;

	memcpy ( sBuf, pWord, iLen+1 );

	if ( iLen )
	{
		if ( sBuf[iLen-1]=='*' )
		{
			iLen--;
			sBuf[iLen] = '\0';
		} else
		{
			sBuf[iLen] = MAGIC_WORD_TAIL;
			iLen++;
			sBuf[iLen] = '\0';
		}
	}

	return m_pDict->GetWordID ( (BYTE*)sBuf, iLen, !m_pDict->GetSettings().m_bStopwordsUnstemmed );
}


SphWordID_t	CSphDictStar::GetWordIDNonStemmed ( BYTE * pWord )
{
	return m_pDict->GetWordIDNonStemmed ( pWord );
}


//////////////////////////////////////////////////////////////////////////

SphWordID_t	CSphDictStarV8::GetWordID ( BYTE * pWord )
{
	char sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	auto iLen = (int) strlen ( (const char*)pWord );
	iLen = Min ( iLen, 16+3*SPH_MAX_WORD_LEN - 1 );

	if ( !iLen )
		return 0;

	bool bHeadStar = ( pWord[0]=='*' );
	bool bTailStar = ( pWord[iLen-1]=='*' ) && ( iLen>1 );
	bool bMagic = ( pWord[0]<' ' );

	if ( !bHeadStar && !bTailStar && !bMagic )
	{
		if ( m_pDict->GetSettings().m_bStopwordsUnstemmed && IsStopWord ( pWord ) )
			return 0;

		m_pDict->ApplyStemmers ( pWord );

		// stemmer might squeeze out the word
		if ( !pWord[0] )
			return 0;

		if ( !m_pDict->GetSettings().m_bStopwordsUnstemmed && IsStopWord ( pWord ) )
			return 0;
	}

	iLen = (int) strlen ( (const char*)pWord );
	assert ( iLen < 16+3*SPH_MAX_WORD_LEN - 2 );

	if ( !iLen || ( bHeadStar && iLen==1 ) )
		return 0;

	if ( bMagic ) // pass throu MAGIC_* words
	{
		memcpy ( sBuf, pWord, iLen );
		sBuf[iLen] = '\0';

	} else if ( m_bInfixes )
	{
		////////////////////////////////////
		// infix or mixed infix+prefix mode
		////////////////////////////////////

		// handle head star
		if ( bHeadStar )
		{
			memcpy ( sBuf, pWord+1, iLen-- ); // chops star, copies trailing zero, updates iLen
		} else
		{
			sBuf[0] = MAGIC_WORD_HEAD;
			memcpy ( sBuf+1, pWord, ++iLen ); // copies everything incl trailing zero, updates iLen
		}

		// handle tail star
		if ( bTailStar )
		{
			sBuf[--iLen] = '\0'; // got star, just chop it away
		} else
		{
			sBuf[iLen] = MAGIC_WORD_TAIL; // no star, add tail marker
			sBuf[++iLen] = '\0';
		}

	} else
	{
		////////////////////
		// prefix-only mode
		////////////////////

		// always ignore head star in prefix mode
		if ( bHeadStar )
		{
			pWord++;
			iLen--;
		}

		// handle tail star
		if ( !bTailStar )
		{
			// exact word search request, always (ie. both in infix/prefix mode) mangles to "\1word\1" in v.8+
			sBuf[0] = MAGIC_WORD_HEAD;
			memcpy ( sBuf+1, pWord, iLen );
			sBuf[iLen+1] = MAGIC_WORD_TAIL;
			sBuf[iLen+2] = '\0';
			iLen += 2;

		} else
		{
			// prefix search request, mangles to word itself (just chop away the star)
			memcpy ( sBuf, pWord, iLen );
			sBuf[--iLen] = '\0';
		}
	}

	// calc id for mangled word
	return m_pDict->GetWordID ( (BYTE*)sBuf, iLen, !bHeadStar && !bTailStar );
}

//////////////////////////////////////////////////////////////////////////

SphWordID_t CSphDictExact::GetWordID ( BYTE * pWord )
{
	auto iLen = (int) strlen ( (const char*)pWord );
	iLen = Min ( iLen, 16+3*SPH_MAX_WORD_LEN - 1 );

	if ( !iLen )
		return 0;

	if ( pWord[0]=='=' )
		pWord[0] = MAGIC_WORD_HEAD_NONSTEMMED;

	if ( pWord[0]<' ' )
		return m_pDict->GetWordIDNonStemmed ( pWord );

	return m_pDict->GetWordID ( pWord );
}

/////////////////////////////////////////////////////////////////////////////

void CSphMatchComparatorState::FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, bool bRemapKeyparts )
{
	for ( int i = 0; i < CSphMatchComparatorState::MAX_ATTRS; ++i )
	{
		sphFixupLocator ( m_tLocator[i], pOldSchema, pNewSchema );

		// update string keypart into str_ptr
		if ( bRemapKeyparts && m_eKeypart[i]==SPH_KEYPART_STRING )
			m_eKeypart[i] = SPH_KEYPART_STRINGPTR;

		// update columnar attrs
		if ( pOldSchema )
		{
			int iOldAttrId = m_dAttrs[i];
			if ( iOldAttrId!=-1 )
				m_dAttrs[i] = pNewSchema->GetAttrIndex ( pOldSchema->GetAttr(iOldAttrId).m_sName.cstr() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////

inline bool sphGroupMatch ( SphAttr_t iGroup, const SphAttr_t * pGroups, int iGroups )
{
	if ( !pGroups ) return true;
	const SphAttr_t * pA = pGroups;
	const SphAttr_t * pB = pGroups+iGroups-1;
	if ( iGroup==*pA || iGroup==*pB ) return true;
	if ( iGroup<(*pA) || iGroup>(*pB) ) return false;

	while ( pB-pA>1 )
	{
		const SphAttr_t * pM = pA + ((pB-pA)/2);
		if ( iGroup==(*pM) )
			return true;
		if ( iGroup<(*pM) )
			pB = pM;
		else
			pA = pM;
	}
	return false;
}


bool CSphIndex_VLN::EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const
{
	tMatch.m_pStatic = GetDocinfoByRowID ( tMatch.m_tRowID );
	pCtx->CalcFilter ( tMatch );
	if ( !pCtx->m_pFilter )
		return false;

	if ( !pCtx->m_pFilter->Eval ( tMatch ) )
	{
		pCtx->FreeDataFilter ( tMatch );
		return true;
	}

	return false;
}


CSphVector<SphAttr_t> CSphIndex_VLN::BuildDocList () const
{
	TlsMsg::Err(); // clean err
	CSphVector<SphAttr_t> dResult;
	if ( !m_iDocinfo )
		return dResult;

	// new[] might fail on 32bit here
	int64_t iSizeMax = (size_t)m_iDocinfo;
	if ( iSizeMax!=m_iDocinfo )
	{
		TlsMsg::Err ( "doc-list build size_t overflow (docs count=%l, size max=%l)", m_iDocinfo, iSizeMax );
		return dResult;
	}

	int iStride = m_tSchema.GetRowSize();
	dResult.Resize ( m_iDocinfo );

	const CSphRowitem * pRow = m_tAttr.GetWritePtr();
	for ( SphAttr_t & tDst : dResult )
	{
		tDst = sphGetDocID ( pRow );
		pRow += iStride;
	}

	dResult.Uniq();
	return dResult;
}


RowID_t CSphIndex_VLN::GetRowidByDocid ( DocID_t tDocID ) const
{
	return m_tLookupReader.Find ( tDocID );
}


int	CSphIndex_VLN::Kill ( DocID_t tDocID )
{
	// FIXME! docid might not be unique
	if ( m_tDeadRowMap.Set ( GetRowidByDocid ( tDocID ) ) )
	{
		m_uAttrsStatus |= IndexUpdateHelper_c::ATTRS_ROWMAP_UPDATED;
		if ( m_pKillHook )
			m_pKillHook->Kill ( tDocID );
		return 1;
	}

	return 0;
}

bool CSphIndex_VLN::IsAlive ( DocID_t tDocID ) const
{
	RowID_t tRow = GetRowidByDocid ( tDocID );
	if ( tRow==INVALID_ROWID )
		return false;

	return ( !m_tDeadRowMap.IsSet ( tRow ) );
}


inline const CSphRowitem * CSphIndex_VLN::FindDocinfo ( DocID_t tDocID ) const
{
	RowID_t tRowID = GetRowidByDocid ( tDocID );
	return tRowID==INVALID_ROWID ? nullptr : GetDocinfoByRowID ( tRowID );
}


inline const CSphRowitem * CSphIndex_VLN::GetDocinfoByRowID ( RowID_t tRowID ) const
{
	//  GetCachedRowSize() is used to avoid several virtual calls
	return m_tAttr.GetWritePtr() + (int64_t)tRowID*m_tSchema.GetCachedRowSize();
}


inline RowID_t CSphIndex_VLN::GetRowIDByDocinfo ( const CSphRowitem * pDocinfo ) const
{
	return RowID_t ( ( pDocinfo - m_tAttr.GetWritePtr() ) / m_tSchema.GetCachedRowSize() );
}


inline void CalcContextItem ( CSphMatch & tMatch, const CSphQueryContext::CalcItem_t & tCalc )
{
	switch ( tCalc.m_eType )
	{
	case SPH_ATTR_BOOL:
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_TIMESTAMP:
		tMatch.SetAttr ( tCalc.m_tLoc, tCalc.m_pExpr->IntEval(tMatch) );
		break;

	case SPH_ATTR_BIGINT:
	case SPH_ATTR_JSON_FIELD:
		tMatch.SetAttr ( tCalc.m_tLoc, tCalc.m_pExpr->Int64Eval(tMatch) );
		break;

	case SPH_ATTR_STRINGPTR:
		tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->StringEvalPacked ( tMatch ) );
		break;

	case SPH_ATTR_FACTORS:
	case SPH_ATTR_FACTORS_JSON:
		tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->FactorEvalPacked ( tMatch ) ); // FIXME! a potential leak of *previous* value?
		break;

	case SPH_ATTR_INT64SET_PTR:
	case SPH_ATTR_UINT32SET_PTR:
		tMatch.SetAttr ( tCalc.m_tLoc, (SphAttr_t)tCalc.m_pExpr->Int64Eval ( tMatch ) );
		break;

	default:
		tMatch.SetAttrFloat ( tCalc.m_tLoc, tCalc.m_pExpr->Eval(tMatch) );
		break;
	}
}


inline void CalcContextItems ( CSphMatch & tMatch, const VecTraits_T<CSphQueryContext::CalcItem_t> & dItems )
{
	for ( auto & i : dItems )
		CalcContextItem ( tMatch, i );
}


void CSphQueryContext::CalcFilter ( CSphMatch & tMatch ) const
{
	CalcContextItems ( tMatch, m_dCalcFilter );
}


void CSphQueryContext::CalcSort ( CSphMatch & tMatch ) const
{
	CalcContextItems ( tMatch, m_dCalcSort );
}


void CSphQueryContext::CalcFinal ( CSphMatch & tMatch ) const
{
	CalcContextItems ( tMatch, m_dCalcFinal );
}


void CSphQueryContext::CalcItem ( CSphMatch & tMatch, const CalcItem_t & tCalc ) const
{
	CalcContextItem ( tMatch, tCalc );
}


static inline void FreeDataPtrAttrs ( CSphMatch & tMatch, const CSphVector<CSphQueryContext::CalcItem_t> & dItems, const IntVec_t & dItemIndexes )
{
	if ( !tMatch.m_pDynamic )
		return;

	for ( auto i : dItemIndexes )
	{
		const auto & tItem = dItems[i];

		BYTE * pData = (BYTE *)tMatch.GetAttr ( tItem.m_tLoc );
		// delete[] pData;
		if ( pData )
		{
			sphDeallocatePacked ( pData );
			tMatch.SetAttr ( tItem.m_tLoc, 0 );
		}
	}
}

void CSphQueryContext::FreeDataFilter ( CSphMatch & tMatch ) const
{
	FreeDataPtrAttrs ( tMatch, m_dCalcFilter, m_dCalcFilterPtrAttrs );
}


void CSphQueryContext::FreeDataSort ( CSphMatch & tMatch ) const
{
	FreeDataPtrAttrs ( tMatch, m_dCalcSort, m_dCalcSortPtrAttrs );
}

void CSphQueryContext::ExprCommand ( ESphExprCommand eCmd, void * pArg )
{
	ARRAY_FOREACH ( i, m_dCalcFilter )
		m_dCalcFilter[i].m_pExpr->Command ( eCmd, pArg );
	ARRAY_FOREACH ( i, m_dCalcSort )
		m_dCalcSort[i].m_pExpr->Command ( eCmd, pArg );
	ARRAY_FOREACH ( i, m_dCalcFinal )
		m_dCalcFinal[i].m_pExpr->Command ( eCmd, pArg );
}


void CSphQueryContext::SetBlobPool ( const BYTE * pBlobPool )
{
	ExprCommand ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	if ( m_pFilter )
		m_pFilter->SetBlobStorage ( pBlobPool );
	if ( m_pWeightFilter )
		m_pWeightFilter->SetBlobStorage ( pBlobPool );
}



void CSphQueryContext::SetColumnar ( const columnar::Columnar_i * pColumnar )
{
	ExprCommand ( SPH_EXPR_SET_COLUMNAR, (void*)pColumnar );
}


/// FIXME, perhaps
/// this rather crappy helper class really serves exactly 1 (one) simple purpose
///
/// it passes a sorting queue internals (namely, weight and float sortkey, if any,
/// of the current-worst queue element) to the MIN_TOP_WORST() and MIN_TOP_SORTVAL()
/// expression classes that expose those to the cruel outside world
///
/// all the COM-like EXTRA_xxx message back and forth is needed because expressions
/// are currently parsed and created earlier than the sorting queue
///
/// that also is the reason why we mischievously return 0 instead of clearly failing
/// with an error when the sortval is not a dynamic float; by the time we are parsing
/// expressions, we do not *yet* know that; but by the time we create a sorting queue,
/// we do not *want* to leak select expression checks into it
///
/// alternatively, we probably want to refactor this and introduce Bind(), to parse
/// expressions once, then bind them to actual searching contexts (aka index or segment,
/// and ranker, and sorter, and whatever else might be referenced by the expressions)
struct ContextExtra final : public ISphExtra
{
	ISphRanker * m_pRanker;
	ISphMatchSorter * m_pSorter;

	ContextExtra ( ISphRanker* pRanker, ISphMatchSorter* pSorter)
		: m_pRanker ( pRanker )
		, m_pSorter ( pSorter )
		{}

	bool ExtraDataImpl ( ExtraData_e eData, void ** ppArg ) final
	{
		if ( eData!=EXTRA_GET_QUEUE_WORST && eData!=EXTRA_GET_QUEUE_SORTVAL )
			return m_pRanker->ExtraData ( eData, ppArg );

		if ( !m_pSorter )
			return false;

		const CSphMatch * pWorst = m_pSorter->GetWorst();
		if ( !pWorst )
			return false;

		if ( eData==EXTRA_GET_QUEUE_WORST )
		{
			*ppArg = (void*)pWorst;
			return true;
		};

		assert ( eData==EXTRA_GET_QUEUE_SORTVAL );
		const CSphMatchComparatorState & tCmp = m_pSorter->GetState();
		if ( tCmp.m_eKeypart[0]==SPH_KEYPART_FLOAT && tCmp.m_tLocator[0].m_bDynamic
			&& tCmp.m_tLocator[0].m_iBitCount==32 && ( tCmp.m_tLocator[0].m_iBitOffset%32==0 )
			&& tCmp.m_dAttrs[1]==-1 )
		{
			*(int*)ppArg = tCmp.m_tLocator[0].m_iBitOffset/32;
			return true;
		}

		// min_top_sortval() only works with order by float_expr for now
		return false;
	}
};


void CSphQueryContext::SetupExtraData ( ISphRanker * pRanker, ISphMatchSorter * pSorter )
{
	ContextExtra tExtra ( pRanker, pSorter );
	ExprCommand ( SPH_EXPR_SET_EXTRA_DATA, &tExtra );
}


template<bool USE_KLIST, bool RANDOMIZE, bool USE_FACTORS>
void CSphIndex_VLN::MatchExtended ( CSphQueryContext& tCtx, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, ISphRanker * pRanker, int iTag, int iIndexWeight ) const
{
	QueryProfile_c * pProfile = tCtx.m_pProfile;
	CSphScopedProfile tProf (pProfile, SPH_QSTATE_UNKNOWN);

	int iCutoff = tQuery.m_iCutoff;
	if ( iCutoff<=0 )
		iCutoff = -1;

	// do searching
	CSphMatch * pMatch = pRanker->GetMatchesBuffer();
	while (true)
	{
		// ranker does profile switches internally in GetMatches()
		int iMatches = pRanker->GetMatches();
		if ( iMatches<=0 )
			break;

		SwitchProfile ( pProfile, SPH_QSTATE_SORT );

		for ( int i=0; i<iMatches; i++ )
		{
			CSphMatch & tMatch = pMatch[i];
			if_const ( USE_KLIST )
			{
				if ( m_tDeadRowMap.IsSet ( tMatch.m_tRowID ) )
					continue;
			}

			tMatch.m_iWeight *= iIndexWeight;
			tCtx.CalcSort ( tMatch );

			if ( tCtx.m_pWeightFilter && !tCtx.m_pWeightFilter->Eval ( tMatch ) )
			{
				tCtx.FreeDataSort ( tMatch );
				continue;
			}

			tMatch.m_iTag = iTag;

			bool bRand = false;
			bool bNewMatch = false;
			for ( ISphMatchSorter * pSorter: dSorters )
			{
				// all non-random sorters are in the beginning,
				// so we can avoid the simple 'first-element' assertion
				if_const ( RANDOMIZE )
				{
					if ( !bRand && pSorter->IsRandom() )
					{
						bRand = true;
						tMatch.m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;

						if ( tCtx.m_pWeightFilter && !tCtx.m_pWeightFilter->Eval ( tMatch ) )
							break;
					}
				}

				bNewMatch |= pSorter->Push ( tMatch );

				if_const ( USE_FACTORS )
				{
					RowID_t tJustPushed = pSorter->GetJustPushed();
					VecTraits_T<RowID_t> dJustPopped = pSorter->GetJustPopped();
					pRanker->ExtraData ( EXTRA_SET_MATCHPUSHED, (void**)&tJustPushed );
					pRanker->ExtraData ( EXTRA_SET_MATCHPOPPED, (void**)&dJustPopped );
				}
			}

			tCtx.FreeDataFilter ( tMatch );
			tCtx.FreeDataSort ( tMatch );

			if ( bNewMatch )
				if ( --iCutoff==0 )
					break;
		}

		if ( iCutoff==0 )
			break;
	}
}

//////////////////////////////////////////////////////////////////////////


struct SphFinalMatchCalc_t final : MatchProcessor_i, ISphNoncopyable
{
	const CSphQueryContext &	m_tCtx;
	int							m_iTag;

	SphFinalMatchCalc_t ( int iTag, const CSphQueryContext & tCtx )
		: m_tCtx ( tCtx )
		, m_iTag ( iTag )
	{}

	bool ProcessInRowIdOrder() const final
	{
		// columnar expressions don't like random access, they are optimized for sequental access
		// that's why if we have a columnar expression, we need to call Process it ascending RowId order
		return m_tCtx.m_dCalcFinal.any_of ( []( const CSphQueryContext::CalcItem_t & i ){ return i.m_pExpr && i.m_pExpr->IsColumnar(); } );
	}

	void Process ( CSphMatch * pMatch ) final
	{
		// fixme! tag is signed int,
		// for distr. tags from remotes set with | 0x80000000,
		// i e in terms of signed int they're <0!
		// Is it intention, or bug?
		// If intention, lt us use uniformely either <0, either &0x80000000
		// conditions to avoid messing. If bug, shit already happened!
		if ( pMatch->m_iTag>=0 )
			return;

		m_tCtx.CalcFinal ( *pMatch );
		pMatch->m_iTag = m_iTag;
	}

	void Process ( VecTraits_T<CSphMatch *> & dMatches ) final
	{
		// process items in column-wise order
		for ( const auto & tItem : m_tCtx.m_dCalcFinal )
			for ( auto & pMatch : dMatches )
			{
				assert(pMatch);
				if ( pMatch->m_iTag>=0 )
					continue;

				m_tCtx.CalcItem ( *pMatch, tItem );
			}

		for ( auto & pMatch : dMatches )
		{
			assert(pMatch);
			if ( pMatch->m_iTag>=0 )
				continue;

			pMatch->m_iTag = m_iTag;
		}
	}
};


/// scoped thread scheduling helper
/// either makes the current thread low priority while the helper lives, or does nothing
class ScopedThreadPriority_c
{
private:
	bool m_bRestore;

public:
	ScopedThreadPriority_c ( bool bLowPriority )
	{
		m_bRestore = false;
		if ( !bLowPriority )
			return;

#if _WIN32
		if ( !SetThreadPriority ( GetCurrentThread(), THREAD_PRIORITY_IDLE ) )
			return;
#else
		struct sched_param p;
		p.sched_priority = 0;
#ifdef SCHED_IDLE
		int iPolicy = SCHED_IDLE;
#else
		int iPolicy = SCHED_OTHER;
#endif
		if ( pthread_setschedparam ( pthread_self (), iPolicy, &p ) )
			return;
#endif

		m_bRestore = true;
	}

	~ScopedThreadPriority_c ()
	{
		if ( !m_bRestore )
			return;

#if _WIN32
		if ( !SetThreadPriority ( GetCurrentThread(), THREAD_PRIORITY_NORMAL ) )
			return;
#else
		struct sched_param p;
		p.sched_priority = 0;
		if ( pthread_setschedparam ( pthread_self (), SCHED_OTHER, &p ) )
			return;
#endif
	}
};

//////////////////////////////////////////////////////////////////////////

template <bool HAVE_DEAD>
class RowIterator_T : public ISphNoncopyable
{
public:
	RowIterator_T ( const RowIdBoundaries_t & tBoundaries, const DeadRowMap_Disk_c & tDeadRowMap )
		: m_tRowID ( tBoundaries.m_tMinRowID )
		, m_tBoundaries ( tBoundaries )
		, m_tDeadRowMap	( tDeadRowMap )
	{}

	inline bool GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock );
	DWORD GetNumProcessed() const { return m_tRowID-m_tBoundaries.m_tMinRowID; }

private:
	static const int MAX_COLLECTED = 128;

	RowID_t						m_tRowID {INVALID_ROWID};
	RowIdBoundaries_t			m_tBoundaries;
	CSphFixedVector<RowID_t>	m_dCollected {MAX_COLLECTED+1};		// store 128 values + end marker (same as .spa attr block size)
	const DeadRowMap_Disk_c &	m_tDeadRowMap;
};

template <>
bool RowIterator_T<true>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + m_dCollected.GetLength()-1;
	RowID_t * pRowID = pRowIdStart;

	while ( pRowID<pRowIdMax && m_tRowID<=m_tBoundaries.m_tMaxRowID )
	{
		if ( !m_tDeadRowMap.IsSet(m_tRowID) )
			*pRowID++ = m_tRowID;

		m_tRowID++;
	}

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}

template <>
bool RowIterator_T<false>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + Min ( RowID_t(m_dCollected.GetLength()-1), m_tBoundaries.m_tMaxRowID-m_tRowID+1 );
	RowID_t * pRowID = pRowIdStart;

	// fixme! use sse?
	while ( pRowID<pRowIdMax )
		*pRowID++ = m_tRowID++;

	return ReturnIteratorResult ( pRowID, pRowIdStart, dRowIdBlock );
}


// adds killlist filtering to a rowid iterator
class RowIteratorAlive_c : public ISphNoncopyable
{
public:
	RowIteratorAlive_c ( RowidIterator_i * pIterator, const DeadRowMap_Disk_c & tDeadRowMap )
		: m_pIterator	( pIterator )
		, m_tDeadRowMap	( tDeadRowMap )
	{
		assert(pIterator);
	}

	FORCE_INLINE bool GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
	{
		RowIdBlock_t dIteratorRowIDs;
		if ( !m_pIterator->GetNextRowIdBlock(dIteratorRowIDs) )
			return false;

		m_dCollected.Resize ( dIteratorRowIDs.GetLength() );

		RowID_t * pRowIdStart = m_dCollected.Begin();
		RowID_t * pRowID = pRowIdStart;

		for ( auto i : dIteratorRowIDs )
		{
			if ( !m_tDeadRowMap.IsSet(i) )
				*pRowID++ = i;
		}

		dRowIdBlock = RowIdBlock_t ( pRowIdStart, pRowID - pRowIdStart );
		return true;	// always return true, even if all values were filtered out. next call will fetch more values
	}

	DWORD GetNumProcessed() const
	{
		return (DWORD)m_pIterator->GetNumProcessed();
	}

private:
	RowidIterator_i *				m_pIterator;
	CSphVector<RowID_t>				m_dCollected {0};
	const DeadRowMap_Disk_c &		m_tDeadRowMap;
};

//////////////////////////////////////////////////////////////////////////

template <bool HAS_FILTER_CALC, bool HAS_SORT_CALC, bool HAS_FILTER, bool HAS_RANDOMIZE, bool HAS_MAX_TIMER, bool HAS_CUTOFF, typename ITERATOR, typename TO_STATIC>
void Fullscan ( ITERATOR & tIterator, TO_STATIC && fnToStatic, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, int iIndexWeight, int64_t tmMaxTimer, bool & bStop )
{
	RowIdBlock_t dRowIDs;
	while ( !bStop && tIterator.GetNextRowIdBlock(dRowIDs) )
		for ( auto & i : dRowIDs )
		{
			tMatch.m_tRowID = i;
			tMatch.m_pStatic = fnToStatic(i);

			// early filter only (no late filters in full-scan because of no @weight)
			if_const ( HAS_FILTER_CALC )
				tCtx.CalcFilter(tMatch);

			if_const ( HAS_FILTER )
			{
				if ( !tCtx.m_pFilter->Eval(tMatch) )
				{
					if_const ( HAS_FILTER_CALC )
						tCtx.FreeDataFilter ( tMatch );

					continue;
				}
			}

			if_const ( HAS_RANDOMIZE )
				tMatch.m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;

			if_const ( HAS_SORT_CALC )
				tCtx.CalcSort(tMatch);

			bool bNewMatch = false;
			dSorters.Apply ( [&tMatch, &bNewMatch] ( ISphMatchSorter * p ) { bNewMatch |= p->Push ( tMatch ); } );

			// stringptr expressions should be duplicated (or taken over) at this point
			if_const ( HAS_FILTER_CALC )
				tCtx.FreeDataFilter ( tMatch );

			if_const ( HAS_SORT_CALC )
				tCtx.FreeDataSort ( tMatch );

			if_const ( HAS_CUTOFF )
			{
				if ( bNewMatch && --iCutoff==0 )
				{
					bStop = true;
					break;
				}
			}

			// handle timer
			if_const ( HAS_MAX_TIMER )
			{
				if ( sph::TimeExceeded ( tmMaxTimer ) )
				{
					tMeta.m_sWarning = "query time exceeded max_query_time";
					bStop = true;
					break;
				}
			}
		}

	tMeta.m_tStats.m_iFetchedDocs = (DWORD)tIterator.GetNumProcessed();
}


template <typename ITERATOR, typename TO_STATIC>
void RunFullscan ( ITERATOR & tIterator, TO_STATIC && fnToStatic, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *>& dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer, bool & bStop )
{
	bool bHasFilterCalc = !tCtx.m_dCalcFilter.IsEmpty();
	bool bHasSortCalc = !tCtx.m_dCalcSort.IsEmpty();
	bool bHasFilter = !!tCtx.m_pFilter;
	bool bHasTimer = tmMaxTimer>0;
	bool bHasCutoff = iCutoff!=-1;
	int iIndex = bHasFilterCalc*32 + bHasSortCalc*16 + bHasFilter*8 + bRandomize*4 + bHasTimer*2 + bHasCutoff;

	switch ( iIndex )
	{
#define DECL_FNSCAN( _, n, params ) case n: Fullscan<!!(n&32), !!(n&16), !!(n&8), !!(n&4), !!(n&2), !!(n&1), ITERATOR, TO_STATIC> params; break;
	BOOST_PP_REPEAT ( 64, DECL_FNSCAN, ( tIterator, std::forward<TO_STATIC> ( fnToStatic ), tCtx, tMeta, dSorters, tMatch, iCutoff, iIndexWeight, tmMaxTimer, bStop ) )
#undef DECL_FNSCAN
		default:
			assert ( 0 && "Internal error" );
			break;
	}
}


void CSphIndex_VLN::RunFullscanOnAttrs ( const RowIdBoundaries_t & tBoundaries, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer, bool & bStop ) const
{
	const CSphRowitem * pStart = m_tAttr.GetWritePtr();
	int iStride = m_tSchema.GetRowSize();
	auto fnToStatic = [pStart, iStride]( RowID_t tRowID ){ return pStart+(int64_t)tRowID*iStride; };

	if ( m_tDeadRowMap.HasDead() )
	{
		RowIterator_T<true> tIt ( tBoundaries, m_tDeadRowMap );
		RunFullscan ( tIt, fnToStatic, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer, bStop );
	}
	else
	{
		RowIterator_T<false> tIt ( tBoundaries, m_tDeadRowMap );
		RunFullscan ( tIt, fnToStatic, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer, bStop );
	}
}


void CSphIndex_VLN::RunFullscanOnIterator ( RowidIterator_i * pIterator, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer ) const
{
	bool bStop = false;
	const CSphRowitem * pStart = m_tAttr.GetWritePtr();
	int iStride = m_tSchema.GetRowSize();
	auto fnToStatic = [pStart, iStride]( RowID_t tRowID ){ return pStart+(int64_t)tRowID*iStride; };

	if ( m_tDeadRowMap.HasDead() )
	{
		RowIteratorAlive_c tIt ( pIterator, m_tDeadRowMap );
		RunFullscan ( tIt, fnToStatic, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer, bStop );
	}
	else
		RunFullscan ( *pIterator, fnToStatic, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer, bStop );
}


template <bool ROWID_LIMITS>
void CSphIndex_VLN::ScanByBlocks ( const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer, const RowIdBoundaries_t * pBoundaries ) const
{
	int iStartIndexEntry = 0;
	int iEndIndexEntry = (int)m_iDocinfoIndex;
	if ( ROWID_LIMITS )
	{
		assert(pBoundaries);

		iStartIndexEntry = pBoundaries->m_tMinRowID / DOCINFO_INDEX_FREQ;
		iEndIndexEntry =   pBoundaries->m_tMaxRowID / DOCINFO_INDEX_FREQ + 1;
	}

	int iStride = m_tSchema.GetRowSize();
	for ( int64_t iIndexEntry=iStartIndexEntry; iIndexEntry!=iEndIndexEntry; iIndexEntry++ )
	{
		// block-level filtering
		const DWORD * pMin = &m_pDocinfoIndex[ iIndexEntry*iStride*2 ];
		const DWORD * pMax = pMin + iStride;
		if ( tCtx.m_pFilter && !tCtx.m_pFilter->EvalBlock ( pMin, pMax ) )
			continue;

		RowIdBoundaries_t tBlockBoundaries;
		tBlockBoundaries.m_tMinRowID = RowID_t ( iIndexEntry*DOCINFO_INDEX_FREQ );
		tBlockBoundaries.m_tMaxRowID = (RowID_t)Min ( ( iIndexEntry+1 )*DOCINFO_INDEX_FREQ, m_iDocinfo ) - 1;

		bool bStop = false;

		if ( ROWID_LIMITS )
		{
			// clamp block start/end to match limits. need this only for first/last blocks
			if ( iIndexEntry==iStartIndexEntry || iIndexEntry==iEndIndexEntry-1 )
			{
				assert(pBoundaries);
				tBlockBoundaries.m_tMinRowID = Max ( tBlockBoundaries.m_tMinRowID, pBoundaries->m_tMinRowID );
				tBlockBoundaries.m_tMaxRowID = Min ( tBlockBoundaries.m_tMaxRowID, pBoundaries->m_tMaxRowID );
			}
		}

		RunFullscanOnAttrs ( tBlockBoundaries, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer, bStop );

		if ( bStop || !iCutoff )
			break;
	}
}


RowidIterator_i * CSphIndex_VLN::CreateColumnarAnalyzerOrPrefilter ( const CSphVector<CSphFilterSettings> & dFilters, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged, const CSphVector<FilterTreeItem_t> & dFilterTree, const ISphFilter * pFilter, ESphCollation eCollation, const ISphSchema & tSchema, CSphString & sWarning ) const
{
	bFiltersChanged = false;

	if ( !m_pColumnar.Ptr() || dFilterTree.GetLength() || !pFilter )
		return nullptr;

	std::vector<columnar::Filter_t> dColumnarFilters;
	std::vector<int> dFilterMap;
	dFilterMap.resize ( dFilters.GetLength() );
	ARRAY_FOREACH ( i, dFilters )
	{
		dFilterMap[i] = -1;
		const CSphColumnInfo * pCol = tSchema.GetAttr ( dFilters[i].m_sAttrName.cstr() );
		bool bColumnarFilter = pCol && ( pCol->IsColumnar() || pCol->IsColumnarExpr() );
		bool bRowIdFilter = dFilters[i].m_sAttrName=="@rowid";
		if ( ( bColumnarFilter || bRowIdFilter )  && AddColumnarFilter ( dColumnarFilters, dFilters[i], eCollation, tSchema, sWarning ) )
			dFilterMap[i] = (int)dColumnarFilters.size()-1;
	}

	if ( dColumnarFilters.empty() || ( dColumnarFilters.size()==1 && dColumnarFilters[0].m_sName=="@rowid" ) )
		return nullptr;

	std::vector<int> dDeletedFilters;
	std::vector<columnar::BlockIterator_i *> dIterators;
	dIterators = m_pColumnar->CreateAnalyzerOrPrefilter ( dColumnarFilters, dDeletedFilters, *pFilter );
	if ( dIterators.empty() )
		return nullptr;

	const CSphFilterSettings * pRowIdFilter = nullptr;
	ARRAY_FOREACH ( i, dFilters )
		if ( dFilters[i].m_sAttrName=="@rowid" )
		{
			dDeletedFilters.push_back(i);
			pRowIdFilter = &dFilters[i];
			break;
		}

	dModifiedFilters.Resize(0);
	bFiltersChanged = true;
	for ( size_t i=0; i < dFilterMap.size(); i++ )
		if ( dFilterMap[i]==-1 || !std::binary_search ( dDeletedFilters.begin(), dDeletedFilters.end(), dFilterMap[i] ) )
			dModifiedFilters.Add ( dFilters[i] );

	RowIdBoundaries_t tBoundaries;
	if ( pRowIdFilter )
		tBoundaries = GetFilterRowIdBoundaries ( *pRowIdFilter, RowID_t(m_iDocinfo) );

	if ( dIterators.size()==1 )
		return CreateIteratorWrapper ( dIterators[0], pRowIdFilter ? &tBoundaries : nullptr );

	return CreateIteratorIntersect ( dIterators, pRowIdFilter ? &tBoundaries : nullptr );
}


static void UpdateSpawnedIterators ( bool bChanged, bool & bFiltersChanged, CSphVector<CSphFilterSettings> & dOriginalFilters, CSphVector<CSphFilterSettings> & dModifiedFilters, const CSphVector<CSphFilterSettings> * & pOriginalFilters, CSphVector<RowidIterator_i*> & dIterators, RowidIterator_i * pIterator )
{
	if ( pIterator )
		dIterators.Add(pIterator);

	if ( bChanged )
	{
		bFiltersChanged = true;
		dOriginalFilters = dModifiedFilters;
		pOriginalFilters = &dOriginalFilters;
	}
}


RowidIterator_i * CSphIndex_VLN::SpawnIterators ( const CSphQuery & tQuery, CSphVector<CSphFilterSettings> & dModifiedFilters, bool & bFiltersChanged, const ISphFilter * pFilter, const ISphSchema & tMaxSorterSchema, CSphString & sWarning ) const
{
	CSphVector<CSphFilterSettings> dOriginalFilters;
	const CSphVector<CSphFilterSettings> * pOriginalFilters = &tQuery.m_dFilters;

	CSphVector<RowidIterator_i*> dIterators;

	// try to spawn an iterator from a secondary index
	{
		bool bChanged = false;
		RowidIterator_i * pIterator = m_pHistograms ? CreateFilteredIterator ( *pOriginalFilters, dModifiedFilters, bChanged, tQuery.m_dFilterTree, tQuery.m_dIndexHints, *m_pHistograms, m_tDocidLookup.GetWritePtr(), RowID_t(m_iDocinfo) ) : nullptr;
		UpdateSpawnedIterators ( bChanged, bFiltersChanged, dOriginalFilters, dModifiedFilters, pOriginalFilters, dIterators, pIterator );
	}

	// try to spawn analyzers or prefilters from columnar storage
	{
		bool bChanged = false;
		RowidIterator_i * pIterator = CreateColumnarAnalyzerOrPrefilter ( *pOriginalFilters, dModifiedFilters, bChanged, tQuery.m_dFilterTree, pFilter, tQuery.m_eCollation, tMaxSorterSchema, sWarning );
		UpdateSpawnedIterators ( bChanged, bFiltersChanged, dOriginalFilters, dModifiedFilters, pOriginalFilters, dIterators, pIterator );
	}

	switch ( dIterators.GetLength() )
	{
	case 0:		return nullptr;
	case 1:		return dIterators[0];
	default:	return CreateIteratorIntersect ( dIterators, nullptr );	// both columnar iterator wrappers and secondary index iterators support rowid filtering, so no need for it here
	}
}


bool CSphIndex_VLN::MultiScan ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const
{
	assert ( tArgs.m_iTag>=0 );
	auto& tMeta = *tResult.m_pMeta;

	// check if index is ready
	if ( !m_bPassedAlloc )
	{
		tMeta.m_sError = "index not preread";
		return false;
	}

	// check if index supports scans
	if ( !m_tSchema.GetAttrsCount() )
	{
		tMeta.m_sError = "need attributes to run fullscan";
		return false;
	}

	// we count documents only (before filters)
	if ( tQuery.m_iMaxPredictedMsec )
		tMeta.m_bHasPrediction = true;

	if ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE )
		tMeta.m_sWarning.SetSprintf ( "packedfactors() will not work with a fullscan; you need to specify a query" );

	// check if index has data
	if ( m_bIsEmpty || m_iDocinfo<=0 )
		return true;

	// start counting
	int64_t tmQueryStart = sphMicroTimer();
	int64_t tmCpuQueryStart = sphTaskCpuTimer();

	ScopedThreadPriority_c tPrio ( tQuery.m_bLowPriority );

	// select the sorter with max schema
	int iMaxSchemaIndex = GetMaxSchemaIndexAndMatchCapacity ( dSorters ).first;
	const ISphSchema & tMaxSorterSchema = *(dSorters[iMaxSchemaIndex]->GetSchema());
	auto dSorterSchemas = SorterSchemas ( dSorters, iMaxSchemaIndex);

	// setup calculations and result schema
	CSphQueryContext tCtx ( tQuery );
	if ( !tCtx.SetupCalc ( tMeta, tMaxSorterSchema, m_tSchema, m_tBlobAttrs.GetWritePtr(), m_pColumnar.Ptr(), dSorterSchemas ) )
		return false;

	// set blob pool for string on_sort expression fix up
	tCtx.SetBlobPool ( m_tBlobAttrs.GetWritePtr() );
	tCtx.SetColumnar ( m_pColumnar.Ptr() );

	// setup filters
	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &tQuery.m_dFilters;
	tFlx.m_pFilterTree = &tQuery.m_dFilterTree;
	tFlx.m_pSchema = &tMaxSorterSchema;
	tFlx.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	tFlx.m_pColumnar = m_pColumnar.Ptr();
	tFlx.m_eCollation = tQuery.m_eCollation;
	tFlx.m_bScan = true;
	tFlx.m_pHistograms = m_pHistograms;
	tFlx.m_iTotalDocs = m_iDocinfo;

	if ( !tCtx.CreateFilters ( tFlx, tMeta.m_sError, tMeta.m_sWarning ) )
		return false;

	if ( CheckEarlyReject ( tQuery.m_dFilters, tCtx.m_pFilter, tQuery.m_eCollation, tMaxSorterSchema ) )
	{
		tMeta.m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
		tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;
		return true;
	}

	// setup sorters
	for ( auto & i : dSorters )
	{
		i->SetBlobPool ( m_tBlobAttrs.GetWritePtr() );
		i->SetColumnar ( m_pColumnar.Ptr() );
	}

	// prepare to work them rows
	bool bRandomize = dSorters[0]->IsRandom();

	CSphMatch tMatch;
	tMatch.Reset ( tMaxSorterSchema.GetDynamicSize() );
	tMatch.m_iWeight = tArgs.m_iIndexWeight;
	// fixme! tag also used over bitmask | 0x80000000,
	// which marks that match comes from remote.
	// using -1 might be also interpreted as 0xFFFFFFFF in such context!
	// Does it intended?
	tMatch.m_iTag = tCtx.m_dCalcFinal.GetLength() ? -1 : tArgs.m_iTag;

	SwitchProfile ( tMeta.m_pProfile, SPH_QSTATE_FULLSCAN );

	// run full scan with block and row filtering for everything else
	int iCutoff = ( tQuery.m_iCutoff<=0 ) ? -1 : tQuery.m_iCutoff;

	// we don't modify the original filters because iterators may use some data from them (to avoid copying)
	CSphVector<CSphFilterSettings> dModifiedFilters;
	bool bFiltersChanged = false;
	CSphScopedPtr<RowidIterator_i> pIterator ( SpawnIterators ( tQuery, dModifiedFilters, bFiltersChanged, tCtx.m_pFilter, tMaxSorterSchema, tMeta.m_sWarning ) );
	if ( pIterator )
	{
		// one or several filters got replaced by an iterator, need to re-create the remaining filters
		if ( bFiltersChanged )
		{
			SafeDelete ( tCtx.m_pFilter );
			tFlx.m_pFilters = &dModifiedFilters;
			tCtx.CreateFilters ( tFlx, tMeta.m_sError, tMeta.m_sWarning );
		}

		RunFullscanOnIterator ( pIterator.Ptr(), tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer );
	}
	else
	{
		RowIdBoundaries_t tBoundaries = { 0, RowID_t(m_iDocinfo)-1 };
		const CSphFilterSettings * pRowIdFilter = nullptr;
		for ( const auto & tFilter : tQuery.m_dFilters )
			if ( tFilter.m_sAttrName=="@rowid" )
			{
				pRowIdFilter = &tFilter;
				break;
			}

		if ( pRowIdFilter )
			tBoundaries = GetFilterRowIdBoundaries ( *pRowIdFilter, RowID_t(m_iDocinfo) );

		bool bAllColumnar = tQuery.m_dFilters.all_of ( [this]( const CSphFilterSettings & tFilter ) {
			const CSphColumnInfo * pCol = m_tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
			return pCol ? ( pCol->IsColumnar() || pCol->IsColumnarExpr() ) : false;
		} );

		if ( bAllColumnar )
		{
			bool bStop = false;
			RunFullscanOnAttrs ( tBoundaries, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer, bStop );
		}
		else
		{
			if ( pRowIdFilter )
				ScanByBlocks<true> ( tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer, &tBoundaries );
			else
				ScanByBlocks<false> ( tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer );
		}
	}

	SwitchProfile ( tMeta.m_pProfile, SPH_QSTATE_FINALIZE );

	// do final expression calculations
	if ( tCtx.m_dCalcFinal.GetLength() )
	{
		SphFinalMatchCalc_t tFinal ( tArgs.m_iTag, tCtx );
		dSorters.Apply ( [&tFinal] ( ISphMatchSorter * p ) { p->Finalize ( tFinal, false ); } );
	}

	if ( tArgs.m_bModifySorterSchemas )
	{
		SwitchProfile ( tMeta.m_pProfile, SPH_QSTATE_DYNAMIC );
		PooledAttrsToPtrAttrs ( dSorters, m_tBlobAttrs.GetReadPtr(), m_pColumnar.Ptr() );
	}

	// done
	tResult.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	tResult.m_pDocstore = m_pDocstore.Ptr() ? this : nullptr;
	tResult.m_pColumnar = m_pColumnar.Ptr();

	tMeta.m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
	tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;

	return true;
}

//////////////////////////////////////////////////////////////////////////////

ISphQword * DiskIndexQwordSetup_c::QwordSpawn ( const XQKeyword_t & tWord ) const
{
	if ( !tWord.m_pPayload )
	{
		WITH_QWORD ( m_pIndex, false, Qword, return new Qword ( tWord.m_bExpanded, tWord.m_bExcluded ) );
	} else
	{
		if ( m_pIndex->GetSettings().m_eHitFormat==SPH_HIT_FORMAT_INLINE )
		{
			return new DiskPayloadQword_c<true> ( (const DiskSubstringPayload_t *)tWord.m_pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist );
		} else
		{
			return new DiskPayloadQword_c<false> ( (const DiskSubstringPayload_t *)tWord.m_pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist );
		}
	}
	return NULL;
}


bool DiskIndexQwordSetup_c::QwordSetup ( ISphQword * pWord ) const
{
	auto * pMyWord = (DiskIndexQwordTraits_c*)pWord;

	// setup attrs
	pMyWord->m_tDoc.Reset ( m_iDynamicRowitems );
	pMyWord->m_tDoc.m_tRowID = INVALID_ROWID;

	return pMyWord->Setup ( this );
}

bool DiskIndexQwordSetup_c::SetupWithWrd ( const DiskIndexQwordTraits_c& tWord, CSphDictEntry& tRes ) const
{
	auto* pIndex = (CSphIndex_VLN*)const_cast<CSphIndex*> ( m_pIndex );
	const char * sWord = tWord.m_sDictWord.cstr();
	int iWordLen = sWord ? (int) strlen ( sWord ) : 0;
	if ( tWord.m_sWord.Ends("*") )
	{
		iWordLen = Max ( iWordLen-1, 0 );

		// might match either infix or prefix
		int iMinLen = Max ( pIndex->m_tSettings.GetMinPrefixLen ( true ), pIndex->m_tSettings.m_iMinInfixLen );
		if ( pIndex->m_tSettings.GetMinPrefixLen ( true ) )
			iMinLen = Min ( iMinLen, pIndex->m_tSettings.GetMinPrefixLen ( true ) );
		if ( pIndex->m_tSettings.m_iMinInfixLen )
			iMinLen = Min ( iMinLen, pIndex->m_tSettings.m_iMinInfixLen );

		// bail out term shorter than prefix or infix allowed
		if ( iWordLen<iMinLen )
			return false;
	}

	// leading special symbols trimming
	if ( tWord.m_sDictWord.Begins("*") )
	{
		++sWord;
		iWordLen = Max ( iWordLen-1, 0 );
		// bail out term shorter than infix allowed
		if ( iWordLen<pIndex->m_tSettings.m_iMinInfixLen )
			return false;
	}

	const CSphWordlistCheckpoint * pCheckpoint = pIndex->m_tWordlist.FindCheckpointWrd ( sWord, iWordLen, false );
	if ( !pCheckpoint )
		return false;

	// decode wordlist chunk
	const BYTE * pBuf = pIndex->m_tWordlist.AcquireDict ( pCheckpoint );
	assert ( pBuf );
	assert ( m_iSkiplistBlockSize>0 );

	KeywordsBlockReader_c tCtx ( pBuf, m_iSkiplistBlockSize );
	while ( tCtx.UnpackWord() )
	{
		// block is sorted
		// so once keywords are greater than the reference word, no more matches
		assert ( tCtx.GetWordLen()>0 );
		int iCmp = sphDictCmpStrictly ( sWord, iWordLen, tCtx.GetWord(), tCtx.GetWordLen() );
		if ( iCmp<0 )
			return false;
		if ( iCmp==0 )
			break;
	}
	if ( tCtx.GetWordLen()<=0 )
		return false;
	tRes = tCtx;
	return true;
}

bool DiskIndexQwordSetup_c::SetupWithCrc ( const DiskIndexQwordTraits_c& tWord, CSphDictEntry& tRes ) const
{
	auto * pIndex = (CSphIndex_VLN *)const_cast<CSphIndex *>(m_pIndex);
	const CSphWordlistCheckpoint * pCheckpoint = pIndex->m_tWordlist.FindCheckpointCrc ( tWord.m_uWordID );
	if ( !pCheckpoint )
		return false;

	const BYTE * pBuf = pIndex->m_tWordlist.AcquireDict ( pCheckpoint );
	assert ( pBuf );
	assert ( m_iSkiplistBlockSize>0 );
	return pIndex->m_tWordlist.GetWord ( pBuf, tWord.m_uWordID, tRes );
}

bool DiskIndexQwordSetup_c::Setup ( ISphQword * pWord ) const
{
	// there was a dynamic_cast here once but it's not necessary
	// maybe it worth to rewrite class hierarchy to avoid c-cast here?
	DiskIndexQwordTraits_c & tWord = *(DiskIndexQwordTraits_c*)pWord;

	// setup stats
	tWord.m_iDocs = 0;
	tWord.m_iHits = 0;

	auto * pIndex = (CSphIndex_VLN *)const_cast<CSphIndex *>(m_pIndex);

	// !COMMIT FIXME!
	// the below stuff really belongs in wordlist
	// which in turn really belongs in dictreader
	// which in turn might or might not be a part of dict

	// binary search through checkpoints for a one whose range matches word ID
	assert ( pIndex->m_bPassedAlloc );
	assert ( !pIndex->m_tWordlist.m_tBuf.IsEmpty() );

	// empty index?
	if ( !pIndex->m_tWordlist.m_dCheckpoints.GetLength() )
		return false;

	CSphDictEntry tRes;
	if ( pIndex->m_pDict->GetSettings().m_bWordDict )
	{
		if ( !SetupWithWrd ( tWord, tRes ) )
			return false;
	} else if ( !SetupWithCrc ( tWord, tRes ) )
		return false;

	const ESphHitless eMode = pIndex->m_tSettings.m_eHitless;
	tWord.m_iDocs = eMode==SPH_HITLESS_SOME ? ( tRes.m_iDocs & HITLESS_DOC_MASK ) : tRes.m_iDocs;
	tWord.m_iHits = tRes.m_iHits;
	tWord.m_bHasHitlist =
		( eMode==SPH_HITLESS_NONE ) ||
		( eMode==SPH_HITLESS_SOME && !( tRes.m_iDocs & HITLESS_DOC_FLAG ) );

	if ( m_bSetupReaders )
	{
		tWord.SetDocReader ( m_pDoclist );

		// read in skiplist
		// OPTIMIZE? maybe cache hot decompressed lists?
		// OPTIMIZE? maybe add an option to decompress on preload instead?
		if ( m_pSkips && tRes.m_iDocs>m_iSkiplistBlockSize )
		{
			const BYTE * pSkip = m_pSkips + tRes.m_iSkiplistOffset;

			tWord.m_dSkiplist.Add();
			tWord.m_dSkiplist.Last().m_tBaseRowIDPlus1 = 0;
			tWord.m_dSkiplist.Last().m_iOffset = tRes.m_iDoclistOffset;
			tWord.m_dSkiplist.Last().m_iBaseHitlistPos = 0;

			for ( int i=1; i<( tWord.m_iDocs/m_iSkiplistBlockSize ); i++ )
			{
				SkiplistEntry_t & t = tWord.m_dSkiplist.Add();
				SkiplistEntry_t & p = tWord.m_dSkiplist [ tWord.m_dSkiplist.GetLength()-2 ];
				t.m_tBaseRowIDPlus1 = p.m_tBaseRowIDPlus1 + m_iSkiplistBlockSize + sphUnzipInt ( pSkip );
				t.m_iOffset = p.m_iOffset + 4*m_iSkiplistBlockSize + sphUnzipOffset ( pSkip );
				t.m_iBaseHitlistPos = p.m_iBaseHitlistPos + sphUnzipOffset ( pSkip );
			}
		}

		tWord.m_rdDoclist->SeekTo ( tRes.m_iDoclistOffset, tRes.m_iDoclistHint );
		tWord.SetHitReader ( m_pHitlist );
	}

	return true;
}

QwordScan_c::QwordScan_c ( int iRowsCount )
	: m_iRowsCount ( iRowsCount )
{
	m_bDone = ( m_iRowsCount==0 );

	m_iDocs = m_iRowsCount;
	m_sWord = "";
	m_sDictWord = "";
	m_bExcluded = true;
	m_dQwordFields.SetAll();
}

const CSphMatch & QwordScan_c::GetNextDoc()
{
	if ( m_bDone )
	{
		m_tDoc.m_tRowID = INVALID_ROWID;
		return m_tDoc;
	}

	if ( m_tDoc.m_tRowID==INVALID_ROWID )
		m_tDoc.m_tRowID = 0;
	else
		m_tDoc.m_tRowID++;

	if ( m_tDoc.m_tRowID>=m_iRowsCount )
	{
		m_bDone = true;
		m_tDoc.m_tRowID = INVALID_ROWID;
	}

	return m_tDoc;
}

ISphQword * DiskIndexQwordSetup_c::ScanSpawn() const
{
	return new QwordScan_c ( m_iRowsCount );
}

//////////////////////////////////////////////////////////////////////////////

static bool RawFileLock ( const CSphString sFile, int &iLockFD, CSphString &sError )
{
	if ( iLockFD<0 )
	{
		iLockFD = ::open ( sFile.cstr (), SPH_O_NEW, 0644 );
		if ( iLockFD<0 )
		{
			sError.SetSprintf ( "failed to open %s: %s", sFile.cstr (), strerrorm ( errno ) );
			sphLogDebug ( "failed to open %s: %s", sFile.cstr (), strerrorm ( errno ) );
			return false;
		}
	}

	if ( !sphLockEx ( iLockFD, false ) )
	{
		sError.SetSprintf ( "failed to lock %s: %s", sFile.cstr (), strerrorm ( errno ) );
		::close ( iLockFD );
		iLockFD = -1;
		return false;
	}
	sphLogDebug ( "lock %s success", sFile.cstr () );
	return true;
}

static void RawFileUnLock ( const CSphString sFile, int &iLockFD )
{
	if ( iLockFD<0 )
		return;
	sphLogDebug ( "File ID ok, closing lock FD %d, unlinking %s", iLockFD, sFile.cstr () );
	sphLockUn ( iLockFD );
	::close ( iLockFD );
	::unlink ( sFile.cstr () );
	iLockFD = -1;
}

bool CSphIndex_VLN::Lock ()
{
	CSphString sName = GetIndexFileName(SPH_EXT_SPL);
	sphLogDebug ( "Locking the index via file %s", sName.cstr() );
	return RawFileLock ( sName, m_iLockFD, m_sLastError );
}

void CSphIndex_VLN::Unlock()
{
	CSphString sName = GetIndexFileName(SPH_EXT_SPL);
	if ( m_iLockFD<0 )
		return;
	sphLogDebug ( "Unlocking the index (lock %s)", sName.cstr() );
	RawFileUnLock ( sName, m_iLockFD );
}


void CSphIndex_VLN::Dealloc ()
{
	if ( !m_bPassedAlloc )
		return;

	m_pDoclistFile = nullptr;
	m_pHitlistFile = nullptr;
	m_pColumnar = nullptr;

	m_tAttr.Reset ();
	m_tBlobAttrs.Reset();
	m_tSkiplists.Reset ();
	m_tWordlist.Reset ();
	m_tDeadRowMap.Dealloc();
	m_tDocidLookup.Reset();
	m_pDocstore.Reset();

	m_iDocinfo = 0;
	m_iMinMaxIndex = 0;

	m_bPassedRead = false;
	m_bPassedAlloc = false;
	m_uAttrsStatus = 0;

	QcacheDeleteIndex ( m_iIndexId );
	m_iIndexId = m_tIdGenerator.fetch_add ( 1, std::memory_order_relaxed );
}


void LoadIndexSettings ( CSphIndexSettings & tSettings, CSphReader & tReader, DWORD uVersion )
{
	tSettings.SetMinPrefixLen ( tReader.GetDword() );
	tSettings.m_iMinInfixLen = tReader.GetDword ();
	tSettings.m_iMaxSubstringLen = tReader.GetDword();

	tSettings.m_bHtmlStrip = !!tReader.GetByte ();
	tSettings.m_sHtmlIndexAttrs = tReader.GetString ();
	tSettings.m_sHtmlRemoveElements = tReader.GetString ();

	tSettings.m_bIndexExactWords = !!tReader.GetByte ();
	tSettings.m_eHitless = (ESphHitless)tReader.GetDword();

	tSettings.m_eHitFormat = (ESphHitFormat)tReader.GetDword();
	tSettings.m_bIndexSP = !!tReader.GetByte();

	tSettings.m_sZones = tReader.GetString();

	tSettings.m_iBoundaryStep = (int)tReader.GetDword();
	tSettings.m_iStopwordStep = (int)tReader.GetDword();

	tSettings.m_iOvershortStep = (int)tReader.GetDword();
	tSettings.m_iEmbeddedLimit = (int)tReader.GetDword();

	tSettings.m_eBigramIndex = (ESphBigram)tReader.GetByte();
	tSettings.m_sBigramWords = tReader.GetString();

	tSettings.m_bIndexFieldLens = ( tReader.GetByte()!=0 );

	tSettings.m_ePreprocessor = tReader.GetByte()==1 ? Preprocessor_e::ICU : Preprocessor_e::NONE;
	tReader.GetString(); // was: RLP context

	tSettings.m_sIndexTokenFilter = tReader.GetString();
	tSettings.m_tBlobUpdateSpace = tReader.GetOffset();

	if ( uVersion<56 )
		tSettings.m_iSkiplistBlockSize = 128;
	else
		tSettings.m_iSkiplistBlockSize = (int)tReader.GetDword();

	if ( uVersion>=60 )
		tSettings.m_sHitlessFiles = tReader.GetString();

	if ( uVersion>=63 )
		tSettings.m_eEngine = (AttrEngine_e)tReader.GetDword();
}


bool CSphIndex_VLN::LoadHeader ( const char * sHeaderName, bool bStripPath, CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder, CSphString & sWarning )
{
	const int MAX_HEADER_SIZE = 32768;
	CSphFixedVector<BYTE> dCacheInfo ( MAX_HEADER_SIZE );

	CSphAutoreader rdInfo ( dCacheInfo.Begin(), MAX_HEADER_SIZE ); // to avoid mallocs
	if ( !rdInfo.Open ( sHeaderName, m_sLastError ) )
		return false;

	// magic header
	const char* sFmt = CheckFmtMagic ( rdInfo.GetDword () );
	if ( sFmt )
	{
		m_sLastError.SetSprintf ( sFmt, sHeaderName );
		return false;
	}

	// version
	m_uVersion = rdInfo.GetDword();
	if ( m_uVersion<=1 || m_uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%u, binary is v.%u", sHeaderName, m_uVersion, INDEX_FORMAT_VERSION );
		return false;
	}

	// we don't support anything prior to v54
	DWORD uMinFormatVer = 54;
	if ( m_uVersion<uMinFormatVer )
	{
		m_sLastError.SetSprintf ( "indexes prior to v.%u are no longer supported (use index_converter tool); %s is v.%u", uMinFormatVer, sHeaderName, m_uVersion );
		return false;
	}

	// schema
	ReadSchema ( rdInfo, m_tSchema, m_uVersion );

	// check schema for dupes
	for ( int iAttr=0; iAttr<m_tSchema.GetAttrsCount(); iAttr++ )
	{
		const CSphColumnInfo & tCol = m_tSchema.GetAttr(iAttr);
		for ( int i=0; i<iAttr; i++ )
			if ( m_tSchema.GetAttr(i).m_sName==tCol.m_sName )
				sWarning.SetSprintf ( "duplicate attribute name: %s", tCol.m_sName.cstr() );
	}

	// dictionary header (wordlist checkpoints, infix blocks, etc)
	m_tWordlist.m_iDictCheckpointsOffset = rdInfo.GetOffset();
	m_tWordlist.m_iDictCheckpoints = rdInfo.GetDword();
	m_tWordlist.m_iInfixCodepointBytes = rdInfo.GetByte();
	m_tWordlist.m_iInfixBlocksOffset = rdInfo.GetDword();
	m_tWordlist.m_iInfixBlocksWordsSize = rdInfo.GetDword();

	m_tWordlist.m_dCheckpoints.Reset ( m_tWordlist.m_iDictCheckpoints );

	// index stats
	m_tStats.m_iTotalDocuments = rdInfo.GetDword ();
	m_tStats.m_iTotalBytes = rdInfo.GetOffset ();

	LoadIndexSettings ( m_tSettings, rdInfo, m_uVersion );

	CSphTokenizerSettings tTokSettings;

	// tokenizer stuff
	if ( !tTokSettings.Load ( pFilenameBuilder, rdInfo, tEmbeddedFiles, m_sLastError ) )
		return false;

	if ( bStripPath )
		StripPath ( tTokSettings.m_sSynonymsFile );

	StrVec_t dWarnings;
	TokenizerRefPtr_c pTokenizer { ISphTokenizer::Create ( tTokSettings, &tEmbeddedFiles, pFilenameBuilder, dWarnings, m_sLastError ) };
	if ( !pTokenizer )
		return false;

	// dictionary stuff
	CSphDictSettings tDictSettings;
	tDictSettings.Load ( rdInfo, tEmbeddedFiles, sWarning );

	if ( bStripPath )
	{
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
			StripPath ( tDictSettings.m_dWordforms[i] );
	}

	DictRefPtr_c pDict { tDictSettings.m_bWordDict
		? sphCreateDictionaryKeywords ( tDictSettings, &tEmbeddedFiles, pTokenizer, m_sIndexName.cstr(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError )
		: sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, pTokenizer, m_sIndexName.cstr(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError )};

	if ( !pDict )
		return false;

	if ( tDictSettings.m_sMorphFingerprint!=pDict->GetMorphDataFingerprint() )
		sWarning.SetSprintf ( "different lemmatizer dictionaries (index='%s', current='%s')",
			tDictSettings.m_sMorphFingerprint.cstr(),
			pDict->GetMorphDataFingerprint().cstr() );

	SetDictionary ( pDict );

	pTokenizer = ISphTokenizer::CreateMultiformFilter ( pTokenizer, pDict->GetMultiWordforms () );
	SetTokenizer ( pTokenizer );
	SetupQueryTokenizer();

	// initialize AOT if needed
	m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );

	m_iDocinfo = rdInfo.GetOffset ();
	m_iDocinfoIndex = rdInfo.GetOffset ();
	m_iMinMaxIndex = rdInfo.GetOffset ();

	FieldFilterRefPtr_c pFieldFilter;
	CSphFieldFilterSettings tFieldFilterSettings;
	tFieldFilterSettings.Load(rdInfo);
	if ( tFieldFilterSettings.m_dRegexps.GetLength() )
		pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );

	if ( !sphSpawnFilterICU ( pFieldFilter, m_tSettings, tTokSettings, sHeaderName, m_sLastError ) )
		return false;

	SetFieldFilter ( pFieldFilter );

	if ( m_tSettings.m_bIndexFieldLens )
		for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
			m_dFieldLens[i] = rdInfo.GetOffset(); // FIXME? ideally 64bit even when off is 32bit..

	// post-load stuff.. for now, bigrams
	CSphIndexSettings & s = m_tSettings;
	if ( s.m_eBigramIndex!=SPH_BIGRAM_NONE && s.m_eBigramIndex!=SPH_BIGRAM_ALL )
	{
		BYTE * pTok;
		m_pTokenizer->SetBuffer ( (BYTE*)const_cast<char*> ( s.m_sBigramWords.cstr() ), s.m_sBigramWords.Length() );
		while ( ( pTok = m_pTokenizer->GetToken() )!=NULL )
			s.m_dBigramWords.Add() = (const char*)pTok;
		s.m_dBigramWords.Sort();
	}


	if ( rdInfo.GetErrorFlag() )
		m_sLastError.SetSprintf ( "%s: failed to parse header (unexpected eof)", sHeaderName );

	return !rdInfo.GetErrorFlag();
}


void CSphIndex_VLN::DebugDumpHeader ( FILE * fp, const char * sHeaderName, bool bConfig )
{
	CreateFilenameBuilder_fn fnCreateFilenameBuilder = GetIndexFilenameBuilder();
	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( fnCreateFilenameBuilder ? fnCreateFilenameBuilder ( m_sIndexName.cstr() ) : nullptr );

	CSphEmbeddedFiles tEmbeddedFiles;
	CSphString sWarning;
	if ( !LoadHeader ( sHeaderName, false, tEmbeddedFiles, pFilenameBuilder.Ptr(), sWarning ) )
		sphDie ( "failed to load header: %s", m_sLastError.cstr ());

	if ( !sWarning.IsEmpty () )
		fprintf ( fp, "WARNING: %s\n", sWarning.cstr () );

	///////////////////////////////////////////////
	// print header in index config section format
	///////////////////////////////////////////////

	if ( bConfig )
	{
		fprintf ( fp, "\nsource $dump\n{\n" );

		fprintf ( fp, "\tsql_query = SELECT id \\\n" );
		for ( int i=0; i < m_tSchema.GetFieldsCount(); i++ )
			fprintf ( fp, "\t, %s \\\n", m_tSchema.GetFieldName(i) );
		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
			fprintf ( fp, "\t, %s \\\n", tAttr.m_sName.cstr() );
		}
		fprintf ( fp, "\tFROM documents\n" );

		if ( m_tSchema.GetAttrsCount() )
			fprintf ( fp, "\n" );

		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
			if ( tAttr.m_eAttrType==SPH_ATTR_UINT32SET )
				fprintf ( fp, "\tsql_attr_multi = uint %s from field\n", tAttr.m_sName.cstr() );
			else if ( tAttr.m_eAttrType==SPH_ATTR_INT64SET )
				fprintf ( fp, "\tsql_attr_multi = bigint %s from field\n", tAttr.m_sName.cstr() );
			else if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.IsBitfield() )
				fprintf ( fp, "\tsql_attr_uint = %s:%d\n", tAttr.m_sName.cstr(), tAttr.m_tLocator.m_iBitCount );
			else if ( tAttr.m_eAttrType==SPH_ATTR_TOKENCOUNT )
			{; // intendedly skip, as these are autogenerated by index_field_lengths=1
			} else
				fprintf ( fp, "\t%s = %s\n", sphTypeDirective ( tAttr.m_eAttrType ), tAttr.m_sName.cstr() );
		}

		fprintf ( fp, "}\n\nindex $dump\n{\n\tsource = $dump\n\tpath = $dump\n" );

		DumpSettingsCfg ( fp, *this, pFilenameBuilder.Ptr() );

		fprintf ( fp, "}\n" );
		return;
	}

	///////////////////////////////////////////////
	// print header and stats in "readable" format
	///////////////////////////////////////////////

	fprintf ( fp, "version: %u\n",			m_uVersion );
	fprintf ( fp, "idbits: 64\n" );

	fprintf ( fp, "fields: %d\n", m_tSchema.GetFieldsCount() );
	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
		fprintf ( fp, "  field %d: %s\n", i, m_tSchema.GetFieldName(i) );

	fprintf ( fp, "attrs: %d\n", m_tSchema.GetAttrsCount() );
	for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		fprintf ( fp, "  attr %d: %s, %s", i, tAttr.m_sName.cstr(), sphTypeName ( tAttr.m_eAttrType ) );
		if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.m_iBitCount!=32 )
			fprintf ( fp, ", bits %d", tAttr.m_tLocator.m_iBitCount );
		fprintf ( fp, ", bitoff %d\n", tAttr.m_tLocator.m_iBitOffset );
	}

	// skipped min doc, wordlist checkpoints
	fprintf ( fp, "total-documents: " INT64_FMT "\n", m_tStats.m_iTotalDocuments );
	fprintf ( fp, "total-bytes: " INT64_FMT "\n", int64_t(m_tStats.m_iTotalBytes) );

	DumpReadable ( fp, *this, tEmbeddedFiles, pFilenameBuilder.Ptr() );

	fprintf ( fp, "\nmin-max-index: " INT64_FMT "\n", m_iMinMaxIndex );
}


void CSphIndex_VLN::DebugDumpDocids ( FILE * fp )
{
	const int iRowStride = m_tSchema.GetRowSize();

	const int64_t iNumMinMaxRow = (m_iDocinfoIndex+1)*iRowStride*2;
	const int64_t iNumRows = m_iDocinfo;

	const int64_t iDocinfoSize = iRowStride*m_iDocinfo*sizeof(DWORD);
	const int64_t iMinmaxSize = iNumMinMaxRow*sizeof(CSphRowitem);

	fprintf ( fp, "docinfo-bytes: docinfo=" INT64_FMT ", min-max=" INT64_FMT ", total=" UINT64_FMT "\n", iDocinfoSize, iMinmaxSize, (uint64_t)m_tAttr.GetLengthBytes() );
	fprintf ( fp, "docinfo-stride: %d\n", (int)(iRowStride*sizeof(DWORD)) );
	fprintf ( fp, "docinfo-rows: " INT64_FMT "\n", iNumRows );

	if ( !m_tAttr.GetLength64() )
		return;

	DWORD * pDocinfo = m_tAttr.GetWritePtr();
	for ( int64_t iRow=0; iRow<iNumRows; iRow++, pDocinfo+=iRowStride )
		printf ( INT64_FMT". id=" INT64_FMT "\n", iRow+1, sphGetDocID ( pDocinfo ) );

	printf ( "--- min-max=" INT64_FMT " ---\n", iNumMinMaxRow );
	for ( int64_t iRow=0; iRow<(m_iDocinfoIndex+1)*2; iRow++, pDocinfo+=iRowStride )
		printf ( "id=" INT64_FMT "\n", sphGetDocID ( pDocinfo ) );
}


void CSphIndex_VLN::DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID )
{
	WITH_QWORD ( this, false, Qword, DumpHitlist<Qword> ( fp, sKeyword, bID ) );
}


template < class Qword >
void CSphIndex_VLN::DumpHitlist ( FILE * fp, const char * sKeyword, bool bID )
{
	// get keyword id
	SphWordID_t uWordID = 0;
	BYTE * sTok = NULL;
	if ( !bID )
	{
		CSphString sBuf ( sKeyword );

		m_pTokenizer->SetBuffer ( (const BYTE*)sBuf.cstr(), (int) strlen ( sBuf.cstr() ) );
		sTok = m_pTokenizer->GetToken();

		if ( !sTok )
			sphDie ( "keyword=%s, no token (too short?)", sKeyword );

		uWordID = m_pDict->GetWordID ( sTok );
		if ( !uWordID )
			sphDie ( "keyword=%s, tok=%s, no wordid (stopped?)", sKeyword, sTok );

		fprintf ( fp, "keyword=%s, tok=%s, wordid=" UINT64_FMT "\n", sKeyword, sTok, uint64_t(uWordID) );

	} else
	{
		uWordID = (SphWordID_t) strtoull ( sKeyword, NULL, 10 );
		if ( !uWordID )
			sphDie ( "failed to convert keyword=%s to id (must be integer)", sKeyword );

		fprintf ( fp, "wordid=" UINT64_FMT "\n", uint64_t(uWordID) );
	}

	// open files
	DataReaderFactoryPtr_c pDoclist {
		NewProxyReader ( GetIndexFileName ( SPH_EXT_SPD ), m_sLastError, DataReaderFactory_c::DOCS,
			m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE )
	};
	if ( !pDoclist )
		sphDie ( "failed to open doclist: %s", m_sLastError.cstr() );

	DataReaderFactoryPtr_c pHitlist {
		NewProxyReader ( GetIndexFileName ( SPH_EXT_SPP ), m_sLastError, DataReaderFactory_c::HITS,
			m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE )
	};
	if ( !pHitlist )
		sphDie ( "failed to open hitlist: %s", m_sLastError.cstr ());


	// aim
	DiskIndexQwordSetup_c tTermSetup ( pDoclist, pHitlist, m_tSkiplists.GetWritePtr(), m_tSettings.m_iSkiplistBlockSize, true, RowID_t(m_iDocinfo) );
	tTermSetup.SetDict ( m_pDict );
	tTermSetup.m_pIndex = this;

	Qword tKeyword ( false, false );
	tKeyword.m_uWordID = uWordID;
	tKeyword.m_sWord = sKeyword;
	tKeyword.m_sDictWord = (const char *)sTok;
	if ( !tTermSetup.QwordSetup ( &tKeyword ) )
		sphDie ( "failed to setup keyword" );

	// press play on tape
	while (true)
	{
		tKeyword.GetNextDoc();
		if ( tKeyword.m_tDoc.m_tRowID==INVALID_ROWID )
			break;

		tKeyword.SeekHitlist ( tKeyword.m_iHitlistPos );

		int iHits = 0;
		if ( tKeyword.m_bHasHitlist )
			for ( Hitpos_t uHit = tKeyword.GetNextHit(); uHit!=EMPTY_HIT; uHit = tKeyword.GetNextHit() )
			{
				fprintf ( fp, "doc=%u, hit=0x%08x\n", tKeyword.m_tDoc.m_tRowID, (uint32_t)uHit );
				iHits++;
			}

		if ( !iHits )
		{
			uint64_t uOff = tKeyword.m_iHitlistPos;
			fprintf ( fp, "doc=%u, NO HITS, inline=%d, off=" UINT64_FMT "\n", tKeyword.m_tDoc.m_tRowID, (int)(uOff>>63), (uOff<<1)>>1 );
		}
	}
}


void CSphIndex_VLN::DebugDumpDict ( FILE * fp )
{
	if ( !m_pDict->GetSettings().m_bWordDict )
	{
		sphDie ( "DebugDumpDict() only supports dict=keywords for now" );
	}

	fprintf ( fp, "keyword,docs,hits,offset\n" );
	m_tWordlist.DebugPopulateCheckpoints();
	ARRAY_FOREACH ( i, m_tWordlist.m_dCheckpoints )
	{
		KeywordsBlockReader_c tCtx ( m_tWordlist.AcquireDict ( &m_tWordlist.m_dCheckpoints[i] ), m_tSettings.m_iSkiplistBlockSize );
		while ( tCtx.UnpackWord() )
			fprintf ( fp, "%s,%d,%d," INT64_FMT "\n", tCtx.GetWord(), tCtx.m_iDocs, tCtx.m_iHits, int64_t(tCtx.m_iDoclistOffset) );
	}
}

//////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::SpawnReader ( DataReaderFactoryPtr_c & m_pFile, ESphExt eExt, DataReaderFactory_c::Kind_e eKind, int iBuffer, FileAccess_e eAccess )
{
	if ( m_tMutableSettings.m_bPreopen || eAccess!=FileAccess_e::FILE )
	{
		m_pFile = NewProxyReader ( GetIndexFileName(eExt), m_sLastError, eKind, iBuffer, eAccess );
		if ( !m_pFile )
			return false;
	}

	return true;
}


bool CSphIndex_VLN::SpawnReaders()
{
	if ( !SpawnReader ( m_pDoclistFile, SPH_EXT_SPD, DataReaderFactory_c::DOCS, m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, m_tMutableSettings.m_tFileAccess.m_eDoclist ) )
		return false;

	if ( !SpawnReader ( m_pHitlistFile, SPH_EXT_SPP, DataReaderFactory_c::HITS, m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, m_tMutableSettings.m_tFileAccess.m_eHitlist ) )
		return false;

	return true;
}


bool CSphIndex_VLN::PreallocWordlist()
{
	if ( !sphIsReadable ( GetIndexFileName(SPH_EXT_SPI).cstr(), &m_sLastError ) )
		return false;

	// might be no dictionary at this point for old index format
	bool bWordDict = m_pDict && m_pDict->GetSettings().m_bWordDict;

	// only checkpoint and wordlist infixes are actually read here; dictionary itself is just mapped
	if ( !m_tWordlist.Preread ( GetIndexFileName(SPH_EXT_SPI), bWordDict, m_tSettings.m_iSkiplistBlockSize, m_sLastError ) )
		return false;

	if ( ( m_tWordlist.m_tBuf.GetLengthBytes()<=1 )!=( m_tWordlist.m_dCheckpoints.GetLength()==0 ) )
		sphWarning ( "wordlist size mismatch (size=%zu, checkpoints=%d)", m_tWordlist.m_tBuf.GetLengthBytes(), m_tWordlist.m_dCheckpoints.GetLength() );

	// make sure checkpoints are loadable
	// pre-11 indices use different offset type (this is fixed up later during the loading)
	assert ( m_tWordlist.m_iDictCheckpointsOffset>0 );

	return true;
}


bool CSphIndex_VLN::PreallocAttributes()
{
	if ( m_bIsEmpty || m_bDebugCheck )
		return true;

	if ( !m_tSchema.HasNonColumnarAttrs() )
		return true;

	if ( !m_tAttr.Setup ( GetIndexFileName(SPH_EXT_SPA), m_sLastError, true ) )
		return false;

	if ( !CheckDocsCount ( m_iDocinfo, m_sLastError ) )
		return false;

	m_pDocinfoIndex = m_tAttr.GetWritePtr() + m_iMinMaxIndex;

	if ( m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
	{
		if ( !m_tBlobAttrs.Setup ( GetIndexFileName(SPH_EXT_SPB), m_sLastError, true ) )
			return false;
	}

	return true;
}


bool CSphIndex_VLN::PreallocDocidLookup()
{
	if ( m_bIsEmpty || m_bDebugCheck )
		return true;

	if ( !m_tDocidLookup.Setup ( GetIndexFileName(SPH_EXT_SPT), m_sLastError, false ) )
		return false;

	m_tLookupReader.SetData ( m_tDocidLookup.GetWritePtr() );

	return true;
}


bool CSphIndex_VLN::PreallocKilllist()
{
	if ( m_bDebugCheck )
		return true;

	return m_tDeadRowMap.Prealloc ( (DWORD)m_iDocinfo, GetIndexFileName(SPH_EXT_SPM), m_sLastError );
}


bool CSphIndex_VLN::PreallocHistograms ( StrVec_t & dWarnings )
{
	if ( m_bDebugCheck )
		return true;

	// we have histograms since v.56, but in v.61 we switched to streamed histograms with no backward compatibility
	if ( m_uVersion<61 )
		return true;

	CSphString sHistogramFile = GetIndexFileName(SPH_EXT_SPHI);
	if ( !sphIsReadable ( sHistogramFile.cstr() ) )
		return true;

	SafeDelete ( m_pHistograms );
	m_pHistograms = new HistogramContainer_c;

	if ( !m_pHistograms->Load ( sHistogramFile, m_sLastError ) )
	{
		SafeDelete ( m_pHistograms );
		if ( !m_sLastError.IsEmpty() )
			dWarnings.Add(m_sLastError);
	}

	// even if we fail to load the histograms, return true (histograms are optional anyway)
	return true;
}


bool CSphIndex_VLN::PreallocDocstore()
{
	if ( m_uVersion<57 )
		return true;

	if ( !m_tSchema.HasStoredFields() )
		return true;

	m_pDocstore = CreateDocstore ( GetIndexFileName(SPH_EXT_SPDS), m_sLastError );

	return !!m_pDocstore;
}


bool CSphIndex_VLN::PreallocColumnar()
{
	if ( m_uVersion<61 )
		return true;

	if ( !m_tSchema.HasColumnarAttrs() )
		return true;

	m_pColumnar = CreateColumnarStorageReader ( GetIndexFileName(SPH_EXT_SPC).cstr(), (DWORD)m_iDocinfo, m_sLastError );
	return !!m_pColumnar;
}


bool CSphIndex_VLN::PreallocSkiplist()
{
	if ( m_bDebugCheck )
		return true;

	return m_tSkiplists.Setup ( GetIndexFileName(SPH_EXT_SPE), m_sLastError, false );
}


bool CSphIndex_VLN::Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings )
{
	MEMORY ( MEM_INDEX_DISK );

	Dealloc();

	CSphEmbeddedFiles tEmbeddedFiles;

	// preload schema
	if ( !LoadHeader ( GetIndexFileName(SPH_EXT_SPH).cstr(), bStripPath, tEmbeddedFiles, pFilenameBuilder, m_sLastWarning ) )
		return false;

	m_bIsEmpty = !m_iDocinfo;

	tEmbeddedFiles.Reset();

	// verify that data files are readable
	if ( !sphIsReadable ( GetIndexFileName(SPH_EXT_SPD).cstr(), &m_sLastError ) )
		return false;

	if ( !sphIsReadable ( GetIndexFileName(SPH_EXT_SPP).cstr(), &m_sLastError ) )
		return false;

	if ( !sphIsReadable ( GetIndexFileName(SPH_EXT_SPE).cstr(), &m_sLastError ) )
		return false;

	if ( !SpawnReaders() )
		return false;

	if ( !PreallocWordlist() )		return false;
	if ( !PreallocAttributes() )	return false;
	if ( !PreallocDocidLookup() )	return false;
	if ( !PreallocKilllist() )		return false;
	if ( !PreallocHistograms(dWarnings) ) return false;
	if ( !PreallocDocstore() )		return false;
	if ( !PreallocColumnar() )		return false;
	if ( !PreallocSkiplist() )		return false;

	// almost done
	m_bPassedAlloc = true;

	return true;
}


void CSphIndex_VLN::Preread()
{
	MEMORY ( MEM_INDEX_DISK );

	sphLogDebug ( "CSphIndex_VLN::Preread invoked '%s'(%s)", m_sIndexName.cstr(), m_sFilename.cstr() );

	assert ( m_bPassedAlloc );
	if ( m_bPassedRead )
		return;

	///////////////////
	// read everything
	///////////////////

	PrereadMapping ( m_sIndexName.cstr(), "attributes", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eAttr ), m_tAttr );
	PrereadMapping ( m_sIndexName.cstr(), "blobs", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eBlob ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eBlob ), m_tBlobAttrs );
	PrereadMapping ( m_sIndexName.cstr(), "skip-list", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), false, m_tSkiplists );
	PrereadMapping ( m_sIndexName.cstr(), "dictionary", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), false, m_tWordlist.m_tBuf );
	PrereadMapping ( m_sIndexName.cstr(), "docid-lookup", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), false, m_tDocidLookup );
	m_tDeadRowMap.Preread ( m_sIndexName.cstr(), "kill-list", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ) );

	m_bPassedRead = true;
	sphLogDebug ( "Preread successfully finished" );
}

void CSphIndex_VLN::SetBase ( const char * sNewBase )
{
	m_sFilename = sNewBase;
}


bool CSphIndex_VLN::Rename ( const char * sNewBase )
{
	if ( m_sFilename==sNewBase )
		return true;

	IndexFiles_c dFiles ( m_sFilename, nullptr, m_uVersion );
	if ( !dFiles.RenameBase ( sNewBase ) )
	{
		m_sLastError = dFiles.ErrorMsg ();
		return false;
	}

	if ( !dFiles.RenameLock ( sNewBase, m_iLockFD ) )
	{
		m_sLastError = dFiles.ErrorMsg ();
		return false;
	}

	SetBase ( sNewBase );

	return true;
}

//////////////////////////////////////////////////////////////////////////

CSphQueryContext::CSphQueryContext ( const CSphQuery & q )
	: m_tQuery ( q )
{}

CSphQueryContext::~CSphQueryContext ()
{
	ResetFilters();
}

void CSphQueryContext::ResetFilters()
{
	SafeDelete ( m_pFilter );
	SafeDelete ( m_pWeightFilter );
	m_dUserVals.Reset();
}

void CSphQueryContext::BindWeights ( const CSphQuery & tQuery, const CSphSchema & tSchema, CSphString & sWarning )
{
	const int HEAVY_FIELDS = SPH_MAX_FIELDS;

	// defaults
	m_iWeights = Min ( tSchema.GetFieldsCount(), HEAVY_FIELDS );
	for ( int i=0; i<m_iWeights; ++i )
		m_dWeights[i] = 1;

	// name-bound weights
	CSphString sFieldsNotFound;
	if ( !tQuery.m_dFieldWeights.IsEmpty() )
	{
		for ( auto& tWeight : tQuery.m_dFieldWeights )
		{
			int j = tSchema.GetFieldIndex ( tWeight.first.cstr() );
			if ( j<0 )
			{
				if ( sFieldsNotFound.IsEmpty() )
					sFieldsNotFound = tWeight.first;
				else
					sFieldsNotFound.SetSprintf ( "%s %s", sFieldsNotFound.cstr(), tWeight.first.cstr() );
			}

			if ( j>=0 && j<HEAVY_FIELDS )
				m_dWeights[j] = tWeight.second;
		}

		if ( !sFieldsNotFound.IsEmpty() )
			sWarning.SetSprintf ( "Fields specified in field_weights option not found: [%s]", sFieldsNotFound.cstr() );

		return;
	}

	// order-bound weights
	if ( !tQuery.m_dWeights.IsEmpty() )
	{
		for ( int i=0, iLim=Min ( m_iWeights, tQuery.m_dWeights.GetLength() ); i<iLim; ++i )
			m_dWeights[i] = (int) tQuery.m_dWeights[i];
	}
}

static ESphEvalStage GetEarliestStage ( ESphEvalStage eStage, const CSphColumnInfo & tIn, const CSphVector<const ISphSchema *> & dSchemas )
{
	for ( const auto * pSchema : dSchemas )
	{
		const CSphColumnInfo * pCol = pSchema->GetAttr ( tIn.m_sName.cstr() );
		if ( !pCol )
			continue;

		eStage = Min ( eStage, pCol->m_eStage );
	}

	return eStage;
}


bool CSphQueryContext::SetupCalc ( CSphQueryResultMeta & tMeta, const ISphSchema & tInSchema, const CSphSchema & tSchema, const BYTE * pBlobPool, const columnar::Columnar_i * pColumnar,
	const CSphVector<const ISphSchema *> & dInSchemas )
{
	m_dCalcFilter.Resize(0);
	m_dCalcSort.Resize(0);
	m_dCalcFinal.Resize(0);

	m_dCalcFilterPtrAttrs.Resize(0);
	m_dCalcSortPtrAttrs.Resize(0);

	// quickly verify that all my real attributes can be stashed there
	if ( tInSchema.GetAttrsCount() < tSchema.GetAttrsCount() )
	{
		tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema mismatch (incount=%d, mycount=%d)",
			tInSchema.GetAttrsCount(), tSchema.GetAttrsCount() );
		return false;
	}

	// now match everyone
	for ( int iIn=0; iIn<tInSchema.GetAttrsCount(); iIn++ )
	{
		const CSphColumnInfo & tIn = tInSchema.GetAttr(iIn);

		// recalculate stage as sorters set column at earlier stage
		// FIXME!!! should we update column?
		ESphEvalStage eStage = GetEarliestStage ( tIn.m_eStage, tIn, dInSchemas );

		switch ( eStage )
		{
			case SPH_EVAL_STATIC:
			{
				// this check may significantly slow down queries with huge schema attribute count
#ifndef NDEBUG
				const CSphColumnInfo * pMy = tSchema.GetAttr ( tIn.m_sName.cstr() );
				if ( !pMy )
				{
					tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema attr missing from index-schema (in=%s)",
						sphDumpAttr(tIn).cstr() );
					return false;
				}

				// static; check for full match
				if (!( tIn==*pMy ))
				{
					assert ( 0 );
					tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema mismatch (in=%s, my=%s)",
						sphDumpAttr(tIn).cstr(), sphDumpAttr(*pMy).cstr() );
					return false;
				}
#endif
				break;
			}

			case SPH_EVAL_PREFILTER:
			case SPH_EVAL_PRESORT:
			case SPH_EVAL_FINAL:
			{
				ISphExprRefPtr_c pExpr { tIn.m_pExpr };
				if ( !pExpr )
				{
					tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: incoming-schema expression missing evaluator (stage=%d, in=%s)",
						(int)eStage, sphDumpAttr(tIn).cstr() );
					return false;
				}

				// an expression that index/searcher should compute
				CalcItem_t tCalc;
				tCalc.m_eType = tIn.m_eAttrType;
				tCalc.m_tLoc = tIn.m_tLocator;
				tCalc.m_pExpr = std::move(pExpr);
				tCalc.m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)&pBlobPool );
				tCalc.m_pExpr->Command ( SPH_EXPR_SET_COLUMNAR, (void*)pColumnar );

				switch ( eStage )
				{
					case SPH_EVAL_PREFILTER:	AddToFilterCalc(tCalc); break;
					case SPH_EVAL_PRESORT:		AddToSortCalc(tCalc); break;
					case SPH_EVAL_FINAL:		m_dCalcFinal.Add(tCalc); break;
					default:					break;
				}
				break;
			}

			case SPH_EVAL_SORTER:
				// sorter tells it will compute itself; so just skip it
			case SPH_EVAL_POSTLIMIT:
				break;

			default:
				tMeta.m_sError.SetSprintf ( "INTERNAL ERROR: unhandled eval stage=%d", (int)eStage );
				return false;
		}
	}

	// ok, we can emit matches in this schema (incoming for sorter, outgoing for index/searcher)
	return true;
}


void CSphQueryContext::AddToFilterCalc ( const CalcItem_t & tCalc )
{
	m_dCalcFilter.Add(tCalc);
	if ( sphIsDataPtrAttr ( tCalc.m_eType ) )
		m_dCalcFilterPtrAttrs.Add ( m_dCalcFilter.GetLength()-1 );
}


void CSphQueryContext::AddToSortCalc ( const CalcItem_t & tCalc )
{
	m_dCalcSort.Add(tCalc);
	if ( sphIsDataPtrAttr ( tCalc.m_eType ) )
		m_dCalcSortPtrAttrs.Add ( m_dCalcSort.GetLength()-1 );
}


bool CSphIndex::IsStarDict ( bool bWordDict ) const
{
	return m_tSettings.GetMinPrefixLen ( bWordDict )>0 || m_tSettings.m_iMinInfixLen>0;
}


void CSphIndex_VLN::SetupStarDict ( DictRefPtr_c &pDict ) const
{
	// spawn wrapper, and put it in the box
	// wrapper type depends on version; v.8 introduced new mangling rules
	if ( IsStarDict ( pDict->GetSettings().m_bWordDict ) )
		pDict = new CSphDictStarV8 ( pDict, m_tSettings.m_iMinInfixLen>0 );

	// FIXME? might wanna verify somehow that the tokenizer has '*' as a character
}

void CSphIndex_VLN::SetupExactDict ( DictRefPtr_c &pDict ) const
{
	if ( m_tSettings.m_bIndexExactWords )
		pDict = new CSphDictExact ( pDict );
}


bool CSphIndex_VLN::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords,
	const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const
{
	WITH_QWORD ( this, false, Qword, return DoGetKeywords<Qword> ( dKeywords, szQuery, tSettings, false, pError ) );
	return false;
}

void CSphIndex_VLN::GetSuggest ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes ) const
{
	if ( m_tWordlist.m_tBuf.IsEmpty() || !m_tWordlist.m_dCheckpoints.GetLength() )
		return;

	assert ( !tRes.m_pWordReader );
	tRes.m_pWordReader = new KeywordsBlockReader_c ( m_tWordlist.m_tBuf.GetWritePtr(), m_tSettings.m_iSkiplistBlockSize );
	tRes.m_bHasExactDict = m_tSettings.m_bIndexExactWords;

	sphGetSuggest ( &m_tWordlist, m_tWordlist.m_iInfixCodepointBytes, tArgs, tRes );

	KeywordsBlockReader_c * pReader = (KeywordsBlockReader_c *)tRes.m_pWordReader;
	SafeDelete ( pReader );
	tRes.m_pWordReader = NULL;
}


DWORD sphParseMorphAot ( const char * sMorphology )
{
	if ( !sMorphology || !*sMorphology )
		return 0;

	StrVec_t dMorphs;
	sphSplit ( dMorphs, sMorphology );

	DWORD uAotFilterMask = 0;
	for ( int j=0; j<AOT_LENGTH; ++j )
	{
		char buf_all[20];
		sprintf ( buf_all, "lemmatize_%s_all", AOT_LANGUAGES[j] ); // NOLINT
		ARRAY_FOREACH ( i, dMorphs )
		{
			if ( dMorphs[i]==buf_all )
			{
				uAotFilterMask |= (1UL) << j;
				break;
			}
		}
	}

	return uAotFilterMask;
}


void ISphQueryFilter::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const ExpansionContext_t & tCtx )
{
	assert ( m_pTokenizer && m_pDict && m_pSettings );

	BYTE sTokenized[3*SPH_MAX_WORD_LEN+4];
	BYTE * sWord;
	int iQpos = 1;
	CSphVector<int> dQposWildcards;

	// FIXME!!! got rid of duplicated term stat and qword setup
	while ( ( sWord = m_pTokenizer->GetToken() )!=NULL )
	{
		const BYTE * sMultiform = m_pTokenizer->GetTokenizedMultiform();
		strncpy ( (char *)sTokenized, sMultiform ? (const char*)sMultiform : (const char*)sWord, sizeof(sTokenized)-1 );

		if ( ( !m_tFoldSettings.m_bFoldWildcards || m_tFoldSettings.m_bStats ) && sphHasExpandableWildcards ( (const char *)sWord ) )
		{
			dQposWildcards.Add ( iQpos );

			ISphWordlist::Args_t tWordlist ( false, tCtx.m_iExpansionLimit, tCtx.m_bHasExactForms, tCtx.m_eHitless, tCtx.m_pIndexData );
			bool bExpanded = sphExpandGetWords ( (const char *)sWord, tCtx, tWordlist );

			int iDocs = 0;
			int iHits = 0;

			// might fold wildcards but still want to sum up stats
			if ( m_tFoldSettings.m_bFoldWildcards && m_tFoldSettings.m_bStats )
			{
				ARRAY_FOREACH ( i, tWordlist.m_dExpanded )
				{
					iDocs += tWordlist.m_dExpanded[i].m_iDocs;
					iHits += tWordlist.m_dExpanded[i].m_iHits;
				}
				bExpanded = false;
			} else
			{
				ARRAY_FOREACH ( i, tWordlist.m_dExpanded )
				{
					CSphKeywordInfo & tInfo = dKeywords.Add();
					tInfo.m_sTokenized = (const char *)sWord;
					tInfo.m_sNormalized = tWordlist.GetWordExpanded ( i );
					tInfo.m_iDocs = tWordlist.m_dExpanded[i].m_iDocs;
					tInfo.m_iHits = tWordlist.m_dExpanded[i].m_iHits;
					tInfo.m_iQpos = iQpos;

					RemoveDictSpecials ( tInfo.m_sNormalized );
				}
			}

			if ( !bExpanded || !tWordlist.m_dExpanded.GetLength() )
			{
				CSphKeywordInfo & tInfo = dKeywords.Add ();
				tInfo.m_sTokenized = (const char *)sWord;
				tInfo.m_sNormalized = (const char *)sWord;
				tInfo.m_iDocs = iDocs;
				tInfo.m_iHits = iHits;
				tInfo.m_iQpos = iQpos;
			}
		} else
		{
			AddKeywordStats ( sWord, sTokenized, iQpos, dKeywords );
		}

		// FIXME!!! handle consecutive blended wo blended parts
		bool bBlended = m_pTokenizer->TokenIsBlended();

		if ( bBlended )
		{
			if ( m_tFoldSettings.m_bFoldBlended )
				iQpos += m_pTokenizer->SkipBlended();
		} else
		{
			iQpos++;
		}
	}

	if ( !m_pSettings->m_uAotFilterMask )
		return;

	XQLimitSpec_t tSpec;
	BYTE sTmp[3*SPH_MAX_WORD_LEN+4];
	BYTE sTmp2[3*SPH_MAX_WORD_LEN+4];
	CSphVector<XQNode_t *> dChildren ( 64 );

	CSphBitvec tSkipTransform;
	if ( dQposWildcards.GetLength () )
	{
		tSkipTransform.Init ( iQpos+1 );
		ARRAY_FOREACH ( i, dQposWildcards )
			tSkipTransform.BitSet ( dQposWildcards[i] );
	}

	int iTokenizedTotal = dKeywords.GetLength();
	for ( int iTokenized=0; iTokenized<iTokenizedTotal; iTokenized++ )
	{
		int iKeywordQpos = dKeywords[iTokenized].m_iQpos;

		// do not transform expanded wild-cards
		if ( !tSkipTransform.IsEmpty() && tSkipTransform.BitGet ( iKeywordQpos ) )
			continue;

		// MUST copy as Dict::GetWordID changes word and might add symbols
		strncpy ( (char *)sTokenized, dKeywords[iTokenized].m_sNormalized.scstr(), sizeof(sTokenized)-1 );
		int iPreAotCount = dKeywords.GetLength();

		XQNode_t tAotNode ( tSpec );
		tAotNode.m_dWords.Resize ( 1 );
		tAotNode.m_dWords.Begin()->m_sWord = (char *)sTokenized;
		TransformAotFilter ( &tAotNode, m_pDict->GetWordforms(), *m_pSettings );

		dChildren.Resize ( 0 );
		dChildren.Add ( &tAotNode );

		// recursion unfolded
		ARRAY_FOREACH ( iChild, dChildren )
		{
			// process all words at node
			ARRAY_FOREACH ( iAotKeyword, dChildren[iChild]->m_dWords )
			{
				// MUST copy as Dict::GetWordID changes word and might add symbols
				strncpy ( (char *)sTmp, dChildren[iChild]->m_dWords[iAotKeyword].m_sWord.scstr(), sizeof(sTmp)-1 );
				// prevent use-after-free-bug due to vector grow: AddKeywordsStats() calls dKeywords.Add()
				strncpy ( (char *)sTmp2, dKeywords[iTokenized].m_sTokenized.scstr (), sizeof ( sTmp2 )-1 );
				AddKeywordStats ( sTmp, sTmp2, iKeywordQpos, dKeywords );
			}

			// push all child nodes at node to process list
			const XQNode_t * pChild = dChildren[iChild];
			ARRAY_FOREACH ( iRec, pChild->m_dChildren )
				dChildren.Add ( pChild->m_dChildren[iRec] );
		}

		bool bGotLemmas = ( iPreAotCount!=dKeywords.GetLength() );

		// remove (replace) original word in case of AOT taken place
		if ( bGotLemmas )
		{
			if ( !m_tFoldSettings.m_bFoldLemmas )
			{
				::Swap ( dKeywords[iTokenized], dKeywords.Last() );
				dKeywords.Resize ( dKeywords.GetLength()-1 );
			} else
			{
				int iKeywordWithMaxHits = iPreAotCount;
				for ( int i=iPreAotCount+1; i<dKeywords.GetLength(); i++ )
					if ( dKeywords[i].m_iHits > dKeywords[iKeywordWithMaxHits].m_iHits )
						iKeywordWithMaxHits = i;

				CSphString sNormalizedWithMaxHits = dKeywords[iKeywordWithMaxHits].m_sNormalized;

				int iDocs = 0;
				int iHits = 0;
				if ( m_tFoldSettings.m_bStats )
				{
					for ( int i=iPreAotCount; i<dKeywords.GetLength(); i++ )
					{
						iDocs += dKeywords[i].m_iDocs;
						iHits += dKeywords[i].m_iHits;
					}
				}
				::Swap ( dKeywords[iTokenized], dKeywords[iPreAotCount] );
				dKeywords.Resize ( iPreAotCount );
				dKeywords[iTokenized].m_iDocs = iDocs;
				dKeywords[iTokenized].m_iHits = iHits;
				dKeywords[iTokenized].m_sNormalized = sNormalizedWithMaxHits;

				RemoveDictSpecials ( dKeywords[iTokenized].m_sNormalized );
			}
		}
	}

	// sort by qpos
	if ( dKeywords.GetLength()!=iTokenizedTotal )
		sphSort ( dKeywords.Begin(), dKeywords.GetLength(), bind ( &CSphKeywordInfo::m_iQpos ) );
}


struct CSphPlainQueryFilter : public ISphQueryFilter
{
	const ISphQwordSetup *	m_pTermSetup;
	ISphQword *				m_pQueryWord;

	void AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords ) override
	{
		assert ( !m_tFoldSettings.m_bStats || ( m_pTermSetup && m_pQueryWord ) );

		SphWordID_t iWord = m_pDict->GetWordID ( sWord );
		if ( !iWord )
			return;

		if ( m_tFoldSettings.m_bStats )
		{
			m_pQueryWord->Reset ();
			m_pQueryWord->m_sWord = (const char*)sWord;
			m_pQueryWord->m_sDictWord = (const char*)sWord;
			m_pQueryWord->m_uWordID = iWord;
			m_pTermSetup->QwordSetup ( m_pQueryWord );
		}

		CSphKeywordInfo & tInfo = dKeywords.Add();
		tInfo.m_sTokenized = (const char *)sTokenized;
		tInfo.m_sNormalized = (const char*)sWord;
		tInfo.m_iDocs = m_tFoldSettings.m_bStats ? m_pQueryWord->m_iDocs : 0;
		tInfo.m_iHits = m_tFoldSettings.m_bStats ? m_pQueryWord->m_iHits : 0;
		tInfo.m_iQpos = iQpos;

		RemoveDictSpecials ( tInfo.m_sNormalized );
	}
};


template < class Qword >
bool CSphIndex_VLN::DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords,
	const char * szQuery, const GetKeywordsSettings_t & tSettings, bool bFillOnly, CSphString * pError ) const
{
	if ( !bFillOnly )
		dKeywords.Resize ( 0 );

	if ( !m_bPassedAlloc )
	{
		if ( pError )
			*pError = "index not preread";
		return false;
	}

	// short-cut if no query or keywords to fill
	if ( ( bFillOnly && !dKeywords.GetLength() ) || ( !bFillOnly && ( !szQuery || !szQuery[0] ) ) )
		return true;

	DictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };
	SetupStarDict ( pDict );
	SetupExactDict ( pDict );

	CSphVector<BYTE> dFiltered;
	FieldFilterRefPtr_c pFieldFilter;
	const BYTE * sModifiedQuery = (const BYTE *)szQuery;
	if ( m_pFieldFilter && szQuery )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter && pFieldFilter->Apply ( sModifiedQuery, dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	// FIXME!!! missed bigram, add flags to fold blended parts, show expanded terms

	// prepare for setup
	DataReaderFactory_c * tDummy1 = nullptr;
	DataReaderFactory_c * tDummy2 = nullptr;

	DiskIndexQwordSetup_c tTermSetup ( tDummy1, tDummy2, m_tSkiplists.GetWritePtr(), m_tSettings.m_iSkiplistBlockSize, false, RowID_t(m_iDocinfo) );
	tTermSetup.SetDict ( pDict );
	tTermSetup.m_pIndex = this;

	Qword tQueryWord ( false, false );

	if ( bFillOnly )
	{
		BYTE sWord[MAX_KEYWORD_BYTES];

		ARRAY_FOREACH ( i, dKeywords )
		{
			CSphKeywordInfo &tInfo = dKeywords[i];
			int iLen = tInfo.m_sTokenized.Length ();
			memcpy ( sWord, tInfo.m_sTokenized.cstr (), iLen );
			sWord[iLen] = '\0';

			SphWordID_t iWord = pDict->GetWordID ( sWord );
			if ( iWord )
			{
				tQueryWord.Reset ();
				tQueryWord.m_sWord = tInfo.m_sTokenized;
				tQueryWord.m_sDictWord = ( const char * ) sWord;
				tQueryWord.m_uWordID = iWord;
				tTermSetup.QwordSetup ( &tQueryWord );

				tInfo.m_iDocs += tQueryWord.m_iDocs;
				tInfo.m_iHits += tQueryWord.m_iHits;
			}
		}
		return true;
	}

	bool bWordDict = pDict->GetSettings ().m_bWordDict;

	TokenizerRefPtr_c pTokenizer { m_pTokenizer->Clone ( SPH_CLONE_INDEX ) };
	pTokenizer->EnableTokenizedMultiformTracking ();

	// need to support '*' and '=' but not the other specials
	// so m_pQueryTokenizer does not work for us, gotta clone and setup one manually
	if ( IsStarDict ( bWordDict ) )
		pTokenizer->AddPlainChar ( '*' );
	if ( m_tSettings.m_bIndexExactWords )
		pTokenizer->AddPlainChar ( '=' );

	ExpansionContext_t tExpCtx;
	// query defined options
	tExpCtx.m_iExpansionLimit = ( tSettings.m_iExpansionLimit ? tSettings.m_iExpansionLimit : m_iExpansionLimit );
	bool bExpandWildcards = ( bWordDict && IsStarDict ( bWordDict ) && !tSettings.m_bFoldWildcards );

	CSphPlainQueryFilter tAotFilter;
	tAotFilter.m_pTokenizer = pTokenizer;
	tAotFilter.m_pDict = pDict;
	tAotFilter.m_pSettings = &m_tSettings;
	tAotFilter.m_pTermSetup = &tTermSetup;
	tAotFilter.m_pQueryWord = &tQueryWord;
	tAotFilter.m_tFoldSettings = tSettings;
	tAotFilter.m_tFoldSettings.m_bFoldWildcards = !bExpandWildcards;

	tExpCtx.m_pWordlist = &m_tWordlist;
	tExpCtx.m_iMinPrefixLen = m_tSettings.GetMinPrefixLen ( bWordDict );
	tExpCtx.m_iMinInfixLen = m_tSettings.m_iMinInfixLen;
	tExpCtx.m_bHasExactForms = ( m_pDict->HasMorphology() || m_tSettings.m_bIndexExactWords );
	tExpCtx.m_bMergeSingles = false;
	tExpCtx.m_eHitless = m_tSettings.m_eHitless;

	pTokenizer->SetBuffer ( sModifiedQuery, (int) strlen ( (const char *)sModifiedQuery) );

	tAotFilter.GetKeywords ( dKeywords, tExpCtx );
	return true;
}


bool CSphIndex_VLN::FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const
{
	GetKeywordsSettings_t tSettings;
	tSettings.m_bStats = true;

	WITH_QWORD ( this, false, Qword, return DoGetKeywords<Qword> ( dKeywords, NULL, tSettings, true, NULL ) );
	return false;
}


bool CSphQueryContext::CreateFilters ( CreateFilterContext_t & tCtx, CSphString & sError, CSphString & sWarning )
{
	if ( !tCtx.m_pFilters || tCtx.m_pFilters->IsEmpty () )
		return true;
	if ( !sphCreateFilters ( tCtx, sError, sWarning ) )
		return false;

	m_pFilter = tCtx.m_pFilter;
	m_pWeightFilter = tCtx.m_pWeightFilter;
	m_dUserVals.SwapData ( tCtx.m_dUserVals );
	tCtx.m_pFilter = nullptr;
	tCtx.m_pWeightFilter = nullptr;

	return true;
}


static int sphQueryHeightCalc ( const XQNode_t * pNode )
{
	if ( !pNode->m_dChildren.GetLength() )
	{
		// exception, pre-cached OR of tiny (rare) keywords is just one node
		if ( pNode->GetOp()==SPH_QUERY_OR )
		{
#ifndef NDEBUG
			// sanity checks
			// this node must be only created for a huge OR of tiny expansions
			assert ( pNode->m_dWords.GetLength() );
			ARRAY_FOREACH ( i, pNode->m_dWords )
			{
				assert ( pNode->m_dWords[i].m_iAtomPos==pNode->m_dWords[0].m_iAtomPos );
				assert ( pNode->m_dWords[i].m_bExpanded );
			}
#endif
			return 1;
		}
		return pNode->m_dWords.GetLength();
	}

	if ( pNode->GetOp()==SPH_QUERY_BEFORE )
		return 1;

	int iMaxChild = 0;
	int iHeight = 0;
	ARRAY_FOREACH ( i, pNode->m_dChildren )
	{
		int iBottom = sphQueryHeightCalc ( pNode->m_dChildren[i] );
		int iTop = pNode->m_dChildren.GetLength()-i-1;
		if ( iBottom+iTop>=iMaxChild+iHeight )
		{
			iMaxChild = iBottom;
			iHeight = iTop;
		}
	}

	return iMaxChild+iHeight;
}
#ifdef __clang__
#ifndef NDEBUG
#define SPH_EXTNODE_STACK_SIZE (0x120)
#else
#define SPH_EXTNODE_STACK_SIZE (160)
#endif
#else
#if _WIN32
#define SPH_EXTNODE_STACK_SIZE (600)
#else
#define SPH_EXTNODE_STACK_SIZE (160)
#endif
#endif

int ConsiderStack ( const struct XQNode_t * pRoot, CSphString & sError )
{
	int iHeight = 0;
	if ( pRoot )
		iHeight = sphQueryHeightCalc ( pRoot );

	auto iStackNeed = iHeight * SPH_EXTNODE_STACK_SIZE;
	int64_t iQueryStack = sphGetStackUsed ()+iStackNeed;
	auto iMyStackSize = sphMyStackSize ();
	if ( iMyStackSize>=iQueryStack )
		return -1;

	// align as stack of tree + 32K
	// (being run in new coro, most probably you'll start near the top of stack, so 32k should be enouth)
	iQueryStack = iStackNeed + 32*1024;
	if ( g_iMaxCoroStackSize>=iQueryStack )
		return (int)iQueryStack;

	sError.SetSprintf ( "query too complex, not enough stack (thread_stack=%dK or higher required)", (int) (( iQueryStack+1024-( iQueryStack % 1024 )) / 1024 ));
	return 0;
}


static XQNode_t * CloneKeyword ( const XQNode_t * pNode )
{
	assert ( pNode );

	XQNode_t * pRes = new XQNode_t ( pNode->m_dSpec );
	pRes->m_dWords = pNode->m_dWords;
	return pRes;
}


static XQNode_t * ExpandKeyword ( XQNode_t * pNode, const CSphIndexSettings & tSettings, int iExpandKeywords, bool bWordDict )
{
	assert ( pNode );

	if ( tSettings.m_bIndexExactWords && ( iExpandKeywords & KWE_MORPH_NONE )==KWE_MORPH_NONE )
	{
		pNode->m_dWords[0].m_sWord.SetSprintf ( "=%s", pNode->m_dWords[0].m_sWord.cstr() );
		return pNode;
	}

	XQNode_t * pExpand = new XQNode_t ( pNode->m_dSpec );
	pExpand->SetOp ( SPH_QUERY_OR, pNode );

	if ( tSettings.m_iMinInfixLen>0 && ( iExpandKeywords & KWE_STAR )==KWE_STAR )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pInfix = CloneKeyword ( pNode );
		pInfix->m_dWords[0].m_sWord.SetSprintf ( "*%s*", pNode->m_dWords[0].m_sWord.cstr() );
		pInfix->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pInfix );
	} else if ( tSettings.GetMinPrefixLen ( bWordDict )>0 && ( iExpandKeywords & KWE_STAR )==KWE_STAR )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pPrefix = CloneKeyword ( pNode );
		pPrefix->m_dWords[0].m_sWord.SetSprintf ( "%s*", pNode->m_dWords[0].m_sWord.cstr() );
		pPrefix->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pPrefix );
	}

	if ( tSettings.m_bIndexExactWords && ( iExpandKeywords & KWE_EXACT )==KWE_EXACT )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pExact = CloneKeyword ( pNode );
		pExact->m_dWords[0].m_sWord.SetSprintf ( "=%s", pNode->m_dWords[0].m_sWord.cstr() );
		pExact->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pExact );
	}

	return pExpand;
}

void sphQueryExpandKeywords ( XQNode_t ** ppNode, const CSphIndexSettings & tSettings, int iExpandKeywords, bool bWordDict )
{
	assert ( ppNode );
	assert ( *ppNode );
	auto& pNode = *ppNode;
	// only if expansion makes sense at all
	assert ( tSettings.m_iMinInfixLen>0 || tSettings.GetMinPrefixLen ( bWordDict )>0 || tSettings.m_bIndexExactWords );
	assert ( iExpandKeywords!=KWE_DISABLED );

	// process children for composite nodes
	if ( pNode->m_dChildren.GetLength() )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
		{
			sphQueryExpandKeywords ( &pNode->m_dChildren[i], tSettings, iExpandKeywords, bWordDict );
			pNode->m_dChildren[i]->m_pParent = pNode;
		}
		return;
	}

	// if that's a phrase/proximity node, create a very special, magic phrase/proximity node
	if ( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM )
	{
		assert ( pNode->m_dWords.GetLength()>1 );
		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			auto * pWord = new XQNode_t ( pNode->m_dSpec );
			pWord->m_dWords.Add ( pNode->m_dWords[i] );
			pNode->m_dChildren.Add ( ExpandKeyword ( pWord, tSettings, iExpandKeywords, bWordDict ) );
			pNode->m_dChildren.Last()->m_iAtomPos = pNode->m_dWords[i].m_iAtomPos;
			pNode->m_dChildren.Last()->m_pParent = pNode;
		}
		pNode->m_dWords.Reset();
		pNode->m_bVirtuallyPlain = true;
		return;
	}

	// skip empty plain nodes
	if ( pNode->m_dWords.GetLength()<=0 )
		return;

	// process keywords for plain nodes
	assert ( pNode->m_dWords.GetLength()==1 );

	XQKeyword_t & tKeyword = pNode->m_dWords[0];
	if ( tKeyword.m_sWord.Begins("=")
		|| tKeyword.m_sWord.Begins("*")
		|| tKeyword.m_sWord.Ends("*") )
		return;

	// do the expansion
	pNode = ExpandKeyword ( pNode, tSettings, iExpandKeywords, bWordDict );
}


// transform the "one two three"/1 quorum into one|two|three (~40% faster)
static void TransformQuorum ( XQNode_t ** ppNode )
{
	XQNode_t *& pNode = *ppNode;

	// recurse non-quorum nodes
	if ( pNode->GetOp()!=SPH_QUERY_QUORUM )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			TransformQuorum ( &pNode->m_dChildren[i] );
		return;
	}

	// skip quorums with thresholds other than 1
	if ( pNode->m_iOpArg!=1 )
		return;

	// transform quorums with a threshold of 1 only
	assert ( pNode->GetOp()==SPH_QUERY_QUORUM && pNode->m_dChildren.GetLength()==0 );
	CSphVector<XQNode_t*> dArgs;
	ARRAY_FOREACH ( i, pNode->m_dWords )
	{
		XQNode_t * pAnd = new XQNode_t ( pNode->m_dSpec );
		pAnd->m_dWords.Add ( pNode->m_dWords[i] );
		dArgs.Add ( pAnd );
	}
	pNode->m_dWords.Reset();
	pNode->SetOp ( SPH_QUERY_OR, dArgs );
}


struct BinaryNode_t
{
	int m_iLo;
	int m_iHi;
};

static void BuildExpandedTree ( const XQKeyword_t & tRootWord, ISphWordlist::Args_t & dWordSrc, XQNode_t * pRoot )
{
	assert ( dWordSrc.m_dExpanded.GetLength() );
	pRoot->m_dWords.Reset();

	// build a binary tree from all the other expansions
	CSphVector<BinaryNode_t> dNodes;
	dNodes.Reserve ( dWordSrc.m_dExpanded.GetLength() );

	XQNode_t * pCur = pRoot;

	dNodes.Add();
	dNodes.Last().m_iLo = 0;
	dNodes.Last().m_iHi = ( dWordSrc.m_dExpanded.GetLength()-1 );

	while ( dNodes.GetLength() )
	{
		BinaryNode_t tNode = dNodes.Pop();
		if ( tNode.m_iHi<tNode.m_iLo )
		{
			pCur = pCur->m_pParent;
			continue;
		}

		int iMid = ( tNode.m_iLo+tNode.m_iHi ) / 2;
		dNodes.Add ();
		dNodes.Last().m_iLo = tNode.m_iLo;
		dNodes.Last().m_iHi = iMid-1;
		dNodes.Add ();
		dNodes.Last().m_iLo = iMid+1;
		dNodes.Last().m_iHi = tNode.m_iHi;

		if ( pCur->m_dWords.GetLength() )
		{
			assert ( pCur->m_dWords.GetLength()==1 );
			XQNode_t * pTerm = CloneKeyword ( pRoot );
			Swap ( pTerm->m_dWords, pCur->m_dWords );
			pCur->m_dChildren.Add ( pTerm );
			pTerm->m_pParent = pCur;
		}

		XQNode_t * pChild = CloneKeyword ( pRoot );
		pChild->m_dWords.Add ( tRootWord );
		pChild->m_dWords.Last().m_sWord = dWordSrc.GetWordExpanded ( iMid );
		pChild->m_dWords.Last().m_bExpanded = true;
		pChild->m_bNotWeighted = pRoot->m_bNotWeighted;

		pChild->m_pParent = pCur;
		pCur->m_dChildren.Add ( pChild );
		pCur->SetOp ( SPH_QUERY_OR );

		pCur = pChild;
	}
}


/// do wildcard expansion for keywords dictionary
/// (including prefix and infix expansion)
XQNode_t * sphExpandXQNode ( XQNode_t * pNode, ExpansionContext_t & tCtx )
{
	assert ( pNode );

	// process children for composite nodes
	if ( pNode->m_dChildren.GetLength() )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
		{
			pNode->m_dChildren[i] = sphExpandXQNode ( pNode->m_dChildren[i], tCtx );
			pNode->m_dChildren[i]->m_pParent = pNode;
		}
		return pNode;
	}

	// if that's a phrase/proximity node, create a very special, magic phrase/proximity node
	if ( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM )
	{
		assert ( pNode->m_dWords.GetLength()>1 );
		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			XQNode_t * pWord = new XQNode_t ( pNode->m_dSpec );
			pWord->m_dWords.Add ( pNode->m_dWords[i] );
			pNode->m_dChildren.Add ( sphExpandXQNode ( pWord, tCtx ) );
			pNode->m_dChildren.Last()->m_iAtomPos = pNode->m_dWords[i].m_iAtomPos;
			pNode->m_dChildren.Last()->m_pParent = pNode;

			// tricky part
			// current node may have field/zone limits attached
			// normally those get pushed down during query parsing
			// but here we create nodes manually and have to push down limits too
			pWord->CopySpecs ( pNode );
		}
		pNode->m_dWords.Reset();
		pNode->m_bVirtuallyPlain = true;
		return pNode;
	}

	// skip empty plain nodes
	if ( pNode->m_dWords.GetLength()<=0 )
		return pNode;

	// process keywords for plain nodes
	assert ( pNode->m_dChildren.GetLength()==0 );
	assert ( pNode->m_dWords.GetLength()==1 );

	// might be a pass to only fixup of the tree
	if ( tCtx.m_bOnlyTreeFix )
		return pNode;

	assert ( tCtx.m_pResult );

	// check the wildcards
	const char * sFull = pNode->m_dWords[0].m_sWord.cstr();

	// no wildcards, or just wildcards? do not expand
	if ( !sphHasExpandableWildcards ( sFull ) )
		return pNode;

	bool bUseTermMerge = ( tCtx.m_bMergeSingles && pNode->m_dSpec.m_dZones.IsEmpty() );
	ISphWordlist::Args_t tWordlist ( bUseTermMerge, tCtx.m_iExpansionLimit, tCtx.m_bHasExactForms, tCtx.m_eHitless, tCtx.m_pIndexData );

	if ( !sphExpandGetWords ( sFull, tCtx, tWordlist ) )
	{
		tCtx.m_pResult->m_sWarning.SetSprintf ( "Query word length is less than min %s length. word: '%s' ", ( tCtx.m_iMinInfixLen>0 ? "infix" : "prefix" ), sFull );
		return pNode;
	}

	// no real expansions?
	// mark source word as expanded to prevent warning on terms mismatch in statistics
	if ( !tWordlist.m_dExpanded.GetLength() && !tWordlist.m_pPayload )
	{
		tCtx.m_pResult->AddStat ( pNode->m_dWords.Begin()->m_sWord, 0, 0 );
		pNode->m_dWords.Begin()->m_bExpanded = true;
		return pNode;
	}

	// copy the original word (iirc it might get overwritten),
	const XQKeyword_t tRootWord = pNode->m_dWords[0];
	tCtx.m_pResult->AddStat ( tRootWord.m_sWord, tWordlist.m_iTotalDocs, tWordlist.m_iTotalHits );

	// and build a binary tree of all the expansions
	if ( tWordlist.m_dExpanded.GetLength() )
	{
		BuildExpandedTree ( tRootWord, tWordlist, pNode );
	}

	if ( tWordlist.m_pPayload )
	{
		ISphSubstringPayload * pPayload = tWordlist.m_pPayload;
		tWordlist.m_pPayload = NULL;
		tCtx.m_pPayloads->Add ( pPayload );

		if ( pNode->m_dWords.GetLength() )
		{
			// all expanded fit to single payload
			pNode->m_dWords.Begin()->m_bExpanded = true;
			pNode->m_dWords.Begin()->m_pPayload = pPayload;
		} else
		{
			// payload added to expanded binary tree
			assert ( pNode->GetOp()==SPH_QUERY_OR );
			assert ( pNode->m_dChildren.GetLength() );

			XQNode_t * pSubstringNode = new XQNode_t ( pNode->m_dSpec );
			pSubstringNode->SetOp ( SPH_QUERY_OR );

			XQKeyword_t tSubstringWord = tRootWord;
			tSubstringWord.m_bExpanded = true;
			tSubstringWord.m_pPayload = pPayload;
			pSubstringNode->m_dWords.Add ( tSubstringWord );

			pNode->m_dChildren.Add ( pSubstringNode );
			pSubstringNode->m_pParent = pNode;
		}
	}

	return pNode;
}


bool sphHasExpandableWildcards ( const char * sWord )
{
	const char * pCur = sWord;
	int iWilds = 0;

	for ( ; *pCur; pCur++ )
		if ( sphIsWild ( *pCur ) )
			iWilds++;

	int iLen = int ( pCur - sWord );
	return ( iWilds && iWilds<iLen );
}

bool sphExpandGetWords ( const char * sWord, const ExpansionContext_t & tCtx, ISphWordlist::Args_t & tWordlist )
{
	// fix for the case '=*term' that should count as infix
	if ( sWord[0]=='=' && sWord[1]=='*' )
		sWord++;

	if ( !sphIsWild ( *sWord ) || tCtx.m_iMinInfixLen==0 )
	{
		// do prefix expansion
		// remove exact form modifier, if any
		const char * sPrefix = sWord;
		if ( *sPrefix=='=' )
			sPrefix++;

		// skip leading wildcards
		// (in case we got here on non-infixed index path)
		const char * sWildcard = sPrefix;
		while ( sphIsWild ( *sPrefix ) )
		{
			sPrefix++;
			sWildcard++;
		}

		// compute non-wildcard prefix length
		int iPrefix = 0;
		const char * sCodes = sPrefix;
		for ( ; *sCodes && !sphIsWild ( *sCodes ); sCodes+=sphUtf8CharBytes ( *sCodes ) )
			iPrefix++;

		// do not expand prefixes under min length
		if ( iPrefix<tCtx.m_iMinPrefixLen )
			return false;

		int iBytes = int ( sCodes - sPrefix );
		// prefix expansion should work on nonstemmed words only
		char sFixed[MAX_KEYWORD_BYTES];
		if ( tCtx.m_bHasExactForms )
		{
			sFixed[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			memcpy ( sFixed+1, sPrefix, iBytes );
			sPrefix = sFixed;
			iBytes++;
		}

		tCtx.m_pWordlist->GetPrefixedWords ( sPrefix, iBytes, sWildcard, tWordlist );

	} else
	{
		// do infix expansion
		assert ( sphIsWild ( *sWord ) );
		assert ( tCtx.m_iMinInfixLen>0 );

		// find the longest substring of non-wildcards
		int iCodepoints = 0;
		int iInfixCodepoints = 0;
		int iInfixBytes = 0;
		const char * sMaxInfix = NULL;
		const char * sInfix = sWord;

		for ( const char * s = sWord; *s; )
		{
			int iCodeLen = sphUtf8CharBytes ( *s );

			if ( sphIsWild ( *s ) )
			{
				sInfix = s + 1;
				iCodepoints = 0;
			} else
			{
				iCodepoints++;
				if ( s - sInfix + iCodeLen > iInfixBytes )
				{
					sMaxInfix = sInfix;
					iInfixBytes = int ( s - sInfix ) + iCodeLen;
					iInfixCodepoints = iCodepoints;
				}
			}

			s += iCodeLen;
		}

		// do not expand infixes under min_infix_len
		if ( iInfixCodepoints < tCtx.m_iMinInfixLen )
			return false;

		// ignore heading star
		tCtx.m_pWordlist->GetInfixedWords ( sMaxInfix, iInfixBytes, sWord, tWordlist );
	}

	return true;
}

XQNode_t * CSphIndex_VLN::ExpandPrefix ( XQNode_t * pNode, CSphQueryResultMeta & tMeta, CSphScopedPayload * pPayloads, DWORD uQueryDebugFlags ) const
{
	if ( !pNode || !m_pDict->GetSettings().m_bWordDict
			|| ( m_tSettings.GetMinPrefixLen ( m_pDict->GetSettings().m_bWordDict )<=0 && m_tSettings.m_iMinInfixLen<=0 ) )
		return pNode;

	assert ( m_bPassedAlloc );
	assert ( !m_tWordlist.m_tBuf.IsEmpty() );

	ExpansionContext_t tCtx;
	tCtx.m_pWordlist = &m_tWordlist;
	tCtx.m_pResult = &tMeta;
	tCtx.m_iMinPrefixLen = m_tSettings.GetMinPrefixLen ( m_pDict->GetSettings().m_bWordDict );
	tCtx.m_iMinInfixLen = m_tSettings.m_iMinInfixLen;
	tCtx.m_iExpansionLimit = m_iExpansionLimit;
	tCtx.m_bHasExactForms = ( m_pDict->HasMorphology() || m_tSettings.m_bIndexExactWords );
	tCtx.m_bMergeSingles = ( uQueryDebugFlags & QUERY_DEBUG_NO_PAYLOAD )==0;
	tCtx.m_pPayloads = pPayloads;
	tCtx.m_eHitless = m_tSettings.m_eHitless;

	pNode = sphExpandXQNode ( pNode, tCtx );
	pNode->Check ( true );

	return pNode;
}


// transform the (A B) NEAR C into A NEAR B NEAR C
static void TransformNear ( XQNode_t ** ppNode )
{
	XQNode_t *& pNode = *ppNode;
	if ( pNode->GetOp()==SPH_QUERY_NEAR )
	{
		assert ( pNode->m_dWords.GetLength()==0 );
		CSphVector<XQNode_t*> dArgs;
		int iStartFrom;

		// transform all (A B C) NEAR D into A NEAR B NEAR C NEAR D
		do
		{
			dArgs.Reset();
			iStartFrom = 0;
			ARRAY_FOREACH ( i, pNode->m_dChildren )
			{
				XQNode_t * pChild = pNode->m_dChildren[i]; ///< shortcut
				if ( pChild->GetOp()==SPH_QUERY_AND && pChild->m_dChildren.GetLength()>0 )
				{
					ARRAY_FOREACH ( j, pChild->m_dChildren )
					{
						if ( j==0 && iStartFrom==0 )
						{
							// we will remove the node anyway, so just replace it with 1-st child instead
							pNode->m_dChildren[i] = pChild->m_dChildren[j];
							pNode->m_dChildren[i]->m_pParent = pNode;
							iStartFrom = i+1;
						} else
						{
							dArgs.Add ( pChild->m_dChildren[j] );
						}
					}
					pChild->m_dChildren.Reset();
					SafeDelete ( pChild );
				} else if ( iStartFrom!=0 )
				{
					dArgs.Add ( pChild );
				}
			}

			if ( iStartFrom!=0 )
			{
				pNode->m_dChildren.Resize ( iStartFrom + dArgs.GetLength() );
				ARRAY_FOREACH ( i, dArgs )
				{
					pNode->m_dChildren [ i + iStartFrom ] = dArgs[i];
					pNode->m_dChildren [ i + iStartFrom ]->m_pParent = pNode;
				}
			}
		} while ( iStartFrom!=0 );
	}

	ARRAY_FOREACH ( i, pNode->m_dChildren )
		TransformNear ( &pNode->m_dChildren[i] );
}


/// tag excluded keywords (rvals to operator NOT)
static void TagExcluded ( XQNode_t * pNode, bool bNot )
{
	if ( pNode->GetOp()==SPH_QUERY_ANDNOT )
	{
		assert ( pNode->m_dChildren.GetLength()==2 );
		assert ( pNode->m_dWords.GetLength()==0 );
		TagExcluded ( pNode->m_dChildren[0], bNot );
		TagExcluded ( pNode->m_dChildren[1], !bNot );

	} else if ( pNode->m_dChildren.GetLength() )
	{
		// FIXME? check if this works okay with "virtually plain" stuff?
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			TagExcluded ( pNode->m_dChildren[i], bNot );
	} else
	{
		// tricky bit
		// no assert on length here and that is intended
		// we have fully empty nodes (0 children, 0 words) sometimes!
		ARRAY_FOREACH ( i, pNode->m_dWords )
			pNode->m_dWords[i].m_bExcluded = bNot;
	}
}


/// optimize phrase queries if we have bigrams
static void TransformBigrams ( XQNode_t * pNode, const CSphIndexSettings & tSettings )
{
	assert ( tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE );
	assert ( tSettings.m_eBigramIndex==SPH_BIGRAM_ALL || tSettings.m_dBigramWords.GetLength() );

	if ( pNode->GetOp()!=SPH_QUERY_PHRASE )
	{
		ARRAY_FOREACH ( i, pNode->m_dChildren )
			TransformBigrams ( pNode->m_dChildren[i], tSettings );
		return;
	}

	CSphBitvec bmRemove;
	bmRemove.Init ( pNode->m_dWords.GetLength() );

	for ( int i=0; i<pNode->m_dWords.GetLength()-1; i++ )
	{
		// check whether this pair was indexed
		bool bBigram = false;
		switch ( tSettings.m_eBigramIndex )
		{
			case SPH_BIGRAM_NONE:
				break;
			case SPH_BIGRAM_ALL:
				bBigram = true;
				break;
			case SPH_BIGRAM_FIRSTFREQ:
				bBigram = tSettings.m_dBigramWords.BinarySearch ( pNode->m_dWords[i].m_sWord )!=NULL;
				break;
			case SPH_BIGRAM_BOTHFREQ:
				bBigram =
					( tSettings.m_dBigramWords.BinarySearch ( pNode->m_dWords[i].m_sWord )!=NULL ) &&
					( tSettings.m_dBigramWords.BinarySearch ( pNode->m_dWords[i+1].m_sWord )!=NULL );
				break;
		}
		if ( !bBigram )
			continue;

		// replace the pair with a bigram keyword
		// FIXME!!! set phrase weight for this "word" here
		pNode->m_dWords[i].m_sWord.SetSprintf ( "%s%c%s",
			pNode->m_dWords[i].m_sWord.cstr(),
			MAGIC_WORD_BIGRAM,
			pNode->m_dWords[i+1].m_sWord.cstr() );

		// only mark for removal now, we will sweep later
		// so that [a b c] would convert to ["a b" "b c"], not just ["a b" c]
		bmRemove.BitClear ( i );
		bmRemove.BitSet ( i+1 );
	}

	// remove marked words
	int iOut = 0;
	ARRAY_FOREACH ( i, pNode->m_dWords )
		if ( !bmRemove.BitGet(i) )
			pNode->m_dWords[iOut++] = pNode->m_dWords[i];
	pNode->m_dWords.Resize ( iOut );

	// fixup nodes that are not real phrases any more
	if ( pNode->m_dWords.GetLength()==1 )
		pNode->SetOp ( SPH_QUERY_AND );
}


/// create a node from a set of lemmas
/// WARNING, tKeyword might or might not be pointing to pNode->m_dWords[0]
/// Called from the daemon side (searchd) in time of query
static void TransformAotFilterKeyword ( XQNode_t * pNode, LemmatizerTrait_i * pLemmatizer, const XQKeyword_t & tKeyword, const CSphWordforms * pWordforms, const CSphIndexSettings & tSettings )
{
	assert ( pNode->m_dWords.GetLength()<=1 );
	assert ( pNode->m_dChildren.GetLength()==0 );

	XQNode_t * pExact = NULL;
	if ( pWordforms )
	{
		// do a copy, because patching in place is not an option
		// short => longlonglong wordform mapping would crash
		// OPTIMIZE? forms that are not found will (?) get looked up again in the dict
		char sBuf [ MAX_KEYWORD_BYTES ];
		strncpy ( sBuf, tKeyword.m_sWord.cstr(), sizeof(sBuf)-1 );
		if ( pWordforms->ToNormalForm ( (BYTE*)sBuf, true, false ) )
		{
			if ( !pNode->m_dWords.GetLength() )
				pNode->m_dWords.Add ( tKeyword );
			pNode->m_dWords[0].m_sWord = sBuf;
			pNode->m_dWords[0].m_bMorphed = true;
			return;
		}
	}

	StrVec_t dLemmas;
	DWORD uLangMask = tSettings.m_uAotFilterMask;
	for ( int i=AOT_BEGIN; i<AOT_LENGTH; ++i )
	{
		if ( uLangMask & (1UL<<i) )
		{
			if ( i==AOT_RU )
				sphAotLemmatizeRu ( dLemmas, (const BYTE*)tKeyword.m_sWord.cstr() );
			else if ( i==AOT_DE )
				sphAotLemmatizeDe ( dLemmas, (const BYTE*)tKeyword.m_sWord.cstr() );
			else if ( i==AOT_UK )
				sphAotLemmatizeUk ( dLemmas, (const BYTE*)tKeyword.m_sWord.cstr(), pLemmatizer );
			else
				sphAotLemmatize ( dLemmas, (const BYTE*)tKeyword.m_sWord.cstr(), i );
		}
	}

	// post-morph wordforms
	if ( pWordforms && pWordforms->m_bHavePostMorphNF )
	{
		char sBuf [ MAX_KEYWORD_BYTES ];
		ARRAY_FOREACH ( i, dLemmas )
		{
			strncpy ( sBuf, dLemmas[i].cstr(), sizeof(sBuf)-1 );
			if ( pWordforms->ToNormalForm ( (BYTE*)sBuf, false, false ) )
				dLemmas[i] = sBuf;
		}
	}

	if ( dLemmas.GetLength() && tSettings.m_bIndexExactWords )
	{
		pExact = CloneKeyword ( pNode );
		if ( !pExact->m_dWords.GetLength() )
			pExact->m_dWords.Add ( tKeyword );

		pExact->m_dWords[0].m_sWord.SetSprintf ( "=%s", tKeyword.m_sWord.cstr() );
		pExact->m_pParent = pNode;
	}

	if ( !pExact && dLemmas.GetLength()<=1 )
	{
		// zero or one lemmas, update node in-place
		if ( !pNode->m_dWords.GetLength() )
			pNode->m_dWords.Add ( tKeyword );
		if ( dLemmas.GetLength() )
		{
			pNode->m_dWords[0].m_sWord = dLemmas[0];
			pNode->m_dWords[0].m_bMorphed = true;
		}
	} else
	{
		// multiple lemmas, create an OR node
		pNode->SetOp ( SPH_QUERY_OR );
		ARRAY_FOREACH ( i, dLemmas )
		{
			pNode->m_dChildren.Add ( new XQNode_t ( pNode->m_dSpec ) );
			pNode->m_dChildren.Last()->m_pParent = pNode;
			XQKeyword_t & tLemma = pNode->m_dChildren.Last()->m_dWords.Add();
			tLemma.m_sWord = dLemmas[i];
			tLemma.m_iAtomPos = tKeyword.m_iAtomPos;
			tLemma.m_bFieldStart = tKeyword.m_bFieldStart;
			tLemma.m_bFieldEnd = tKeyword.m_bFieldEnd;
			tLemma.m_bMorphed = true;
		}
		pNode->m_dWords.Reset();
		if ( pExact )
			pNode->m_dChildren.Add ( pExact );
	}
}


/// AOT morph guesses transform
/// replaces tokens with their respective morph guesses subtrees
/// used in lemmatize_ru_all morphology processing mode that can generate multiple guesses
/// in other modes, there is always exactly one morph guess, and the dictionary handles it
/// Called from the daemon side (searchd)
void TransformAotFilter ( XQNode_t * pNode, LemmatizerTrait_i * pLemmatizer, const CSphWordforms * pWordforms, const CSphIndexSettings & tSettings )
{
	if ( !pNode )
		return;
	// case one, regular operator (and empty nodes)
	ARRAY_FOREACH ( i, pNode->m_dChildren )
		TransformAotFilter ( pNode->m_dChildren[i], pLemmatizer, pWordforms, tSettings );
	if ( pNode->m_dChildren.GetLength() || pNode->m_dWords.GetLength()==0 )
		return;

	// case two, operator on a bag of words
	// FIXME? check phrase vs expand_keywords vs lemmatize_ru_all?
	if ( pNode->m_dWords.GetLength()
		&& ( pNode->GetOp()==SPH_QUERY_PHRASE || pNode->GetOp()==SPH_QUERY_PROXIMITY || pNode->GetOp()==SPH_QUERY_QUORUM ) )
	{
		assert ( pNode->m_dWords.GetLength() );

		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			XQNode_t * pNew = new XQNode_t ( pNode->m_dSpec );
			pNew->m_pParent = pNode;
			pNew->m_iAtomPos = pNode->m_dWords[i].m_iAtomPos;
			pNode->m_dChildren.Add ( pNew );
			TransformAotFilterKeyword ( pNew, pLemmatizer, pNode->m_dWords[i], pWordforms, tSettings );
		}

		pNode->m_dWords.Reset();
		pNode->m_bVirtuallyPlain = true;
		return;
	}

	// case three, plain old single keyword
	assert ( pNode->m_dWords.GetLength()==1 );
	TransformAotFilterKeyword ( pNode, pLemmatizer, pNode->m_dWords[0], pWordforms, tSettings );
}

void TransformAotFilter ( XQNode_t * pNode, const CSphWordforms * pWordforms, const CSphIndexSettings & tSettings )
{
	if ( !tSettings.m_uAotFilterMask )
		return;

	int iAotLang = ( tSettings.m_uAotFilterMask & ( 1<<AOT_UK ) ) ? AOT_UK : AOT_LENGTH;
	CSphScopedPtr<LemmatizerTrait_i> tLemmatizer ( CreateLemmatizer ( iAotLang ) );
	TransformAotFilter ( pNode, tLemmatizer.Ptr(), pWordforms, tSettings );
}

void sphTransformExtendedQuery ( XQNode_t ** ppNode, const CSphIndexSettings & tSettings, bool bHasBooleanOptimization, const ISphKeywordsStat * pKeywords )
{
	TransformQuorum ( ppNode );
	( *ppNode )->Check ( true );
	TransformNear ( ppNode );
	( *ppNode )->Check ( true );
	if ( tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE )
		TransformBigrams ( *ppNode, tSettings );
	TagExcluded ( *ppNode, false );
	( *ppNode )->Check ( true );

	// boolean optimization
	if ( bHasBooleanOptimization )
		sphOptimizeBoolean ( ppNode, pKeywords );
}


static void SetupSplitFilter ( CSphFilterSettings & tFilter, int iPart, int iTotal )
{
	tFilter.m_eType = SPH_FILTER_RANGE;
	tFilter.m_sAttrName = "@rowid";
	tFilter.m_iMinValue = iPart;
	tFilter.m_iMaxValue = iTotal;
}

// basically the same code as QueryDiskChunks in an RT index
static bool RunSplitQuery ( const CSphIndex * pIndex, const CSphQuery & tQuery, CSphQueryResultMeta & tResult, VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, QueryProfile_c * pProfiler, const SmallStringHash_T<int64_t> * pLocalDocs, int64_t iTotalDocs, const char * szIndexName, int iSplit, int64_t tmMaxTimer )
{
	assert ( !dSorters.IsEmpty () );

	// counter of tasks we will issue now
	int iJobs = iSplit;
	assert ( iJobs>=1 );

	Threads::ClonableCtx_T<DiskChunkSearcherCtx_t, DiskChunkSearcherCloneCtx_t> tClonableCtx { dSorters, tResult };

	auto iConcurrency = tQuery.m_iCouncurrency;
	if ( !iConcurrency )
		iConcurrency = GetEffectiveDistThreads();

	tClonableCtx.LimitConcurrency ( iConcurrency );

	auto iStart = sphMicroTimer();
	sphLogDebugv ( "Started: " INT64_FMT, sphMicroTimer()-iStart );

	// because disk chunk search within the loop will switch the profiler state
	SwitchProfile ( pProfiler, SPH_QSTATE_INIT );

	std::atomic<bool> bInterrupt {false};
	std::atomic<int32_t> iCurChunk { iJobs-1 };
	Threads::CoExecuteN ( tClonableCtx.Concurrency ( iJobs ), [&]
	{
		auto iChunk = iCurChunk.fetch_sub ( 1, std::memory_order_acq_rel );
		if ( iChunk<0 || bInterrupt )
			return; // already nothing to do, early finish.

		auto tCtx = tClonableCtx.CloneNewContext();
		Threads::CoThrottler_c tThrottler ( session::ThrottlingPeriodMS() );
		int iTick=1; // num of times coro rescheduled by throttler
		while ( !bInterrupt ) // some earlier job met error; abort.
		{
			myinfo::SetThreadInfo ( "%d ch %d:", iTick, iChunk );
			auto & dLocalSorters = tCtx.m_dSorters;
			CSphQueryResultMeta tChunkMeta;
			CSphQueryResult tChunkResult;
			tChunkResult.m_pMeta = &tChunkMeta;
			CSphQueryResultMeta & tThMeta = tCtx.m_tMeta;
			tChunkMeta.m_pProfile = tThMeta.m_pProfile;

			CSphMultiQueryArgs tMultiArgs ( tArgs.m_iIndexWeight );
			tMultiArgs.m_iTag = 1;
			tMultiArgs.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;
			tMultiArgs.m_pLocalDocs = pLocalDocs;
			tMultiArgs.m_iTotalDocs = iTotalDocs;
			tMultiArgs.m_bModifySorterSchemas = false;

			CSphQuery tQueryWithExtraFilter = tQuery;
			SetupSplitFilter ( tQueryWithExtraFilter.m_dFilters.Add(), iChunk, iJobs );
			bInterrupt = !pIndex->MultiQuery ( tChunkResult, tQueryWithExtraFilter, dLocalSorters, tMultiArgs );

			// check terms inconsistency among disk chunks
			tCtx.m_tMeta.MergeWordStats ( tChunkMeta );

			if ( tThMeta.m_bHasPrediction )
				tThMeta.m_tStats.Add ( tChunkMeta.m_tStats );

			if ( iChunk && tmMaxTimer>0 && sph::TimeExceeded ( tmMaxTimer ) )
			{
				tThMeta.m_sWarning = "query time exceeded max_query_time";
				bInterrupt = true;
			}

			if ( tThMeta.m_sWarning.IsEmpty() && !tChunkMeta.m_sWarning.IsEmpty() )
				tThMeta.m_sWarning = tChunkMeta.m_sWarning;

			if ( bInterrupt && !tChunkMeta.m_sError.IsEmpty() )
				// FIXME? maybe handle this more gracefully (convert to a warning)?
				tThMeta.m_sError = tChunkMeta.m_sError;

			iChunk = iCurChunk.fetch_sub ( 1, std::memory_order_acq_rel );
			if ( iChunk<0 || bInterrupt )
				return; // all is done

			// yield and reschedule every quant of time. It gives work to other tasks
			if ( tThrottler.ThrottleAndKeepCrashQuery() )
				// report current disk chunk processing
				++iTick;
		}
	});

	tClonableCtx.Finalize();
	return !bInterrupt;
}


bool CSphIndex_VLN::SplitQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const
{
	auto & tMeta = *tResult.m_pMeta;
	QueryProfile_c * pProfile = tMeta.m_pProfile;

	CSphVector<ISphMatchSorter*> dSorters;
	dSorters.Reserve ( dAllSorters.GetLength() );
	dAllSorters.Apply ([&dSorters] ( ISphMatchSorter* p ) { if ( p ) dSorters.Add(p); });

	int64_t iTotalDocs = tArgs.m_iTotalDocs ? tArgs.m_iTotalDocs : m_tStats.m_iTotalDocuments;
	int iSplit = Max ( Min ( (int)iTotalDocs, tArgs.m_iSplit ), 1 );
	bool bOk = RunSplitQuery ( this, tQuery, *tResult.m_pMeta, dSorters, tArgs, pProfile, tArgs.m_pLocalDocs, iTotalDocs, m_sIndexName.cstr(), iSplit, tmMaxTimer );
	if ( !bOk )
		return false;

	tResult.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	tResult.m_pDocstore = m_pDocstore.Ptr() ? this : nullptr;
	tResult.m_pColumnar = m_pColumnar.Ptr();

	if ( tArgs.m_bModifySorterSchemas )
	{
		SwitchProfile ( pProfile, SPH_QSTATE_DYNAMIC );
		PooledAttrsToPtrAttrs ( dSorters, m_tBlobAttrs.GetWritePtr(), m_pColumnar.Ptr() );
	}

	return true;
}

/// one regular query vs many sorters (like facets, or similar for common-tree optimization)
bool CSphIndex_VLN::MultiQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs & tArgs ) const
{
	auto & tMeta = *tResult.m_pMeta;
	QueryProfile_c * pProfile = tMeta.m_pProfile;

	int64_t tmMaxTimer = 0;
	sph::MiniTimer_c dTimerGuard;
	if ( tQuery.m_uMaxQueryMsec>0 )
		tmMaxTimer = dTimerGuard.MiniTimerEngage ( tQuery.m_uMaxQueryMsec ); // max_query_time

	const QueryParser_i * pQueryParser = tQuery.m_pQueryParser;
	assert ( pQueryParser );

	const int SPLIT_THRESH = 1024;
	if ( tArgs.m_iSplit>1 && pQueryParser->IsFullscan(tQuery) && m_iDocinfo>SPLIT_THRESH )
		return SplitQuery ( tResult, tQuery, dAllSorters, tArgs, tmMaxTimer );

	MEMORY ( MEM_DISK_QUERY );

	// to avoid the checking of a ppSorters's element for NULL on every next step, just filter out all nulls right here
	CSphVector<ISphMatchSorter*> dSorters;
	dSorters.Reserve ( dAllSorters.GetLength() );
	dAllSorters.Apply ([&dSorters] ( ISphMatchSorter* p) { if ( p ) dSorters.Add(p); });

	// if we have anything to work with
	if ( dSorters.IsEmpty () )
		return false;

	// non-random at the start, random at the end
	dSorters.Sort ( CmpPSortersByRandom_fn() );

	// fast path for scans
	if ( pQueryParser->IsFullscan ( tQuery ) )
		return MultiScan ( tResult, tQuery, dSorters, tArgs, tmMaxTimer );

	SwitchProfile ( pProfile, SPH_QSTATE_DICT_SETUP );

	DictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };
	SetupStarDict ( pDict );
	SetupExactDict ( pDict );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (const BYTE *)tQuery.m_sQuery.cstr();

	FieldFilterRefPtr_c pFieldFilter;
	if ( m_pFieldFilter && sModifiedQuery )
	{
		pFieldFilter = m_pFieldFilter->Clone();
		if ( pFieldFilter && pFieldFilter->Apply ( sModifiedQuery, dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	// parse query
	SwitchProfile ( pProfile, SPH_QSTATE_PARSE );

	XQQuery_t tParsed;
	if ( !pQueryParser->ParseQuery ( tParsed, (const char*)sModifiedQuery, &tQuery, m_pQueryTokenizer, m_pQueryTokenizerJson, &m_tSchema, pDict, m_tSettings ) )
	{
		// FIXME? might wanna reset profile to unknown state
		tMeta.m_sError = tParsed.m_sParseError;
		return false;
	}

	// check again for fullscan
	if ( pQueryParser->IsFullscan ( tParsed ) )
		return MultiScan ( tResult, tQuery, dSorters, tArgs, tmMaxTimer );

	if ( !tParsed.m_sParseWarning.IsEmpty() )
		tMeta.m_sWarning = tParsed.m_sParseWarning;

	// transform query if needed (quorum transform, etc.)
	SwitchProfile ( pProfile, SPH_QSTATE_TRANSFORMS );
	sphTransformExtendedQuery ( &tParsed.m_pRoot, m_tSettings, tQuery.m_bSimplify, this );

	bool bWordDict = pDict->GetSettings().m_bWordDict;
	int iExpandKeywords = ExpandKeywords ( m_tMutableSettings.m_iExpandKeywords, tQuery.m_eExpandKeywords, m_tSettings, bWordDict );
	if ( iExpandKeywords!=KWE_DISABLED )
	{
		sphQueryExpandKeywords ( &tParsed.m_pRoot, m_tSettings, iExpandKeywords, bWordDict );
		tParsed.m_pRoot->Check ( true );
	}

	// this should be after keyword expansion
	TransformAotFilter ( tParsed.m_pRoot, pDict->GetWordforms(), m_tSettings );

	// expanding prefix in word dictionary case
	CSphScopedPayload tPayloads;
	XQNode_t * pPrefixed = ExpandPrefix ( tParsed.m_pRoot, tMeta, &tPayloads, tQuery.m_uDebugFlags );
	if ( !pPrefixed )
		return false;
	tParsed.m_pRoot = pPrefixed;

	auto iStackNeed = ConsiderStack ( tParsed.m_pRoot, tMeta.m_sError );
	if ( !iStackNeed  )
		return false;

	return Threads::CoContinueBool ( iStackNeed, [&] {

	// flag common subtrees
	int iCommonSubtrees = 0;
	if ( m_iMaxCachedDocs && m_iMaxCachedHits )
		iCommonSubtrees = sphMarkCommonSubtrees ( 1, &tParsed );

	tParsed.m_bNeedSZlist = tQuery.m_bZSlist;

	CSphQueryNodeCache tNodeCache ( iCommonSubtrees, m_iMaxCachedDocs, m_iMaxCachedHits );
	bool bResult = ParsedMultiQuery ( tQuery, tResult, dSorters, tParsed, pDict, tArgs, &tNodeCache, tmMaxTimer );

	if ( tArgs.m_bModifySorterSchemas )
	{
		SwitchProfile ( pProfile, SPH_QSTATE_DYNAMIC );
		PooledAttrsToPtrAttrs ( dSorters, m_tBlobAttrs.GetWritePtr(), m_pColumnar.Ptr() );
	}

	return bResult;
	});
}


/// many regular queries with one sorter attached to each query.
/// returns true if at least one query succeeded. The failed queries indicated with pResult->m_iMultiplier==-1
bool CSphIndex_VLN::MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult * pResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const
{
	// ensure we have multiple queries
	if ( iQueries==1 )
		return MultiQuery ( pResults[0], pQueries[0], { ppSorters, 1}, tArgs );

	MEMORY ( MEM_DISK_QUERYEX );

	assert ( ppSorters );

	DictRefPtr_c pDict { GetStatelessDict ( m_pDict ) };
	SetupStarDict ( pDict );
	SetupExactDict ( pDict );

	CSphFixedVector<XQQuery_t> dXQ ( iQueries );
	CSphScopedPayload tPayloads;
	bool bResult = false;
	bool bResultScan = false;
	int iStackNeed = 0;
	bool bWordDict = pDict->GetSettings().m_bWordDict;
	for ( int i=0; i<iQueries; ++i )
	{
		const auto & tCurQuery = pQueries[i];
		auto & tCurResult = pResults[i];
		auto & tMeta = *tCurResult.m_pMeta;

		// nothing to do without a sorter
		if ( !ppSorters[i] )
		{
			tMeta.m_iMultiplier = -1; ///< show that this particular query failed
			continue;
		}

		// fast path for scans
		if ( tCurQuery.m_sQuery.IsEmpty() )
		{
			int64_t tmMaxTimer = 0;
			sph::MiniTimer_c tTimerGuard;
			if ( tCurQuery.m_uMaxQueryMsec>0 )
				tmMaxTimer = tTimerGuard.MiniTimerEngage ( tCurQuery.m_uMaxQueryMsec ); // max_query_time

			if ( MultiScan ( tCurResult, tCurQuery, { ppSorters+i, 1 }, tArgs, tmMaxTimer ) )
				bResultScan = true;
			else
				tMeta.m_iMultiplier = -1; ///< show that this particular query failed
			continue;
		}

		tMeta.m_tIOStats.Start();

		// parse query
		const QueryParser_i * pQueryParser = tCurQuery.m_pQueryParser;
		assert ( pQueryParser );

		if ( pQueryParser->ParseQuery ( dXQ[i], tCurQuery.m_sQuery.cstr(), &tCurQuery, m_pQueryTokenizer, m_pQueryTokenizerJson, &m_tSchema, pDict, m_tSettings ) )
		{
			// transform query if needed (quorum transform, keyword expansion, etc.)
			sphTransformExtendedQuery ( &dXQ[i].m_pRoot, m_tSettings, tCurQuery.m_bSimplify, this );

			int iExpandKeywords = ExpandKeywords ( m_tMutableSettings.m_iExpandKeywords, tCurQuery.m_eExpandKeywords, m_tSettings, bWordDict );
			if ( iExpandKeywords!=KWE_DISABLED )
			{
				sphQueryExpandKeywords ( &dXQ[i].m_pRoot, m_tSettings, iExpandKeywords, bWordDict );
				dXQ[i].m_pRoot->Check ( true );
			}

			// this should be after keyword expansion
			TransformAotFilter ( dXQ[i].m_pRoot, pDict->GetWordforms(), m_tSettings );

			// expanding prefix in word dictionary case
			XQNode_t * pPrefixed = ExpandPrefix ( dXQ[i].m_pRoot, tMeta, &tPayloads, tCurQuery.m_uDebugFlags );
			if ( pPrefixed )
			{
				dXQ[i].m_pRoot = pPrefixed;
				iStackNeed = ConsiderStack ( dXQ[i].m_pRoot, tMeta.m_sError );
				if ( iStackNeed!=0 )
				{
					bResult = true;
				} else
				{
					tMeta.m_iMultiplier = -1;
					SafeDelete ( dXQ[i].m_pRoot );
				}
			} else
			{
				tMeta.m_iMultiplier = -1;
				SafeDelete ( dXQ[i].m_pRoot );
			}
		} else
		{
			tMeta.m_sError = dXQ[i].m_sParseError;
			tMeta.m_iMultiplier = -1;
		}
		if ( !dXQ[i].m_sParseWarning.IsEmpty() )
			tMeta.m_sWarning = dXQ[i].m_sParseWarning;

		tMeta.m_tIOStats.Stop();
	}

	// continue only if we have at least one non-failed
	if ( bResult )
		Threads::CoContinue ( iStackNeed, [&]
	{
		int iCommonSubtrees = 0;
		if ( m_iMaxCachedDocs && m_iMaxCachedHits )
			iCommonSubtrees = sphMarkCommonSubtrees ( iQueries, &dXQ[0] );

		CSphQueryNodeCache tNodeCache ( iCommonSubtrees, m_iMaxCachedDocs, m_iMaxCachedHits );
		bResult = false;
		for ( int j=0; j<iQueries; ++j )
		{
			const auto & tCurQuery = pQueries[j];
			auto & tCurResult = pResults[j];
			auto & tMeta = *tCurResult.m_pMeta;

			// fullscan case
			if ( tCurQuery.m_sQuery.IsEmpty() )
				continue;

			tMeta.m_tIOStats.Start();

			int64_t tmMaxTimer = 0;
			sph::MiniTimer_c tTimerGuard;
			if ( tCurQuery.m_uMaxQueryMsec>0 )
				tmMaxTimer = tTimerGuard.MiniTimerEngage ( tCurQuery.m_uMaxQueryMsec ); // max_query_time

			if ( dXQ[j].m_pRoot && ppSorters[j] && ParsedMultiQuery ( tCurQuery, tCurResult, { ppSorters+j, 1 }, dXQ[j], pDict, tArgs, &tNodeCache, tmMaxTimer ) )
			{
				bResult = true;
				tMeta.m_iMultiplier = iCommonSubtrees ? iQueries : 1;
			} else
				tMeta.m_iMultiplier = -1;

			tMeta.m_tIOStats.Stop();
		}
	});

	if ( tArgs.m_bModifySorterSchemas )
	{
		SwitchProfile ( pResults[0].m_pMeta->m_pProfile, SPH_QSTATE_DYNAMIC );
		PooledAttrsToPtrAttrs (  { ppSorters, iQueries }, m_tBlobAttrs.GetWritePtr(), m_pColumnar.Ptr() );
	}

	return bResult || bResultScan;
}


bool CSphIndex_VLN::CheckEarlyReject ( const CSphVector<CSphFilterSettings> & dFilters, ISphFilter * pFilter, ESphCollation eCollation, const ISphSchema & tSchema ) const
{
	if ( !pFilter || dFilters.IsEmpty() )
		return false;

	bool bHaveColumnar = false;

	if ( m_pColumnar )
	{
		// check .SPC file minmax
		CSphString sWarning;
		int iNonColumnar = 0;
		std::vector<columnar::Filter_t> dColumnarFilters;
		ARRAY_FOREACH ( i, dFilters )
		{
			const CSphColumnInfo * pCol = m_tSchema.GetAttr ( dFilters[i].m_sAttrName.cstr() );
			if ( !pCol )
				continue;

			if ( pCol->IsColumnar() || pCol->IsColumnarExpr() )
				AddColumnarFilter ( dColumnarFilters, dFilters[i], eCollation, tSchema, sWarning );
			else
				iNonColumnar++;
		}

		bHaveColumnar = !dColumnarFilters.empty();

		// can't process mixed plain and columnar attributes
		if ( iNonColumnar )
			return false;

		if ( m_pColumnar->EarlyReject ( dColumnarFilters, *pFilter ) )
			return true;
	}

	if ( bHaveColumnar )
		return false;

	if ( m_iDocinfoIndex )
	{
		// check .SPA file minmax
		DWORD uStride = m_tSchema.GetRowSize();
		DWORD * pMinEntry = const_cast<DWORD*> ( &m_pDocinfoIndex [ m_iDocinfoIndex*uStride*2 ] );
		DWORD * pMaxEntry = pMinEntry + uStride;

		if ( !pFilter->EvalBlock ( pMinEntry, pMaxEntry ) )
			return true;
	}

	return false;
}


bool CSphIndex_VLN::ParsedMultiQuery ( const CSphQuery & tQuery, CSphQueryResult & tResult, const VecTraits_T<ISphMatchSorter *> & dSorters, const XQQuery_t & tXQ, CSphDict * pDict, const CSphMultiQueryArgs & tArgs, CSphQueryNodeCache * pNodeCache, int64_t tmMaxTimer ) const
{
	assert ( !tQuery.m_sQuery.IsEmpty() && tQuery.m_eMode!=SPH_MATCH_FULLSCAN ); // scans must go through MultiScan()
	assert ( tArgs.m_iTag>=0 );

	auto& tMeta = *tResult.m_pMeta;

	// start counting
	int64_t tmQueryStart = sphMicroTimer();
	auto tmCpuQueryStart = sphTaskCpuTimer ();

	QueryProfile_c * pProfile = tMeta.m_pProfile;
	ESphQueryState eOldState = SPH_QSTATE_UNKNOWN;
	if ( pProfile )
		eOldState = pProfile->Switch ( SPH_QSTATE_INIT );

	ScopedThreadPriority_c tPrio ( tQuery.m_bLowPriority );

	///////////////////
	// setup searching
	///////////////////

	// non-ready index, empty response!
	if ( !m_bPassedAlloc )
	{
		tMeta.m_sError = "index not preread";
		return false;
	}

	// select the sorter with max schema
	int iMaxSchemaIndex = GetMaxSchemaIndexAndMatchCapacity ( dSorters ).first;
	const ISphSchema & tMaxSorterSchema = *(dSorters[iMaxSchemaIndex]->GetSchema());
	auto dSorterSchemas = SorterSchemas ( dSorters, iMaxSchemaIndex);

	// setup calculations and result schema
	CSphQueryContext tCtx ( tQuery );
	tCtx.m_pProfile = pProfile;
	tCtx.m_pLocalDocs = tArgs.m_pLocalDocs;
	tCtx.m_iTotalDocs = ( tArgs.m_iTotalDocs ? tArgs.m_iTotalDocs : m_tStats.m_iTotalDocuments );

	if ( !tCtx.SetupCalc ( tMeta, tMaxSorterSchema, m_tSchema, m_tBlobAttrs.GetWritePtr(), m_pColumnar.Ptr(), dSorterSchemas ) )
		return false;

	// set blob pool for string on_sort expression fix up
	tCtx.SetBlobPool ( m_tBlobAttrs.GetWritePtr() );
	tCtx.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;

	// open files
	DataReaderFactoryPtr_c pDoclist = m_pDoclistFile;
	DataReaderFactoryPtr_c pHitlist = m_pHitlistFile;

	if ( !pDoclist || !pHitlist )
		SwitchProfile ( pProfile, SPH_QSTATE_OPEN );

	if ( !pDoclist )
	{
		pDoclist = NewProxyReader ( GetIndexFileName ( SPH_EXT_SPD ), tMeta.m_sError, DataReaderFactory_c::DOCS, m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE );
		if ( !pDoclist )
			return false;
	}

	if ( !pHitlist )
	{
		pHitlist = NewProxyReader ( GetIndexFileName ( SPH_EXT_SPP ), tMeta.m_sError, DataReaderFactory_c::HITS, m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE );
		if ( !pHitlist )
			return false;
	}

	pDoclist->SetProfile ( pProfile );
	pHitlist->SetProfile ( pProfile );

	SwitchProfile ( pProfile, SPH_QSTATE_INIT );

	// setup search terms
	DiskIndexQwordSetup_c tTermSetup ( pDoclist, pHitlist, m_tSkiplists.GetWritePtr(), m_tSettings.m_iSkiplistBlockSize, true, RowID_t(m_iDocinfo) );

	tTermSetup.SetDict ( pDict );
	tTermSetup.m_pIndex = this;
	tTermSetup.m_iDynamicRowitems = tMaxSorterSchema.GetDynamicSize();
	tTermSetup.m_iMaxTimer = tmMaxTimer;
	tTermSetup.m_pWarning = &tMeta.m_sWarning;
	tTermSetup.m_pCtx = &tCtx;
	tTermSetup.m_pNodeCache = pNodeCache;
	tTermSetup.m_bHasWideFields = ( m_tSchema.GetFieldsCount()>32 );

	// setup prediction constrain
	CSphQueryStats tQueryStats;
	bool bCollectPredictionCounters = ( tQuery.m_iMaxPredictedMsec>0 );
	int64_t iNanoBudget = (int64_t)( tQuery.m_iMaxPredictedMsec) * 1000000; // from milliseconds to nanoseconds
	tQueryStats.m_pNanoBudget = &iNanoBudget;
	if ( bCollectPredictionCounters )
		tTermSetup.m_pStats = &tQueryStats;

	// bind weights
	tCtx.BindWeights ( tQuery, m_tSchema, tMeta.m_sWarning );

	// setup query
	// must happen before index-level reject, in order to build proper keyword stats
	CSphScopedPtr<ISphRanker> pRanker ( sphCreateRanker ( tXQ, tQuery, tMeta, tTermSetup, tCtx, tMaxSorterSchema )  );
	if ( !pRanker.Ptr() )
		return false;

	if ( ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE ) && tQuery.m_eRanker!=SPH_RANK_EXPR )
		tMeta.m_sWarning.SetSprintf ( "packedfactors() and bm25f() requires using an expression ranker" );

	tCtx.SetupExtraData ( pRanker.Ptr(), dSorters.GetLength()==1 ? dSorters[0] : nullptr );

	BYTE * pBlobPool = m_tBlobAttrs.GetWritePtr();
	pRanker->ExtraData ( EXTRA_SET_BLOBPOOL, (void**)&pBlobPool );

	int iMatchPoolSize = 0;
	dSorters.Apply ( [&iMatchPoolSize] ( const ISphMatchSorter * p ) { iMatchPoolSize += p->GetMatchCapacity(); } );

	pRanker->ExtraData ( EXTRA_SET_POOL_CAPACITY, (void**)&iMatchPoolSize );

	// check for the possible integer overflow in m_dPool.Resize
	int64_t iPoolSize = 0;
	if ( pRanker->ExtraData ( EXTRA_GET_POOL_SIZE, (void**)&iPoolSize ) && iPoolSize>INT_MAX )
	{
		tMeta.m_sError.SetSprintf ( "ranking factors pool too big (%d Mb), reduce max_matches", (int)( iPoolSize/1024/1024 ) );
		return false;
	}

	// empty index, empty response!
	if ( m_bIsEmpty )
		return true;

	// setup filters
 	CreateFilterContext_t tFlx;
	tFlx.m_pFilters = &tQuery.m_dFilters;
	tFlx.m_pFilterTree = &tQuery.m_dFilterTree;
	tFlx.m_pSchema = &tMaxSorterSchema;
	tFlx.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	tFlx.m_pColumnar = m_pColumnar.Ptr();
	tFlx.m_eCollation = tQuery.m_eCollation;
	tFlx.m_bScan = tQuery.m_sQuery.IsEmpty ();
	tFlx.m_pHistograms = m_pHistograms;

	if ( !tCtx.CreateFilters ( tFlx, tMeta.m_sError, tMeta.m_sWarning ) )
		return false;

	if ( CheckEarlyReject ( tQuery.m_dFilters, tCtx.m_pFilter, tQuery.m_eCollation, tMaxSorterSchema ) )
	{
		tMeta.m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
		tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;
		return true;
	}

	for ( auto & i : dSorters )
	{
		i->SetBlobPool ( m_tBlobAttrs.GetWritePtr() );
		i->SetColumnar ( m_pColumnar.Ptr() );
	}

	bool bHaveRandom = false;
	dSorters.Apply ( [&bHaveRandom] ( const ISphMatchSorter * p ) { bHaveRandom |= p->IsRandom(); } );

	bool bUseFactors = !!( tCtx.m_uPackedFactorFlags & SPH_FACTOR_ENABLE );
	bool bHaveDead = m_tDeadRowMap.HasDead();

	//////////////////////////////////////
	// find and weight matching documents
	//////////////////////////////////////

	bool bFinalPass = !!tCtx.m_dCalcFinal.GetLength();
	int iMyTag = bFinalPass ? -1 : tArgs.m_iTag;

	// a shortcut to avoid listing all args every time
	void (CSphIndex_VLN::*pFunc)(CSphQueryContext &, const CSphQuery &, const VecTraits_T<ISphMatchSorter*>&, ISphRanker *, int, int) const = nullptr;

	switch ( tQuery.m_eMode )
	{
		case SPH_MATCH_ALL:
		case SPH_MATCH_PHRASE:
		case SPH_MATCH_ANY:
		case SPH_MATCH_EXTENDED:
		case SPH_MATCH_EXTENDED2:
		case SPH_MATCH_BOOLEAN:
			if ( bHaveDead )
			{
				if ( bHaveRandom )
				{
					if ( bUseFactors )
						pFunc = &CSphIndex_VLN::MatchExtended<true, true, true>;
					else
						pFunc = &CSphIndex_VLN::MatchExtended<true, true, false>;
				}
				else
				{
					if ( bUseFactors )
						pFunc = &CSphIndex_VLN::MatchExtended<true, false, true>;
					else
						pFunc = &CSphIndex_VLN::MatchExtended<true, false, false>;
				}
			}
			else
			{
				if ( bHaveRandom )
				{
					if ( bUseFactors )
						pFunc = &CSphIndex_VLN::MatchExtended<false, true, true>;
					else
						pFunc = &CSphIndex_VLN::MatchExtended<false, true, false>;
				}
				else
				{
					if ( bUseFactors )
						pFunc = &CSphIndex_VLN::MatchExtended<false, false, true>;
					else
						pFunc = &CSphIndex_VLN::MatchExtended<false, false, false>;
				}
			}

			(*this.*pFunc)( tCtx, tQuery, dSorters, pRanker.Ptr(), iMyTag, tArgs.m_iIndexWeight );
			break;

		default:
			sphDie ( "INTERNAL ERROR: unknown matching mode (mode=%d)", tQuery.m_eMode );
	}

	////////////////////
	// cook result sets
	////////////////////

	SwitchProfile ( pProfile, SPH_QSTATE_FINALIZE );

	// adjust result sets
	if ( bFinalPass )
	{
		// GotUDF means promise to UDFs that final-stage calls will be evaluated
		// a) over the final, pre-limit result set
		// b) in the final result set order
		bool bGotUDF = false;
		ARRAY_FOREACH_COND ( i, tCtx.m_dCalcFinal, !bGotUDF )
			tCtx.m_dCalcFinal[i].m_pExpr->Command ( SPH_EXPR_GET_UDF, &bGotUDF );

		SphFinalMatchCalc_t tFinal ( tArgs.m_iTag, tCtx );
		dSorters.Apply ( [&] ( ISphMatchSorter * p ) { p->Finalize ( tFinal, bGotUDF ); } );
	}

	pRanker->FinalizeCache ( tMaxSorterSchema );

	tResult.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	tResult.m_pDocstore = m_pDocstore.Ptr() ? this : nullptr;
	tResult.m_pColumnar = m_pColumnar.Ptr();

	// query timer
	int64_t tmWall = sphMicroTimer() - tmQueryStart;
	tMeta.m_iQueryTime += (int)( tmWall/1000 );
	tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;

#if 0
	printf ( "qtm %d, %d, %d, %d, %d\n", int(tmWall), tQueryStats.m_iFetchedDocs,
		tQueryStats.m_iFetchedHits, tQueryStats.m_iSkips, dSorters[0]->GetTotalCount() );
#endif

	SwitchProfile ( pProfile, eOldState );

	if ( bCollectPredictionCounters )
	{
		tMeta.m_tStats.Add ( tQueryStats );
		tMeta.m_bHasPrediction = true;
	}

	return true;
}


void CSphIndex_VLN::CreateReader ( int64_t iSessionId ) const
{
	if ( !m_pDocstore.Ptr() )
		return;

	m_pDocstore->CreateReader(iSessionId);
}


bool CSphIndex_VLN::GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const
{
	if ( !m_pDocstore.Ptr() )
		return false;

	RowID_t tRowID = GetRowidByDocid ( tDocID );
	if ( tRowID==INVALID_ROWID || m_tDeadRowMap.IsSet(tRowID) )
		return false;

	tDoc = m_pDocstore->GetDoc ( tRowID, pFieldIds, iSessionId, bPack );
	return true;
}


int CSphIndex_VLN::GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const
{
	if ( !m_pDocstore.Ptr() )
		return -1;

	return m_pDocstore->GetFieldId ( sName, eType );
}


//////////////////////////////////////////////////////////////////////////
// INDEX STATUS
//////////////////////////////////////////////////////////////////////////

void CSphIndex_VLN::GetStatus ( CSphIndexStatus* pRes ) const
{
	assert ( pRes );
	if ( !pRes )
		return;

	pRes->m_iMapped = m_tAttr.GetLengthBytes ()
			+m_tBlobAttrs.GetLengthBytes ()
			+m_tWordlist.m_tBuf.GetLengthBytes ()
			+m_tDeadRowMap.GetLengthBytes ()
			+m_tSkiplists.GetLengthBytes ();

	pRes->m_iMappedResident = m_tAttr.GetCoreSize ()
			+m_tBlobAttrs.GetCoreSize ()
			+m_tWordlist.m_tBuf.GetCoreSize ()
			+m_tDeadRowMap.GetCoreSize ()
			+m_tSkiplists.GetCoreSize ();

	pRes->m_iDead = m_tDeadRowMap.GetNumDeads();

	if ( m_pDoclistFile )
	{
		pRes->m_iMappedDocs = m_pDoclistFile->GetMappedsize ();
		pRes->m_iMappedResidentDocs = m_pDoclistFile->GetCoresize ();
		pRes->m_iMapped += pRes->m_iMappedDocs;
		pRes->m_iMappedResident += pRes->m_iMappedResidentDocs;
	}

	if ( m_pHitlistFile )
	{
		pRes->m_iMappedHits = m_pHitlistFile->GetMappedsize ();
		pRes->m_iMappedResidentHits = m_pHitlistFile->GetCoresize ();
		pRes->m_iMapped += pRes->m_iMappedHits;
		pRes->m_iMappedResident += pRes->m_iMappedResidentHits;
	}

	pRes->m_iRamUse = sizeof(CSphIndex_VLN) + m_dFieldLens.GetLengthBytes() + pRes->m_iMappedResident;
	pRes->m_iDiskUse = 0;

	CSphVector<IndexFileExt_t> dExts = sphGetExts();
	for ( const auto & i : dExts )
		if ( i.m_eExt!=SPH_EXT_SPL )
		{
			CSphString sFile;
			sFile.SetSprintf ( "%s%s", m_sFilename.cstr(), i.m_szExt );
			struct_stat st;
			if ( stat ( sFile.cstr(), &st )==0 )
				pRes->m_iDiskUse += st.st_size;
		}
//	sphWarning ( "Chunks: %d, RAM: %d, DISK: %d", pRes->m_iNumChunks, (int) pRes->m_iRamUse, (int) pRes->m_iMapped );
}

//////////////////////////////////////////////////////////////////////////
// SHA1 digests
//////////////////////////////////////////////////////////////////////////

// SHA1 from https://github.com/shodanium/nanomysql/blob/master/nanomysql.cpp

// nanomysql, a tiny MySQL client
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/

void SHA1_c::Transform ( const BYTE buf[64] )
{
	DWORD a = state[0], b = state[1], c = state[2], d = state[3], e = state[4], block[16];
	memset ( block, 0, sizeof ( block ) ); // initial conversion to big-endian units
	for ( int i = 0; i<64; i++ )
		block[i >> 2] += buf[i] << ( ( 3 - ( i & 3 ) ) * 8 );
	for ( int i = 0; i<80; i++ ) // do hashing rounds
	{
#define _LROT( value, bits ) ( ( (value) << (bits) ) | ( (value) >> ( 32-(bits) ) ) )
		if ( i>=16 )
			block[i & 15] = _LROT (
				block[( i + 13 ) & 15] ^ block[( i + 8 ) & 15] ^ block[( i + 2 ) & 15] ^ block[i & 15], 1 );

		if ( i<20 )
			e += ( ( b & ( c ^ d ) ) ^ d ) + 0x5A827999;
		else if ( i<40 )
			e += ( b ^ c ^ d ) + 0x6ED9EBA1;
		else if ( i<60 )
			e += ( ( ( b | c ) & d ) | ( b & c ) ) + 0x8F1BBCDC;
		else
			e += ( b ^ c ^ d ) + 0xCA62C1D6;

		e += block[i & 15] + _LROT ( a, 5 );
		DWORD t = e;
		e = d;
		d = c;
		c = _LROT ( b, 30 );
		b = a;
		a = t;
	}
	state[0] += a; // save state
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
}

SHA1_c & SHA1_c::Init()
{
	const DWORD dInit[5] = { 0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0 };
	memcpy ( state, dInit, sizeof ( state ) );
	count[0] = count[1] = 0;
	return *this;
}

SHA1_c & SHA1_c::Update ( const BYTE * data, int len )
{
	int i, j = ( count[0] >> 3 ) & 63;
	count[0] += ( len << 3 );
	if ( count[0]<( DWORD ) ( len << 3 ) )
		count[1]++;
	count[1] += ( len >> 29 );
	if ( ( j + len )>63 )
	{
		i = 64 - j;
		memcpy ( &buffer[j], data, i );
		Transform ( buffer );
		for ( ; i + 63<len; i += 64 )
			Transform ( data + i );
		j = 0;
	} else
		i = 0;
	memcpy ( &buffer[j], &data[i], len - i );
	return *this;
}

void SHA1_c::Final ( BYTE digest[SHA1_SIZE] )
{
	BYTE finalcount[8];
	for ( auto i = 0; i<8; i++ )
		finalcount[i] = ( BYTE ) ( ( count[( i>=4 ) ? 0 : 1] >> ( ( 3 - ( i & 3 ) ) * 8 ) )
			& 255 ); // endian independent
	Update ( (const BYTE *) "\200", 1 ); // add padding
	while ( ( count[0] & 504 )!=448 )
		Update ( (const BYTE *) "\0", 1 );
	Update ( finalcount, 8 ); // should cause a SHA1_Transform()
	for ( auto i = 0; i<SHA1_SIZE; i++ )
		digest[i] = ( BYTE ) ( ( state[i >> 2] >> ( ( 3 - ( i & 3 ) ) * 8 ) ) & 255 );
}


CSphString BinToHex ( const BYTE * pHash, int iLen )
{
	const char * sDigits = "0123456789abcdef";
	if ( !iLen )
		return "";

	CSphString sRes;
	int iStrLen = 2*iLen+2;
	sRes.Reserve ( iStrLen );
	char * sHash = const_cast<char *> (sRes.cstr ());

	for ( int i=0; i<iLen; i++ )
	{
		*sHash++ = sDigits[pHash[i] >> 4];
		*sHash++ = sDigits[pHash[i] & 0x0f];
	}
	*sHash = '\0';
	return sRes;
}

CSphString BinToHex ( const VecTraits_T<BYTE> & dHash )
{
	return BinToHex ( dHash.Begin(), dHash.GetLength() );
}

CSphString CalcSHA1 ( const void * pData, int iLen )
{
	CSphFixedVector<BYTE> dHashValue ( HASH20_SIZE );
	SHA1_c dHasher;
	dHasher.Init();
	dHasher.Update ( (const BYTE*)pData, iLen );
	dHasher.Final ( dHashValue.begin() );
	return BinToHex ( dHashValue );
}

bool CalcSHA1 ( const CSphString & sFileName, CSphString & sRes, CSphString & sError )
{
	CSphAutofile tFile ( sFileName, SPH_O_READ, sError, false );
	if ( tFile.GetFD()<0 )
		return false;

	CSphFixedVector<BYTE> dHashValue ( HASH20_SIZE );
	SHA1_c dHasher;
	dHasher.Init();

	const int64_t iFileSize = tFile.GetSize();
	const int iBufSize = (int)Min ( iFileSize, DEFAULT_READ_BUFFER );
	int64_t iOff = 0;
	CSphFixedVector<BYTE> dFileData ( iBufSize );
	while ( iOff<iFileSize )
	{
		const int iLen = (int)Min ( iBufSize, iFileSize - iOff );
		if ( !tFile.Read ( dFileData.Begin(), iLen, sError ) )
			return false;

		dHasher.Update ( dFileData.Begin(), iLen );
		iOff += iLen;
	}

	dHasher.Final ( dHashValue.Begin() );
	sRes = BinToHex ( dHashValue );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// WriterWithHash_c - CSphWriter which also calc SHA1 on-the-fly
//////////////////////////////////////////////////////////////////////////

WriterWithHash_c::WriterWithHash_c ( const char * sExt, HashCollection_c * pCollector)
	: m_pCollection { pCollector }
	, m_sExt { sExt }
{
	m_pHasher = new SHA1_c;
	m_pHasher->Init();
}

WriterWithHash_c::~WriterWithHash_c ()
{
	SafeDelete ( m_pHasher );
}

void WriterWithHash_c::Flush()
{
	assert ( !m_bHashDone ); // can't do anything with already finished hash
	if ( m_iPoolUsed>0 )
	{
		m_pHasher->Update ( m_pBuffer, m_iPoolUsed );
		CSphWriter::Flush();
	}
}

const BYTE * WriterWithHash_c::GetHASHBlob () const
{
	assert ( m_bHashDone );
	return m_dHashValue;
}

void WriterWithHash_c::CloseFile ()
{
	assert ( !m_bHashDone );
	CSphWriter::CloseFile ();
	m_pHasher->Final ( m_dHashValue );
	if ( m_pCollection )
		m_pCollection->AppendNewHash ( m_sExt, m_dHashValue );
	m_bHashDone = true;
}


//////////////////////////////////////////////////////////////////////////
// TaggedHash20_t - string tag (filename) with 20-bytes binary hash
//////////////////////////////////////////////////////////////////////////

const BYTE TaggedHash20_t::m_dZeroHash[HASH20_SIZE] = { 0 };

// by tag + hash
TaggedHash20_t::TaggedHash20_t ( const char* sTagName, const BYTE* pHashValue )
	: m_sTagName ( sTagName )
{
	if ( !pHashValue )
		pHashValue = m_dZeroHash;
	memcpy ( m_dHashValue, pHashValue, HASH20_SIZE );
}

// serialize to FIPS form
CSphString TaggedHash20_t::ToFIPS () const
{
	const char * sDigits = "0123456789abcdef";
	char sHash [41];
	StringBuilder_c sResult;

	if ( Empty() )
		return "";

	for ( auto i = 0; i<HASH20_SIZE; ++i )
	{
		sHash[i << 1] = sDigits[m_dHashValue[i] >> 4];
		sHash[1 + (i << 1)] = sDigits[m_dHashValue[i] & 0x0f];
	}
	sHash[40] = '\0';

	// FIPS-180-1 - checksum, space, "*" (indicator of binary mode), tag
	sResult.Appendf ("%s *%s\n", sHash, m_sTagName.cstr());
	return sResult.cstr();
}

inline BYTE hex_char ( unsigned char c )
{
	if ( c>=0x30 && c<=0x39 )
		return c - '0';
	else if ( c>=0x61 && c<=0x66 )
		return c - 'a' + 10;
	assert ( false && "broken hex num - expected digits and a..f letters in the num" );
	return 0;
}

// de-serialize from FIPS, returns len of parsed chunk of sFIPS or -1 on error
int TaggedHash20_t::FromFIPS ( const char * sFIPS )
{
	// expects hash in form FIPS-180-1, that is:
	// 45f44fd2db02b08b4189abf21e90edd712c9616d *rt_full.ram\n
	// i.e. 40 symbols hex hash in small letters, space, '*' and tag, finished by '\n'

	assert ( sFIPS[HASH20_SIZE * 2]==' ' && "broken FIPS - space expected after hash" );
	assert ( sFIPS[HASH20_SIZE * 2 + 1]=='*' && "broken FIPS - * expected after hash and space" );

	for ( auto i = 0; i<HASH20_SIZE; ++i )
	{
		BYTE & uCode = m_dHashValue[i];
		uCode = hex_char ( sFIPS[i * 2] );
		uCode = BYTE ( ( uCode << 4 ) + hex_char ( sFIPS[i * 2 + 1] ) );
	}

	sFIPS += 2 + HASH20_SIZE * 2;
	auto len = (int) strlen ( sFIPS );

	if ( sFIPS[len - 1]!='\n' )
		return -1;

	m_sTagName.SetBinary ( sFIPS, len - 1 );
	return len;
}

bool TaggedHash20_t::operator== ( const BYTE * pRef ) const
{
	assert ( pRef );
	return !memcmp ( m_dHashValue, pRef, HASH20_SIZE );
}

//////////////////////////////////////////////////////////////////////////
// HashCollection_c
//////////////////////////////////////////////////////////////////////////

void HashCollection_c::AppendNewHash ( const char * sExt, const BYTE * pHash )
{
	m_dHashes.Add ( TaggedHash20_t ( sExt, pHash ) );
}

//////////////////////////////////////////////////////////////////////////
// INDEX CHECKING
//////////////////////////////////////////////////////////////////////////

void CSphIndex_VLN::SetDebugCheck ( bool bCheckIdDups, int )
{
	m_bDebugCheck = true;
	m_bCheckIdDups = bCheckIdDups;
}


// no strnlen on some OSes (Mac OS)
#if !HAVE_STRNLEN
size_t strnlen ( const char * s, size_t iMaxLen )
{
	if ( !s )
		return 0;

	size_t iRes = 0;
	while ( *s++ && iRes<iMaxLen )
		++iRes;
	return iRes;
}
#endif


int CSphIndex_VLN::DebugCheck ( FILE * fp )
{
	DebugCheckError_c tReporter(fp);
	CSphScopedPtr<DiskIndexChecker_i> pIndexChecker ( CreateDiskIndexChecker ( *this, tReporter ) );

	pIndexChecker->Setup ( m_iDocinfo, m_iDocinfoIndex, m_iMinMaxIndex, m_bCheckIdDups );

	// check if index is ready
	if ( !m_bPassedAlloc )
		tReporter.Fail ( "index not preread" );

	CSphString sError;
	if ( !pIndexChecker->OpenFiles(sError) )
		return 1;

	if ( !LoadHitlessWords ( m_tSettings.m_sHitlessFiles, m_pTokenizer, m_pDict, pIndexChecker->GetHitlessWords(), m_sLastError ) )
		tReporter.Fail ( "unable to load hitless words: %s", m_sLastError.cstr() );

	CSphSavedFile tStat;
	const CSphTokenizerSettings & tTokenizerSettings = m_pTokenizer->GetSettings ();
	if ( !tTokenizerSettings.m_sSynonymsFile.IsEmpty() && !tStat.Collect ( tTokenizerSettings.m_sSynonymsFile.cstr(), &sError ) )
		tReporter.Fail ( "unable to open exceptions '%s': %s", tTokenizerSettings.m_sSynonymsFile.cstr(), sError.cstr() );

	const CSphDictSettings & tDictSettings = m_pDict->GetSettings ();
	const char * pStop = tDictSettings.m_sStopwords.cstr();
	while (true)
	{
		// find next name start
		while ( pStop && *pStop && isspace(*pStop) ) pStop++;
		if ( !pStop || !*pStop ) break;

		const char * sNameStart = pStop;

		// find next name end
		while ( *pStop && !isspace(*pStop) ) pStop++;

		CSphString sStopFile;
		sStopFile.SetBinary ( sNameStart, int ( pStop-sNameStart ) );

		if ( !tStat.Collect ( sStopFile.cstr(), &sError ) )
			tReporter.Fail ( "unable to open stopwords '%s': %s", sStopFile.cstr(), sError.cstr() );
	}

	if ( !tDictSettings.m_dWordforms.GetLength() )
	{
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
		{
			if ( !tStat.Collect ( tDictSettings.m_dWordforms[i].cstr(), &sError ) )
				tReporter.Fail ( "unable to open wordforms '%s': %s", tDictSettings.m_dWordforms[i].cstr(), sError.cstr() );
		}
	}

	pIndexChecker->Check();

	tReporter.Done();

	return (int)Min ( tReporter.GetNumFails(), 255 ); // this is the exitcode; so cap it
} // NOLINT function length


static void AddFields ( const char * sQuery, CSphSchema & tSchema )
{
	CSphColumnInfo tField;

	const char * sToken = sQuery;
	if ( !sToken )
	{
		tField.m_sName = "dummy_field"; // for query with only all fields, @*
		tSchema.AddField ( tField );
		return;
	}

	const char * OPTION_RELAXED = "@@relaxed";
	const auto OPTION_RELAXED_LEN = (int) strlen ( OPTION_RELAXED );
	if ( strncmp ( sToken, OPTION_RELAXED, OPTION_RELAXED_LEN )==0 && !sphIsAlpha ( sToken[OPTION_RELAXED_LEN] ) )
		sToken += OPTION_RELAXED_LEN;

	while ( *sToken )
	{
		if ( *sToken!='@' )
		{
			sToken++;
			continue;
		}

		sToken++;
		if ( !*sToken )
			break;
		if ( *sToken=='!' || *sToken=='*' )
			sToken++;
		if ( !*sToken )
			break;
		bool bBlock = ( *sToken=='(' );
		if ( bBlock )
			sToken++;
		if ( !*sToken )
			break;

		// handle block with field names
		while ( *sToken )
		{
			const char * sField = sToken;
			while ( *sToken && sphIsAlpha( *sToken ) )
				sToken++;

			int iLen = int ( sToken - sField );
			if ( iLen )
			{
				tField.m_sName.SetBinary ( sField, iLen );
				if ( !tSchema.GetField ( tField.m_sName.cstr() ) )
					tSchema.AddField ( tField );
			}

			if ( !bBlock )
				break;

			if ( *sToken && *sToken==',' )
				sToken++;

			if ( *sToken && *sToken==')' )
				break;
		}
	}

	if ( !tSchema.GetFieldsCount() )
	{
		tField.m_sName = "dummy_field"; // for query with only all fields, @*
		tSchema.AddField ( tField );
	}
}

Bson_t EmptyBson ()
{
	Bson_t dEmpty;
	return dEmpty;
}

Bson_t Explain ( ExplainQueryArgs_t & tArgs )
{
	if ( tArgs.m_sQuery.IsEmpty() )
		return EmptyBson ();

	CSphScopedPtr<QueryParser_i> pQueryParser ( sphCreatePlainQueryParser() );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (const BYTE *)tArgs.m_sQuery.cstr();

	if ( tArgs.m_pFieldFilter && sModifiedQuery )
	{
		if ( tArgs.m_pFieldFilter->Apply ( sModifiedQuery, dFiltered, true ) )
			sModifiedQuery = dFiltered.Begin();
	}

	CSphSchema tSchema;
	const CSphSchema * pSchema = tArgs.m_pSchema;
	if ( !pSchema )
	{
		pSchema = &tSchema;
		// need to fill up schema with fields from query
		AddFields ( tArgs.m_sQuery.cstr(), tSchema );
	}

	XQQuery_t tParsed;
	if ( !pQueryParser->ParseQuery ( tParsed, (const char*)sModifiedQuery, nullptr, tArgs.m_pQueryTokenizer, nullptr, pSchema, tArgs.m_pDict, *tArgs.m_pSettings ) )
	{
		TlsMsg::Err ( tParsed.m_sParseError );
		return EmptyBson ();
	}

	sphTransformExtendedQuery ( &tParsed.m_pRoot, *tArgs.m_pSettings, false, nullptr );

	bool bWordDict = tArgs.m_pDict->GetSettings().m_bWordDict;
	int iExpandKeywords = ExpandKeywords ( tArgs.m_iExpandKeywords, QUERY_OPT_DEFAULT, *tArgs.m_pSettings, bWordDict );
	if ( iExpandKeywords!=KWE_DISABLED )
	{
		sphQueryExpandKeywords ( &tParsed.m_pRoot, *tArgs.m_pSettings, iExpandKeywords, bWordDict );
		tParsed.m_pRoot->Check ( true );
	}

	// this should be after keyword expansion
	TransformAotFilter ( tParsed.m_pRoot, tArgs.m_pDict->GetWordforms(), *tArgs.m_pSettings );

	//// expanding prefix in word dictionary case
	if ( tArgs.m_bExpandPrefix )
	{
		CSphQueryResultMeta tMeta;
		CSphScopedPayload tPayloads;

		ExpansionContext_t tExpCtx;
		tExpCtx.m_pWordlist = tArgs.m_pWordlist;
		tExpCtx.m_pResult = &tMeta;
		tExpCtx.m_iMinPrefixLen = tArgs.m_pSettings->GetMinPrefixLen ( tArgs.m_pDict->GetSettings().m_bWordDict );
		tExpCtx.m_iMinInfixLen = tArgs.m_pSettings->m_iMinInfixLen;
		tExpCtx.m_iExpansionLimit = tArgs.m_iExpansionLimit;
		tExpCtx.m_bHasExactForms = ( tArgs.m_pDict->HasMorphology() || tArgs.m_pSettings->m_bIndexExactWords );
		tExpCtx.m_bMergeSingles = false;
		tExpCtx.m_pPayloads = &tPayloads;
		tExpCtx.m_pIndexData = tArgs.m_pIndexData;

		tParsed.m_pRoot = sphExpandXQNode ( tParsed.m_pRoot, tExpCtx );
	}

	return sphExplainQuery ( tParsed.m_pRoot, *pSchema, tParsed.m_dZones );
}

Bson_t CSphIndex_VLN::ExplainQuery ( const CSphString & sQuery ) const
{
	ExplainQueryArgs_t tArgs ( sQuery );
	tArgs.m_pSchema = &GetMatchSchema();
	tArgs.m_pDict = GetStatelessDict ( m_pDict );
	SetupStarDict ( tArgs.m_pDict );
	SetupExactDict ( tArgs.m_pDict );
	if ( m_pFieldFilter )
		tArgs.m_pFieldFilter = m_pFieldFilter->Clone();
	tArgs.m_pSettings = &m_tSettings;
	tArgs.m_pWordlist = &m_tWordlist;
	tArgs.m_pQueryTokenizer = m_pQueryTokenizer;
	tArgs.m_iExpandKeywords = m_tMutableSettings.m_iExpandKeywords;
	tArgs.m_iExpansionLimit = m_iExpansionLimit;
	tArgs.m_bExpandPrefix = ( m_pDict->GetSettings().m_bWordDict && IsStarDict ( m_pDict->GetSettings().m_bWordDict ) );

	return Explain ( tArgs );
}


bool CSphIndex_VLN::CopyExternalFiles ( int iPostfix, StrVec_t & dCopied )
{
	CSphVector<std::pair<CSphString,CSphString>> dExtFiles;
	if ( m_pTokenizer && !m_pTokenizer->GetSettings().m_sSynonymsFile.IsEmpty() )
	{
		CSphString sRenameTo;
		sRenameTo.SetSprintf ( "exceptions_chunk%d.txt", iPostfix );
		dExtFiles.Add ( { m_pTokenizer->GetSettings().m_sSynonymsFile, sRenameTo } );
		const_cast<CSphTokenizerSettings &>(m_pTokenizer->GetSettings()).m_sSynonymsFile = sRenameTo;
	}

	if ( m_pDict )
	{
		const CSphString & sStopwords = m_pDict->GetSettings().m_sStopwords;
		if ( !sStopwords.IsEmpty() )
		{
			StringBuilder_c sNewStopwords(" ");
			StrVec_t dStops = sphSplit ( sStopwords.cstr(), sStopwords.Length(), " \t," );
			ARRAY_FOREACH ( i, dStops )
			{
				CSphString sTmp;
				sTmp.SetSprintf ( "stopwords_chunk%d_%d.txt", iPostfix, i );
				dExtFiles.Add ( { dStops[i], sTmp } );

				sNewStopwords << sTmp;
			}

			const_cast<CSphDictSettings &>(m_pDict->GetSettings()).m_sStopwords = sNewStopwords.cstr();
		}

		StrVec_t dNewWordforms;
		ARRAY_FOREACH ( i, m_pDict->GetSettings().m_dWordforms )
		{
			CSphString sTmp;
			sTmp.SetSprintf ( "wordforms_chunk%d_%d.txt", iPostfix, i );
			dExtFiles.Add( { m_pDict->GetSettings().m_dWordforms[i], sTmp } );
			dNewWordforms.Add(sTmp);
		}

		const_cast<CSphDictSettings &>(m_pDict->GetSettings()).m_dWordforms = dNewWordforms;
	}

	CSphString sPathOnly = GetPathOnly(m_sFilename);
	for ( const auto & i : dExtFiles )
	{
		CSphString sDest;
		sDest.SetSprintf ( "%s%s", sPathOnly.cstr(), i.second.cstr() );
		if ( !CopyFile ( i.first, sDest, m_sLastError ) )
			return false;

		dCopied.Add(sDest);
	}

	BuildHeader_t tBuildHeader(m_tStats);
	*(DictHeader_t*)&tBuildHeader = *(DictHeader_t*)&m_tWordlist;
	tBuildHeader.m_iDocinfo = m_iDocinfo;
	tBuildHeader.m_iDocinfoIndex = m_iDocinfoIndex;
	tBuildHeader.m_iMinMaxIndex = m_iMinMaxIndex;

	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &m_tSettings;
	tWriteHeader.m_pSchema = &m_tSchema;
	tWriteHeader.m_pTokenizer = m_pTokenizer;
	tWriteHeader.m_pDict = m_pDict;
	tWriteHeader.m_pFieldFilter = m_pFieldFilter;
	tWriteHeader.m_pFieldLens = m_dFieldLens.Begin();

	// save the header
	return IndexBuildDone ( tBuildHeader, tWriteHeader, GetIndexFileName(SPH_EXT_SPH), m_sLastError );
}

void CSphIndex_VLN::CollectFiles ( StrVec_t & dFiles, StrVec_t & dExt ) const
{
	if ( m_pTokenizer && !m_pTokenizer->GetSettings().m_sSynonymsFile.IsEmpty() )
		dExt.Add ( m_pTokenizer->GetSettings ().m_sSynonymsFile );

	if ( !m_pDict )
		return;

	const CSphString & sStopwords = m_pDict->GetSettings().m_sStopwords;
	if ( !sStopwords.IsEmpty() )
		sphSplit ( dExt, sStopwords.cstr(), sStopwords.Length(), " \t," );

	m_pDict->GetSettings ().m_dWordforms.Apply ( [&dExt] ( const CSphString & a ) { dExt.Add ( a ); } );

	dFiles.Add ( GetIndexFileName ( SPH_EXT_SPH ) );
	dFiles.Add ( GetIndexFileName ( SPH_EXT_SPD ) );
	dFiles.Add ( GetIndexFileName ( SPH_EXT_SPP ) );
	dFiles.Add ( GetIndexFileName ( SPH_EXT_SPE ) );
	dFiles.Add ( GetIndexFileName ( SPH_EXT_SPI ) );
	dFiles.Add ( GetIndexFileName ( SPH_EXT_SPM ) );
	if ( !m_bIsEmpty )
	{
		if ( m_tSchema.HasNonColumnarAttrs() )
			dFiles.Add ( GetIndexFileName ( SPH_EXT_SPA ) );

		dFiles.Add ( GetIndexFileName ( SPH_EXT_SPT ) );
		if ( m_tSchema.GetAttr ( sphGetBlobLocatorName () ) )
			dFiles.Add ( GetIndexFileName ( SPH_EXT_SPB ) );

		if ( m_uVersion>=63 && m_tSchema.HasColumnarAttrs() )
			dFiles.Add ( GetIndexFileName ( SPH_EXT_SPC ) );
	}

	if ( m_uVersion>=55 )
		dFiles.Add ( GetIndexFileName ( SPH_EXT_SPHI ) );

	if ( m_uVersion>=57 && m_tSchema.HasStoredFields() )
		dFiles.Add ( GetIndexFileName ( SPH_EXT_SPDS ) );

	CSphString sPath = GetIndexFileName ( SPH_EXT_SPK );
	if ( sphIsReadable ( sPath ) )
		dFiles.Add ( sPath );
}

//////////////////////////////////////////////////////////////////////////

/// morphology
enum
{
	SPH_MORPH_STEM_EN,
	SPH_MORPH_STEM_RU_UTF8,
	SPH_MORPH_STEM_CZ,
	SPH_MORPH_STEM_AR_UTF8,
	SPH_MORPH_SOUNDEX,
	SPH_MORPH_METAPHONE_UTF8,
	SPH_MORPH_AOTLEMMER_BASE,
	SPH_MORPH_AOTLEMMER_RU_UTF8 = SPH_MORPH_AOTLEMMER_BASE,
	SPH_MORPH_AOTLEMMER_EN,
	SPH_MORPH_AOTLEMMER_DE_UTF8,
	SPH_MORPH_AOTLEMMER_UK,
	SPH_MORPH_AOTLEMMER_BASE_ALL,
	SPH_MORPH_AOTLEMMER_RU_ALL = SPH_MORPH_AOTLEMMER_BASE_ALL,
	SPH_MORPH_AOTLEMMER_EN_ALL,
	SPH_MORPH_AOTLEMMER_DE_ALL,
	SPH_MORPH_AOTLEMMER_UK_ALL,
	SPH_MORPH_LIBSTEMMER_FIRST,
	SPH_MORPH_LIBSTEMMER_LAST = SPH_MORPH_LIBSTEMMER_FIRST + 64
};


/////////////////////////////////////////////////////////////////////////////
// BASE DICTIONARY INTERFACE
/////////////////////////////////////////////////////////////////////////////

void CSphDict::DictBegin ( CSphAutofile &, CSphAutofile &, int )						{}
void CSphDict::DictEntry ( const CSphDictEntry & )										{}
void CSphDict::DictEndEntries ( SphOffset_t )											{}
bool CSphDict::DictEnd ( DictHeader_t *, int, CSphString & )							{ return true; }
bool CSphDict::DictIsError () const														{ return true; }

/////////////////////////////////////////////////////////////////////////////
// CRC32/64 DICTIONARIES
/////////////////////////////////////////////////////////////////////////////

struct CSphTemplateDictTraits : DictStub_c
{
	CSphTemplateDictTraits ();
protected:
				~CSphTemplateDictTraits () override;

public:
	void		LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile ) final;
	void		LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) final;
	void		WriteStopwords ( CSphWriter & tWriter ) const final;
	void		WriteStopwords ( JsonEscapedBuilder & tOut ) const final;
	bool		LoadWordforms ( const StrVec_t & dFiles, const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex ) final;
	void		WriteWordforms ( CSphWriter & tWriter ) const final;
	void		WriteWordforms ( JsonEscapedBuilder & tOut ) const final;
	const CSphWordforms *	GetWordforms() final { return m_pWordforms; }
	void		DisableWordforms() final { m_bDisableWordforms = true; }
	int			SetMorphology ( const char * szMorph, CSphString & sMessage ) final;
	bool		HasMorphology() const final;
	void		ApplyStemmers ( BYTE * pWord ) const final;
	const CSphMultiformContainer * GetMultiWordforms () const final;
	uint64_t	GetSettingsFNV () const final;
	static void			SweepWordformContainers ( const CSphVector<CSphSavedFile> & dFiles );

protected:
	CSphVector < int >	m_dMorph;
#if WITH_STEMMER
	CSphVector < sb_stemmer * >	m_dStemmers;
	StrVec_t			m_dDescStemmers;
#endif
	CSphScopedPtr<LemmatizerTrait_i> m_tLemmatizer { nullptr };

	int					m_iStopwords;	///< stopwords count
	SphWordID_t *		m_pStopwords;	///< stopwords ID list
	CSphFixedVector<SphWordID_t> m_dStopwordContainer;

protected:
	int					ParseMorphology ( const char * szMorph, CSphString & sError );
	SphWordID_t			FilterStopword ( SphWordID_t uID ) const;	///< filter ID against stopwords list
	CSphDict *			CloneBase ( CSphTemplateDictTraits * pDict ) const;
	bool				HasState () const final;

	bool				m_bDisableWordforms;

private:
	CSphWordforms *				m_pWordforms;
	static CSphVector<CSphWordforms*>		m_dWordformContainers;

	CSphWordforms *		GetWordformContainer ( const CSphVector<CSphSavedFile> & dFileInfos, const StrVec_t * pEmbeddedWordforms, const ISphTokenizer * pTokenizer, const char * sIndex );
	CSphWordforms *		LoadWordformContainer ( const CSphVector<CSphSavedFile> & dFileInfos, const StrVec_t * pEmbeddedWordforms, const ISphTokenizer * pTokenizer, const char * sIndex );

	int					InitMorph ( const char * szMorph, int iLength, CSphString & sError );
	int					AddMorph ( int iMorph ); ///< helper that always returns ST_OK
	bool				StemById ( BYTE * pWord, int iStemmer ) const;
	void				AddWordform ( CSphWordforms * pContainer, char * sBuffer, int iLen, ISphTokenizer * pTokenizer, const char * szFile, const CSphVector<int> & dBlended, int iFileId );
};

CSphVector<CSphWordforms*> CSphTemplateDictTraits::m_dWordformContainers;


/// common CRC32/64 dictionary stuff
struct CSphDiskDictTraits : CSphTemplateDictTraits
{
	void DictBegin ( CSphAutofile & tTempDict, CSphAutofile & tDict, int iDictLimit ) override;
	void DictEntry ( const CSphDictEntry & tEntry ) override;
	void DictEndEntries ( SphOffset_t iDoclistOffset ) override;
	bool DictEnd ( DictHeader_t * pHeader, int iMemLimit, CSphString & sError ) override;
	bool DictIsError () const final { return m_wrDict.IsError(); }
	void SetSkiplistBlockSize ( int iSize ) final { m_iSkiplistBlockSize=iSize; }

protected:
	~CSphDiskDictTraits () override = default; // fixme! remove

protected:
	CSphTightVector<CSphWordlistCheckpoint>	m_dCheckpoints;		///< checkpoint offsets
	CSphWriter			m_wrDict;			///< final dict file writer
	CSphString			m_sWriterError;		///< writer error message storage
	int					m_iEntries = 0;			///< dictionary entries stored
	SphOffset_t			m_iLastDoclistPos = 0;
	SphWordID_t			m_iLastWordID = 0;
	int					m_iSkiplistBlockSize = 0;
};


template < bool CRCALGO >
struct CCRCEngine
{
	inline static SphWordID_t		DoCrc ( const BYTE * pWord );
	inline static SphWordID_t		DoCrc ( const BYTE * pWord, int iLen );
};

/// specialized CRC32/64 implementations
template < bool CRC32DICT >
struct CSphDictCRC : public CSphDiskDictTraits, CCRCEngine<CRC32DICT>
{
	typedef CCRCEngine<CRC32DICT> tHASH;
	SphWordID_t		GetWordID ( BYTE * pWord ) override;
	SphWordID_t		GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) override;
	SphWordID_t		GetWordIDWithMarkers ( BYTE * pWord ) override;
	SphWordID_t		GetWordIDNonStemmed ( BYTE * pWord ) override;
	bool			IsStopWord ( const BYTE * pWord ) const final;

	CSphDict *		Clone () const override { return CloneBase ( new CSphDictCRC<CRC32DICT>() ); }
protected:
	~CSphDictCRC() override {} // fixme! remove
};

struct CSphDictTemplate final : public CSphTemplateDictTraits, CCRCEngine<false> // based on flv64
{
	SphWordID_t		GetWordID ( BYTE * pWord ) final;
	SphWordID_t		GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) final;
	SphWordID_t		GetWordIDWithMarkers ( BYTE * pWord ) final;
	SphWordID_t		GetWordIDNonStemmed ( BYTE * pWord ) final;
	bool			IsStopWord ( const BYTE * pWord ) const final;

	CSphDict *		Clone () const final { return CloneBase ( new CSphDictTemplate() ); }

protected:
	~CSphDictTemplate () final 	{} // fixme! remove
};

/////////////////////////////////////////////////////////////////////////////

CSphWordforms::CSphWordforms()
	: m_iRefCount ( 0 )
	, m_uTokenizerFNV ( 0 )
	, m_bHavePostMorphNF ( false )
	, m_pMultiWordforms ( NULL )
{
}


CSphWordforms::~CSphWordforms()
{
	if ( m_pMultiWordforms )
	{
		m_pMultiWordforms->m_Hash.IterateStart ();
		while ( m_pMultiWordforms->m_Hash.IterateNext () )
		{
			CSphMultiforms * pWordforms = m_pMultiWordforms->m_Hash.IterateGet ();
			ARRAY_FOREACH ( i, pWordforms->m_pForms )
				SafeDelete ( pWordforms->m_pForms[i] );

			SafeDelete ( pWordforms );
		}

		SafeDelete ( m_pMultiWordforms );
	}
}


bool CSphWordforms::IsEqual ( const CSphVector<CSphSavedFile> & dFiles )
{
	if ( m_dFiles.GetLength()!=dFiles.GetLength() )
		return false;

	// don't check file time AND check filename w/o path
	// that way same file in different dirs will be only loaded once
	ARRAY_FOREACH ( i, m_dFiles )
	{
		const CSphSavedFile & tF1 = m_dFiles[i];
		const CSphSavedFile & tF2 = dFiles[i];
		CSphString sFile1 = tF1.m_sFilename;
		CSphString sFile2 = tF2.m_sFilename;
		StripPath(sFile1);
		StripPath(sFile2);

		if ( sFile1!=sFile2 || tF1.m_uCRC32!=tF2.m_uCRC32 || tF1.m_uSize!=tF2.m_uSize )
			return false;
	}

	return true;
}


bool CSphWordforms::ToNormalForm ( BYTE * pWord, bool bBefore, bool bOnlyCheck ) const
{
	int * pIndex = m_hHash ( (char *)pWord );
	if ( !pIndex )
		return false;

	if ( *pIndex<0 || *pIndex>=m_dNormalForms.GetLength () )
		return false;

	if ( bBefore==m_dNormalForms[*pIndex].m_bAfterMorphology )
		return false;

	if ( m_dNormalForms[*pIndex].m_sWord.IsEmpty() )
		return false;

	if ( bOnlyCheck )
		return true;

	strcpy ( (char *)pWord, m_dNormalForms[*pIndex].m_sWord.cstr() ); // NOLINT
	return true;
}

/////////////////////////////////////////////////////////////////////////////

CSphTemplateDictTraits::CSphTemplateDictTraits ()
	: m_iStopwords	( 0 )
	, m_pStopwords	( NULL )
	, m_dStopwordContainer ( 0 )
	, m_bDisableWordforms ( false )
	, m_pWordforms	( NULL )
{
}


CSphTemplateDictTraits::~CSphTemplateDictTraits ()
{
#if WITH_STEMMER
	ARRAY_FOREACH ( i, m_dStemmers )
		sb_stemmer_delete ( m_dStemmers[i] );
#endif

	if ( m_pWordforms )
		--m_pWordforms->m_iRefCount;
}


SphWordID_t CSphTemplateDictTraits::FilterStopword ( SphWordID_t uID ) const
{
	if ( !m_iStopwords )
		return uID;

	// OPTIMIZE: binary search is not too good, could do some hashing instead
	SphWordID_t * pStart = m_pStopwords;
	SphWordID_t * pEnd = m_pStopwords + m_iStopwords - 1;
	do
	{
		if ( uID==*pStart || uID==*pEnd )
			return 0;

		if ( uID<*pStart || uID>*pEnd )
			return uID;

		SphWordID_t * pMid = pStart + (pEnd-pStart)/2;
		if ( uID==*pMid )
			return 0;

		if ( uID<*pMid )
			pEnd = pMid;
		else
			pStart = pMid;
	} while ( pEnd-pStart>1 );

	return uID;
}


int CSphTemplateDictTraits::ParseMorphology ( const char * sMorph, CSphString & sMessage )
{
	int iRes = ST_OK;
	for ( const char * sStart=sMorph; ; )
	{
		while ( *sStart && ( sphIsSpace ( *sStart ) || *sStart==',' ) )
			++sStart;
		if ( !*sStart )
			break;

		const char * sWordStart = sStart;
		while ( *sStart && !sphIsSpace ( *sStart ) && *sStart!=',' )
			++sStart;

		if ( sStart > sWordStart )
		{
			switch ( InitMorph ( sWordStart, int ( sStart - sWordStart ), sMessage ) )
			{
				case ST_ERROR:		return ST_ERROR;
				case ST_WARNING:	iRes = ST_WARNING; break;
				default:			break;
			}
		}
	}
	return iRes;
}


int CSphTemplateDictTraits::InitMorph ( const char * szMorph, int iLength, CSphString & sMessage )
{
	if ( iLength==0 )
		return ST_OK;

	if ( iLength==4 && !strncmp ( szMorph, "none", iLength ) )
		return ST_OK;

	if ( iLength==7 && !strncmp ( szMorph, "stem_en", iLength ) )
	{
		if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_EN ) )
		{
			sMessage.SetSprintf ( "stem_en and lemmatize_en clash" );
			return ST_ERROR;
		}

		if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_EN_ALL ) )
		{
			sMessage.SetSprintf ( "stem_en and lemmatize_en_all clash" );
			return ST_ERROR;
		}

		stem_en_init ();
		return AddMorph ( SPH_MORPH_STEM_EN );
	}

	if ( iLength==7 && !strncmp ( szMorph, "stem_ru", iLength ) )
	{
		if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_RU_UTF8 ) )
		{
			sMessage.SetSprintf ( "stem_ru and lemmatize_ru clash" );
			return ST_ERROR;
		}

		if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_RU_ALL ) )
		{
			sMessage.SetSprintf ( "stem_ru and lemmatize_ru_all clash" );
			return ST_ERROR;
		}

		stem_ru_init ();
		return AddMorph ( SPH_MORPH_STEM_RU_UTF8 );
	}

	for ( int j=0; j<AOT_LENGTH; ++j )
	{
		char buf[20];
		char buf_all[20];
		sprintf ( buf, "lemmatize_%s", AOT_LANGUAGES[j] ); // NOLINT
		sprintf ( buf_all, "lemmatize_%s_all", AOT_LANGUAGES[j] ); // NOLINT

		if ( iLength==12 && !strncmp ( szMorph, buf, iLength ) )
		{
			if ( j==AOT_RU && m_dMorph.Contains ( SPH_MORPH_STEM_RU_UTF8 ) )
			{
				sMessage.SetSprintf ( "stem_ru and lemmatize_ru clash" );
				return ST_ERROR;
			}

			if ( j==AOT_EN && m_dMorph.Contains ( SPH_MORPH_STEM_EN ) )
			{
				sMessage.SetSprintf ( "stem_en and lemmatize_en clash" );
				return ST_ERROR;
			}

			// no test for SPH_MORPH_STEM_DE since we doesn't have it.

			if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_BASE_ALL+j ) )
			{
				sMessage.SetSprintf ( "%s and %s clash", buf, buf_all );
				return ST_ERROR;
			}

			CSphString sDictFile;
			sDictFile.SetSprintf ( "%s/%s.pak", g_sLemmatizerBase.cstr(), AOT_LANGUAGES[j] );
			if ( !sphAotInit ( sDictFile, sMessage, j ) )
				return ST_ERROR;

			if ( j==AOT_UK && !m_tLemmatizer.Ptr() )
				m_tLemmatizer = CreateLemmatizer ( j );

			// add manually instead of AddMorph(), because we need to update that fingerprint
			int iMorph = j + SPH_MORPH_AOTLEMMER_BASE;
			if ( j==AOT_RU )
				iMorph = SPH_MORPH_AOTLEMMER_RU_UTF8;
			else if ( j==AOT_DE )
				iMorph = SPH_MORPH_AOTLEMMER_DE_UTF8;
			else if ( j==AOT_UK )
				iMorph = SPH_MORPH_AOTLEMMER_UK;

			if ( !m_dMorph.Contains ( iMorph ) )
			{
				if ( m_sMorphFingerprint.IsEmpty() )
					m_sMorphFingerprint.SetSprintf ( "%s:%08x"
						, sphAotDictinfo(j).first.cstr()
						, sphAotDictinfo(j).second );
				else
					m_sMorphFingerprint.SetSprintf ( "%s;%s:%08x"
					, m_sMorphFingerprint.cstr()
					, sphAotDictinfo(j).first.cstr()
					, sphAotDictinfo(j).second );
				m_dMorph.Add ( iMorph );
			}
			return ST_OK;
		}

		if ( iLength==16 && !strncmp ( szMorph, buf_all, iLength ) )
		{
			if ( j==AOT_RU && ( m_dMorph.Contains ( SPH_MORPH_STEM_RU_UTF8 ) ) )
			{
				sMessage.SetSprintf ( "stem_ru and lemmatize_ru_all clash" );
				return ST_ERROR;
			}

			if ( m_dMorph.Contains ( SPH_MORPH_AOTLEMMER_BASE+j ) )
			{
				sMessage.SetSprintf ( "%s and %s clash", buf, buf_all );
				return ST_ERROR;
			}

			CSphString sDictFile;
			sDictFile.SetSprintf ( "%s/%s.pak", g_sLemmatizerBase.cstr(), AOT_LANGUAGES[j] );
			if ( !sphAotInit ( sDictFile, sMessage, j ) )
				return ST_ERROR;

			if ( j==AOT_UK && !m_tLemmatizer.Ptr() )
				m_tLemmatizer = CreateLemmatizer ( j );

			return AddMorph ( SPH_MORPH_AOTLEMMER_BASE_ALL+j );
		}
	}

	if ( iLength==7 && !strncmp ( szMorph, "stem_cz", iLength ) )
	{
		stem_cz_init ();
		return AddMorph ( SPH_MORPH_STEM_CZ );
	}

	if ( iLength==7 && !strncmp ( szMorph, "stem_ar", iLength ) )
		return AddMorph ( SPH_MORPH_STEM_AR_UTF8 );

	if ( iLength==9 && !strncmp ( szMorph, "stem_enru", iLength ) )
	{
		stem_en_init ();
		stem_ru_init ();
		AddMorph ( SPH_MORPH_STEM_EN );
		return AddMorph ( SPH_MORPH_STEM_RU_UTF8 );
	}

	if ( iLength==7 && !strncmp ( szMorph, "soundex", iLength ) )
		return AddMorph ( SPH_MORPH_SOUNDEX );

	if ( iLength==9 && !strncmp ( szMorph, "metaphone", iLength ) )
		return AddMorph ( SPH_MORPH_METAPHONE_UTF8 );

#if WITH_STEMMER
	const int LIBSTEMMER_LEN = 11;
	const int MAX_ALGO_LENGTH = 64;
	if ( iLength > LIBSTEMMER_LEN && iLength - LIBSTEMMER_LEN < MAX_ALGO_LENGTH && !strncmp ( szMorph, "libstemmer_", LIBSTEMMER_LEN ) )
	{
		CSphString sAlgo;
		sAlgo.SetBinary ( szMorph+LIBSTEMMER_LEN, iLength - LIBSTEMMER_LEN );

		sb_stemmer * pStemmer = NULL;

		pStemmer = sb_stemmer_new ( sAlgo.cstr(), "UTF_8" );

		if ( !pStemmer )
		{
			sMessage.SetSprintf ( "unknown stemmer libstemmer_%s; skipped", sAlgo.cstr() );
			return ST_WARNING;
		}

		AddMorph ( SPH_MORPH_LIBSTEMMER_FIRST + m_dStemmers.GetLength () );
		ARRAY_FOREACH ( i, m_dStemmers )
		{
			if ( m_dStemmers[i]==pStemmer )
			{
				sb_stemmer_delete ( pStemmer );
				return ST_OK;
			}
		}

		m_dStemmers.Add ( pStemmer );
		m_dDescStemmers.Add ( sAlgo );
		return ST_OK;
	}
#endif

	if ( iLength==11 && !strncmp ( szMorph, "icu_chinese", iLength ) )
		return ST_OK;

	sMessage.SetBinary ( szMorph, iLength );
	sMessage.SetSprintf ( "unknown stemmer %s", sMessage.cstr() );
	return ST_ERROR;
}


int CSphTemplateDictTraits::AddMorph ( int iMorph )
{
	if ( !m_dMorph.Contains ( iMorph ) )
		m_dMorph.Add ( iMorph );
	return ST_OK;
}



void CSphTemplateDictTraits::ApplyStemmers ( BYTE * pWord ) const
{
	// try wordforms
	if ( m_pWordforms && m_pWordforms->ToNormalForm ( pWord, true, m_bDisableWordforms ) )
		return;

	// check length
	if ( m_tSettings.m_iMinStemmingLen<=1 || sphUTF8Len ( (const char*)pWord )>=m_tSettings.m_iMinStemmingLen )
	{
		// try stemmers
		ARRAY_FOREACH ( i, m_dMorph )
			if ( StemById ( pWord, m_dMorph[i] ) )
				break;
	}

	if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
		m_pWordforms->ToNormalForm ( pWord, false, m_bDisableWordforms );
}

const CSphMultiformContainer * CSphTemplateDictTraits::GetMultiWordforms () const
{
	return m_pWordforms ? m_pWordforms->m_pMultiWordforms : NULL;
}

uint64_t CSphTemplateDictTraits::GetSettingsFNV () const
{
	uint64_t uHash = (uint64_t)m_pWordforms;

	if ( m_pStopwords )
		uHash = sphFNV64 ( m_pStopwords, m_iStopwords*sizeof(*m_pStopwords), uHash );

	uHash = sphFNV64 ( &m_tSettings.m_iMinStemmingLen, sizeof(m_tSettings.m_iMinStemmingLen), uHash );
	DWORD uFlags = 0;
	if ( m_tSettings.m_bWordDict )
		uFlags |= 1<<0;
	if ( m_tSettings.m_bStopwordsUnstemmed )
		uFlags |= 1<<2;
	uHash = sphFNV64 ( &uFlags, sizeof(uFlags), uHash );

	uHash = sphFNV64 ( m_dMorph.Begin(), m_dMorph.GetLength()*sizeof(m_dMorph[0]), uHash );
#if WITH_STEMMER
	ARRAY_FOREACH ( i, m_dDescStemmers )
		uHash = sphFNV64 ( m_dDescStemmers[i].cstr(), m_dDescStemmers[i].Length(), uHash );
#endif

	return uHash;
}


CSphDict * CSphTemplateDictTraits::CloneBase ( CSphTemplateDictTraits * pDict ) const
{
	assert ( pDict );
	pDict->m_tSettings = m_tSettings;
	pDict->m_iStopwords = m_iStopwords;
	pDict->m_pStopwords = m_pStopwords;
	pDict->m_dSWFileInfos = m_dSWFileInfos;
	pDict->m_dWFFileInfos = m_dWFFileInfos;
	pDict->m_pWordforms = m_pWordforms;
	if ( m_pWordforms )
		m_pWordforms->m_iRefCount++;

	pDict->m_dMorph = m_dMorph;
#if WITH_STEMMER
	assert ( m_dDescStemmers.GetLength()==m_dStemmers.GetLength() );
	pDict->m_dDescStemmers = m_dDescStemmers;
	ARRAY_FOREACH ( i, m_dDescStemmers )
	{
		pDict->m_dStemmers.Add ( sb_stemmer_new ( m_dDescStemmers[i].cstr(), "UTF_8" ) );
		assert ( pDict->m_dStemmers.Last() );
	}
#endif
	if ( m_tLemmatizer.Ptr() )
		pDict->m_tLemmatizer = CreateLemmatizer ( AOT_UK );

	return pDict;
}

bool CSphTemplateDictTraits::HasState() const
{
#if !WITH_STEMMER
	return ( m_tLemmatizer.Ptr() );
#else
	return ( m_dDescStemmers.GetLength()>0 || m_tLemmatizer.Ptr() );
#endif
}

/////////////////////////////////////////////////////////////////////////////

template<>
SphWordID_t CCRCEngine<true>::DoCrc ( const BYTE * pWord )
{
	return sphCRC32 ( pWord );
}


template<>
SphWordID_t CCRCEngine<false>::DoCrc ( const BYTE * pWord )
{
	return (SphWordID_t) sphFNV64 ( pWord );
}


template<>
SphWordID_t CCRCEngine<true>::DoCrc ( const BYTE * pWord, int iLen )
{
	return sphCRC32 ( pWord, iLen );
}


template<>
SphWordID_t CCRCEngine<false>::DoCrc ( const BYTE * pWord, int iLen )
{
	return (SphWordID_t) sphFNV64 ( pWord, iLen );
}


template < bool CRC32DICT >
SphWordID_t CSphDictCRC<CRC32DICT>::GetWordID ( BYTE * pWord )
{
	// apply stopword filter before stemmers
	if ( GetSettings().m_bStopwordsUnstemmed && !FilterStopword ( tHASH::DoCrc ( pWord ) ) )
		return 0;

	// skip stemmers for magic words
	if ( pWord[0]>=0x20 )
		ApplyStemmers ( pWord );

	// stemmer might squeeze out the word
	if ( !pWord[0] )
		return 0;

	return GetSettings().m_bStopwordsUnstemmed
		? tHASH::DoCrc ( pWord )
		: FilterStopword ( tHASH::DoCrc ( pWord ) );
}


template < bool CRC32DICT >
SphWordID_t CSphDictCRC<CRC32DICT>::GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uId = tHASH::DoCrc ( pWord, iLen );
	return bFilterStops ? FilterStopword ( uId ) : uId;
}


template < bool CRC32DICT >
SphWordID_t CSphDictCRC<CRC32DICT>::GetWordIDWithMarkers ( BYTE * pWord )
{
	ApplyStemmers ( pWord + 1 );
	SphWordID_t uWordId = tHASH::DoCrc ( pWord + 1 );
	auto iLength = strlen ( (const char *)(pWord + 1) );
	pWord [iLength + 1] = MAGIC_WORD_TAIL;
	pWord [iLength + 2] = '\0';
	return FilterStopword ( uWordId ) ? tHASH::DoCrc ( pWord ) : 0;
}


template < bool CRC32DICT >
SphWordID_t CSphDictCRC<CRC32DICT>::GetWordIDNonStemmed ( BYTE * pWord )
{
	// this method can generally receive both '=stopword' with a marker and 'stopword' without it
	// so for filtering stopwords, let's handle both
	int iOff = ( pWord[0]<' ' );
	SphWordID_t uWordId = tHASH::DoCrc ( pWord+iOff );
	if ( !FilterStopword ( uWordId ) )
		return 0;

	return tHASH::DoCrc ( pWord );
}


template < bool CRC32DICT >
bool CSphDictCRC<CRC32DICT>::IsStopWord ( const BYTE * pWord ) const
{
	return FilterStopword ( tHASH::DoCrc ( pWord ) )==0;
}


//////////////////////////////////////////////////////////////////////////
SphWordID_t CSphDictTemplate::GetWordID ( BYTE * pWord )
{
	// apply stopword filter before stemmers
	if ( GetSettings().m_bStopwordsUnstemmed && !FilterStopword ( DoCrc ( pWord ) ) )
		return 0;

	// skip stemmers for magic words
	if ( pWord[0]>=0x20 )
		ApplyStemmers ( pWord );

	return GetSettings().m_bStopwordsUnstemmed
		? DoCrc ( pWord )
		: FilterStopword ( DoCrc ( pWord ) );
}


SphWordID_t CSphDictTemplate::GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uId = DoCrc ( pWord, iLen );
	return bFilterStops ? FilterStopword ( uId ) : uId;
}


SphWordID_t CSphDictTemplate::GetWordIDWithMarkers ( BYTE * pWord )
{
	ApplyStemmers ( pWord + 1 );
	// stemmer might squeeze out the word
	if ( !pWord[1] )
		return 0;
	SphWordID_t uWordId = DoCrc ( pWord + 1 );
	auto iLength = (int) strlen ( (const char *)(pWord + 1) );
	pWord [iLength + 1] = MAGIC_WORD_TAIL;
	pWord [iLength + 2] = '\0';
	return FilterStopword ( uWordId ) ? DoCrc ( pWord ) : 0;
}


SphWordID_t CSphDictTemplate::GetWordIDNonStemmed ( BYTE * pWord )
{
	SphWordID_t uWordId = DoCrc ( pWord + 1 );
	if ( !FilterStopword ( uWordId ) )
		return 0;

	return DoCrc ( pWord );
}

bool CSphDictTemplate::IsStopWord ( const BYTE * pWord ) const
{
	return FilterStopword ( DoCrc ( pWord ) )==0;
}

//////////////////////////////////////////////////////////////////////////

void CSphTemplateDictTraits::LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile )
{
	assert ( !m_pStopwords );
	assert ( !m_iStopwords );

	// tokenize file list
	if ( !sFiles || !*sFiles )
		return;

	m_dSWFileInfos.Resize ( 0 );

	TokenizerRefPtr_c tTokenizer ( pTokenizer->Clone ( SPH_CLONE_INDEX ) );
	CSphFixedVector<char> dList ( 1+(int)strlen(sFiles) );
	strcpy ( dList.Begin(), sFiles ); // NOLINT

	char * pCur = dList.Begin();
	char * sName = NULL;

	CSphVector<SphWordID_t> dStop;

	while (true)
	{
		// find next name start
		while ( *pCur && ( isspace(*pCur) || *pCur==',' ) ) pCur++;
		if ( !*pCur ) break;
		sName = pCur;

		// find next name end
		while ( *pCur && !( isspace(*pCur) || *pCur==',' ) ) pCur++;
		if ( *pCur ) *pCur++ = '\0';

		CSphString sFileName = sName;
		bool bGotFile = sphIsReadable ( sFileName );
		if ( !bGotFile )
		{
			if ( bStripFile )
			{
				StripPath ( sFileName );
				bGotFile = sphIsReadable ( sFileName );
			}
			if ( !bGotFile )
			{
				if ( !bStripFile )
					StripPath ( sFileName );
				sFileName.SetSprintf ( "%s/stopwords/%s", GET_FULL_SHARE_DIR(), sFileName.cstr() );
				bGotFile = sphIsReadable ( sFileName );
			}
		}

		CSphFixedVector<BYTE> dBuffer ( 0 );
		CSphSavedFile tInfo;
		tInfo.Collect ( sFileName.cstr() );

		// need to store original name to compatible with original behavior of load order
		// from path defined; from tool CWD; from SHARE_DIR
		tInfo.m_sFilename = sName; 
		m_dSWFileInfos.Add ( tInfo );

		if ( !bGotFile )
		{
			StringBuilder_c sError;
			sError.Appendf ( "failed to load stopwords from either '%s' or '%s'", sName, sFileName.cstr() );
			if ( bStripFile )
				sError += ", current work directory";
			sphWarn ( "%s", sError.cstr() );
			continue;
		}

		// open file
		FILE * fp = fopen ( sFileName.cstr(), "rb" );
		if ( !fp )
		{
			sphWarn ( "failed to load stopwords from '%s'", sFileName.cstr() );
			continue;
		}

		struct_stat st = {0};
		if ( fstat ( fileno (fp) , &st )==0 )
			dBuffer.Reset ( st.st_size );
		else
		{
			fclose ( fp );
			sphWarn ( "stopwords: failed to get file size for '%s'", sFileName.cstr() );
			continue;
		}

		// tokenize file
		int iLength = (int)fread ( dBuffer.Begin(), 1, (size_t)st.st_size, fp );

		BYTE * pToken;
		tTokenizer->SetBuffer ( dBuffer.Begin(), iLength );
		while ( ( pToken = tTokenizer->GetToken() )!=NULL )
			if ( m_tSettings.m_bStopwordsUnstemmed )
				dStop.Add ( GetWordIDNonStemmed ( pToken ) );
			else
				dStop.Add ( GetWordID ( pToken ) );

		// close file
		fclose ( fp );
	}

	// sort stopwords
	dStop.Uniq();

	// store IDs
	if ( dStop.GetLength() )
	{
		m_dStopwordContainer.Reset ( dStop.GetLength() );
		ARRAY_FOREACH ( i, dStop )
			m_dStopwordContainer[i] = dStop[i];

		m_iStopwords = m_dStopwordContainer.GetLength ();
		m_pStopwords = m_dStopwordContainer.Begin();
	}
}


void CSphTemplateDictTraits::LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords )
{
	m_dStopwordContainer.Reset ( dStopwords.GetLength() );
	ARRAY_FOREACH ( i, dStopwords )
		m_dStopwordContainer[i] = dStopwords[i];

	m_iStopwords = m_dStopwordContainer.GetLength ();
	m_pStopwords = m_dStopwordContainer.Begin();
}


void CSphTemplateDictTraits::WriteStopwords ( CSphWriter & tWriter ) const
{
	tWriter.PutDword ( (DWORD)m_iStopwords );
	for ( int i = 0; i < m_iStopwords; ++i )
		tWriter.ZipOffset ( m_pStopwords[i] );
}

void CSphTemplateDictTraits::WriteStopwords ( JsonEscapedBuilder& tOut ) const
{
	if ( !m_iStopwords )
		return;

	tOut.Named ( "stopwords_list" );
	auto _ = tOut.Array();
	for ( int i = 0; i < m_iStopwords; ++i )
		tOut << cast2signed ( m_pStopwords[i] );
}


void CSphTemplateDictTraits::SweepWordformContainers ( const CSphVector<CSphSavedFile> & dFiles )
{
	for ( int i = 0; i < m_dWordformContainers.GetLength (); )
	{
		CSphWordforms * WC = m_dWordformContainers[i];
		if ( WC->m_iRefCount==0 && !WC->IsEqual ( dFiles ) )
		{
			delete WC;
			m_dWordformContainers.Remove ( i );
		} else
			++i;
	}
}


static const int MAX_REPORT_LEN = 1024;

static void AddStringToReport ( CSphString & sReport, const CSphString & sString, bool bLast )
{
	int iLen = sReport.Length();
	if ( iLen + sString.Length() + 2 > MAX_REPORT_LEN )
		return;

	char * szReport = const_cast<char *>( sReport.cstr() );
	strcat ( szReport, sString.cstr() );	// NOLINT
	iLen += sString.Length();
	if ( bLast )
		szReport[iLen] = '\0';
	else
	{
		szReport[iLen] = ' ';
		szReport[iLen+1] = '\0';
	}
}


static void ConcatReportStrings ( const CSphTightVector<CSphString> & dStrings, CSphString & sReport )
{
	sReport.Reserve ( MAX_REPORT_LEN );
	*const_cast<char *>(sReport.cstr()) = '\0';

	ARRAY_FOREACH ( i, dStrings )
		AddStringToReport ( sReport, dStrings[i], i==dStrings.GetLength()-1 );
}


static void ConcatReportStrings ( const CSphTightVector<CSphNormalForm> & dStrings, CSphString & sReport )
{
	sReport.Reserve ( MAX_REPORT_LEN );
	*const_cast<char *>(sReport.cstr()) = '\0';

	ARRAY_FOREACH ( i, dStrings )
		AddStringToReport ( sReport, dStrings[i].m_sForm, i==dStrings.GetLength()-1 );
}


CSphWordforms * CSphTemplateDictTraits::GetWordformContainer ( const CSphVector<CSphSavedFile> & dFileInfos, const StrVec_t * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex )
{
	uint64_t uTokenizerFNV = pTokenizer->GetSettingsFNV();
	ARRAY_FOREACH ( i, m_dWordformContainers )
		if ( m_dWordformContainers[i]->IsEqual ( dFileInfos ) )
		{
			CSphWordforms * pContainer = m_dWordformContainers[i];
			if ( uTokenizerFNV==pContainer->m_uTokenizerFNV )
				return pContainer;

			CSphTightVector<CSphString> dErrorReport;
			ARRAY_FOREACH ( j, dFileInfos )
				dErrorReport.Add ( dFileInfos[j].m_sFilename );

			CSphString sAllFiles;
			ConcatReportStrings ( dErrorReport, sAllFiles );
			sphWarning ( "index '%s': wordforms file '%s' is shared with index '%s', "
				"but tokenizer settings are different",
				sIndex, sAllFiles.cstr(), pContainer->m_sIndexName.cstr() );
		}

	CSphWordforms * pContainer = LoadWordformContainer ( dFileInfos, pEmbedded, pTokenizer, sIndex );
	if ( pContainer )
		m_dWordformContainers.Add ( pContainer );

	return pContainer;
}




void CSphTemplateDictTraits::AddWordform ( CSphWordforms * pContainer, char * sBuffer, int iLen,
	ISphTokenizer * pTokenizer, const char * szFile, const CSphVector<int> & dBlended, int iFileId )
{
	StrVec_t dTokens;

	bool bSeparatorFound = false;
	bool bAfterMorphology = false;

	// parse the line
	pTokenizer->SetBuffer ( (BYTE*)sBuffer, iLen );

	bool bFirstToken = true;
	bool bStopwordsPresent = false;
	bool bCommentedWholeLine = false;

	BYTE * pFrom = nullptr;
	while ( ( pFrom = pTokenizer->GetToken () )!=nullptr )
	{
		if ( *pFrom=='#' )
		{
			bCommentedWholeLine = bFirstToken;
			break;
		}

		if ( *pFrom=='~' && bFirstToken )
		{
			bAfterMorphology = true;
			bFirstToken = false;
			continue;
		}

		bFirstToken = false;

		if ( *pFrom=='>' )
		{
			bSeparatorFound = true;
			break;
		}

		if ( *pFrom=='=' && *pTokenizer->GetBufferPtr()=='>' )
		{
			pTokenizer->GetToken();
			bSeparatorFound = true;
			break;
		}

		if ( GetWordID ( pFrom, (int)strlen ( (const char*)pFrom ), true ) )
			dTokens.Add ( (const char*)pFrom );
		else
			bStopwordsPresent = true;
	}

	if ( !dTokens.GetLength() )
	{
		if ( !bCommentedWholeLine )
			sphWarning ( "index '%s': all source tokens are stopwords (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( !bSeparatorFound )
	{
		sphWarning ( "index '%s': no wordform separator found (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	BYTE * pTo = pTokenizer->GetToken ();
	if ( !pTo )
	{
		sphWarning ( "index '%s': no destination token found (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( *pTo=='#' )
	{
		sphWarning ( "index '%s': misplaced comment (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	CSphVector<CSphNormalForm> dDestTokens;
	bool bFirstDestIsStop = !GetWordID ( pTo, (int) strlen ( (const char*)pTo ), true );
	CSphNormalForm & tForm = dDestTokens.Add();
	tForm.m_sForm = (const char *)pTo;
	tForm.m_iLengthCP = pTokenizer->GetLastTokenLen();

	// what if we have more than one word in the right part?
	const BYTE * pDestToken;
	while ( ( pDestToken = pTokenizer->GetToken() )!=nullptr )
	{
		bool bStop = ( !GetWordID ( pDestToken, (int) strlen ( (const char*)pDestToken ), true ) );
		if ( !bStop )
		{
			CSphNormalForm & tNewForm = dDestTokens.Add();
			tNewForm.m_sForm = (const char *)pDestToken;
			tNewForm.m_iLengthCP = pTokenizer->GetLastTokenLen();
		}

		bStopwordsPresent |= bStop;
	}

	// we can have wordforms with 1 destination token that is a stopword
	if ( dDestTokens.GetLength()>1 && bFirstDestIsStop )
		dDestTokens.Remove(0);

	if ( !dDestTokens.GetLength() )
	{
		sphWarning ( "index '%s': destination token is a stopword (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( bStopwordsPresent )
		sphWarning ( "index '%s': wordform contains stopwords (wordform='%s'). Fix your wordforms file '%s'.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );

	// we disabled all blended, so we need to filter them manually
	bool bBlendedPresent = false;
	if ( dBlended.GetLength() )
		ARRAY_FOREACH ( i, dDestTokens )
		{
			int iCode;
			const BYTE * pBuf = (const BYTE *) dDestTokens[i].m_sForm.cstr();
			while ( ( iCode = sphUTF8Decode ( pBuf ) )>0 && !bBlendedPresent )
				bBlendedPresent = ( dBlended.BinarySearch ( iCode )!=nullptr );
		}

	if ( bBlendedPresent )
		sphWarning ( "invalid mapping (destination contains blended characters) (wordform='%s'). Fix your wordforms file '%s'.", sBuffer, szFile );

	if ( bBlendedPresent && dDestTokens.GetLength()>1 )
	{
		sphWarning ( "blended characters are not allowed with multiple destination tokens (wordform='%s', file='%s'). IGNORED.", sBuffer, szFile );
		return;
	}

	if ( dTokens.GetLength()>1 || dDestTokens.GetLength()>1 )
	{
		auto * pMultiWordform = new CSphMultiform;
		pMultiWordform->m_iFileId = iFileId;
		pMultiWordform->m_dNormalForm.Resize ( dDestTokens.GetLength() );
		ARRAY_FOREACH ( i, dDestTokens )
			pMultiWordform->m_dNormalForm[i] = dDestTokens[i];

		for ( int i = 1; i < dTokens.GetLength(); i++ )
			pMultiWordform->m_dTokens.Add ( dTokens[i] );

		if ( !pContainer->m_pMultiWordforms )
			pContainer->m_pMultiWordforms = new CSphMultiformContainer;

		CSphMultiforms ** ppWordforms = pContainer->m_pMultiWordforms->m_Hash ( dTokens[0] );
		if ( ppWordforms )
		{
			auto * pWordforms = *ppWordforms;
			for ( CSphMultiform *pStoredMF : pWordforms->m_pForms )
			{
				if ( pStoredMF->m_dTokens.GetLength()==pMultiWordform->m_dTokens.GetLength() )
				{
					bool bSameTokens = true;
					ARRAY_FOREACH_COND ( iToken, pStoredMF->m_dTokens, bSameTokens )
						if ( pStoredMF->m_dTokens[iToken]!=pMultiWordform->m_dTokens[iToken] )
							bSameTokens = false;

					if ( bSameTokens )
					{
						CSphString sStoredTokens, sStoredForms;
						ConcatReportStrings ( pStoredMF->m_dTokens, sStoredTokens );
						ConcatReportStrings ( pStoredMF->m_dNormalForm, sStoredForms );
						sphWarning ( "index '%s': duplicate wordform found - overridden ( current='%s', old='%s %s > %s' ). Fix your wordforms file '%s'.",
							pContainer->m_sIndexName.cstr(), sBuffer, dTokens[0].cstr(), sStoredTokens.cstr(), sStoredForms.cstr(), szFile );

						pStoredMF->m_dNormalForm.Resize ( pMultiWordform->m_dNormalForm.GetLength() );
						ARRAY_FOREACH ( iForm, pMultiWordform->m_dNormalForm )
							pStoredMF->m_dNormalForm[iForm] = pMultiWordform->m_dNormalForm[iForm];

						pStoredMF->m_iFileId = iFileId;

						SafeDelete ( pMultiWordform );
						break; // otherwise, we crash next turn
					}
				}
			}

			if ( pMultiWordform )
			{
				pWordforms->m_pForms.Add ( pMultiWordform );

				// sort forms by files and length
				// but do not sort if we're loading embedded
				if ( iFileId>=0 )
					pWordforms->m_pForms.Sort ( Lesser ( [] ( const CSphMultiform* pA, const CSphMultiform* pB ) {
						assert ( pA && pB );
						return ( pA->m_iFileId == pB->m_iFileId ) ? pA->m_dTokens.GetLength() > pB->m_dTokens.GetLength() : pA->m_iFileId > pB->m_iFileId;
					} ) );

				pWordforms->m_iMinTokens = Min ( pWordforms->m_iMinTokens, pMultiWordform->m_dTokens.GetLength () );
				pWordforms->m_iMaxTokens = Max ( pWordforms->m_iMaxTokens, pMultiWordform->m_dTokens.GetLength () );
				pContainer->m_pMultiWordforms->m_iMaxTokens = Max ( pContainer->m_pMultiWordforms->m_iMaxTokens, pWordforms->m_iMaxTokens );
			}
		} else
		{
			auto * pNewWordforms = new CSphMultiforms;
			pNewWordforms->m_pForms.Add ( pMultiWordform );
			pNewWordforms->m_iMinTokens = pMultiWordform->m_dTokens.GetLength ();
			pNewWordforms->m_iMaxTokens = pMultiWordform->m_dTokens.GetLength ();
			pContainer->m_pMultiWordforms->m_iMaxTokens = Max ( pContainer->m_pMultiWordforms->m_iMaxTokens, pNewWordforms->m_iMaxTokens );
			pContainer->m_pMultiWordforms->m_Hash.Add ( pNewWordforms, dTokens[0] );
		}

		// let's add destination form to regular wordform to keep destination from being stemmed
		// FIXME!!! handle multiple destination tokens and ~flag for wordforms
		if ( !bAfterMorphology && dDestTokens.GetLength()==1 && !pContainer->m_hHash.Exists ( dDestTokens[0].m_sForm ) )
		{
			CSphStoredNF tStoredForm;
			tStoredForm.m_sWord = dDestTokens[0].m_sForm;
			tStoredForm.m_bAfterMorphology = bAfterMorphology;
			pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			if ( !pContainer->m_dNormalForms.GetLength()
				|| pContainer->m_dNormalForms.Last().m_sWord!=dDestTokens[0].m_sForm
				|| pContainer->m_dNormalForms.Last().m_bAfterMorphology!=bAfterMorphology )
				pContainer->m_dNormalForms.Add ( tStoredForm );

			pContainer->m_hHash.Add ( pContainer->m_dNormalForms.GetLength()-1, dDestTokens[0].m_sForm );
		}
	} else
	{
		if ( bAfterMorphology )
		{
			BYTE pBuf [16+3*SPH_MAX_WORD_LEN];
			memcpy ( pBuf, dTokens[0].cstr(), dTokens[0].Length()+1 );
			ApplyStemmers ( pBuf );
			dTokens[0] = (char *)pBuf;
		}

		// check wordform that source token is a new token or has same destination token
		int * pRefTo = pContainer->m_hHash ( dTokens[0] );
		assert ( !pRefTo || ( *pRefTo>=0 && *pRefTo<pContainer->m_dNormalForms.GetLength() ) );
		if ( pRefTo )
		{
			// replace with a new wordform
			if ( pContainer->m_dNormalForms[*pRefTo].m_sWord!=dDestTokens[0].m_sForm || pContainer->m_dNormalForms[*pRefTo].m_bAfterMorphology!=bAfterMorphology )
			{
				CSphStoredNF & tRefTo = pContainer->m_dNormalForms[*pRefTo];
				sphWarning ( "index '%s': duplicate wordform found - overridden ( current='%s', old='%s%s > %s' ). Fix your wordforms file '%s'.",
					pContainer->m_sIndexName.cstr(), sBuffer, tRefTo.m_bAfterMorphology ? "~" : "", dTokens[0].cstr(), tRefTo.m_sWord.cstr(), szFile );

				tRefTo.m_sWord = dDestTokens[0].m_sForm;
				tRefTo.m_bAfterMorphology = bAfterMorphology;
				pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			} else
				sphWarning ( "index '%s': duplicate wordform found ( '%s' ). Fix your wordforms file '%s'.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		} else
		{
			CSphStoredNF tStoredForm;
			tStoredForm.m_sWord = dDestTokens[0].m_sForm;
			tStoredForm.m_bAfterMorphology = bAfterMorphology;
			pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			if ( !pContainer->m_dNormalForms.GetLength()
				|| pContainer->m_dNormalForms.Last().m_sWord!=dDestTokens[0].m_sForm
				|| pContainer->m_dNormalForms.Last().m_bAfterMorphology!=bAfterMorphology)
				pContainer->m_dNormalForms.Add ( tStoredForm );

			pContainer->m_hHash.Add ( pContainer->m_dNormalForms.GetLength()-1, dTokens[0] );
		}
	}
}


CSphWordforms * CSphTemplateDictTraits::LoadWordformContainer ( const CSphVector<CSphSavedFile> & dFileInfos,
	const StrVec_t * pEmbeddedWordforms, const ISphTokenizer * pTokenizer, const char * sIndex )
{
	// allocate it
	auto * pContainer = new CSphWordforms();
	pContainer->m_dFiles = dFileInfos;
	pContainer->m_uTokenizerFNV = pTokenizer->GetSettingsFNV();
	pContainer->m_sIndexName = sIndex;

	TokenizerRefPtr_c pMyTokenizer ( pTokenizer->Clone ( SPH_CLONE_INDEX ) );
	const CSphTokenizerSettings & tSettings = pMyTokenizer->GetSettings();
	CSphVector<int> dBlended;

	// get a list of blend chars and set add them to the tokenizer as simple chars
	if ( tSettings.m_sBlendChars.Length() )
	{
		CSphVector<char> dNewCharset;
		dNewCharset.Append ( tSettings.m_sCaseFolding.cstr (), tSettings.m_sCaseFolding.Length ());

		CSphVector<CSphRemapRange> dRemaps;
		CSphCharsetDefinitionParser tParser;
		if ( tParser.Parse ( tSettings.m_sBlendChars.cstr(), dRemaps ) )
			ARRAY_FOREACH ( i, dRemaps )
				for ( int j = dRemaps[i].m_iStart; j<=dRemaps[i].m_iEnd; j++ )
				{
					dNewCharset.Add ( ',' );
					dNewCharset.Add ( ' ' );
					dNewCharset.Add ( char(j) );
					dBlended.Add ( j );
				}

		dNewCharset.Add(0);

		// sort dBlended for binary search
		dBlended.Sort ();

		CSphString sError;
		pMyTokenizer->SetCaseFolding ( dNewCharset.Begin(), sError );

		// disable blend chars
		pMyTokenizer->SetBlendChars ( NULL, sError );
	}

	// add wordform-specific specials
	pMyTokenizer->AddSpecials ( "#=>~" );

	if ( pEmbeddedWordforms )
	{
		CSphTightVector<CSphString> dFilenames;
		dFilenames.Resize ( dFileInfos.GetLength() );
		ARRAY_FOREACH ( i, dFileInfos )
			dFilenames[i] = dFileInfos[i].m_sFilename;

		CSphString sAllFiles;
		ConcatReportStrings ( dFilenames, sAllFiles );

		for ( auto & sWordForm : (*pEmbeddedWordforms) )
			AddWordform ( pContainer, const_cast<char*>( sWordForm.cstr() ), sWordForm.Length(), pMyTokenizer, sAllFiles.cstr(), dBlended, -1 );
	} else
	{
		char sBuffer [ 6*SPH_MAX_WORD_LEN + 512 ]; // enough to hold 2 UTF-8 words, plus some whitespace overhead

		ARRAY_FOREACH ( i, dFileInfos )
		{
			CSphAutoreader rdWordforms;
			const char * szFile = dFileInfos[i].m_sFilename.cstr();
			CSphString sError;
			if ( !rdWordforms.Open ( szFile, sError ) )
			{
				sphWarning ( "index '%s': %s", sIndex, sError.cstr() );
				SafeDelete ( pContainer );
				return nullptr;
			}

			int iLen;
			while ( ( iLen = rdWordforms.GetLine ( sBuffer, sizeof(sBuffer) ) )>=0 )
				AddWordform ( pContainer, sBuffer, iLen, pMyTokenizer, szFile, dBlended, i );
		}
	}

	return pContainer;
}


bool CSphTemplateDictTraits::LoadWordforms ( const StrVec_t & dFiles, const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex )
{
	if ( pEmbedded )
	{
		m_dWFFileInfos.Resize ( pEmbedded->m_dWordformFiles.GetLength() );
		ARRAY_FOREACH ( i, m_dWFFileInfos )
			m_dWFFileInfos[i] = pEmbedded->m_dWordformFiles[i];
	} else
	{
		m_dWFFileInfos.Reserve ( dFiles.GetLength() );
		CSphSavedFile tFile;
		ARRAY_FOREACH ( i, dFiles )
			if ( !dFiles[i].IsEmpty() )
			{
				if ( tFile.Collect ( dFiles[i].cstr() ) )
					m_dWFFileInfos.Add ( tFile );
				else
					sphWarning ( "index '%s': wordforms file '%s' not found", sIndex, dFiles[i].cstr() );
			}
	}

	if ( !m_dWFFileInfos.GetLength() )
		return false;

	SweepWordformContainers ( m_dWFFileInfos );

	m_pWordforms = GetWordformContainer ( m_dWFFileInfos, pEmbedded ? &(pEmbedded->m_dWordforms) : nullptr, pTokenizer, sIndex );
	if ( m_pWordforms )
	{
		m_pWordforms->m_iRefCount++;
		if ( m_pWordforms->m_bHavePostMorphNF && !m_dMorph.GetLength() )
			sphWarning ( "index '%s': wordforms contain post-morphology normal forms, but no morphology was specified", sIndex );
	}

	return !!m_pWordforms;
}


void CSphTemplateDictTraits::WriteWordforms ( CSphWriter & tWriter ) const
{
	if ( !m_pWordforms )
	{
		tWriter.PutDword(0);
		return;
	}

	int nMultiforms = 0;
	if ( m_pWordforms->m_pMultiWordforms )
		for ( const auto& tMF : m_pWordforms->m_pMultiWordforms->m_Hash )
			nMultiforms += tMF.second ? tMF.second->m_pForms.GetLength() : 0;

	tWriter.PutDword ( m_pWordforms->m_hHash.GetLength()+nMultiforms );
	for ( const auto& tForm : m_pWordforms->m_hHash )
	{
		CSphString sLine;
		sLine.SetSprintf ( "%s%s > %s", m_pWordforms->m_dNormalForms[tForm.second].m_bAfterMorphology ? "~" : "",
			tForm.first.cstr(), m_pWordforms->m_dNormalForms[tForm.second].m_sWord.cstr() );
		tWriter.PutString ( sLine );
	}

	if ( m_pWordforms->m_pMultiWordforms )
	{
		CSphMultiformContainer::CSphMultiformHash & tHash = m_pWordforms->m_pMultiWordforms->m_Hash;
		tHash.IterateStart();
		while ( tHash.IterateNext() )
		{
			const CSphString & sKey = tHash.IterateGetKey();
			CSphMultiforms * pMF = tHash.IterateGet();
			if ( !pMF )
				continue;

			ARRAY_FOREACH ( i, pMF->m_pForms )
			{
				CSphString sLine, sTokens, sForms;
				ConcatReportStrings ( pMF->m_pForms[i]->m_dTokens, sTokens );
				ConcatReportStrings ( pMF->m_pForms[i]->m_dNormalForm, sForms );

				sLine.SetSprintf ( "%s %s > %s", sKey.cstr(), sTokens.cstr(), sForms.cstr() );
				tWriter.PutString ( sLine );
			}
		}
	}
}

void CSphTemplateDictTraits::WriteWordforms ( JsonEscapedBuilder & tOut ) const
{
	if ( !m_pWordforms )
		return;

	bool bHaveData = ( m_pWordforms->m_hHash.GetLength() != 0 );

	using HASHIT = std::pair<CSphString, CSphMultiforms*>;
	auto * pMulti = m_pWordforms->m_pMultiWordforms; // shortcut
	if ( pMulti )
		bHaveData |= ::any_of ( pMulti->m_Hash, [] ( const HASHIT& tMF ) { return tMF.second && !tMF.second->m_pForms.IsEmpty(); } );

	if ( !bHaveData )
		return;

	tOut.Named ( "word_forms" );
	auto _ = tOut.ArrayW();

	if ( m_pWordforms->m_hHash.GetLength() )
		for ( const auto& tForm : m_pWordforms->m_hHash )
		{
			CSphString sLine;
			sLine.SetSprintf ( "%s%s > %s", m_pWordforms->m_dNormalForms[tForm.second].m_bAfterMorphology ? "~" : "", tForm.first.cstr(), m_pWordforms->m_dNormalForms[tForm.second].m_sWord.cstr() );
			tOut.FixupSpacedAndAppendEscaped ( sLine.cstr() );
		}

	if ( !pMulti )
		return;

	for ( const HASHIT& tForms : pMulti->m_Hash )
	{
		if ( !tForms.second )
			continue;

		for ( const CSphMultiform* pMF : tForms.second->m_pForms )
		{
			CSphString sLine, sTokens, sForms;
			ConcatReportStrings ( pMF->m_dTokens, sTokens );
			ConcatReportStrings ( pMF->m_dNormalForm, sForms );
			sLine.SetSprintf ( "%s %s > %s", tForms.first.cstr(), sTokens.cstr(), sForms.cstr() );
			tOut.FixupSpacedAndAppendEscaped ( sLine.cstr() );
		}
	}
}


int CSphTemplateDictTraits::SetMorphology ( const char * szMorph, CSphString & sMessage )
{
	m_dMorph.Reset ();
#if WITH_STEMMER
	ARRAY_FOREACH ( i, m_dStemmers )
		sb_stemmer_delete ( m_dStemmers[i] );
	m_dStemmers.Reset ();
#endif

	if ( !szMorph )
		return ST_OK;

	CSphString sOption = szMorph;
	sOption.ToLower ();

	CSphString sError;
	int iRes = ParseMorphology ( sOption.cstr(), sMessage );
	if ( iRes==ST_WARNING && sMessage.IsEmpty() )
		sMessage.SetSprintf ( "invalid morphology option %s; skipped", sOption.cstr() );
	return iRes;
}


bool CSphTemplateDictTraits::HasMorphology() const
{
	return ( m_dMorph.GetLength()>0 );
}


/// common id-based stemmer
bool CSphTemplateDictTraits::StemById ( BYTE * pWord, int iStemmer ) const
{
	char szBuf [ MAX_KEYWORD_BYTES ];

	// safe quick strncpy without (!) padding and with a side of strlen
	char * p = szBuf;
	char * pMax = szBuf + sizeof(szBuf) - 1;
	BYTE * pLastSBS = NULL;
	while ( *pWord && p<pMax )
	{
		pLastSBS = ( *pWord )<0x80 ? pWord : pLastSBS;
		*p++ = *pWord++;
	}
	int iLen = int ( p - szBuf );
	*p = '\0';
	pWord -= iLen;

	switch ( iStemmer )
	{
	case SPH_MORPH_STEM_EN:
		stem_en ( pWord, iLen );
		break;

	case SPH_MORPH_STEM_RU_UTF8:
		// skip stemming in case of SBC at the end of the word
		if ( pLastSBS && ( pLastSBS-pWord+1 )>=iLen )
			break;

		// stem only UTF8 tail
		if ( !pLastSBS )
		{
			stem_ru_utf8 ( (WORD*)pWord );
		} else
		{
			stem_ru_utf8 ( (WORD *)( pLastSBS+1 ) );
		}
		break;

	case SPH_MORPH_STEM_CZ:
		stem_cz ( pWord );
		break;

	case SPH_MORPH_STEM_AR_UTF8:
		stem_ar_utf8 ( pWord );
		break;

	case SPH_MORPH_SOUNDEX:
		stem_soundex ( pWord );
		break;

	case SPH_MORPH_METAPHONE_UTF8:
		stem_dmetaphone ( pWord );
		break;

	case SPH_MORPH_AOTLEMMER_RU_UTF8:
		sphAotLemmatizeRuUTF8 ( pWord );
		break;

	case SPH_MORPH_AOTLEMMER_EN:
		sphAotLemmatize ( pWord, AOT_EN );
		break;

	case SPH_MORPH_AOTLEMMER_DE_UTF8:
		sphAotLemmatizeDeUTF8 ( pWord );
		break;

	case SPH_MORPH_AOTLEMMER_UK:
		sphAotLemmatizeUk ( pWord, m_tLemmatizer.Ptr() );
		break;

	case SPH_MORPH_AOTLEMMER_RU_ALL:
	case SPH_MORPH_AOTLEMMER_EN_ALL:
	case SPH_MORPH_AOTLEMMER_DE_ALL:
	case SPH_MORPH_AOTLEMMER_UK_ALL:
		// do the real work somewhere else
		// this is mostly for warning suppressing and making some features like
		// index_exact_words=1 vs expand_keywords=1 work
		break;

	default:
#if WITH_STEMMER
		if ( iStemmer>=SPH_MORPH_LIBSTEMMER_FIRST && iStemmer<SPH_MORPH_LIBSTEMMER_LAST )
		{
			sb_stemmer * pStemmer = m_dStemmers [iStemmer - SPH_MORPH_LIBSTEMMER_FIRST];
			assert ( pStemmer );

			const sb_symbol * sStemmed = sb_stemmer_stem ( pStemmer, (sb_symbol*)pWord, (int) strlen ( (const char*)pWord ) );
			int iStemmedLen = sb_stemmer_length ( pStemmer );

			memcpy ( pWord, sStemmed, iStemmedLen );
			pWord[iStemmedLen] = '\0';
		} else
			return false;

	break;
#else
		return false;
#endif
	}

	return strcmp ( (char *)pWord, szBuf )!=0;
}

void CSphDiskDictTraits::DictBegin ( CSphAutofile & , CSphAutofile & tDict, int )
{
	m_wrDict.CloseFile ();
	m_wrDict.SetFile ( tDict, NULL, m_sWriterError );
	m_wrDict.PutByte ( 1 );
}

bool CSphDiskDictTraits::DictEnd ( DictHeader_t * pHeader, int, CSphString & sError )
{
	// flush wordlist checkpoints
	pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos();
	pHeader->m_iDictCheckpoints = m_dCheckpoints.GetLength();
	ARRAY_FOREACH ( i, m_dCheckpoints )
	{
		assert ( m_dCheckpoints[i].m_iWordlistOffset );
		m_wrDict.PutOffset ( m_dCheckpoints[i].m_uWordID );
		m_wrDict.PutOffset ( m_dCheckpoints[i].m_iWordlistOffset );
	}

	// done
	m_wrDict.CloseFile ();
	if ( m_wrDict.IsError() )
		sError = m_sWriterError;
	return !m_wrDict.IsError();
}

void CSphDiskDictTraits::DictEntry ( const CSphDictEntry & tEntry )
{
	assert ( m_iSkiplistBlockSize>0 );

	// insert wordlist checkpoint
	if ( ( m_iEntries % SPH_WORDLIST_CHECKPOINT )==0 )
	{
		if ( m_iEntries ) // but not the 1st entry
		{
			assert ( tEntry.m_iDoclistOffset > m_iLastDoclistPos );
			m_wrDict.ZipInt ( 0 ); // indicate checkpoint
			m_wrDict.ZipOffset ( tEntry.m_iDoclistOffset - m_iLastDoclistPos ); // store last length
		}

		// restart delta coding, once per SPH_WORDLIST_CHECKPOINT entries
		m_iLastWordID = 0;
		m_iLastDoclistPos = 0;

		// begin new wordlist entry
		assert ( m_wrDict.GetPos()<=UINT_MAX );

		CSphWordlistCheckpoint & tCheckpoint = m_dCheckpoints.Add();
		tCheckpoint.m_uWordID = tEntry.m_uWordID;
		tCheckpoint.m_iWordlistOffset = m_wrDict.GetPos();
	}

	assert ( tEntry.m_iDoclistOffset>m_iLastDoclistPos );
	m_wrDict.ZipOffset ( tEntry.m_uWordID - m_iLastWordID ); // FIXME! slow with 32bit wordids
	m_wrDict.ZipOffset ( tEntry.m_iDoclistOffset - m_iLastDoclistPos );

	m_iLastWordID = tEntry.m_uWordID;
	m_iLastDoclistPos = tEntry.m_iDoclistOffset;

	assert ( tEntry.m_iDocs );
	assert ( tEntry.m_iHits );
	m_wrDict.ZipInt ( tEntry.m_iDocs );
	m_wrDict.ZipInt ( tEntry.m_iHits );

	// write skiplist location info, if any
	if ( tEntry.m_iDocs > m_iSkiplistBlockSize )
		m_wrDict.ZipOffset ( tEntry.m_iSkiplistOffset );

	m_iEntries++;
}

void CSphDiskDictTraits::DictEndEntries ( SphOffset_t iDoclistOffset )
{
	assert ( iDoclistOffset>=m_iLastDoclistPos );
	m_wrDict.ZipInt ( 0 ); // indicate checkpoint
	m_wrDict.ZipOffset ( iDoclistOffset - m_iLastDoclistPos ); // store last doclist length
}

//////////////////////////////////////////////////////////////////////////
// KEYWORDS STORING DICTIONARY, INFIX HASH BUILDER
//////////////////////////////////////////////////////////////////////////

template < int SIZE >
struct Infix_t
{
	DWORD m_Data[SIZE];

#ifndef NDEBUG
	BYTE m_TrailingZero { 0 };
#endif

	void Reset ()
	{
		for ( int i=0; i<SIZE; i++ )
			m_Data[i] = 0;
	}

	bool operator == ( const Infix_t<SIZE> & rhs ) const;
};


template<>
bool Infix_t<2>::operator == ( const Infix_t<2> & rhs ) const
{
	return m_Data[0]==rhs.m_Data[0] && m_Data[1]==rhs.m_Data[1];
}


template<>
bool Infix_t<3>::operator == ( const Infix_t<3> & rhs ) const
{
	return m_Data[0]==rhs.m_Data[0] && m_Data[1]==rhs.m_Data[1] && m_Data[2]==rhs.m_Data[2];
}


template<>
bool Infix_t<5>::operator == ( const Infix_t<5> & rhs ) const
{
	return m_Data[0]==rhs.m_Data[0] && m_Data[1]==rhs.m_Data[1] && m_Data[2]==rhs.m_Data[2]
		&& m_Data[3]==rhs.m_Data[3] && m_Data[4]==rhs.m_Data[4];
}


struct InfixIntvec_t
{
public:
	// do not change the order of fields in this union - it matters a lot
	union
	{
		DWORD			m_dData[4];
		struct
		{
			int				m_iDynLen;
			int				m_iDynLimit;
			DWORD *			m_pDynData;
		};
	};

public:
	InfixIntvec_t()
	{
		m_dData[0] = 0;
		m_dData[1] = 0;
		m_dData[2] = 0;
		m_dData[3] = 0;
	}

	~InfixIntvec_t()
	{
		if ( IsDynamic() )
			SafeDeleteArray ( m_pDynData );
	}

	bool IsDynamic() const
	{
		return ( m_dData[0] & 0x80000000UL )!=0;
	}

	void Add ( DWORD uVal )
	{
		if ( !m_dData[0] )
		{
			// empty
			m_dData[0] = uVal | ( 1UL<<24 );

		} else if ( !IsDynamic() )
		{
			// 1..4 static entries
			int iLen = m_dData[0] >> 24;
			DWORD uLast = m_dData [ iLen-1 ] & 0xffffffUL;

			// redundant
			if ( uVal==uLast )
				return;

			// grow static part
			if ( iLen<4 )
			{
				m_dData[iLen] = uVal;
				m_dData[0] = ( m_dData[0] & 0xffffffUL ) | ( ++iLen<<24 );
				return;
			}

			// dynamize
			DWORD * pDyn = new DWORD[16];
			pDyn[0] = m_dData[0] & 0xffffffUL;
			pDyn[1] = m_dData[1];
			pDyn[2] = m_dData[2];
			pDyn[3] = m_dData[3];
			pDyn[4] = uVal;
			m_iDynLen = 0x80000005UL; // dynamic flag, len=5
			m_iDynLimit = 16; // limit=16
			m_pDynData = pDyn;

		} else
		{
			// N dynamic entries
			int iLen = m_iDynLen & 0xffffffUL;
			if ( uVal==m_pDynData[iLen-1] )
				return;
			if ( iLen>=m_iDynLimit )
			{
				m_iDynLimit *= 2;
				DWORD * pNew = new DWORD [ m_iDynLimit ];
				for ( int i=0; i<iLen; i++ )
					pNew[i] = m_pDynData[i];
				SafeDeleteArray ( m_pDynData );
				m_pDynData = pNew;
			}

			m_pDynData[iLen] = uVal;
			m_iDynLen++;
		}
	}

	bool operator == ( const InfixIntvec_t & rhs ) const
	{
		// check dynflag, length, maybe first element
		if ( m_dData[0]!=rhs.m_dData[0] )
			return false;

		// check static data
		if ( !IsDynamic() )
		{
			for ( int i=1; i<(int)(m_dData[0]>>24); i++ )
				if ( m_dData[i]!=rhs.m_dData[i] )
					return false;
			return true;
		}

		// check dynamic data
		const DWORD * a = m_pDynData;
		const DWORD * b = rhs.m_pDynData;
		const DWORD * m = a + ( m_iDynLen & 0xffffffUL );
		while ( a<m )
			if ( *a++!=*b++ )
				return false;
		return true;
	}

public:
	int GetLength() const
	{
		if ( !IsDynamic() )
			return m_dData[0] >> 24;
		return m_iDynLen & 0xffffffUL;
	}

	DWORD operator[] ( int iIndex )const
	{
		if ( !IsDynamic() )
			return m_dData[iIndex] & 0xffffffUL;
		return m_pDynData[iIndex];
	}
};


template < int SIZE >
struct InfixHashEntry_t
{
	Infix_t<SIZE>	m_tKey;		///< key, owned by the hash
	InfixIntvec_t	m_tValue;	///< data, owned by the hash
	int				m_iNext;	///< next entry in hash arena
};


template < int SIZE >
class InfixBuilder_c : public ISphInfixBuilder
{
protected:
	static const int							LENGTH = 1048576;

protected:
	int											m_dHash [ LENGTH ];		///< all the hash entries
	CSphSwapVector < InfixHashEntry_t<SIZE> >	m_dArena;
	CSphVector<InfixBlock_t>					m_dBlocks;
	CSphTightVector<BYTE>						m_dBlocksWords;

public:
				InfixBuilder_c();
	void		AddWord ( const BYTE * pWord, int iWordLength, int iCheckpoint, bool bHasMorphology ) override;
	void		SaveEntries ( CSphWriter & wrDict ) override;
	int64_t		SaveEntryBlocks ( CSphWriter & wrDict ) override;
	int			GetBlocksWordsSize() const override { return m_dBlocksWords.GetLength(); }

protected:
	/// add new entry
	void AddEntry ( const Infix_t<SIZE> & tKey, DWORD uHash, int iCheckpoint )
	{
		uHash &= ( LENGTH-1 );

		int iEntry = m_dArena.GetLength();
		InfixHashEntry_t<SIZE> & tNew = m_dArena.Add();
		tNew.m_tKey = tKey;
		tNew.m_tValue.m_dData[0] = 0x1000000UL | iCheckpoint; // len=1, data=iCheckpoint
		tNew.m_iNext = m_dHash[uHash];
		m_dHash[uHash] = iEntry;
	}

	/// get value pointer by key
	InfixIntvec_t * LookupEntry ( const Infix_t<SIZE> & tKey, DWORD uHash )
	{
		uHash &= ( LENGTH-1 );
		int iEntry = m_dHash [ uHash ];
		int iiEntry = 0;

		while ( iEntry )
		{
			if ( m_dArena[iEntry].m_tKey==tKey )
			{
				// mtf it, if needed
				if ( iiEntry )
				{
					m_dArena[iiEntry].m_iNext = m_dArena[iEntry].m_iNext;
					m_dArena[iEntry].m_iNext = m_dHash[uHash];
					m_dHash[uHash] = iEntry;
				}
				return &m_dArena[iEntry].m_tValue;
			}
			iiEntry = iEntry;
			iEntry = m_dArena[iEntry].m_iNext;
		}
		return NULL;
	}
};


template < int SIZE >
InfixBuilder_c<SIZE>::InfixBuilder_c()
{
	// init the hash
	for ( int i=0; i<LENGTH; i++ )
		m_dHash[i] = 0;
	m_dArena.Reserve ( 1048576 );
	m_dArena.Resize ( 1 ); // 0 is a reserved index
}


/// single-byte case, 2-dword infixes
template<>
void InfixBuilder_c<2>::AddWord ( const BYTE * pWord, int iWordLength, int iCheckpoint, bool bHasMorphology )
{
	if ( bHasMorphology && *pWord!=MAGIC_WORD_HEAD_NONSTEMMED )
		return;

	if ( *pWord<0x20 ) // skip heading magic chars, like NONSTEMMED maker
	{
		pWord++;
		iWordLength--;
	}

	Infix_t<2> sKey;
	for ( int p=0; p<=iWordLength-2; p++ )
	{
		sKey.Reset();

		BYTE * pKey = (BYTE*)sKey.m_Data;
		const BYTE * s = pWord + p;
		const BYTE * sMax = s + Min ( 6, iWordLength-p );

		DWORD uHash = 0xffffffUL ^ g_dSphinxCRC32 [ 0xff ^ *s ];
		*pKey++ = *s++; // copy first infix byte

		while ( s<sMax )
		{
			uHash = (uHash >> 8) ^ g_dSphinxCRC32 [ (uHash ^ *s) & 0xff ];
			*pKey++ = *s++; // copy another infix byte

			InfixIntvec_t * pVal = LookupEntry ( sKey, uHash );
			if ( pVal )
				pVal->Add ( iCheckpoint );
			else
				AddEntry ( sKey, uHash, iCheckpoint );
		}
	}
}

static int Utf8CodeLen ( BYTE iCode )
{
	if ( !iCode )
		return 0;

	// check for 7-bit case
	if ( iCode<128 )
		return 1;

	// get number of bytes
	int iBytes = 0;
	while ( iCode & 0x80 )
	{
		iBytes++;
		iCode <<= 1;
	}

	return iBytes;
}

/// UTF-8 case, 3/5-dword infixes
template < int SIZE >
void InfixBuilder_c<SIZE>::AddWord ( const BYTE * pWord, int iWordLength, int iCheckpoint, bool bHasMorphology )
{
	if ( bHasMorphology && *pWord!=MAGIC_WORD_HEAD_NONSTEMMED )
		return;

	if ( *pWord<0x20 ) // skip heading magic chars, like NONSTEMMED maker
	{
		pWord++;
		iWordLength--;
	}

	const BYTE * pWordMax = pWord+iWordLength;
#ifndef NDEBUG
	bool bInvalidTailCp = false;
#endif
	int iCodes = 0; // codepoints in current word
	BYTE dBytes[SPH_MAX_WORD_LEN+1]; // byte offset for each codepoints

	// build an offsets table into the bytestring
	dBytes[0] = 0;
	for ( const BYTE * p = pWord; p<pWordMax && iCodes<SPH_MAX_WORD_LEN; )
	{
		int iLen = 0;
		BYTE uVal = *p;
		while ( uVal & 0x80 )
		{
			uVal <<= 1;
			iLen++;
		}
		if ( !iLen )
			iLen = 1;
		
		// break on tail cut codepoint
		if ( p+iLen>pWordMax )
		{
#ifndef NDEBUG
			bInvalidTailCp = true;
#endif
			break;
		}

		// skip word with large codepoints
		if ( iLen>SIZE )
			return;

		assert ( iLen>=1 && iLen<=4 );
		p += iLen;

		dBytes[iCodes+1] = dBytes[iCodes] + (BYTE)iLen;
		iCodes++;
	}
	assert ( pWord[dBytes[iCodes]]==0 || iCodes==SPH_MAX_WORD_LEN || bInvalidTailCp );

	// generate infixes
	Infix_t<SIZE> sKey;
	for ( int p=0; p<=iCodes-2; p++ )
	{
		sKey.Reset();
		BYTE * pKey = (BYTE*)sKey.m_Data;
		const BYTE * pKeyMax = pKey+sizeof(sKey.m_Data);

		const BYTE * s = pWord + dBytes[p];
		const BYTE * sMax = pWord + dBytes[ p+Min ( 6, iCodes-p ) ];

		// copy first infix codepoint
		DWORD uHash = 0xffffffffUL;
		do
		{
			uHash = (uHash >> 8) ^ g_dSphinxCRC32 [ (uHash ^ *s) & 0xff ];
			*pKey++ = *s++;
		} while ( ( *s & 0xC0 )==0x80 );

		assert ( s - ( pWord + dBytes[p] )==(dBytes[p+1] - dBytes[p]) );

		while ( s<sMax && pKey<pKeyMax && pKey+Utf8CodeLen ( *s )<=pKeyMax )
		{
			// copy next infix codepoint
			do
			{
				uHash = (uHash >> 8) ^ g_dSphinxCRC32 [ (uHash ^ *s) & 0xff ];
				*pKey++ = *s++;
			} while ( ( *s & 0xC0 )==0x80 && pKey<pKeyMax );

			assert ( sphUTF8Len ( (const char *)sKey.m_Data, sizeof(sKey.m_Data) )>=2 );

			InfixIntvec_t * pVal = LookupEntry ( sKey, uHash );
			if ( pVal )
				pVal->Add ( iCheckpoint );
			else
				AddEntry ( sKey, uHash, iCheckpoint );
		}

		assert ( (size_t)( pKey-(BYTE*)sKey.m_Data )<=int(sizeof(sKey.m_Data)) );
	}
}


template < int SIZE >
struct InfixHashCmp_fn
{
	InfixHashEntry_t<SIZE> * m_pBase;

	explicit InfixHashCmp_fn ( InfixHashEntry_t<SIZE> * pBase )
		: m_pBase ( pBase )
	{}

	bool IsLess ( int a, int b ) const
	{
		return strncmp ( (const char*)m_pBase[a].m_tKey.m_Data, (const char*)m_pBase[b].m_tKey.m_Data, sizeof(DWORD)*SIZE )<0;
	}
};


static inline int ZippedIntSize ( DWORD v )
{
	if ( v < (1UL<<7) )
		return 1;
	if ( v < (1UL<<14) )
		return 2;
	if ( v < (1UL<<21) )
		return 3;
	if ( v < (1UL<<28) )
		return 4;
	return 5;
}


const char * g_sTagInfixEntries = "infix-entries";

template < int SIZE >
void InfixBuilder_c<SIZE>::SaveEntries ( CSphWriter & wrDict )
{
	// intentionally local to this function
	// we mark the block end with an editcode of 0
	const int INFIX_BLOCK_SIZE = 64;

	wrDict.PutBytes ( g_sTagInfixEntries, strlen ( g_sTagInfixEntries ) );

	CSphVector<int> dIndex;
	dIndex.Resize ( m_dArena.GetLength()-1 );
	for ( int i=0; i<m_dArena.GetLength()-1; i++ )
		dIndex[i] = i+1;

	InfixHashCmp_fn<SIZE> fnCmp ( m_dArena.Begin() );
	dIndex.Sort ( fnCmp );

	m_dBlocksWords.Reserve ( m_dArena.GetLength()/INFIX_BLOCK_SIZE*sizeof(DWORD)*SIZE );
	int iBlock = 0;
	int iPrevKey = -1;
	ARRAY_FOREACH ( iIndex, dIndex )
	{
		InfixIntvec_t & dData = m_dArena[dIndex[iIndex]].m_tValue;
		const BYTE * sKey = (const BYTE*) m_dArena[dIndex[iIndex]].m_tKey.m_Data;
		int iChars = ( SIZE==2 )
			? (int) strnlen ( (const char*)sKey, sizeof(DWORD)*SIZE )
			: sphUTF8Len ( (const char*)sKey, (int) sizeof(DWORD)*SIZE );
		assert ( iChars>=2 && iChars<int(1 + sizeof ( Infix_t<SIZE> ) ) );

		// keep track of N-infix blocks
		auto iAppendBytes = (int) strnlen ( (const char*)sKey, sizeof(DWORD)*SIZE );
		if ( !iBlock )
		{
			int iOff = m_dBlocksWords.GetLength();
			m_dBlocksWords.Resize ( iOff+iAppendBytes+1 );

			InfixBlock_t & tBlock = m_dBlocks.Add();
			tBlock.m_iInfixOffset = iOff;
			tBlock.m_iOffset = (DWORD)wrDict.GetPos();

			memcpy ( m_dBlocksWords.Begin()+iOff, sKey, iAppendBytes );
			m_dBlocksWords[iOff+iAppendBytes] = '\0';
		}

		// compute max common prefix
		// edit_code = ( num_keep_chars<<4 ) + num_append_chars
		int iEditCode = iChars;
		if ( iPrevKey>=0 )
		{
			const BYTE * sPrev = (const BYTE*) m_dArena[dIndex[iPrevKey]].m_tKey.m_Data;
			const BYTE * sCur = (const BYTE*) sKey;
			const BYTE * sMax = sCur + iAppendBytes;

			int iKeepChars = 0;
			if_const ( SIZE==2 )
			{
				// SBCS path
				while ( sCur<sMax && *sCur && *sCur==*sPrev )
				{
					sCur++;
					sPrev++;
				}
				iKeepChars = (int)( sCur- ( const BYTE* ) sKey );

				assert ( iKeepChars>=0 && iKeepChars<16 );
				assert ( iChars-iKeepChars>=0 );
				assert ( iChars-iKeepChars<16 );

				iEditCode = ( iKeepChars<<4 ) + ( iChars-iKeepChars );
				iAppendBytes = ( iChars-iKeepChars );
				sKey = sCur;

			} else
			{
				// UTF-8 path
				const BYTE * sKeyMax = sCur; // track max matching sPrev prefix in [sKey,sKeyMax)
				while ( sCur<sMax && *sCur && *sCur==*sPrev )
				{
					// current byte matches, move the pointer
					sCur++;
					sPrev++;

					// tricky bit
					// if the next (!) byte is a valid UTF-8 char start (or eof!)
					// then we just matched not just a byte, but a full char
					// so bump the matching prefix boundary and length
					if ( sCur>=sMax || ( *sCur & 0xC0 )!=0x80 )
					{
						sKeyMax = sCur;
						iKeepChars++;
					}
				}

				assert ( iKeepChars>=0 && iKeepChars<16 );
				assert ( iChars-iKeepChars>=0 );
				assert ( iChars-iKeepChars<16 );

				iEditCode = ( iKeepChars<<4 ) + ( iChars-iKeepChars );
				iAppendBytes -= (int)( sKeyMax-sKey );
				sKey = sKeyMax;
			}
		}

		// write edit code, postfix
		wrDict.PutByte ( (BYTE)iEditCode );
		wrDict.PutBytes ( sKey, iAppendBytes );

		// compute data length
		int iDataLen = ZippedIntSize ( dData[0] );
		for ( int j=1; j<dData.GetLength(); j++ )
			iDataLen += ZippedIntSize ( dData[j] - dData[j-1] );

		// write data length, data
		wrDict.ZipInt ( iDataLen );
		wrDict.ZipInt ( dData[0] );
		for ( int j=1; j<dData.GetLength(); j++ )
			wrDict.ZipInt ( dData[j] - dData[j-1] );

		// mark block end, restart deltas
		iPrevKey = iIndex;
		if ( ++iBlock==INFIX_BLOCK_SIZE )
		{
			iBlock = 0;
			iPrevKey = -1;
			wrDict.PutByte ( 0 );
		}
	}

	// put end marker
	if ( iBlock )
		wrDict.PutByte ( 0 );

	const char * pBlockWords = (const char *)m_dBlocksWords.Begin();
	ARRAY_FOREACH ( i, m_dBlocks )
		m_dBlocks[i].m_sInfix = pBlockWords+m_dBlocks[i].m_iInfixOffset;

	if ( wrDict.GetPos()>UINT_MAX ) // FIXME!!! change to int64
		sphDie ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at infix save", wrDict.GetPos() );
}


const char * g_sTagInfixBlocks = "infix-blocks";

template < int SIZE >
int64_t InfixBuilder_c<SIZE>::SaveEntryBlocks ( CSphWriter & wrDict )
{
	// save the blocks
	wrDict.PutBytes ( g_sTagInfixBlocks, strlen ( g_sTagInfixBlocks ) );

	SphOffset_t iInfixBlocksOffset = wrDict.GetPos();
	assert ( iInfixBlocksOffset<=INT_MAX );

	wrDict.ZipInt ( m_dBlocks.GetLength() );
	ARRAY_FOREACH ( i, m_dBlocks )
	{
		auto iBytes = strlen ( m_dBlocks[i].m_sInfix );
		wrDict.PutByte ( BYTE(iBytes) );
		wrDict.PutBytes ( m_dBlocks[i].m_sInfix, iBytes );
		wrDict.ZipInt ( m_dBlocks[i].m_iOffset ); // maybe delta these on top?
	}

	return iInfixBlocksOffset;
}


ISphInfixBuilder * sphCreateInfixBuilder ( int iCodepointBytes, CSphString * pError )
{
	assert ( pError );
	*pError = CSphString();
	switch ( iCodepointBytes )
	{
	case 0:		return NULL;
	case 1:		return new InfixBuilder_c<2>(); // upto 6x1 bytes, 2 dwords, sbcs
	case 2:		return new InfixBuilder_c<3>(); // upto 6x2 bytes, 3 dwords, utf-8
	case 3:		return new InfixBuilder_c<5>(); // upto 6x3 bytes, 5 dwords, utf-8
	default:	pError->SetSprintf ( "unhandled max infix codepoint size %d", iCodepointBytes ); return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// KEYWORDS STORING DICTIONARY
//////////////////////////////////////////////////////////////////////////

class CSphDictKeywords final : public CSphDictCRC<true>
{
public:
	// OPTIMIZE? change pointers to 8:24 locators to save RAM on x64 gear?
	struct HitblockKeyword_t
	{
		SphWordID_t					m_uWordid;			// locally unique word id (crc value, adjusted in case of collsion)
		HitblockKeyword_t *			m_pNextHash;		// next hashed entry
		char *						m_pKeyword;			// keyword
	};

	struct HitblockException_t
	{
		HitblockKeyword_t *			m_pEntry;			// hash entry
		SphWordID_t					m_uCRC;				// original unadjusted crc

		bool operator < ( const HitblockException_t & rhs ) const
		{
			return m_pEntry->m_uWordid < rhs.m_pEntry->m_uWordid;
		}
	};

	struct DictKeyword_t
	{
		char *						m_sKeyword;
		SphOffset_t					m_uOff;
		int							m_iDocs;
		int							m_iHits;
		BYTE						m_uHint;
		int64_t						m_iSkiplistPos;		///< position in .spe file
	};

	struct DictBlock_t
	{
		SphOffset_t					m_iPos;
		int							m_iLen;
	};

public:
	explicit		CSphDictKeywords ();

	void			HitblockBegin () final { m_bHitblock = true; }
	void			HitblockPatch ( CSphWordHit * pHits, int iHits ) const final;
	const char *	HitblockGetKeyword ( SphWordID_t uWordID ) final;
	int				HitblockGetMemUse () final { return m_iMemUse; }
	void			HitblockReset () final;

	void			DictBegin ( CSphAutofile & tTempDict, CSphAutofile & tDict, int iDictLimit ) final;
	void			DictEntry ( const CSphDictEntry & tEntry ) final;
	void			DictEndEntries ( SphOffset_t ) final {}
	bool			DictEnd ( DictHeader_t * pHeader, int iMemLimit, CSphString & sError ) final;

	SphWordID_t		GetWordID ( BYTE * pWord ) final;
	SphWordID_t		GetWordIDWithMarkers ( BYTE * pWord ) final;
	SphWordID_t		GetWordIDNonStemmed ( BYTE * pWord ) final;
	SphWordID_t		GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) final;
	CSphDict *		Clone () const final { return CloneBase ( new CSphDictKeywords() ); }

protected:
					~CSphDictKeywords () final;

private:
	struct DictKeywordTagged_t : public DictKeyword_t
	{
		int m_iBlock;

		static inline bool IsLess ( const DictKeywordTagged_t & a, const DictKeywordTagged_t & b );
	};

	static const int				SLOTS			= 65536;
	static const int				ENTRY_CHUNK		= 65536;
	static const int				KEYWORD_CHUNK	= 1048576;
	static const int				DICT_CHUNK		= 65536;

	HitblockKeyword_t *				m_dHash [ SLOTS ];	///< hash by wordid (!)
	CSphVector<HitblockException_t>	m_dExceptions;

	bool							m_bHitblock;		///< should we store words on GetWordID or not
	int								m_iMemUse;			///< current memory use by all the chunks
	int								m_iDictLimit;		///< allowed memory limit for dict block collection

	CSphVector<HitblockKeyword_t*>	m_dEntryChunks;		///< hash chunks, only used when indexing hitblocks
	HitblockKeyword_t *				m_pEntryChunk;
	int								m_iEntryChunkFree;

	CSphVector<BYTE*>				m_dKeywordChunks;	///< keyword storage
	BYTE *							m_pKeywordChunk;
	int								m_iKeywordChunkFree;

	CSphVector<DictKeyword_t*>		m_dDictChunks;		///< dict entry chunks, only used when sorting final dict
	DictKeyword_t *					m_pDictChunk;
	int								m_iDictChunkFree;

	int								m_iTmpFD;			///< temp dict file descriptor
	CSphWriter						m_wrTmpDict;		///< temp dict writer
	CSphVector<DictBlock_t>			m_dDictBlocks;		///< on-disk locations of dict entry blocks

	char							m_sClippedWord[MAX_KEYWORD_BYTES]; ///< keyword storage for cliiped word

private:
	SphWordID_t				HitblockGetID ( const char * pWord, int iLen, SphWordID_t uCRC );
	HitblockKeyword_t *		HitblockAddKeyword ( DWORD uHash, const char * pWord, int iLen, SphWordID_t uID );

	void					DictReadEntry ( CSphBin & dBin, DictKeywordTagged_t & tEntry, BYTE * pKeyword );
	void					DictFlush ();
};

//////////////////////////////////////////////////////////////////////////

CSphDictKeywords::CSphDictKeywords ()
	: m_bHitblock ( false )
	, m_iMemUse ( 0 )
	, m_iDictLimit ( 0 )
	, m_pEntryChunk ( NULL )
	, m_iEntryChunkFree ( 0 )
	, m_pKeywordChunk ( NULL )
	, m_iKeywordChunkFree ( 0 )
	, m_pDictChunk ( NULL )
	, m_iDictChunkFree ( 0 )
	, m_iTmpFD ( -1 )
{
	memset ( m_dHash, 0, sizeof(m_dHash) );
}

CSphDictKeywords::~CSphDictKeywords ()
{
	HitblockReset();
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

	memset ( m_dHash, 0, sizeof(m_dHash) );
}

CSphDictKeywords::HitblockKeyword_t * CSphDictKeywords::HitblockAddKeyword ( DWORD uHash, const char * sWord, int iLen, SphWordID_t uID )
{
	assert ( iLen<MAX_KEYWORD_BYTES );

	// alloc entry
	if ( !m_iEntryChunkFree )
	{
		m_pEntryChunk = new HitblockKeyword_t [ ENTRY_CHUNK ];
		m_iEntryChunkFree = ENTRY_CHUNK;
		m_dEntryChunks.Add ( m_pEntryChunk );
		m_iMemUse += sizeof(HitblockKeyword_t)*ENTRY_CHUNK;
	}
	HitblockKeyword_t * pEntry = m_pEntryChunk++;
	m_iEntryChunkFree--;

	// alloc keyword
	iLen++;
	if ( m_iKeywordChunkFree < iLen )
	{
		m_pKeywordChunk = new BYTE [ KEYWORD_CHUNK ];
		m_iKeywordChunkFree = KEYWORD_CHUNK;
		m_dKeywordChunks.Add ( m_pKeywordChunk );
		m_iMemUse += KEYWORD_CHUNK;
	}

	// fill it
	memcpy ( m_pKeywordChunk, sWord, iLen );
	m_pKeywordChunk[iLen-1] = '\0';
	pEntry->m_pKeyword = (char*)m_pKeywordChunk;
	pEntry->m_uWordid = uID;
	m_pKeywordChunk += iLen;
	m_iKeywordChunkFree -= iLen;

	// mtf it
	pEntry->m_pNextHash = m_dHash [ uHash ];
	m_dHash [ uHash ] = pEntry;

	return pEntry;
}

SphWordID_t CSphDictKeywords::HitblockGetID ( const char * sWord, int iLen, SphWordID_t uCRC )
{
	if ( iLen>MAX_KEYWORD_BYTES-4 ) // fix of very long word (zones)
	{
		memcpy ( m_sClippedWord, sWord, MAX_KEYWORD_BYTES-4 );
		memset ( m_sClippedWord+MAX_KEYWORD_BYTES-4, 0, 4 );

		CSphString sOrig;
		sOrig.SetBinary ( sWord, iLen );
		sphWarn ( "word overrun buffer, clipped!!!\n"
			"clipped (len=%d, word='%s')\noriginal (len=%d, word='%s')",
			MAX_KEYWORD_BYTES-4, m_sClippedWord, iLen, sOrig.cstr() );

		sWord = m_sClippedWord;
		iLen = MAX_KEYWORD_BYTES-4;
		uCRC = sphCRC32 ( m_sClippedWord, MAX_KEYWORD_BYTES-4 );
	}

	// is this a known one? find it
	// OPTIMIZE? in theory we could use something faster than crc32; but quick lookup3 test did not show any improvements
	const DWORD uHash = (DWORD)( uCRC % SLOTS );

	HitblockKeyword_t * pEntry = m_dHash [ uHash ];
	HitblockKeyword_t ** ppEntry = &m_dHash [ uHash ];
	while ( pEntry )
	{
		// check crc
		if ( pEntry->m_uWordid!=uCRC )
		{
			// crc mismatch, try next entry
			ppEntry = &pEntry->m_pNextHash;
			pEntry = pEntry->m_pNextHash;
			continue;
		}

		// crc matches, check keyword
		register int iWordLen = iLen;
		register const char * a = pEntry->m_pKeyword;
		register const char * b = sWord;
		while ( *a==*b && iWordLen-- )
		{
			if ( !*a || !iWordLen )
			{
				// known word, mtf it, and return id
				(*ppEntry) = pEntry->m_pNextHash;
				pEntry->m_pNextHash = m_dHash [ uHash ];
				m_dHash [ uHash ] = pEntry;
				return pEntry->m_uWordid;
			}
			a++;
			b++;
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
			const HitblockKeyword_t * pExcWord = m_dExceptions[i].m_pEntry;

			// incoming word is a known exception? just return the pre-assigned wordid
			if ( m_dExceptions[i].m_uCRC==uCRC && strncmp ( pExcWord->m_pKeyword, sWord, iLen )==0 )
				return pExcWord->m_uWordid;

			// incoming word collided into a known exception? clear the matched entry; no need to re-add it (see below)
			if ( pExcWord==pEntry )
				pEntry = NULL;

			// find first exception with wordid greater or equal to our candidate
			if ( pExcWord->m_uWordid>=uWordid && iExc==iExcLen )
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
		while (true)
		{
			// iExc must be either the first exception greater or equal to current candidate, or out of bounds
			assert ( iExc==iExcLen || m_dExceptions[iExc].m_pEntry->m_uWordid>=uWordid );
			assert ( iExc==0 || m_dExceptions[iExc-1].m_pEntry->m_uWordid<uWordid );

			// candidate collides with a known exception? increment it, and keep looking
			if ( iExc<iExcLen && m_dExceptions[iExc].m_pEntry->m_uWordid==uWordid )
			{
				uWordid++;
				while ( iExc<iExcLen && m_dExceptions[iExc].m_pEntry->m_uWordid<uWordid )
					iExc++;
				continue;
			}

			// candidate collides with a keyword? must be a regular one; add it as an exception, and keep looking
			HitblockKeyword_t * pCheck = m_dHash [ (DWORD)( uWordid % SLOTS ) ];
			while ( pCheck )
			{
				if ( pCheck->m_uWordid==uWordid )
					break;
				pCheck = pCheck->m_pNextHash;
			}

			// no collisions; we've found our unique wordid!
			if ( !pCheck )
				break;

			// got a collision; add it
			HitblockException_t & tColl = m_dExceptions.Add();
			tColl.m_pEntry = pCheck;
			tColl.m_uCRC = pCheck->m_uWordid; // not a known exception; hence, wordid must equal crc

			// and keep looking
			uWordid++;
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


inline bool CSphDictKeywords::DictKeywordTagged_t::IsLess ( const DictKeywordTagged_t & a, const DictKeywordTagged_t & b )
{
	return strcmp ( a.m_sKeyword, b.m_sKeyword ) < 0;
};


void CSphDictKeywords::DictReadEntry ( CSphBin & dBin, DictKeywordTagged_t & tEntry, BYTE * pKeyword )
{
	int iKeywordLen = dBin.ReadByte ();
	if ( iKeywordLen<0 )
	{
		// early eof or read error; flag must be raised
		assert ( dBin.IsError() );
		return;
	}

	assert ( iKeywordLen>0 && iKeywordLen<MAX_KEYWORD_BYTES-1 );
	if ( dBin.ReadBytes ( pKeyword, iKeywordLen )!=BIN_READ_OK )
	{
		assert ( dBin.IsError() );
		return;
	}
	pKeyword[iKeywordLen] = '\0';

	assert ( m_iSkiplistBlockSize>0 );

	tEntry.m_sKeyword = (char*)pKeyword;
	tEntry.m_uOff = dBin.UnzipOffset();
	tEntry.m_iDocs = dBin.UnzipInt();
	tEntry.m_iHits = dBin.UnzipInt();
	tEntry.m_uHint = (BYTE) dBin.ReadByte();
	if ( tEntry.m_iDocs > m_iSkiplistBlockSize )
		tEntry.m_iSkiplistPos = dBin.UnzipOffset();
	else
		tEntry.m_iSkiplistPos = 0;
}


void CSphDictKeywords::DictBegin ( CSphAutofile & tTempDict, CSphAutofile & tDict, int iDictLimit )
{
	m_iTmpFD = tTempDict.GetFD();
	m_wrTmpDict.CloseFile ();
	m_wrTmpDict.SetFile ( tTempDict, NULL, m_sWriterError );

	m_wrDict.CloseFile ();
	m_wrDict.SetFile ( tDict, NULL, m_sWriterError );
	m_wrDict.PutByte ( 1 );

	m_iDictLimit = Max ( iDictLimit, KEYWORD_CHUNK + DICT_CHUNK*(int)sizeof(DictKeyword_t) ); // can't use less than 1 chunk
}


bool CSphDictKeywords::DictEnd ( DictHeader_t * pHeader, int iMemLimit, CSphString & sError )
{
	DictFlush ();
	m_wrTmpDict.CloseFile (); // tricky: file is not owned, so it won't get closed, and iTmpFD won't get invalidated

	if ( m_dDictBlocks.IsEmpty() )
		m_wrDict.CloseFile();

	if ( m_wrTmpDict.IsError() || m_wrDict.IsError() )
	{
		sError.SetSprintf ( "dictionary write error (out of space?)" );
		return false;
	}

	if ( m_dDictBlocks.IsEmpty() )
	{
		pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos ();
		pHeader->m_iDictCheckpoints = 0;
		return true;
	}

	// infix builder, if needed
	CSphScopedPtr<ISphInfixBuilder> pInfixer { sphCreateInfixBuilder ( pHeader->m_iInfixCodepointBytes, &sError ) };
	if ( !sError.IsEmpty() )
		return false;

	assert ( m_iSkiplistBlockSize>0 );

	// initialize readers
	RawVector_T<CSphBin> dBins;
	dBins.Reserve_static ( m_dDictBlocks.GetLength() );

	int iMaxBlock = 0;
	ARRAY_FOREACH ( i, m_dDictBlocks )
		iMaxBlock = Max ( iMaxBlock, m_dDictBlocks[i].m_iLen );

	iMemLimit = Max ( iMemLimit, iMaxBlock*m_dDictBlocks.GetLength() );
	int iBinSize = CSphBin::CalcBinSize ( iMemLimit, m_dDictBlocks.GetLength(), "sort_dict" );

	SphOffset_t iSharedOffset = -1;
	ARRAY_FOREACH ( i, m_dDictBlocks )
	{
		auto& dBin = dBins.Add();
		dBin.m_iFileLeft = m_dDictBlocks[i].m_iLen;
		dBin.m_iFilePos = m_dDictBlocks[i].m_iPos;
		dBin.Init ( m_iTmpFD, &iSharedOffset, iBinSize );
	}

	// keywords storage
	CSphFixedVector<BYTE> dKeywords { MAX_KEYWORD_BYTES * dBins.GetLength() };
	BYTE* pKeywords = dKeywords.begin();

	// do the sort
	CSphQueue < DictKeywordTagged_t, DictKeywordTagged_t > qWords ( dBins.GetLength() );
	DictKeywordTagged_t tEntry;

	ARRAY_FOREACH ( i, dBins )
	{
		DictReadEntry ( dBins[i], tEntry, pKeywords + i*MAX_KEYWORD_BYTES );
		if ( dBins[i].IsError() )
		{
			sError.SetSprintf ( "entry read error in dictionary sort (bin %d of %d)", i, dBins.GetLength() );
			return false;
		}

		tEntry.m_iBlock = i;
		qWords.Push ( tEntry );
	}

	bool bHasMorphology = HasMorphology();
	CSphKeywordDeltaWriter tLastKeyword;
	int iWords = 0;
	while ( qWords.GetLength() )
	{
		const DictKeywordTagged_t & tWord = qWords.Root();
		auto iLen = (const int) strlen ( tWord.m_sKeyword ); // OPTIMIZE?

		// store checkpoints as needed
		if ( ( iWords % SPH_WORDLIST_CHECKPOINT )==0 )
		{
			// emit a checkpoint, unless we're at the very dict beginning
			if ( iWords )
			{
				m_wrDict.ZipInt ( 0 );
				m_wrDict.ZipInt ( 0 );
			}

			BYTE * sClone = new BYTE [ iLen+1 ]; // OPTIMIZE? pool these?
			memcpy ( sClone, tWord.m_sKeyword, iLen );
			sClone[iLen] = '\0';

			CSphWordlistCheckpoint & tCheckpoint = m_dCheckpoints.Add ();
			tCheckpoint.m_sWord = (char*) sClone;
			tCheckpoint.m_iWordlistOffset = m_wrDict.GetPos();

			tLastKeyword.Reset();
		}
		iWords++;

		// write final dict entry
		assert ( iLen );
		assert ( tWord.m_uOff );
		assert ( tWord.m_iDocs );
		assert ( tWord.m_iHits );

		tLastKeyword.PutDelta ( m_wrDict, (const BYTE *)tWord.m_sKeyword, iLen );
		m_wrDict.ZipOffset ( tWord.m_uOff );
		m_wrDict.ZipInt ( tWord.m_iDocs );
		m_wrDict.ZipInt ( tWord.m_iHits );
		if ( tWord.m_uHint )
			m_wrDict.PutByte ( tWord.m_uHint );
		if ( tWord.m_iDocs > m_iSkiplistBlockSize )
			m_wrDict.ZipOffset ( tWord.m_iSkiplistPos );

		// build infixes
		if ( pInfixer )
			pInfixer->AddWord ( (const BYTE*)tWord.m_sKeyword, iLen, m_dCheckpoints.GetLength(), bHasMorphology );

		// next
		int iBin = tWord.m_iBlock;
		qWords.Pop ();

		if ( !dBins[iBin].IsDone() )
		{
			DictReadEntry ( dBins[iBin], tEntry, pKeywords + iBin*MAX_KEYWORD_BYTES );
			if ( dBins[iBin].IsError() )
			{
				sError.SetSprintf ( "entry read error in dictionary sort (bin %d of %d)", iBin, dBins.GetLength() );
				return false;
			}

			tEntry.m_iBlock = iBin;
			qWords.Push ( tEntry );
		}
	}

	// end of dictionary block
	m_wrDict.ZipInt ( 0 );
	m_wrDict.ZipInt ( 0 );

	// flush infix hash entries, if any
	if ( pInfixer )
		pInfixer->SaveEntries ( m_wrDict );

	// flush wordlist checkpoints (blocks)
	pHeader->m_iDictCheckpointsOffset = m_wrDict.GetPos();
	pHeader->m_iDictCheckpoints = m_dCheckpoints.GetLength();

	ARRAY_FOREACH ( i, m_dCheckpoints )
	{
		auto iLen = (const int) strlen ( m_dCheckpoints[i].m_sWord );

		assert ( m_dCheckpoints[i].m_iWordlistOffset>0 );
		assert ( iLen>0 && iLen<MAX_KEYWORD_BYTES );

		m_wrDict.PutDword ( iLen );
		m_wrDict.PutBytes ( m_dCheckpoints[i].m_sWord, iLen );
		m_wrDict.PutOffset ( m_dCheckpoints[i].m_iWordlistOffset );

		SafeDeleteArray ( m_dCheckpoints[i].m_sWord );
	}

	// flush infix hash blocks
	if ( pInfixer )
	{
		pHeader->m_iInfixBlocksOffset = pInfixer->SaveEntryBlocks ( m_wrDict );
		pHeader->m_iInfixBlocksWordsSize = pInfixer->GetBlocksWordsSize();
		if ( pHeader->m_iInfixBlocksOffset>UINT_MAX ) // FIXME!!! change to int64
			sphDie ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at dictend save", pHeader->m_iInfixBlocksOffset );
	}

	// flush header
	// mostly for debugging convenience
	// primary storage is in the index wide header
	m_wrDict.PutBytes ( "dict-header", 11 );
	m_wrDict.ZipInt ( pHeader->m_iDictCheckpoints );
	m_wrDict.ZipOffset ( pHeader->m_iDictCheckpointsOffset );
	m_wrDict.ZipInt ( pHeader->m_iInfixCodepointBytes );
	m_wrDict.ZipInt ( (DWORD)pHeader->m_iInfixBlocksOffset );

	// about it
	m_wrDict.CloseFile ();
	if ( m_wrDict.IsError() )
		sError.SetSprintf ( "dictionary write error (out of space?)" );
	return !m_wrDict.IsError();
}

struct DictKeywordCmp_fn
{
	inline bool IsLess ( CSphDictKeywords::DictKeyword_t * a, CSphDictKeywords::DictKeyword_t * b ) const
	{
		return strcmp ( a->m_sKeyword, b->m_sKeyword ) < 0;
	}
};

void CSphDictKeywords::DictFlush ()
{
	if ( !m_dDictChunks.GetLength() )
		return;

	assert ( m_dDictChunks.GetLength() && m_dKeywordChunks.GetLength() );
	assert ( m_iSkiplistBlockSize>0 );

	// sort em
	int iTotalWords = m_dDictChunks.GetLength()*DICT_CHUNK - m_iDictChunkFree;
	CSphVector<DictKeyword_t*> dWords ( iTotalWords );

	int iIdx = 0;
	ARRAY_FOREACH ( i, m_dDictChunks )
	{
		int iWords = DICT_CHUNK;
		if ( i==m_dDictChunks.GetLength()-1 )
			iWords -= m_iDictChunkFree;

		DictKeyword_t * pWord = m_dDictChunks[i];
		for ( int j=0; j<iWords; j++ )
			dWords[iIdx++] = pWord++;
	}

	dWords.Sort ( DictKeywordCmp_fn() );

	// write em
	DictBlock_t & tBlock = m_dDictBlocks.Add();
	tBlock.m_iPos = m_wrTmpDict.GetPos ();

	ARRAY_FOREACH ( i, dWords )
	{
		const DictKeyword_t * pWord = dWords[i];
		auto iLen = (int) strlen ( pWord->m_sKeyword );
		m_wrTmpDict.PutByte ( (BYTE)iLen );
		m_wrTmpDict.PutBytes ( pWord->m_sKeyword, iLen );
		m_wrTmpDict.ZipOffset ( pWord->m_uOff );
		m_wrTmpDict.ZipInt ( pWord->m_iDocs );
		m_wrTmpDict.ZipInt ( pWord->m_iHits );
		m_wrTmpDict.PutByte ( pWord->m_uHint );
		assert ( ( pWord->m_iDocs > m_iSkiplistBlockSize )==( pWord->m_iSkiplistPos!=0 ) );
		if ( pWord->m_iDocs > m_iSkiplistBlockSize )
			m_wrTmpDict.ZipOffset ( pWord->m_iSkiplistPos );
	}

	tBlock.m_iLen = (int)( m_wrTmpDict.GetPos() - tBlock.m_iPos );

	// clean up buffers
	ARRAY_FOREACH ( i, m_dDictChunks )
		SafeDeleteArray ( m_dDictChunks[i] );
	m_dDictChunks.Resize ( 0 );
	m_pDictChunk = NULL;
	m_iDictChunkFree = 0;

	ARRAY_FOREACH ( i, m_dKeywordChunks )
		SafeDeleteArray ( m_dKeywordChunks[i] );
	m_dKeywordChunks.Resize ( 0 );
	m_pKeywordChunk = NULL;
	m_iKeywordChunkFree = 0;

	m_iMemUse = 0;
}

void CSphDictKeywords::DictEntry ( const CSphDictEntry & tEntry )
{
	// they say, this might just happen during merge
	// FIXME! can we make merge avoid sending such keywords to dict and assert here?
	if ( !tEntry.m_iDocs )
		return;

	assert ( tEntry.m_iHits );
	assert ( tEntry.m_iDoclistLength>0 );
	assert ( m_iSkiplistBlockSize>0 );

	DictKeyword_t * pWord = NULL;
	auto iLen = (int) strlen ( (const char*)tEntry.m_sKeyword ) + 1;

	while (true)
	{
		// alloc dict entry
		if ( !m_iDictChunkFree )
		{
			if ( m_iDictLimit && ( m_iMemUse + (int)sizeof(DictKeyword_t)*DICT_CHUNK )>m_iDictLimit )
				DictFlush ();

			m_pDictChunk = new DictKeyword_t [ DICT_CHUNK ];
			m_iDictChunkFree = DICT_CHUNK;
			m_dDictChunks.Add ( m_pDictChunk );
			m_iMemUse += sizeof(DictKeyword_t)*DICT_CHUNK;
		}

		// alloc keyword
		if ( m_iKeywordChunkFree < iLen )
		{
			if ( m_iDictLimit && ( m_iMemUse + KEYWORD_CHUNK )>m_iDictLimit )
			{
				DictFlush ();
				continue; // because we just flushed pWord
			}

			m_pKeywordChunk = new BYTE [ KEYWORD_CHUNK ];
			m_iKeywordChunkFree = KEYWORD_CHUNK;
			m_dKeywordChunks.Add ( m_pKeywordChunk );
			m_iMemUse += KEYWORD_CHUNK;
		}
		// aw kay
		break;
	}

	pWord = m_pDictChunk++;
	m_iDictChunkFree--;
	pWord->m_sKeyword = (char*)m_pKeywordChunk;
	memcpy ( m_pKeywordChunk, tEntry.m_sKeyword, iLen );
	m_pKeywordChunk[iLen-1] = '\0';
	m_pKeywordChunk += iLen;
	m_iKeywordChunkFree -= iLen;

	pWord->m_uOff = tEntry.m_iDoclistOffset;
	pWord->m_iDocs = tEntry.m_iDocs;
	pWord->m_iHits = tEntry.m_iHits;
	pWord->m_uHint = sphDoclistHintPack ( tEntry.m_iDocs, tEntry.m_iDoclistLength );
	pWord->m_iSkiplistPos = 0;
	if ( tEntry.m_iDocs > m_iSkiplistBlockSize )
		pWord->m_iSkiplistPos = tEntry.m_iSkiplistOffset;
}

SphWordID_t CSphDictKeywords::GetWordID ( BYTE * pWord )
{
	SphWordID_t uCRC = CSphDictCRC<true>::GetWordID ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	auto iLen = (int) strlen ( (const char *)pWord );
	return HitblockGetID ( (const char *)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordIDWithMarkers ( BYTE * pWord )
{
	SphWordID_t uCRC = CSphDictCRC<true>::GetWordIDWithMarkers ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	auto iLen = (int) strlen ( (const char *)pWord );
	return HitblockGetID ( (const char *)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordIDNonStemmed ( BYTE * pWord )
{
	SphWordID_t uCRC = CSphDictCRC<true>::GetWordIDNonStemmed ( pWord );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	auto iLen = (int) strlen ( (const char *)pWord );
	return HitblockGetID ( (const char *)pWord, iLen, uCRC );
}

SphWordID_t CSphDictKeywords::GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops )
{
	SphWordID_t uCRC = CSphDictCRC<true>::GetWordID ( pWord, iLen, bFilterStops );
	if ( !uCRC || !m_bHitblock )
		return uCRC;

	return HitblockGetID ( (const char *)pWord, iLen, uCRC ); // !COMMIT would break, we kind of strcmp inside; but must never get called?
}

/// binary search for the first hit with wordid greater than or equal to reference
static CSphWordHit * FindFirstGte ( CSphWordHit * pHits, int iHits, SphWordID_t uID )
{
	if ( pHits->m_uWordID==uID )
		return pHits;

	CSphWordHit * pL = pHits;
	CSphWordHit * pR = pHits + iHits - 1;
	if ( pL->m_uWordID > uID || pR->m_uWordID < uID )
		return NULL;

	while ( pR-pL!=1 )
	{
		CSphWordHit * pM = pL + ( pR-pL )/2;
		if ( pM->m_uWordID < uID )
			pL = pM;
		else
			pR = pM;
	}

	assert ( pR-pL==1 );
	assert ( pL->m_uWordID<uID );
	assert ( pR->m_uWordID>=uID );
	return pR;
}

/// full crc and keyword check
static inline bool FullIsLess ( const CSphDictKeywords::HitblockException_t & a, const CSphDictKeywords::HitblockException_t & b )
{
	if ( a.m_uCRC!=b.m_uCRC )
		return a.m_uCRC < b.m_uCRC;
	return strcmp ( a.m_pEntry->m_pKeyword, b.m_pEntry->m_pKeyword ) < 0;
}

/// sort functor to compute collided hits reordering
struct HitblockPatchSort_fn
{
	const CSphDictKeywords::HitblockException_t * m_pExc;

	explicit HitblockPatchSort_fn ( const CSphDictKeywords::HitblockException_t * pExc )
		: m_pExc ( pExc )
	{}

	bool IsLess ( int a, int b ) const
	{
		return FullIsLess ( m_pExc[a], m_pExc[b] );
	}
};

/// do hit block patching magic
void CSphDictKeywords::HitblockPatch ( CSphWordHit * pHits, int iHits ) const
{
	if ( !pHits || iHits<=0 )
		return;

	const CSphVector<HitblockException_t> & dExc = m_dExceptions; // shortcut
	CSphVector<CSphWordHit*> dChunk;

	// reorder hit chunks for exceptions (aka crc collisions)
	for ( int iFirst = 0; iFirst < dExc.GetLength()-1; )
	{
		// find next span of collisions, iFirst inclusive, iMax exclusive ie. [iFirst,iMax)
		// (note that exceptions array is always sorted)
		SphWordID_t uFirstWordid = dExc[iFirst].m_pEntry->m_uWordid;
		assert ( dExc[iFirst].m_uCRC==uFirstWordid );

		int iMax = iFirst+1;
		SphWordID_t uSpan = uFirstWordid+1;
		while ( iMax < dExc.GetLength() && dExc[iMax].m_pEntry->m_uWordid==uSpan )
		{
			iMax++;
			uSpan++;
		}

		// check whether they are in proper order already
		bool bSorted = true;
		for ( int i=iFirst; i<iMax-1 && bSorted; i++ )
			if ( FullIsLess ( dExc[i+1], dExc[i] ) )
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
		dChunk.Resize ( iMax-iFirst+1 );

		// find the end
		dChunk.Last() = FindFirstGte ( pHits, iHits, uFirstWordid+iMax-iFirst );
		if ( !dChunk.Last() )
		{
			assert ( iMax==dExc.GetLength() && pHits[iHits-1].m_uWordID==uFirstWordid+iMax-1-iFirst );
			dChunk.Last() = pHits+iHits;
		}

		// find the start
		dChunk[0] = FindFirstGte ( pHits, int ( dChunk.Last()-pHits ), uFirstWordid );
		assert ( dChunk[0] && dChunk[0]->m_uWordID==uFirstWordid );

		// find the chunk starts
		for ( int i=1; i<dChunk.GetLength()-1; i++ )
		{
			dChunk[i] = FindFirstGte ( dChunk[i-1], int ( dChunk.Last()-dChunk[i-1] ), uFirstWordid+i );
			assert ( dChunk[i] && dChunk[i]->m_uWordID==uFirstWordid+i );
		}

		CSphWordHit * pTemp;
		if ( iMax-iFirst==2 )
		{
			// most frequent case, just two collisions
			// OPTIMIZE? allocate buffer for the smaller chunk, not just first chunk
			pTemp = new CSphWordHit [ dChunk[1]-dChunk[0] ];
			memcpy ( pTemp, dChunk[0], ( dChunk[1]-dChunk[0] )*sizeof(CSphWordHit) );
			memmove ( dChunk[0], dChunk[1], ( dChunk[2]-dChunk[1] )*sizeof(CSphWordHit) );
			memcpy ( dChunk[0] + ( dChunk[2]-dChunk[1] ), pTemp, ( dChunk[1]-dChunk[0] )*sizeof(CSphWordHit) );
		} else
		{
			// generic case, more than two
			CSphVector<int> dReorder ( iMax-iFirst );
			ARRAY_FOREACH ( i, dReorder )
				dReorder[i] = i;

			HitblockPatchSort_fn fnSort ( &dExc[iFirst] );
			dReorder.Sort ( fnSort );

			// OPTIMIZE? could skip heading and trailing blocks that are already in position
			pTemp = new CSphWordHit [ dChunk.Last()-dChunk[0] ];
			CSphWordHit * pOut = pTemp;

			ARRAY_FOREACH ( i, dReorder )
			{
				int iChunk = dReorder[i];
				int iChunkHits = int ( dChunk[iChunk+1] - dChunk[iChunk] );
				memcpy ( pOut, dChunk[iChunk], iChunkHits*sizeof(CSphWordHit) );
				pOut += iChunkHits;
			}

			assert ( ( pOut-pTemp )==( dChunk.Last()-dChunk[0] ) );
			memcpy ( dChunk[0], pTemp, ( dChunk.Last()-dChunk[0] )*sizeof(CSphWordHit) );
		}

		// patching done
		SafeDeleteArray ( pTemp );
		iFirst = iMax;
	}
}

const char * CSphDictKeywords::HitblockGetKeyword ( SphWordID_t uWordID )
{
	const DWORD uHash = (DWORD)( uWordID % SLOTS );

	HitblockKeyword_t * pEntry = m_dHash [ uHash ];
	while ( pEntry )
	{
		// check crc
		if ( pEntry->m_uWordid!=uWordID )
		{
			// crc mismatch, try next entry
			pEntry = pEntry->m_pNextHash;
			continue;
		}

		return pEntry->m_pKeyword;
	}

	assert ( m_dExceptions.GetLength() );
	ARRAY_FOREACH ( i, m_dExceptions )
		if ( m_dExceptions[i].m_pEntry->m_uWordid==uWordID )
			return m_dExceptions[i].m_pEntry->m_pKeyword;

	sphWarning ( "hash missing value in operator [] (wordid=" INT64_FMT ", hash=%u)", (int64_t)uWordID, uHash );
	assert ( 0 && "hash missing value in operator []" );
	return "\31oops";
}

//////////////////////////////////////////////////////////////////////////
// KEYWORDS STORING DICTIONARY
//////////////////////////////////////////////////////////////////////////

class CRtDictKeywords final : public ISphRtDictWraper
{
private:
	DictRefPtr_c		m_pBase;
	SmallStringHash_T<int, 8192>	m_hKeywords;

	CSphVector<BYTE>		m_dPackedKeywords {0};

	CSphString				m_sWarning;
	int						m_iKeywordsOverrun = 0;
	CSphString				m_sWord; // For allocation reuse.
	const bool				m_bStoreID = false;

protected:
	~CRtDictKeywords () final = default; // Is here since protected. fixme! remove

public:
	explicit CRtDictKeywords ( CSphDict * pBase, bool bStoreID )
		: m_pBase ( pBase )
		, m_bStoreID ( bStoreID )
	{
		SafeAddRef ( pBase );
		m_dPackedKeywords.Add ( 0 ); // avoid zero offset at all costs
	}

	SphWordID_t GetWordID ( BYTE * pWord ) final
	{
		SphWordID_t tWordID = m_pBase->GetWordID ( pWord );
		if ( tWordID )
			return AddKeyword ( pWord, tWordID );
		return 0;
	}

	SphWordID_t GetWordIDWithMarkers ( BYTE * pWord ) final
	{
		SphWordID_t tWordID = m_pBase->GetWordIDWithMarkers ( pWord );
		if ( tWordID )
			return AddKeyword ( pWord, tWordID );
		return 0;
	}

	SphWordID_t GetWordIDNonStemmed ( BYTE * pWord ) final
	{
		SphWordID_t tWordID = m_pBase->GetWordIDNonStemmed ( pWord );
		if ( tWordID )
			return AddKeyword ( pWord, tWordID );
		return 0;
	}

	SphWordID_t GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) final
	{
		SphWordID_t tWordID = m_pBase->GetWordID ( pWord, iLen, bFilterStops );
		if ( tWordID )
			return AddKeyword ( pWord, tWordID );
		return 0;
	}

	const BYTE * GetPackedKeywords () final { return m_dPackedKeywords.Begin(); }
	int GetPackedLen () final { return m_dPackedKeywords.GetLength(); }
	void ResetKeywords() final
	{
		m_dPackedKeywords.Resize ( 0 );
		m_dPackedKeywords.Add ( 0 ); // avoid zero offset at all costs
		m_hKeywords.Reset();
	}

	void LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile ) final { m_pBase->LoadStopwords ( sFiles, pTokenizer, bStripFile ); }
	void LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) final { m_pBase->LoadStopwords ( dStopwords ); }
	void WriteStopwords ( CSphWriter & tWriter ) const final { m_pBase->WriteStopwords ( tWriter ); }
	void WriteStopwords ( JsonEscapedBuilder & tOut ) const final { m_pBase->WriteStopwords ( tOut ); }
	bool LoadWordforms ( const StrVec_t & dFiles, const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex ) final
		{ return m_pBase->LoadWordforms ( dFiles, pEmbedded, pTokenizer, sIndex ); }
	void WriteWordforms ( CSphWriter & tWriter ) const final { m_pBase->WriteWordforms ( tWriter ); }
	void WriteWordforms ( JsonEscapedBuilder & tOut ) const final { m_pBase->WriteWordforms ( tOut ); }
	int SetMorphology ( const char * szMorph, CSphString & sMessage ) final { return m_pBase->SetMorphology ( szMorph, sMessage ); }
	void Setup ( const CSphDictSettings & tSettings ) final { m_pBase->Setup ( tSettings ); }
	const CSphDictSettings & GetSettings () const final { return m_pBase->GetSettings(); }
	const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () const final { return m_pBase->GetStopwordsFileInfos(); }
	const CSphVector <CSphSavedFile> & GetWordformsFileInfos () const final { return m_pBase->GetWordformsFileInfos(); }
	const CSphMultiformContainer * GetMultiWordforms () const final { return m_pBase->GetMultiWordforms(); }
	bool IsStopWord ( const BYTE * pWord ) const final { return m_pBase->IsStopWord ( pWord ); }
	const char * GetLastWarning() const final { return m_iKeywordsOverrun ? m_sWarning.cstr() : nullptr; }
	void ResetWarning () final { m_iKeywordsOverrun = 0; }
	uint64_t GetSettingsFNV () const final { return m_pBase->GetSettingsFNV(); }

private:
	SphWordID_t AddKeyword ( const BYTE * pWord, SphWordID_t tWordID )
	{
		auto iLen = (int) strlen ( ( const char * ) pWord );
		// stemmer might squeeze out the word
		if ( !iLen )
			return 0;

		// fix of very long word (zones)
		if ( iLen>( SPH_MAX_WORD_LEN * 3 ) )
		{
			int iClippedLen = SPH_MAX_WORD_LEN * 3;
			m_sWord.SetBinary ( ( const char * ) pWord, iClippedLen );
			if ( m_iKeywordsOverrun )
			{
				m_sWarning.SetSprintf ( "word overrun buffer, clipped!!! clipped='%s', length=%d(%d)", m_sWord.cstr ()
										, iClippedLen, iLen );
			} else
			{
				m_sWarning.SetSprintf ( ", clipped='%s', length=%d(%d)", m_sWord.cstr (), iClippedLen, iLen );
			}
			iLen = iClippedLen;
			m_iKeywordsOverrun++;
		} else
		{
			m_sWord.SetBinary ( ( const char * ) pWord, iLen );
		}

		int * pOff = m_hKeywords ( m_sWord );
		if ( pOff )
		{
			return *pOff;
		}

		int iOff = m_dPackedKeywords.GetLength ();
		int iPackedLen = iOff + iLen + 1;
		if ( m_bStoreID )
			iPackedLen += sizeof ( tWordID );
		m_dPackedKeywords.Resize ( iPackedLen );
		m_dPackedKeywords[iOff] = ( BYTE ) ( iLen & 0xFF );
		memcpy ( m_dPackedKeywords.Begin () + iOff + 1, pWord, iLen );
		if ( m_bStoreID )
			memcpy ( m_dPackedKeywords.Begin () + iOff + 1 + iLen, &tWordID, sizeof(tWordID) );

		m_hKeywords.Add ( iOff, m_sWord );

		return iOff;
	}
};

ISphRtDictWraper * sphCreateRtKeywordsDictionaryWrapper ( CSphDict * pBase, bool bStoreID )
{
	return new CRtDictKeywords ( pBase, bStoreID );
}


//////////////////////////////////////////////////////////////////////////
// DICTIONARY FACTORIES
//////////////////////////////////////////////////////////////////////////

static void SetupDictionary ( DictRefPtr_c & pDict, const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile, FilenameBuilder_i * pFilenameBuilder, CSphString & sError )
{
	assert ( pTokenizer );

	pDict->Setup ( tSettings );
	if ( pDict->SetMorphology ( tSettings.m_sMorphology.cstr (), sError )==CSphDict::ST_ERROR )
	{
		pDict = nullptr;
		return;
	}

	if ( pFiles && pFiles->m_bEmbeddedStopwords )
		pDict->LoadStopwords ( pFiles->m_dStopwords );
	else
	{
		CSphString sStopwordFile = tSettings.m_sStopwords;
		if ( !sStopwordFile.IsEmpty() )
		{
			if ( pFilenameBuilder )
				sStopwordFile = pFilenameBuilder->GetFullPath(sStopwordFile);

			pDict->LoadStopwords ( sStopwordFile.cstr(), pTokenizer, bStripFile );
		}
	}

	StrVec_t dWordformFiles;
	if ( pFilenameBuilder )
	{
		dWordformFiles.Resize ( tSettings.m_dWordforms.GetLength() );
		ARRAY_FOREACH ( i, tSettings.m_dWordforms )
			dWordformFiles[i] = pFilenameBuilder->GetFullPath ( tSettings.m_dWordforms[i] );
	}

	pDict->LoadWordforms ( pFilenameBuilder ? dWordformFiles : tSettings.m_dWordforms, pFiles && pFiles->m_bEmbeddedWordforms ? pFiles : NULL, pTokenizer, sIndex );
}

CSphDict * sphCreateDictionaryTemplate ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile,
	FilenameBuilder_i * pFilenameBuilder, CSphString & sError )
{
	DictRefPtr_c pDict { new CSphDictTemplate() };
	SetupDictionary ( pDict, tSettings, pFiles, pTokenizer, sIndex, bStripFile, pFilenameBuilder, sError );
	return pDict.Leak();
}


CSphDict * sphCreateDictionaryCRC ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile,
	int iSkiplistBlockSize, FilenameBuilder_i * pFilenameBuilder, CSphString & sError )
{
	DictRefPtr_c pDict { new CSphDictCRC<false> };
	SetupDictionary ( pDict, tSettings, pFiles, pTokenizer, sIndex, bStripFile, pFilenameBuilder, sError );
	// might be empty due to wrong morphology setup
	if ( pDict.Ptr() )
		pDict->SetSkiplistBlockSize ( iSkiplistBlockSize );
	return pDict.Leak ();
}


CSphDict * sphCreateDictionaryKeywords ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile,
	int iSkiplistBlockSize, FilenameBuilder_i * pFilenameBuilder, CSphString & sError )
{
	DictRefPtr_c pDict { new CSphDictKeywords() };
	SetupDictionary ( pDict, tSettings, pFiles, pTokenizer, sIndex, bStripFile, pFilenameBuilder, sError );
	// might be empty due to wrong morphology setup
	if ( pDict.Ptr() )
		pDict->SetSkiplistBlockSize ( iSkiplistBlockSize );
	return pDict.Leak ();
}


void sphShutdownWordforms ()
{
	CSphVector<CSphSavedFile> dEmptyFiles;
	CSphDiskDictTraits::SweepWordformContainers ( dEmptyFiles );
}

CSphDict * GetStatelessDict ( CSphDict * pDict )
{
	if ( !pDict )
		return nullptr;

	if ( pDict->HasState () )
		return pDict->Clone();

	SafeAddRef ( pDict );
	return pDict;
}

/////////////////////////////////////////////////////////////////////////////
// HTML STRIPPER
/////////////////////////////////////////////////////////////////////////////

static inline int sphIsTag ( int c )
{
	return sphIsAlpha(c) || c=='.' || c==':';
}

static inline int sphIsTagStart ( int c )
{
	return ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_' || c=='.' || c==':';
}

CSphHTMLStripper::CSphHTMLStripper ( bool bDefaultTags )
{
	if ( bDefaultTags )
	{
		// known inline tags
		const char * dKnown[] =
		{
			"a", "b", "i", "s", "u",
			"basefont", "big", "em", "font", "img",
			"label", "small", "span", "strike", "strong",
			"sub\0", "sup\0", // fix gcc 3.4.3 on solaris10 compiler bug
			"tt"
		};

		m_dTags.Resize ( sizeof(dKnown)/sizeof(dKnown[0]) );
		ARRAY_FOREACH ( i, m_dTags )
		{
			m_dTags[i].m_sTag = dKnown[i];
			m_dTags[i].m_iTagLen = (int) strlen ( dKnown[i] );
			m_dTags[i].m_bInline = true;
		}
	}

	UpdateTags ();
}


int CSphHTMLStripper::GetCharIndex ( int iCh ) const
{
	if ( iCh>='a' && iCh<='z' ) return iCh-'a';
	if ( iCh>='A' && iCh<='Z' ) return iCh-'A';
	if ( iCh=='_' ) return 26;
	if ( iCh==':' ) return 27;
	return -1;
}


void CSphHTMLStripper::UpdateTags ()
{
	m_dTags.Sort ();

	for ( int i=0; i<MAX_CHAR_INDEX; i++ )
	{
		m_dStart[i] = INT_MAX;
		m_dEnd[i] = -1;
	}

	ARRAY_FOREACH ( i, m_dTags )
	{
		int iIdx = GetCharIndex ( m_dTags[i].m_sTag.cstr()[0] );
		if ( iIdx<0 )
			continue;

		m_dStart[iIdx] = Min ( m_dStart[iIdx], i );
		m_dEnd[iIdx] = Max ( m_dEnd[iIdx], i );
	}
}


bool CSphHTMLStripper::SetIndexedAttrs ( const char * sConfig, CSphString & sError )
{
	if ( !sConfig || !*sConfig )
		return true;

	char sTag[256], sAttr[256];

	const char * p = sConfig, * s;
	#define LOC_ERROR(_msg,_pos) { sError.SetSprintf ( "SetIndexedAttrs(): %s near '%s'", _msg, _pos ); return false; }

	while ( *p )
	{
		// skip spaces
		while ( *p && isspace(*p) ) p++;
		if ( !*p ) break;

		// check tag name
		s = p; while ( sphIsTag(*p) ) p++;
		if ( s==p ) LOC_ERROR ( "invalid character in tag name", s );

		// get tag name
		if ( p-s>=(int)sizeof(sTag) ) LOC_ERROR ( "tag name too long", s );
		strncpy ( sTag, s, p-s );
		sTag[p-s] = '\0';

		// skip spaces
		while ( *p && isspace(*p) ) p++;
		if ( *p++!='=' ) LOC_ERROR ( "'=' expected", p-1 );

		// add indexed tag entry, if not there yet
		strlwr ( sTag );

		int iIndexTag = -1;
		ARRAY_FOREACH ( i, m_dTags )
			if ( m_dTags[i].m_sTag==sTag )
		{
			iIndexTag = i;
			break;
		}
		if ( iIndexTag<0 )
		{
			m_dTags.Add();
			m_dTags.Last().m_sTag = sTag;
			m_dTags.Last().m_iTagLen = (int) strlen ( sTag );
			iIndexTag = m_dTags.GetLength()-1;
		}

		m_dTags[iIndexTag].m_bIndexAttrs = true;
		StrVec_t & dAttrs = m_dTags[iIndexTag].m_dAttrs;

		// scan attributes
		while ( *p )
		{
			// skip spaces
			while ( *p && isspace(*p) ) p++;
			if ( !*p ) break;

			// check attr name
			s = p; while ( sphIsTag(*p) ) p++;
			if ( s==p ) LOC_ERROR ( "invalid character in attribute name", s );

			// get attr name
			if ( p-s>=(int)sizeof(sAttr) ) LOC_ERROR ( "attribute name too long", s );
			strncpy ( sAttr, s, p-s );
			sAttr[p-s] = '\0';

			// add attr, if not there yet
			int iAttr;
			for ( iAttr=0; iAttr<dAttrs.GetLength(); iAttr++ )
				if ( dAttrs[iAttr]==sAttr )
					break;

			if ( iAttr==dAttrs.GetLength() )
				dAttrs.Add ( sAttr );

			// skip spaces
			while ( *p && isspace(*p) ) p++;
			if ( !*p ) break;

			// check if there's next attr or tag
			if ( *p==',' ) { p++; continue; } // next attr
			if ( *p==';' ) { p++; break; } // next tag
			LOC_ERROR ( "',' or ';' or end of line expected", p );
		}
	}

	#undef LOC_ERROR

	UpdateTags ();
	return true;
}


bool CSphHTMLStripper::SetRemovedElements ( const char * sConfig, CSphString & )
{
	if ( !sConfig || !*sConfig )
		return true;

	const char * p = sConfig;
	while ( *p )
	{
		// skip separators
		while ( *p && !sphIsTag(*p) ) p++;
		if ( !*p ) break;

		// get tag name
		const char * s = p;
		while ( sphIsTag(*p) ) p++;

		CSphString sTag;
		sTag.SetBinary ( s, int(p-s) );
		sTag.ToLower ();

		// mark it
		int iTag;
		for ( iTag=0; iTag<m_dTags.GetLength(); iTag++ )
			if ( m_dTags[iTag].m_sTag==sTag )
		{
			m_dTags[iTag].m_bRemove = true;
			break;
		}

		if ( iTag==m_dTags.GetLength() )
		{
			m_dTags.Add();
			m_dTags.Last().m_sTag = sTag;
			m_dTags.Last().m_iTagLen = (int) strlen ( sTag.cstr() );
			m_dTags.Last().m_bRemove = true;
		}
	}

	UpdateTags ();
	return true;
}


void CSphHTMLStripper::EnableParagraphs ()
{
	// known block-level elements
	const char * dBlock[] = { "address", "blockquote", "caption", "center",
		"dd", "div", "dl", "dt", "h1", "h2", "h3", "h4", "h5", "li", "menu",
		"ol", "p", "pre", "table", "tbody", "td", "tfoot", "th", "thead",
		"tr", "ul", NULL };

	for ( int iBlock=0; dBlock[iBlock]; iBlock++ )
	{
		const char * sTag = dBlock[iBlock];

		// mark if known already
		int iTag;
		for ( iTag=0; iTag<m_dTags.GetLength(); iTag++ )
			if ( m_dTags[iTag].m_sTag==sTag )
		{
			m_dTags[iTag].m_bPara = true;
			break;
		}

		// add if not known yet
		if ( iTag==m_dTags.GetLength() )
		{
			StripperTag_t& dTag = m_dTags.Add();
			dTag.m_sTag = sTag;
			dTag.m_iTagLen = (int) strlen(sTag);
			dTag.m_bPara = true;
		}
	}

	UpdateTags ();
}


bool CSphHTMLStripper::SetZones ( const char * sZones, CSphString & sError )
{
	// yet another mini parser!
	// index_zones = {tagname | prefix*} [, ...]
	if ( !sZones || !*sZones )
		return true;

	const char * s = sZones;
	while ( *s )
	{
		// skip spaces
		while ( sphIsSpace(*s) )
			s++;
		if ( !*s )
			break;

		// expect ident
		if ( !sphIsTagStart(*s) )
		{
			sError.SetSprintf ( "unexpected char near '%s' in index_zones", s );
			return false;
		}

		// get ident (either tagname or prefix*)
		const char * sTag = s;
		while ( sphIsTag(*s) )
			s++;

		const char * sTagEnd = s;
		bool bPrefix = false;
		if ( *s=='*' )
		{
			s++;
			bPrefix = true;
		}

		// skip spaces
		while ( sphIsSpace(*s) )
			s++;

		// expect eof or comma after ident
		if ( *s && *s!=',' )
		{
			sError.SetSprintf ( "unexpected char near '%s' in index_zones", s );
			return false;
		}
		if ( *s==',' )
			s++;

		// got valid entry, handle it
		CSphHTMLStripper::StripperTag_t & tTag = m_dTags.Add();
		tTag.m_sTag.SetBinary ( sTag, int(sTagEnd-sTag) );
		tTag.m_iTagLen = (int)( sTagEnd-sTag );
		tTag.m_bZone = true;
		tTag.m_bZonePrefix = bPrefix;
	}

	UpdateTags ();
	return true;
}


const BYTE * SkipQuoted ( const BYTE * p )
{
	const BYTE * pMax = p + 512; // 512 bytes should be enough for a reasonable HTML attribute value, right?!
	const BYTE * pProbEnd = NULL; // (most) probable end location in case we don't find a matching quote
	BYTE cEnd = *p++; // either apostrophe or quote

	while ( p<pMax && *p && *p!=cEnd )
	{
		if ( !pProbEnd )
			if ( *p=='>' || *p=='\r' )
				pProbEnd = p;
		p++;
	}

	if ( *p==cEnd )
		return p+1;

	if ( pProbEnd )
		return pProbEnd;

	return p;
}


struct HtmlEntity_t
{
	const char *	m_sName;
	int				m_iCode;
};


static inline DWORD HtmlEntityHash ( const BYTE * str, int len )
{
	static const unsigned short asso_values[] =
	{
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 4,
		6, 22, 1, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 170, 48, 0, 5, 44,
		0, 10, 10, 86, 421, 7, 0, 1, 42, 93,
		41, 421, 0, 5, 8, 14, 421, 421, 5, 11,
		8, 421, 421, 421, 421, 421, 421, 1, 25, 27,
		9, 2, 113, 82, 14, 3, 179, 1, 81, 91,
		12, 0, 1, 180, 56, 17, 5, 31, 60, 7,
		3, 161, 2, 3, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421
	};

	register int hval = len;
	switch ( hval )
	{
		default:	hval += asso_values [ str[4] ];
		case 4:
		case 3:		hval += asso_values [ str[2] ];
		case 2:		hval += asso_values [ str[1]+1 ];
		case 1:		hval += asso_values [ str[0] ];
					break;
	}
	return hval + asso_values [ str[len-1] ];
}


static inline int HtmlEntityLookup ( const BYTE * str, int len )
{
	static const unsigned char lengthtable[] =
	{
		0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 3,
		4, 3, 3, 5, 3, 6, 5, 5, 3, 4, 4, 5, 3, 4,
		4, 0, 5, 4, 5, 6, 5, 6, 4, 5, 3, 3, 5, 0,
		0, 0, 0, 6, 0, 5, 5, 0, 5, 6, 6, 3, 0, 3,
		5, 3, 0, 6, 0, 4, 3, 6, 3, 6, 6, 6, 6, 5,
		5, 5, 5, 5, 5, 2, 6, 4, 0, 6, 3, 3, 3, 0,
		4, 5, 4, 4, 4, 3, 7, 4, 3, 6, 2, 3, 6, 4,
		3, 6, 5, 6, 5, 5, 4, 2, 0, 0, 4, 6, 8, 0,
		0, 0, 5, 5, 0, 6, 6, 2, 2, 4, 4, 6, 6, 4,
		4, 5, 6, 2, 3, 4, 6, 5, 0, 2, 0, 0, 6, 6,
		6, 6, 6, 4, 6, 5, 0, 6, 4, 5, 4, 6, 6, 0,
		0, 4, 6, 5, 6, 0, 6, 4, 5, 6, 5, 6, 4, 0,
		3, 6, 0, 4, 4, 4, 5, 4, 6, 0, 4, 4, 6, 5,
		6, 7, 2, 2, 6, 2, 5, 2, 5, 0, 0, 0, 4, 4,
		2, 4, 2, 2, 4, 0, 4, 4, 4, 5, 5, 0, 3, 7,
		5, 0, 5, 6, 5, 0, 6, 0, 6, 0, 4, 6, 4, 6,
		6, 2, 6, 0, 5, 5, 4, 6, 6, 0, 5, 6, 4, 4,
		4, 4, 0, 5, 0, 5, 0, 4, 5, 4, 0, 4, 4, 4,
		0, 0, 0, 4, 0, 0, 0, 5, 6, 5, 3, 0, 0, 6,
		5, 4, 5, 5, 5, 5, 0, 5, 5, 0, 5, 0, 0, 0,
		4, 6, 0, 3, 0, 5, 5, 0, 0, 3, 6, 5, 0, 4,
		0, 0, 0, 0, 5, 7, 5, 3, 5, 3, 0, 0, 6, 0,
		6, 0, 0, 7, 0, 0, 5, 0, 5, 0, 0, 0, 0, 5,
		4, 0, 0, 0, 0, 0, 7, 4, 0, 0, 3, 0, 0, 0,
		3, 0, 6, 0, 0, 7, 5, 5, 0, 3, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 5,
		5, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0,
		4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5
	};

	static const struct HtmlEntity_t wordlist[] =
	{
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Rho", 929},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Chi", 935},
		{"phi", 966},
		{"iota", 953},
		{"psi", 968},
		{"int", 8747},
		{"theta", 952},
		{"amp", 38},
		{"there4", 8756},
		{"Theta", 920},
		{"omega", 969},
		{"and", 8743},
		{"prop", 8733},
		{"ensp", 8194},
		{"image", 8465},
		{"not", 172},
		{"isin", 8712},
		{"sdot", 8901},
		{"", 0},
		{"prime", 8242},
		{"prod", 8719},
		{"trade", 8482},
		{"Scaron", 352},
		{"kappa", 954},
		{"thinsp", 8201},
		{"emsp", 8195},
		{"thorn", 254},
		{"eta", 951},
		{"chi", 967},
		{"Kappa", 922},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"scaron", 353},
		{"", 0},
		{"notin", 8713},
		{"ndash", 8211},
		{"", 0},
		{"acute", 180},
		{"otilde", 245},
		{"atilde", 227},
		{"Phi", 934},
		{"", 0},
		{"Psi", 936},
		{"pound", 163},
		{"cap", 8745},
		{"", 0},
		{"otimes", 8855},
		{"", 0},
		{"nbsp", 32},
		{"rho", 961},
		{"ntilde", 241},
		{"eth", 240},
		{"oacute", 243},
		{"aacute", 225},
		{"eacute", 233},
		{"iacute", 237},
		{"nabla", 8711},
		{"Prime", 8243},
		{"ocirc", 244},
		{"acirc", 226},
		{"ecirc", 234},
		{"icirc", 238},
		{"or", 8744},
		{"Yacute", 221},
		{"nsub", 8836},
		{"", 0},
		{"Uacute", 218},
		{"Eta", 919},
		{"ETH", 208},
		{"sup", 8835},
		{"", 0},
		{"supe", 8839},
		{"Ucirc", 219},
		{"sup1", 185},
		{"para", 182},
		{"sup2", 178},
		{"loz", 9674},
		{"omicron", 959},
		{"part", 8706},
		{"cup", 8746},
		{"Ntilde", 209},
		{"Mu", 924},
		{"tau", 964},
		{"uacute", 250},
		{"Iota", 921},
		{"Tau", 932},
		{"rsaquo", 8250},
		{"alpha", 945},
		{"Ccedil", 199},
		{"ucirc", 251},
		{"oline", 8254},
		{"sup3", 179},
		{"nu", 957},
		{"", 0}, {"", 0},
		{"sube", 8838},
		{"Eacute", 201},
		{"thetasym", 977},
		{"", 0}, {"", 0}, {"", 0},
		{"Omega", 937},
		{"Ecirc", 202},
		{"", 0},
		{"lowast", 8727},
		{"iquest", 191},
		{"lt", 60},
		{"gt", 62},
		{"ordm", 186},
		{"euro", 8364},
		{"oslash", 248},
		{"lsaquo", 8249},
		{"zeta", 950},
		{"cong", 8773},
		{"mdash", 8212},
		{"ccedil", 231},
		{"ne", 8800},
		{"sub", 8834},
		{"Zeta", 918},
		{"Lambda", 923},
		{"Gamma", 915},
		{"", 0},
		{"Nu", 925},
		{"", 0}, {"", 0},
		{"ograve", 242},
		{"agrave", 224},
		{"egrave", 232},
		{"igrave", 236},
		{"frac14", 188},
		{"ordf", 170},
		{"Otilde", 213},
		{"infin", 8734},
		{"", 0},
		{"frac12", 189},
		{"beta", 946},
		{"radic", 8730},
		{"darr", 8595},
		{"Iacute", 205},
		{"Ugrave", 217},
		{"", 0}, {"", 0},
		{"harr", 8596},
		{"hearts", 9829},
		{"Icirc", 206},
		{"Oacute", 211},
		{"", 0},
		{"frac34", 190},
		{"cent", 162},
		{"crarr", 8629},
		{"curren", 164},
		{"Ocirc", 212},
		{"brvbar", 166},
		{"sect", 167},
		{"", 0},
		{"ang", 8736},
		{"ugrave", 249},
		{"", 0},
		{"Beta", 914},
		{"uarr", 8593},
		{"dArr", 8659},
		{"asymp", 8776},
		{"perp", 8869},
		{"Dagger", 8225},
		{"", 0},
		{"hArr", 8660},
		{"rang", 9002},
		{"dagger", 8224},
		{"exist", 8707},
		{"Egrave", 200},
		{"Omicron", 927},
		{"mu", 956},
		{"pi", 960},
		{"weierp", 8472},
		{"xi", 958},
		{"clubs", 9827},
		{"Xi", 926},
		{"aring", 229},
		{"", 0}, {"", 0}, {"", 0},
		{"copy", 169},
		{"uArr", 8657},
		{"ni", 8715},
		{"rarr", 8594},
		{"le", 8804},
		{"ge", 8805},
		{"zwnj", 8204},
		{"", 0},
		{"apos", 39},
		{"macr", 175},
		{"lang", 9001},
		{"gamma", 947},
		{"Delta", 916},
		{"", 0},
		{"uml", 168},
		{"alefsym", 8501},
		{"delta", 948},
		{"", 0},
		{"bdquo", 8222},
		{"lambda", 955},
		{"equiv", 8801},
		{"", 0},
		{"Oslash", 216},
		{"", 0},
		{"hellip", 8230},
		{"", 0},
		{"rArr", 8658},
		{"Atilde", 195},
		{"larr", 8592},
		{"spades", 9824},
		{"Igrave", 204},
		{"Pi", 928},
		{"yacute", 253},
		{"", 0},
		{"diams", 9830},
		{"sbquo", 8218},
		{"fnof", 402},
		{"Ograve", 210},
		{"plusmn", 177},
		{"", 0},
		{"rceil", 8969},
		{"Aacute", 193},
		{"ouml", 246},
		{"auml", 228},
		{"euml", 235},
		{"iuml", 239},
		{"", 0},
		{"Acirc", 194},
		{"", 0},
		{"rdquo", 8221},
		{"", 0},
		{"lArr", 8656},
		{"rsquo", 8217},
		{"Yuml", 376},
		{"", 0},
		{"quot", 34},
		{"Uuml", 220},
		{"bull", 8226},
		{"", 0}, {"", 0}, {"", 0},
		{"real", 8476},
		{"", 0}, {"", 0}, {"", 0},
		{"lceil", 8968},
		{"permil", 8240},
		{"upsih", 978},
		{"sum", 8721},
		{"", 0}, {"", 0},
		{"divide", 247},
		{"raquo", 187},
		{"uuml", 252},
		{"ldquo", 8220},
		{"Alpha", 913},
		{"szlig", 223},
		{"lsquo", 8216},
		{"", 0},
		{"Sigma", 931},
		{"tilde", 732},
		{"", 0},
		{"THORN", 222},
		{"", 0}, {"", 0}, {"", 0},
		{"Euml", 203},
		{"rfloor", 8971},
		{"", 0},
		{"lrm", 8206},
		{"", 0},
		{"sigma", 963},
		{"iexcl", 161},
		{"", 0}, {"", 0},
		{"deg", 176},
		{"middot", 183},
		{"laquo", 171},
		{"", 0},
		{"circ", 710},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"frasl", 8260},
		{"epsilon", 949},
		{"oplus", 8853},
		{"yen", 165},
		{"micro", 181},
		{"piv", 982},
		{"", 0}, {"", 0},
		{"lfloor", 8970},
		{"", 0},
		{"Agrave", 192},
		{"", 0}, {"", 0},
		{"Upsilon", 933},
		{"", 0}, {"", 0},
		{"times", 215},
		{"", 0},
		{"cedil", 184},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"minus", 8722},
		{"Iuml", 207},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"upsilon", 965},
		{"Ouml", 214},
		{"", 0}, {"", 0},
		{"rlm", 8207},
		{"", 0}, {"", 0}, {"", 0},
		{"reg", 174},
		{"", 0},
		{"forall", 8704},
		{"", 0}, {"", 0},
		{"Epsilon", 917},
		{"empty", 8709},
		{"OElig", 338},
		{"", 0},
		{"shy", 173},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Aring", 197},
		{"", 0}, {"", 0}, {"", 0},
		{"oelig", 339},
		{"aelig", 230},
		{"", 0},
		{"zwj", 8205},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"sim", 8764},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"yuml", 255},
		{"sigmaf", 962},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Auml", 196},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"AElig", 198}
	};

	const int MIN_WORD_LENGTH		= 2;
	const int MAX_WORD_LENGTH		= 8;
	const int MAX_HASH_VALUE		= 420;

	if ( len<=MAX_WORD_LENGTH && len>=MIN_WORD_LENGTH )
	{
		register int key = HtmlEntityHash ( str, len );
		if ( key<=MAX_HASH_VALUE && key>=0 )
			if ( len==lengthtable[key] )
		{
			register const char * s = wordlist[key].m_sName;
			if ( *str==*s && !memcmp ( str+1, s+1, len-1 ) )
				return wordlist[key].m_iCode;
		}
	}
	return 0;
}


static const BYTE * SkipPI ( const BYTE * s )
{
	assert ( s[0]=='<' && s[1]=='?' );
	s += 2;

	const BYTE * pStart = s;
	const BYTE * pMax = s + 256;
	for ( ; s<pMax && *s; s++ )
	{
		// for now, let's just bail whenever we see ">", like Firefox does!!!
		// that covers the valid case, ie. the closing "?>", just as well
		if ( s[0]=='>' )
			return s+1;
	}

	if ( !*s )
		return s;

	// no closing end marker ever found; just skip non-whitespace after "<?" then
	s = pStart;
	while ( s<pMax && *s && !sphIsSpace(*s) )
		s++;
	return s;
}


void CSphHTMLStripper::Strip ( BYTE * sData ) const
{
	if ( !sData )
		return;

	const BYTE * s = sData;
	BYTE * d = sData;
	while (true)
	{
		/////////////////////////////////////
		// scan until eof, or tag, or entity
		/////////////////////////////////////

		while ( *s && *s!='<' && *s!='&' )
		{
			if ( *s>=0x20 )
				*d++ = *s;
			else
				*d++ = ' ';
			s++;
		}
		if ( !*s )
			break;

		/////////////////
		// handle entity
		/////////////////

		if ( *s=='&' )
		{
			if ( s[1]=='#' )
			{
				// handle "&#number;" and "&#xnumber;" forms
				DWORD uCode = 0;
				s += 2;

				bool bHex = ( *s && ( *s=='x' || *s=='X') );
				if ( !bHex )
				{
					while ( isdigit(*s) )
						uCode = uCode*10 + (*s++) - '0';
				} else
				{
					s++;
					while ( *s )
					{
						if ( isdigit(*s) )
							uCode = uCode*16 + (*s++) - '0';
						else if ( *s>=0x41 && *s<=0x46 )
							uCode = uCode*16 + (*s++) - 'A' + 0xA;
						else if ( *s>=0x61 && *s<=0x66 )
							uCode = uCode*16 + (*s++) - 'a' + 0xA;
						else
							break;
					}
				}

				uCode = uCode % 0x110000; // there is no unicode code-points bigger than this value

				if ( uCode<=0x1f || *s!=';' ) // 0-31 are reserved codes
					continue;

				d += sphUTF8Encode ( d, (int)uCode );
				s++;

			} else
			{
				// skip until ';' or max length
				if ( ( s[1]>='a' && s[1]<='z' ) || ( s[1]>='A' && s[1]<='Z' ) )
				{
					const int MAX_ENTITY_LEN = 8;
					const BYTE * sStart = s+1;
					while ( *s && *s!=';' && s-sStart<=MAX_ENTITY_LEN )
						s++;

					if ( *s==';' )
					{
						int iCode = HtmlEntityLookup ( sStart, (int)(s-sStart) );
						if ( iCode>0 )
						{
							// this is a known entity; encode it
							d += sphUTF8Encode ( d, iCode );
							s++;
							continue;
						}
					}

					// rollback
					s = sStart-1;
				}

				// if we're here, it's not an entity; pass the leading ampersand and rescan
				*d++ = *s++;
			}
			continue;
		}

		//////////////
		// handle tag
		//////////////

		assert ( *s=='<' );
		if ( GetCharIndex(s[1])<0 )
		{
			if ( s[1]=='/' )
			{
				// check if it's valid closing tag
				if ( GetCharIndex(s[2])<0 )
				{
					*d++ = *s++;
					continue;
				}

			} else if ( s[1]=='!' )
			{
				if ( s[2]=='-' && s[3]=='-' )
				{
					// it's valid comment; scan until comment end
					s += 4; // skip opening '<!--'
					while ( *s )
					{
						if ( s[0]=='-' && s[1]=='-' && s[2]=='>' )
							break;
						s++;
					}
					if ( !*s )
						break;
					s += 3; // skip closing '-->'
					continue;

				} else if ( isalpha(s[2]) )
				{
					// it's <!doctype> style PI; scan until PI end
					s += 2;
					while ( *s && *s!='>' )
					{
						if ( *s=='\'' || *s=='"' )
						{
							s = SkipQuoted ( s );
							while ( isspace(*s) ) s++;
						} else
						{
							s++;
						}
					}
					if ( *s=='>' )
						s++;
					continue;

				} else
				{
					// it's something malformed; just ignore
					*d++ = *s++;
					continue;
				}

			} else if ( s[1]=='?' )
			{
				// scan until PI end
				s = SkipPI ( s );
				continue;

			} else
			{
				// simply malformed
				*d++ = *s++;
				continue;
			}
		}
		s++; // skip '<'

		//////////////////////////////////////
		// lookup this tag in known tags list
		//////////////////////////////////////

		const StripperTag_t * pTag = NULL;
		int iZoneNameLen = 0;
		const BYTE * sZoneName = NULL;
		s = FindTag ( s, &pTag, &sZoneName, &iZoneNameLen );

		/////////////////////////////////////
		// process tag contents
		// index attributes if needed
		// gracefully handle malformed stuff
		/////////////////////////////////////

#define LOC_SKIP_SPACES() { while ( sphIsSpace(*s) ) s++; if ( !*s || *s=='>' ) break; }

		bool bIndexAttrs = ( pTag && pTag->m_bIndexAttrs );
		while ( *s && *s!='>' )
		{
			LOC_SKIP_SPACES();
			if ( sphIsTagStart(*s) )
			{
				// skip attribute name while it's valid
				const BYTE * sAttr = s;
				while ( sphIsTag(*s) )
					s++;

				// blanks or a value after a valid attribute name?
				if ( sphIsSpace(*s) || *s=='=' )
				{
					const int iAttrLen = (int)( s - sAttr );
					LOC_SKIP_SPACES();

					// a valid name but w/o a value; keep scanning
					if ( *s!='=' )
						continue;

					// got value!
					s++;
					LOC_SKIP_SPACES();

					// check attribute name
					// OPTIMIZE! remove linear search
					int iAttr = -1;
					if ( bIndexAttrs )
					{
						for ( iAttr=0; iAttr<pTag->m_dAttrs.GetLength(); iAttr++ )
						{
							auto iLen = (int) strlen ( pTag->m_dAttrs[iAttr].cstr() );
							if ( iLen==iAttrLen && !strncasecmp ( pTag->m_dAttrs[iAttr].cstr(), (const char*)sAttr, iLen ) )
								break;
						}
						if ( iAttr==pTag->m_dAttrs.GetLength() )
							iAttr = -1;
					}

					// process the value
					const BYTE * sVal = s;
					if ( *s=='\'' || *s=='"' )
					{
						// skip quoted value until a matching quote
						s = SkipQuoted ( s );
					} else
					{
						// skip unquoted value until tag end or whitespace
						while ( *s && *s!='>' && !sphIsSpace(*s) )
							s++;
					}

					// if this one is to be indexed, copy it
					if ( iAttr>=0 )
					{
						const BYTE * sMax = s;
						if ( *sVal=='\'' || *sVal=='"' )
						{
							if ( sMax[-1]==sVal[0] )
								sMax--;
							sVal++;
						}
						while ( sVal<sMax )
							*d++ = *sVal++;
						*d++ = ' ';
					}

					// handled the value; keep scanning
					continue;
				}

				// nope, got an invalid character in the sequence (or maybe eof)
				// fall through to an invalid name handler
			}

			// keep skipping until tag end or whitespace
			while ( *s && *s!='>' && !sphIsSpace(*s) )
				s++;
		}

#undef LOC_SKIP_SPACES

		// skip closing angle bracket, if any
		if ( *s )
			s++;

		// unknown tag is done; others might require a bit more work
		if ( !pTag )
		{
			*d++ = ' '; // unknown tags are *not* inline by default
			continue;
		}

		// handle zones
		if ( pTag->m_bZone )
		{
			// should be at tag's end
			assert ( s[0]=='\0' || s[-1]=='>' );

			// emit secret codes
			*d++ = MAGIC_CODE_ZONE;
			for ( int i=0; i<iZoneNameLen; i++ )
				*d++ = (BYTE) tolower ( sZoneName[i] );
			if ( *d )
				*d++ = MAGIC_CODE_ZONE;

			if ( !*s )
				break;
			continue;
		}

		// handle paragraph boundaries
		if ( pTag->m_bPara )
		{
			*d++ = MAGIC_CODE_PARAGRAPH;
			continue;
		}

		// in all cases, the tag must be fully processed at this point
		// not a remove-tag? we're done
		if ( !pTag->m_bRemove )
		{
			if ( !pTag->m_bInline )
				*d++ = ' ';
			continue;
		}

		// sudden eof? bail out
		if ( !*s )
			break;

		// must be a proper remove-tag end, then
		assert ( pTag->m_bRemove && s[-1]=='>' );

		// short-form? we're done
		if ( s[-2]=='/' )
			continue;

		// skip everything until the closing tag
		// FIXME! should we handle insane cases with quoted closing tag within tag?
		while (true)
		{
			while ( *s && ( s[0]!='<' || s[1]!='/' ) ) s++;
			if ( !*s ) break;

			s += 2; // skip </
			if ( strncasecmp ( pTag->m_sTag.cstr(), (const char*)s, pTag->m_iTagLen )!=0 ) continue;
			if ( !sphIsTag ( s[pTag->m_iTagLen] ) )
			{
				s += pTag->m_iTagLen; // skip tag
				if ( *s=='>' ) s++;
				break;
			}
		}

		if ( !pTag->m_bInline ) *d++ = ' ';
	}
	*d++ = '\0';

	// space, paragraph sequences elimination pass
	s = sData;
	d = sData;
	bool bSpaceOut = false;
	bool bParaOut = false;
	bool bZoneOut = false;
	while ( const char c = *s++ )
	{
		assert ( d<=s-1 );

		// handle different character classes
		if ( sphIsSpace(c) )
		{
			// handle whitespace, skip dupes
			if ( !bSpaceOut )
				*d++ = ' ';

			bSpaceOut = true;
			continue;

		} else if ( c==MAGIC_CODE_PARAGRAPH )
		{
			// handle paragraph marker, skip dupes
			if ( !bParaOut && !bZoneOut )
			{
				*d++ = c;
				bParaOut = true;
			}

			bSpaceOut = true;
			continue;

		} else if ( c==MAGIC_CODE_ZONE )
		{
			// zone marker
			// rewind preceding paragraph, if any, it is redundant
			if ( bParaOut )
			{
				assert ( d>sData && d[-1]==MAGIC_CODE_PARAGRAPH );
				d--;
			}

			// copy \4zoneid\4
			*d++ = c;
			while ( *s && *s!=MAGIC_CODE_ZONE )
				*d++ = *s++;

			if ( *s )
				*d++ = *s++;

			// update state
			// no spaces paragraphs allowed
			bSpaceOut = bZoneOut = true;
			bParaOut = false;
			continue;

		} else
		{
			*d++ = c;
			bSpaceOut = bParaOut = bZoneOut = false;
		}
	}
	*d++ = '\0';
}

const BYTE * CSphHTMLStripper::FindTag ( const BYTE * sSrc, const StripperTag_t ** ppTag,
	const BYTE ** ppZoneName, int * pZoneNameLen ) const
{
	assert ( sSrc && ppTag && ppZoneName && pZoneNameLen );
	assert ( sSrc[0]!='/' || sSrc[1]!='\0' );

	const BYTE * sTagName = ( sSrc[0]=='/' ) ? sSrc+1 : sSrc;

	*ppZoneName = sSrc;
	*pZoneNameLen = 0;

	int iIdx = GetCharIndex ( sTagName[0] );
	assert ( iIdx>=0 && iIdx<MAX_CHAR_INDEX );

	if ( iIdx>=0 && m_dEnd[iIdx]>=0 )
	{
		int iStart = m_dStart[iIdx];
		int iEnd = m_dEnd[iIdx];

		for ( int i=iStart; i<=iEnd; i++ )
		{
			int iLen = m_dTags[i].m_iTagLen;
			int iCmp = strncasecmp ( m_dTags[i].m_sTag.cstr(), (const char*)sTagName, iLen );

			// the tags are sorted; so if current candidate is already greater, rest can be skipped
			if ( iCmp>0 )
				break;

			// do we have a match?
			if ( iCmp==0 )
			{
				// got exact match?
				if ( !sphIsTag ( sTagName[iLen] ) )
				{
					*ppTag = m_dTags.Begin() + i;
					sSrc = sTagName + iLen; // skip tag name
					if ( m_dTags[i].m_bZone )
						*pZoneNameLen = int ( sSrc - *ppZoneName );
					break;
				}

				// got wildcard match?
				if ( m_dTags[i].m_bZonePrefix )
				{
					*ppTag = m_dTags.Begin() + i;
					sSrc = sTagName + iLen;
					while ( sphIsTag(*sSrc) )
						sSrc++;
					*pZoneNameLen = int ( sSrc - *ppZoneName );
					break;
				}
			}
		}
	}

	return sSrc;
}

bool CSphHTMLStripper::IsValidTagStart ( int iCh ) const
{
	int i = GetCharIndex ( iCh );
	return ( i>=0 && i<MAX_CHAR_INDEX );
}

//////////////////////////////////////////////////////////////////////////
ISphFieldFilter::ISphFieldFilter()
	: m_pParent ( NULL )
{
}


ISphFieldFilter::~ISphFieldFilter()
{
	SafeRelease ( m_pParent );
}


void ISphFieldFilter::SetParent ( ISphFieldFilter * pParent )
{
	SafeAddRef ( pParent );
	SafeRelease ( m_pParent );
	m_pParent = pParent;
}


#if WITH_RE2
class CSphFieldRegExps : public ISphFieldFilter
{
protected:
	~CSphFieldRegExps() override;
public:
						CSphFieldRegExps () = default;
						CSphFieldRegExps ( const StrVec_t& m_dRegexps, CSphString &	sError );

	int					Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool ) final;
	void				GetSettings ( CSphFieldFilterSettings & tSettings ) const final;
	ISphFieldFilter *	Clone() const final;

	bool				AddRegExp ( const char * sRegExp, CSphString & sError );

private:
	struct RegExp_t
	{
		CSphString	m_sFrom;
		CSphString	m_sTo;

		RE2 *		m_pRE2;
	};

	CSphVector<RegExp_t>	m_dRegexps;
	bool					m_bCloned = true;
};

CSphFieldRegExps::CSphFieldRegExps ( const StrVec_t &dRegexps, CSphString &sError )
	: m_bCloned ( false )
{
	for ( const auto &sRegexp : dRegexps )
		AddRegExp ( sRegexp.cstr (), sError );
}


CSphFieldRegExps::~CSphFieldRegExps ()
{
	if ( !m_bCloned )
	{
		for ( auto & dRegexp : m_dRegexps )
			SafeDelete ( dRegexp.m_pRE2 );
	}
}


int CSphFieldRegExps::Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool )
{
	dStorage.Resize ( 0 );
	if ( !sField || !*sField )
		return 0;

	bool bReplaced = false;
	std::string sRe2 = ( iLength ? std::string ( (const char *) sField, iLength ) : (const char *) sField );
	ARRAY_FOREACH ( i, m_dRegexps )
	{
		assert ( m_dRegexps[i].m_pRE2 );
		bReplaced |= ( RE2::GlobalReplace ( &sRe2, *m_dRegexps[i].m_pRE2, m_dRegexps[i].m_sTo.cstr() )>0 );
	}

	if ( !bReplaced )
		return 0;

	auto iDstLen = (int) sRe2.length();
	dStorage.Resize ( iDstLen+4 ); // string SAFETY_GAP
	strncpy ( (char *)dStorage.Begin(), sRe2.c_str(), dStorage.GetLength() );
	return iDstLen;
}


void CSphFieldRegExps::GetSettings ( CSphFieldFilterSettings & tSettings ) const
{
	tSettings.m_dRegexps.Resize ( m_dRegexps.GetLength() );
	ARRAY_FOREACH ( i, m_dRegexps )
		tSettings.m_dRegexps[i].SetSprintf ( "%s => %s", m_dRegexps[i].m_sFrom.cstr(), m_dRegexps[i].m_sTo.cstr() );
}


bool CSphFieldRegExps::AddRegExp ( const char * sRegExp, CSphString & sError )
{
	if ( m_bCloned )
		return false;

	const char sSplitter [] = "=>";
	const char * sSplit = strstr ( sRegExp, sSplitter );
	if ( !sSplit )
	{
		sError = "mapping token (=>) not found";
		return false;
	} else if ( strstr ( sSplit + strlen ( sSplitter ), sSplitter ) )
	{
		sError = "mapping token (=>) found more than once";
		return false;
	}

	m_dRegexps.Resize ( m_dRegexps.GetLength () + 1 );
	RegExp_t & tRegExp = m_dRegexps.Last();
	tRegExp.m_sFrom.SetBinary ( sRegExp, int ( sSplit-sRegExp ) );
	tRegExp.m_sTo = sSplit + strlen ( sSplitter );
	tRegExp.m_sFrom.Trim();
	tRegExp.m_sTo.Trim();

	RE2::Options tOptions;
	tOptions.set_encoding ( RE2::Options::Encoding::EncodingUTF8 );
	tRegExp.m_pRE2 = new RE2 ( tRegExp.m_sFrom.cstr(), tOptions );

	std::string sRE2Error;
	if ( !tRegExp.m_pRE2->CheckRewriteString ( tRegExp.m_sTo.cstr(), &sRE2Error ) )
	{
		sError.SetSprintf ( "\"%s => %s\" is not a valid mapping: %s", tRegExp.m_sFrom.cstr(), tRegExp.m_sTo.cstr(), sRE2Error.c_str() );
		SafeDelete ( tRegExp.m_pRE2 );
		m_dRegexps.Remove ( m_dRegexps.GetLength() - 1 );
		return false;
	}

	return true;
}


ISphFieldFilter * CSphFieldRegExps::Clone() const
{
	auto * pCloned = new CSphFieldRegExps;
	pCloned->m_dRegexps = m_dRegexps;

	return pCloned;
}
#endif


#if WITH_RE2
ISphFieldFilter * sphCreateRegexpFilter ( const CSphFieldFilterSettings & tFilterSettings, CSphString & sError )
{
	return new CSphFieldRegExps ( tFilterSettings.m_dRegexps, sError );
}
#else
ISphFieldFilter * sphCreateRegexpFilter ( const CSphFieldFilterSettings &, CSphString & )
{
	return nullptr;
}
#endif


/////////////////////////////////////////////////////////////////////////////
// GENERIC SOURCE
/////////////////////////////////////////////////////////////////////////////

bool ParseMorphFields ( const CSphString & sMorphology, const CSphString & sMorphFields, const CSphVector<CSphColumnInfo> & dFields, CSphBitvec & tMorphFields, CSphString & sError )
{
	if ( sMorphology.IsEmpty() || sMorphFields.IsEmpty() )
		return true;

	CSphString sFields = sMorphFields;
	sFields.ToLower();
	sFields.Trim();
	if ( !sFields.Length() )
		return true;

	OpenHash_T<int, int64_t, HashFunc_Int64_t> hFields;
	ARRAY_FOREACH ( i, dFields )
		hFields.Add ( sphFNV64 ( dFields[i].m_sName.cstr() ), i );

	StringBuilder_c sMissed;
	int iFieldsGot = 0;
	tMorphFields.Init ( dFields.GetLength() );
	tMorphFields.Set(); // all fields have morphology by default

	for ( const char * sCur=sFields.cstr(); ; )
	{
		while ( *sCur && ( sphIsSpace ( *sCur ) || *sCur==',' ) )
			++sCur;
		if ( !*sCur )
			break;

		const char * sStart = sCur;
		while ( *sCur && !sphIsSpace ( *sCur ) && *sCur!=',' )
			++sCur;

		if ( sCur==sStart )
			break;

		int * pField = hFields.Find ( sphFNV64 ( sStart, int ( sCur - sStart ) ) );
		if ( !pField )
		{
			const char * sSep = sMissed.GetLength() ? ", " : "";
			sMissed.Appendf ( "%s%.*s", sSep, (int)(sCur - sStart), sStart );
			break;
		}

		iFieldsGot++;
		tMorphFields.BitClear ( *pField );
	}

	// no fields set - need to reset bitvec to skip checks on indexing
	if ( !iFieldsGot )
		tMorphFields.Init ( 0 );

	if ( sMissed.GetLength() )
		sError.SetSprintf ( "morphology_skip_fields contains out of schema fields: %s", sMissed.cstr() );

	return ( !sMissed.GetLength() );
}

CSphSource::CSphSource ( const char * sName )
	: m_tSchema ( sName )
{
}


CSphSource::~CSphSource()
{
	SafeDelete ( m_pStripper );
}


void CSphSource::SetDict ( CSphDict * pDict )
{
	assert ( pDict );
	SafeAddRef ( pDict );
	m_pDict = pDict;
}


const CSphSourceStats & CSphSource::GetStats ()
{
	return m_tStats;
}


bool CSphSource::SetStripHTML ( const char * sExtractAttrs, const char * sRemoveElements,
	bool bDetectParagraphs, const char * sZones, CSphString & sError )
{
	if ( !m_pStripper )
		m_pStripper = new CSphHTMLStripper ( true );

	if ( !m_pStripper->SetIndexedAttrs ( sExtractAttrs, sError ) )
		return false;

	if ( !m_pStripper->SetRemovedElements ( sRemoveElements, sError ) )
		return false;

	if ( bDetectParagraphs )
		m_pStripper->EnableParagraphs ();

	if ( !m_pStripper->SetZones ( sZones, sError ) )
		return false;

	return true;
}


void CSphSource::SetFieldFilter ( ISphFieldFilter * pFilter )
{
	SafeAddRef ( pFilter );
	m_pFieldFilter = pFilter;
}

void CSphSource::SetTokenizer ( ISphTokenizer * pTokenizer )
{
	assert ( pTokenizer );
	SafeAddRef ( pTokenizer );
	m_pTokenizer = pTokenizer;
}


bool CSphSource::UpdateSchema ( CSphSchema * pInfo, CSphString & sError )
{
	assert ( pInfo );

	// fill it
	if ( pInfo->GetFieldsCount()==0 && pInfo->GetAttrsCount()==0 )
	{
		*pInfo = m_tSchema;
		return true;
	}

	// check it
	return m_tSchema.CompareTo ( *pInfo, sError );
}


void CSphSource::Setup ( const CSphSourceSettings & tSettings, StrVec_t * pWarnings )
{
	SetMinPrefixLen ( Max ( tSettings.RawMinPrefixLen(), 0 ) );
	m_iMinInfixLen = Max ( tSettings.m_iMinInfixLen, 0 );
	m_iMaxSubstringLen = Max ( tSettings.m_iMaxSubstringLen, 0 );
	m_iBoundaryStep = Max ( tSettings.m_iBoundaryStep, -1 );
	m_bIndexExactWords = tSettings.m_bIndexExactWords;
	m_iOvershortStep = Min ( Max ( tSettings.m_iOvershortStep, 0 ), 1 );
	m_iStopwordStep = Min ( Max ( tSettings.m_iStopwordStep, 0 ), 1 );
	m_bIndexSP = tSettings.m_bIndexSP;
	m_dPrefixFields = tSettings.m_dPrefixFields;
	m_dInfixFields = tSettings.m_dInfixFields;
	m_dStoredFields = tSettings.m_dStoredFields;
	m_dStoredOnlyFields = tSettings.m_dStoredOnlyFields;
	m_dColumnarAttrs = tSettings.m_dColumnarAttrs;
	m_dRowwiseAttrs = tSettings.m_dRowwiseAttrs;
	m_dColumnarStringsNoHash = tSettings.m_dColumnarStringsNoHash;
	m_bIndexFieldLens = tSettings.m_bIndexFieldLens;
	m_eEngine = tSettings.m_eEngine;

	m_tSchema.SetupFlags ( *this, false, pWarnings );
}


bool CSphSource::SetupMorphFields ( CSphString & sError )
{
	return ParseMorphFields ( m_pDict->GetSettings().m_sMorphology, m_pDict->GetSettings().m_sMorphFields, m_tSchema.GetFields(), m_tMorphFields, sError );
}


ISphHits * CSphSource::IterateJoinedHits ( CSphString & )
{
	static ISphHits dDummy;
	m_tDocInfo.m_tRowID = INVALID_ROWID; // pretend that's an eof
	return &dDummy;
}

/////////////////////////////////////////////////////////////////////////////
// DOCUMENT SOURCE
/////////////////////////////////////////////////////////////////////////////
CSphSource_Document::CSphBuildHitsState_t::CSphBuildHitsState_t ()
{
	Reset();
}

CSphSource_Document::CSphBuildHitsState_t::~CSphBuildHitsState_t ()
{
	Reset();
}

void CSphSource_Document::CSphBuildHitsState_t::Reset ()
{
	m_bProcessingHits = false;
	m_bDocumentDone = false;
	m_dFields = nullptr;
	m_dFieldLengths.Resize(0);
	m_iStartPos = 0;
	m_iHitPos = 0;
	m_iField = 0;
	m_iStartField = 0;
	m_iEndField = 0;
	m_iBuildLastStep = 1;

	ARRAY_FOREACH ( i, m_dTmpFieldStorage )
		SafeDeleteArray ( m_dTmpFieldStorage[i] );

	m_dTmpFieldStorage.Resize ( 0 );
	m_dTmpFieldPtrs.Resize ( 0 );
	m_dFiltered.Resize ( 0 );
}

CSphSource_Document::CSphSource_Document ( const char * sName )
	: CSphSource ( sName )
	, m_iMaxHits ( MAX_SOURCE_HITS )
{
}


bool CSphSource_Document::IterateDocument ( bool & bEOF, CSphString & sError )
{
	assert ( m_pTokenizer );
	assert ( !m_tState.m_bProcessingHits );

	m_tHits.Resize ( 0 );

	m_tState.Reset();
	m_tState.m_iEndField = m_iPlainFieldsLength;
	m_tState.m_dFieldLengths.Resize ( m_tState.m_iEndField );

	if ( m_pFieldFilter )
	{
		m_tState.m_dTmpFieldPtrs.Resize ( m_tState.m_iEndField );
		m_tState.m_dTmpFieldStorage.Resize ( m_tState.m_iEndField );

		ARRAY_FOREACH ( i, m_tState.m_dTmpFieldPtrs )
		{
			m_tState.m_dTmpFieldPtrs[i] = NULL;
			m_tState.m_dTmpFieldStorage[i] = NULL;
		}
	}

	m_dDocFields.Resize ( m_tSchema.GetFieldsCount() );
	for ( auto & i : m_dDocFields )
		i.Resize(0);

	// fetch next document
	while (true)
	{
		m_tState.m_dFields = NextDocument ( bEOF, sError );
		if ( bEOF )
			return ( sError.IsEmpty() );

		if ( !m_tState.m_dFields )
			return false;

		const int * pFieldLengths = GetFieldLengths ();
		for ( int iField=0; iField<m_tState.m_iEndField; iField++ )
		{
			m_tState.m_dFieldLengths[iField] = pFieldLengths[iField];

			if ( m_tSchema.GetField(iField).m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
			{
				int iFieldLen = m_tState.m_dFieldLengths[iField];
				m_dDocFields[iField].Resize(iFieldLen);
				memcpy ( m_dDocFields[iField].Begin(), m_tState.m_dFields[iField], iFieldLen );
			}
		}

		// tricky bit
		// we can only skip document indexing from here, IterateHits() is too late
		// so in case the user chose to skip documents with file field problems
		// we need to check for those here
		if ( m_eOnFileFieldError==FFE_SKIP_DOCUMENT || m_eOnFileFieldError==FFE_FAIL_INDEX )
		{
			bool bOk = true;
			for ( int iField=0; iField<m_tState.m_iEndField && bOk; iField++ )
			{
				const BYTE * sFilename = m_tState.m_dFields[iField];
				if ( m_tSchema.GetField(iField).m_bFilename )
					bOk &= CheckFileField ( sFilename );

				if ( !bOk && m_eOnFileFieldError==FFE_FAIL_INDEX )
				{
					sError.SetSprintf ( "error reading file field data (docid=" INT64_FMT ", filename=%s)",	m_dAttrs[0], sFilename );
					return false;
				}
			}
			if ( !bOk && m_eOnFileFieldError==FFE_SKIP_DOCUMENT )
				continue;
		}

		if ( m_pFieldFilter )
		{
			bool bHaveModifiedFields = false;
			for ( int iField=0; iField<m_tState.m_iEndField; iField++ )
			{
				if ( m_tSchema.GetField(iField).m_bFilename )
				{
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dFields[iField];
					continue;
				}

				CSphVector<BYTE> dFiltered;
				int iFilteredLen = m_pFieldFilter->Apply ( m_tState.m_dFields[iField], m_tState.m_dFieldLengths[iField], dFiltered, false );
				if ( iFilteredLen )
				{
					m_tState.m_dTmpFieldStorage[iField] = dFiltered.LeakData();
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dTmpFieldStorage[iField];
					m_tState.m_dFieldLengths[iField] = iFilteredLen;
					bHaveModifiedFields = true;
				} else
					m_tState.m_dTmpFieldPtrs[iField] = m_tState.m_dFields[iField];
			}

			if ( bHaveModifiedFields )
				m_tState.m_dFields = (BYTE **)&( m_tState.m_dTmpFieldPtrs[0] );
		}

		// we're good
		break;
	}

	++m_tStats.m_iTotalDocuments;
	return true;
}


// hack notification for joined fields
void CSphSource_Document::RowIDAssigned ( DocID_t tDocID, RowID_t tRowID )
{
	if ( HasJoinedFields() )
	{
		IDPair_t & tPair = m_dAllIds.Add();
		tPair.m_tDocID = tDocID;
		tPair.m_tRowID = tRowID;
	}
}


ISphHits * CSphSource_Document::IterateHits ( CSphString & sError )
{
	if ( m_tState.m_bDocumentDone )
		return NULL;

	m_tHits.Resize ( 0 );

	BuildHits ( sError, false );

	return &m_tHits;
}


bool CSphSource_Document::CheckFileField ( const BYTE * sField )
{
	CSphAutofile tFileSource;
	CSphString sError;

	if ( tFileSource.Open ( (const char *)sField, SPH_O_READ, sError )==-1 )
	{
		sphWarning ( "%s", sError.cstr() );
		return false;
	}

	int64_t iFileSize = tFileSource.GetSize();
	if ( iFileSize+16 > m_iMaxFileBufferSize )
	{
		sphWarning ( "file '%s' too big for a field (size=" INT64_FMT ", max_file_field_buffer=%d)", (const char *)sField, iFileSize, m_iMaxFileBufferSize );
		return false;
	}

	return true;
}


/// returns file size on success, and replaces *ppField with a pointer to data
/// returns -1 on failure (and emits a warning)
int CSphSource_Document::LoadFileField ( BYTE ** ppField, CSphString & sError )
{
	CSphAutofile tFileSource;

	BYTE * sField = *ppField;
	if ( tFileSource.Open ( (const char *)sField, SPH_O_READ, sError )==-1 )
	{
		sphWarning ( "%s", sError.cstr() );
		return -1;
	}

	int64_t iFileSize = tFileSource.GetSize();
	if ( iFileSize+16 > m_iMaxFileBufferSize )
	{
		sphWarning ( "file '%s' too big for a field (size=" INT64_FMT ", max_file_field_buffer=%d)", (const char *)sField, iFileSize, m_iMaxFileBufferSize );
		return -1;
	}

	int iFieldBytes = (int)iFileSize;
	if ( !iFieldBytes )
		return 0;

	int iBufSize = Max ( m_iReadFileBufferSize, 1 << sphLog2 ( iFieldBytes+15 ) );
	if ( m_iReadFileBufferSize < iBufSize )
		SafeDeleteArray ( m_pReadFileBuffer );

	if ( !m_pReadFileBuffer )
	{
		m_pReadFileBuffer = new char [ iBufSize ];
		m_iReadFileBufferSize = iBufSize;
	}

	if ( !tFileSource.Read ( m_pReadFileBuffer, iFieldBytes, sError ) )
	{
		sphWarning ( "read failed: %s", sError.cstr() );
		return -1;
	}

	m_pReadFileBuffer[iFieldBytes] = '\0';

	*ppField = (BYTE*)m_pReadFileBuffer;
	return iFieldBytes;
}


bool AddFieldLens ( CSphSchema & tSchema, bool bDynamic, CSphString & sError )
{
	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
	{
		CSphColumnInfo tCol;
		tCol.m_sName.SetSprintf ( "%s_len", tSchema.GetFieldName(i) );

		int iGot = tSchema.GetAttrIndex ( tCol.m_sName.cstr() );
		if ( iGot>=0 )
		{
			if ( tSchema.GetAttr(iGot).m_eAttrType==SPH_ATTR_TOKENCOUNT )
			{
				// looks like we already added these
				assert ( tSchema.GetAttr(iGot).m_sName==tCol.m_sName );
				return true;
			}

			sError.SetSprintf ( "attribute %s conflicts with index_field_lengths=1; remove it", tCol.m_sName.cstr() );
			return false;
		}

		tCol.m_eAttrType = SPH_ATTR_TOKENCOUNT;
		tSchema.AddAttr ( tCol, bDynamic ); // everything's dynamic at indexing time
	}
	return true;
}


bool CSphSource_Document::AddAutoAttrs ( CSphString & sError, StrVec_t * pDefaults )
{
	// id is the first attr
	if ( m_tSchema.GetAttr ( sphGetDocidName() ) )
	{
		assert ( m_tSchema.GetAttrIndex ( sphGetDocidName() )==0 );
		assert ( m_tSchema.GetAttr ( sphGetDocidName() )->m_eAttrType==SPH_ATTR_BIGINT );
	} else
	{
		CSphColumnInfo tCol ( sphGetDocidName() );
		tCol.m_eAttrType = SPH_ATTR_BIGINT;

		for ( const auto & i : m_dColumnarAttrs )
			if ( i==tCol.m_sName )
				tCol.m_uAttrFlags |= CSphColumnInfo::ATTR_COLUMNAR;

		m_tSchema.InsertAttr ( 0, tCol, true );

		if ( pDefaults )
			pDefaults->Insert ( 0, "" );
	}

	if ( m_tSchema.HasBlobAttrs() && !m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
	{
		CSphColumnInfo tCol ( sphGetBlobLocatorName() );
		tCol.m_eAttrType = SPH_ATTR_BIGINT;

		// should be right after docid
		m_tSchema.InsertAttr ( 1, tCol, true );

		if ( pDefaults )
			pDefaults->Insert ( 1, "" );
	}

	// rebuild locators in the schema
	const char * szTmpColName = "$_tmp";
	CSphColumnInfo tCol ( szTmpColName, SPH_ATTR_BIGINT );
	m_tSchema.AddAttr ( tCol, true );
	m_tSchema.RemoveAttr ( szTmpColName, true );

	// auto-computed length attributes
	if ( m_bIndexFieldLens && !AddFieldLens ( m_tSchema, true, sError ) )
		return false;

	return true;
}


void CSphSource_Document::AllocDocinfo()
{
	// tricky bit
	// with in-config schema, attr storage gets allocated in Setup() when source is initially created
	// so when this AddAutoAttrs() additionally changes the count, we have to change the number of attributes
	// but Reset() prohibits that, because that is usually a programming mistake, hence the Swap() dance
	CSphMatch tNew;
	tNew.Reset ( m_tSchema.GetRowSize() );
	Swap ( m_tDocInfo, tNew );

	m_dStrAttrs.Resize ( m_tSchema.GetAttrsCount() );
	m_dAttrs.Resize ( m_tSchema.GetAttrsCount() );

	if ( m_bIndexFieldLens && m_tSchema.GetAttrsCount() && m_tSchema.GetFieldsCount() )
	{
		int iFirst = m_tSchema.GetAttrId_FirstFieldLen();
		assert ( m_tSchema.GetAttr ( iFirst ).m_eAttrType==SPH_ATTR_TOKENCOUNT );
		assert ( m_tSchema.GetAttr ( iFirst+m_tSchema.GetFieldsCount()-1 ).m_eAttrType==SPH_ATTR_TOKENCOUNT );

		m_pFieldLengthAttrs = m_tDocInfo.m_pDynamic + ( m_tSchema.GetAttr ( iFirst ).m_tLocator.m_iBitOffset / 32 );
	}
}

//////////////////////////////////////////////////////////////////////////
// HIT GENERATORS
//////////////////////////////////////////////////////////////////////////

bool CSphSource_Document::BuildZoneHits ( RowID_t tRowID, BYTE uCode )
{
	switch (uCode)
	{
	case MAGIC_CODE_SENTENCE:
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_SENTENCE) ), m_tState.m_iHitPos } );
		m_tState.m_iBuildLastStep = 1;
		return true;
	case MAGIC_CODE_PARAGRAPH:
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_SENTENCE) ), m_tState.m_iHitPos } );
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_PARAGRAPH) ), m_tState.m_iHitPos } );
		m_tState.m_iBuildLastStep = 1;
		return true;
	case MAGIC_CODE_ZONE:
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_SENTENCE) ), m_tState.m_iHitPos } );
		m_tHits.Add ( { tRowID, m_pDict->GetWordID ( (BYTE *)const_cast<char*>(MAGIC_WORD_PARAGRAPH) ), m_tState.m_iHitPos } );
		{
			BYTE * pZone = (BYTE*)const_cast<char *> ( m_pTokenizer->GetBufferPtr() );
			BYTE * pEnd = pZone;
			while ( *pEnd && *pEnd!=MAGIC_CODE_ZONE )
				++pEnd;

			if ( *pEnd==MAGIC_CODE_ZONE )
			{
				*pEnd = '\0';
				m_tHits.Add ( { tRowID, m_pDict->GetWordID ( pZone-1 ), m_tState.m_iHitPos } );
				m_pTokenizer->SetBufferPtr ( (const char *) pEnd+1 );
			}
		}
		m_tState.m_iBuildLastStep = 1;
		return true;
	default:
		return false;
	}
}


// track blended start and reset on not blended token
static int TrackBlendedStart ( const ISphTokenizer * pTokenizer, int iBlendedHitsStart, int iHitsCount )
{
	if ( pTokenizer->TokenIsBlended() )
		return iHitsCount;

	return pTokenizer->TokenIsBlendedPart () ? iBlendedHitsStart : -1;
}


#define BUILD_SUBSTRING_HITS_COUNT 4

void CSphSource_Document::BuildSubstringHits ( RowID_t tRowID, bool bPayload, ESphWordpart eWordpart, bool bSkipEndMarker )
{
	bool bPrefixField = ( eWordpart==SPH_WORDPART_PREFIX );
	bool bInfixMode = m_iMinInfixLen > 0;

	int iMinInfixLen = bPrefixField ? GetMinPrefixLen ( false ) : m_iMinInfixLen;
	if ( !m_tState.m_bProcessingHits )
		m_tState.m_iBuildLastStep = 1;

	BYTE * sWord = NULL;
	BYTE sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	int iIterHitCount = BUILD_SUBSTRING_HITS_COUNT;
	if ( bPrefixField )
		iIterHitCount += SPH_MAX_WORD_LEN - GetMinPrefixLen ( false );
	else
		iIterHitCount += ( ( m_iMinInfixLen+SPH_MAX_WORD_LEN ) * ( SPH_MAX_WORD_LEN-m_iMinInfixLen ) / 2 );

	// FIELDEND_MASK at blended token stream should be set for HEAD token too
	int iBlendedHitsStart = -1;

	// index all infixes
	while ( ( m_iMaxHits==0 || m_tHits.GetLength()+iIterHitCount<m_iMaxHits )
		&& ( sWord = m_pTokenizer->GetToken() )!=NULL )
	{
		int iLastBlendedStart = TrackBlendedStart ( m_pTokenizer, iBlendedHitsStart, m_tHits.GetLength() );

		if ( !bPayload )
		{
			HITMAN::AddPos ( &m_tState.m_iHitPos, Max ( m_tState.m_iBuildLastStep + m_pTokenizer->GetOvershortCount()*m_iOvershortStep, 0 ) );
			if ( m_pTokenizer->GetBoundary() )
				HITMAN::AddPos ( &m_tState.m_iHitPos, m_iBoundaryStep );
			m_tState.m_iBuildLastStep = 1;
		}

		if ( BuildZoneHits ( tRowID, *sWord ) )
			continue;

		int iLen = m_pTokenizer->GetLastTokenLen ();

		// always index full word (with magic head/tail marker(s))
		auto iBytes = (int) strlen ( (const char*)sWord );
		memcpy ( sBuf + 1, sWord, iBytes );
		sBuf[iBytes+1] = '\0';

		SphWordID_t uExactWordid = 0;
		if ( m_bIndexExactWords )
		{
			sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			uExactWordid = m_pDict->GetWordIDNonStemmed ( sBuf );
		}

		sBuf[0] = MAGIC_WORD_HEAD;

		// stemmed word w/markers
		SphWordID_t iWord = m_pDict->GetWordIDWithMarkers ( sBuf );
		if ( !iWord )
		{
			m_tState.m_iBuildLastStep = m_iStopwordStep;
			continue;
		}

		if ( m_bIndexExactWords )
			m_tHits.Add ( { tRowID, uExactWordid, m_tState.m_iHitPos } );
		iBlendedHitsStart = iLastBlendedStart;
		m_tHits.Add ( { tRowID, iWord, m_tState.m_iHitPos } );
		m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;

		// restore stemmed word
		auto iStemmedLen = (int) strlen ( ( const char *)sBuf );
		sBuf [iStemmedLen - 1] = '\0';

		// stemmed word w/o markers
		if ( strcmp ( (const char *)sBuf + 1, (const char *)sWord ) )
			m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sBuf + 1, iStemmedLen - 2, true ), m_tState.m_iHitPos } );

		// restore word
		memcpy ( sBuf + 1, sWord, iBytes );
		sBuf[iBytes+1] = MAGIC_WORD_TAIL;
		sBuf[iBytes+2] = '\0';

		// if there are no infixes, that's it
		if ( iMinInfixLen > iLen )
		{
			// index full word
			m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sWord ), m_tState.m_iHitPos } );
			continue;
		}

		// process all infixes
		int iMaxStart = bPrefixField ? 0 : ( iLen - iMinInfixLen );

		BYTE * sInfix = sBuf + 1;

		for ( int iStart=0; iStart<=iMaxStart; iStart++ )
		{
			BYTE * sInfixEnd = sInfix;
			for ( int i = 0; i < iMinInfixLen; i++ )
				sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );

			int iMaxSubLen = ( iLen-iStart );
			if ( m_iMaxSubstringLen )
				iMaxSubLen = Min ( m_iMaxSubstringLen, iMaxSubLen );

			for ( int i=iMinInfixLen; i<=iMaxSubLen; i++ )
			{
				m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sInfix, int ( sInfixEnd-sInfix ), false ), m_tState.m_iHitPos } );

				// word start: add magic head
				if ( bInfixMode && iStart==0 )
					m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sInfix - 1, int ( sInfixEnd-sInfix ) + 1, false ), m_tState.m_iHitPos } );

				// word end: add magic tail
				if ( bInfixMode && i==iLen-iStart )
					m_tHits.Add ( { tRowID, m_pDict->GetWordID ( sInfix, int ( sInfixEnd-sInfix ) + 1, false ), m_tState.m_iHitPos } );

				sInfixEnd += m_pTokenizer->GetCodepointLength ( *sInfixEnd );
			}

			sInfix += m_pTokenizer->GetCodepointLength ( *sInfix );
		}
	}

	m_tState.m_bProcessingHits = ( sWord!=NULL );

	// mark trailing hits
	// and compute fields lengths
	if ( !bSkipEndMarker && !m_tState.m_bProcessingHits && m_tHits.GetLength() )
	{
		CSphWordHit * pTail = const_cast < CSphWordHit * > ( &m_tHits.Last() );

		if ( m_pFieldLengthAttrs )
			m_pFieldLengthAttrs [ HITMAN::GetField ( pTail->m_uWordPos ) ] = HITMAN::GetPos ( pTail->m_uWordPos );

		Hitpos_t uEndPos = pTail->m_uWordPos;
		if ( iBlendedHitsStart>=0 )
		{
			assert ( iBlendedHitsStart>=0 && iBlendedHitsStart<m_tHits.GetLength() );
			Hitpos_t uBlendedPos = m_tHits[iBlendedHitsStart].m_uWordPos;
			uEndPos = Min ( uEndPos, uBlendedPos );
		}

		// set end marker for all tail hits
		const CSphWordHit * pStart = m_tHits.Begin();
		while ( pStart<=pTail && uEndPos<=pTail->m_uWordPos )
		{
			HITMAN::SetEndMarker ( &pTail->m_uWordPos );
			pTail--;
		}
	}
}


#define BUILD_REGULAR_HITS_COUNT 6

void CSphSource_Document::BuildRegularHits ( RowID_t tRowID, bool bPayload, bool bSkipEndMarker )
{
	bool bWordDict = m_pDict->GetSettings().m_bWordDict;
	bool bGlobalPartialMatch = !bWordDict && ( GetMinPrefixLen ( bWordDict ) > 0 || m_iMinInfixLen > 0 );

	if ( !m_tState.m_bProcessingHits )
		m_tState.m_iBuildLastStep = 1;

	BYTE * sWord = NULL;
	BYTE sBuf [ 16+3*SPH_MAX_WORD_LEN ];

	// FIELDEND_MASK at last token stream should be set for HEAD token too
	int iBlendedHitsStart = -1;
	bool bMorphDisabled = ( m_tMorphFields.GetBits()>0 && !m_tMorphFields.BitGet ( m_tState.m_iField ) );

	// index words only
	while ( ( m_iMaxHits==0 || m_tHits.GetLength()+BUILD_REGULAR_HITS_COUNT<m_iMaxHits )
		&& ( sWord = m_pTokenizer->GetToken() )!=NULL )
	{
		int iLastBlendedStart = TrackBlendedStart ( m_pTokenizer, iBlendedHitsStart, m_tHits.GetLength() );

		if ( !bPayload )
		{
			HITMAN::AddPos ( &m_tState.m_iHitPos, Max ( m_tState.m_iBuildLastStep + m_pTokenizer->GetOvershortCount()*m_iOvershortStep, 0 ) );
			if ( m_pTokenizer->GetBoundary() )
				HITMAN::AddPos ( &m_tState.m_iHitPos, m_iBoundaryStep );
		}

		if ( BuildZoneHits ( tRowID, *sWord ) )
			continue;

		if ( bGlobalPartialMatch )
		{
			auto iBytes = strlen ( (const char*)sWord );
			memcpy ( sBuf + 1, sWord, iBytes );
			sBuf[0] = MAGIC_WORD_HEAD;
			sBuf[iBytes+1] = '\0';
			m_tHits.Add ( { tRowID, m_pDict->GetWordIDWithMarkers ( sBuf ), m_tState.m_iHitPos } );
		}

		ESphTokenMorph eMorph = m_pTokenizer->GetTokenMorph();
		if ( m_bIndexExactWords && eMorph!=SPH_TOKEN_MORPH_GUESS )
		{
			auto iBytes = strlen ( (const char*)sWord );
			memcpy ( sBuf + 1, sWord, iBytes );
			sBuf[0] = MAGIC_WORD_HEAD_NONSTEMMED;
			sBuf[iBytes+1] = '\0';
		}

		if ( m_bIndexExactWords && ( eMorph==SPH_TOKEN_MORPH_ORIGINAL || bMorphDisabled ) )
		{
			// can not use GetWordID here due to exception vs missed hit, ie
			// stemmed sWord hasn't got added to hit stream but might be added as exception to dictionary
			// that causes error at hit sorting phase \ dictionary HitblockPatch
			if ( !m_pDict->GetSettings().m_bStopwordsUnstemmed )
				m_pDict->ApplyStemmers ( sWord );

			if ( !m_pDict->IsStopWord ( sWord ) )
				m_tHits.Add ( { tRowID, m_pDict->GetWordIDNonStemmed ( sBuf ), m_tState.m_iHitPos } );

			m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;
			continue;
		}

		SphWordID_t iWord = ( eMorph==SPH_TOKEN_MORPH_GUESS )
			? m_pDict->GetWordIDNonStemmed ( sWord ) // tokenizer did morphology => dict must not stem
			: m_pDict->GetWordID ( sWord ); // tokenizer did not => stemmers can be applied
		if ( iWord )
		{
#if 0
			if ( HITMAN::GetPos ( m_tState.m_iHitPos )==1 )
				printf ( "\n" );
			printf ( "doc %d. pos %d. %s\n", uDocid, HITMAN::GetPos ( m_tState.m_iHitPos ), sWord );
#endif
			iBlendedHitsStart = iLastBlendedStart;
			m_tState.m_iBuildLastStep = m_pTokenizer->TokenIsBlended() ? 0 : 1;
			m_tHits.Add ( { tRowID, iWord, m_tState.m_iHitPos } );
			if ( m_bIndexExactWords && eMorph!=SPH_TOKEN_MORPH_GUESS )
				m_tHits.Add ( { tRowID, m_pDict->GetWordIDNonStemmed ( sBuf ), m_tState.m_iHitPos } );
		} else
		{
			// need to count all blended part tokens to match query
			m_tState.m_iBuildLastStep = ( m_pTokenizer->TokenIsBlendedPart() ? 1 : m_iStopwordStep );
		}
	}

	m_tState.m_bProcessingHits = ( sWord!=NULL );

	// mark trailing hit
	// and compute field lengths
	if ( !bSkipEndMarker && !m_tState.m_bProcessingHits && m_tHits.GetLength() )
	{
		auto * pTail = const_cast < CSphWordHit * > ( &m_tHits.Last() );

		if ( m_pFieldLengthAttrs )
			m_pFieldLengthAttrs [ HITMAN::GetField ( pTail->m_uWordPos ) ] = HITMAN::GetPos ( pTail->m_uWordPos );

		Hitpos_t uEndPos = pTail->m_uWordPos;
		if ( iBlendedHitsStart>=0 )
		{
			assert ( iBlendedHitsStart>=0 && iBlendedHitsStart<m_tHits.GetLength() );
			Hitpos_t uBlendedPos = m_tHits[iBlendedHitsStart].m_uWordPos;
			uEndPos = Min ( uEndPos, uBlendedPos );
		}

		// set end marker for all tail hits
		const CSphWordHit * pStart = m_tHits.Begin();
		while ( pStart<=pTail && uEndPos<=pTail->m_uWordPos )
		{
			HITMAN::SetEndMarker ( &pTail->m_uWordPos );
			--pTail;
		}
	}
}


void CSphSource_Document::BuildHits ( CSphString & sError, bool bSkipEndMarker )
{
	RowID_t tRowID = m_tDocInfo.m_tRowID;

	for ( ; m_tState.m_iField<m_tState.m_iEndField; m_tState.m_iField++ )
	{
		if ( !m_tState.m_bProcessingHits )
		{
			// get that field
			BYTE * sField = m_tState.m_dFields[m_tState.m_iField-m_tState.m_iStartField];
			int iFieldBytes = m_tState.m_dFieldLengths[m_tState.m_iField-m_tState.m_iStartField];
			if ( !sField || !(*sField) || !iFieldBytes )
				continue;

			// load files
			const BYTE * sTextToIndex;
			const CSphColumnInfo & tField = m_tSchema.GetField(m_tState.m_iField);
			if ( tField.m_bFilename )
			{
				LoadFileField ( &sField, sError );
				sTextToIndex = sField;
				iFieldBytes = (int) strlen ( (char*)sField );

				if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
				{
					m_dDocFields[m_tState.m_iField].Resize(iFieldBytes);
					memcpy ( m_dDocFields[m_tState.m_iField].Begin(), sField, iFieldBytes );
				}

				if ( m_pFieldFilter && iFieldBytes )
				{
					m_tState.m_dFiltered.Resize ( 0 );
					int iFiltered = m_pFieldFilter->Apply ( sTextToIndex, iFieldBytes, m_tState.m_dFiltered, false );
					if ( iFiltered )
					{
						sTextToIndex = m_tState.m_dFiltered.Begin();
						iFieldBytes = iFiltered;
					}
				}
			} else
				sTextToIndex = sField;

			if ( iFieldBytes<=0 )
				continue;

			// strip html
			if ( m_pStripper )
			{
				m_pStripper->Strip ( const_cast<BYTE*>(sTextToIndex) );
				iFieldBytes = (int) strlen ( (char*)const_cast<BYTE*>(sTextToIndex) );
			}

			// tokenize and build hits
			m_tStats.m_iTotalBytes += iFieldBytes;

			m_pTokenizer->BeginField ( m_tState.m_iField );
			m_pTokenizer->SetBuffer ( const_cast<BYTE*> ( sTextToIndex ), iFieldBytes );

			m_tState.m_iHitPos = HITMAN::Create ( m_tState.m_iField, m_tState.m_iStartPos );
		}

		const CSphColumnInfo & tField = m_tSchema.GetField ( m_tState.m_iField );

		if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED )
		{
			if ( tField.m_eWordpart!=SPH_WORDPART_WHOLE )
				BuildSubstringHits ( tRowID, tField.m_bPayload, tField.m_eWordpart, bSkipEndMarker );
			else
				BuildRegularHits ( tRowID, tField.m_bPayload, bSkipEndMarker );
		}

		if ( m_tState.m_bProcessingHits )
			break;
	}

	m_tState.m_bDocumentDone = !m_tState.m_bProcessingHits;
}

//////////////////////////////////////////////////////////////////////////

CSphVector<int64_t> * CSphSource_Document::GetFieldMVA ( int iAttr )
{
	return &m_dMvas[iAttr];
}


const CSphString & CSphSource_Document::GetStrAttr ( int iAttr )
{
	return m_dStrAttrs[iAttr];
}


SphAttr_t CSphSource_Document::GetAttr ( int iAttr )
{
	return m_dAttrs[iAttr];
}


void CSphSource_Document::GetDocFields ( CSphVector<VecTraits_T<BYTE>> & dFields )
{
	dFields.Resize ( m_dDocFields.GetLength() );
	ARRAY_FOREACH ( i, m_dDocFields )
		dFields[i] = VecTraits_T<BYTE>( m_dDocFields[i].Begin(), m_dDocFields[i].GetLength() );
}


void CSphSource_Document::ParseFieldMVA ( int iAttr, const char * szValue )
{
	if ( !szValue )
		return;

	const char * pPtr = szValue;
	const char * pDigit = NULL;
	const int MAX_NUMBER_LEN = 64;
	char szBuf [MAX_NUMBER_LEN];

	while ( *pPtr )
	{
		if ( ( *pPtr>='0' && *pPtr<='9' ) || *pPtr=='-' )
		{
			if ( !pDigit )
				pDigit = pPtr;
		} else
		{
			if ( pDigit )
			{
				if ( pPtr - pDigit < MAX_NUMBER_LEN )
				{
					strncpy ( szBuf, pDigit, pPtr - pDigit );
					szBuf [pPtr - pDigit] = '\0';
					m_dMvas[iAttr].Add ( sphToInt64 ( szBuf ) );
				}

				pDigit = NULL;
			}
		}

		pPtr++;
	}

	if ( pDigit )
		m_dMvas[iAttr].Add ( sphToInt64 ( pDigit ) );
}



bool SchemaConfigureCheckAttribute ( const CSphSchema & tSchema, const CSphColumnInfo & tCol, CSphString & sError )
{
	if ( tCol.m_sName.IsEmpty() )
	{
		sError.SetSprintf ( "column number %d has no name", tCol.m_iIndex );
		return false;
	}

	if ( tSchema.GetAttr ( tCol.m_sName.cstr() ) )
	{
		sError.SetSprintf ( "can not add multiple attributes with same name '%s'", tCol.m_sName.cstr () );
		return false;
	}

	if ( CSphSchema::IsReserved ( tCol.m_sName.cstr() ) )
	{
		sError.SetSprintf ( "%s is not a valid attribute name", tCol.m_sName.cstr() );
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////


void sphSetJsonOptions ( bool bStrict, bool bAutoconvNumbers, bool bKeynamesToLowercase )
{
	g_bJsonStrict = bStrict;
	g_bJsonAutoconvNumbers = bAutoconvNumbers;
	g_bJsonKeynamesToLowercase = bKeynamesToLowercase;
}


//////////////////////////////////////////////////////////////////////////

int sphDictCmp ( const char * pStr1, int iLen1, const char * pStr2, int iLen2 )
{
	assert ( pStr1 && pStr2 );
	assert ( iLen1 && iLen2 );
	const int iCmpLen = Min ( iLen1, iLen2 );
	return memcmp ( pStr1, pStr2, iCmpLen );
}

int sphDictCmpStrictly ( const char * pStr1, int iLen1, const char * pStr2, int iLen2 )
{
	assert ( pStr1 && pStr2 );
	assert ( iLen1 && iLen2 );
	const int iCmpLen = Min ( iLen1, iLen2 );
	const int iCmpRes = memcmp ( pStr1, pStr2, iCmpLen );
	return iCmpRes==0 ? iLen1-iLen2 : iCmpRes;
}


ISphWordlist::Args_t::Args_t ( bool bPayload, int iExpansionLimit, bool bHasExactForms, ESphHitless eHitless, cRefCountedRefPtr_t pIndexData )
	: m_bPayload ( bPayload )
	, m_iExpansionLimit ( iExpansionLimit )
	, m_bHasExactForms ( bHasExactForms )
	, m_eHitless ( eHitless )
	, m_pIndexData ( pIndexData )
{
	m_sBuf.Reserve ( 2048 * SPH_MAX_WORD_LEN * 3 );
	m_dExpanded.Reserve ( 2048 );
	m_pPayload = nullptr;
	m_iTotalDocs = 0;
	m_iTotalHits = 0;
}


ISphWordlist::Args_t::~Args_t ()
{
	SafeDelete ( m_pPayload );
}


void ISphWordlist::Args_t::AddExpanded ( const BYTE * sName, int iLen, int iDocs, int iHits )
{
	SphExpanded_t & tExpanded = m_dExpanded.Add();
	tExpanded.m_iDocs = iDocs;
	tExpanded.m_iHits = iHits;
	int iOff = m_sBuf.GetLength();
	tExpanded.m_iNameOff = iOff;

	m_sBuf.Resize ( iOff + iLen + 1 );
	memcpy ( m_sBuf.Begin()+iOff, sName, iLen );
	m_sBuf[iOff+iLen] = '\0';
}


const char * ISphWordlist::Args_t::GetWordExpanded ( int iIndex ) const
{
	assert ( m_dExpanded[iIndex].m_iNameOff<m_sBuf.GetLength() );
	return (const char *)m_sBuf.Begin() + m_dExpanded[iIndex].m_iNameOff;
}


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


struct DictEntryDiskPayload_t
{
	explicit DictEntryDiskPayload_t ( bool bPayload, ESphHitless eHitless )
	{
		m_bPayload = bPayload;
		m_eHitless = eHitless;
		if ( bPayload )
			m_dWordPayload.Reserve ( 1000 );

		m_dWordExpand.Reserve ( 1000 );
		m_dWordBuf.Reserve ( 8096 );
	}

	void Add ( const CSphDictEntry & tWord, int iWordLen )
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

static bool operator < ( const InfixBlock_t & a, const char * b )
{
	return strcmp ( a.m_sInfix, b )<0;
}

static bool operator == ( const InfixBlock_t & a, const char * b )
{
	return strcmp ( a.m_sInfix, b )==0;
}

static bool operator < ( const char * a, const InfixBlock_t & b )
{
	return strcmp ( a, b.m_sInfix )<0;
}


bool sphLookupInfixCheckpoints ( const char * sInfix, int iBytes, const BYTE * pInfixes, const CSphVector<InfixBlock_t> & dInfixBlocks, int iInfixCodepointBytes, CSphVector<DWORD> & dCheckpoints )
{
	assert ( pInfixes );

	char dInfixBuf[3*SPH_MAX_WORD_LEN+4];
	memcpy ( dInfixBuf, sInfix, iBytes );
	dInfixBuf[iBytes] = '\0';

	// lookup block
	int iBlock = FindSpan ( dInfixBlocks, dInfixBuf );
	if ( iBlock<0 )
		return false;
	const BYTE * pBlock = pInfixes + dInfixBlocks[iBlock].m_iOffset;

	// decode block and check for exact infix match
	// block entry is { byte edit_code, byte[] key_append, zint data_len, zint data_deltas[] }
	// zero edit_code marks block end
	BYTE sKey[32];
	while (true)
	{
		// unpack next key
		int iCode = *pBlock++;
		if ( !iCode )
			break;

		BYTE * pOut = sKey;
		if ( iInfixCodepointBytes==1 )
		{
			pOut = sKey + ( iCode>>4 );
			iCode &= 15;
			while ( iCode-- )
				*pOut++ = *pBlock++;
		} else
		{
			int iKeep = ( iCode>>4 );
			while ( iKeep-- )
				pOut += sphUtf8CharBytes ( *pOut ); ///< wtf? *pOut (=sKey) is NOT initialized?
			assert ( pOut-sKey<=(int)sizeof(sKey) );
			iCode &= 15;
			while ( iCode-- )
			{
				int i = sphUtf8CharBytes ( *pBlock );
				while ( i-- )
					*pOut++ = *pBlock++;
			}
			assert ( pOut-sKey<=(int)sizeof(sKey) );
		}
		assert ( pOut-sKey<(int)sizeof(sKey) );
#ifndef NDEBUG
		*pOut = '\0'; // handy for debugging, but not used for real matching
#endif

		if ( pOut==sKey+iBytes && memcmp ( sKey, dInfixBuf, iBytes )==0 )
		{
			// found you! decompress the data
			int iLast = 0;
			int iPackedLen = sphUnzipInt ( pBlock );
			const BYTE * pMax = pBlock + iPackedLen;
			while ( pBlock<pMax )
			{
				iLast += sphUnzipInt ( pBlock );
				dCheckpoints.Add ( (DWORD)iLast );
			}
			return true;
		}

		int iSkip = sphUnzipInt ( pBlock );
		pBlock += iSkip;
	}
	return false;
}


// calculate length, upto iInfixCodepointBytes chars from infix start
int sphGetInfixLength ( const char * sInfix, int iBytes, int iInfixCodepointBytes )
{
	int iBytes1 = Min ( 6, iBytes );
	if ( iInfixCodepointBytes!=1 )
	{
		int iCharsLeft = 6;
		const char * s = sInfix;
		const char * sMax = sInfix + iBytes;
		while ( iCharsLeft-- && s<sMax )
			s += sphUtf8CharBytes(*s);
		iBytes1 = (int)( s - sInfix );
	}

	return iBytes1;
}


static int BuildUtf8Offsets ( const char * sWord, int iLen, int * pOff, int DEBUGARG ( iBufSize ) )
{
	const BYTE * s = (const BYTE *)sWord;
	const BYTE * sEnd = s + iLen;
	int * pStartOff = pOff;
	*pOff = 0;
	pOff++;
	while ( s<sEnd )
	{
		sphUTF8Decode ( s );
		*pOff = int ( s-(const BYTE *)sWord );
		pOff++;
	}
	assert ( pOff-pStartOff<iBufSize );
	return int ( pOff - pStartOff - 1 );
}

void sphBuildNGrams ( const char * sWord, int iLen, char cDelimiter, CSphVector<char> & dNGrams )
{
	int dOff[SPH_MAX_WORD_LEN+1];
	int iCodepoints = BuildUtf8Offsets ( sWord, iLen, dOff, sizeof ( dOff ) );
	if ( iCodepoints<3 )
		return;

	dNGrams.Reserve ( iLen*3 );
	for ( int iChar=0; iChar<=iCodepoints-3; iChar++ )
	{
		int iStart = dOff[iChar];
		int iEnd = dOff[iChar+3];
		int iGramLen = iEnd - iStart;

		char * sDst = dNGrams.AddN ( iGramLen + 1 );
		memcpy ( sDst, sWord+iStart, iGramLen );
		sDst[iGramLen] = cDelimiter;
	}
	// n-grams split by delimiter
	// however it's still null terminated
	dNGrams.Last() = '\0';
}

template <typename T>
int sphLevenshtein ( const T * sWord1, int iLen1, const T * sWord2, int iLen2 )
{
	if ( !iLen1 )
		return iLen2;
	if ( !iLen2 )
		return iLen1;

	int dTmp [ 3*SPH_MAX_WORD_LEN+1 ]; // FIXME!!! remove extra length after utf8->codepoints conversion

	for ( int i=0; i<=iLen2; i++ )
		dTmp[i] = i;

	for ( int i=0; i<iLen1; i++ )
	{
		dTmp[0] = i+1;
		int iWord1 = sWord1[i];
		int iDist = i;

		for ( int j=0; j<iLen2; j++ )
		{
			int iDistNext = dTmp[j+1];
			dTmp[j+1] = ( iWord1==sWord2[j] ? iDist : ( 1 + Min ( Min ( iDist, iDistNext ), dTmp[j] ) ) );
			iDist = iDistNext;
		}
	}

	return dTmp[iLen2];
}

int sphLevenshtein ( const char * sWord1, int iLen1, const char * sWord2, int iLen2 )
{
	return sphLevenshtein<char> ( sWord1, iLen1, sWord2, iLen2 );
}

int sphLevenshtein ( const int * sWord1, int iLen1, const int * sWord2, int iLen2 )
{
	return sphLevenshtein<int> ( sWord1, iLen1, sWord2, iLen2 );
}

// sort by distance(uLen) desc, checkpoint index(uOff) asc
struct CmpHistogram_fn
{
	inline bool IsLess ( const Slice_t & a, const Slice_t & b ) const
	{
		return ( a.m_uLen>b.m_uLen || ( a.m_uLen==b.m_uLen && a.m_uOff<b.m_uOff ) );
	}
};

// convert utf8 to unicode string
static int DecodeUtf8 ( const BYTE * sWord, int * pBuf )
{
	if ( !sWord )
		return 0;

	int * pCur = pBuf;
	while ( *sWord )
	{
		*pCur = sphUTF8Decode ( sWord );
		pCur++;
	}
	return int ( pCur - pBuf );
}


bool SuggestResult_t::SetWord ( const char * sWord, const ISphTokenizer * pTok, bool bUseLastWord )
{
	TokenizerRefPtr_c pTokenizer ( pTok->Clone ( SPH_CLONE_QUERY_LIGHTWEIGHT ) );
	pTokenizer->SetBuffer ( (BYTE *)const_cast<char*>(sWord), (int) strlen ( sWord ) );

	const BYTE * pToken = pTokenizer->GetToken();
	for ( ; pToken!=NULL; )
	{
		m_sWord = (const char *)pToken;
		if ( !bUseLastWord )
			break;

		if ( pTokenizer->TokenIsBlended() )
			pTokenizer->SkipBlended();
		pToken = pTokenizer->GetToken();
	}


	m_iLen = m_sWord.Length();
	m_iCodepoints = DecodeUtf8 ( (const BYTE *)m_sWord.cstr(), m_dCodepoints );
	m_bUtf8 = ( m_iCodepoints!=m_iLen );

	bool bValidWord = ( m_iCodepoints>=3 );
	if ( bValidWord )
		sphBuildNGrams ( m_sWord.cstr(), m_iLen, '\0', m_dTrigrams );

	return bValidWord;
}

void SuggestResult_t::Flattern ( int iLimit )
{
	int iCount = Min ( m_dMatched.GetLength(), iLimit );
	m_dMatched.Resize ( iCount );
}

struct SliceInt_t
{
	int		m_iOff;
	int		m_iEnd;
};

static void SuggestGetChekpoints ( const ISphWordlistSuggest * pWordlist, int iInfixCodepointBytes, const CSphVector<char> & dTrigrams, CSphVector<Slice_t> & dCheckpoints, SuggestResult_t & tStats )
{
	CSphVector<DWORD> dWordCp; // FIXME!!! add mask that trigram matched
	// v1 - current index, v2 - end index
	CSphVector<SliceInt_t> dMergeIters;

	int iReserveLen = 0;
	int iLastLen = 0;
	const char * sTrigram = dTrigrams.Begin();
	const char * sTrigramEnd = sTrigram + dTrigrams.GetLength();
	while (true)
	{
		auto iTrigramLen = (int) strlen ( sTrigram );
		int iInfixLen = sphGetInfixLength ( sTrigram, iTrigramLen, iInfixCodepointBytes );

		// count how many checkpoint we will get
		iReserveLen = Max ( iReserveLen, dWordCp.GetLength () - iLastLen );
		iLastLen = dWordCp.GetLength();

		dMergeIters.Add().m_iOff = dWordCp.GetLength();
		pWordlist->SuffixGetChekpoints ( tStats, sTrigram, iInfixLen, dWordCp );

		sTrigram += iTrigramLen + 1;
		if ( sTrigram>=sTrigramEnd )
			break;

		if ( sphInterrupted() )
			return;
	}
	if ( !dWordCp.GetLength() )
		return;

	for ( int i=0; i<dMergeIters.GetLength()-1; i++ )
	{
		dMergeIters[i].m_iEnd = dMergeIters[i+1].m_iOff;
	}
	dMergeIters.Last().m_iEnd = dWordCp.GetLength();

	// v1 - checkpoint index, v2 - checkpoint count
	dCheckpoints.Reserve ( iReserveLen );
	dCheckpoints.Resize ( 0 );

	// merge sorting of already ordered checkpoints
	while (true)
	{
		DWORD iMinCP = UINT_MAX;
		DWORD iMinIndex = UINT_MAX;
		ARRAY_FOREACH ( i, dMergeIters )
		{
			const SliceInt_t & tElem = dMergeIters[i];
			if ( tElem.m_iOff<tElem.m_iEnd && dWordCp[tElem.m_iOff]<iMinCP )
			{
				iMinIndex = i;
				iMinCP = dWordCp[tElem.m_iOff];
			}
		}

		if ( iMinIndex==UINT_MAX )
			break;

		if ( dCheckpoints.GetLength()==0 || iMinCP!=dCheckpoints.Last().m_uOff )
		{
			dCheckpoints.Add().m_uOff = iMinCP;
			dCheckpoints.Last().m_uLen = 1;
		} else
		{
			dCheckpoints.Last().m_uLen++;
		}

		assert ( iMinIndex!=UINT_MAX && iMinCP!=UINT_MAX );
		assert ( dMergeIters[iMinIndex].m_iOff<dMergeIters[iMinIndex].m_iEnd );
		dMergeIters[iMinIndex].m_iOff++;
	}
	dCheckpoints.Sort ( CmpHistogram_fn() );
}


struct CmpSuggestOrder_fn
{
	bool IsLess ( const SuggestWord_t & a, const SuggestWord_t & b ) const
	{
		if ( a.m_iDistance==b.m_iDistance )
			return a.m_iDocs>b.m_iDocs;

		return a.m_iDistance<b.m_iDistance;
	}
};


static void SuggestMergeDocs ( CSphVector<SuggestWord_t> & dMatched )
{
	if ( !dMatched.GetLength() )
		return;

	dMatched.Sort ( bind ( &SuggestWord_t::m_iNameHash ) );

	int iSrc = 1;
	int iDst = 1;
	while ( iSrc<dMatched.GetLength() )
	{
		if ( dMatched[iDst-1].m_iNameHash==dMatched[iSrc].m_iNameHash )
		{
			dMatched[iDst-1].m_iDocs += dMatched[iSrc].m_iDocs;
			iSrc++;
		} else
		{
			dMatched[iDst++] = dMatched[iSrc++];
		}
	}

	dMatched.Resize ( iDst );
}

template <bool SINGLE_BYTE_CHAR>
void SuggestMatchWords ( const ISphWordlistSuggest * pWordlist, const CSphVector<Slice_t> & dCheckpoints, const SuggestArgs_t & tArgs, SuggestResult_t & tRes )
{
	// walk those checkpoints, check all their words

	const int iMinWordLen = ( tArgs.m_iDeltaLen>0 ? Max ( 0, tRes.m_iCodepoints - tArgs.m_iDeltaLen ) : -1 );
	const int iMaxWordLen = ( tArgs.m_iDeltaLen>0 ? tRes.m_iCodepoints + tArgs.m_iDeltaLen : INT_MAX );

	OpenHash_T<int, int64_t, HashFunc_Int64_t> dHashTrigrams;
	const char * sBuf = tRes.m_dTrigrams.Begin ();
	const char * sEnd = sBuf + tRes.m_dTrigrams.GetLength();
	while ( sBuf<sEnd )
	{
		dHashTrigrams.Add ( sphCRC32 ( sBuf ), 1 );
		while ( *sBuf ) sBuf++;
		sBuf++;
	}
	int dCharOffset[SPH_MAX_WORD_LEN+1];
	int dDictWordCodepoints[SPH_MAX_WORD_LEN];

	const int iQLen = Max ( tArgs.m_iQueueLen, tArgs.m_iLimit );
	const int iRejectThr = tArgs.m_iRejectThr;
	int iQueueRejected = 0;
	int iLastBad = 0;
	bool bSorted = true;
	const bool bMergeWords = tRes.m_bMergeWords;
	const bool bHasExactDict = tRes.m_bHasExactDict;
	const int iMaxEdits = tArgs.m_iMaxEdits;
	const bool bNonCharAllowed = tArgs.m_bNonCharAllowed;
	tRes.m_dMatched.Reserve ( iQLen * 2 );
	CmpSuggestOrder_fn fnCmp;

	ARRAY_FOREACH ( i, dCheckpoints )
	{
		DWORD iCP = dCheckpoints[i].m_uOff;
		pWordlist->SetCheckpoint ( tRes, iCP );

		ISphWordlistSuggest::DictWord_t tWord;
		while ( pWordlist->ReadNextWord ( tRes, tWord ) )
		{
			const char * sDictWord = tWord.m_sWord;
			int iDictWordLen = tWord.m_iLen;
			int iDictCodepoints = iDictWordLen;

			// for stemmer \ lematizer suggest should match only original words
			if ( bHasExactDict && sDictWord[0]!=MAGIC_WORD_HEAD_NONSTEMMED )
				continue;

			if ( bHasExactDict )
			{
				// skip head MAGIC_WORD_HEAD_NONSTEMMED char
				sDictWord++;
				iDictWordLen--;
				iDictCodepoints--;
			}

			if_const ( SINGLE_BYTE_CHAR )
			{
				if ( iDictWordLen<=iMinWordLen || iDictWordLen>=iMaxWordLen )
					continue;
			}

			int iChars = 0;

			const BYTE * s = (const BYTE *)sDictWord;
			const BYTE * sDictWordEnd = s + iDictWordLen;
			bool bGotNonChar = false;
			while ( !bGotNonChar && s<sDictWordEnd )
			{
				dCharOffset[iChars] = int ( s - (const BYTE *)sDictWord );
				int iCode = sphUTF8Decode ( s );
				if ( !bNonCharAllowed )
					bGotNonChar = ( iCode<'A' || ( iCode>'Z' && iCode<'a' ) ); // skip words with any numbers or special characters

				if_const ( !SINGLE_BYTE_CHAR )
				{
					dDictWordCodepoints[iChars] = iCode;
				}
				iChars++;
			}
			dCharOffset[iChars] = int ( s - (const BYTE *)sDictWord );
			iDictCodepoints = iChars;

			if_const ( !SINGLE_BYTE_CHAR )
			{
				if ( iDictCodepoints<=iMinWordLen || iDictCodepoints>=iMaxWordLen )
					continue;
			}

			// skip word in case of non char symbol found
			if ( bGotNonChar )
				continue;

			// FIXME!!! should we skip in such cases
			// utf8 reference word			!=	single byte dictionary word
			// single byte reference word	!=	utf8 dictionary word

			bool bGotMatch = false;
			for ( int iChar=0; iChar<=iDictCodepoints-3 && !bGotMatch; iChar++ )
			{
				int iStart = dCharOffset[iChar];
				int iEnd = dCharOffset[iChar+3];
				bGotMatch = ( dHashTrigrams.Find ( sphCRC32 ( sDictWord + iStart, iEnd - iStart ) )!=NULL );
			}

			// skip word in case of no trigrams matched
			if ( !bGotMatch )
				continue;

			int iDist = INT_MAX;
			if_const ( SINGLE_BYTE_CHAR )
				iDist = sphLevenshtein ( tRes.m_sWord.cstr(), tRes.m_iLen, sDictWord, iDictWordLen );
			else
				iDist = sphLevenshtein ( tRes.m_dCodepoints, tRes.m_iCodepoints, dDictWordCodepoints, iDictCodepoints );

			// skip word in case of too many edits
			if ( iDist>iMaxEdits )
				continue;

			SuggestWord_t tElem;
			tElem.m_iNameOff = tRes.m_dBuf.GetLength();
			tElem.m_iLen = iDictWordLen;
			tElem.m_iDistance = iDist;
			tElem.m_iDocs = tWord.m_iDocs;

			// store in k-buffer up to 2*QLen words
			if ( !iLastBad || fnCmp.IsLess ( tElem, tRes.m_dMatched[iLastBad] ) )
			{
				tElem.m_iNameHash = bMergeWords ? sphCRC32 ( sDictWord, iDictWordLen ) : 0;
				tRes.m_dMatched.Add ( tElem );
				BYTE * sWord = tRes.m_dBuf.AddN ( iDictWordLen+1 );
				memcpy ( sWord, sDictWord, iDictWordLen );
				sWord[iDictWordLen] = '\0';
				iQueueRejected = 0;
				bSorted = false;
			} else
			{
				iQueueRejected++;
			}

			// sort k-buffer in case of threshold overflow
			if ( tRes.m_dMatched.GetLength()>iQLen*2 )
			{
				if ( bMergeWords )
					SuggestMergeDocs ( tRes.m_dMatched );
				int iTotal = tRes.m_dMatched.GetLength();
				tRes.m_dMatched.Sort ( CmpSuggestOrder_fn() );
				bSorted = true;

				// there might be less than necessary elements after merge operation
				if ( iTotal>iQLen )
				{
					iQueueRejected += iTotal - iQLen;
					tRes.m_dMatched.Resize ( iQLen );
				}
				iLastBad = tRes.m_dMatched.GetLength()-1;
			}
		}

		if ( sphInterrupted () )
			break;

		// stop dictionary unpacking in case queue rejects a lot of matched words
		if ( iQueueRejected && iQueueRejected>iQLen*iRejectThr )
			break;
	}

	// sort at least once or any unsorted
	if ( !bSorted )
	{
		if ( bMergeWords )
			SuggestMergeDocs ( tRes.m_dMatched );
		tRes.m_dMatched.Sort ( CmpSuggestOrder_fn() );
	}
}


void sphGetSuggest ( const ISphWordlistSuggest * pWordlist, int iInfixCodepointBytes, const SuggestArgs_t & tArgs, SuggestResult_t & tRes )
{
	assert ( pWordlist );

	CSphVector<Slice_t> dCheckpoints;
	SuggestGetChekpoints ( pWordlist, iInfixCodepointBytes, tRes.m_dTrigrams, dCheckpoints, tRes );
	if ( !dCheckpoints.GetLength() )
		return;

	if ( tRes.m_bUtf8 )
		SuggestMatchWords<false> ( pWordlist, dCheckpoints, tArgs, tRes );
	else
		SuggestMatchWords<true> ( pWordlist, dCheckpoints, tArgs, tRes );

	if ( sphInterrupted() )
		return;

	tRes.Flattern ( tArgs.m_iLimit );
}

//////////////////////////////////////////////////////////////////////////
// CSphQueryResultMeta
//////////////////////////////////////////////////////////////////////////

void RemoveDictSpecials ( CSphString & sWord )
{
	if ( sWord.cstr()[0]==MAGIC_WORD_HEAD )
	{
		*const_cast<char *>( sWord.cstr() ) = '*';
	} else if ( sWord.cstr()[0]==MAGIC_WORD_HEAD_NONSTEMMED )
	{
		*const_cast<char *>( sWord.cstr() ) = '=';
	} else
	{
		const char * p = strchr ( sWord.cstr(), MAGIC_WORD_BIGRAM );
		if ( p )
			*const_cast<char *>(p) = ' ';
	}
}

const CSphString & RemoveDictSpecials ( const CSphString & sWord, CSphString & sFixed )
{
	const CSphString * pFixed = &sWord;
	if ( sWord.cstr()[0]==MAGIC_WORD_HEAD )
	{
		sFixed = sWord;
		*const_cast<char *>( sFixed.cstr() ) = '*';
		pFixed = &sFixed;
	} else if ( sWord.cstr()[0]==MAGIC_WORD_HEAD_NONSTEMMED )
	{
		sFixed = sWord;
		*const_cast<char *>( sFixed.cstr() ) = '=';
		pFixed = &sFixed;
	} else
	{
		const char * p = strchr ( sWord.cstr(), MAGIC_WORD_BIGRAM );
		if ( p )
		{
			sFixed.SetSprintf ( "\"%s\"", sWord.cstr() );
			*( const_cast<char *> ( sFixed.cstr() ) + ( p - sWord.cstr() ) + 1 ) = ' ';
			pFixed = &sFixed;
		}
	}

	return *pFixed;
}

void CSphQueryResultMeta::AddStat ( const CSphString & sWord, int64_t iDocs, int64_t iHits )
{
	CSphString sBuf;
	const CSphString & tFixed = RemoveDictSpecials ( sWord, sBuf );
	WordStat_t & tStats = m_hWordStats.AddUnique ( tFixed );
	tStats.first += iDocs;
	tStats.second += iHits;
}


void CSphQueryResultMeta::MergeWordStats ( const CSphQueryResultMeta & tOther )
{
	const auto & hOtherStats = tOther.m_hWordStats;
	if ( !m_hWordStats.GetLength () )
		// nothing has been set yet; just copy
		m_hWordStats = hOtherStats;
	else
		for ( auto & tStat : hOtherStats )
			AddStat ( tStat.first, tStat.second.first, tStat.second.second );
}

///< sort wordstat to achieve reproducable result over different runs
CSphFixedVector<SmallStringHash_T<CSphQueryResultMeta::WordStat_t>::KeyValue_t *> CSphQueryResultMeta::MakeSortedWordStat () const
{
	using kv_t = SmallStringHash_T<WordStat_t>::KeyValue_t;
	CSphFixedVector<kv_t*> dWords { m_hWordStats.GetLength() };

	int i = 0;
	for ( auto & tStat : m_hWordStats )
		dWords[i++] = &tStat;

	dWords.Sort ( Lesser ( [] ( kv_t * l, kv_t * r ) { return l->first<r->first; } ) );
	return dWords;
}

//////////////////////////////////////////////////////////////////////////

CSphVector<const ISphSchema *> SorterSchemas ( const VecTraits_T<ISphMatchSorter *> & dSorters, int iSkipSorter )
{
	CSphVector<const ISphSchema *> dSchemas;
	if ( !dSorters.IsEmpty() )
	{
		dSchemas.Reserve ( dSorters.GetLength() - 1 );
		ARRAY_FOREACH ( i, dSorters )
		{
			if ( i==iSkipSorter || !dSorters[i] )
				continue;

			const ISphSchema * pSchema = dSorters[i]->GetSchema();
			dSchemas.Add ( pSchema );
		}
	}
	return dSchemas;
}

std::pair<int, int> GetMaxSchemaIndexAndMatchCapacity ( const VecTraits_T<ISphMatchSorter*> & dSorters )
{
	int iMaxSchemaSize = -1;
	int iMaxSchemaIndex = -1;
	int iMatchPoolSize = 0;
	ARRAY_FOREACH ( i, dSorters )
	{
		iMatchPoolSize += dSorters[i]->GetMatchCapacity();
		if ( dSorters[i]->GetSchema ()->GetAttrsCount ()>iMaxSchemaSize )
		{
			iMaxSchemaSize = dSorters[i]->GetSchema ()->GetAttrsCount ();
			iMaxSchemaIndex = i;
		}
	}
	return {iMaxSchemaIndex, iMatchPoolSize};
}


//////////////////////////////////////////////////////////////////////////
CSphString IndexFiles_c::FatalMsg ( const char * sMsg )
{
	CSphString sFatalMsg;
	if ( sMsg )
	{
		if ( m_sIndexName.IsEmpty () )
			sFatalMsg.SetSprintf ( "%s: %s", sMsg, ErrorMsg () );
		else
			sFatalMsg.SetSprintf ( "%s index '%s': %s", sMsg, m_sIndexName.cstr(), ErrorMsg() );
	} else
	{
		if ( m_sIndexName.IsEmpty () )
			sFatalMsg.SetSprintf ( "%s", ErrorMsg () );
		else
			sFatalMsg.SetSprintf ( "index '%s': %s", m_sIndexName.cstr (), ErrorMsg () );
	}
	return sFatalMsg;
}

CSphString IndexFiles_c::FullPath ( const char * sExt, const char* sSuffix, const char * sBase )
{
	CSphString sResult;
	sResult.SetSprintf ( "%s%s%s", sBase ? sBase : m_sFilename.cstr (), sSuffix, sExt );
	return sResult;
}

CSphString IndexFiles_c::MakePath ( const char * sSuffix, const char * sBase )
{
	CSphString sResult;
	sResult.SetSprintf ( "%s%s", sBase ? sBase : m_sFilename.cstr (), sSuffix );
	return sResult;
}

bool IndexFiles_c::HasAllFiles ( const char * sType )
{
	for ( const auto & dExt : g_dIndexFilesExts )
	{
		if ( m_uVersion<dExt.m_uMinVer || dExt.m_bOptional )
			continue;

		if ( !sphIsReadable ( FullPath ( dExt.m_szExt, sType ) ) )
			return false;
	}
	return true;
}

void IndexFiles_c::Unlink ( const char * sType )
{
	for ( const auto &dExt : g_dIndexFilesExts )
	{
		auto sFile = FullPath ( dExt.m_szExt, sType );
		if ( ::unlink ( sFile.cstr() ) && !dExt.m_bOptional )
			sphWarning ( "unlink failed (file '%s', error '%s'", sFile.cstr (), strerrorm ( errno ) );
	}
}

bool IndexFiles_c::Rename ( const char * sFromSz, const char * sToSz )  // move files between different bases
{
	m_bFatal = false;
	bool bRenamed[SPH_EXT_TOTAL] = { false };
	bool bAllOk = true;
	for ( int i = 0; i<SPH_EXT_TOTAL; i++ )
	{
		const auto & dExt = g_dIndexFilesExts[i];
		if ( m_uVersion<dExt.m_uMinVer || !dExt.m_bCopy )
			continue;

		auto sFrom = FullPath ( dExt.m_szExt, "", sFromSz );
		auto sTo = FullPath ( dExt.m_szExt, "", sToSz );

#if _WIN32
		::unlink ( sTo.cstr() );
		sphLogDebug ( "%s unlinked", sTo.cstr() );
#endif

		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			// this is no reason to fail if not necessary files missed.
			if ( dExt.m_bOptional )
				continue;

			m_sLastError.SetSprintf ( "rename %s to %s failed: %s", sFrom.cstr (), sTo.cstr (), strerrorm ( errno ) );
			bAllOk = false;
			break;
		}
		bRenamed[i] = true;
	}

	if ( bAllOk )
		return true;

	for ( int i = 0; i<SPH_EXT_TOTAL; ++i )
	{
		if ( !bRenamed[i] )
			continue;

		const auto & dExt = g_dIndexFilesExts[i];
		auto sFrom = FullPath ( dExt.m_szExt, "", sToSz );
		auto sTo = FullPath ( dExt.m_szExt, "", sFromSz );
		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			sphLogDebug ( "rollback failure when renaming %s to %s", sFrom.cstr (), sTo.cstr () );
			m_bFatal = true;
		}
	}

	return false;
}

bool IndexFiles_c::RenameLock ( const char * sToSz, int &iLockFD )
{
	if ( iLockFD<0 ) // no lock, no renaming need
		return true;

	m_bFatal = false;
	auto sFrom = FullPath ( sphGetExt(SPH_EXT_SPL) );
	auto sTo = FullPath ( sphGetExt(SPH_EXT_SPL), "", sToSz );

#if !_WIN32
	if ( !sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		return true;

	m_sLastError.SetSprintf ("failed to rename lock %s to %s, fd=%d, error %s (%d); ", sFrom.cstr(),
				   sTo.cstr(), iLockFD, strerrorm ( errno ), errno );

	// that is renaming of only 1 file failed; no need to rollback.
	m_bFatal = true;
	return false;

#else
	// on Windows - no direct rename. Lock new instead, release previous.
	int iNewLock=-1;
	if ( !RawFileLock ( sTo, iNewLock, m_sLastError ) )
		return false;
	auto iOldLock = iLockFD;
	iLockFD = iNewLock;
	RawFileUnLock ( sFrom, iOldLock );
	return true;
#endif
}

// move from backup to path using full (long) paths; fail is fatal
bool IndexFiles_c::Rollback ( const char * sBackup, const char * sPath )
{
	m_bFatal = false;
	for ( const auto &dExt : g_dIndexFilesExts )
	{
		auto sFrom = FullPath ( dExt.m_szExt, "", sBackup );
		auto sTo = FullPath ( dExt.m_szExt, "", sPath );

		if ( !sphIsReadable ( sFrom ) )
		{
			::unlink ( sTo.cstr () );
			continue;
		}
#if _WIN32
		::unlink ( sTo.cstr() );
		sphLogDebug ( "%s unlinked", sTo.cstr() );
#endif
		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			sphLogDebug ( "rollback rename %s to %s failed: %s", sFrom.cstr (), sTo.cstr (), strerrorm (	errno ) );
			m_bFatal = true;
			return false;
		}
	}
	return true;
}

// move everything except not intended for copying.
bool IndexFiles_c::RenameSuffix ( const char * sFromSuffix, const char * sToSuffix )
{
	return Rename ( FullPath ( "", sFromSuffix ).cstr (), FullPath ( "", sToSuffix ).cstr () );
}

bool IndexFiles_c::RenameBase ( const char * sToBase )  // move files to different base
{
	return Rename ( FullPath ( "" ).cstr (), sToBase );
}

bool IndexFiles_c::RelocateToNew ( const char * sNewBase )
{
	return Rollback( FullPath ( "", "", sNewBase ).cstr(), FullPath ( "", ".new" ).cstr() );
}

bool IndexFiles_c::RollbackSuff ( const char * sBackupSuffix, const char * sActiveSuffix )
{
	return Rollback ( FullPath ( "", sBackupSuffix ).cstr (), FullPath ( "", sActiveSuffix ).cstr () );
}

bool IndexFiles_c::CheckHeader ( const char * sType )
{
	auto sPath = FullPath ( sphGetExt(SPH_EXT_SPH), sType );
	BYTE dBuffer[8];

	CSphAutoreader rdHeader ( dBuffer, sizeof ( dBuffer ) );
	if ( !rdHeader.Open ( sPath, m_sLastError ) )
		return false;

	// check magic header
	const char * sMsg = CheckFmtMagic ( rdHeader.GetDword () );
	if ( sMsg )
	{
		m_sLastError.SetSprintf ( sMsg, sPath.cstr() );
		return false;
	}

	// get version
	DWORD uVersion = rdHeader.GetDword ();
	if ( uVersion==0 || uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%u, binary is v.%u", sPath.cstr(), uVersion, INDEX_FORMAT_VERSION );
		return false;
	}
	m_uVersion = uVersion;
	return true;
}


bool IndexFiles_c::ReadKlistTargets ( StrVec_t & dTargets, const char * szType )
{
	CSphString sPath = FullPath ( sphGetExt(SPH_EXT_SPK), szType );
	if ( !sphIsReadable(sPath) )
		return true;

	CSphString sError;
	CSphAutoreader tReader;
	if ( !tReader.Open ( sPath, sError ) )
		return false;

	DWORD nIndexes = tReader.GetDword();
	dTargets.Resize ( nIndexes );
	for ( auto & i : dTargets )
	{
		i = tReader.GetString();
		tReader.GetDword();	// skip flags
	}

	return true;
}

volatile int &sphGetTFO () noexcept
{
	static int iTFO = 0;
	return iTFO;
}

volatile bool& sphGetbCpuStat () noexcept
{
	static bool bCpuStat = false;
	return bCpuStat;
}
