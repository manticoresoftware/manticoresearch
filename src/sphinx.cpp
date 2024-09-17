//
//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
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
#include "docidlookup.h"
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
#include "client_task_info.h"
#include "chunksearchctx.h"
#include "std/lrucache.h"
#include "std/sys.h"
#include "indexfiles.h"
#include "task_dispatcher.h"
#include "secondarylib.h"
#include "knnlib.h"
#include "attrindex_merge.h"
#include "knnmisc.h"
#include "querycontext.h"
#include "dict/infix/infix_builder.h"
#include "skip_cache.h"
#include "jsonsi.h"
#include "tracer.h"

#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#include <algorithm>

#if WITH_RE2
#include <string>
#include <re2/re2.h>
#endif

#if !_WIN32
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
	tm * pRes = localtime ( clock );
	if ( pRes )
		*res = *pRes;
}

void gmtime_r ( const time_t * clock, struct tm * res )
{
	tm * pRes = gmtime ( clock );
	if ( pRes )
		*res = *pRes;
}
#endif

#include <boost/preprocessor/repetition/repeat.hpp>

#include "attrindex_builder.h"
#include "stripper/html_stripper.h"
#include "queryfilter.h"
#include "indexing_sources/source_document.h"
#include "indexing_sources/source_stats.h"
#include "dict/dict_base.h"
#include "dict/bin.h"

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////

const char *		MAGIC_WORD_SENTENCE		= "\3sentence";		// emitted from source on sentence boundary, stored in dictionary
const char *		MAGIC_WORD_PARAGRAPH	= "\3paragraph";	// emitted from source on paragraph boundary, stored in dictionary

bool				g_bJsonStrict				= false;
bool				g_bJsonAutoconvNumbers		= false;
bool				g_bJsonKeynamesToLowercase	= false;

static const int	MIN_READ_BUFFER			= 8192;
static const int	MIN_READ_UNHINTED		= 1024;

static int 			g_iReadUnhinted 		= DEFAULT_READ_UNHINTED;

static bool			g_bPseudoSharding		= true;
static int			g_iPseudoShardingThresh	= 8192;

static BuildBufferSettings_t g_tMergeSettings;

static int			g_iLowPriorityDivisor = 10;			// how smaller quantum low-priority tasks take comparing to normal in case of load

static bool LOG_LEVEL_SPLIT_QUERY = val_from_env ( "MANTICORE_LOG_SPLIT_QUERY", false ); // verbose logging split query events, ruled by this env variable
#define LOG_COMPONENT_QUERYINFO __LINE__ << " "
#define QUERYINFO LOGINFO ( SPLIT_QUERY, QUERYINFO )

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

/////////////////////////////////////////////////////////////////////

/// everything required to setup search term
class DiskIndexQwordSetup_c final : public ISphQwordSetup
{
public:
	DiskIndexQwordSetup_c ( DataReaderFactoryPtr_c pDoclist, DataReaderFactoryPtr_c pHitlist, const BYTE * pSkips, int iSkiplistBlockSize, bool bSetupReaders, RowID_t iRowsCount )
		: m_pDoclist ( std::move ( pDoclist ) )
		, m_pHitlist ( std::move ( pHitlist ) )
		, m_pSkips ( pSkips )
		, m_iSkiplistBlockSize ( iSkiplistBlockSize )
		, m_bSetupReaders ( bSetupReaders )
		, m_iRowsCount ( iRowsCount )
	{}

	ISphQword *			QwordSpawn ( const XQKeyword_t & tWord ) const final;
	bool				QwordSetup ( ISphQword * ) const final;
	bool				Setup ( ISphQword * ) const;
	ISphQword *			ScanSpawn() const final;

private:
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

/// query word from the searcher's point of view
template < bool INLINE_HITS, bool DISABLE_HITLIST_SEEK >
class DiskIndexQword_c : public DiskIndexQwordTraits_c
{
public:
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
			if constexpr ( DISABLE_HITLIST_SEEK )
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

private:
	int64_t m_iIndexId = 0;
};


DiskIndexQwordTraits_c * sphCreateDiskIndexQword ( bool bInlineHits )
{
	if ( bInlineHits )
		return new DiskIndexQword_c<true,false> ( false, false, 0 );

	return new DiskIndexQword_c<false,false> ( false, false, 0 );
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


// duplicated in sphinxformat.cpp
struct Slice64_t
{
	uint64_t	m_uOff;
	int			m_iLen;
};

// duplicated in sphinxformat.cpp
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

const char* CheckFmtMagic ( DWORD uHeader )
{
	if ( uHeader!=INDEX_MAGIC_HEADER )
	{
		FlipEndianness ( &uHeader );
		if ( uHeader==INDEX_MAGIC_HEADER )
#if USE_LITTLE_ENDIAN
			return "This instance is working on little-endian platform, but %s seems built on big-endian host.";
#else
			return "This instance is working on big-endian platform, but %s seems built on little-endian host.";
#endif
		else
			return "%s is invalid header file (too old table version?)";
	}
	return nullptr;
}


/// this pseudo-index used to store and manage the tokenizer
/// without any footprint in real files
//////////////////////////////////////////////////////////////////////////
class CSphTokenizerIndex : public CSphIndexStub
{
public:
						CSphTokenizerIndex ( CSphString sIndexName ) : CSphIndexStub ( std::move ( sIndexName ), "" ) {}
	bool				GetKeywords ( CSphVector <CSphKeywordInfo> & , const char * , const GetKeywordsSettings_t & tSettings, CSphString * ) const final ;
	Bson_t				ExplainQuery ( const CSphString & sQuery ) const final;
};

bool CSphTokenizerIndex::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * ) const
{
	// short-cut if no query or keywords to fill
	if ( !szQuery || !szQuery[0] )
		return true;

	TokenizerRefPtr_c pTokenizer = m_pTokenizer->Clone ( SPH_CLONE_INDEX );
	pTokenizer->EnableTokenizedMultiformTracking ();

	// need to support '*' and '=' but not the other specials
	// so m_pQueryTokenizer does not work for us, gotta clone and setup one manually

	DictRefPtr_c pDict = GetStatelessDict ( m_pDict );

	if ( IsStarDict ( pDict->GetSettings().m_bWordDict ) )
	{
		pTokenizer->AddPlainChars ( "*" );
		SetupStarDictOld ( pDict );
	}

	if ( m_tSettings.m_bIndexExactWords )
	{
		pTokenizer->AddSpecials ( "=" );
		SetupExactDict ( pDict );
	}

	dKeywords.Resize ( 0 );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (const BYTE *)szQuery;
	if ( m_pFieldFilter && szQuery && m_pFieldFilter->Clone()->Apply ( sModifiedQuery, dFiltered, true ) )
		sModifiedQuery = dFiltered.Begin();

	pTokenizer->SetBuffer ( sModifiedQuery, (int) strlen ( (const char*)sModifiedQuery) );

	CSphTemplateQueryFilter tAotFilter;
	tAotFilter.m_pTokenizer = std::move ( pTokenizer );
	tAotFilter.m_pDict = std::move ( pDict );
	tAotFilter.m_pSettings = &m_tSettings;
	tAotFilter.m_tFoldSettings = tSettings;
	tAotFilter.m_tFoldSettings.m_bStats = false;
	tAotFilter.m_tFoldSettings.m_bFoldWildcards = true;

	ExpansionContext_t tExpCtx;

	tAotFilter.GetKeywords ( dKeywords, tExpCtx );

	return true;
}


std::unique_ptr<CSphIndex> sphCreateIndexTemplate ( CSphString sIndexName )
{
	return std::make_unique<CSphTokenizerIndex> ( std::move ( sIndexName ) );
}

Bson_t CSphTokenizerIndex::ExplainQuery ( const CSphString & sQuery ) const
{
	bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	WordlistStub_c tWordlist;
	ExplainQueryArgs_t tArgs;
	tArgs.m_szQuery = sQuery.cstr();
	tArgs.m_pDict = GetStatelessDict ( m_pDict );
	if ( IsStarDict ( bWordDict ) )
		SetupStarDictV8 ( tArgs.m_pDict, m_tSettings.m_iMinInfixLen>0 );
	if ( m_tSettings.m_bIndexExactWords )
		SetupExactDict ( tArgs.m_pDict );
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
	, m_iStride ( tSchema.GetRowSize() )
	, m_dUpdatedAttrs ( tUpd.m_pUpdate->m_dAttributes.GetLength() )
	, m_dSchemaUpdateMask ( tSchema.GetAttrsCount() )
{}

//////////////////////////////////////////////////////////////////////////

bool Update_CheckAttributes ( const CSphAttrUpdate & tUpd, const ISphSchema & tSchema, CSphString & sError )
{
	for ( const auto & tUpdAttr : tUpd.m_dAttributes )
	{
		const CSphString & sUpdAttrName = tUpdAttr.m_sName;
		int iUpdAttrId = tSchema.GetAttrIndex ( sUpdAttrName.cstr() );

		// try to find JSON attribute with a field
		if ( iUpdAttrId<0 )
		{
			CSphString sJsonCol;
			if ( sphJsonNameSplit ( sUpdAttrName.cstr(), nullptr, &sJsonCol ) )
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
		switch ( tCol.m_eAttrType )
		{
		case SPH_ATTR_BOOL:
		case SPH_ATTR_INTEGER:
		case SPH_ATTR_TIMESTAMP:
		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
		case SPH_ATTR_FLOAT_VECTOR:
		case SPH_ATTR_STRING:
		case SPH_ATTR_BIGINT:
		case SPH_ATTR_FLOAT:
		case SPH_ATTR_JSON:
			break;
		default:
			sError.SetSprintf ( "attribute '%s' can not be updated (must be boolean, integer, bigint, float, timestamp, string, MVA or JSON)", sUpdAttrName.cstr() );
			return false;
		}

		bool bSrcMva = tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET || tCol.m_eAttrType==SPH_ATTR_FLOAT_VECTOR;
		bool bDstMva = tUpdAttr.m_eType==SPH_ATTR_UINT32SET || tUpdAttr.m_eType==SPH_ATTR_INT64SET || tUpdAttr.m_eType==SPH_ATTR_FLOAT_VECTOR;
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

		if( ( tCol.m_eAttrType==SPH_ATTR_UINT32SET || tCol.m_eAttrType==SPH_ATTR_INT64SET ) && tUpdAttr.m_eType==SPH_ATTR_FLOAT_VECTOR )
		{
			sError.SetSprintf ( "can't update MVA attribute '%s' bits with float vector value", sUpdAttrName.cstr() );
			return false;
		}

		if ( tCol.IsColumnar() )
		{
			sError.SetSprintf ( "unable to update columnar attribute '%s'", sUpdAttrName.cstr() );
			return false;
		}

		if ( tCol.IsIndexedKNN() )
		{
			sError.SetSprintf ( "unable to update attribute '%s' that has a KNN index", sUpdAttrName.cstr() );
			return false;
		}
	}

	return true;
}


void UpdateContext_t::PrepareListOfUpdatedAttributes ( CSphString & sError )
{
	const auto & tUpd = *m_tUpd.m_pUpdate;
	ARRAY_FOREACH ( i, tUpd.m_dAttributes )
	{
		const CSphString & sUpdAttrName = tUpd.m_dAttributes[i].m_sName;
		ESphAttr eUpdAttrType = tUpd.m_dAttributes[i].m_eType;
		UpdatedAttribute_t & tUpdAttr = m_dUpdatedAttrs[i];

		int iUpdAttrId = m_tSchema.GetAttrIndex ( sUpdAttrName.cstr() );

		if ( iUpdAttrId<0 )
		{
			CSphString sJsonCol;
			if ( sphJsonNameSplit ( sUpdAttrName.cstr(), nullptr, &sJsonCol ) )
			{
				iUpdAttrId = m_tSchema.GetAttrIndex ( sJsonCol.cstr() );
				if ( iUpdAttrId>=0 )
				{
					ExprParseArgs_t tExprArgs;
					tUpdAttr.m_pExpr = sphExprParse ( sUpdAttrName.cstr(), m_tSchema, nullptr, sError, tExprArgs );
				}
			}
		}

		if ( iUpdAttrId>=0 )
		{
			const CSphColumnInfo & tCol = m_tSchema.GetAttr(iUpdAttrId);

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
			tUpdAttr.m_pHistogram = m_pHistograms ? m_pHistograms->Get(tCol.m_sName) : nullptr;
			tUpdAttr.m_bExisting = true;
			tUpdAttr.m_iSchemaAttr = iUpdAttrId;

			m_dSchemaUpdateMask.BitSet(iUpdAttrId);
			m_bBlobUpdate |= sphIsBlobAttr(tCol);
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
		// Fix cases like UPDATE float_attr=1 WHERE id=1;
		assert ( iUpdAttrId>=0 );
		if ( eUpdAttrType==SPH_ATTR_INTEGER && m_tSchema.GetAttr(iUpdAttrId).m_eAttrType==SPH_ATTR_FLOAT )
		{
			assert ( tUpd.m_dRowOffset.IsEmpty() ); // fixme! Now we don't fixup more then 1 value
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
	case SPH_ATTR_FLOAT_VECTOR:
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


bool IndexSegment_c::Update_InplaceJson ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx, CSphString & sError, bool bDryRun )
{
	const auto& tUpd = *tCtx.m_tUpd.m_pUpdate;
	for ( const auto & tRow : dRows )
	{
		int iUpd = tRow.m_iIdx;
		auto pDocinfo = tCtx.GetDocinfo ( tRow.m_tRow );
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

			if ( sphJsonInplaceUpdate ( eType, uValue, tCtx.m_dUpdatedAttrs[i].m_pExpr, tCtx.m_pBlobPool, pDocinfo, !bDryRun ) )
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


bool IndexSegment_c::Update_Blobs ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx, bool & bCritical, CSphString & sError )
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
	std::unique_ptr<BlobRowBuilder_i> pBlobRowBuilder = sphCreateBlobRowBuilderUpdate ( tCtx.m_tSchema, tUpd.m_dAttributes, tBlobPool, tCtx.m_dSchemaUpdateMask );

	const CSphColumnInfo * pBlobLocator = tCtx.m_tSchema.GetAttr ( sphGetBlobLocatorName() );

	for ( const auto & tRow : dRows )
	{
		int iUpd = tRow.m_iIdx;
		auto pDocinfo = tCtx.GetDocinfo ( tRow.m_tRow );
		tBlobPool.Resize(0);
		ARRAY_CONSTFOREACH ( iBlobId, dBlobAttrIds )
		{
			int iCol = dBlobAttrIds[iBlobId];
			if ( tCtx.m_dSchemaUpdateMask.BitGet(iCol) )
				continue;

			const CSphColumnInfo & tAttr = tCtx.m_tSchema.GetAttr(iCol);
			int iLengthBytes = 0;
			const BYTE* pData = sphGetBlobAttr ( pDocinfo, tAttr.m_tLocator, tCtx.m_pBlobPool, iLengthBytes );
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
			case SPH_ATTR_FLOAT_VECTOR:
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
		if ( !Update_WriteBlobRow ( tCtx, tRow.m_tRow, tBlobPool, iBlobAttrId, pBlobLocator->m_tLocator, bCritical, sError ) )
			return false;
	}

	return true;
}


bool UpdateContext_t::HandleJsonWarnings ( int iUpdated, CSphString & sWarning, CSphString & sError ) const
{
	if ( !m_iJsonWarnings )
		return true;

	sWarning.SetSprintf ( "%d attribute(s) can not be updated (not found or incompatible types)", m_iJsonWarnings );
	if ( !iUpdated )
		sError = sWarning;

	return !!iUpdated;
}

CSphRowitem* UpdateContext_t::GetDocinfo ( RowID_t iRowID ) const
{
	assert ( iRowID != INVALID_ROWID );
	assert ( m_pAttrPool );
	assert ( m_iStride );
	return m_pAttrPool + iRowID * m_iStride;
}


void IndexSegment_c::Update_Plain ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx )
{
	const auto & tUpd = *tCtx.m_tUpd.m_pUpdate;

	for ( const auto & tRow : dRows )
	{
		int iPos = tUpd.GetRowOffset ( tRow.m_iIdx );
		auto pDocinfo = tCtx.GetDocinfo ( tRow.m_tRow );
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
				SphAttr_t tOldValue = sphGetRowAttr ( pDocinfo, tLoc );
				pHistogram->Delete ( tOldValue );
				pHistogram->UpdateCounter ( uValue );
			}

			sphSetRowAttr ( pDocinfo, tLoc, uValue );
			tCtx.m_tUpd.MarkUpdated ( tRow.m_iIdx );
			tCtx.m_uUpdateMask |= ATTRS_UPDATED;

			// next
			IncUpdatePoolPos ( tCtx, iCol, iPos );
		}
	}
}

bool IndexSegment_c::Update_UpdateAttributes ( const RowsToUpdate_t& dRows, UpdateContext_t& tCtx, bool& bCritical, CSphString& sError )
{
	// FIXME! FIXME! FIXME! overwriting just-freed blocks might hurt concurrent searchers;
	// should implement a simplistic MVCC-style delayed-free to avoid that

	TRACE_CORO ( "rt", "IndexSegment_c::Update_UpdateAttributes" );
	// first pass, if needed
	if ( tCtx.m_tUpd.m_pUpdate->m_bStrict )
		if ( !Update_InplaceJson ( dRows, tCtx, sError, true ) )
			return false;

	// second pass
	int iSaveWarnings = tCtx.m_iJsonWarnings;
	tCtx.m_iJsonWarnings = 0;
	Update_InplaceJson ( dRows, tCtx, sError, false );
	tCtx.m_iJsonWarnings += iSaveWarnings;

	if ( !Update_Blobs ( dRows, tCtx, bCritical, sError ) )
		return false;

	Update_Plain ( dRows, tCtx );
	return true;
}

class QueryMvaContainer_c
{
public:
	CSphVector<OpenHashTable_T<int64_t,CSphVector<int64_t>>*> m_tContainer;

	~QueryMvaContainer_c()
	{
		for ( auto i : m_tContainer )
			delete i;
	}
};

class CSphHitBuilder;

/// this is my actual VLN-compressed phrase index implementation
class CSphIndex_VLN : public CSphIndex, public IndexAlterHelper_c, public DebugCheckHelper_c
{
	friend class DiskIndexQwordSetup_c;
	friend class CSphMerger;
	friend class AttrIndexBuilder_c;
	friend struct SphFinalMatchCalc_t;
	friend class KeepAttrs_c;

public:
						CSphIndex_VLN ( CSphString sIndexName, CSphString sFilename );
						~CSphIndex_VLN() override;

	int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer, CSphIndexProgress & tProgress ) final; // fixme! build only

	enum class LOAD_E { ParseError_e, GeneralError_e, Ok_e };
	LOAD_E				LoadHeaderLegacy ( const CSphString& sHeaderName, bool bStripPath, CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder, CSphString & sWarning );
	LOAD_E				LoadHeaderJson ( const CSphString& sHeaderName, bool bStripPath, CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder, CSphString & sWarning );

	void				DebugDumpHeader ( FILE * fp, const CSphString& sHeaderName, bool bConfig ) final;
	void				DebugDumpDocids ( FILE * fp ) final;
	void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID ) final;
	void				DebugDumpDict ( FILE * fp, bool bDumpOnly ) final;
	void				SetDebugCheck ( bool bCheckIdDups, int iCheckChunk ) final;
	int					DebugCheck ( DebugCheckError_i & , FilenameBuilder_i * pFilenameBuilder ) final;
	template <class Qword> void		DumpHitlist ( FILE * fp, const char * sKeyword, bool bID );

	bool				Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings ) final;
	void				Dealloc () final;
	void				Preread () final;

	RenameResult_e		RenameEx ( CSphString sNewBase ) final;

	bool				Lock () final;
	void				Unlock () final;

	bool				MultiQuery ( CSphQueryResult& tResult, const CSphQuery& tQuery, const VecTraits_T<ISphMatchSorter*>& dAllSorters, const CSphMultiQueryArgs& tArgs ) const final;
	bool				MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult* pResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const final;
	bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const final;
	template <class Qword> bool		DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, bool bFillOnly, CSphString * pError ) const;
	bool 				FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const final;
	void				GetSuggest ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes ) const final;

	bool				Merge ( CSphIndex * pSource, const VecTraits_T<CSphFilterSettings> & dFilters, bool bSupressDstDocids, CSphIndexProgress & tProgress ) final; // fixme! build only

	template <class QWORDDST, class QWORDSRC>
	static bool			MergeWords ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, VecTraits_T<RowID_t> dDstRows, VecTraits_T<RowID_t> dSrcRows, CSphHitBuilder * pHitBuilder, CSphString & sError, CSphIndexProgress & tProgress);
	static bool			DoMerge ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, const ISphFilter * pFilter, CSphString & sError, CSphIndexProgress & tProgress, bool bSrcSettings, bool bSupressDstDocids );
	std::unique_ptr<ISphFilter>		CreateMergeFilters ( const VecTraits_T<CSphFilterSettings> & dSettings ) const;
	template <class QWORD>
	static bool			DeleteField ( const CSphIndex_VLN * pIndex, CSphHitBuilder * pHitBuilder, CSphString & sError, CSphSourceStats & tStat, int iKillField );

	int					CheckThenUpdateAttributes ( AttrUpdateInc_t& tUpd, bool& bCritical, CSphString& sError, CSphString& sWarning ) final;
	void				UpdateAttributesOffline ( VecTraits_T<PostponedUpdate_t> & dPostUpdates ) final;

	// the only txn we can replay is 'update attributes', but it is processed by dedicated branch in binlog, so we have nothing to do here.
	Binlog::CheckTnxResult_t ReplayTxn ( CSphReader&, CSphString&, BYTE, Binlog::CheckTxn_fn&& ) final;
	bool				SaveAttributes ( CSphString & sError ) const final;
	DWORD				GetAttributeStatus () const final;

	bool				AddRemoveAttribute ( bool bAddAttr, const AttrAddRemoveCtx_t & tCtx, CSphString & sError ) final;
	bool				AddRemoveField ( bool bAdd, const CSphString & sFieldName, DWORD uFieldFlags, CSphString & sError ) final;

	void				FlushDeadRowMap ( bool bWaitComplete ) const final;
	bool				LoadKillList ( CSphFixedVector<DocID_t> * pKillList, KillListTargets_c & tTargets, CSphString & sError ) const final;
	bool				AlterKillListTarget ( KillListTargets_c & tTargets, CSphString & sError ) final;
	void				KillExistingDocids ( CSphIndex * pTarget ) const final;

	bool				EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const final;

	void				SetKeepAttrs ( const CSphString & sKeepAttrs, const StrVec_t & dAttrs ) final { m_sKeepAttrs = sKeepAttrs; m_dKeepAttrs = dAttrs; }
	RowID_t				GetRowidByDocid ( DocID_t tDocID ) const;
	int					Kill ( DocID_t tDocID ) final;
	int					KillMulti ( const VecTraits_T<DocID_t> & dKlist ) final;
	int					KillDupes() final;
	int					CheckThenKillMulti ( const VecTraits_T<DocID_t>& dKlist, BlockerFn&& fnWatcher ) final;
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
	bool				PreallocKNN();
	bool				PreallocSkiplist();

	bool				LoadSecondaryIndex ( const CSphString & sFile );
	bool				PreallocSecondaryIndex();

	void				PrepareHeaders ( BuildHeader_t & tBuildHeader, WriteHeader_t & tWriteHeader, bool bCopyDictHeader = true );
	bool				SaveHeader ( CSphString & sError );

	CSphVector<SphAttr_t> 	BuildDocList () const final;

	// docstore-related section
	void				CreateReader ( int64_t iSessionId ) const final;
	bool				GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const final;
	int					GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const final;
	Bson_t				ExplainQuery ( const CSphString & sQuery ) const final;

	HistogramContainer_c * Debug_GetHistograms() const override { return m_pHistograms; }
	const SIContainer_c * Debug_GetSI() const override { return &m_tSI; }

	bool				CheckEarlyReject ( const CSphVector<CSphFilterSettings> & dFilters, const ISphFilter * pFilter, ESphCollation eCollation, const ISphSchema & tSchema ) const;
	std::pair<int64_t,int> GetPseudoShardingMetric ( const VecTraits_T<const CSphQuery> & dQueries, const VecTraits_T<int64_t> & dMaxCountDistinct, int iThreads, bool & bForceSingleThread ) const override;
	int64_t				GetCountDistinct ( const CSphString & sAttr, CSphString & sModifiedAttr ) const override;
	int64_t				GetCountFilter ( const CSphFilterSettings & tFilter, CSphString & sModifiedAttr ) const override;
	int64_t				GetCount() const override;

private:
	static const int			MIN_WRITE_BUFFER		= 262144;	///< min write buffer size
	static const int			DEFAULT_WRITE_BUFFER	= 1048576;	///< default write buffer size

private:
	// common stuff
	int								m_iLockFD;
	CSphSourceStats					m_tStats;			///< my stats
	CSphFixedVector<int64_t>		m_dFieldLens;	///< total per-field lengths summed over entire indexed data, in tokens
	CSphString						m_sKeepAttrs;			///< retain attributes of that index reindexing //fixme! build only
	StrVec_t						m_dKeepAttrs;		// fixme! build only

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

	std::unique_ptr<Docstore_i>	m_pDocstore;
	std::unique_ptr<columnar::Columnar_i> m_pColumnar;
	std::unique_ptr<knn::KNN_i>	m_pKNN;
	SIContainer_c				m_tSI;

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
	void						GetIndexFiles ( StrVec_t& dFiles, StrVec_t& dExt, const FilenameBuilder_i* = nullptr ) const override;

	bool						ParsedMultiQuery ( const CSphQuery & tQuery, CSphQueryResult & tResult, const VecTraits_T<ISphMatchSorter*> & dSorters, const XQQuery_t & tXQ, DictRefPtr_c pDict, const CSphMultiQueryArgs & tArgs, CSphQueryNodeCache * pNodeCache, int64_t tmMaxTimer ) const;
	bool						RunParsedMultiQuery ( int iStackNeed, DictRefPtr_c & pDict, bool bCloneDict, const CSphQuery & tQuery, CSphQueryResult & tResult, VecTraits_T<ISphMatchSorter*> & dSorters, const XQQuery_t & tParsed, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const;

	template <bool ROWID_LIMITS>
	bool						ScanByBlocks ( const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer, const RowIdBoundaries_t * pBoundaries = nullptr ) const;
	bool						RunFullscanOnAttrs ( const RowIdBoundaries_t & tBoundaries, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer ) const;
	bool						RunFullscanOnIterator ( RowidIterator_i * pIterator, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer ) const;
	bool						MultiScan ( CSphQueryResult& tResult, const CSphQuery& tQuery, const VecTraits_T<ISphMatchSorter*>& dSorters, const CSphMultiQueryArgs& tArgs, int64_t tmMaxTimer ) const;

	template<bool USE_KLIST, bool RANDOMIZE, bool USE_FACTORS, bool HAS_SORT_CALC, bool HAS_WEIGHT_FILTER, bool HAS_FILTER_CALC, bool HAS_CUTOFF>
	void						MatchExtended ( CSphQueryContext & tCtx, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *>& dSorters, ISphRanker * pRanker, int iTag, int iIndexWeight, int iCutoff ) const;

	const CSphRowitem *			FindDocinfo ( DocID_t tDocID ) const;

	const DWORD *				GetDocinfoByRowID ( RowID_t tRowID ) const;
	RowID_t						GetRowIDByDocinfo ( const CSphRowitem * pDocinfo ) const;

	void						SetupStarDict ( DictRefPtr_c &pDict ) const;
	void						SetupExactDict ( DictRefPtr_c &pDict ) const;

	bool						RelocateBlock ( int iFile, BYTE * pBuffer, int iRelocationSize, SphOffset_t * pFileSize, CSphBin & dMinBin, SphOffset_t * pSharedOffset ); // build only

	bool						SortDocidLookup ( int iFD, int nBlocks, int iMemoryLimit, int nLookupsInBlock, int nLookupsInLastBlock, CSphIndexProgress& tProgress ); // build only

private:
	bool						JuggleFile ( ESphExt eExt, CSphString & sError, bool bNeedSrc=true, bool bNeedDst=true ) const;
	XQNode_t *					ExpandPrefix ( XQNode_t * pNode, CSphQueryResultMeta & tMeta, CSphScopedPayload * pPayloads, DWORD uQueryDebugFlags, int iQueryExpansionLimit ) const;

	static std::pair<DWORD,DWORD>		CreateRowMapsAndCountTotalDocs ( const CSphIndex_VLN* pSrcIndex, const CSphIndex_VLN* pDstIndex, CSphFixedVector<RowID_t>& dSrcRowMap, CSphFixedVector<RowID_t>& dDstRowMap, const ISphFilter* pFilter, bool bSupressDstDocids, MergeCb_c& tMonitor );
	RowsToUpdateData_t			Update_CollectRowPtrs ( const UpdateContext_t & tCtx );
	RowsToUpdate_t				Update_PrepareGatheredRowPtrs ( RowsToUpdate_t & dWRows, const VecTraits_T<DocID_t> & dDocids );
	bool						Update_WriteBlobRow ( UpdateContext_t & tCtx, RowID_t tRowID, ByteBlob_t tBlob, int nBlobAttrs, const CSphAttrLocator & tBlobRowLoc, bool & bCritical, CSphString & sError ) final;
	void						Update_MinMax ( const RowsToUpdate_t& dRows, const UpdateContext_t & tCtx );
	void						MaybeAddPostponedUpdate ( RowsToUpdateData_t& dRows, const UpdateContext_t& tCtx );
	bool						DoUpdateAttributes ( const RowsToUpdate_t& dRows, UpdateContext_t& tCtx, bool & bCritical, CSphString & sError );

	bool						Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const override;
	bool						AddRemoveColumnarAttr ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, const ISphSchema & tOldSchema, const ISphSchema & tNewSchema, CSphString & sError );
	bool						DeleteFieldFromDict ( int iFieldId, BuildHeader_t & tBuildHeader, CSphString & sError );
	bool						AddRemoveFromDocstore ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, CSphString & sError );

	bool						Build_SetupInplace ( SphOffset_t & iHitsGap, int iHitsMax, int iFdHits ) const; // fixme! build only
	bool						Build_SetupDocstore ( std::unique_ptr<DocstoreBuilder_i> & pDocstore, CSphBitvec & dStoredFields, CSphBitvec & dStoredAttrs, CSphVector<CSphVector<BYTE>> & dTmpDocstoreFieldStorage, CSphVector<CSphVector<BYTE>> & dTmpDocstoreAttrStorage ); // fixme! build only
	bool						Build_SetupBlobBuilder ( std::unique_ptr<BlobRowBuilder_i> & pBuilder ); // fixme! build only
	bool						Build_SetupColumnar ( std::unique_ptr<columnar::Builder_i> & pBuilder, CSphBitvec & tColumnarAttrs ); // fixme! build only
	bool						Build_SetupSI ( std::unique_ptr<SI::Builder_i> & pSIBuilder, std::unique_ptr<JsonSIBuilder_i> & pJsonSIBuilder, CSphBitvec & tSIAttrs, int64_t iMemoryLimit );

	void						Build_AddToDocstore ( DocstoreBuilder_i * pDocstoreBuilder, DocID_t tDocID, QueryMvaContainer_c & tMvaContainer, CSphSource & tSource, const CSphBitvec & dStoredFields, const CSphBitvec & dStoredAttrs, CSphVector<CSphVector<BYTE>> & dTmpDocstoreFieldStorage, CSphVector<CSphVector<BYTE>> & dTmpDocstoreAttrStorage, const CSphVector<std::unique_ptr<OpenHashTable_T<uint64_t, uint64_t>>> & dJoinedOffsets, CSphReader & tJoinedReader ); // fixme! build only
	bool						Build_StoreBlobAttrs ( DocID_t tDocId, std::pair<SphOffset_t,SphOffset_t> & tOffsetSize, BlobRowBuilder_i & tBlobRowBuilderconst, QueryMvaContainer_c & tMvaContainer, AttrSource_i & tSource, bool bForceSource ); // fixme! build only
	bool						Build_CollectQueryMvas ( const CSphVector<CSphSource*> & dSources, QueryMvaContainer_c & tMvaContainer ); // build only
	bool						Build_CollectJoinedFields ( const CSphVector<CSphSource*> & dSources, CSphAutofile & tFile, CSphVector<std::unique_ptr<OpenHashTable_T<uint64_t, uint64_t>>> & dJoinedOffsets );

	bool						SpawnReader ( DataReaderFactoryPtr_c & m_pFile, ESphExt eExt, DataReaderFactory_c::Kind_e eKind, int iBuffer, FileAccess_e eAccess );
	bool						SpawnReaders();

	RowIteratorsWithEstimates_t	CreateColumnarAnalyzerOrPrefilter ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, const ISphFilter * pFilter, ESphCollation eCollation, const ISphSchema & tSchema, CSphString & sWarning ) const;

	template<typename RUN>
	bool						SplitQuery ( RUN && tRun, CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const;
	bool						ChooseIterators ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphQuery & tQuery, const CSphVector<CSphFilterSettings> & dFilters, CSphQueryContext & tCtx, CreateFilterContext_t & tFlx, const ISphSchema & tMaxSorterSchema, CSphQueryResultMeta & tMeta, int iCutoff, int iThreads, CSphVector<CSphFilterSettings> & dModifiedFilters, ISphRanker * pRanker ) const;
	std::pair<RowidIterator_i *, bool> SpawnIterators ( const CSphQuery & tQuery, const CSphVector<CSphFilterSettings> & dFilters, CSphQueryContext & tCtx, CreateFilterContext_t & tFlx, const ISphSchema & tMaxSorterSchema, CSphQueryResultMeta & tMeta, int iCutoff, int iThreads, CSphVector<CSphFilterSettings> & dModifiedFilters, ISphRanker * pRanker ) const;
	bool						SelectIteratorsFT ( const CSphQuery & tQuery, const CSphVector<CSphFilterSettings> & dFilters, const ISphSchema & tSorterSchema, ISphRanker * pRanker, CSphVector<SecondaryIndexInfo_t> & dSIInfo, int iCutoff, int iThreads, StrVec_t & dWarnings ) const;

	bool						IsQueryFast ( const CSphQuery & tQuery, const CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, float fCost ) const;
	CSphVector<SecondaryIndexInfo_t> GetEnabledIndexes ( const CSphQuery & tQuery, bool bFT, float & fCost, int iThreads ) const;

	bool						SetupFiltersAndContext ( CSphQueryContext & tCtx, CreateFilterContext_t & tFlx, CSphQueryResultMeta & tMeta, const ISphSchema * & pMaxSorterSchema, CSphVector<CSphFilterSettings> & dTransformedFilters, CSphVector<FilterTreeItem_t> & dTransformedFilterTree, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs ) const;

	Docstore_i *				GetDocstore() const override { return m_pDocstore.get(); }
	columnar::Columnar_i *		GetColumnar() const override { return m_pColumnar.get(); }
	const DWORD *				GetRawAttrs() const override { return m_tAttr.GetReadPtr(); }
	const BYTE *				GetRawBlobAttrs() const override { return m_tBlobAttrs.GetReadPtr(); }
	bool						AlterSI ( CSphString & sError ) override;
};


/////////////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS
/////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////


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
	return ( m_eAttr==tOther.m_eAttr && m_eBlob==tOther.m_eBlob && m_eDoclist==tOther.m_eDoclist && m_eHitlist==tOther.m_eHitlist && m_eDict==tOther.m_eDict &&
		m_iReadBufferDocList==tOther.m_iReadBufferDocList && m_iReadBufferHitList==tOther.m_iReadBufferHitList );
}

bool FileAccessSettings_t::operator!= ( const FileAccessSettings_t & tOther ) const
{
	return !operator==( tOther );
}


//////////////////////////////////////////////////////////////////////////

RowTagged_t::RowTagged_t ( const CSphMatch & tMatch )
{
	m_tID = tMatch.m_tRowID;
	m_iTag = tMatch.m_iTag;
}

RowTagged_t::RowTagged_t ( RowID_t tRowID, int iTag )
{
	m_tID = tRowID;
	m_iTag = iTag;
}

bool RowTagged_t::operator== ( const RowTagged_t & tRow ) const
{
	return ( m_tID==tRow.m_tID && m_iTag==tRow.m_iTag );
}

bool RowTagged_t::operator!= ( const RowTagged_t & tRow ) const
{
	return !( *this==tRow );
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


/////////////////////////////////////////////////////////////////////////////
// FILTER
/////////////////////////////////////////////////////////////////////////////
void CSphFilterSettings::SetExternalValues ( const VecTraits_T<SphAttr_t>& dValues )
{
	m_dExtValues = dValues;
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
	pParser->m_sParserError.SetSprintf ( "P07: %s near '%s'", sMessage, pParser->m_pLastTokenStart );
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

CSphMultiQueryArgs::CSphMultiQueryArgs ( int iIndexWeight )
	: m_iIndexWeight ( iIndexWeight )
{
	assert ( iIndexWeight>0 );
}

/////////////////////////////////////////////////////////////////////////////
// INDEX
/////////////////////////////////////////////////////////////////////////////

CSphIndex::CSphIndex ( CSphString sIndexName, CSphString sFileBase )
	: IndexFileBase_c { sFileBase }
	, m_tSchema { std::move ( sFileBase ) }
	, m_sIndexName ( std::move ( sIndexName ) )
{
	m_iIndexId = GetIndexUid();
	m_tMutableSettings = MutableIndexSettings_c::GetDefaults();
}


CSphIndex::~CSphIndex ()
{
	QcacheDeleteIndex ( m_iIndexId );
	SkipCache::DeleteAll ( m_iIndexId );
}


void CSphIndex::SetInplaceSettings ( int iHitGap, float fRelocFactor, float fWriteFactor ) // fixme! build only
{
	m_iHitGap = iHitGap;
	m_fRelocFactor = fRelocFactor;
	m_fWriteFactor = fWriteFactor;
	m_bInplaceSettings = true;
}


void CSphIndex::SetFieldFilter ( std::unique_ptr<ISphFieldFilter> pFieldFilter )
{
	m_pFieldFilter = std::move ( pFieldFilter );
}


void CSphIndex::SetTokenizer ( TokenizerRefPtr_c pTokenizer )
{
	m_pTokenizer = std::move ( pTokenizer );
}


void CSphIndex::SetupQueryTokenizer()
{
	bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	// create and setup a master copy of query time tokenizer
	// that we can then use to create lightweight clones
	m_pQueryTokenizer = sphCloneAndSetupQueryTokenizer ( m_pTokenizer, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, false );
	m_pQueryTokenizerJson = sphCloneAndSetupQueryTokenizer ( m_pTokenizer, IsStarDict ( bWordDict ), m_tSettings.m_bIndexExactWords, true );
}

void CSphIndex::PostSetup()
{
	SetupQueryTokenizer();

	const CSphDictSettings & tDictSettings = m_pDict->GetSettings();
	if ( !ParseMorphFields ( tDictSettings.m_sMorphology, tDictSettings.m_sMorphFields, m_tSchema.GetFields(), m_tMorphFields, m_sLastError ) )
		sphWarning ( "table '%s': %s", GetName(), m_sLastError.cstr() );
}

TokenizerRefPtr_c CSphIndex::GetTokenizer() const
{
	return m_pTokenizer;
}

TokenizerRefPtr_c CSphIndex::GetQueryTokenizer() const
{
	return m_pQueryTokenizer;
}

TokenizerRefPtr_c&	CSphIndex::ModifyTokenizer ()
{
	return m_pTokenizer;
}


void CSphIndex::SetDictionary ( DictRefPtr_c pDict )
{
	m_pDict = std::move ( pDict );
}

DictRefPtr_c CSphIndex::GetDictionary() const
{
	return m_pDict;
}

void CSphIndex::Setup ( const CSphIndexSettings & tSettings )
{
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

bool CSphIndex::HasGlobalIDF() const
{
	return ( !m_sGlobalIDFPath.IsEmpty() && sph::GetIDFer ( m_sGlobalIDFPath ) );
}


int CSphIndex::UpdateAttributes ( AttrUpdateSharedPtr_t pUpd, bool & bCritical, CSphString & sError, CSphString & sWarning )
{
	AttrUpdateInc_t tUpdInc { std::move (pUpd) };
	return UpdateAttributes ( tUpdInc, bCritical, sError, sWarning );
}

int CSphIndex::UpdateAttributes ( AttrUpdateInc_t& tUpd, bool& bCritical, CSphString& sError, CSphString& sWarning )
{
	return CheckThenUpdateAttributes ( tUpd, bCritical, sError, sWarning );
}

CSphVector<SphAttr_t> CSphIndex::BuildDocList () const
{
	TlsMsg::ResetErr(); // reset error
	return {};
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


static bool DetectNonClonableSorters ( const CSphQuery & tQuery )
{
	if ( !tQuery.m_sGroupDistinct.IsEmpty() )
		return true;

	// FIXME: also need to handle 
	// 1. Stateful UDFs
	// 2. Update/delete queue

	return false;
}


static bool DetectPrecalcSorters ( const CSphQuery & tQuery, const ISphSchema & tIndexSchema, bool bHasSI )
{
	if ( tQuery.m_dItems.any_of ( []( auto & tItem ){ return tItem.m_eAggrFunc!=SPH_AGGR_NONE; } ) )
		return false;

	if ( !HasImplicitGrouping(tQuery) )
		return false;

	if ( !tQuery.m_sQuery.IsEmpty() )
		return false;

	if ( !tQuery.m_sKNNAttr.IsEmpty() )
		return false;

	bool bDistinct = !tQuery.m_sGroupDistinct.IsEmpty();
	if ( bHasSI )
	{
		// check for count distinct precalc
		if ( bDistinct && tQuery.m_dFilters.IsEmpty() )
			return true;

		// check for count(*) precalc w/one filter
		if ( !bDistinct && tQuery.m_dFilters.GetLength()==1 )
		{
			if ( tIndexSchema.GetAttr ( tQuery.m_dFilters[0].m_sAttrName.cstr() ) )
				return true;
		}
	}

	// check for count(*) w/o filters
	if ( !bDistinct && tQuery.m_dFilters.IsEmpty() )
		return true;

	return false;
}


bool CSphIndex::MustRunInSingleThread ( const VecTraits_T<const CSphQuery> & dQueries, bool bHasSI, const VecTraits_T<int64_t> & dMaxCountDistinct, bool & bForceSingleThread ) const
{
	ARRAY_FOREACH ( i, dQueries )
	{
		auto & tQuery = dQueries[i];

		// check for potential non-clonable sorters (we don't have actual sorters at this stage)
		if ( DetectNonClonableSorters(tQuery) )
			return true;

		if ( DetectPrecalcSorters ( tQuery, m_tSchema, bHasSI ) )
			return true;

		// at this point we are trying to decide how many threads this index gets
		// we did not correct max_matches yet (to achieve max grouping accuracy)
		// but if increasing max_matches would be enough to achieve max accuracy, there's no need to turn off multithreading
		// that's why now we try to guess if increasing max_matches is enough
		int iMaxCountDistinct = dMaxCountDistinct[i];
		bool bAccurateAggregation = tQuery.m_bExplicitAccurateAggregation ? tQuery.m_bAccurateAggregation : GetAccurateAggregationDefault();
		if ( bAccurateAggregation && !tQuery.m_sGroupBy.IsEmpty() )
		{
			int iGroupby = GetAliasedAttrIndex ( tQuery.m_sGroupBy, tQuery, m_tSchema );
			if ( iGroupby>0 )
			{
				if ( iMaxCountDistinct==-1 )
				{
					CSphString sModifiedAttr;
					iMaxCountDistinct = GetCountDistinct ( tQuery.m_sGroupBy, sModifiedAttr );
				}

				if ( iMaxCountDistinct==-1 )
				{
					bForceSingleThread = true;
					return true;	// no info on max_matches; disable ps
				}
				else
				{
					if ( tQuery.m_bExplicitMaxMatches && iMaxCountDistinct > tQuery.m_iMaxMatches )
					{
						bForceSingleThread = true;
						return true;	// can't change max_matches and not enough were set; disable ps
					}

					if ( iMaxCountDistinct > tQuery.m_iMaxMatchThresh )
					{
						bForceSingleThread = true;
						return true; // max_matches can't be increased; disable ps
					}
				}
			}
		}
	}

	return GetStats().m_iTotalDocuments<=g_iPseudoShardingThresh;
}

//////////////////////////////////////////////////////////////////////////

static void PooledAttrsToPtrAttrs ( const VecTraits_T<ISphMatchSorter *> & dSorters, const BYTE * pBlobPool, columnar::Columnar_i * pColumnar, bool bFinalizeMatches, QueryProfile_c * pProfile, bool bModifySorterSchemas )
{
	if ( !bModifySorterSchemas )
		return;

	CSphScopedProfile tProfile ( pProfile, SPH_QSTATE_DYNAMIC );

	dSorters.Apply ( [&] ( ISphMatchSorter * p )
	{
		if ( p )
			p->TransformPooled2StandalonePtrs ( [pBlobPool] ( const CSphMatch * ) { return pBlobPool; }, [pColumnar] ( const CSphMatch * ) { return pColumnar; }, bFinalizeMatches );
	});
}


std::unique_ptr<CSphIndex> sphCreateIndexPhrase ( CSphString sIndexName, CSphString sFilename )
{
	return std::make_unique<CSphIndex_VLN> ( std::move ( sIndexName ), std::move ( sFilename ) );
}

//////////////////////////////////////////////////////////////////////////


CSphIndex_VLN::CSphIndex_VLN ( CSphString sIndexName, CSphString sFilename )
	: CSphIndex ( std::move ( sIndexName ), std::move ( sFilename ) )
	, m_iLockFD ( -1 )
	, m_dFieldLens ( SPH_MAX_FIELDS )
{
	m_iDocinfo = 0;
	m_iDocinfoIndex = 0;
	m_pDocinfoIndex = nullptr;

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

	static inline void HintDocID ( DocID_t ) {}

private:
	const int * m_pCur;
	const int * m_pEnd;
	const VecTraits_T<DocID_t> & m_dDocids;
};



// fill collect rows which will be updated in this index
RowsToUpdateData_t CSphIndex_VLN::Update_CollectRowPtrs ( const UpdateContext_t & tCtx )
{
	TRACE_CORO ( "sph", "CSphIndex_VLN::Update_CollectRowPtrs" );
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
	Intersect ( tLookupReader, tSortedReader, [&dRowsToUpdate, this] ( RowID_t tRowID, DocID_t, DocIdIndexReader_c& tSortedReader )
	{
		if ( m_tDeadRowMap.IsSet ( tRowID ) )
			return;
		auto& dUpd = dRowsToUpdate.Add();
		dUpd.m_tRow = tRowID;
		dUpd.m_iIdx = tSortedReader.GetIndex();
		assert ( dUpd.m_tRow != INVALID_ROWID );
	} );
	return dRowsToUpdate;
}

// We fill docinfo ptr for actual rows, and move out non-actual (the ones which doesn't point to existing document)
// note, it actually changes (rearranges) rows!
RowsToUpdate_t CSphIndex_VLN::Update_PrepareGatheredRowPtrs ( RowsToUpdate_t & dWRows, const VecTraits_T<DocID_t>& dDocids )
{
	RowsToUpdate_t & dRows = dWRows; // that is actually to indicate that we CHANGE contents inside dWRows, so it should be passed by non-const reference.

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
			dRows[iWriteIdx].m_tRow = tRowID;
			assert ( tRowID != INVALID_ROWID );
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


bool CSphIndex_VLN::Update_WriteBlobRow ( UpdateContext_t & tCtx, RowID_t tRowID, ByteBlob_t tBlob, int nBlobAttrs, const CSphAttrLocator & tBlobRowLoc, bool & bCritical, CSphString & sError )
{
	auto pDocinfo = tCtx.GetDocinfo ( tRowID );
	BYTE * pExistingBlob = m_tBlobAttrs.GetWritePtr() + sphGetRowAttr ( pDocinfo, tBlobRowLoc );
	DWORD uExistingBlobLen = sphGetBlobTotalLen ( pExistingBlob, nBlobAttrs );

	bCritical = false;

	// overwrite old record (because we have write-lock)
	if ( (DWORD)tBlob.second<=uExistingBlobLen )
	{
		memcpy ( pExistingBlob, tBlob.first, tBlob.second );
		return true;
	}

	BYTE * pOldBlobPool = m_tBlobAttrs.GetWritePtr();
	SphOffset_t tBlobSpaceUsed = *(SphOffset_t*)pOldBlobPool;
	SphOffset_t tSpaceLeft = m_tBlobAttrs.GetLengthBytes()-tBlobSpaceUsed;

	// not great: we have to resize our .spb file and create new memory maps
	if ( (SphOffset_t)tBlob.second > tSpaceLeft )
	{
		SphOffset_t tSizeDelta = Max ( (SphOffset_t)tBlob.second-tSpaceLeft, m_tSettings.m_tBlobUpdateSpace );
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
	memcpy ( pEnd, tBlob.first, tBlob.second );
	sphSetRowAttr ( pDocinfo, tBlobRowLoc, tBlobSpaceUsed );
	tBlobSpaceUsed += tBlob.second;
	*(SphOffset_t*)m_tBlobAttrs.GetWritePtr() = tBlobSpaceUsed;
	return true;
}


void CSphIndex_VLN::Update_MinMax ( const RowsToUpdate_t& dRows, const UpdateContext_t & tCtx )
{
	int iRowStride = tCtx.m_tSchema.GetRowSize();

	for ( const auto & tRow : dRows )
	{
		auto iBlock = (int64_t) tRow.m_tRow / DOCINFO_INDEX_FREQ;
		DWORD * pBlockRanges = m_pDocinfoIndex + ( iBlock * iRowStride * 2 );
		DWORD * pIndexRanges = m_pDocinfoIndex + ( m_iDocinfoIndex * iRowStride * 2 );
		assert ( iBlock>=0 && iBlock<m_iDocinfoIndex );
		const auto* pDocinfo = tCtx.GetDocinfo ( tRow.m_tRow );

		ARRAY_CONSTFOREACH ( iCol, tCtx.m_tUpd.m_pUpdate->m_dAttributes )
		{
			const UpdatedAttribute_t & tUpdAttr = tCtx.m_dUpdatedAttrs[iCol];
			if ( !tUpdAttr.m_bExisting )
				continue;

			const CSphAttrLocator & tLoc = tUpdAttr.m_tLocator;
			if ( tLoc.IsBlobAttr() )
				continue;

			SphAttr_t uValue = sphGetRowAttr ( pDocinfo, tLoc );

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

// Collect updated docs and store them into vec of
// postponed updates (it might happen be more than one update during the operation)
void CSphIndex_VLN::MaybeAddPostponedUpdate ( RowsToUpdateData_t& dRows, const UpdateContext_t& tCtx )
{
	TRACE_CORO ( "sph", "CSphIndex_VLN::MaybeAddPostponedUpdate" );
	if ( !m_bAttrsBusy.load ( std::memory_order_acquire ) )
		return;

	auto& tUpd = tCtx.m_tUpd;
	auto& tNewUpdate = m_dPostponedUpdates.Add();
	tNewUpdate.m_pUpdate = MakeReusableUpdate ( tUpd.m_pUpdate );
	tNewUpdate.m_dRowsToUpdate.SwapData ( dRows );
}


bool CSphIndex_VLN::DoUpdateAttributes ( const RowsToUpdate_t& dRows, UpdateContext_t& tCtx, bool& bCritical, CSphString& sError )
{
	TRACE_CORO ( "sph", "CSphIndex_VLN::DoUpdateAttributes" );
	if ( dRows.IsEmpty() )
		return true;

	if ( !Update_CheckAttributes ( *tCtx.m_tUpd.m_pUpdate, tCtx.m_tSchema, sError ) )
		return false;

	tCtx.m_pHistograms = m_pHistograms;
	tCtx.m_pBlobPool = m_tBlobAttrs.GetWritePtr();
	tCtx.m_pAttrPool = m_tAttr.GetWritePtr ();

	tCtx.PrepareListOfUpdatedAttributes ( sError );

	if ( !Update_UpdateAttributes ( dRows, tCtx, bCritical, sError ) )
		return false;
	Update_MinMax ( dRows, tCtx );
	return true;
}
void CommitUpdateAttributes ( int64_t * pTID, const char * szName, const CSphAttrUpdate & tUpd )
{
	CSphString sError;
	Binlog::Commit ( pTID, szName, sError, [&tUpd] ( Writer_i & tWriter ) {

		// my user op
		tWriter.PutByte ( Binlog::UPDATE_ATTRS );

		// update data
		tWriter.ZipOffset ( tUpd.m_dAttributes.GetLength () );
		for ( const auto & i: tUpd.m_dAttributes )
		{
			tWriter.PutZString ( i.m_sName );
			tWriter.ZipOffset ( i.m_eType );
		}

		// POD vectors
		Binlog::SaveVector ( tWriter, tUpd.m_dPool );
		Binlog::SaveVector ( tWriter, tUpd.m_dDocids );
		Binlog::SaveVector ( tWriter, tUpd.m_dRowOffset );
		Binlog::SaveVector ( tWriter, tUpd.m_dBlobs );
	} );
}


Binlog::CheckTnxResult_t CSphIndex::ReplayUpdate ( CSphReader & tReader, CSphString & sError, Binlog::CheckTxn_fn && fnCanContinue )
{
	// load transaction data
	AttrUpdateSharedPtr_t pUpd { new CSphAttrUpdate };
	auto & tUpd = *pUpd;
	tUpd.m_bIgnoreNonexistent = true;

	int iAttrs = (int) tReader.UnzipOffset ();
	tUpd.m_dAttributes.Resize ( iAttrs ); // FIXME! sanity check
	for ( auto & i: tUpd.m_dAttributes )
	{
		i.m_sName = tReader.GetZString ();
		i.m_eType = (ESphAttr) tReader.UnzipOffset (); // safe, we'll crc check later
	}

	if ( tReader.GetErrorFlag () )
		return {};

	if ( !Binlog::LoadVector ( tReader, tUpd.m_dPool ) ) return {};
	if ( !Binlog::LoadVector ( tReader, tUpd.m_dDocids ) ) return {};
	if ( !Binlog::LoadVector ( tReader, tUpd.m_dRowOffset ) ) return {};
	if ( !Binlog::LoadVector ( tReader, tUpd.m_dBlobs ) ) return {};

	Binlog::CheckTnxResult_t tRes = fnCanContinue ();
	if ( tRes.m_bValid && tRes.m_bApply )
	{
		CSphString sError, sWarning;
		bool bCritical = false;
		UpdateAttributes ( pUpd, bCritical, sError, sWarning ); // FIXME! check for errors
		assert ( !bCritical ); // fixme! handle this
		tRes.m_bApply = true;
	}
	return tRes;
}


Binlog::CheckTnxResult_t CSphIndex_VLN::ReplayTxn ( CSphReader & tReader, CSphString & sError, BYTE uOp, Binlog::CheckTxn_fn && fnCanContinue )
{
	switch ( uOp )
	{
	case Binlog::UPDATE_ATTRS:
		return ReplayUpdate ( tReader, sError, std::move ( fnCanContinue ) );
	default:
		assert ( false && "unknown op provided to replay" );
	}
	return {};
}

int CSphIndex_VLN::CheckThenUpdateAttributes ( AttrUpdateInc_t& tUpd, bool& bCritical, CSphString& sError, CSphString& sWarning )
{
	TRACE_CORO ( "sph", "CSphIndex_VLN::CheckThenUpdateAttributes" );
	assert ( tUpd.m_pUpdate->m_dRowOffset.IsEmpty() || tUpd.m_pUpdate->m_dDocids.GetLength()==tUpd.m_pUpdate->m_dRowOffset.GetLength() );

	// check if we have to
	if ( !m_iDocinfo || tUpd.m_pUpdate->m_dDocids.IsEmpty() )
		return 0;

	UpdateContext_t tCtx ( tUpd, m_tSchema );
	int iUpdated = tUpd.m_iAffected;

	auto dRowsToUpdate = Update_CollectRowPtrs ( tCtx );

	if ( !DoUpdateAttributes ( dRowsToUpdate, tCtx, bCritical, sError ))
		return -1;

	MaybeAddPostponedUpdate ( dRowsToUpdate, tCtx );

	if ( tCtx.m_uUpdateMask && m_bBinlog )
		CommitUpdateAttributes ( &m_iTID, GetName(), *tUpd.m_pUpdate );

	m_uAttrsStatus |= tCtx.m_uUpdateMask; // FIXME! add lock/atomic?

	if ( ( tCtx.m_uUpdateMask & IndexSegment_c::ATTRS_UPDATED ) || ( tCtx.m_uUpdateMask & IndexSegment_c::ATTRS_BLOB_UPDATED ) )
	{
		for ( const UpdatedAttribute_t & tAttr : tCtx.m_dUpdatedAttrs )
			if ( tAttr.m_iSchemaAttr!=-1 )
				m_tSI.ColumnUpdated ( m_tSchema.GetAttr ( tAttr.m_iSchemaAttr ).m_sName );
	}

	iUpdated = tUpd.m_iAffected - iUpdated;
	if ( !tCtx.HandleJsonWarnings ( iUpdated, sWarning, sError ) )
		return -1;

	return iUpdated;
}

void CSphIndex_VLN::UpdateAttributesOffline ( VecTraits_T<PostponedUpdate_t> & dPostUpdates )
{
	if ( dPostUpdates.IsEmpty () )
		return;

	CSphString sError;
	bool bCritical;

	for ( auto & tPostUpdate : dPostUpdates )
	{
		RowsToUpdate_t dRows = Update_PrepareGatheredRowPtrs ( tPostUpdate.m_dRowsToUpdate, tPostUpdate.m_pUpdate->m_dDocids );

		AttrUpdateInc_t tUpdInc { tPostUpdate.m_pUpdate }; // don't move, keep update (need twice when split chunks)
		UpdateContext_t tCtx ( tUpdInc, m_tSchema );
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
	CSphString sExt = GetFilename ( eExt );
	CSphString sExtNew = GetTmpFilename ( eExt );
	CSphString sExtOld = SphSprintf ( "%s.old", sExt.cstr() );

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
				sError.SetSprintf ( "rollback rename to '%s' failed: %s; TABLE UNUSABLE; FIX FILE NAMES MANUALLY", sExt.cstr(), strerror(errno) );
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

	sphLogDebugvv ( "table '%s' attrs (%u) saving...", GetName(), uAttrStatus );

	if ( uAttrStatus & IndexSegment_c::ATTRS_UPDATED )
	{
		if ( !m_tAttr.Flush ( true, sError ) )
			return false;

		if ( m_pHistograms && !m_pHistograms->Save ( GetFilename ( SPH_EXT_SPHI ), sError ) )
			return false;
	}

	if ( uAttrStatus & IndexSegment_c::ATTRS_BLOB_UPDATED )
	{
		if ( !m_tBlobAttrs.Flush ( true, sError ) )
			return false;
	}

	if ( uAttrStatus & IndexSegment_c::ATTRS_ROWMAP_UPDATED )
	{
		if ( !m_tDeadRowMap.Flush ( true, sError ) )
			return false;
	}

	bool bAttrsUpdated = ( uAttrStatus & IndexSegment_c::ATTRS_UPDATED ) || ( uAttrStatus & IndexSegment_c::ATTRS_BLOB_UPDATED );
	if ( bAttrsUpdated && !m_tSI.SaveMeta(sError) )
		return false;

	if ( m_bBinlog )
		Binlog::NotifyIndexFlush ( m_iTID, GetName(), Binlog::NoShutdown, Binlog::NoSave );

	if ( m_uAttrsStatus==uAttrStatus )
		m_uAttrsStatus = 0;

	sphLogDebugvv ( "table '%s' attrs (%u) saved", GetName(), m_uAttrsStatus );

	return true;
}

DWORD CSphIndex_VLN::GetAttributeStatus () const
{
	return m_uAttrsStatus;
}


//////////////////////////////////////////////////////////////////////////

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
	return pDocinfo-m_tAttr.GetReadPtr() >= m_iDocinfo*iStride;
}


bool CSphIndex_VLN::AddRemoveColumnarAttr ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, const ISphSchema & tOldSchema, const ISphSchema & tNewSchema, CSphString & sError )
{
	BuildBufferSettings_t tSettings; // use default buffer settings
	auto pBuilder = CreateColumnarBuilder ( tNewSchema, GetTmpFilename ( SPH_EXT_SPC ), tSettings.m_iBufferColumnar, sError );
	if ( !pBuilder )
		return false;

	return Alter_AddRemoveColumnar ( bAddAttr, m_tSchema, tNewSchema, m_pColumnar.get(), pBuilder.get(), (DWORD)m_iDocinfo, GetName(), sError );
}


bool CSphIndex_VLN::AddRemoveAttribute ( bool bAddAttr, const AttrAddRemoveCtx_t & tCtx, CSphString & sError )
{
	AttrEngine_e eAttrEngine = CombineEngines ( m_tSettings.m_eEngine, tCtx.m_eEngine );
	AttrAddRemoveCtx_t tNewCtx = tCtx;
	if ( eAttrEngine==AttrEngine_e::COLUMNAR )
		tNewCtx.m_uFlags |= CSphColumnInfo::ATTR_COLUMNAR;
	else
		tNewCtx.m_uFlags &= ~( CSphColumnInfo::ATTR_COLUMNAR_HASHES | CSphColumnInfo::ATTR_STORED );

	CSphSchema tNewSchema = m_tSchema;
	if ( !Alter_AddRemoveFromSchema ( tNewSchema, tNewCtx, bAddAttr, sError ) )
		return false;

	int iNewStride = tNewSchema.GetRowSize();
	int64_t iNewMinMaxIndex = m_iDocinfo * iNewStride;

	BuildHeader_t tBuildHeader;
	WriteHeader_t tWriteHeader;
	PrepareHeaders ( tBuildHeader, tWriteHeader );

	tBuildHeader.m_iMinMaxIndex = iNewMinMaxIndex;
	tWriteHeader.m_pSchema = &tNewSchema;

	// save the header
	if ( !IndexBuildDone ( tBuildHeader, tWriteHeader, GetTmpFilename(SPH_EXT_SPH), sError ) )
		return false;

	// generate new .SPA, .SPB files
	CSphWriter tSPAWriter;
	CSphWriter tSPBWriter;
	tSPAWriter.SetBufferSize ( 524288 );
	tSPBWriter.SetBufferSize ( 524288 );

	std::unique_ptr<WriteWrapper_c> pSPAWriteWrapper { CreateWriteWrapperDisk(tSPAWriter) };
	std::unique_ptr<WriteWrapper_c> pSPBWriteWrapper { CreateWriteWrapperDisk(tSPBWriter) };

	CSphString sSPAfile = GetTmpFilename ( SPH_EXT_SPA );
	CSphString sSPBfile = GetTmpFilename ( SPH_EXT_SPB );
	CSphString sSPHIfile = GetTmpFilename ( SPH_EXT_SPHI );
	if ( !tSPAWriter.OpenFile ( sSPAfile, sError ) )
		return false;

	bool bHadBlobs = false;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		bHadBlobs |= sphIsBlobAttr ( m_tSchema.GetAttr(i) );

	bool bHaveBlobs = false;
	for ( int i = 0; i < tNewSchema.GetAttrsCount(); i++ )
		bHaveBlobs |= sphIsBlobAttr ( tNewSchema.GetAttr(i) );

	bool bBlob = sphIsBlobAttr ( tCtx.m_eType );
	bool bBlobsModified = bBlob && ( bAddAttr || bHaveBlobs==bHadBlobs );
	if ( bBlobsModified )
	{
		if ( !tSPBWriter.OpenFile ( sSPBfile, sError ) )
			return false;

		tSPBWriter.PutOffset(0);
	}

	if ( !tNewSchema.GetAttrsCount() )
	{
		sError = "table must have at least one attribute";
		return false;
	}

	bool bColumnar = bAddAttr ? tNewSchema.GetAttr ( tCtx.m_sName.cstr() )->IsColumnar() : m_tSchema.GetAttr ( tCtx.m_sName.cstr() )->IsColumnar();
	if ( bColumnar )
		AddRemoveColumnarAttr ( bAddAttr, tCtx.m_sName, tCtx.m_eType, m_tSchema, tNewSchema, sError );
	else
	{
		int64_t iTotalRows = m_iDocinfo + (m_iDocinfoIndex+1)*2;
		Alter_AddRemoveRowwiseAttr ( m_tSchema, tNewSchema, m_tAttr.GetReadPtr(), (DWORD)iTotalRows, m_tBlobAttrs.GetReadPtr(), *pSPAWriteWrapper, *pSPBWriteWrapper, bAddAttr, tCtx.m_sName );
	}

	if ( m_pHistograms )
	{
		if ( bAddAttr )
		{
			std::unique_ptr<Histogram_i> pNewHistogram = CreateHistogram ( tCtx.m_sName, tCtx.m_eType );
			if ( pNewHistogram )
			{
				for ( DWORD i = 0; i < m_iDocinfo; i++ )
					pNewHistogram->Insert(0);

				m_pHistograms->Add ( std::move ( pNewHistogram ) );
			}
		}
		else
			m_pHistograms->Remove ( tCtx.m_sName );

		if ( !m_pHistograms->Save ( sSPHIfile, sError ) )
			return false;
	}

	if ( !AddRemoveFromDocstore ( m_tSchema, tNewSchema, sError ) )
		return false;

	if ( tSPAWriter.IsError() )
	{
		sError.SetSprintf ( "error writing to %s", sSPAfile.cstr() );
		return false;
	}

	tSPAWriter.CloseFile();

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
			m_pColumnar = CreateColumnarStorageReader ( GetFilename ( SPH_EXT_SPC ), (DWORD)m_iDocinfo, sError );
			if ( !m_pColumnar )
				return false;
		}
		else
			m_pColumnar.reset();
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

	if ( bHaveNonColumnar && !m_tAttr.Setup ( GetFilename ( SPH_EXT_SPA ), sError, true ) )
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

			if ( !m_tBlobAttrs.Setup ( GetFilename ( SPH_EXT_SPB ), sError, true ) )
				return false;
		} else
			::unlink ( GetFilename ( SPH_EXT_SPB ).cstr() );
	}

	m_tSchema = tNewSchema;
	m_iMinMaxIndex = iNewMinMaxIndex;
	m_pDocinfoIndex = m_tAttr.GetWritePtr() + m_iMinMaxIndex;

	PrereadMapping ( GetName(), "attributes", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eAttr ), m_tAttr );

	if ( bBlobsModified )
		PrereadMapping ( GetName(), "blob attributes", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eBlob ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eBlob ), m_tBlobAttrs );

	return true;
}


void CSphIndex_VLN::PrepareHeaders ( BuildHeader_t & tBuildHeader, WriteHeader_t & tWriteHeader, bool bCopyDictHeader )
{
	tBuildHeader.m_iTotalDocuments = m_tStats.m_iTotalDocuments;
	tBuildHeader.m_iTotalBytes = m_tStats.m_iTotalBytes;
	tBuildHeader.m_iDocinfo = m_iDocinfo;
	tBuildHeader.m_iDocinfoIndex = m_iDocinfoIndex;
	tBuildHeader.m_iMinMaxIndex = m_iMinMaxIndex;

	if ( bCopyDictHeader )
		*(DictHeader_t*)&tBuildHeader = *(DictHeader_t*)&m_tWordlist;

	tWriteHeader.m_pSettings = &m_tSettings;
	tWriteHeader.m_pSchema = &m_tSchema;
	tWriteHeader.m_pTokenizer = m_pTokenizer;
	tWriteHeader.m_pDict = m_pDict;
	tWriteHeader.m_pFieldFilter = m_pFieldFilter.get();
	tWriteHeader.m_pFieldLens = m_dFieldLens.Begin();
	tWriteHeader.m_pSI = &m_tSI;
}


bool CSphIndex_VLN::SaveHeader ( CSphString & sError )
{
	BuildHeader_t tBuildHeader;
	WriteHeader_t tWriteHeader;
	PrepareHeaders ( tBuildHeader, tWriteHeader );

	// save the header
	if ( !IndexBuildDone ( tBuildHeader, tWriteHeader, GetTmpFilename(SPH_EXT_SPH), sError ) )
		return false;

	return JuggleFile ( SPH_EXT_SPH, sError );
}


void CSphIndex_VLN::FlushDeadRowMap ( bool bWaitComplete ) const
{
	// FIXME! handle errors
	CSphString sError;
	m_tDeadRowMap.Flush ( bWaitComplete, sError );
}


bool CSphIndex_VLN::LoadKillList ( CSphFixedVector<DocID_t> *pKillList, KillListTargets_c & tTargets, CSphString & sError ) const
{
	CSphString sSPK = GetFilename ( SPH_EXT_SPK );
	if ( !sphIsReadable ( sSPK ) )
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

	if ( !WriteKillList ( GetTmpFilename ( SPH_EXT_SPK ), dKillList.Begin(), dKillList.GetLength(), tTargets, sError ) )
		return false;

	if ( !JuggleFile ( SPH_EXT_SPK, sError, false ) )
		return false;

	return true;
}


void CSphIndex_VLN::KillExistingDocids ( CSphIndex * pTarget ) const
{
	// FIXME! collecting all docids is a waste of memory
	LookupReaderIterator_c tLookup ( m_tDocidLookup.GetReadPtr() );
	CSphFixedVector<DocID_t> dKillList ( m_iDocinfo );
	for ( auto& dKill : dKillList )
		tLookup.ReadDocID ( dKill );

	pTarget->KillMulti ( dKillList );
}


int CSphIndex_VLN::KillMulti ( const VecTraits_T<DocID_t> & dKlist )
{
	LookupReaderIterator_c tTargetReader ( m_tDocidLookup.GetReadPtr() );
	DocidListReader_c tKillerReader ( dKlist );

	int iTotalKilled;
	if ( !HasKillHook() )
		iTotalKilled = KillByLookup ( tTargetReader, tKillerReader, m_tDeadRowMap );
	else
		iTotalKilled = ProcessIntersected ( tTargetReader, tKillerReader, [this] ( RowID_t tRow, DocID_t tDoc )
		{
			if ( !m_tDeadRowMap.Set ( tRow ) )
				return false;
			KillHook ( tDoc );
			return true;
		} );

	if ( iTotalKilled )
		m_uAttrsStatus |= IndexSegment_c::ATTRS_ROWMAP_UPDATED;

	return iTotalKilled;
}

int CSphIndex_VLN::KillDupes()
{
	LookupReaderIterator_c tLookup ( m_tDocidLookup.GetReadPtr() );
	int iTotalKilled = 0;

	RowID_t tRowID = INVALID_ROWID;
	DocID_t tLastDocID = 0, tDocID = 0;

	while ( tLookup.Read ( tDocID, tRowID ) )
	{
		if ( tDocID == tLastDocID )
		{
			m_tDeadRowMap.Set ( tRowID );
			++iTotalKilled;
			continue;
		}
		tLastDocID = tDocID;
	}

	if ( iTotalKilled )
		m_uAttrsStatus |= IndexSegment_c::ATTRS_ROWMAP_UPDATED;

	return iTotalKilled;
}

int CSphIndex_VLN::CheckThenKillMulti ( const VecTraits_T<DocID_t>& dKlist, BlockerFn&& fnWatcher )
{
	LookupReaderIterator_c tTargetReader ( m_tDocidLookup.GetReadPtr() );
	DocidListReader_c tKillerReader ( dKlist );

	int iTotalKilled = ProcessIntersected ( tTargetReader, tKillerReader, [this,fnWatcher=std::move(fnWatcher)] ( RowID_t tRow, DocID_t tDoc )
	{
		if ( m_tDeadRowMap.IsSet ( tRow ) ) // already killed, nothing to do.
			return false;

		if ( !fnWatcher() )
			return false;

		Verify ( m_tDeadRowMap.Set ( tRow ) );
		KillHook ( tDoc );
		return true;
	} );

	if ( iTotalKilled )
		m_uAttrsStatus |= IndexSegment_c::ATTRS_ROWMAP_UPDATED;

	return iTotalKilled;
};


bool CSphIndex_VLN::IsQueryFast ( const CSphQuery & tQuery, const CSphVector<SecondaryIndexInfo_t> & dEnabledIndexes, float fCost ) const
{
	const float COST_THRESH = 0.5f;

	if ( tQuery.m_sQuery.IsEmpty() )
	{
		int iNumIterators = dEnabledIndexes.count_of ( [] ( const auto & tIndex ){ return tIndex.m_eType!=SecondaryIndexType_e::FILTER && tIndex.m_eType!=SecondaryIndexType_e::NONE; } );
		int iNumFilters = dEnabledIndexes.count_of ( [] ( const auto & tIndex ){ return tIndex.m_eType==SecondaryIndexType_e::FILTER; } );
		return iNumIterators && !iNumFilters && fCost<=COST_THRESH;
	}

	if ( m_pFieldFilter )
		return false;

	if ( !tQuery.m_sQueryTokenFilterName.IsEmpty() )
		return false;

	if ( m_tSettings.m_uAotFilterMask!=0 )
		return false;

	if ( m_tSettings.GetMinPrefixLen ( m_pDict->GetSettings().m_bWordDict )>0 )
		return false;

	if ( m_tSettings.m_iMinInfixLen>0 )
		return false;

	GetKeywordsSettings_t tSettings;
	tSettings.m_bStats = true;
	tSettings.m_iCutoff = 1;

	CSphVector <CSphKeywordInfo> dKeywords;
	if ( !GetKeywords ( dKeywords, tQuery.m_sQuery.cstr(), tSettings, nullptr ) )
		return true;

	if ( dKeywords.GetLength()<1 )
		return true;

	if ( dKeywords.GetLength()>1 )
		return false;

	const int DOCS_THRESH = 1024;
	return dKeywords[0].m_iDocs<=DOCS_THRESH;
}


static bool CheckQueryFilters ( const CSphQuery & tQuery, const CSphSchema & tIndexSchema )
{
	for ( auto & tFilter : tQuery.m_dFilters )
	{
		CommonFilterSettings_t tFixedSettings;
		CSphString sError;
		CreateFilterContext_t tCtx;
		tCtx.m_pMatchSchema = &tIndexSchema;
		tCtx.m_pIndexSchema = &tIndexSchema;
		if ( !FixupFilterSettings ( tFilter, tFixedSettings, tCtx, tFilter.m_sAttrName, sError ) )
			return false;
	}

	return true;
}


CSphVector<SecondaryIndexInfo_t> CSphIndex_VLN::GetEnabledIndexes ( const CSphQuery & tQuery, bool bFT, float & fCost, int iThreads ) const
{
	// if there's a filter tree, we don't have any indexes and there's no point in wasting time to eval them
	if ( tQuery.m_dFilterTree.GetLength() )
		return {};

	int iCutoff = ApplyImplicitCutoff ( tQuery, {}, bFT );

	StrVec_t dWarnings;
	SelectIteratorCtx_t tCtx ( tQuery, tQuery.m_dFilters, m_tSchema, m_tSchema, m_pHistograms, m_pColumnar.get(), m_tSI, iCutoff, m_iDocinfo, iThreads );
	return SelectIterators ( tCtx, fCost, dWarnings );
}


std::pair<int64_t,int> CSphIndex_VLN::GetPseudoShardingMetric ( const VecTraits_T<const CSphQuery> & dQueries, const VecTraits_T<int64_t> & dMaxCountDistinct, int iThreads, bool & bForceSingleThread ) const
{
	if ( MustRunInSingleThread ( dQueries, !m_tSI.IsEmpty(), dMaxCountDistinct, bForceSingleThread ) )
		return { 0, 1 };

	bool bAllFast = true;
	int iThreadCap = 0;
	int iNumProc = GetNumPhysicalCPUs();
	if ( iNumProc==-1 )
		iNumProc = GetNumLogicalCPUs()/2;

	ARRAY_FOREACH ( i, dQueries )
	{
		auto & tQuery = dQueries[i];

		// limit the number of threads for anything with FT as it looks better in average (some queries are faster without thread cap)
		bool bFulltext = !tQuery.m_pQueryParser->IsFullscan(tQuery);
		if ( bFulltext )
			iThreadCap = iThreadCap ? Min ( iThreadCap, iNumProc ) : iNumProc;

		if ( !tQuery.m_sKNNAttr.IsEmpty() )
			iThreadCap = 1;

		if ( !CheckQueryFilters ( tQuery, m_tSchema ) )
			continue;

		float fCost = FLT_MAX;
		CSphVector<SecondaryIndexInfo_t> dEnabledIndexes = GetEnabledIndexes ( tQuery, bFulltext, fCost, iThreads );
		bAllFast &= IsQueryFast ( tQuery, dEnabledIndexes, fCost );

		// disable pseudo sharding if any of the queries use docid lookups
		if ( dEnabledIndexes.any_of ( []( const SecondaryIndexInfo_t & tSI ){ return tSI.m_eType==SecondaryIndexType_e::LOOKUP; } ) )
			return { 0, 1 };

		// enable pseudo sharding but limit number of threads when we use SI in fullscan
		if ( dEnabledIndexes.any_of ( []( const SecondaryIndexInfo_t & tSI ){ return tSI.m_eType==SecondaryIndexType_e::INDEX; } ) )
			iThreadCap = iThreadCap ? Min ( iThreadCap, iNumProc ) : iNumProc;
	}

	if ( bAllFast )
		return { 0, 1 };

	return  { GetStats().m_iTotalDocuments, iThreadCap };
}


int64_t	CSphIndex_VLN::GetCountDistinct ( const CSphString & sAttr, CSphString & sModifiedAttr ) const
{
	if ( m_tDeadRowMap.HasDead() )
		return -1;

	sModifiedAttr = sAttr;
	if ( !m_tSchema.GetAttr ( sAttr.cstr() ) && sphJsonNameSplit ( sAttr.cstr() ) )
		sModifiedAttr = UnifyJsonFieldName(sAttr);

	return m_tSI.GetCountDistinct(sModifiedAttr);
}


int64_t CSphIndex_VLN::GetCountFilter ( const CSphFilterSettings & tFilter, CSphString & sModifiedAttr ) const
{
	if ( m_tDeadRowMap.HasDead() )
		return -1;

	CSphFilterSettings tModifiedFilter = tFilter;

	CSphQuery tQuery;
	SelectIteratorCtx_t tCtx ( tQuery, tQuery.m_dFilters, m_tSchema, m_tSchema, m_pHistograms, m_pColumnar.get(), m_tSI, 0, m_iDocinfo, 1 );

	sModifiedAttr = tFilter.m_sAttrName;
	if ( !m_tSchema.GetAttr ( sModifiedAttr.cstr() ) && sphJsonNameSplit ( sModifiedAttr.cstr() ) )
	{
		tModifiedFilter.m_sAttrName = UnifyJsonFieldName(sModifiedAttr);
		sModifiedAttr = tModifiedFilter.m_sAttrName;
	}

	if ( !tCtx.IsEnabled_SI(tModifiedFilter) )
		return -1;

	common::Filter_t tColumnarFilter;
	CSphString sWarning;
	if ( !ToColumnarFilter ( tColumnarFilter, tModifiedFilter, SPH_COLLATION_DEFAULT, m_tSchema, sWarning ) )
		return -1;

	uint32_t uCount = 0;
	CSphString sError;
	if ( !m_tSI.CalcCount ( uCount, tColumnarFilter, m_iDocinfo, sError ) )
		return -1;

	return uCount;
}


int64_t CSphIndex_VLN::GetCount() const
{
	return m_iDocinfo - m_tDeadRowMap.GetNumDeads();
}

/////////////////////////////////////////////////////////////////////////////

struct CmpHit_fn
{
	inline static bool IsLess ( const CSphWordHit & a, const CSphWordHit & b )
	{
		return ( a.m_uWordID<b.m_uWordID ) ||
				( a.m_uWordID==b.m_uWordID && a.m_tRowID<b.m_tRowID ) ||
				( a.m_uWordID==b.m_uWordID && a.m_tRowID==b.m_tRowID && HITMAN::GetPosWithField ( a.m_uWordPos )<HITMAN::GetPosWithField ( b.m_uWordPos ) );
	}
};

void CSphIndex_VLN::GetIndexFiles ( StrVec_t& dFiles, StrVec_t& dExt, const FilenameBuilder_i* pParentFilenameBuilder ) const
{
	if ( !m_pDict )
		return;

	std::unique_ptr<FilenameBuilder_i> pFilenameBuilder { nullptr };
	if ( !pParentFilenameBuilder && GetIndexFilenameBuilder() )
	{
		pFilenameBuilder = GetIndexFilenameBuilder() ( GetName() );
		pParentFilenameBuilder = pFilenameBuilder.get();
	}
	GetSettingsFiles ( m_pTokenizer, m_pDict, GetSettings(), pParentFilenameBuilder, dExt );

	auto fnAddFile = [this,&dFiles] ( ESphExt eFile ) {
		auto sFile = GetFilename ( eFile );
		if ( sphIsReadable ( sFile.cstr() ) )
			dFiles.Add ( std::move ( sFile ) );
	};

	for ( auto eExt : { SPH_EXT_SPH, SPH_EXT_SPD, SPH_EXT_SPP, SPH_EXT_SPE, SPH_EXT_SPI, SPH_EXT_SPM, SPH_EXT_SPK } )
		fnAddFile ( eExt );

	if ( m_uVersion >= 55 )
		fnAddFile ( SPH_EXT_SPHI );

	if ( m_uVersion >= 57 && ( m_tSchema.HasStoredFields() || m_tSchema.HasStoredAttrs() ) )
		fnAddFile ( SPH_EXT_SPDS );

	if ( m_uVersion >= 65 && m_tSchema.HasKNNAttrs() )
		fnAddFile ( SPH_EXT_SPKNN );

	if ( m_bIsEmpty )
		return;

	fnAddFile ( SPH_EXT_SPT );

	if ( m_uVersion >= 64 )
		fnAddFile ( SPH_EXT_SPIDX );

	if ( m_uVersion >= 66 )
		fnAddFile ( SPH_EXT_SPJIDX );

	if ( m_tSchema.HasNonColumnarAttrs() )
		fnAddFile ( SPH_EXT_SPA );

	if ( m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
		fnAddFile ( SPH_EXT_SPB );

	if ( m_uVersion >= 63 && m_tSchema.HasColumnarAttrs() )
		fnAddFile ( SPH_EXT_SPC );
}

void GetSettingsFiles ( const TokenizerRefPtr_c& pTok, const DictRefPtr_c& pDict, const CSphIndexSettings & tSettings, const FilenameBuilder_i* pFilenameBuilder, StrVec_t & dFiles )
{
	assert ( pTok );
	assert ( pDict );

	StringBuilder_c sFiles ( "," );
	sFiles << pDict->GetSettings().m_sStopwords << pTok->GetSettings().m_sSynonymsFile << tSettings.m_sHitlessFiles;
	auto dFileNames = sphSplit ( sFiles.cstr(), " \t," );
	if ( pFilenameBuilder )
	{
		pDict->GetSettings().m_dWordforms.for_each ( [&] ( const auto& sFileName ) { dFiles.Add ( pFilenameBuilder->GetFullPath ( sFileName ) ); } );
		dFileNames.for_each ( [&] ( const auto& sFileName ) { dFiles.Add ( pFilenameBuilder->GetFullPath ( sFileName ) ); } );
	} else
	{
		pDict->GetSettings().m_dWordforms.for_each ( [&] ( const auto& sFileName ) { dFiles.Add ( sFileName ); } );
		dFileNames.for_each ( [&] ( const auto& sFileName ) { dFiles.Add ( sFileName ); } );
	}
}


class CSphHitBuilder
{
public:
	CSphHitBuilder ( const CSphIndexSettings & tSettings, const CSphVector<SphWordID_t> & dHitless, bool bMerging, int iBufSize, DictRefPtr_c pDict, CSphString * sError, StrVec_t * pCreatedFiles );

	bool	CreateIndexFiles ( const CSphString& sDocName, const CSphString& sHitName, const CSphString& sSkipName, bool bInplace, int iWriteBuffer, CSphAutofile & tHit, SphOffset_t * pSharedOffset=nullptr );
	void	HitReset ();

	void	cidxHit ( AggregateHit_t * pHit );
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

	AggregateHit_t				m_tLastHit;				///< hitlist entry
	Hitpos_t					m_iPrevHitPos {0};		///< previous hit position
	bool						m_bGotFieldEnd = false;
	std::array<BYTE, MAX_KEYWORD_BYTES> m_sLastKeyword;

	const CSphVector<SphWordID_t> &	m_dHitlessWords;
	DictRefPtr_c				m_pDict;
	CSphString *				m_pLastError;

	int							m_iSkiplistBlockSize = 0;
	SphOffset_t					m_iLastHitlistPos = 0;		///< doclist entry
	SphOffset_t					m_iLastHitlistDelta = 0;	///< doclist entry
	FieldMask_t					m_dLastDocFields;		///< doclist entry
	DWORD						m_uLastDocHits = 0;			///< doclist entry

	DictEntry_t					m_tWord;				///< dictionary entry

	ESphHitFormat				m_eHitFormat;
	ESphHitless					m_eHitless;

	CSphVector<SkiplistEntry_t>	m_dSkiplist;
	StrVec_t *					m_pCreatedFiles { nullptr };
#ifndef NDEBUG
	bool m_bMerging;
#endif
};


CSphHitBuilder::CSphHitBuilder ( const CSphIndexSettings & tSettings, const CSphVector<SphWordID_t> & dHitless, bool bMerging, int iBufSize, DictRefPtr_c pDict, CSphString * sError, StrVec_t * pCreatedFiles )
	: m_dWriteBuffer ( iBufSize )
	, m_dHitlessWords ( dHitless )
	, m_pDict ( std::move ( pDict ) )
	, m_pLastError ( sError )
	, m_iSkiplistBlockSize ( tSettings.m_iSkiplistBlockSize )
	, m_eHitFormat ( tSettings.m_eHitFormat )
	, m_eHitless ( tSettings.m_eHitless )
	, m_pCreatedFiles ( pCreatedFiles )
#ifndef NDEBUG
	, m_bMerging ( bMerging )
#endif
{
	m_sLastKeyword[0] = '\0';
	HitReset();
	m_dLastDocFields.UnsetAll();
	assert ( m_pDict );
	assert ( m_pLastError );

	m_pDict->SetSkiplistBlockSize ( m_iSkiplistBlockSize );
}


bool CSphHitBuilder::CreateIndexFiles ( const CSphString& sDocName, const CSphString& sHitName, const CSphString& sSkipName, bool bInplace, int iWriteBuffer, CSphAutofile & tHit, SphOffset_t * pSharedOffset )
{
	// doclist and hitlist files
	m_wrDoclist.CloseFile();
	m_wrHitlist.CloseFile();
	m_wrSkiplist.CloseFile();

	m_wrDoclist.SetBufferSize ( m_dWriteBuffer.GetLength() );
	m_wrHitlist.SetBufferSize ( bInplace ? iWriteBuffer : m_dWriteBuffer.GetLength() );

	if ( !m_wrDoclist.OpenFile ( sDocName, *m_pLastError ) )
		return false;
	if ( m_pCreatedFiles )
		m_pCreatedFiles->Add ( m_wrDoclist.GetFilename() );

	if ( bInplace )
	{
		sphSeek ( tHit.GetFD(), 0, SEEK_SET );
		m_wrHitlist.SetFile ( tHit, pSharedOffset, *m_pLastError );
	} else
	{
		if ( !m_wrHitlist.OpenFile ( sHitName, *m_pLastError ) )
			return false;
		if ( m_pCreatedFiles )
			m_pCreatedFiles->Add ( m_wrHitlist.GetFilename() );
	}

	if ( !m_wrSkiplist.OpenFile ( sSkipName, *m_pLastError ) )
		return false;
	if ( m_pCreatedFiles )
		m_pCreatedFiles->Add ( m_wrSkiplist.GetFilename() );

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
	m_tLastHit.m_szKeyword = m_sLastKeyword.data();
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

void CSphHitBuilder::cidxHit ( AggregateHit_t * pHit )
{
	assert (
		( pHit->m_uWordID!=0 && pHit->m_iWordPos!=EMPTY_HIT && pHit->m_tRowID!=INVALID_ROWID ) || // it's either ok hit
		( pHit->m_uWordID==0 && pHit->m_iWordPos==EMPTY_HIT ) ); // or "flush-hit"

	/////////////
	// next word
	/////////////

	const bool bNextWord = ( m_tLastHit.m_uWordID!=pHit->m_uWordID ||	( m_pDict->GetSettings().m_bWordDict && strcmpp ( (const char*)m_tLastHit.m_szKeyword, (const char*)pHit->m_szKeyword ) ) ); // OPTIMIZE?
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
			m_tWord.m_szKeyword = m_tLastHit.m_szKeyword;
			m_tWord.m_iDoclistLength = m_wrDoclist.GetPos() - m_tWord.m_iDoclistOffset;
			if ( m_tWord.m_iDocs )
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
				pHit->m_uWordID==m_tLastHit.m_uWordID && strcmp ( (const char*)pHit->m_szKeyword, (const char*)m_tLastHit.m_szKeyword )>0 )
			|| m_bMerging );
#endif // usually assert excluded in release, but this is 'paranoid' clause
		m_tWord.m_iDoclistOffset = m_wrDoclist.GetPos();
		m_tLastHit.m_uWordID = pHit->m_uWordID;
		if ( m_pDict->GetSettings().m_bWordDict )
		{
			assert ( strlen ( (const char *)pHit->m_szKeyword )<sizeof(m_sLastKeyword)-1 );
			strncpy ( (char*)const_cast<BYTE*>(m_tLastHit.m_szKeyword), (const char*)pHit->m_szKeyword, sizeof(m_sLastKeyword) ); // OPTIMIZE?
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


void ReadSchema ( CSphReader & rdInfo, CSphSchema & tSchema, DWORD uVersion )
{
	tSchema.Reset ();

	int iNumFields = rdInfo.GetDword();
	for ( int i=0; i<iNumFields; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaField ( rdInfo, tCol, uVersion );
		tSchema.AddField ( tCol );
	}

	int iNumAttrs = rdInfo.GetDword();
	for ( int i=0; i<iNumAttrs; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumn ( rdInfo, tCol, uVersion );
		tSchema.AddAttr ( tCol, false );
	}
}

static void ReadLocatorJson ( bson::Bson_c tNode, CSphAttrLocator & tLoc )
{
	using namespace bson;
	tLoc.m_iBitOffset = (int) Int ( tNode.ChildByName ( "pos" ) );
	tLoc.m_iBitCount = (int) Int ( tNode.ChildByName ( "bits" ) );
}

static void ReadSchemaColumnJson ( bson::Bson_c tNode, CSphColumnInfo & tCol )
{
	using namespace bson;
	tCol.m_sName = String ( tNode.ChildByName ( "name" ), "@emptyname" );
	tCol.m_sName.ToLower();
	tCol.m_uAttrFlags = (DWORD)Int ( tNode.ChildByName ( "flags" ), CSphColumnInfo::ATTR_NONE );
	tCol.m_bPayload = Bool ( tNode.ChildByName ( "payload" ), false );
	tCol.m_eEngine = (AttrEngine_e)Int ( tNode.ChildByName ( "engine" ), (DWORD)AttrEngine_e::DEFAULT );
	tCol.m_eAttrType = (ESphAttr)Int ( tNode.ChildByName ( "type" ) );
	ReadLocatorJson ( tNode.ChildByName ("locator"), tCol.m_tLocator );

	NodeHandle_t tKNN = tNode.ChildByName ("knn");
	if ( tKNN!=nullnode )
		tCol.m_tKNN = ReadKNNJson(tKNN);
}


static void ReadSchemaFieldJson ( bson::Bson_c tNode, CSphColumnInfo & tCol )
{
	using namespace bson;
	tCol.m_sName = String ( tNode.ChildByName ( "name" ) );
	tCol.m_uFieldFlags = (DWORD)Int ( tNode.ChildByName ( "flags" ), CSphColumnInfo::FIELD_INDEXED );
	tCol.m_bPayload = Bool ( tNode.ChildByName ( "payload" ) );
}


void ReadSchemaJson ( bson::Bson_c tNode, CSphSchema & tSchema )
{
	using namespace bson;
	tSchema.Reset ();

	Bson_c ( tNode.ChildByName ( "fields" ) ).ForEach ( [&tSchema] ( const NodeHandle_t& tNode )
	{
		CSphColumnInfo tCol;
		ReadSchemaFieldJson ( tNode, tCol );
		tSchema.AddField ( tCol );
	} );

	Bson_c ( tNode.ChildByName ( "attributes" ) ).ForEach ( [&tSchema] ( const NodeHandle_t& tNode )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumnJson ( tNode, tCol );
		tSchema.AddAttr ( tCol, false );
	} );
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

	if ( tCol.IsIndexedKNN() )
		tOut.NamedVal ( "knn", tCol.m_tKNN );
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


bool IndexBuildDone ( const BuildHeader_t & tBuildHeader, const WriteHeader_t & tWriteHeader, const CSphString & sFileName, CSphString & sError )
{
	JsonEscapedBuilder sJson;
	IndexWriteHeader ( tBuildHeader, tWriteHeader, sJson, false );

	CSphWriter wrHeaderJson;
	if ( wrHeaderJson.OpenFile ( sFileName, sError ) )
	{
		wrHeaderJson.PutString ( (Str_t)sJson );
		wrHeaderJson.CloseFile();
		assert ( bson::ValidateJson ( sJson.cstr(), &sError ) );
		return true;
	}

	sphWarning ( "failed to serialize header to json: %s", sError.cstr() );
	return false;
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
				pBuf += ZipToPtrLE ( pBuf, uHitCount );
				pBuf += ZipToPtrLE ( pBuf, uHitFieldMask );
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
		if ( d1 ) pBuf += ZipToPtrLE ( pBuf, 0 );
		if ( d2 && !bFlushed ) pBuf += ZipToPtrLE ( pBuf, 0 );

		assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );

		// encode deltas

		// encode keyword
		if ( d1 )
		{
			if ( m_pDict->GetSettings().m_bWordDict )
				pBuf += encodeKeyword ( pBuf, m_pDict->HitblockGetKeyword ( pHit->m_uWordID ) ); // keyword itself in case of keywords dict
			else
				pBuf += ZipToPtrLE ( pBuf, d1 ); // delta in case of CRC dict

			assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
		}

		// encode docid delta
		if ( d2 )
		{
			pBuf += ZipToPtrLE ( pBuf, d2 );
			assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
		}

		assert ( d3 );
		if ( !uHitCount ) // encode position delta, unless accumulating hits
		{
			pBuf += ZipToPtrLE ( pBuf, d3 << iPositionShift );
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
		pBuf += ZipToPtrLE ( pBuf, uHitCount );
		pBuf += ZipToPtrLE ( pBuf, uHitFieldMask );

		assert ( pBuf<m_dWriteBuffer.Begin() + m_dWriteBuffer.GetLength() );
	}

	pBuf += ZipToPtrLE ( pBuf, 0 );
	pBuf += ZipToPtrLE ( pBuf, 0 );
	pBuf += ZipToPtrLE ( pBuf, 0 );
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
inline bool SPH_CMPAGGRHIT_LESS ( const AggregateHit_t & a, const AggregateHit_t & b )
{
	if ( a.m_uWordID < b.m_uWordID )
		return true;

	if ( a.m_uWordID > b.m_uWordID )
		return false;

	if ( a.m_szKeyword )
	{
		int iCmp = strcmp ( (const char*)a.m_szKeyword, (const char*)b.m_szKeyword ); // OPTIMIZE?
		if ( iCmp!=0 )
			return ( iCmp<0 );
	}

	return
		( a.m_tRowID < b.m_tRowID ) ||
		( a.m_tRowID==b.m_tRowID && HITMAN::GetPosWithField ( a.m_iWordPos )<HITMAN::GetPosWithField ( b.m_iWordPos ) );
}


/// hit priority queue entry
struct CSphHitQueueEntry : public AggregateHit_t // fixme! used for build
{
	int m_iBin;
};


/// hit priority queue
struct CSphHitQueue // fixme! used for build
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
	void Push ( AggregateHit_t & tHit, int iBin )
	{
		// check for overflow and do add
		assert ( m_iUsed<m_iSize );
		auto & tEntry = m_pData[m_iUsed];

		tEntry.m_tRowID = tHit.m_tRowID;
		tEntry.m_uWordID = tHit.m_uWordID;
		tEntry.m_szKeyword = tHit.m_szKeyword; // bin must hold the actual data for the queue
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

static const int MIN_KEYWORDS_DICT	= 4*1048576;	// FIXME! ideally must be in sync with impl (ENTRY_CHUNKS, KEYWORD_CHUNKS)

/////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::RelocateBlock ( int iFile, BYTE * pBuffer, int iRelocationSize,
	SphOffset_t * pFileSize, CSphBin & dMinBin, SphOffset_t * pSharedOffset ) // build only
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


bool LoadHitlessWords ( const CSphString & sHitlessFiles, const TokenizerRefPtr_c& pTok, const DictRefPtr_c& pDict, CSphVector<SphWordID_t> & dHitlessWords, CSphString & sError )
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


bool CSphIndex_VLN::Build_CollectQueryMvas ( const CSphVector<CSphSource*> & dSources, QueryMvaContainer_c & tMvaContainer )
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
				pHash = new OpenHashTable_T<int64_t, CSphVector<int64_t>>;

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


bool CSphIndex_VLN::Build_CollectJoinedFields ( const CSphVector<CSphSource*> & dSources, CSphAutofile & tFile, CSphVector<std::unique_ptr<OpenHashTable_T<uint64_t, uint64_t>>> & dJoinedOffsets )
{
	for ( auto & pSource : dSources )
	{
		assert ( pSource );
		if ( !pSource->Connect ( m_sLastError ) )
			return false;

		if ( !pSource->FetchJoinedFields ( tFile, dJoinedOffsets, m_sLastError ) )
			return false;

		pSource->Disconnect();
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


bool CSphIndex_VLN::Build_StoreBlobAttrs ( DocID_t tDocId, std::pair<SphOffset_t,SphOffset_t> & tOffsetSize, BlobRowBuilder_i & tBlobRowBuilder, QueryMvaContainer_c & tMvaContainer, AttrSource_i & tSource, bool bForceSource )
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

	tOffsetSize = tBlobRowBuilder.Flush();
	return true;
}

template<typename T>
static void Builder_StoreAttrs ( const CSphSchema & tSchema, DocID_t tDocId, CSphSource & tSource, QueryMvaContainer_c & tMvaContainer, T * pBuilder, const CSphBitvec & tAttrsUsed )
{
	int iAttrId = 0;
	for ( int i=0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( !tAttrsUsed.BitGet(i) )
			continue;

		switch ( tAttr.m_eAttrType )
		{
		case SPH_ATTR_STRING:
			{
				const CSphString & sStrAttr = tSource.GetStrAttr(i);
				pBuilder->SetAttr ( iAttrId, (const BYTE*)sStrAttr.cstr(), sStrAttr.Length() );
			}
			break;

		case SPH_ATTR_UINT32SET:
		case SPH_ATTR_INT64SET:
			{
				const CSphVector<int64_t> * pMva = FetchMVA ( tDocId, i, tAttr, tMvaContainer, tSource, false );
				pBuilder->SetAttr ( iAttrId, pMva ? pMva->Begin() : nullptr, pMva ? pMva->GetLength() : 0 );
			}
			break;

		default:
			pBuilder->SetAttr ( iAttrId, tSource.GetAttr(i) );
			break;
		}

		iAttrId++;
	}
}


static void BuildStoreHistograms ( const CSphSchema & tSchema, DocID_t tDocId, CSphSource & tSource, QueryMvaContainer_c & tMvaContainer, CSphVector<HistogramSource_t> & dHistograms )
{
	for ( auto & tItem : dHistograms )
	{
		switch ( tItem.m_eAttrType )
		{
			case SPH_ATTR_STRING:
			{
				const CSphString & sVal = tSource.GetStrAttr ( tItem.m_iAttr );
				int iLen = sVal.Length();
				tItem.m_pHist->Insert ( iLen ? LibcCIHash_fn::Hash ( (const BYTE*)sVal.scstr(), iLen ) : 0 );
			}
			break;

			case SPH_ATTR_UINT32SET:
			case SPH_ATTR_INT64SET:
			{
				const CSphVector<int64_t> * pMva = FetchMVA ( tDocId, tItem.m_iAttr, tSchema.GetAttr ( tItem.m_iAttr ), tMvaContainer, tSource, false );
				if ( pMva )
				{
					for ( int64_t tVal : *pMva )
						tItem.m_pHist->Insert ( tVal );
				}
			}
			break;

			default:
				tItem.m_pHist->Insert ( tSource.GetAttr ( tItem.m_iAttr ) );
				break;
		}
	}
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

		m_pIndex = std::make_unique<CSphIndex_VLN> ( "keep-attrs", sKeepAttrs.cstr() );
		ResetFileAccess ( m_pIndex.get() );

		if ( !m_pIndex->Prealloc ( false, nullptr, dWarnings ) )
		{
			if ( !m_pIndex->GetLastError().IsEmpty() )
				sError.SetSprintf ( "%s error: '%s'", sError.scstr(), m_pIndex->GetLastError().cstr() );

			sphWarn ( "unable to load 'keep-attrs' table (%s); ignoring --keep-attrs", sError.cstr() );

			m_pIndex.reset();
		} else
		{
			// check schema
			if ( !tSchema.CompareTo ( m_pIndex->GetMatchSchema(), sError, false ) )
			{
				sphWarn ( "schemas are different (%s); ignoring --keep-attrs", sError.cstr() );
				m_pIndex.reset();
			}
		}

		for ( const auto & i : dWarnings )
			sphWarn ( "%s", i.cstr() );

		if ( m_pIndex )
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
						m_pIndex.reset();
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

		if ( m_pIndex )
			m_pIndex->Preread();

		return ( m_pIndex!=nullptr );
	}

	bool Keep ( DocID_t tDocid )
	{
		if ( !m_pIndex )
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
		if ( !m_pIndex || !m_pRow )
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
		if ( !m_pIndex || !m_pRow )
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
		if ( !m_pIndex || !m_pRow )
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
		m_pIndex.reset();
	}

private:
	std::unique_ptr<CSphIndex_VLN>	m_pIndex;
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
		const BYTE * pPool = m_pIndex->m_tBlobAttrs.GetReadPtr();
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


bool CSphIndex_VLN::SortDocidLookup ( int iFD, int nBlocks, int iMemoryLimit, int nLookupsInBlock, int nLookupsInLastBlock, CSphIndexProgress & tProgress ) // build only
{
	tProgress.PhaseBegin ( CSphIndexProgress::PHASE_LOOKUP );
	assert (!tProgress.m_iDocids);
	tProgress.m_iDocidsTotal = m_tStats.m_iTotalDocuments;

	if ( !nBlocks )
		return true;

	CSphWriter tfWriter;
	if ( !tfWriter.OpenFile ( GetFilename ( SPH_EXT_SPT ), m_sLastError ) )
		return false;

	DocidLookupWriter_c tWriter ( tfWriter, (DWORD)m_tStats.m_iTotalDocuments );
	tWriter.Start();

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

	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
		if ( tSchema.IsAttrStored(i) )
		{
			tFields.AddField ( tSchema.GetAttr(i).m_sName, DOCSTORE_ATTR );
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


bool CSphIndex_VLN::Build_SetupDocstore ( std::unique_ptr<DocstoreBuilder_i> & pDocstore, CSphBitvec & dStoredFields, CSphBitvec & dStoredAttrs, CSphVector<CSphVector<BYTE>> & dTmpDocstoreFieldStorage, CSphVector<CSphVector<BYTE>> & dTmpDocstoreAttrStorage )
{
	if ( !m_tSchema.HasStoredFields() && !m_tSchema.HasStoredAttrs() )
		return true;

	BuildBufferSettings_t tSettings; // use default buffer settings
	auto pBuilder = CreateDocstoreBuilder ( GetFilename ( SPH_EXT_SPDS ), GetSettings(), tSettings.m_iBufferStorage, m_sLastError );
	if ( !pBuilder )
		return false;

	SetupDocstoreFields ( *pBuilder, m_tSchema );

	dStoredFields.Init ( m_tSchema.GetFieldsCount() );
	dStoredAttrs.Init ( m_tSchema.GetAttrsCount() );

	for ( int i = 0; i < m_tSchema.GetFieldsCount(); i++ )
		if ( pBuilder->GetFieldId ( m_tSchema.GetFieldName(i), DOCSTORE_TEXT )!=-1 )
			dStoredFields.BitSet(i);

	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		if ( pBuilder->GetFieldId ( m_tSchema.GetAttr(i).m_sName, DOCSTORE_ATTR )!=-1 )
			dStoredAttrs.BitSet(i);

	dTmpDocstoreFieldStorage.Resize ( m_tSchema.GetFieldsCount() );
	dTmpDocstoreAttrStorage.Resize ( m_tSchema.GetAttrsCount() );

	pDocstore = std::move ( pBuilder );
	return true;
}


bool CSphIndex_VLN::Build_SetupBlobBuilder ( std::unique_ptr<BlobRowBuilder_i> & pBuilder )
{
	if ( !m_tSchema.HasBlobAttrs() )
		return true;

	BuildBufferSettings_t tSettings; // use default buffer settings
	pBuilder = sphCreateBlobRowBuilder ( m_tSchema, GetTmpFilename ( SPH_EXT_SPB ), m_tSettings.m_tBlobUpdateSpace, tSettings.m_iBufferAttributes, m_sLastError );
	return !!pBuilder;
}


bool CSphIndex_VLN::Build_SetupColumnar ( std::unique_ptr<columnar::Builder_i> & pBuilder, CSphBitvec & tColumnarsAttrs )
{
	if ( !m_tSchema.HasColumnarAttrs() )
		return true;

	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
		if ( m_tSchema.GetAttr(i).IsColumnar() )
			tColumnarsAttrs.BitSet(i);

	BuildBufferSettings_t tSettings; // use default buffer settings
	pBuilder = CreateColumnarBuilder ( m_tSchema, GetTmpFilename ( SPH_EXT_SPC ), tSettings.m_iBufferColumnar, m_sLastError );
	return !!pBuilder;
}


bool CSphIndex_VLN::Build_SetupSI ( std::unique_ptr<SI::Builder_i> & pSIBuilder, std::unique_ptr<JsonSIBuilder_i> & pJsonSIBuilder, CSphBitvec & tSIAttrs, int64_t iMemoryLimit )
{
	if ( !IsSecondaryLibLoaded() )
		return true;

	BuildBufferSettings_t tSettings; // use default buffer settings
	pSIBuilder = CreateIndexBuilder ( iMemoryLimit, m_tSchema, tSIAttrs, GetFilename ( SPH_EXT_SPIDX ), tSettings.m_iBufferStorage, m_sLastError );
	if ( !pSIBuilder )
		return false;

	if ( m_tSchema.HasJsonSIAttrs() )
	{
		pJsonSIBuilder = CreateJsonSIBuilder ( m_tSchema, GetFilename(SPH_EXT_SPB), GetFilename(SPH_EXT_SPJIDX), m_sLastError );
		if ( !pJsonSIBuilder )
			return false;
	}

	return true;
}


static bool BuildSetupHistograms ( const ISphSchema & tSchema, std::unique_ptr<HistogramContainer_c> & pContainer, CSphVector<HistogramSource_t> & dHistograms )
{
	const int MAX_HISTOGRAM_SIZE = 8192;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		Histogram_i * pHistogram = CreateHistogram ( tAttr.m_sName, tAttr.m_eAttrType, MAX_HISTOGRAM_SIZE ).release();
		if ( pHistogram )
			dHistograms.Add ( { pHistogram, i, tAttr.m_eAttrType } );
	}

	if ( !dHistograms.GetLength() )
		return true;

	pContainer = std::make_unique<HistogramContainer_c>();
	for ( const auto & i : dHistograms )
		Verify ( pContainer->Add ( std::unique_ptr<Histogram_i> { i.m_pHist } ) );

	return true;
}


static VecTraits_T<const BYTE> GetAttrForDocstore ( DocID_t tDocID, int iAttr, const CSphSchema & tSchema, QueryMvaContainer_c & tMvaContainer, CSphSource & tSource, CSphVector<BYTE> & dTmpStorage )
{
	const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_STRING:
	{
		const CSphString & sStrAttr = tSource.GetStrAttr(iAttr);
		return { (const BYTE*)sStrAttr.cstr(), sStrAttr.Length() };
	}

	case SPH_ATTR_UINT32SET:
	{
		const CSphVector<int64_t> * pMva = FetchMVA ( tDocID, iAttr, tAttr, tMvaContainer, tSource, false );
		dTmpStorage.Resize ( pMva->GetLength()*sizeof(DWORD) );
		DWORD * pAttrs = (DWORD*)dTmpStorage.Begin();
		for ( int iValue = 0; iValue < pMva->GetLength(); iValue++ )
			pAttrs[iValue] = (DWORD)(*pMva)[iValue];

		return dTmpStorage;
	}

	case SPH_ATTR_INT64SET:
	{
		const CSphVector<int64_t> * pMva = FetchMVA ( tDocID, iAttr, tAttr, tMvaContainer, tSource, false );
		return { pMva ? (const BYTE*)pMva->Begin() : nullptr, pMva ? (int64_t)pMva->GetLengthBytes() : 0 };
	}

	case SPH_ATTR_BIGINT:
	{
		int64_t iValue = tSource.GetAttr(iAttr);
		dTmpStorage.Resize ( sizeof(iValue) );
		memcpy ( dTmpStorage.Begin(), &iValue, dTmpStorage.GetLength() );
		return dTmpStorage;
	}

	default:
		// assume 32-bit integer
		uint32_t uValue = tSource.GetAttr(iAttr);
		dTmpStorage.Resize ( sizeof(uValue) );
		memcpy ( dTmpStorage.Begin(), &uValue, dTmpStorage.GetLength() );
		return dTmpStorage;
	}
}


static uint64_t CreateJoinedKey ( DocID_t tDocID, int iEntry )
{
	uint64_t uRes = sphFNV64 ( &tDocID, sizeof(tDocID) );
	return sphFNV64 ( &iEntry, sizeof(iEntry), uRes );
}


void CSphIndex_VLN::Build_AddToDocstore ( DocstoreBuilder_i * pDocstoreBuilder, DocID_t tDocID, QueryMvaContainer_c & tMvaContainer, CSphSource & tSource, const CSphBitvec & dStoredFields, const CSphBitvec & dStoredAttrs, CSphVector<CSphVector<BYTE>> & dTmpDocstoreFieldStorage, CSphVector<CSphVector<BYTE>> & dTmpDocstoreAttrStorage, const CSphVector<std::unique_ptr<OpenHashTable_T<uint64_t, uint64_t>>> & dJoinedOffsets, CSphReader & tJoinedReader )
{
	if ( !pDocstoreBuilder )
		return;

	DocstoreBuilder_i::Doc_t tDoc;
	tSource.GetDocFields ( tDoc.m_dFields );

	assert ( tDoc.m_dFields.GetLength()==m_tSchema.GetFieldsCount() );

	// filter out non-hl fields (should already be null)
	int iField = 0;
	for ( int i = 0; i < dStoredFields.GetSize(); i++ )
	{
		if ( !dStoredFields.BitGet(i) )
			tDoc.m_dFields.Remove(iField);
		else
		{
			// override with joined fields that were already prefetched
			if ( dJoinedOffsets.GetLength() && dJoinedOffsets[i] )
			{
				uint64_t * pOffset;
				int iEntry = 0;
				auto & dTmp = dTmpDocstoreFieldStorage[i];
				dTmp.Resize(0);
				while ( ( pOffset = dJoinedOffsets[i]->Find ( CreateJoinedKey ( tDocID, iEntry ) ) ) != nullptr )
				{
					tJoinedReader.SeekTo ( *pOffset, 0 );

					tJoinedReader.UnzipOffset();	// docid
					tJoinedReader.UnzipInt();		// joined field id
					if ( m_tSchema.GetField(i).m_bPayload )
						tJoinedReader.UnzipInt();	// payload

					DWORD uLength = tJoinedReader.UnzipInt();
					DWORD uOldFieldLength = dTmp.GetLength();
					DWORD uSpaceOffset = uOldFieldLength ? 1 : 0;
					DWORD uNewFieldLength = uOldFieldLength + uLength + uSpaceOffset;
					dTmp.Resize(uNewFieldLength);
					tJoinedReader.GetBytes ( &dTmp[uOldFieldLength+uSpaceOffset], uLength );
					if ( uSpaceOffset )
						dTmp[uOldFieldLength] = ' ';

					iEntry++;
				}

				tDoc.m_dFields[iField] = dTmp;
			}

			iField++;
		}
	}

	VecTraits_T<BYTE> * pAddedAttrs = tDoc.m_dFields.AddN ( dStoredAttrs.BitCount() );
	int iAttr = 0;
	for ( int i = 0; i < dStoredAttrs.GetSize(); i++ )
		if ( dStoredAttrs.BitGet(i) )
			pAddedAttrs[iAttr++] = GetAttrForDocstore ( tDocID, i, m_tSchema, tMvaContainer, tSource, dTmpDocstoreAttrStorage[i] );

	pDocstoreBuilder->AddDoc ( tSource.m_tDocInfo.m_tRowID, tDoc );
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


	bool bHaveJoined = pSource0->HasJoinedFields();

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
		if ( !Build_CollectQueryMvas ( dSources, tQueryMvaContainer ) )
			return 0;
	}

	CSphAutofile tTmpJoinedFields ( GetFilename ( "tmp3" ), SPH_O_NEW, m_sLastError, true );
	CSphVector<std::unique_ptr<OpenHashTable_T<uint64_t, uint64_t>>> dJoinedOffsets;
	CSphReader tJoinedReader;
	if ( bHaveJoined )
	{
		pSource0->Disconnect();
		if ( !Build_CollectJoinedFields ( dSources, tTmpJoinedFields, dJoinedOffsets ) )
			return 0;

		tJoinedReader.SetFile(tTmpJoinedFields);
	}

	int iFieldLens = m_tSchema.GetAttrId_FirstFieldLen();

	const CSphColumnInfo * pBlobLocatorAttr = m_tSchema.GetAttr ( sphGetBlobLocatorName() );

	if ( !m_pTokenizer->SetFilterSchema ( m_tSchema, m_sLastError ) )
		return 0;

	int iHitBuilderBufferSize = ( iWriteBuffer>0 )
		? Max ( iWriteBuffer, MIN_WRITE_BUFFER )
		: DEFAULT_WRITE_BUFFER;

	CSphHitBuilder tHitBuilder ( m_tSettings, dHitlessWords, false, iHitBuilderBufferSize, m_pDict, &m_sLastError, nullptr );

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
	CSphAutofile fdLock ( GetFilename ( "tmp0" ), SPH_O_NEW, m_sLastError, true );
	CSphAutofile fdHits ( ( m_bInplaceSettings ? GetFilename ( SPH_EXT_SPP ) : GetFilename ( "tmp1" ) ), SPH_O_NEW, m_sLastError, true );
	CSphAutofile fdTmpLookup ( GetFilename ( "tmp2" ), SPH_O_NEW, m_sLastError, true );

	CSphWriter tWriterSPA;
	bool bHaveNonColumnarAttrs = m_tSchema.HasNonColumnarAttrs();

	// write to temp file because of possible --keep-attrs option which loads prev index
	if ( bHaveNonColumnarAttrs && !tWriterSPA.OpenFile ( GetTmpFilename ( SPH_EXT_SPA ), m_sLastError ) )
		return 0;

	if ( fdLock.GetFD()<0 || fdHits.GetFD()<0 )
		return 0;

	std::unique_ptr<BlobRowBuilder_i> pBlobRowBuilder;
	if ( !Build_SetupBlobBuilder(pBlobRowBuilder) )
		return 0;

	std::unique_ptr<columnar::Builder_i> pColumnarBuilder;
	CSphBitvec tColumnarAttrs ( m_tSchema.GetAttrsCount() );
	if ( !Build_SetupColumnar ( pColumnarBuilder, tColumnarAttrs ) )
		return 0;

	std::unique_ptr<SI::Builder_i> pSIBuilder;
	std::unique_ptr<JsonSIBuilder_i> pJsonSIBuilder;
	CSphBitvec tSIAttrs ( m_tSchema.GetAttrsCount() );
	if ( !Build_SetupSI ( pSIBuilder, pJsonSIBuilder, tSIAttrs, iMemoryLimit ) )
		return 0;

	std::unique_ptr<DocstoreBuilder_i> pDocstoreBuilder;
	CSphBitvec dStoredFields, dStoredAttrs;
	CSphVector<CSphVector<BYTE>> dTmpDocstoreFieldStorage, dTmpDocstoreAttrStorage;
	if ( !Build_SetupDocstore ( pDocstoreBuilder, dStoredFields, dStoredAttrs, dTmpDocstoreFieldStorage, dTmpDocstoreAttrStorage ) )
		return 0;

	std::unique_ptr<HistogramContainer_c> pHistogramContainer;
	CSphVector<HistogramSource_t> dHistograms;
	if ( !BuildSetupHistograms ( m_tSchema, pHistogramContainer, dHistograms ) )
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
		bool bNeedToConnect = iSource>0 || bHaveQueryMVAs || bHaveJoined;
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
			if ( pBlobRowBuilder )
			{
				AttrSource_i * pBlobSource = pSource;
				if ( bKeepRow )
					pBlobSource = &tPrevAttrs;
				std::pair<SphOffset_t,SphOffset_t> tOffsetSize = {0,0};
				if ( !Build_StoreBlobAttrs ( tDocID, tOffsetSize, *pBlobRowBuilder, tQueryMvaContainer, *pBlobSource, bKeepRow ) )
					return 0;

				if ( pJsonSIBuilder )
					pJsonSIBuilder->AddRowSize ( tOffsetSize.second );

				pSource->m_tDocInfo.SetAttr ( pBlobLocatorAttr->m_tLocator, tOffsetSize.first );
			}

			// store anything columnar
			if ( pColumnarBuilder )
				Builder_StoreAttrs ( m_tSchema, tDocID, *pSource, tQueryMvaContainer, pColumnarBuilder.get(), tColumnarAttrs );

			if ( pSIBuilder )
			{
				pSIBuilder->SetRowID ( pSource->m_tDocInfo.m_tRowID );
				Builder_StoreAttrs ( m_tSchema, tDocID, *pSource, tQueryMvaContainer, pSIBuilder.get(), tSIAttrs );
			}

			BuildStoreHistograms ( m_tSchema, tDocID, *pSource, tQueryMvaContainer, dHistograms );

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

			Build_AddToDocstore ( pDocstoreBuilder.get(), tDocID, tQueryMvaContainer, *pSource, dStoredFields, dStoredAttrs, dTmpDocstoreFieldStorage, dTmpDocstoreAttrStorage, dJoinedOffsets, tJoinedReader );

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
		if ( bHaveJoined )
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

			tJoinedReader.SeekTo(0,0);

			while (true)
			{
				// get next doc, and handle errors
				ISphHits * pJoinedHits = pSource->IterateJoinedHits ( tJoinedReader, m_sLastError );
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
		m_sLastError.SetSprintf ( "table over %d documents not supported (got documents count=" INT64_FMT ")", INT_MAX, m_tStats.m_iTotalDocuments );
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

	if ( pBlobRowBuilder && !pBlobRowBuilder->Done(m_sLastError) )
		return 0;

	std::string sError;
	if ( pColumnarBuilder && !pColumnarBuilder->Done(sError) )
	{
		m_sLastError = sError.c_str();
		return 0;
	}

	if ( pSIBuilder )
	{
		tProgress.PhaseBegin ( CSphIndexProgress::PHASE_SI_BUILD );
		tProgress.Show();
		bool bSiDone = pSIBuilder->Done(sError);
		tProgress.PhaseEnd();

		if ( !bSiDone )
		{
			m_sLastError = sError.c_str();
			return 0;
		}
	}

	if ( pHistogramContainer && !pHistogramContainer->Save ( GetFilename ( SPH_EXT_SPHI ), m_sLastError ) )
		return 0;

	if ( bGotPrevIndex )
		tPrevAttrs.Reset();

	CSphString sSPA = GetFilename ( SPH_EXT_SPA );
	CSphString sSPATmp = GetTmpFilename ( SPH_EXT_SPA );
	if ( bHaveNonColumnarAttrs && sph::rename ( sSPATmp.cstr(), sSPA.cstr() )!=0 )
	{
		m_sLastError.SetSprintf ( "failed to rename %s to %s", sSPATmp.cstr(), sSPA.cstr() );
		return false;
	}

	CSphString sSPB = GetFilename ( SPH_EXT_SPB );
	CSphString sSPBTmp = GetTmpFilename ( SPH_EXT_SPB );
	if ( m_tSchema.HasBlobAttrs() && sph::rename ( sSPBTmp.cstr(), sSPB.cstr() )!=0 )
	{
		m_sLastError.SetSprintf ( "failed to rename %s to %s", sSPBTmp.cstr(), sSPB.cstr() );
		return false;
	}

	CSphString sSPC = GetFilename ( SPH_EXT_SPC );
	CSphString sSPCTmp = GetTmpFilename ( SPH_EXT_SPC );
	if ( m_tSchema.HasColumnarAttrs() && sph::rename ( sSPCTmp.cstr(), sSPC.cstr() )!=0 )
	{
		m_sLastError.SetSprintf ( "failed to rename %s to %s", sSPCTmp.cstr(), sSPC.cstr() );
		return false;
	}

	if ( pJsonSIBuilder )
	{
		tProgress.PhaseBegin ( CSphIndexProgress::PHASE_JSONSI_BUILD );
		tProgress.Show();
		bool bSiDone = pJsonSIBuilder->Done(m_sLastError);
		tProgress.PhaseEnd();

		if ( !bSiDone )
			return 0;
	}

	if ( !WriteDeadRowMap ( GetFilename ( SPH_EXT_SPM ), (DWORD)m_tStats.m_iTotalDocuments, m_sLastError ) )
		return 0;

	dKillList.Uniq();
	if ( !WriteKillList ( GetFilename ( SPH_EXT_SPK ), dKillList.Begin(), dKillList.GetLength(), m_tSettings.m_tKlistTargets, m_sLastError ) )
		return 0;

	if ( pDocstoreBuilder )
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

	tHitBuilder.CreateIndexFiles ( GetFilename ( SPH_EXT_SPD ), GetFilename ( SPH_EXT_SPP ), GetFilename ( SPH_EXT_SPE ), m_bInplaceSettings, iWriteBuffer, fdHits, &iSharedOffset );

	// dict files
	CSphAutofile fdTmpDict ( GetFilename ( "tmp8" ), SPH_O_NEW, m_sLastError, true );
	CSphAutofile fdDict ( GetFilename ( SPH_EXT_SPI ), SPH_O_NEW, m_sLastError, false );
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
		AggregateHit_t tHit;

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

		AggregateHit_t tFlush;
		tFlush.m_tRowID = INVALID_ROWID;
		tFlush.m_uWordID = 0;
		tFlush.m_szKeyword = nullptr;
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

	BuildHeader_t tBuildHeader;
	WriteHeader_t tWriteHeader;
	PrepareHeaders ( tBuildHeader, tWriteHeader, false );
	tBuildHeader.m_iDocinfo = m_tStats.m_iTotalDocuments;

	if ( !tHitBuilder.cidxDone ( iMemoryLimit, m_tSettings.m_iMinInfixLen, m_pTokenizer->GetMaxCodepointLength(), &tBuildHeader ) )
		return 0;

	dRelocationBuffer.Reset(0);

	// we're done
	if ( !IndexBuildDone ( tBuildHeader, tWriteHeader, GetFilename(SPH_EXT_SPH), m_sLastError ) )
		return 0;

	// when the party's over..
	ARRAY_FOREACH ( i, dSources )
		dSources[i]->PostIndex ();

	if ( m_bInplaceSettings )
		fdHits.SetPersistent();
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


std::unique_ptr<ISphFilter> CSphIndex_VLN::CreateMergeFilters ( const VecTraits_T<CSphFilterSettings> & dSettings ) const
{
	CSphString sError, sWarning;
	std::unique_ptr<ISphFilter> pResult;
	CreateFilterContext_t tCtx;
	tCtx.m_pMatchSchema = &m_tSchema;
	tCtx.m_pIndexSchema = &m_tSchema;
	tCtx.m_pBlobPool = m_tBlobAttrs.GetReadPtr();

	for ( const auto& dSetting : dSettings )
		pResult = sphJoinFilters ( std::move ( pResult ), sphCreateFilter ( dSetting, tCtx, sError, sWarning ) );

	if ( pResult )
		pResult->SetColumnar ( m_pColumnar.get() );

	return pResult;
}

bool CheckDocsCount ( int64_t iDocs, CSphString & sError )
{
	if ( iDocs<INT_MAX )
		return true;

	sError.SetSprintf ( "table over %d documents not supported (got " INT64_FMT " documents)", INT_MAX, iDocs );
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
	inline void TransferData ( QWORD & tQword, SphWordID_t iWordID, const BYTE * szWord,
							const CSphIndex_VLN * pSourceIndex, const VecTraits_T<RowID_t>& dRows,
							MergeCb_c & tMonitor )
	{
		AggregateHit_t tHit;
		tHit.m_uWordID = iWordID;
		tHit.m_szKeyword = szWord;
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
	inline void TransferHits ( QWORD & tQword, AggregateHit_t & tHit, const VecTraits_T<RowID_t> & dRows )
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

// QWORDDST, QWORDSRC = DiskIndexQword_c
template < typename QWORDDST, typename QWORDSRC >
bool CSphIndex_VLN::MergeWords ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, VecTraits_T<RowID_t> dDstRows, VecTraits_T<RowID_t> dSrcRows, CSphHitBuilder * pHitBuilder, CSphString & sError, CSphIndexProgress & tProgress )
{
	auto& tMonitor = tProgress.GetMergeCb();
	CSphAutofile tDummy;
	pHitBuilder->CreateIndexFiles ( pDstIndex->GetTmpFilename ( SPH_EXT_SPD ), pDstIndex->GetTmpFilename ( SPH_EXT_SPP ), pDstIndex->GetTmpFilename ( SPH_EXT_SPE ), false, 0, tDummy );

	static_assert ( QWORDDST::is_worddict::value == QWORDDST::is_worddict::value, "can't merge worddict with non-worddict" );

	CSphDictReader<QWORDDST::is_worddict::value> tDstReader ( pDstIndex->GetSettings().m_iSkiplistBlockSize );
	CSphDictReader<QWORDSRC::is_worddict::value> tSrcReader ( pSrcIndex->GetSettings().m_iSkiplistBlockSize );

	/// compress means: I don't want true merge, I just want to apply deadrows and filter
	bool bCompress = pDstIndex==pSrcIndex;

	if ( !tDstReader.Setup ( pDstIndex->GetFilename ( SPH_EXT_SPI ), pDstIndex->m_tWordlist.GetWordsEnd(), pDstIndex->m_tSettings.m_eHitless, sError ) )
		return false;
	if ( !bCompress && !tSrcReader.Setup ( pSrcIndex->GetFilename ( SPH_EXT_SPI ), pSrcIndex->m_tWordlist.GetWordsEnd(), pSrcIndex->m_tSettings.m_eHitless, sError ) )
		return false;

	/// prepare for indexing
	pHitBuilder->HitblockBegin();
	pHitBuilder->HitReset();

	/// setup qwords

	QWORDDST tDstQword ( false, false, pDstIndex->GetIndexId() );
	QWORDSRC tSrcQword ( false, false, pSrcIndex->GetIndexId() );

	DataReaderFactoryPtr_c tSrcDocs {
		NewProxyReader ( pSrcIndex->GetFilename ( SPH_EXT_SPD ), sError,
			DataReaderFactory_c::DOCS, pSrcIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE )
	};
	if ( !tSrcDocs )
		return false;

	DataReaderFactoryPtr_c tSrcHits {
		NewProxyReader ( pSrcIndex->GetFilename ( SPH_EXT_SPP ), sError,
			DataReaderFactory_c::HITS, pSrcIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE  )
	};
	if ( !tSrcHits )
		return false;

	if ( !sError.IsEmpty () || tMonitor.NeedStop () )
		return false;

	DataReaderFactoryPtr_c tDstDocs {
		NewProxyReader ( pDstIndex->GetFilename ( SPH_EXT_SPD ), sError,
			DataReaderFactory_c::DOCS, pDstIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE )
	};
	if ( !tDstDocs )
		return false;

	DataReaderFactoryPtr_c tDstHits {
		NewProxyReader ( pDstIndex->GetFilename ( SPH_EXT_SPP ), sError,
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

		if ( tMonitor.NeedStop () || !sError.IsEmpty () )
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

			AggregateHit_t tHit;
			tHit.m_uWordID = tDstReader.m_uWordID; // !COMMIT m_sKeyword anyone?
			tHit.m_szKeyword = tDstReader.GetWord();
			tHit.m_dFieldMask.UnsetAll();

			// we assume that all the duplicates have been removed
			// and we don't need to merge hits from the same document

			// transfer hits from destination
			while ( QwordIteration::NextDocument ( tDstQword, pDstIndex, dDstRows ) )
			{
				if ( tMonitor.NeedStop () || !sError.IsEmpty () )
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
				if ( tMonitor.NeedStop () || !sError.IsEmpty () )
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

// called only from indexer
bool CSphIndex_VLN::Merge ( CSphIndex * pSource, const VecTraits_T<CSphFilterSettings> & dFilters, bool bSupressDstDocids, CSphIndexProgress& tProgress )
{
	StrVec_t dWarnings;

	ResetFileAccess ( this );
	ResetFileAccess ( pSource );

	if ( !Prealloc ( false, nullptr, dWarnings ) )
		return false;
	Preread();

	if ( !pSource->Prealloc ( false, nullptr, dWarnings ) )
	{
		m_sLastError.SetSprintf ( "source table preload failed: %s", pSource->GetLastError().cstr() );
		return false;
	}
	pSource->Preread();

	for ( const auto & i : dWarnings )
		sphWarn ( "%s", i.cstr() );

	// create filters
	std::unique_ptr<ISphFilter> pFilter =CreateMergeFilters ( dFilters );
	return DoMerge ( this, (const CSphIndex_VLN *)pSource, pFilter.get(), m_sLastError, tProgress, false, bSupressDstDocids );
}

std::pair<DWORD,DWORD> CSphIndex_VLN::CreateRowMapsAndCountTotalDocs ( const CSphIndex_VLN* pSrcIndex, const CSphIndex_VLN* pDstIndex, CSphFixedVector<RowID_t>& dSrcRowMap, CSphFixedVector<RowID_t>& dDstRowMap, const ISphFilter* pFilter, bool bSupressDstDocids, MergeCb_c& tMonitor )
{
	TRACE_CORO ( "sph", "CSphIndex_VLN::CreateRowMapsAndCountTotalDocs" );
	if ( pSrcIndex!=pDstIndex )
		dSrcRowMap.Reset ( pSrcIndex->m_iDocinfo );
	dDstRowMap.Reset ( pDstIndex->m_iDocinfo );

	int iStride = pDstIndex->m_tSchema.GetRowSize();

	DeadRowMap_Ram_c tExtraDeadMap(0);
	if ( bSupressDstDocids ) // skip monitoring supressions as they're used _only_ from indexer call and never from flavours of optimize.
	{
		tExtraDeadMap.Reset ( dDstRowMap.GetLength() );

		LookupReaderIterator_c tDstLookupReader ( pDstIndex->m_tDocidLookup.GetReadPtr() );
		LookupReaderIterator_c tSrcLookupReader ( pSrcIndex->m_tDocidLookup.GetReadPtr() );

		KillByLookup ( tDstLookupReader, tSrcLookupReader, tExtraDeadMap );
	}

	dSrcRowMap.Fill ( INVALID_ROWID );
	dDstRowMap.Fill ( INVALID_ROWID );

	const DWORD * pRow = pDstIndex->m_tAttr.GetReadPtr();
	int64_t iTotalDocs = 0;
	std::pair<DWORD, DWORD> tPerIndexDocs {0,0};

	// say to observer we're going to collect alive rows from dst index
	// (kills directed to that index must be collected to reapply at the finish)
	BEGIN_CORO ( "sph", "collect dst rowmap");
	tMonitor.SetEvent ( MergeCb_c::E_COLLECT_START, pDstIndex->m_iChunk );
	for ( RowID_t i = 0; i < dDstRowMap.GetLength(); ++i, pRow+=iStride )
	{
		if ( pDstIndex->m_tDeadRowMap.IsSet(i) )
			continue;

		if ( bSupressDstDocids && tExtraDeadMap.IsSet ( i ) )
			continue;

		if ( pFilter && !pFilter->Eval ( { i, pRow } ) )
			continue;

		dDstRowMap[i] = (RowID_t)iTotalDocs++;
	}
	tMonitor.SetEvent ( MergeCb_c::E_COLLECT_FINISHED, pDstIndex->m_iChunk );
	END_CORO ( "sph" );
	tPerIndexDocs.first = (DWORD)iTotalDocs;
	if ( dSrcRowMap.IsEmpty() )
		return tPerIndexDocs;

	// say to observer we're going to collect alive rows from src index (again, issue to kills).
	BEGIN_CORO ( "sph", "collect src rowmap" );
	tMonitor.SetEvent ( MergeCb_c::E_COLLECT_START, pSrcIndex->m_iChunk );
	for ( int i = 0; i < dSrcRowMap.GetLength(); ++i )
	{
		if ( pSrcIndex->m_tDeadRowMap.IsSet(i) )
			continue;

		dSrcRowMap[i] = (RowID_t)iTotalDocs++;
	}
	tMonitor.SetEvent ( MergeCb_c::E_COLLECT_FINISHED, pSrcIndex->m_iChunk );
	tPerIndexDocs.second = DWORD ( iTotalDocs - tPerIndexDocs.first );
	END_CORO ( "sph" );
	return tPerIndexDocs;
}


bool CSphIndex_VLN::DoMerge ( const CSphIndex_VLN * pDstIndex, const CSphIndex_VLN * pSrcIndex, const ISphFilter * pFilter, CSphString & sError, CSphIndexProgress & tProgress,	bool bSrcSettings, bool bSupressDstDocids )
{
	TRACE_CORO ( "sph", "CSphIndex_VLN::DoMerge" );
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
		sError = "hitless settings must be the same on merged tables";
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

	// merging attributes is separate complete stage; files are finally prepared after it.
	// however, if interrupt is requested after that stage - we need list of the files
	// to gracefully unlink them.
	StrVec_t dDeleteOnInterrupt;
	// unlink prepared attribute files on exit, if any
	AT_SCOPE_EXIT ( [&dDeleteOnInterrupt]
	{
		dDeleteOnInterrupt.for_each ( [] ( const auto & sFile )
		{
			if ( !sFile.IsEmpty() && sphFileExists ( sFile.cstr() ) )
				::unlink ( sFile.cstr() );
		} ); 
	});

	// merging attributes
	{
		AttrMerger_c tAttrMerger { tMonitor, sError, iTotalDocs, g_tMergeSettings };
		if ( !tAttrMerger.Prepare ( pSrcIndex, pDstIndex ) )
			return false;

		if ( !tAttrMerger.CopyAttributes ( *pDstIndex, dDstRows, tTotalDocs.first ) )
			return false;

		if ( !bCompress && !tAttrMerger.CopyAttributes ( *pSrcIndex, dSrcRows, tTotalDocs.second ) )
			return false;

		if ( !tAttrMerger.FinishMergeAttributes ( pDstIndex, tBuildHeader, &dDeleteOnInterrupt ) )
			return false;
	}

	const CSphIndex_VLN* pSettings = ( bSrcSettings ? pSrcIndex : pDstIndex );
	CSphAutofile tDict ( pDstIndex->GetTmpFilename ( SPH_EXT_SPI ), SPH_O_NEW, sError, true );

	if ( !sError.IsEmpty() || tDict.GetFD()<0 || tMonitor.NeedStop() )
		return false;

	DictRefPtr_c pDict { pSettings->m_pDict->Clone() };

	CSphVector<SphWordID_t> dDummy;
	CSphHitBuilder tHitBuilder ( pSettings->m_tSettings, dDummy, true, g_tMergeSettings.m_iBufferDict, pDict, &sError, &dDeleteOnInterrupt );

	int iInfixCodepointBytes = 0;
	if ( pSettings->m_tSettings.m_iMinInfixLen > 0 && pDict->GetSettings().m_bWordDict )
		iInfixCodepointBytes = pSettings->m_pTokenizer->GetMaxCodepointLength();

	// FIXME? is this magic dict block constant any good?..
	pDict->SortedDictBegin ( tDict, g_tMergeSettings.m_iBufferDict, iInfixCodepointBytes );

	BEGIN_CORO ( "sph", "merge dicts, doclists and hitlists" );
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

	END_CORO ( "sph" );

	if ( tMonitor.NeedStop () || !sError.IsEmpty() )
		return false;

	// finalize
	AggregateHit_t tFlush;
	tFlush.m_tRowID = INVALID_ROWID;
	tFlush.m_uWordID = 0;
	tFlush.m_szKeyword = (const BYTE*)""; // tricky: assertion in cidxHit calls strcmp on this in case of empty index!
	tFlush.m_iWordPos = EMPTY_HIT;
	tFlush.m_dFieldMask.UnsetAll();
	tHitBuilder.cidxHit ( &tFlush );

	int iMinInfixLen = pSettings->m_tSettings.m_iMinInfixLen;
	if ( !tHitBuilder.cidxDone ( g_tMergeSettings.m_iBufferDict, iMinInfixLen, pSettings->m_pTokenizer->GetMaxCodepointLength(), &tBuildHeader ) )
		return false;

	WriteHeader_t tWriteHeader;
	tWriteHeader.m_pSettings = &pSettings->m_tSettings;
	tWriteHeader.m_pSchema = &pSettings->m_tSchema;
	tWriteHeader.m_pTokenizer = pSettings->m_pTokenizer;
	tWriteHeader.m_pDict = pSettings->m_pDict;
	tWriteHeader.m_pFieldFilter = pSettings->m_pFieldFilter.get();
	tWriteHeader.m_pFieldLens = pSettings->m_dFieldLens.Begin();

	IndexBuildDone ( tBuildHeader, tWriteHeader, pDstIndex->GetTmpFilename ( SPH_EXT_SPH ), sError );

	// we're done; clean all deferred deletes
	tDict.SetPersistent();
	dDeleteOnInterrupt.Reset();

	return true;
}


bool sphMerge ( const CSphIndex * pDst, const CSphIndex * pSrc, VecTraits_T<CSphFilterSettings> dFilters, CSphIndexProgress & tProgress, CSphString& sError )
{
	TRACE_CORO ( "sph", "sphMerge" );
	auto pDstIndex = (const CSphIndex_VLN*) pDst;
	auto pSrcIndex = (const CSphIndex_VLN*) pSrc;

	std::unique_ptr<ISphFilter> pFilter = pDstIndex->CreateMergeFilters ( dFilters );
	return CSphIndex_VLN::DoMerge ( pDstIndex, pSrcIndex, pFilter.get(), sError, tProgress, dFilters.IsEmpty(), false );
}

template < typename QWORD >
bool CSphIndex_VLN::DeleteField ( const CSphIndex_VLN * pIndex, CSphHitBuilder * pHitBuilder, CSphString & sError, CSphSourceStats & tStat, int iKillField )
{
	assert ( iKillField>=0 );

	CSphAutofile tDummy;
	pHitBuilder->CreateIndexFiles ( pIndex->GetTmpFilename ( SPH_EXT_SPD ), pIndex->GetTmpFilename ( SPH_EXT_SPP ), pIndex->GetTmpFilename ( SPH_EXT_SPE ), false, 0, tDummy );

	CSphDictReader<QWORD::is_worddict::value> tWordsReader ( pIndex->GetSettings().m_iSkiplistBlockSize );
	if ( !tWordsReader.Setup ( pIndex->GetFilename ( SPH_EXT_SPI ), pIndex->m_tWordlist.GetWordsEnd(), pIndex->m_tSettings.m_eHitless, sError ) )
		return false;

	/// prepare for indexing
	pHitBuilder->HitblockBegin();
	pHitBuilder->HitReset();

	/// setup qword
	QWORD tQword ( false, false, pIndex->GetIndexId() );
	DataReaderFactoryPtr_c tDocs {
		NewProxyReader ( pIndex->GetFilename ( SPH_EXT_SPD ), sError,
			DataReaderFactory_c::DOCS, pIndex->m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE )
	};
	if ( !tDocs )
		return false;

	DataReaderFactoryPtr_c tHits {
		NewProxyReader ( pIndex->GetFilename ( SPH_EXT_SPP ), sError,
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

		AggregateHit_t tHit;
		tHit.m_uWordID = tWordsReader.m_uWordID; // !COMMIT m_sKeyword anyone?
		tHit.m_szKeyword = tWordsReader.GetWord();
		tHit.m_dFieldMask.UnsetAll();

		// transfer hits
		while ( QwordIteration::NextDocument ( tQword, pIndex ) )
		{
			if ( sphInterrupted () )
				return false;

			tHit.m_tRowID = tQword.m_tDoc.m_tRowID;
			if ( bHitless )
			{
				tHit.m_dFieldMask = tQword.m_dQwordFields; // fixme! what field mask on hitless? m.b. write 0 here?
				tHit.m_dFieldMask.DeleteBit (iKillField);
				if ( tHit.m_dFieldMask.TestAll ( false ) )
					continue;
				tHit.SetAggrCount ( tQword.m_uMatchHits );
				pHitBuilder->cidxHit ( &tHit );
			} else
			{
				assert ( tQword.m_bHasHitlist );
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
	CSphAutofile tNewDict ( GetTmpFilename ( SPH_EXT_SPI ), SPH_O_NEW, sError );

	if ( !sError.IsEmpty () || tNewDict.GetFD ()<0 || sphInterrupted () )
		return false;

	DictRefPtr_c pDict { m_pDict->Clone () };

	int iHitBufferSize = 8 * 1024 * 1024;
	CSphVector<SphWordID_t> dDummy;
	CSphHitBuilder tHitBuilder ( m_tSettings, dDummy, true, iHitBufferSize, pDict, &sError, nullptr );

	// FIXME? is this magic dict block constant any good?..

	int iInfixCodepointBytes = 0;
	if ( m_tSettings.m_iMinInfixLen > 0 && pDict->GetSettings().m_bWordDict )
		iInfixCodepointBytes = m_pTokenizer->GetMaxCodepointLength();

	pDict->SortedDictBegin ( tNewDict, iHitBufferSize, iInfixCodepointBytes );

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
	AggregateHit_t tFlush;
	tFlush.m_tRowID = INVALID_ROWID;
	tFlush.m_uWordID = 0;
	tFlush.m_szKeyword = (const BYTE*)""; // tricky: assertion in cidxHit calls strcmp on this in case of empty index!
	tFlush.m_iWordPos = EMPTY_HIT;
	tFlush.m_dFieldMask.UnsetAll();
	tHitBuilder.cidxHit ( &tFlush );

	int iMinInfixLen = m_tSettings.m_iMinInfixLen;
	if ( !tHitBuilder.cidxDone ( iHitBufferSize, iMinInfixLen, m_pTokenizer->GetMaxCodepointLength(), &tBuildHeader ) )
		return false;

	/// as index is w-locked, we can also detach doclist/hitlist/dictionary and juggle them.
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


bool CSphIndex_VLN::AddRemoveFromDocstore ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, CSphString & sError )
{
	int iOldNumStored = 0;
	for ( int i = 0; i < tOldSchema.GetFieldsCount(); i++ )
		if ( tOldSchema.IsFieldStored(i) )
			iOldNumStored++;

	for ( int i = 0; i < tOldSchema.GetAttrsCount(); i++ )
		if ( tOldSchema.IsAttrStored(i) )
			iOldNumStored++;

	int iNewNumStored = 0;
	for ( int i = 0; i < tNewSchema.GetFieldsCount(); i++ )
		if ( tNewSchema.IsFieldStored(i) )
			iNewNumStored++;

	for ( int i = 0; i < tNewSchema.GetAttrsCount(); i++ )
		if ( tNewSchema.IsAttrStored(i) )
			iOldNumStored++;

	if ( iOldNumStored==iNewNumStored )
		return true;

	std::unique_ptr<DocstoreBuilder_i> pDocstoreBuilder;
	if ( iNewNumStored )
	{
		BuildBufferSettings_t tSettings; // use default buffer settings
		pDocstoreBuilder = CreateDocstoreBuilder ( GetTmpFilename ( SPH_EXT_SPDS ), m_pDocstore->GetDocstoreSettings(), tSettings.m_iBufferStorage, sError );
		if ( !pDocstoreBuilder )
			return false;

		Alter_AddRemoveFromDocstore ( *pDocstoreBuilder, m_pDocstore.get(), (DWORD)m_iDocinfo, tNewSchema );
	}

	if ( !JuggleFile ( SPH_EXT_SPDS, sError, !!iOldNumStored, !!iNewNumStored ) )
		return false;

	m_pDocstore.reset();
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

	BuildHeader_t tBuildHeader;
	WriteHeader_t tWriteHeader;
	PrepareHeaders ( tBuildHeader, tWriteHeader );

	if ( !bAddField && !DeleteFieldFromDict ( iRemoveIdx, tBuildHeader, sError ) )
		return false;

	if ( !AddRemoveFromDocstore ( tOldSchema, tNewSchema, sError ) )
		return false;

	tWriteHeader.m_pSchema = &tNewSchema;

	// save the header
	if ( !IndexBuildDone ( tBuildHeader, tWriteHeader, GetTmpFilename ( SPH_EXT_SPH ), sError ) )
		return false;

	return JuggleFile ( SPH_EXT_SPH, sError );
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
	TlsMsg::ResetErr(); // clean err
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

	const CSphRowitem * pRow = m_tAttr.GetReadPtr();
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
		m_uAttrsStatus |= IndexSegment_c::ATTRS_ROWMAP_UPDATED;
		KillHook ( tDocID );
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
	return m_tAttr.GetReadPtr() + (int64_t)tRowID*m_tSchema.GetCachedRowSize();
}


inline RowID_t CSphIndex_VLN::GetRowIDByDocinfo ( const CSphRowitem * pDocinfo ) const
{
	return RowID_t ( ( pDocinfo - m_tAttr.GetReadPtr() ) / m_tSchema.GetCachedRowSize() );
}

template<bool USE_KLIST, bool RANDOMIZE, bool USE_FACTORS, bool HAS_SORT_CALC, bool HAS_WEIGHT_FILTER, bool HAS_FILTER_CALC, bool HAS_CUTOFF>
void CSphIndex_VLN::MatchExtended ( CSphQueryContext& tCtx, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, ISphRanker * pRanker, int iTag, int iIndexWeight, int iCutoff ) const
{
	if ( !iCutoff )
		return;

	QueryProfile_c * pProfile = tCtx.m_pProfile;
	CSphScopedProfile tProf (pProfile, SPH_QSTATE_UNKNOWN);

	if constexpr ( USE_FACTORS )
		pRanker->ExtraData ( EXTRA_SET_MATCHTAG, (void**)&iTag );

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
			if constexpr ( USE_KLIST )
			{
				if ( m_tDeadRowMap.IsSet ( tMatch.m_tRowID ) )
					continue;
			}

			tMatch.m_iWeight *= iIndexWeight;

			if constexpr ( HAS_SORT_CALC )
				tCtx.CalcSort ( tMatch );

			if constexpr ( HAS_WEIGHT_FILTER )
			{
				if ( tCtx.m_pWeightFilter && !tCtx.m_pWeightFilter->Eval ( tMatch ) )
				{
					if constexpr ( HAS_SORT_CALC )
						tCtx.FreeDataSort ( tMatch );

					continue;
				}
			}

			tMatch.m_iTag = iTag;

			bool bRand = false;
			bool bNewMatch = false;
			for ( ISphMatchSorter * pSorter: dSorters )
			{
				// all non-random sorters are in the beginning,
				// so we can avoid the simple 'first-element' assertion
				if constexpr ( RANDOMIZE )
				{
					if ( !bRand && pSorter->IsRandom() )
					{
						bRand = true;
						tMatch.m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;

						if constexpr ( HAS_WEIGHT_FILTER )
						{
							if ( tCtx.m_pWeightFilter && !tCtx.m_pWeightFilter->Eval ( tMatch ) )
								break;
						}
					}
				}

				bNewMatch |= pSorter->Push ( tMatch );

				if constexpr ( USE_FACTORS )
				{
					RowTagged_t tJustPushed = pSorter->GetJustPushed();
					VecTraits_T<RowTagged_t> dJustPopped = pSorter->GetJustPopped();
					pRanker->ExtraData ( EXTRA_SET_MATCHPUSHED, (void**)&tJustPushed );
					pRanker->ExtraData ( EXTRA_SET_MATCHPOPPED, (void**)&dJustPopped );
				}
			}

			if constexpr ( HAS_FILTER_CALC )
				tCtx.FreeDataFilter ( tMatch );

			if constexpr ( HAS_SORT_CALC )
				tCtx.FreeDataSort ( tMatch );

			if constexpr ( HAS_CUTOFF )
			{
				if ( bNewMatch && --iCutoff==0 )
					break;
			}
		}

		if constexpr ( HAS_CUTOFF )
		{
			if ( !iCutoff )
				break;
		}
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
		return m_tCtx.m_dCalcFinal.any_of ( []( const ContextCalcItem_t & i ){ return i.m_pExpr && i.m_pExpr->IsColumnar(); } );
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
		CSphVector<ContextCalcItem_t *> dColumnWise, dRowWise;

		// process columnar items in column-wise order (and the rest in rowwise order)
		for ( auto & i : m_tCtx.m_dCalcFinal )
			if ( i.m_pExpr->IsColumnar() )
				dColumnWise.Add(&i);
			else
				dRowWise.Add(&i);

		for ( const auto & pItem : dColumnWise )
			for ( auto & pMatch : dMatches )
			{
				assert(pMatch);
				if ( pMatch->m_iTag>=0 )
					continue;

				m_tCtx.CalcItem ( *pMatch, *pItem );
			}

		for ( auto & pMatch : dMatches )
			for ( const auto & pItem : dRowWise )
			{
				assert(pMatch);
				if ( pMatch->m_iTag>=0 )
					continue;

				m_tCtx.CalcItem ( *pMatch, *pItem );
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


/// scoped worker scheduling helper
/// makes quantum of current task smaller
class ScopedLowPriority_c : public ISphNonCopyMovable
{
	int64_t m_iStoredThrottlingPeriodUS;

public:
	ScopedLowPriority_c()
		: m_iStoredThrottlingPeriodUS { Threads::Coro::GetThrottlingPeriodUS() }
	{
		if ( m_iStoredThrottlingPeriodUS > 0 )
			Threads::Coro::SetThrottlingPeriodUS ( Max ( m_iStoredThrottlingPeriodUS / g_iLowPriorityDivisor, 1 ) );
	}

	~ScopedLowPriority_c()
	{
		if ( m_iStoredThrottlingPeriodUS > 0 )
			Threads::Coro::SetThrottlingPeriodUS ( m_iStoredThrottlingPeriodUS );
	}
};

//////////////////////////////////////////////////////////////////////////

// the iterator does not support cutoff
// overwise tIterator.WasCutoffHit() at the Fullscan ends iterating blocks after 0 block fully scanned (!m_iRowsLeft)
// and for small cuttoff itrator scans only up to cuttoff rows in each block

template <bool HAVE_DEAD>
class RowIterator_T : public ISphNoncopyable
{
public:
	RowIterator_T ( const RowIdBoundaries_t & tBoundaries, const DeadRowMap_Disk_c & tDeadRowMap )
		: m_tRowID ( tBoundaries.m_tMinRowID )
		, m_tBoundaries ( tBoundaries )
		, m_tDeadRowMap	( tDeadRowMap )
	{}

	FORCE_INLINE bool GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock );
	DWORD		GetNumProcessed() const	{ return m_tRowID-m_tBoundaries.m_tMinRowID; }
	bool		WasCutoffHit() const	{ return false; }

private:
	static const int MAX_COLLECTED = 128;

	RowID_t						m_tRowID {INVALID_ROWID};
	RowIdBoundaries_t			m_tBoundaries;
	CSphFixedVector<RowID_t>	m_dCollected {MAX_COLLECTED};		// store 128 values (same as .spa attr block size)
	const DeadRowMap_Disk_c &	m_tDeadRowMap;
};

template <>
bool RowIterator_T<true>::GetNextRowIdBlock ( RowIdBlock_t & dRowIdBlock )
{
	RowID_t * pRowIdStart = m_dCollected.Begin();
	RowID_t * pRowIdMax = pRowIdStart + m_dCollected.GetLength();
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
	int64_t iDelta = Min ( RowID_t(m_dCollected.GetLength()), int64_t(m_tBoundaries.m_tMaxRowID)-m_tRowID+1 );
	assert ( iDelta>=0 );
	RowID_t * pRowIdMax = pRowIdStart + iDelta;
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

	DWORD	GetNumProcessed() const { return (DWORD)m_pIterator->GetNumProcessed(); }
	bool	WasCutoffHit() const { return m_pIterator->WasCutoffHit(); }

private:
	RowidIterator_i *				m_pIterator;
	CSphVector<RowID_t>				m_dCollected {0};
	const DeadRowMap_Disk_c &		m_tDeadRowMap;
};

//////////////////////////////////////////////////////////////////////////

template <bool SINGLE_SORTER, bool HAS_FILTER_CALC, bool HAS_SORT_CALC, bool HAS_FILTER, bool HAS_RANDOMIZE, bool HAS_MAX_TIMER, bool HAS_CUTOFF, typename ITERATOR, typename TO_STATIC>
bool Fullscan ( ITERATOR & tIterator, TO_STATIC && fnToStatic, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, int iIndexWeight, int64_t tmMaxTimer )
{
	auto tScopedStats = AtScopeExit ( [&tMeta, &tIterator]{tMeta.m_tStats.m_iFetchedDocs = (DWORD)tIterator.GetNumProcessed(); } );

	RowIdBlock_t dRowIDs;
	Threads::Coro::HighFreqChecker_c fnHeavyCheck;
	const int64_t& iCheckTimePoint { Threads::Coro::GetNextTimePointUS() };

	while ( tIterator.GetNextRowIdBlock(dRowIDs) )
	{
		for ( auto i : dRowIDs )
		{
			tMatch.m_tRowID = i;
			tMatch.m_pStatic = fnToStatic(i);

			// early filter only (no late filters in full-scan because of no @weight)
			if constexpr ( HAS_FILTER_CALC )
				tCtx.CalcFilter(tMatch);

			if constexpr ( HAS_FILTER )
			{
				if ( !tCtx.m_pFilter->Eval(tMatch) )
				{
					if_const ( HAS_FILTER_CALC )
						tCtx.FreeDataFilter ( tMatch );

					continue;
				}
			}

			if constexpr ( HAS_RANDOMIZE )
				tMatch.m_iWeight = ( sphRand() & 0xffff ) * iIndexWeight;

			if constexpr ( HAS_SORT_CALC )
				tCtx.CalcSort(tMatch);

			bool bNewMatch = false;
			if constexpr  ( SINGLE_SORTER )
				bNewMatch = dSorters[0]->Push(tMatch);
			else
				dSorters.for_each( [&tMatch, &bNewMatch] ( ISphMatchSorter * p ) { bNewMatch |= p->Push ( tMatch ); } );

			// stringptr expressions should be duplicated (or taken over) at this point
			if constexpr ( HAS_FILTER_CALC )
				tCtx.FreeDataFilter ( tMatch );

			if constexpr ( HAS_SORT_CALC )
				tCtx.FreeDataSort ( tMatch );

			if constexpr ( HAS_CUTOFF )
			{
				if ( bNewMatch && --iCutoff==0 )
					return true;
			}
		}

		// handle timer
		if constexpr ( HAS_MAX_TIMER )
		{
			if ( sph::TimeExceeded ( tmMaxTimer ) )
			{
				tMeta.m_sWarning = "query time exceeded max_query_time";
				return true;
			}
		}

		if ( fnHeavyCheck() && sph::TimeExceeded ( iCheckTimePoint ) )
		{
			if ( session::GetKilled() )
			{
				tMeta.m_sWarning = "query was killed";
				return true;
			}
			Threads::Coro::RescheduleAndKeepCrashQuery();
		}
	}

	return tIterator.WasCutoffHit();
}


template <typename ITERATOR, typename TO_STATIC>
bool RunFullscan ( ITERATOR & tIterator, TO_STATIC && fnToStatic, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *>& dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer )
{
	bool bHasFilterCalc = !tCtx.m_dCalcFilter.IsEmpty();
	bool bHasSortCalc = !tCtx.m_dCalcSort.IsEmpty();
	bool bHasFilter = !!tCtx.m_pFilter;
	bool bHasTimer = tmMaxTimer>0;
	bool bHasCutoff = iCutoff!=-1;

	// when we have join query with multiple sorters, the first sorter does all the work (including pushing to all other sorters)
	// we can avoid pushing to other sorters to improve performance
	bool bSingleSorter = dSorters.GetLength()==1 || dSorters[0]->IsJoin();
	int iIndex = bSingleSorter*64 + bHasFilterCalc*32 + bHasSortCalc*16 + bHasFilter*8 + bRandomize*4 + bHasTimer*2 + bHasCutoff;

	switch ( iIndex )
	{
#define DECL_FNSCAN( _, n, params ) case n: return Fullscan<!!(n&64), !!(n&32), !!(n&16), !!(n&8), !!(n&4), !!(n&2), !!(n&1), ITERATOR, TO_STATIC> params;
	BOOST_PP_REPEAT ( 128, DECL_FNSCAN, ( tIterator, std::forward<TO_STATIC> ( fnToStatic ), tCtx, tMeta, dSorters, tMatch, iCutoff, iIndexWeight, tmMaxTimer ) )
#undef DECL_FNSCAN
		default:
			assert ( 0 && "Internal error" );
			break;
	}

	return false;
}


bool CSphIndex_VLN::RunFullscanOnAttrs ( const RowIdBoundaries_t & tBoundaries, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer ) const
{
	const CSphRowitem * pStart = m_tAttr.GetReadPtr();
	int iStride = m_tSchema.GetRowSize();
	auto fnToStatic = [pStart, iStride]( RowID_t tRowID ){ return pStart+(int64_t)tRowID*iStride; };

	if ( m_tDeadRowMap.HasDead() )
	{
		RowIterator_T<true> tIt ( tBoundaries, m_tDeadRowMap );
		return RunFullscan ( tIt, fnToStatic, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer );
	}
	else
	{
		RowIterator_T<false> tIt ( tBoundaries, m_tDeadRowMap );
		return RunFullscan ( tIt, fnToStatic, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer );
	}
}


bool CSphIndex_VLN::RunFullscanOnIterator ( RowidIterator_i * pIterator, const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer ) const
{
	const CSphRowitem * pStart = m_tAttr.GetReadPtr();
	int iStride = m_tSchema.GetRowSize();
	auto fnToStatic = [pStart, iStride]( RowID_t tRowID ){ return pStart+(int64_t)tRowID*iStride; };

	if ( m_tDeadRowMap.HasDead() )
	{
		RowIteratorAlive_c tIt ( pIterator, m_tDeadRowMap );
		return RunFullscan ( tIt, fnToStatic, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer );
	}

	return RunFullscan ( *pIterator, fnToStatic, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer );
}


template <bool ROWID_LIMITS>
bool CSphIndex_VLN::ScanByBlocks ( const CSphQueryContext & tCtx, CSphQueryResultMeta & tMeta, const VecTraits_T<ISphMatchSorter *> & dSorters, CSphMatch & tMatch, int iCutoff, bool bRandomize, int iIndexWeight, int64_t tmMaxTimer, const RowIdBoundaries_t * pBoundaries ) const
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

		if ( RunFullscanOnAttrs ( tBlockBoundaries, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, iIndexWeight, tmMaxTimer ) )
			return true;
	}

	return false;
}


RowIteratorsWithEstimates_t CSphIndex_VLN::CreateColumnarAnalyzerOrPrefilter ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, const ISphFilter * pFilter, ESphCollation eCollation, const ISphSchema & tSchema, CSphString & sWarning ) const
{
	if ( !m_pColumnar || dFilterTree.GetLength() || !pFilter )
		return {};

	std::vector<common::Filter_t> dColumnarFilters;
	std::vector<int> dFilterMap;
	ToColumnarFilters ( dFilters, dColumnarFilters, dFilterMap, tSchema, eCollation, sWarning );

	// remove disabled analyzers
	std::vector<int> dToDelete;
	for ( size_t i = 0; i < dFilterMap.size(); i++ )
	{
		if ( dFilterMap[i]==-1 )
			continue;

		if ( dSIInfo[i].m_eType!=SecondaryIndexType_e::ANALYZER && dFilters[i].m_sAttrName!="@rowid" )
		{
			dToDelete.push_back ( dFilterMap[i] );
			dFilterMap[i] = -1;
		}
	}

	for ( int i = (int)dToDelete.size()-1; i>=0; i-- )
		dColumnarFilters.erase ( dColumnarFilters.begin() + dToDelete[i] );

	if ( dColumnarFilters.empty() || ( dColumnarFilters.size()==1 && dColumnarFilters[0].m_sName=="@rowid" ) )
		return {};

	std::vector<int> dDeletedFilters;
	std::vector<common::BlockIterator_i *> dIterators;
	dIterators = m_pColumnar->CreateAnalyzerOrPrefilter ( dColumnarFilters, dDeletedFilters, *pFilter );
	if ( dIterators.empty() )
		return {};

	for ( size_t i = 0; i < dFilterMap.size(); i++ )
		if ( dFilterMap[i]!=-1 && std::binary_search ( dDeletedFilters.begin(), dDeletedFilters.end(), dFilterMap[i] ) )
			dSIInfo[i].m_bCreated = true;

	RowIdBoundaries_t tBoundaries;
	const CSphFilterSettings * pRowIdFilter = GetRowIdFilter ( dFilters, RowID_t(m_iDocinfo), tBoundaries );

	RowIteratorsWithEstimates_t dResult;
	for ( auto & i : dIterators )
		dResult.Add ( { CreateIteratorWrapper ( i, pRowIdFilter ? &tBoundaries : nullptr ), 0 } );

	for ( int i = 0; i < (int)dFilterMap.size(); i++ )
	{
		int iColumnarFilter = dFilterMap[i];
		if ( iColumnarFilter<0 )
			continue;

		auto tFound = std::find ( dDeletedFilters.begin(), dDeletedFilters.end(), iColumnarFilter );
		if ( tFound == dDeletedFilters.end() )
			continue;

		int iFound = tFound - dDeletedFilters.begin();
		dResult[iFound].second = dSIInfo[i].m_iRsetEstimate; 
	}

	return dResult;
}


static void RecreateFilters ( const CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphVector<CSphFilterSettings> & dFilters, CSphQueryContext & tCtx, CreateFilterContext_t & tFlx, CSphQueryResultMeta & tMeta, CSphVector<CSphFilterSettings> & dModifiedFilters )
{
	dModifiedFilters.Resize(0);
	ARRAY_FOREACH ( i, dSIInfo )
	{
		bool bRemovedOptional = dFilters[i].m_bOptional && dSIInfo[i].m_eType==SecondaryIndexType_e::NONE;
		if (  !dSIInfo[i].m_bCreated && !bRemovedOptional )
			dModifiedFilters.Add ( dFilters[i] );
	}

	tCtx.m_pFilter.reset();
	tFlx.m_pFilters = &dModifiedFilters;
	tCtx.CreateFilters ( tFlx, tMeta.m_sError, tMeta.m_sWarning );
}


bool CSphIndex_VLN::SelectIteratorsFT ( const CSphQuery & tQuery, const CSphVector<CSphFilterSettings> & dFilters, const ISphSchema & tSorterSchema, ISphRanker * pRanker, CSphVector<SecondaryIndexInfo_t> & dSIInfo, int iCutoff, int iThreads, StrVec_t & dWarnings ) const
{
	// in fulltext case we do the following:
	// 1. calculate cost of FT search and number of docs after FT search
	// 2. calculate cost of filters over the number of docs after FT search
	// 3. calculate the best cost of filters/scan/SI over the whole index
	// 4. estimate the cost of intersecting FT and iterator results
	NodeEstimate_t tEstimate = pRanker->Estimate ( m_iDocinfo );

	// always do single-thread estimates here
	SelectIteratorCtx_t tSelectIteratorCtx ( tQuery, dFilters, m_tSchema, tSorterSchema, m_pHistograms, m_pColumnar.get(), m_tSI, iCutoff, m_iDocinfo, 1 );
	tSelectIteratorCtx.IgnorePushCost();
	float fBestCost = FLT_MAX;
	dSIInfo = SelectIterators ( tSelectIteratorCtx, fBestCost, dWarnings );

	// check that we have anything non-plain-filter. if not, bail out
	if ( !dSIInfo.any_of ( []( const auto & tInfo ){ return tInfo.m_eType==SecondaryIndexType_e::LOOKUP || tInfo.m_eType==SecondaryIndexType_e::INDEX || tInfo.m_eType==SecondaryIndexType_e::ANALYZER; } ) )
		return false;

	CSphVector<SecondaryIndexInfo_t> dSIInfoFilters { dSIInfo.GetLength() };
	float fValuesAfterFilters = 1.0f;
	ARRAY_FOREACH ( i, dSIInfo )
		if ( dFilters[i].m_sAttrName != "@rowid" )
		{
			dSIInfoFilters[i] = dSIInfo[i];
			fValuesAfterFilters *= float(dSIInfo[i].m_iRsetEstimate) / m_iDocinfo;
			dSIInfoFilters[i].m_eType = SecondaryIndexType_e::FILTER;
		}

	// correct rset estimates (we are estimating filters after FT)
	float fCostOfFilters = 0.0f;
	if ( tEstimate.m_iDocs>0 )
	{
		// fixme! update code to use SelectIteratorCtx_t::m_fDocsLeft
		float fRatio = float ( tEstimate.m_iDocs ) / tSelectIteratorCtx.m_iTotalDocs;
		for ( auto & i : dSIInfoFilters )
			i.m_iRsetEstimate *= fRatio;

		tSelectIteratorCtx.m_iTotalDocs = tEstimate.m_iDocs;
		tSelectIteratorCtx.m_bFromIterator = true;
		std::unique_ptr<CostEstimate_i> pCostEstimate ( CreateCostEstimate ( dSIInfoFilters, tSelectIteratorCtx, tSelectIteratorCtx.m_iCutoff ) );
		fCostOfFilters = pCostEstimate->CalcQueryCost();
	}

	int64_t iDocsAfterFilters = int64_t(fValuesAfterFilters*m_iDocinfo);
	NodeEstimate_t tIteratorEst = { fBestCost, iDocsAfterFilters, 1 };
	const int ITERATOR_BLOCK_SIZE = 1024;
	tEstimate.m_iDocs = Max ( tEstimate.m_iDocs, 1 );
	float fIteratorWithFT = CalcFTIntersectCost ( tIteratorEst, tEstimate, m_iDocinfo, ITERATOR_BLOCK_SIZE, MAX_BLOCK_DOCS );
	float fFTWithFilters = tEstimate.m_fCost + fCostOfFilters;

	fIteratorWithFT = EstimateMTCostSIFT ( fIteratorWithFT, iThreads );
	fFTWithFilters = EstimateMTCost ( fFTWithFilters, iThreads );

	if ( fIteratorWithFT<fFTWithFilters )
	{
		return true;
	} else
	{
		// if has any forced indexes when should use the path with iterators even FT estimates faster
		return dSIInfo.any_of ( []( const auto & tInfo ){ return tInfo.m_eForce!=SecondaryIndexType_e::NONE; } );
	}
}


static int CalcRemovedOptionalFilters ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<SecondaryIndexInfo_t> & dSIInfo )
{
	int iRemovedOptional = 0;
	ARRAY_FOREACH ( i, dFilters )
		if ( dFilters[i].m_bOptional && dSIInfo[i].m_eType==SecondaryIndexType_e::NONE )
			iRemovedOptional++;

	return iRemovedOptional;
}


static void RemoveOptionalFilters ( const CSphVector<CSphFilterSettings> & dFilters, CSphQueryContext & tCtx, CreateFilterContext_t & tFlx, CSphQueryResultMeta & tMeta, CSphVector<CSphFilterSettings> & dModifiedFilters )
{
	int iNumOptional = 0;
	dModifiedFilters.Resize(0);
	for ( auto & i : dFilters )
		if ( i.m_bOptional )
			iNumOptional++;
		else
			dModifiedFilters.Add(i);

	if ( iNumOptional )
	{
		tCtx.m_pFilter.reset();
		tFlx.m_pFilters = &dModifiedFilters;
		tCtx.CreateFilters ( tFlx, tMeta.m_sError, tMeta.m_sWarning );
	}
}


bool CSphIndex_VLN::ChooseIterators ( CSphVector<SecondaryIndexInfo_t> & dSIInfo, const CSphQuery & tQuery, const CSphVector<CSphFilterSettings> & dFilters, CSphQueryContext & tCtx, CreateFilterContext_t & tFlx, const ISphSchema & tMaxSorterSchema, CSphQueryResultMeta & tMeta, int iCutoff, int iThreads, CSphVector<CSphFilterSettings> & dModifiedFilters, ISphRanker * pRanker ) const
{
	StrVec_t dWarnings;
	bool bKNN = !tQuery.m_sKNNAttr.IsEmpty();
	float fBestCost = FLT_MAX;

	if ( bKNN )
	{
		SelectIteratorCtx_t tSelectIteratorCtx ( tQuery, dFilters, m_tSchema, tMaxSorterSchema, m_pHistograms, m_pColumnar.get(), m_tSI, iCutoff, m_iDocinfo, 1 );
		tSelectIteratorCtx.m_bFromIterator = true;

		int iRequestedKNNDocs = Min ( tQuery.m_iKNNK, m_iDocinfo );
		tSelectIteratorCtx.m_fDocsLeft = float(iRequestedKNNDocs)/m_iDocinfo;
		dSIInfo = SelectIterators ( tSelectIteratorCtx, fBestCost, dWarnings );
	}
	else
	{
		if ( !pRanker )
		{
			// In order to maintain some consistency with GetPseudoShardingMetric() we need to do one of the following:
			// a. Run this with the number of docs in this pseudo_chunk and one thread
			// b. Run this with the same number of docs and number of threads as in GetPseudoShardingMetric()
			// For now we use approach b) as it is simpler
			SelectIteratorCtx_t tSelectIteratorCtx ( tQuery, dFilters, m_tSchema, tMaxSorterSchema, m_pHistograms, m_pColumnar.get(), m_tSI, iCutoff, m_iDocinfo, iThreads );
			dSIInfo = SelectIterators ( tSelectIteratorCtx, fBestCost, dWarnings );
		}
		else
		{
			bool bRes = SelectIteratorsFT ( tQuery, dFilters, tMaxSorterSchema, pRanker, dSIInfo, iCutoff, iThreads, dWarnings );
			if ( !bRes )
			{
				// if we did not spawn any iterators, we need to remove optional filters (as they assume they will be replaced by iterators)
				RemoveOptionalFilters ( dFilters, tCtx, tFlx, tMeta, dModifiedFilters );
				return false;
			}
		}
	}

	if ( dWarnings.GetLength() )
		tMeta.m_sWarning = ConcatWarnings(dWarnings);

	return true;
}


std::pair<RowidIterator_i *, bool> CSphIndex_VLN::SpawnIterators ( const CSphQuery & tQuery, const CSphVector<CSphFilterSettings> & dFilters, CSphQueryContext & tCtx, CreateFilterContext_t & tFlx, const ISphSchema & tMaxSorterSchema, CSphQueryResultMeta & tMeta, int iCutoff, int iThreads, CSphVector<CSphFilterSettings> & dModifiedFilters, ISphRanker * pRanker ) const
{
	if ( !dFilters.GetLength() )
	{
		if ( !tQuery.m_sKNNAttr.IsEmpty() )
			return CreateKNNIterator ( m_pKNN.get(), tQuery, m_tSchema, tMaxSorterSchema, tMeta.m_sError );

		return { nullptr, false };
	}

	// using g_iPseudoShardingThresh==0 check so that iterators are still spawned in test suite (when g_iPseudoShardingThresh=0)
	const int64_t SMALL_INDEX_THRESH = 8192;
	if ( m_iDocinfo < SMALL_INDEX_THRESH && g_iPseudoShardingThresh > 0 )
	{
		dModifiedFilters = dFilters;
		return { nullptr, false };
	}

	CSphVector<SecondaryIndexInfo_t> dSIInfo;
	if ( !ChooseIterators ( dSIInfo, tQuery, dFilters, tCtx, tFlx, tMaxSorterSchema, tMeta, iCutoff, iThreads, dModifiedFilters, pRanker ) )
		return { nullptr, false };

	RowIteratorsWithEstimates_t dSIIterators, dLookupIterators, dAnalyzerIterators, dKNNIterators;

	int iRemovedOptional = CalcRemovedOptionalFilters ( dFilters, dSIInfo );

	// knn iterators
	bool bError = false;
	dKNNIterators = CreateKNNIterators ( m_pKNN.get(), tQuery, m_tSchema, tMaxSorterSchema, bError, tMeta.m_sError );
	if ( bError )
		return { nullptr, true };

	// secondary index iterators
	dSIIterators = m_tSI.CreateSecondaryIndexIterator ( dSIInfo, dFilters, tQuery.m_eCollation, tMaxSorterSchema, RowID_t(m_iDocinfo), iCutoff );

	// lookup-by-id (.SPT) iterators
	dLookupIterators = CreateLookupIterator ( dSIInfo, dFilters, m_tDocidLookup.GetReadPtr(), RowID_t(m_iDocinfo) );

	// try to spawn analyzers or prefilters from columnar storage
	// if we already created an iterator at prev stage, we need to recreate filters here,
	// so we won't be doing unnecessary minmax eval over filters that were replaced by iterators
	int iCreated = 0;
	dSIInfo.for_each ( [&]( const SecondaryIndexInfo_t & tInfo ){ if ( tInfo.m_bCreated ) iCreated++; } );
	if ( ( iCreated && m_pColumnar ) || iRemovedOptional )
		RecreateFilters ( dSIInfo, dFilters, tCtx, tFlx, tMeta, dModifiedFilters );

	dAnalyzerIterators = CreateColumnarAnalyzerOrPrefilter ( dSIInfo, dFilters, tQuery.m_dFilterTree, tCtx.m_pFilter.get(), tQuery.m_eCollation, tMaxSorterSchema, tMeta.m_sWarning );

	int iCreatedAfterColumnar = 0;
	dSIInfo.for_each ( [&]( const SecondaryIndexInfo_t & tInfo ){ if ( tInfo.m_bCreated ) iCreatedAfterColumnar++; } );

	// if we created an analyzer, we need to recreate filters
	if ( ( !m_pColumnar && iCreated>0 ) || iCreatedAfterColumnar!=iCreated )
		RecreateFilters ( dSIInfo, dFilters, tCtx, tFlx, tMeta, dModifiedFilters );

	RowIteratorsWithEstimates_t dAllIterators;
	for ( auto i : dKNNIterators )
		dAllIterators.Add(i);

	for ( auto i : dSIIterators )
		dAllIterators.Add(i);

	for ( auto i : dLookupIterators )
		dAllIterators.Add(i);

	for ( auto i : dAnalyzerIterators )
		dAllIterators.Add(i);

	dAllIterators.Sort ( ::bind ( &std::pair<RowidIterator_i *,int64_t>::second ) );

	CSphVector<RowidIterator_i *> dFinalIterators;
	for ( auto i : dAllIterators )
		dFinalIterators.Add ( i.first );

	switch ( dFinalIterators.GetLength() )
	{
	case 0:
		RemoveOptionalFilters ( dFilters, tCtx, tFlx, tMeta, dModifiedFilters );
		return { nullptr, false };

	case 1:
		return { dFinalIterators[0], false };

	default:
		// both columnar iterator wrappers and secondary index iterators support rowid filtering, so no need for it here
		return { CreateIteratorIntersect ( dFinalIterators, nullptr ), false };
	}
}


static bool AreAllFiltersColumnar ( const CSphVector<CSphFilterSettings> & dFilters, const ISphSchema & tSchema )
{
	return dFilters.all_of ( [&tSchema]( const CSphFilterSettings & tFilter )
		{
			if ( tFilter.m_sAttrName=="@rowid" )
				return true;

			const CSphColumnInfo * pCol = tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
			return pCol ? ( pCol->IsColumnar() || pCol->IsColumnarExpr() ) : false;
		} );
}


static bool AreAllFiltersExpressions ( const CSphVector<CSphFilterSettings> & dFilters, const ISphSchema & tSchema )
{
	return dFilters.all_of ( [&tSchema]( const CSphFilterSettings & tFilter )
		{
			if ( tFilter.m_sAttrName=="@rowid" )
				return true;

			const CSphColumnInfo * pCol = tSchema.GetAttr ( tFilter.m_sAttrName.cstr() );
			return pCol && pCol->m_pExpr;
		} );

}


bool CSphIndex_VLN::SetupFiltersAndContext ( CSphQueryContext & tCtx, CreateFilterContext_t & tFlx, CSphQueryResultMeta & tMeta, const ISphSchema * & pMaxSorterSchema, CSphVector<CSphFilterSettings> & dTransformedFilters, CSphVector<FilterTreeItem_t> & dTransformedFilterTree, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs ) const
{
	// select the sorter with max schema
	int iMaxSchemaIndex = GetMaxSchemaIndexAndMatchCapacity ( dSorters ).first;
	pMaxSorterSchema = dSorters[iMaxSchemaIndex]->GetSchema();
	auto dSorterSchemas = SorterSchemas ( dSorters, iMaxSchemaIndex);

	auto & tQuery = tCtx.m_tQuery;

	// setup filters
	tFlx.m_pFilters		= &tQuery.m_dFilters;
	tFlx.m_pFilterTree	= &tQuery.m_dFilterTree;
	tFlx.m_pMatchSchema	= pMaxSorterSchema;
	tFlx.m_pIndexSchema	= &m_tSchema;
	tFlx.m_pBlobPool	= m_tBlobAttrs.GetReadPtr();
	tFlx.m_pColumnar	= m_pColumnar.get();
	tFlx.m_eCollation	= tQuery.m_eCollation;
	tFlx.m_bScan		= tQuery.m_sQuery.IsEmpty();
	tFlx.m_pHistograms	= m_pHistograms;
	tFlx.m_pSI			= &m_tSI;
	tFlx.m_iTotalDocs	= m_iDocinfo;
	tFlx.m_sJoinIdx		= tQuery.m_sJoinIdx;

	// may modify eval stages in schema; needs to be before SetupCalc
	if ( !TransformFilters ( tFlx, dTransformedFilters, dTransformedFilterTree, tQuery.m_dItems, tMeta.m_sError ) )
		return false;

	tFlx.m_pFilters = &dTransformedFilters;
	tFlx.m_pFilterTree = dTransformedFilterTree.GetLength() ? &dTransformedFilterTree : nullptr;

	// setup calculations and result schema
	if ( !tCtx.SetupCalc ( tMeta, *pMaxSorterSchema, m_tSchema, m_tBlobAttrs.GetReadPtr(), m_pColumnar.get(), dSorterSchemas ) )
		return false;

	// set blob pool for string on_sort expression fix up
	tCtx.SetBlobPool ( m_tBlobAttrs.GetReadPtr() );
	tCtx.SetColumnar ( m_pColumnar.get() );
	tCtx.m_pProfile = tMeta.m_pProfile;
	tCtx.m_pLocalDocs = tArgs.m_pLocalDocs;
	tCtx.m_iTotalDocs = ( tArgs.m_iTotalDocs ? tArgs.m_iTotalDocs : m_tStats.m_iTotalDocuments );
	tCtx.m_iIndexTotalDocs = m_iDocinfo;

	return tCtx.CreateFilters ( tFlx, tMeta.m_sError, tMeta.m_sWarning );
}


bool CSphIndex_VLN::MultiScan ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const
{
	assert ( tArgs.m_iTag>=0 );
	auto& tMeta = *tResult.m_pMeta;

	// check if index is ready
	if ( !m_bPassedAlloc )
	{
		tMeta.m_sError = "table not preread";
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
	{
		PooledAttrsToPtrAttrs ( dSorters, m_tBlobAttrs.GetReadPtr(), m_pColumnar.get(), tArgs.m_bFinalizeSorters, tMeta.m_pProfile, tArgs.m_bModifySorterSchemas );
		return true;
	}

	// start counting
	int64_t tmQueryStart = sphMicroTimer();
	int64_t tmCpuQueryStart = sphTaskCpuTimer();

	std::optional<ScopedLowPriority_c> tPrio;
	if ( tQuery.m_bLowPriority )
		tPrio.emplace();

	CSphQueryContext tCtx(tQuery);
	CreateFilterContext_t tFlx;
	const ISphSchema * pMaxSorterSchema = nullptr;
	CSphVector<CSphFilterSettings> dTransformedFilters; // holds filter settings if they were modified. filters hold pointers to those settings
	CSphVector<FilterTreeItem_t> dTransformedFilterTree;
	if ( !SetupFiltersAndContext ( tCtx, tFlx, tMeta, pMaxSorterSchema, dTransformedFilters, dTransformedFilterTree, dSorters, tArgs ) )
		return false;

	assert(pMaxSorterSchema);
	const ISphSchema & tMaxSorterSchema = *pMaxSorterSchema;

	if ( CheckEarlyReject ( dTransformedFilters, tCtx.m_pFilter.get(), tQuery.m_eCollation, tMaxSorterSchema ) )
	{
		PooledAttrsToPtrAttrs ( dSorters, m_tBlobAttrs.GetReadPtr(), m_pColumnar.get(), tArgs.m_bFinalizeSorters, tMeta.m_pProfile, tArgs.m_bModifySorterSchemas );

		tMeta.m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
		tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;
		return true;
	}

	// setup sorters
	for ( auto & i : dSorters )
	{
		i->SetBlobPool ( m_tBlobAttrs.GetReadPtr() );
		i->SetColumnar ( m_pColumnar.get() );
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

	auto & tSess = session::Info();
	tSess.m_pSessionOpaque1 = (void*)(const DocstoreReader_i*)this;
	tSess.m_pSessionOpaque2 = (void*)m_pDocstore.get();

	SwitchProfile ( tMeta.m_pProfile, SPH_QSTATE_SETUP_ITER );

	int iCutoff = ApplyImplicitCutoff ( tQuery, dSorters, false );
	bool bAllPrecalc = dSorters.GetLength() && dSorters.all_of ( []( auto pSorter ){ return pSorter->IsPrecalc(); } );

	int iOldLen = tMeta.m_tIteratorStats.m_dIterators.GetLength();
	for ( auto & i : dSorters )
		i->AddDesc ( tMeta.m_tIteratorStats.m_dIterators );

	if ( tMeta.m_tIteratorStats.m_dIterators.GetLength()!=iOldLen )
		tMeta.m_tIteratorStats.m_iTotal = 1;

	// try to spawn an iterator from a secondary index
	CSphVector<CSphFilterSettings> dFiltersAfterIterator; // holds filter settings if they were modified. filters hold pointers to those settings
	std::unique_ptr<RowidIterator_i> pIterator;
	if ( bAllPrecalc )
		tCtx.m_pFilter.reset();
	else
	{
		auto tSpawned = SpawnIterators ( tQuery, dTransformedFilters, tCtx, tFlx, tMaxSorterSchema, tMeta, iCutoff, tArgs.m_iTotalThreads, dFiltersAfterIterator, nullptr );
		pIterator = std::unique_ptr<RowidIterator_i> ( tSpawned.first );
		if ( tSpawned.second )
			return false;
	}
	
	SwitchProfile ( tMeta.m_pProfile, SPH_QSTATE_FULLSCAN );

	bool bCutoffHit =  false;
	if ( pIterator )
	{
		if ( iCutoff>=0 && !tCtx.m_pFilter )
			pIterator->SetCutoff(iCutoff);

		bCutoffHit = RunFullscanOnIterator ( pIterator.get(), tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer );
		pIterator->AddDesc ( tMeta.m_tIteratorStats.m_dIterators );

		tMeta.m_tIteratorStats.m_iTotal = 1;
	}
	else
	{
		RowIdBoundaries_t tBoundaries;
		const CSphFilterSettings * pRowIdFilter = GetRowIdFilter ( dFiltersAfterIterator, (RowID_t)m_iDocinfo, tBoundaries );
		if ( !pRowIdFilter )
			tBoundaries.m_tMaxRowID = RowID_t(m_iDocinfo)-1;

		bool bAllFiltersColumnar = AreAllFiltersColumnar ( dFiltersAfterIterator, m_tSchema );
		bool bOnlyExprFilters = AreAllFiltersExpressions ( dFiltersAfterIterator, tMaxSorterSchema );
		bool bAllAttrsColumnar = !m_iDocinfoIndex;

		// use block filtering only when we have attribute with block index
		if ( bAllFiltersColumnar || bAllAttrsColumnar || bOnlyExprFilters )
			bCutoffHit = RunFullscanOnAttrs ( tBoundaries, tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer );
		else
		{
			if ( pRowIdFilter )
				bCutoffHit = ScanByBlocks<true> ( tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer, &tBoundaries );
			else
				bCutoffHit = ScanByBlocks<false> ( tCtx, tMeta, dSorters, tMatch, iCutoff, bRandomize, tArgs.m_iIndexWeight, tmMaxTimer );
		}
	}

	tMeta.m_bTotalMatchesApprox = bCutoffHit && !bAllPrecalc;

	SwitchProfile ( tMeta.m_pProfile, SPH_QSTATE_FINALIZE );

	if ( dSorters.any_of ( [&] ( ISphMatchSorter * p ) { return !p->FinalizeJoin ( tMeta.m_sError, tMeta.m_sWarning ); } ) )
		return false;

	// do final expression calculations
	if ( tCtx.m_dCalcFinal.GetLength() )
	{
		DocstoreSession_c tSession;
		int64_t iSessionUID = tSession.GetUID();

		// spawn buffered readers for the current session
		// put them to a global hash
		if ( m_pDocstore )
			m_pDocstore->CreateReader ( iSessionUID );

		DocstoreSession_c::InfoRowID_t tSessionInfo;
		tSessionInfo.m_pDocstore = m_pDocstore.get();
		tSessionInfo.m_iSessionId = iSessionUID;

		for ( auto & i : tCtx.m_dCalcFinal )
		{
			assert ( i.m_pExpr );
			if ( m_pDocstore )
				i.m_pExpr->Command ( SPH_EXPR_SET_DOCSTORE_ROWID, &tSessionInfo );
		}

		SphFinalMatchCalc_t tFinal ( tArgs.m_iTag, tCtx );
		dSorters.Apply ( [&] ( ISphMatchSorter * p ) { p->Finalize ( tFinal, false, false ); } );
	}

	PooledAttrsToPtrAttrs ( dSorters, m_tBlobAttrs.GetReadPtr(), m_pColumnar.get(), tArgs.m_bFinalizeSorters, tMeta.m_pProfile, tArgs.m_bModifySorterSchemas );

	// done
	tResult.m_pBlobPool = m_tBlobAttrs.GetReadPtr();
	tResult.m_pDocstore = m_pDocstore ? this : nullptr;
	tResult.m_pColumnar = m_pColumnar.get();

	tMeta.m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
	tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;

	return true;
}

//////////////////////////////////////////////////////////////////////////////

ISphQword * DiskIndexQwordSetup_c::QwordSpawn ( const XQKeyword_t & tWord ) const
{
	if ( !tWord.m_pPayload )
	{
		WITH_QWORD ( m_pIndex, false, Qword, return new Qword ( tWord.m_bExpanded, tWord.m_bExcluded, m_pIndex->GetIndexId() ) );
	} else
	{
		if ( m_pIndex->GetSettings().m_eHitFormat==SPH_HIT_FORMAT_INLINE )
			return new DiskPayloadQword_c<true> ( (const DiskSubstringPayload_t *)tWord.m_pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist, m_pIndex->GetIndexId() );
		else
			return new DiskPayloadQword_c<false> ( (const DiskSubstringPayload_t *)tWord.m_pPayload, tWord.m_bExcluded, m_pDoclist, m_pHitlist, m_pIndex->GetIndexId() );
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

bool DiskIndexQwordSetup_c::SetupWithWrd ( const DiskIndexQwordTraits_c& tWord, DictEntry_t& tRes ) const
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

bool DiskIndexQwordSetup_c::SetupWithCrc ( const DiskIndexQwordTraits_c& tWord, DictEntry_t& tRes ) const
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

	DictEntry_t tRes;
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
		// OPTIMIZE? maybe add an option to decompress on preload instead?
		if ( m_pSkips && tRes.m_iDocs>m_iSkiplistBlockSize )
		{
			int iSkips = tRes.m_iDocs/m_iSkiplistBlockSize;
			const int SMALL_SKIP_THRESH = 256;
			bool bNeedCache = iSkips > SMALL_SKIP_THRESH;

			bool & bFromCache = tWord.m_bSkipFromCache;

			bFromCache = bNeedCache && SkipCache::Find ( { m_pIndex->GetIndexId(), tWord.m_uWordID }, tWord.m_pSkipData );
			if ( !bFromCache )
			{
				tWord.m_pSkipData = new SkipData_t;
				tWord.m_pSkipData->Read ( m_pSkips, tRes, tWord.m_iDocs, m_iSkiplistBlockSize );
				bFromCache = bNeedCache && SkipCache::Add ( { m_pIndex->GetIndexId(), tWord.m_uWordID }, tWord.m_pSkipData );
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

	// at the RT index some rows could be killed at any segment
	while ( true )
	{
		if ( m_tDoc.m_tRowID==INVALID_ROWID )
			m_tDoc.m_tRowID = 0;
		else
			m_tDoc.m_tRowID++;

		if ( m_tDoc.m_tRowID>=m_iRowsCount )
		{
			m_bDone = true;
			m_tDoc.m_tRowID = INVALID_ROWID;
			break;
		}

		if ( IsAliveRow ( m_tDoc.m_tRowID ) )
			break;
	}

	return m_tDoc;
}

ISphQword * DiskIndexQwordSetup_c::ScanSpawn() const
{
	return new QwordScan_c ( m_iRowsCount );
}

//////////////////////////////////////////////////////////////////////////////

bool CSphIndex_VLN::Lock ()
{
	CSphString sName = GetFilename ( SPH_EXT_SPL );
	sphLogDebug ( "Locking the table via file %s", sName.cstr() );
	return RawFileLock ( sName, m_iLockFD, m_sLastError );
}

void CSphIndex_VLN::Unlock()
{
	CSphString sName = GetFilename ( SPH_EXT_SPL );
	if ( m_iLockFD<0 )
		return;
	sphLogDebug ( "Unlocking the table (lock %s)", sName.cstr() );
	RawFileUnLock ( sName, m_iLockFD );
}


void CSphIndex_VLN::Dealloc ()
{
	if ( !m_bPassedAlloc )
		return;

	m_pDoclistFile = nullptr;
	m_pHitlistFile = nullptr;
	m_pColumnar = nullptr;
	m_tSI.Reset();

	m_tAttr.Reset ();
	m_tBlobAttrs.Reset();
	m_tSkiplists.Reset ();
	m_tWordlist.Reset ();
	m_tDeadRowMap.Dealloc();
	m_tDocidLookup.Reset();
	m_pDocstore.reset();

	m_iDocinfo = 0;
	m_iMinMaxIndex = 0;

	m_bPassedRead = false;
	m_bPassedAlloc = false;
	m_uAttrsStatus = 0;

	QcacheDeleteIndex ( m_iIndexId );
	SkipCache::DeleteAll ( m_iIndexId );

	m_iIndexId = GetIndexUid();
}


CSphIndex_VLN::LOAD_E CSphIndex_VLN::LoadHeaderLegacy ( const CSphString& sHeaderName, bool bStripPath, CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder, CSphString & sWarning )
{
	const int MAX_HEADER_SIZE = 32768;
	CSphFixedVector<BYTE> dCacheInfo ( MAX_HEADER_SIZE );

	m_sLastError = "";

	CSphAutoreader rdInfo ( dCacheInfo.Begin(), MAX_HEADER_SIZE ); // to avoid mallocs
	if ( !rdInfo.Open ( sHeaderName, m_sLastError ) )
		return LOAD_E::GeneralError_e;

	// magic header
	const char* sFmt = CheckFmtMagic ( rdInfo.GetDword () );
	if ( sFmt )
	{
		m_sLastError.SetSprintf ( sFmt, sHeaderName.cstr() );
		return LOAD_E::ParseError_e;
	}

	// version
	m_uVersion = rdInfo.GetDword();
	if ( m_uVersion<=1 || m_uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%u, binary is v.%u", sHeaderName.cstr(), m_uVersion, INDEX_FORMAT_VERSION );
		return LOAD_E::GeneralError_e;
	}

	// we don't support anything prior to v54
	DWORD uMinFormatVer = 54;
	if ( m_uVersion<uMinFormatVer )
	{
		m_sLastError.SetSprintf ( "tables prior to v.%u are no longer supported (use index_converter tool); %s is v.%u", uMinFormatVer, sHeaderName.cstr(), m_uVersion );
		return LOAD_E::GeneralError_e;
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
		return LOAD_E::GeneralError_e;

	if ( bStripPath )
		StripPath ( tTokSettings.m_sSynonymsFile );

	StrVec_t dWarnings;
	TokenizerRefPtr_c pTokenizer = Tokenizer::Create ( tTokSettings, &tEmbeddedFiles, pFilenameBuilder, dWarnings, m_sLastError );
	if ( !pTokenizer )
		return LOAD_E::GeneralError_e;

	// dictionary stuff
	CSphDictSettings tDictSettings;
	tDictSettings.Load ( rdInfo, tEmbeddedFiles, pFilenameBuilder, sWarning );

	if ( bStripPath )
	{
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
			StripPath ( tDictSettings.m_dWordforms[i] );
	}

	DictRefPtr_c pDict { tDictSettings.m_bWordDict
		? sphCreateDictionaryKeywords ( tDictSettings, &tEmbeddedFiles, pTokenizer, GetName(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError )
		: sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, pTokenizer, GetName(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError )};

	if ( !pDict )
		return LOAD_E::GeneralError_e;

	if ( tDictSettings.m_sMorphFingerprint!=pDict->GetMorphDataFingerprint() )
		sWarning.SetSprintf ( "different lemmatizer dictionaries (table='%s', current='%s')",
			tDictSettings.m_sMorphFingerprint.cstr(),
			pDict->GetMorphDataFingerprint().cstr() );

	SetDictionary ( pDict );

	Tokenizer::AddToMultiformFilterTo ( pTokenizer, pDict->GetMultiWordforms () );
	SetTokenizer ( pTokenizer );
	SetupQueryTokenizer();

	// initialize AOT if needed
	m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );

	m_iDocinfo = rdInfo.GetOffset ();
	m_iDocinfoIndex = rdInfo.GetOffset ();
	m_iMinMaxIndex = rdInfo.GetOffset ();

	std::unique_ptr<ISphFieldFilter> pFieldFilter;
	CSphFieldFilterSettings tFieldFilterSettings;
	tFieldFilterSettings.Load(rdInfo);
	if ( tFieldFilterSettings.m_dRegexps.GetLength() )
		pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );

	if ( !sphSpawnFilterICU ( pFieldFilter, m_tSettings, tTokSettings, sHeaderName.cstr(), m_sLastError ) )
		return LOAD_E::GeneralError_e;

	SetFieldFilter ( std::move ( pFieldFilter ) );

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
		m_sLastError.SetSprintf ( "%s: failed to parse header (unexpected eof)", sHeaderName.cstr() );

	return rdInfo.GetErrorFlag() ? LOAD_E::GeneralError_e : LOAD_E::Ok_e;
}

CSphIndex_VLN::LOAD_E CSphIndex_VLN::LoadHeaderJson ( const CSphString& sHeaderName, bool bStripPath, CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder, CSphString & sWarning )
{
	using namespace bson;

	CSphVector<BYTE> dData;
	if ( !sphJsonParse ( dData, sHeaderName, m_sLastError ) )
		return LOAD_E::ParseError_e;

	Bson_c tBson ( dData );
	if ( tBson.IsEmpty() || !tBson.IsAssoc() )
	{
		m_sLastError = "Something wrong read from json header - it is either empty, either not root object.";
		return LOAD_E::ParseError_e;
	}

	// version
	m_uVersion = (DWORD)Int ( tBson.ChildByName ( "index_format_version" ) );
	if ( m_uVersion<=1 || m_uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%u, binary is v.%u", sHeaderName.cstr(), m_uVersion, INDEX_FORMAT_VERSION );
		return LOAD_E::GeneralError_e;
	}

	// we don't support anything prior to v54
	DWORD uMinFormatVer = 54;
	if ( m_uVersion<uMinFormatVer )
	{
		m_sLastError.SetSprintf ( "tables prior to v.%u are no longer supported (use index_converter tool); %s is v.%u", uMinFormatVer, sHeaderName.cstr(), m_uVersion );
		return LOAD_E::GeneralError_e;
	}

	// index stats
	m_tStats.m_iTotalDocuments = Int ( tBson.ChildByName ( "total_documents" ) );
	m_tStats.m_iTotalBytes = Int ( tBson.ChildByName ( "total_bytes" ) );

	// schema
	ReadSchemaJson ( tBson.ChildByName ( "schema" ), m_tSchema );

	// check schema for dupes
	for ( int iAttr = 0; iAttr < m_tSchema.GetAttrsCount(); ++iAttr )
	{
		const CSphColumnInfo& tCol = m_tSchema.GetAttr ( iAttr );
		for ( int i = 0; i < iAttr; ++i )
			if ( m_tSchema.GetAttr ( i ).m_sName == tCol.m_sName )
				sWarning.SetSprintf ( "duplicate attribute name: %s", tCol.m_sName.cstr() );
	}

	// index settings
	LoadIndexSettingsJson ( tBson.ChildByName ( "index_settings" ), m_tSettings );

	CSphTokenizerSettings tTokSettings;
	// tokenizer stuff
	if ( !tTokSettings.Load ( pFilenameBuilder, tBson.ChildByName ( "tokenizer_settings" ), tEmbeddedFiles, m_sLastError ) )
		return LOAD_E::GeneralError_e;

	// dictionary stuff
	CSphDictSettings tDictSettings;
	tDictSettings.Load ( tBson.ChildByName ( "dictionary_settings" ), tEmbeddedFiles, pFilenameBuilder, sWarning );

	// dictionary header (wordlist checkpoints, infix blocks, etc)
	m_tWordlist.m_iDictCheckpointsOffset = Int ( tBson.ChildByName ( "dict_checkpoints_offset" ) );
	m_tWordlist.m_iDictCheckpoints = (int)Int ( tBson.ChildByName ( "dict_checkpoints" ) );
	m_tWordlist.m_iInfixCodepointBytes = (int)Int ( tBson.ChildByName ( "infix_codepoint_bytes" ) );
	m_tWordlist.m_iInfixBlocksOffset = Int ( tBson.ChildByName ( "infix_blocks_offset" ) );
	m_tWordlist.m_iInfixBlocksWordsSize = (int)Int ( tBson.ChildByName ( "infix_block_words_size" ) );

	m_tWordlist.m_dCheckpoints.Reset ( m_tWordlist.m_iDictCheckpoints );

	if ( bStripPath )
	{
		StripPath ( tTokSettings.m_sSynonymsFile );
		for ( auto& i : tDictSettings.m_dWordforms )
			StripPath ( i );
	}

	StrVec_t dWarnings;
	TokenizerRefPtr_c pTokenizer = Tokenizer::Create ( tTokSettings, &tEmbeddedFiles, pFilenameBuilder, dWarnings, m_sLastError );
	if ( !pTokenizer )
		return LOAD_E::GeneralError_e;

	DictRefPtr_c pDict { tDictSettings.m_bWordDict
		? sphCreateDictionaryKeywords ( tDictSettings, &tEmbeddedFiles, pTokenizer, GetName(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError )
		: sphCreateDictionaryCRC ( tDictSettings, &tEmbeddedFiles, pTokenizer, GetName(), bStripPath, m_tSettings.m_iSkiplistBlockSize, pFilenameBuilder, m_sLastError )};

	if ( !pDict )
		return LOAD_E::GeneralError_e;

	if ( tDictSettings.m_sMorphFingerprint!=pDict->GetMorphDataFingerprint() )
		sWarning.SetSprintf ( "different lemmatizer dictionaries (table='%s', current='%s')",
			tDictSettings.m_sMorphFingerprint.cstr(),
			pDict->GetMorphDataFingerprint().cstr() );

	SetDictionary ( pDict );

	Tokenizer::AddToMultiformFilterTo ( pTokenizer, pDict->GetMultiWordforms () );
	SetTokenizer ( pTokenizer );
	SetupQueryTokenizer();

	// initialize AOT if needed
	m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tDictSettings.m_sMorphology.cstr() );

	m_iDocinfo = Int ( tBson.ChildByName ( "docinfo" ) );
	m_iDocinfoIndex = Int ( tBson.ChildByName ( "docinfo_index" ) );
	m_iMinMaxIndex = Int ( tBson.ChildByName ( "min_max_index" ) );

	std::unique_ptr<ISphFieldFilter> pFieldFilter;
	auto tFieldFilterSettingsNode = tBson.ChildByName ( "field_filter_settings" );
	if ( !IsNullNode(tFieldFilterSettingsNode) )
	{
		CSphFieldFilterSettings tFieldFilterSettings;
		Bson_c ( tFieldFilterSettingsNode ).ForEach ( [&tFieldFilterSettings] ( const NodeHandle_t& tNode ) {
			tFieldFilterSettings.m_dRegexps.Add ( String ( tNode ) );
		} );

		if ( !tFieldFilterSettings.m_dRegexps.IsEmpty() )
			pFieldFilter = sphCreateRegexpFilter ( tFieldFilterSettings, m_sLastError );
	}

	if ( !sphSpawnFilterICU ( pFieldFilter, m_tSettings, tTokSettings, sHeaderName.cstr(), m_sLastError ) )
		return LOAD_E::GeneralError_e;

	SetFieldFilter ( std::move ( pFieldFilter ) );

	auto tIndexFieldsLenNode = tBson.ChildByName ( "index_fields_lens" );
	if ( m_tSettings.m_bIndexFieldLens )
	{
		assert (!IsNullNode ( tIndexFieldsLenNode ));
		m_dFieldLens.Reset ( m_tSchema.GetFieldsCount() );
		int i = 0;
		Bson_c ( tIndexFieldsLenNode ).ForEach ( [&i,this] ( const NodeHandle_t& tNode ) {
			m_dFieldLens[i++] = Int ( tNode );
		} );
	}

	// post-load stuff.. for now, bigrams
	CSphIndexSettings & s = m_tSettings;
	if ( s.m_eBigramIndex!=SPH_BIGRAM_NONE && s.m_eBigramIndex!=SPH_BIGRAM_ALL )
	{
		BYTE * pTok;
		m_pTokenizer->SetBuffer ( (BYTE*)const_cast<char*> ( s.m_sBigramWords.cstr() ), s.m_sBigramWords.Length() );
		while ( ( pTok = m_pTokenizer->GetToken() )!=nullptr )
			s.m_dBigramWords.Add() = (const char*)pTok;
		s.m_dBigramWords.Sort();
	}

	return LOAD_E::Ok_e;
}


void CSphIndex_VLN::DebugDumpHeader ( FILE * fp, const CSphString& sHeaderName, bool bConfig )
{
	std::unique_ptr<FilenameBuilder_i> pFilenameBuilder;
	if ( GetIndexFilenameBuilder() )
		pFilenameBuilder = GetIndexFilenameBuilder() ( GetName() );

	CSphEmbeddedFiles tEmbeddedFiles;
	CSphString sWarning;
	auto eRes = LoadHeaderJson ( sHeaderName, false, tEmbeddedFiles, pFilenameBuilder.get(), sWarning );
	if ( eRes == LOAD_E::ParseError_e )
	{
		eRes = LoadHeaderLegacy ( sHeaderName, false, tEmbeddedFiles, pFilenameBuilder.get(), sWarning );
		if ( eRes == LOAD_E::ParseError_e )
			sphDie ( "failed to load header: %s", m_sLastError.cstr() );
	}
	if ( eRes == LOAD_E::GeneralError_e )
		sphDie ( "failed to load header: %s", m_sLastError.cstr() );

	assert ( eRes == LOAD_E::Ok_e );
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

		DumpSettingsCfg ( fp, *this, pFilenameBuilder.get() );

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

	DumpReadable ( fp, *this, tEmbeddedFiles, pFilenameBuilder.get() );

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

	const DWORD * pDocinfo = m_tAttr.GetReadPtr();
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
		NewProxyReader ( GetFilename ( SPH_EXT_SPD ), m_sLastError, DataReaderFactory_c::DOCS,
			m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE )
	};
	if ( !pDoclist )
		sphDie ( "failed to open doclist: %s", m_sLastError.cstr() );

	DataReaderFactoryPtr_c pHitlist {
		NewProxyReader ( GetFilename ( SPH_EXT_SPP ), m_sLastError, DataReaderFactory_c::HITS,
			m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE )
	};
	if ( !pHitlist )
		sphDie ( "failed to open hitlist: %s", m_sLastError.cstr ());


	// aim
	DiskIndexQwordSetup_c tTermSetup ( pDoclist, pHitlist, m_tSkiplists.GetReadPtr(), m_tSettings.m_iSkiplistBlockSize, true, RowID_t(m_iDocinfo) );
	tTermSetup.SetDict ( m_pDict );
	tTermSetup.m_pIndex = this;

	Qword tKeyword ( false, false, m_iIndexId );
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


void CSphIndex_VLN::DebugDumpDict ( FILE * fp, bool bDumpOnly )
{
	if ( !m_pDict->GetSettings().m_bWordDict )
	{
		sphDie ( "DebugDumpDict() only supports dict=keywords for now" );
	}

	if ( !bDumpOnly )
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
		m_pFile = NewProxyReader ( GetFilename ( eExt ), m_sLastError, eKind, iBuffer, eAccess );
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
	if ( !sphIsReadable ( GetFilename ( SPH_EXT_SPI ), &m_sLastError ) )
		return false;

	assert ( m_pDict );
	bool bWordDict = m_pDict->GetSettings().m_bWordDict;

	// only checkpoint and wordlist infixes are actually read here; dictionary itself is just mapped
	if ( !m_tWordlist.Preread ( GetFilename ( SPH_EXT_SPI ), bWordDict, m_tSettings.m_iSkiplistBlockSize, m_sLastError ) )
		return false;

	if ( ( m_tWordlist.m_tBuf.GetLengthBytes()<=18 )!=( m_tWordlist.m_dCheckpoints.GetLength()==0 ) )
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

	if ( !m_tAttr.Setup ( GetFilename ( SPH_EXT_SPA ), m_sLastError, true ) )
		return false;

	if ( !CheckDocsCount ( m_iDocinfo, m_sLastError ) )
		return false;

	m_pDocinfoIndex = m_tAttr.GetWritePtr() + m_iMinMaxIndex;

	if ( m_tSchema.GetAttr ( sphGetBlobLocatorName() ) )
	{
		if ( !m_tBlobAttrs.Setup ( GetFilename ( SPH_EXT_SPB ), m_sLastError, true ) )
			return false;
	}

	return true;
}


bool CSphIndex_VLN::PreallocDocidLookup()
{
	if ( m_bIsEmpty || m_bDebugCheck )
		return true;

	if ( !m_tDocidLookup.Setup ( GetFilename ( SPH_EXT_SPT ), m_sLastError, false ) )
		return false;

	m_tLookupReader.SetData ( m_tDocidLookup.GetReadPtr() );

	return true;
}


bool CSphIndex_VLN::PreallocKilllist()
{
	if ( m_bDebugCheck )
		return true;

	return m_tDeadRowMap.Prealloc ( (DWORD)m_iDocinfo, GetFilename ( SPH_EXT_SPM ), m_sLastError );
}


bool CSphIndex_VLN::PreallocHistograms ( StrVec_t & dWarnings )
{
	if ( m_bDebugCheck )
		return true;

	// we have histograms since v.56, but in v.61 we switched to streamed histograms with no backward compatibility
	if ( m_uVersion<61 )
		return true;

	CSphString sHistogramFile = GetFilename ( SPH_EXT_SPHI );
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

	if ( !m_tSchema.HasStoredFields() && !m_tSchema.HasStoredAttrs() )
		return true;

	m_pDocstore = CreateDocstore ( m_iIndexId, GetFilename ( SPH_EXT_SPDS ), m_sLastError );

	return !!m_pDocstore;
}


bool CSphIndex_VLN::PreallocColumnar()
{
	if ( m_uVersion<61 )
		return true;

	if ( !m_tSchema.HasColumnarAttrs() )
		return true;

	m_pColumnar = CreateColumnarStorageReader ( GetFilename ( SPH_EXT_SPC ), (DWORD)m_iDocinfo, m_sLastError );
	return !!m_pColumnar;
}


bool CSphIndex_VLN::PreallocKNN()
{
	if ( m_uVersion<65 )
		return true;

	if ( !m_tSchema.HasKNNAttrs() )
		return true;

	m_pKNN = CreateKNN(m_sLastError);
	if ( !m_pKNN )
		return false;

	std::string sErrorSTL;
	if ( !m_pKNN->Load ( GetFilename ( SPH_EXT_SPKNN ).cstr(), sErrorSTL ) )
	{
		m_sLastError = sErrorSTL.c_str();
		return false;
	}

	return !!m_pKNN;
}


bool CSphIndex_VLN::PreallocSkiplist()
{
	if ( m_bDebugCheck )
		return true;

	return m_tSkiplists.Setup ( GetFilename ( SPH_EXT_SPE ), m_sLastError, false );
}


bool CSphIndex_VLN::LoadSecondaryIndex ( const CSphString & sFile )
{
	if ( !sphFileExists ( sFile.cstr() ) )
	{
		if ( GetSecondaryIndexDefault()!=SIDefault_e::DISABLED )
		{
			if ( GetSecondaryIndexDefault()==SIDefault_e::FORCE )
				m_sLastError.SetSprintf ( "missing secondary index %s", sFile.cstr() );
			else
				sphWarning ( "missing %s; secondary index(es) disabled, consider using ALTER REBUILD SECONDARY to recover the secondary index", sFile.cstr() );
		}

		return GetSecondaryIndexDefault()!=SIDefault_e::FORCE;
	}

	if ( !m_tSI.Load ( sFile, m_sLastError ) && GetSecondaryIndexDefault()!=SIDefault_e::DISABLED )
	{
		if ( GetSecondaryIndexDefault()!=SIDefault_e::FORCE )
		{
			sphWarning ( "'%s': secondary index not loaded, %s; secondary index(es) disabled, consider using ALTER REBUILD SECONDARY to recover the secondary index", GetName(), m_sLastError.cstr() );
			m_sLastError = "";
		}

		if ( GetSecondaryIndexDefault()==SIDefault_e::FORCE )
			return false;
	}

	return true;
}


bool CSphIndex_VLN::PreallocSecondaryIndex()
{
	if ( m_uVersion<61 )
		return true;

	if ( !IsSecondaryLibLoaded() )
	{
		if ( GetSecondaryIndexDefault()!=SIDefault_e::DISABLED )
		{
			if ( GetSecondaryIndexDefault()==SIDefault_e::FORCE )
				m_sLastError = "secondary library not loaded";
			else
				sphWarning ( "secondary library not loaded; secondary index(es) disabled" );
		}
		return ( GetSecondaryIndexDefault()!=SIDefault_e::FORCE );
	}

	if ( !LoadSecondaryIndex ( GetFilename(SPH_EXT_SPIDX) ) )
		return false;

	if ( m_tSchema.HasJsonSIAttrs() && !LoadSecondaryIndex ( GetFilename(SPH_EXT_SPJIDX) ) )
		return false;

	return true;
}

bool CSphIndex_VLN::Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings )
{
	MEMORY ( MEM_INDEX_DISK );

	Dealloc();

	CSphEmbeddedFiles tEmbeddedFiles;

	// preload schema
	auto eRes = LoadHeaderJson ( GetFilename ( SPH_EXT_SPH ), bStripPath, tEmbeddedFiles, pFilenameBuilder, m_sLastWarning ) ;
	if ( eRes == LOAD_E::ParseError_e )
	{
		sphInfo ( "Index header format is not json, will try it as binary..." );
		eRes = LoadHeaderLegacy ( GetFilename ( SPH_EXT_SPH ), bStripPath, tEmbeddedFiles, pFilenameBuilder, m_sLastWarning );
		if ( eRes == LOAD_E::ParseError_e )
		{
			sphWarning ( "Unable to parse header... Error %s", m_sLastError.cstr() );
			return false;
		}
	}
	if ( eRes == LOAD_E::GeneralError_e )
	{
		sphWarning ( "Unable to load header... Error %s", m_sLastError.cstr() );
		return false;
	}

	assert ( eRes == LOAD_E::Ok_e );

	m_bIsEmpty = !m_iDocinfo;

	tEmbeddedFiles.Reset();

	// verify that data files are readable
	for ( auto& eExt : { SPH_EXT_SPD, SPH_EXT_SPP, SPH_EXT_SPE } )
		if ( !sphIsReadable ( GetFilename ( eExt ), &m_sLastError ) )
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
	if ( !PreallocKNN() )			return false;
	if ( !PreallocSkiplist() )		return false;
	if ( !PreallocSecondaryIndex() ) return false;

	// almost done
	m_bPassedAlloc = true;
	return true;
}


void CSphIndex_VLN::Preread()
{
	MEMORY ( MEM_INDEX_DISK );

	sphLogDebug ( "CSphIndex_VLN::Preread invoked '%s'(%s)", GetName(), GetFilebase() );

	assert ( m_bPassedAlloc );
	if ( m_bPassedRead )
		return;

	///////////////////
	// read everything
	///////////////////

	PrereadMapping ( GetName(), "attributes", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eAttr ), m_tAttr );
	if ( sphInterrupted() ) return;

	PrereadMapping ( GetName(), "blobs", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eBlob ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eBlob ), m_tBlobAttrs );
	if ( sphInterrupted() ) return;

	PrereadMapping ( GetName(), "skip-list", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), false, m_tSkiplists );
	if ( sphInterrupted() ) return;

	PrereadMapping ( GetName(), "dictionary", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eDict ), IsOndisk ( m_tMutableSettings.m_tFileAccess.m_eDict ), m_tWordlist.m_tBuf );
	if ( sphInterrupted() ) return;

	PrereadMapping ( GetName(), "docid-lookup", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ), false, m_tDocidLookup );
	if ( sphInterrupted() ) return;

	m_tDeadRowMap.Preread ( GetName(), "kill-list", IsMlock ( m_tMutableSettings.m_tFileAccess.m_eAttr ) );
	if ( sphInterrupted() ) return;

	m_bPassedRead = true;
	sphLogDebug ( "Preread successfully finished" );
}


CSphIndex::RenameResult_e CSphIndex_VLN::RenameEx ( CSphString sNewBase )
{
	if ( sNewBase == GetFilebase() )
		return RE_OK;

	IndexFiles_c dFiles ( GetFilebase(), nullptr, m_uVersion );
	if ( !dFiles.TryRenameBase ( sNewBase ) )
	{
		m_sLastError = dFiles.ErrorMsg ();
		return dFiles.IsFatal() ? RE_FATAL : RE_FAIL;
	}

	if ( !dFiles.RenameLock ( sNewBase, m_iLockFD ) )
	{
		m_sLastError = dFiles.ErrorMsg ();
		return dFiles.IsFatal() ? RE_FATAL : RE_FAIL;
	}

	SetFilebase ( std::move ( sNewBase ) );

	return RE_OK;
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
		::SetupStarDictV8 ( pDict, m_tSettings.m_iMinInfixLen>0 );

	// FIXME? might wanna verify somehow that the tokenizer has '*' as a character
}

void CSphIndex_VLN::SetupExactDict ( DictRefPtr_c &pDict ) const
{
	if ( m_tSettings.m_bIndexExactWords )
		::SetupExactDict ( pDict );
}


bool CSphIndex_VLN::GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const
{
	WITH_QWORD ( this, false, Qword, return DoGetKeywords<Qword> ( dKeywords, szQuery, tSettings, false, pError ) );
	return false;
}

void CSphIndex_VLN::GetSuggest ( const SuggestArgs_t & tArgs, SuggestResult_t & tRes ) const
{
	if ( m_tWordlist.m_tBuf.IsEmpty() || !m_tWordlist.m_dCheckpoints.GetLength() )
		return;

	assert ( !tRes.m_pWordReader );
	tRes.m_pWordReader = new KeywordsBlockReader_c ( m_tWordlist.m_tBuf.GetReadPtr(), m_tSettings.m_iSkiplistBlockSize );
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
		snprintf ( buf_all, 19, "lemmatize_%s_all", AOT_LANGUAGES[j] ); // NOLINT
		buf_all[19] = '\0';
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


template < class Qword >
bool CSphIndex_VLN::DoGetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, bool bFillOnly, CSphString * pError ) const
{
	if ( !bFillOnly )
		dKeywords.Resize ( 0 );

	if ( !m_bPassedAlloc )
	{
		if ( pError )
			*pError = "table not preread";
		return false;
	}

	// short-cut if no query or keywords to fill
	if ( ( bFillOnly && !dKeywords.GetLength() ) || ( !bFillOnly && ( !szQuery || !szQuery[0] ) ) )
		return true;

	DictRefPtr_c pDict = GetStatelessDict ( m_pDict );
	SetupStarDict ( pDict );
	SetupExactDict ( pDict );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (const BYTE *)szQuery;
	if ( m_pFieldFilter && szQuery && m_pFieldFilter->Clone()->Apply ( sModifiedQuery, dFiltered, true ) )
		sModifiedQuery = dFiltered.Begin();

	// FIXME!!! missed bigram, add flags to fold blended parts, show expanded terms

	// prepare for setup
	DiskIndexQwordSetup_c tTermSetup ( DataReaderFactoryPtr_c{}, DataReaderFactoryPtr_c{}, m_tSkiplists.GetReadPtr(), m_tSettings.m_iSkiplistBlockSize, false, RowID_t(m_iDocinfo) );
	tTermSetup.SetDict ( pDict );
	tTermSetup.m_pIndex = this;

	Qword tQueryWord ( false, false, m_iIndexId );

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

	TokenizerRefPtr_c pTokenizer = m_pQueryTokenizer->Clone ( SPH_CLONE );
	pTokenizer->EnableTokenizedMultiformTracking ();

	// need to support '*' and '=' but not the other specials
	// so m_pQueryTokenizer does not work for us, gotta clone and setup one manually
	if ( IsStarDict ( bWordDict ) )
		pTokenizer->AddPlainChars ( "*" );
	if ( m_tSettings.m_bIndexExactWords )
		pTokenizer->AddSpecials ( "=" );

	if ( !m_tSettings.m_sIndexTokenFilter.IsEmpty() )
	{
		CSphString sError;
		Tokenizer::AddPluginFilterTo ( pTokenizer, m_tSettings.m_sIndexTokenFilter, sError );
		if ( !sError.IsEmpty() )
		{
			if ( pError )
				*pError = sError;
			return false;
		}
		if ( !pTokenizer->SetFilterSchema ( m_tSchema, sError ) )
		{
			if ( pError )
				*pError = sError;
			return false;
		}
	}

	pTokenizer->SetBuffer ( sModifiedQuery, (int)strlen ( (const char*)sModifiedQuery ) );

	ExpansionContext_t tExpCtx;
	// query defined options
	tExpCtx.m_iExpansionLimit = ( tSettings.m_iExpansionLimit ? tSettings.m_iExpansionLimit : m_iExpansionLimit );
	bool bExpandWildcards = ( bWordDict && IsStarDict ( bWordDict ) && !tSettings.m_bFoldWildcards );

	CSphPlainQueryFilter tAotFilter;
	tAotFilter.m_pTokenizer = std::move ( pTokenizer );
	tAotFilter.m_pDict = std::move ( pDict );
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
	tExpCtx.m_iCutoff = tSettings.m_iCutoff;
	tExpCtx.m_bAllowExpansion = tSettings.m_bAllowExpansion;

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


static int sphQueryHeightCalc ( const XQNode_t * pNode )
{
	if ( pNode->m_dChildren.IsEmpty() )
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
#if defined( __clang__ )
#if defined( __x86_64__ )
static int SPH_EXTNODE_STACK_SIZE = 0x140;
#else // if defined ( __ARM_ARCH_ISA_A64 ) and all the others
static int SPH_EXTNODE_STACK_SIZE = 0x160;
#endif
#elif defined( _WIN32 )
static int SPH_EXTNODE_STACK_SIZE = 630;
#else
static int SPH_EXTNODE_STACK_SIZE = 160;
#endif

// extra stack which need despite EXTNODE_STACK_SIZE
static DWORD SPH_EXTRA_BUDGET = 0x2000;

void SetExtNodeStackSize ( int iDelta, int iExtra )
{
	if ( iDelta )
	{
		SPH_EXTNODE_STACK_SIZE = iDelta;

#if defined( _WIN32 )
		SPH_EXTNODE_STACK_SIZE += 0x80;
#endif
	}

	if ( iExtra )
		SPH_EXTRA_BUDGET = iExtra;// + 0x100;
}

/*
Why EXTRA_BUDGET?

CREATE TABLE if not exists t ( id bigint, f text );
replace into t (id,f) values (1, 'a b');
flush rtindex t;

crash:
SELECT * FROM t WHERE MATCH('(a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a a b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b b "a b") | ( a -b )');
WARNING: Stack used 42409, need 88000, sum 130409, have 131072
(data got on debug build with clang 14 on linux)

Strictly speaking, we need to mock it - create rt, then query - without any disk footprint.
 You see, that having 131072 is not enough to process query which needs 130409. Tree size is right, but extra space is need - first, to move inside query evaluation,
 and also on query leaves - reading docs/hits from disk with extra functions for caching, working with fs, profiling, etc. That is oneshot extra budget over calculated expression stuff.
*/


int ConsiderStackAbsolute ( const struct XQNode_t* pRoot )
{
	int iHeight = 0;
	if ( pRoot )
		iHeight = sphQueryHeightCalc ( pRoot );

	return iHeight * SPH_EXTNODE_STACK_SIZE + SPH_EXTRA_BUDGET;
}

int ConsiderStack ( const struct XQNode_t * pRoot, CSphString & sError )
{
	auto iStackNeed = ConsiderStackAbsolute ( pRoot );
	int64_t iQueryStack = Threads::GetStackUsed() + iStackNeed;
//	sphWarning ( "Stack used %d, need %d (%d * %d + %d), sum %d, have %d", (int)Threads::GetStackUsed(), iStackNeed, iHeight, SPH_EXTNODE_STACK_SIZE, SPH_EXTRA_BUDGET, (int)iQueryStack, Threads::MyStackSize() );
	auto iMyStackSize = Threads::MyStackSize ();
	if ( iMyStackSize>=iQueryStack )
		return -1;

	// align as stack of tree + 8K
	// (being run in new coro, most probably you'll start near the top of stack, so 32k should be enough)
	iQueryStack = iStackNeed + 8*1024;
	if ( session::GetMaxStackSize()>=iQueryStack )
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
		if ( !pNode->m_dWords[0].m_sWord.Begins( "=" ) )
			pNode->m_dWords[0].m_sWord.SetSprintf ( "=%s", pNode->m_dWords[0].m_sWord.cstr() );
		return pNode;
	}

	// do not expand into wildcard words shorter than min_prefix_len or min_infix_len
	bool bExpandInfix = false;
	bool bExpandPrefix = false;
	if ( ( iExpandKeywords & KWE_STAR )==KWE_STAR )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		int iLen = sphUTF8Len ( pNode->m_dWords[0].m_sWord.cstr() );

		int iMinInfix = tSettings.m_iMinInfixLen;
		int iMinPrefix = tSettings.RawMinPrefixLen();

		if ( iMinInfix>0 && iLen>=iMinInfix )
			bExpandInfix = true;
		else if ( iMinPrefix>0 && iLen>=iMinPrefix )
			bExpandPrefix = true;
	}
	bool bExpandExact = ( tSettings.m_bIndexExactWords && ( iExpandKeywords & KWE_EXACT )==KWE_EXACT );
	if ( !bExpandInfix && !bExpandPrefix && !bExpandExact )
		return pNode;

	XQNode_t * pExpand = new XQNode_t ( pNode->m_dSpec );
	pExpand->SetOp ( SPH_QUERY_OR, pNode );

	if ( bExpandInfix )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pInfix = CloneKeyword ( pNode );
		pInfix->m_dWords[0].m_sWord.SetSprintf ( "*%s*", pNode->m_dWords[0].m_sWord.cstr() );
		pInfix->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pInfix );
	} else if ( bExpandPrefix )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pPrefix = CloneKeyword ( pNode );
		pPrefix->m_dWords[0].m_sWord.SetSprintf ( "%s*", pNode->m_dWords[0].m_sWord.cstr() );
		pPrefix->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pPrefix );
	}

	if ( bExpandExact )
	{
		assert ( pNode->m_dChildren.GetLength()==0 );
		assert ( pNode->m_dWords.GetLength()==1 );
		XQNode_t * pExact = CloneKeyword ( pNode );
		if ( pNode->m_dWords[0].m_sWord.Begins( "=" ) )
		{
			pExact->m_dWords[0].m_sWord = pNode->m_dWords[0].m_sWord;
		} else
		{
			pExact->m_dWords[0].m_sWord.SetSprintf ( "=%s", pNode->m_dWords[0].m_sWord.cstr() );
		}
		pExact->m_pParent = pExpand;
		pExpand->m_dChildren.Add ( pExact );
	}

	return pExpand;
}

static bool SkipExpand ( const CSphString & sWord )
{
	return ( sWord.Begins("=") || sWord.Begins("*") || sWord.Ends("*") );
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

		// should skip expansion if all terms have modifiers
		if ( pNode->m_dWords.all_of ( [] ( const XQKeyword_t & tWord ) { return SkipExpand ( tWord.m_sWord ); } ) )
			return;

		ARRAY_FOREACH ( i, pNode->m_dWords )
		{
			auto * pWord = new XQNode_t ( pNode->m_dSpec );
			pWord->m_dWords.Add ( pNode->m_dWords[i] );

			// should not expand if word already has any modifiers
			if ( SkipExpand ( pWord->m_dWords[0].m_sWord ) )
				pNode->m_dChildren.Add ( pWord );
			else
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

	if ( SkipExpand ( pNode->m_dWords[0].m_sWord ) )
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

static void BuildExpandedTree ( const XQKeyword_t & tRootWord, const ISphWordlist::Args_t & dWordSrc, XQNode_t * pRoot )
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

static XQNode_t * ExpandXQNode ( const ExpansionContext_t & tCtx, ISphWordlist::Args_t & tArgs, XQNode_t * pNode );

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
	const bool bRegex = ( pNode->m_dWords[0].m_bRegex );

	// no wildcards, or just wildcards? do not expand
	if ( !( bRegex || sphHasExpandableWildcards ( sFull ) ) )
		return pNode;

	// regex builds tree in batch mode
	if ( bRegex )
	{
		tCtx.m_dRegexTerms.Add ( std::make_pair ( sFull, pNode ) );
		return pNode;
	}

	bool bUseTermMerge = ( tCtx.m_bMergeSingles && pNode->m_dSpec.m_dZones.IsEmpty() );
	ISphWordlist::Args_t tArgs ( bUseTermMerge, tCtx );

	if ( !sphExpandGetWords ( sFull, tCtx, tArgs ) )
	{
		tCtx.m_pResult->m_sWarning.SetSprintf ( "Query word length is less than min %s length. word: '%s' ", ( tCtx.m_iMinInfixLen>0 ? "infix" : "prefix" ), sFull );
		return pNode;
	}

	return ExpandXQNode ( tCtx, tArgs, pNode );
}

XQNode_t * ExpandXQNode ( const ExpansionContext_t & tCtx, ISphWordlist::Args_t & tArgs, XQNode_t * pNode )
{
	// no real expansions?
	// mark source word as expanded to prevent warning on terms mismatch in statistics
	if ( !tArgs.m_dExpanded.GetLength() && !tArgs.m_pPayload )
	{
		tCtx.m_pResult->AddStat ( pNode->m_dWords.Begin()->m_sWord, 0, 0 );
		pNode->m_dWords.Begin()->m_bExpanded = true;
		return pNode;
	}

	// copy the original word (iirc it might get overwritten),
	const XQKeyword_t tRootWord = pNode->m_dWords[0];
	tCtx.m_pResult->AddStat ( tRootWord.m_sWord, tArgs.m_iTotalDocs, tArgs.m_iTotalHits );

	// and build a binary tree of all the expansions
	if ( tArgs.m_dExpanded.GetLength() )
	{
		BuildExpandedTree ( tRootWord, tArgs, pNode );
	}

	if ( tArgs.m_pPayload )
	{
		ISphSubstringPayload * pPayload = tArgs.m_pPayload.release();
		tCtx.m_pPayloads->Add( pPayload );

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

bool ExpandRegex ( ExpansionContext_t & tCtx, CSphString & sError )
{
	if ( !tCtx.m_dRegexTerms.GetLength() )
		return true;

	if ( tCtx.m_dRegexTerms.GetLength() )
	{
#if !WITH_RE2
		sError.SetSprintf ( "REGEXP full-text operator used but no regexp support compiled" );
		return false;
#endif
	}

	CSphFixedVector<std::unique_ptr < DictTerm2Expanded_i > > dConverters ( tCtx.m_dRegexTerms.GetLength() );
	ISphWordlist::Args_t tArgs ( true, tCtx );

	assert ( tCtx.m_pWordlist );
	tCtx.m_pWordlist->ScanRegexWords ( tCtx.m_dRegexTerms, tArgs, dConverters );
	assert ( !dConverters.GetLength() || ( tCtx.m_dRegexTerms.GetLength()==dConverters.GetLength() ) );

	ARRAY_FOREACH ( i, dConverters )
	{
		if ( !dConverters[i] )
			continue;

		ISphWordlist::Args_t tArgs ( true, tCtx );
		dConverters[i]->Convert ( tArgs );

		ExpandXQNode ( tCtx, tArgs, tCtx.m_dRegexTerms[i].second );
	}

	return true;
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

XQNode_t * CSphIndex_VLN::ExpandPrefix ( XQNode_t * pNode, CSphQueryResultMeta & tMeta, CSphScopedPayload * pPayloads, DWORD uQueryDebugFlags, int iQueryExpansionLimit ) const
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
	tCtx.m_iExpansionLimit = GetExpansionLimit ( iQueryExpansionLimit, m_iExpansionLimit );
	tCtx.m_bHasExactForms = ( m_pDict->HasMorphology() || m_tSettings.m_bIndexExactWords );
	tCtx.m_bMergeSingles = ( uQueryDebugFlags & QUERY_DEBUG_NO_PAYLOAD )==0;
	tCtx.m_pPayloads = pPayloads;
	tCtx.m_eHitless = m_tSettings.m_eHitless;

	pNode = sphExpandXQNode ( pNode, tCtx );
	if ( !ExpandRegex ( tCtx, tMeta.m_sError ) )
		return nullptr;

	pNode->Check ( true );
	tCtx.AggregateStats();

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
template<typename RUN>
static bool RunSplitQuery ( RUN && tRun, const CSphQuery & tQuery, CSphQueryResultMeta & tResult, VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs, QueryProfile_c * pProfiler, const SmallStringHash_T<int64_t> * pLocalDocs, int64_t iTotalDocs, const char * szIndexName, int iSplit, int64_t tmMaxTimer )
{
	assert ( !dSorters.IsEmpty () );

	// counter of tasks we will issue now
	int iJobs = iSplit;
	assert ( iJobs>=1 );

	// pseudo-sharding scheduler
	auto tDispatch = GetEffectivePseudoShardingDispatcherTemplate();
	Dispatcher::Unify ( tDispatch, tQuery.m_tPseudoShardingDispatcher );

	// the context
	Threads::ClonableCtx_T<DiskChunkSearcherCtx_t, DiskChunkSearcherCloneCtx_t, Threads::ECONTEXT::ORDERED> tClonableCtx { dSorters, tResult };
	auto pDispatcher = Dispatcher::Make ( iJobs, 0, tDispatch, tClonableCtx.IsSingle() );
	tClonableCtx.LimitConcurrency ( pDispatcher->GetConcurrency() );

	auto iStart = sphMicroTimer();
	QUERYINFO << " Started: " << ( sphMicroTimer()-iStart ) << " index:" << szIndexName;

	// because disk chunk search within the loop will switch the profiler state
	SwitchProfile ( pProfiler, SPH_QSTATE_INIT );

	if ( pProfiler )
		pProfiler->m_iPseudoShards = iSplit;

	std::atomic<bool> bInterrupt {false};
	auto CheckInterrupt = [&bInterrupt]() { return bInterrupt.load ( std::memory_order_relaxed ); };

	int iConcurrency = tClonableCtx.Concurrency(iJobs);
	Threads::Coro::ExecuteN ( iConcurrency, [&]
	{
		auto pSource = pDispatcher->MakeSource();
		int iJob = -1; // make it consumed

		if ( !pSource->FetchTask ( iJob ) || CheckInterrupt() )
		{
			QUERYINFO << "Early finish parallel RunSplitQuery because of empty queue" << " index:" << szIndexName;
			return; // already nothing to do, early finish.
		}

		auto tJobContext = tClonableCtx.CloneNewContext ( !iJob );
		auto& tCtx = tJobContext.first;
		auto Interrupt = [&bInterrupt, &tCtx, &szIndexName] ( const char* szReason ) {
			tCtx.m_tMeta.m_sWarning = szReason;
			bInterrupt.store ( true, std::memory_order_relaxed );
			QUERYINFO << "RunSplitQuery interrupted " << szReason << " index:" << szIndexName;
		};
		QUERYINFO << "RunSplitQuery cloned context " << tJobContext.second << " index:" << szIndexName;
		tClonableCtx.SetJobOrder ( tJobContext.second, iJob );
		Threads::Coro::SetThrottlingPeriodMS ( session::GetThrottlingPeriodMS() );
		while ( !CheckInterrupt() ) // some earlier job met error; abort.
		{
			QUERYINFO << "RunSplitQuery " << tJobContext.second << ", job " << iJob << " index:" << szIndexName;
			myinfo::SetTaskInfo ( "%d ch %d:", Threads::Coro::NumOfRestarts(), iJob );
			auto & dLocalSorters = tCtx.m_dSorters;
			CSphQueryResultMeta tChunkMeta;
			CSphQueryResult tChunkResult;
			tChunkResult.m_pMeta = &tChunkMeta;
			CSphQueryResultMeta & tThMeta = tCtx.m_tMeta;
			tChunkMeta.m_pProfile = tThMeta.m_pProfile;

			CSphMultiQueryArgs tMultiArgs ( tArgs.m_iIndexWeight );
			tMultiArgs.m_iTag = tArgs.m_bModifySorterSchemas ? iJob+1 : tArgs.m_iTag;
			tMultiArgs.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;
			tMultiArgs.m_pLocalDocs = pLocalDocs;
			tMultiArgs.m_iTotalDocs = iTotalDocs;
			tMultiArgs.m_bModifySorterSchemas = false;
			tMultiArgs.m_iTotalThreads = iConcurrency;

			CSphQuery tQueryWithExtraFilter = tQuery;
			SetupSplitFilter ( tQueryWithExtraFilter.m_dFilters.Add(), iJob, iJobs );
			bool bRes = tRun ( tChunkResult, tQueryWithExtraFilter, dLocalSorters, tMultiArgs );
			bInterrupt.store ( !bRes, std::memory_order_relaxed );

			if ( !iJob )
				tThMeta.MergeWordStats ( tChunkMeta );

			tThMeta.m_bHasPrediction |= tChunkMeta.m_bHasPrediction;

			if ( tThMeta.m_bHasPrediction )
				tThMeta.m_tStats.Add ( tChunkMeta.m_tStats );

			if ( iJob < iJobs-1 && sph::TimeExceeded ( tmMaxTimer ) )
				Interrupt ( "query time exceeded max_query_time" );

			if ( tThMeta.m_sWarning.IsEmpty() && !tChunkMeta.m_sWarning.IsEmpty() )
				tThMeta.m_sWarning = tChunkMeta.m_sWarning;

			tThMeta.m_bTotalMatchesApprox |= tChunkMeta.m_bTotalMatchesApprox;
			tThMeta.m_tIteratorStats.Merge ( tChunkMeta.m_tIteratorStats );

			if ( CheckInterrupt() && !tChunkMeta.m_sError.IsEmpty() )
				// FIXME? maybe handle this more gracefully (convert to a warning)?
				tThMeta.m_sError = tChunkMeta.m_sError;

			iJob = -1; // mark it consumed
			if ( !pSource->FetchTask ( iJob ) || CheckInterrupt() )
				return; // all is done

			// yield and reschedule every quant of time. It gives work to other tasks
			if ( Threads::Coro::RuntimeExceeded() )
			{
				if ( session::GetKilled() )
					Interrupt ( "query was killed" );
				else
					Threads::Coro::RescheduleAndKeepCrashQuery();
			}
		}
	});
	QUERYINFO << "RunSplitQuery processed in " << tClonableCtx.NumWorked() << " thread(s)" << " index:" << szIndexName;
	tClonableCtx.Finalize();
	// can not fail query due to interruption or timeout
	// that is valid result set with just warning
	// parent sorters merge well in case of interruption or timeout
	return ( tResult.m_sError.IsEmpty() );
}

template<typename RUN>
bool CSphIndex_VLN::SplitQuery ( RUN && tRun, CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const
{
	auto & tMeta = *tResult.m_pMeta;
	QueryProfile_c * pProfile = tMeta.m_pProfile;

	CSphVector<ISphMatchSorter*> dSorters;
	dSorters.Reserve ( dAllSorters.GetLength() );
	dAllSorters.Apply ([&dSorters] ( ISphMatchSorter* p ) { if ( p ) dSorters.Add(p); });

	int iSplit = Max ( Min ( (int)m_tStats.m_iTotalDocuments, tArgs.m_iThreads ), 1 );
	int64_t iTotalDocs = tArgs.m_iTotalDocs ? tArgs.m_iTotalDocs : m_tStats.m_iTotalDocuments;
	bool bOk = RunSplitQuery ( tRun, tQuery, *tResult.m_pMeta, dSorters, tArgs, pProfile, tArgs.m_pLocalDocs, iTotalDocs, GetName(), iSplit, tmMaxTimer );

	tResult.m_pBlobPool = m_tBlobAttrs.GetReadPtr();
	tResult.m_pDocstore = m_pDocstore ? this : nullptr;
	tResult.m_pColumnar = m_pColumnar.get();

	PooledAttrsToPtrAttrs ( dSorters, m_tBlobAttrs.GetReadPtr(), m_pColumnar.get(), tArgs.m_bFinalizeSorters, pProfile, tArgs.m_bModifySorterSchemas );
	return bOk;
}


bool CSphIndex_VLN::RunParsedMultiQuery ( int iStackNeed, DictRefPtr_c & pDict, bool bCloneDict, const CSphQuery & tQuery, CSphQueryResult & tResult, VecTraits_T<ISphMatchSorter*> & dSorters, const XQQuery_t & tParsed, const CSphMultiQueryArgs & tArgs, int64_t tmMaxTimer ) const
{
	return Threads::Coro::ContinueBool ( iStackNeed, [&] {

		// we could reuse tParsed as-is, but marking common subtrees modifies the tree
		// so for now clone the tree before using
		const XQQuery_t * pTree = &tParsed;
		std::unique_ptr<XQQuery_t> pClonedTree;
		if ( bCloneDict )
		{
			pClonedTree = std::unique_ptr<XQQuery_t> ( CloneXQQuery(tParsed) );
			pTree = pClonedTree.get();
		}

		// flag common subtrees
		int iCommonSubtrees = 0;
		if ( m_iMaxCachedDocs && m_iMaxCachedHits )
			iCommonSubtrees = sphMarkCommonSubtrees ( 1, pTree );

		CSphQueryNodeCache tNodeCache ( iCommonSubtrees, m_iMaxCachedDocs, m_iMaxCachedHits );

		DictRefPtr_c pClonedDict;
		if ( bCloneDict )
		{
			pClonedDict = GetStatelessDict ( m_pDict );
			SetupStarDict ( pClonedDict );
			SetupExactDict ( pClonedDict );
		}

		bool bResult = ParsedMultiQuery ( tQuery, tResult, dSorters, *pTree, bCloneDict ? std::move (pClonedDict) : std::move (pDict), tArgs, &tNodeCache, tmMaxTimer );

		PooledAttrsToPtrAttrs ( dSorters, m_tBlobAttrs.GetReadPtr(), m_pColumnar.get(), tArgs.m_bFinalizeSorters, tResult.m_pMeta->m_pProfile, tArgs.m_bModifySorterSchemas );

		return bResult;
		});
}

/// one regular query vs many sorters (like facets, or similar for common-tree optimization)
bool CSphIndex_VLN::MultiQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dAllSorters, const CSphMultiQueryArgs & tArgs ) const
{
	auto & tMeta = *tResult.m_pMeta;
	QueryProfile_c * pProfile = tMeta.m_pProfile;

	int64_t	tmMaxTimer = 0;
	std::unique_ptr<MiniTimer_c> pTimerGuard;
	if ( tQuery.m_uMaxQueryMsec> 0 )
	{
		pTimerGuard = std::make_unique<MiniTimer_c>();
		tmMaxTimer = pTimerGuard->Engage ( tQuery.m_uMaxQueryMsec ); // max_query_time
	}

	const QueryParser_i * pQueryParser = tQuery.m_pQueryParser;
	assert ( pQueryParser );

	MEMORY ( MEM_DISK_QUERY );

	// to avoid the checking of a ppSorters's element for NULL on every next step, just filter out all nulls right here
	CSphVector<ISphMatchSorter*> dSorters;
	dSorters.Reserve ( dAllSorters.GetLength() );
	dAllSorters.Apply ([&dSorters] ( ISphMatchSorter* p) { if ( p ) dSorters.Add(p); });

	// if we have anything to work with
	if ( dSorters.IsEmpty () )
		return false;

	// non-random at the start, random at the end
	if ( dSorters.any_of ( []( ISphMatchSorter * p) { return p->IsRandom(); } ) )
		dSorters.Sort ( CmpPSortersByRandom_fn() );

	// fast path for scans
	if ( pQueryParser->IsFullscan ( tQuery ) )
	{
		if ( tArgs.m_iThreads>1 )
		{
			return SplitQuery (
				[this, &tmMaxTimer]
				( CSphQueryResult & tChunkResult, const CSphQuery & tQuery, VecTraits_T<ISphMatchSorter *> dLocalSorters, const CSphMultiQueryArgs & tMultiArgs )
				{ return MultiScan ( tChunkResult, tQuery, dLocalSorters, tMultiArgs, tmMaxTimer ); },
				tResult, tQuery, dAllSorters, tArgs, tmMaxTimer );
		} else
		{
			return MultiScan ( tResult, tQuery, dSorters, tArgs, tmMaxTimer );
		}
	}

	SwitchProfile ( pProfile, SPH_QSTATE_DICT_SETUP );

	DictRefPtr_c pDict = GetStatelessDict ( m_pDict );
	SetupStarDict ( pDict );
	SetupExactDict ( pDict );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (const BYTE *)tQuery.m_sQuery.cstr();

	if ( m_pFieldFilter && sModifiedQuery && m_pFieldFilter->Clone()->Apply ( sModifiedQuery, dFiltered, true ) )
		sModifiedQuery = dFiltered.Begin();

	// parse query
	SwitchProfile ( pProfile, SPH_QSTATE_PARSE );

	assert ( m_pQueryTokenizer.Ptr() && m_pQueryTokenizerJson.Ptr() );
	XQQuery_t tParsed;
	if ( !pQueryParser->ParseQuery ( tParsed, (const char*)sModifiedQuery, &tQuery, m_pQueryTokenizer, m_pQueryTokenizerJson, &m_tSchema, pDict, m_tSettings, &m_tMorphFields ) )
	{
		// FIXME? might wanna reset profile to unknown state
		tMeta.m_sError = tParsed.m_sParseError;
		return false;
	}

	// check again for fullscan
	if ( pQueryParser->IsFullscan ( tParsed ) )
	{
		if ( tArgs.m_iThreads>1 )
		{
			return SplitQuery (
				[this, &tmMaxTimer]
				( CSphQueryResult & tChunkResult, const CSphQuery & tQuery, VecTraits_T<ISphMatchSorter *> dLocalSorters, const CSphMultiQueryArgs & tMultiArgs )
				{ return MultiScan ( tChunkResult, tQuery, dLocalSorters, tMultiArgs, tmMaxTimer ); },
				tResult, tQuery, dAllSorters, tArgs, tmMaxTimer );
		} else
		{
			return MultiScan ( tResult, tQuery, dSorters, tArgs, tmMaxTimer );
		}
	}

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
	XQNode_t * pPrefixed = ExpandPrefix ( tParsed.m_pRoot, tMeta, &tPayloads, tQuery.m_uDebugFlags, tQuery.m_iExpansionLimit );
	if ( !pPrefixed )
		return false;

	tParsed.m_pRoot = pPrefixed;
	tParsed.m_bNeedSZlist = tQuery.m_bZSlist;

	int iStackNeed = ConsiderStack ( tParsed.m_pRoot, tMeta.m_sError );
	if ( !iStackNeed  )
		return false;

	if ( tArgs.m_iThreads>1 )
	{
		return SplitQuery (
			[this, iStackNeed, &pDict, &tParsed, &tmMaxTimer]
			( CSphQueryResult & tChunkResult, const CSphQuery & tQuery, VecTraits_T<ISphMatchSorter *> dLocalSorters, const CSphMultiQueryArgs & tMultiArgs )
			{ return RunParsedMultiQuery ( iStackNeed, pDict, true, tQuery, tChunkResult, dLocalSorters, tParsed, tMultiArgs, tmMaxTimer ); },
			tResult, tQuery, dAllSorters, tArgs, tmMaxTimer );
	} else
	{
		return RunParsedMultiQuery ( iStackNeed, pDict, false, tQuery, tResult, dSorters, tParsed, tArgs, tmMaxTimer );
	}
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

	DictRefPtr_c pDict = GetStatelessDict ( m_pDict );
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
			MiniTimer_c tTimerGuard;
			int64_t tmMaxTimer = tTimerGuard.Engage ( tCurQuery.m_uMaxQueryMsec ); // max_query_time

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

		if ( pQueryParser->ParseQuery ( dXQ[i], tCurQuery.m_sQuery.cstr(), &tCurQuery, m_pQueryTokenizer, m_pQueryTokenizerJson, &m_tSchema, pDict, m_tSettings, &m_tMorphFields ) )
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
			XQNode_t * pPrefixed = ExpandPrefix ( dXQ[i].m_pRoot, tMeta, &tPayloads, tCurQuery.m_uDebugFlags, tCurQuery.m_iExpansionLimit );
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
		Threads::Coro::Continue ( iStackNeed, [&]
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

			MiniTimer_c tTimerGuard;
			int64_t tmMaxTimer = tTimerGuard.Engage ( tCurQuery.m_uMaxQueryMsec ); // max_query_time

			if ( dXQ[j].m_pRoot && ppSorters[j] && ParsedMultiQuery ( tCurQuery, tCurResult, { ppSorters+j, 1 }, dXQ[j], pDict, tArgs, &tNodeCache, tmMaxTimer ) )
			{
				bResult = true;
				tMeta.m_iMultiplier = iCommonSubtrees ? iQueries : 1;
			} else
				tMeta.m_iMultiplier = -1;

			tMeta.m_tIOStats.Stop();
		}
	});

	PooledAttrsToPtrAttrs ( { ppSorters, iQueries }, m_tBlobAttrs.GetReadPtr(), m_pColumnar.get(), tArgs.m_bFinalizeSorters, pResults[0].m_pMeta->m_pProfile, tArgs.m_bModifySorterSchemas );

	return bResult || bResultScan;
}


bool CSphIndex_VLN::CheckEarlyReject ( const CSphVector<CSphFilterSettings> & dFilters, const ISphFilter * pFilter, ESphCollation eCollation, const ISphSchema & tSchema ) const
{
	if ( !pFilter || dFilters.IsEmpty() )
		return false;

	bool bHaveColumnar = false;

	if ( m_pColumnar )
	{
		// check .SPC file minmax
		CSphString sWarning;
		int iNonColumnar = 0;
		std::vector<common::Filter_t> dColumnarFilters;
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


static void SetupRowIdBoundaries ( const CSphVector<CSphFilterSettings> & dFilters, RowID_t uTotalDocs, ISphRanker & tRanker )
{
	const CSphFilterSettings * pRowIdFilter = nullptr;
	for ( auto & i : dFilters )
		if ( i.m_sAttrName=="@rowid" )
		{
			pRowIdFilter = &i;
			break;
		}

	if ( !pRowIdFilter )
		return;

	RowIdBoundaries_t tBoundaries = GetFilterRowIdBoundaries ( *pRowIdFilter, uTotalDocs );
	tRanker.ExtraData ( EXTRA_SET_BOUNDARIES, (void**)&tBoundaries );
}


bool CSphIndex_VLN::ParsedMultiQuery ( const CSphQuery & tQuery, CSphQueryResult & tResult, const VecTraits_T<ISphMatchSorter *> & dSorters, const XQQuery_t & tXQ, DictRefPtr_c pDict, const CSphMultiQueryArgs & tArgs, CSphQueryNodeCache * pNodeCache, int64_t tmMaxTimer ) const
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

	std::optional<ScopedLowPriority_c> tPrio;
	if ( tQuery.m_bLowPriority )
		tPrio.emplace();

	///////////////////
	// setup searching
	///////////////////

	// non-ready index, empty response!
	if ( !m_bPassedAlloc )
	{
		tMeta.m_sError = "table not preread";
		return false;
	}

	CSphQueryContext tCtx(tQuery);
	CreateFilterContext_t tFlx;
	const ISphSchema * pMaxSorterSchema = nullptr;
	CSphVector<CSphFilterSettings> dTransformedFilters; // holds filter settings if they were modified. filters hold pointers to those settings
	CSphVector<FilterTreeItem_t> dTransformedFilterTree;
	if ( !SetupFiltersAndContext ( tCtx, tFlx, tMeta, pMaxSorterSchema, dTransformedFilters, dTransformedFilterTree, dSorters, tArgs ) )
		return false;

	assert(pMaxSorterSchema);
	const ISphSchema & tMaxSorterSchema = *pMaxSorterSchema;

	// set blob pool for string on_sort expression fix up
	tCtx.SetBlobPool ( m_tBlobAttrs.GetReadPtr() );
	tCtx.m_uPackedFactorFlags = tArgs.m_uPackedFactorFlags;

	// open files
	DataReaderFactoryPtr_c pDoclist = m_pDoclistFile;
	DataReaderFactoryPtr_c pHitlist = m_pHitlistFile;

	if ( !pDoclist || !pHitlist )
		SwitchProfile ( pProfile, SPH_QSTATE_OPEN );

	if ( !pDoclist )
	{
		pDoclist = NewProxyReader ( GetFilename ( SPH_EXT_SPD ), tMeta.m_sError, DataReaderFactory_c::DOCS, m_tMutableSettings.m_tFileAccess.m_iReadBufferDocList, FileAccess_e::FILE );
		if ( !pDoclist )
			return false;
	}

	if ( !pHitlist )
	{
		pHitlist = NewProxyReader ( GetFilename ( SPH_EXT_SPP ), tMeta.m_sError, DataReaderFactory_c::HITS, m_tMutableSettings.m_tFileAccess.m_iReadBufferHitList, FileAccess_e::FILE );
		if ( !pHitlist )
			return false;
	}

	pDoclist->SetProfile ( pProfile );
	pHitlist->SetProfile ( pProfile );

	SwitchProfile ( pProfile, SPH_QSTATE_INIT );

	// setup search terms
	DiskIndexQwordSetup_c tTermSetup ( pDoclist, pHitlist, m_tSkiplists.GetReadPtr(), m_tSettings.m_iSkiplistBlockSize, true, RowID_t(m_iDocinfo) );

	tTermSetup.SetDict ( std::move ( pDict ) );
	tTermSetup.m_pIndex = this;
	tTermSetup.m_iDynamicRowitems = tMaxSorterSchema.GetDynamicSize();
	tTermSetup.m_iMaxTimer = tmMaxTimer;
	tTermSetup.m_pWarning = &tMeta.m_sWarning;
	tTermSetup.m_pCtx = &tCtx;
	tTermSetup.m_pNodeCache = pNodeCache;
	tTermSetup.m_bHasWideFields = ( m_tSchema.GetFieldsCount()>32 );
	tMeta.m_bBigram = ( m_tSettings.m_eBigramIndex!=SPH_BIGRAM_NONE );

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
	std::unique_ptr<ISphRanker> pRanker = sphCreateRanker ( tXQ, tQuery, tMeta, tTermSetup, tCtx, tMaxSorterSchema );
	if ( !pRanker )
		return false;

	if ( ( tArgs.m_uPackedFactorFlags & SPH_FACTOR_ENABLE ) && tQuery.m_eRanker!=SPH_RANK_EXPR )
		tMeta.m_sWarning.SetSprintf ( "packedfactors() and bm25f() requires using an expression ranker" );

	tCtx.SetupExtraData ( pRanker.get(), dSorters.GetLength()==1 ? dSorters[0] : nullptr );

	const BYTE * pBlobPool = m_tBlobAttrs.GetReadPtr();
	pRanker->ExtraData ( EXTRA_SET_BLOBPOOL, (void**)&pBlobPool );
	const columnar::Columnar_i * pColumnar = m_pColumnar.get();
	pRanker->ExtraData ( EXTRA_SET_COLUMNAR, (void**)&pColumnar );

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

	SetupRowIdBoundaries ( tQuery.m_dFilters, RowID_t(m_iDocinfo), *pRanker );

	if ( CheckEarlyReject ( dTransformedFilters, tCtx.m_pFilter.get(), tQuery.m_eCollation, tMaxSorterSchema ) )
	{
		tMeta.m_iQueryTime += (int)( ( sphMicroTimer()-tmQueryStart )/1000 );
		tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;
		return true;
	}

	for ( auto & i : dSorters )
	{
		i->SetBlobPool ( m_tBlobAttrs.GetReadPtr() );
		i->SetColumnar ( m_pColumnar.get() );
	}

	SwitchProfile ( pProfile, SPH_QSTATE_SETUP_ITER );

	int iCutoff = ApplyImplicitCutoff ( tQuery, dSorters, true );

	int iOldLen = tMeta.m_tIteratorStats.m_dIterators.GetLength();
	for ( auto & i : dSorters )
		i->AddDesc ( tMeta.m_tIteratorStats.m_dIterators );

	if ( tMeta.m_tIteratorStats.m_dIterators.GetLength()!=iOldLen )
		tMeta.m_tIteratorStats.m_iTotal = 1;

	CSphVector<CSphFilterSettings> dFiltersAfterIterator; // holds filter settings if they were modified. filters hold pointers to those settings
	std::pair<RowidIterator_i *, bool> tSpawned = SpawnIterators ( tQuery, dTransformedFilters, tCtx, tFlx, tMaxSorterSchema, tMeta, iCutoff, tArgs.m_iTotalThreads, dFiltersAfterIterator, pRanker.get() );
	std::unique_ptr<RowidIterator_i> pIterator = std::unique_ptr<RowidIterator_i> ( tSpawned.first );
	if ( tSpawned.second )
		return false;

	if ( pIterator )
	{
		auto pIter = pIterator.get();
		pRanker->ExtraData ( EXTRA_SET_ITERATOR, (void**)&pIter );
	}

	//////////////////////////////////////
	// find and weight matching documents
	//////////////////////////////////////

	bool bFinalPass = !!tCtx.m_dCalcFinal.GetLength();
	int iMyTag = bFinalPass ? -1 : tArgs.m_iTag;

	assert ( tQuery.m_eMode==SPH_MATCH_ALL || tQuery.m_eMode==SPH_MATCH_PHRASE || tQuery.m_eMode==SPH_MATCH_ANY || tQuery.m_eMode==SPH_MATCH_EXTENDED || tQuery.m_eMode==SPH_MATCH_EXTENDED2 || tQuery.m_eMode==SPH_MATCH_BOOLEAN  );

	bool bHaveRandom = false;
	dSorters.Apply ( [&bHaveRandom] ( const ISphMatchSorter * p ) { bHaveRandom |= p->IsRandom(); } );

	bool bUseFactors = !!( tCtx.m_uPackedFactorFlags & SPH_FACTOR_ENABLE );
	bool bUseKlist = m_tDeadRowMap.HasDead();
	bool bHasSortCalc = !tCtx.m_dCalcSort.IsEmpty();
	bool bHasWeightFilter = !!tCtx.m_pWeightFilter;
	bool bHasFilterCalc = !tCtx.m_dCalcFilter.IsEmpty();
	bool bHasCutoff = iCutoff!=-1;

	int iIndex = bUseKlist*64 + bHaveRandom*32 + bUseFactors*16 + bHasSortCalc*8 + bHasWeightFilter*4 + bHasFilterCalc*2 + bHasCutoff;

	switch ( iIndex )
	{
#define DECL_FNSCAN( _, n, params ) case n: MatchExtended<!!(n&64), !!(n&32), !!(n&16), !!(n&8), !!(n&4), !!(n&2), !!(n&1)> params; break;
	BOOST_PP_REPEAT ( 128, DECL_FNSCAN, ( tCtx, tQuery, dSorters, pRanker.get(), iMyTag, tArgs.m_iIndexWeight, iCutoff ) )
#undef DECL_FNSCAN
		default:
			assert ( 0 && "Internal error" );
			break;
	}

	////////////////////
	// cook result sets
	////////////////////

	SwitchProfile ( pProfile, SPH_QSTATE_FINALIZE );

	if ( pIterator )
	{
		pIterator->AddDesc ( tMeta.m_tIteratorStats.m_dIterators );
		tMeta.m_tIteratorStats.m_iTotal = 1;
	}

	if ( dSorters.any_of ( [&] ( ISphMatchSorter * p ) { return !p->FinalizeJoin ( tMeta.m_sError, tMeta.m_sWarning ); } ) )
		return false;

	// adjust result sets
	if ( bFinalPass )
	{
		// GotUDF means promise to UDFs that final-stage calls will be evaluated
		// a) over the final, pre-limit result set
		// b) in the final result set order
		bool bGotUDF = false;

		DocstoreSession_c tSession;
		int64_t iSessionUID = tSession.GetUID();

		// spawn buffered readers for the current session
		// put them to a global hash
		if ( m_pDocstore )
			m_pDocstore->CreateReader ( iSessionUID );

		DocstoreSession_c::InfoRowID_t tSessionInfo;
		tSessionInfo.m_pDocstore = m_pDocstore.get();
		tSessionInfo.m_iSessionId = iSessionUID;

		for ( auto & i : tCtx.m_dCalcFinal )
		{
			assert ( i.m_pExpr );
			if ( !bGotUDF )
				i.m_pExpr->Command ( SPH_EXPR_GET_UDF, &bGotUDF );

			if ( m_pDocstore )
				i.m_pExpr->Command ( SPH_EXPR_SET_DOCSTORE_ROWID, &tSessionInfo );
		}

		SphFinalMatchCalc_t tFinal ( tArgs.m_iTag, tCtx );
		dSorters.Apply ( [&] ( ISphMatchSorter * p ) { p->Finalize ( tFinal, bGotUDF, tArgs.m_bFinalizeSorters ); } );
	}

	pRanker->FinalizeCache ( tMaxSorterSchema );

	tResult.m_pBlobPool = m_tBlobAttrs.GetReadPtr();
	tResult.m_pDocstore = m_pDocstore ? this : nullptr;
	tResult.m_pColumnar = m_pColumnar.get();

	// query timer
	int64_t tmWall = sphMicroTimer() - tmQueryStart;
	tMeta.m_iQueryTime += (int)( tmWall/1000 );
	tMeta.m_iCpuTime += sphTaskCpuTimer ()-tmCpuQueryStart;

	QUERYINFO << GetName() << ": qtm " << (int)(tmWall) << ", " << tQueryStats.m_iFetchedDocs << ", " << tQueryStats.m_iFetchedHits << ", " << tQueryStats.m_iSkips << ", " << dSorters[0]->GetTotalCount();

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
	if ( !m_pDocstore )
		return;

	m_pDocstore->CreateReader(iSessionId);
}


bool CSphIndex_VLN::GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const
{
	if ( !m_pDocstore )
		return false;

	RowID_t tRowID = GetRowidByDocid ( tDocID );
	if ( tRowID==INVALID_ROWID || m_tDeadRowMap.IsSet(tRowID) )
		return false;

	tDoc = m_pDocstore->GetDoc ( tRowID, pFieldIds, iSessionId, bPack );
	return true;
}


int CSphIndex_VLN::GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const
{
	if ( !m_pDocstore )
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
	{
		if ( i.m_eExt==SPH_EXT_SPL )
			continue;

		CSphString sFile = GetFilename ( i.m_eExt );
		struct_stat st;
		if ( stat ( sFile.cstr(), &st )==0 )
			pRes->m_iDiskUse += st.st_size;
	}
//	sphWarning ( "Chunks: %d, RAM: %d, DISK: %d", pRes->m_iNumChunks, (int) pRes->m_iRamUse, (int) pRes->m_iMapped );
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


int CSphIndex_VLN::DebugCheck ( DebugCheckError_i & tReporter, FilenameBuilder_i * pFilenameBuilder )
{
	auto pIndexChecker = std::make_unique<DiskIndexChecker_c> ( *this, tReporter );

	pIndexChecker->Setup ( m_iDocinfo, m_iDocinfoIndex, m_iMinMaxIndex, m_bCheckIdDups );

	// check if index is ready
	if ( !m_bPassedAlloc )
		tReporter.Fail ( "table not preread" );

	if ( !pIndexChecker->OpenFiles() )
		return 1;

	if ( !LoadHitlessWords ( m_tSettings.m_sHitlessFiles, m_pTokenizer, m_pDict, pIndexChecker->GetHitlessWords(), m_sLastError ) )
		tReporter.Fail ( "unable to load hitless words: %s", m_sLastError.cstr() );

	CSphSavedFile tStat;
	CSphString sError;
	const CSphTokenizerSettings & tTokenizerSettings = m_pTokenizer->GetSettings ();
	if ( !tTokenizerSettings.m_sSynonymsFile.IsEmpty() )
	{
		CSphString sSynonymsFile = tTokenizerSettings.m_sSynonymsFile;
		if ( pFilenameBuilder )
			sSynonymsFile = pFilenameBuilder->GetFullPath ( sSynonymsFile );

		if ( !tStat.Collect ( sSynonymsFile.cstr(), &sError ) )
			tReporter.Fail ( "unable to open exceptions '%s': %s", sSynonymsFile.cstr(), sError.cstr() );
	}

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
		if ( pFilenameBuilder )
			sStopFile = pFilenameBuilder->GetFullPath ( sStopFile );

		if ( !tStat.Collect ( sStopFile.cstr(), &sError ) )
			tReporter.Fail ( "unable to open stopwords '%s': %s", sStopFile.cstr(), sError.cstr() );
	}

	if ( !tDictSettings.m_dWordforms.GetLength() )
	{
		ARRAY_FOREACH ( i, tDictSettings.m_dWordforms )
		{
			CSphString sWordforms = tDictSettings.m_dWordforms[i];
			if ( pFilenameBuilder )
				sWordforms = pFilenameBuilder->GetFullPath ( sWordforms );

			if ( !tStat.Collect ( sWordforms.cstr(), &sError ) )
				tReporter.Fail ( "unable to open wordforms '%s': %s", sWordforms.cstr(), sError.cstr() );
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
	if ( !tArgs.m_szQuery )
		return EmptyBson ();

	std::unique_ptr<QueryParser_i> pQueryParser ( sphCreatePlainQueryParser() );

	CSphVector<BYTE> dFiltered;
	const BYTE * sModifiedQuery = (const BYTE *)tArgs.m_szQuery;

	if ( tArgs.m_pFieldFilter && sModifiedQuery && tArgs.m_pFieldFilter->Apply ( sModifiedQuery, dFiltered, true ) )
		sModifiedQuery = dFiltered.Begin();

	CSphSchema tSchema;
	const CSphSchema * pSchema = tArgs.m_pSchema;
	if ( !pSchema )
	{
		pSchema = &tSchema;
		// need to fill up schema with fields from query
		AddFields ( tArgs.m_szQuery, tSchema );
	}

	XQQuery_t tParsed;
	if ( !pQueryParser->ParseQuery ( tParsed, (const char*)sModifiedQuery, nullptr, tArgs.m_pQueryTokenizer, nullptr, pSchema, tArgs.m_pDict, *tArgs.m_pSettings, tArgs.m_pMorphFields ) )
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
	ExplainQueryArgs_t tArgs;
	tArgs.m_szQuery = sQuery.cstr();
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
	tArgs.m_pMorphFields = &m_tMorphFields;

	return Explain ( tArgs );
}


bool CSphIndex_VLN::AlterSI ( CSphString & sError )
{
	if ( !IsSecondaryLibLoaded() )
	{
		sError = "secondary index library not loaded";
		return false;
	}

	MergeCb_c tMonitor;
	StrVec_t dCurFiles, dNewFiles;
	if ( !SiRecreate ( tMonitor, *this, m_iDocinfo, dCurFiles, dNewFiles, sError ) )
		return false;

	ARRAY_FOREACH ( i, dCurFiles )
	{
		StrVec_t dFilesFrom(1);
		StrVec_t dFilesTo(1);
		CSphString sFileOld;
		bool bCurExists = sphFileExists ( dCurFiles[i].cstr() );
		if ( bCurExists )
		{
			sFileOld.SetSprintf ( "%s.old", dCurFiles[i].cstr() );

			dFilesFrom[0] = dCurFiles[i];
			dFilesTo[0] = sFileOld;

			if ( !RenameWithRollback ( dFilesFrom, dFilesTo, sError ) )
				return false;

			if ( !m_tSI.Drop ( dCurFiles[i], sError ) )
				return false;
		}

		dFilesFrom[0] = dNewFiles[i];
		dFilesTo[0] = dCurFiles[i];

		if ( !RenameWithRollback ( dFilesFrom, dFilesTo, sError ) )
			return false;

		if ( !m_tSI.Load ( dCurFiles[i].cstr(), sError ) )
			return false;

		if ( bCurExists )
			::unlink ( sFileOld.cstr() );
	}

	return true;
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
	explicit CRtDictKeywords ( DictRefPtr_c pBase, bool bStoreID )
		: m_pBase ( std::move ( pBase ) )
		, m_bStoreID ( bStoreID )
	{
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

	void LoadStopwords ( const char * sFiles, FilenameBuilder_i * pFilenameBuilder, const TokenizerRefPtr_c& pTokenizer, bool bStripFile ) final { m_pBase->LoadStopwords ( sFiles, pFilenameBuilder, pTokenizer, bStripFile ); }
	void LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) final { m_pBase->LoadStopwords ( dStopwords ); }
	void WriteStopwords ( Writer_i & tWriter ) const final { m_pBase->WriteStopwords ( tWriter ); }
	void WriteStopwords ( JsonEscapedBuilder & tOut ) const final { m_pBase->WriteStopwords ( tOut ); }
	bool LoadWordforms ( const StrVec_t & dFiles, const CSphEmbeddedFiles * pEmbedded, const TokenizerRefPtr_c& pTokenizer, const char * szIndex ) final
		{ return m_pBase->LoadWordforms ( dFiles, pEmbedded, pTokenizer, szIndex ); }
	void WriteWordforms ( Writer_i & tWriter ) const final { m_pBase->WriteWordforms ( tWriter ); }
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

ISphRtDictWraperRefPtr_c sphCreateRtKeywordsDictionaryWrapper ( DictRefPtr_c pBase, bool bStoreID )
{
	return ISphRtDictWraperRefPtr_c { new CRtDictKeywords ( std::move ( pBase ), bStoreID ) };
}


//////////////////////////////////////////////////////////////////////////
// DICTIONARY FACTORIES
//////////////////////////////////////////////////////////////////////////

const CSphSourceStats& GetStubStats()
{
	static CSphSourceStats tDummy;
	return tDummy;
}

//////////////////////////////////////////////////////////////////////////

#if WITH_RE2
class CSphFieldRegExps : public ISphFieldFilter
{
public:
						CSphFieldRegExps () = default;
						CSphFieldRegExps ( const StrVec_t& m_dRegexps, CSphString &	sError );
						~CSphFieldRegExps() override;

	int					Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool ) final;
	void				GetSettings ( CSphFieldFilterSettings & tSettings ) const final;
	std::unique_ptr<ISphFieldFilter>	Clone() const final;

	void				AddRegExp ( const char * sRegExp, StringBuilder_c & sErrors );

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
	StringBuilder_c sErrors (", ");
	for ( const auto &sRegexp : dRegexps )
		AddRegExp ( sRegexp.cstr (), sErrors );
	sErrors.MoveTo ( sError );
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


void CSphFieldRegExps::AddRegExp ( const char * sRegExp, StringBuilder_c & sErrors )
{
	if ( m_bCloned )
		return;

	const char sSplitter [] = "=>";
	const char * sSplit = strstr ( sRegExp, sSplitter );
	if ( !sSplit )
	{
		sErrors << "mapping token (=>) not found";
		return;
	} else if ( strstr ( sSplit + strlen ( sSplitter ), sSplitter ) )
	{
		sErrors << "mapping token (=>) found more than once";
		return;
	}

	RegExp_t tRegExp;
	tRegExp.m_sFrom.SetBinary ( sRegExp, int ( sSplit-sRegExp ) );
	tRegExp.m_sTo = sSplit + strlen ( sSplitter );
	tRegExp.m_sFrom.Trim();
	tRegExp.m_sTo.Trim();

	RE2::Options tOptions;
	tOptions.set_encoding ( RE2::Options::Encoding::EncodingUTF8 );
	auto pRE2 = std::make_unique<RE2> ( tRegExp.m_sFrom.cstr(), tOptions );

	std::string sRE2Error;
	if ( !pRE2->CheckRewriteString ( tRegExp.m_sTo.cstr(), &sRE2Error ) )
	{
		sErrors.Sprintf( "\"%s => %s\" is not a valid mapping: %s", tRegExp.m_sFrom.cstr(), tRegExp.m_sTo.cstr(), sRE2Error.c_str() );
		return;
	}
	tRegExp.m_pRE2 = pRE2.release();
	m_dRegexps.Add ( std::move ( tRegExp ) );
}


std::unique_ptr<ISphFieldFilter> CSphFieldRegExps::Clone() const
{
	auto pCloned = std::make_unique<CSphFieldRegExps>();
	pCloned->m_dRegexps = m_dRegexps;

	return pCloned;
}
#endif


#if WITH_RE2
std::unique_ptr<ISphFieldFilter> sphCreateRegexpFilter ( const CSphFieldFilterSettings & tFilterSettings, CSphString & sError )
{
	return std::make_unique<CSphFieldRegExps> ( tFilterSettings.m_dRegexps, sError );
}
#else
std::unique_ptr<ISphFieldFilter> sphCreateRegexpFilter ( const CSphFieldFilterSettings &, CSphString & )
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

	OpenHashTable_T<int64_t, int> hFields;
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


void SetPseudoSharding ( bool bSet )
{
	g_bPseudoSharding = bSet;
}


bool GetPseudoSharding()
{
	return g_bPseudoSharding;
}


void SetPseudoShardingThresh ( int iThresh )
{
	g_iPseudoShardingThresh = iThresh;
}


void SetMergeSettings ( const BuildBufferSettings_t & tSettings )
{
	g_tMergeSettings = tSettings;
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

ISphWordlist::Args_t::Args_t ( bool bPayload, ExpansionContext_t & tCtx )
	: ExpansionTrait_t ( tCtx )
	, m_bPayload ( bPayload )
	, m_tExpansionStats ( tCtx.m_tExpansionStats )
{
	m_sBuf.Reserve ( 2048 * SPH_MAX_WORD_LEN * 3 );
	m_dExpanded.Reserve ( 2048 );
	m_pPayload = nullptr;
	m_iTotalDocs = 0;
	m_iTotalHits = 0;
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

void ExpansionContext_t::AggregateStats ()
{
	if ( m_pResult )
		m_pResult->AddStat ( m_tExpansionStats );
}

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
			int iPackedLen = UnzipIntBE ( pBlock );
			const BYTE * pMax = pBlock + iPackedLen;
			while ( pBlock<pMax )
			{
				iLast += UnzipIntBE ( pBlock );
				dCheckpoints.Add ( (DWORD)iLast );
			}
			return true;
		}

		int iSkip = UnzipIntBE ( pBlock );
		pBlock += iSkip;
	}
	return false;
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

static void sphBuildNGrams ( const char * sWord, int iLen, int iGramLen, CSphVector<char> & dNGrams )
{
	int dOff[SPH_MAX_WORD_LEN+1];
	int iCodepoints = BuildUtf8Offsets ( sWord, iLen, dOff, sizeof ( dOff ) );
	if ( iCodepoints<iGramLen )
		return;

	dNGrams.Reserve ( iLen*3 );
	for ( int iChar=0; iChar<=iCodepoints-iGramLen; iChar++ )
	{
		int iStart = dOff[iChar];
		int iEnd = dOff[iChar+iGramLen];
		int iSize = iEnd - iStart;

		char * sDst = dNGrams.AddN ( iSize + 1 );
		memcpy ( sDst, sWord+iStart, iSize );
		sDst[iSize] = '\0';
	}
	// n-grams split by delimiter
	// however it's still null terminated
	dNGrams.Last() = '\0';
}

template <typename T>
int sphLevenshtein ( const T * sWord1, int iLen1, const T * sWord2, int iLen2, CSphVector<int> & dTmp )
{
	if ( !iLen1 )
		return iLen2;
	if ( !iLen2 )
		return iLen1;

	// FIXME!!! remove extra length after utf8->codepoints conversion
	dTmp.Resize ( Max ( iLen1, iLen2 )+1 );

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

int sphLevenshtein ( const char * sWord1, int iLen1, const char * sWord2, int iLen2, CSphVector<int> & dTmp )
{
	return sphLevenshtein<char> ( sWord1, iLen1, sWord2, iLen2, dTmp );
}

int sphLevenshtein ( const int * sWord1, int iLen1, const int * sWord2, int iLen2, CSphVector<int> & dTmp )
{
	return sphLevenshtein<int> ( sWord1, iLen1, sWord2, iLen2, dTmp );
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


bool SuggestResult_t::SetWord ( const char * sWord, const TokenizerRefPtr_c & pTok, bool bUseLastWord, bool bSetSentence )
{
	assert ( pTok->IsQueryTok() );
	TokenizerRefPtr_c pTokenizer = pTok->Clone ( SPH_CLONE );
	pTokenizer->SetBuffer ( (BYTE *)const_cast<char*>(sWord), (int) strlen ( sWord ) );

	int iWord = 0;
	const char * pPrevToken = nullptr;
	const char * pLastToken = nullptr;

	for ( const BYTE* pToken = pTokenizer->GetToken(); pToken; pToken = pTokenizer->GetToken() )
	{
		if ( bUseLastWord || iWord==0 )
			m_sWord = (const char *)pToken;

		if ( bSetSentence )
		{
			pPrevToken = pLastToken;
			pLastToken = pTokenizer->GetTokenEnd();
		}

		iWord++;

		if ( !bUseLastWord && !bSetSentence )
			break;

		if ( pTokenizer->TokenIsBlended() )
			pTokenizer->SkipBlended();
	}

	if ( bSetSentence && pPrevToken )
	{
		int iSentenceLen = pPrevToken - sWord;
		m_sSentence.SetBinary ( sWord, iSentenceLen );
	}

	m_iLen = m_sWord.Length();
	m_iCodepoints = DecodeUtf8 ( (const BYTE *)m_sWord.cstr(), m_dCodepoints );
	m_bUtf8 = ( m_iCodepoints!=m_iLen );

	bool bValidWord = ( m_iCodepoints>=3 );
	if ( bValidWord )
	{
		// lets generate bigrams for short words as trigrams for 5char word could all contain the same wrong symbol
		if ( m_iCodepoints<6 )
			m_iNGramLen = 2;

		sphBuildNGrams ( m_sWord.cstr(), m_iLen, m_iNGramLen, m_dTrigrams );
	}

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


void SuggestMergeDocs ( CSphVector<SuggestWord_t> & dMatched )
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

	OpenHashTable_T<int64_t, int> dHashTrigrams;
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
	const int iNGramLen = tRes.m_iNGramLen;
	tRes.m_dMatched.Reserve ( iQLen * 2 );
	CmpSuggestOrder_fn fnCmp;
	CSphVector<int> dLevenshteinTmp;

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
			for ( int iChar=0; iChar<=iDictCodepoints-iNGramLen && !bGotMatch; iChar++ )
			{
				int iStart = dCharOffset[iChar];
				int iEnd = dCharOffset[iChar+iNGramLen];
				bGotMatch = ( dHashTrigrams.Find ( sphCRC32 ( sDictWord + iStart, iEnd - iStart ) )!=NULL );
			}

			// skip word in case of no trigrams matched
			if ( !bGotMatch )
				continue;

			int iDist = INT_MAX;
			if_const ( SINGLE_BYTE_CHAR )
				iDist = sphLevenshtein ( tRes.m_sWord.cstr(), tRes.m_iLen, sDictWord, iDictWordLen, dLevenshteinTmp );
			else
				iDist = sphLevenshtein ( tRes.m_dCodepoints, tRes.m_iCodepoints, dDictWordCodepoints, iDictCodepoints, dLevenshteinTmp );

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

void IteratorStats_t::Merge ( const IteratorStats_t & tSrc )
{
	m_iTotal += tSrc.m_iTotal;

	for ( const auto & i : tSrc.m_dIterators )
	{
		bool bFound = false;
		for ( auto & j : m_dIterators )
			if ( i.m_sAttr==j.m_sAttr && i.m_sType==j.m_sType )
			{
				j.m_iUsed += i.m_iUsed;
				bFound = true;
			}

		if ( !bFound )
			m_dIterators.Add(i);
	}
}

//////////////////////////////////////////////////////////////////////////
// CSphQueryResultMeta
//////////////////////////////////////////////////////////////////////////

void RemoveDictSpecials ( CSphString & sWord, bool bBigram )
{
	if ( sWord.cstr()[0]==MAGIC_WORD_HEAD )
	{
		*const_cast<char *>( sWord.cstr() ) = '*';
	} else if ( sWord.cstr()[0]==MAGIC_WORD_HEAD_NONSTEMMED )
	{
		*const_cast<char *>( sWord.cstr() ) = '=';
	}

	if ( bBigram )
	{
		const char * p = strchr ( sWord.cstr(), MAGIC_WORD_BIGRAM );
		if ( p )
			*const_cast<char *>(p) = ' ';
	}
}

const CSphString & RemoveDictSpecials ( const CSphString & sWord, CSphString & sFixed, bool bBigram )
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
	}

	if ( bBigram )
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
	const CSphString & tFixed = RemoveDictSpecials ( sWord, sBuf, m_bBigram );
	WordStat_t & tStats = m_hWordStats.AddUnique ( tFixed );
	tStats.first += iDocs;
	tStats.second += iHits;
}

void CSphQueryResultMeta::AddStat ( const ExpansionStats_t & tExpansionStats )
{
	m_tExpansionStats.m_iTerms += tExpansionStats.m_iTerms;
	m_tExpansionStats.m_iMerged += tExpansionStats.m_iMerged;
}

void CSphQueryResultMeta::MergeWordStats ( const CSphQueryResultMeta & tOther )
{
	const auto & hOtherStats = tOther.m_hWordStats;
	if ( !m_hWordStats.GetLength () )
	{
		// nothing has been set yet; just copy
		m_hWordStats = hOtherStats;
	} else
	{
		for ( auto & tSrc : hOtherStats )
		{
			WordStat_t & tDst = m_hWordStats.AddUnique ( tSrc.first );
			tDst.first += tSrc.second.first;
			tDst.second += tSrc.second.second;
		}
	}

	AddStat ( tOther.m_tExpansionStats );
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
