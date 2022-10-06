//
// Copyright (c) 2018-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#include "sphinxstd.h"
#include "fileutils.h"
#include "sphinxutils.h"
#include "sphinxint.h"
#include "icu.h"
#include "attribute.h"
#include "sphinxsearch.h"
#include "secondaryindex.h"
#include "histogram.h"
#include "sphinxpq.h"
#include "accumulator.h"
#include "indexformat.h"
#include "indexsettings.h"
#include "indexfiles.h"
#include "docidlookup.h"
#include "attrindex_builder.h"
#include "tokenizer/charset_definition_parser.h"
#include "tokenizer/tokenizer.h"

namespace legacy
{

typedef uint64_t		SphWordID_t;
typedef uint64_t		SphDocID_t;
#define DOCINFO_IDSIZE	2

STATIC_SIZE_ASSERT ( SphWordID_t, 8 );
STATIC_SIZE_ASSERT ( SphDocID_t, 8 );

#define DWSIZEOF(a) ( sizeof(a) / sizeof(DWORD) )
#define MVA_OFFSET_MASK		0x7fffffffUL	// MVA offset mask
#define MVA_ARENA_FLAG		0x80000000UL	// MVA global-arena flag

const DWORD SPH_SKIPLIST_BLOCK=128;

static const DWORD META_HEADER_MAGIC	= 0x54525053;	///< my magic 'SPRT' header
static const DWORD META_VERSION		= 17;			///< current version

static const DWORD PQ_META_HEADER_MAGIC = 0x50535451;	///< magic 'PSTQ' header
static const DWORD PQ_META_VERSION = 7;					///< current version

static bool g_bLargeDocid = false;
static CSphString g_sOutDir;

//////////////////////////////////////////////////////////////////////////

/// row entry (storage only, does not necessarily map 1:1 to attributes)
typedef DWORD			CSphRowitem;
typedef const BYTE *	CSphRowitemPtr;

/// widest integer type that can be be stored as an attribute (ideally, fully decoupled from rowitem size!)
typedef int64_t			SphAttr_t;

const int				ROWITEM_BITS	= 8*sizeof(CSphRowitem);
const int				ROWITEM_SHIFT	= 5;

STATIC_ASSERT ( ( 1 << ROWITEM_SHIFT )==ROWITEM_BITS, INVALID_ROWITEM_SHIFT );

#ifndef USE_LITTLE_ENDIAN
#error Please define endianness
#endif

template < typename DOCID >
inline DOCID DOCINFO2ID_T ( const DWORD * pDocinfo );

template<> inline DWORD DOCINFO2ID_T ( const DWORD * pDocinfo )
{
	return pDocinfo[0];
}

template<> inline uint64_t DOCINFO2ID_T ( const DWORD * pDocinfo )
{
#if USE_LITTLE_ENDIAN
	return uint64_t(pDocinfo[0]) + (uint64_t(pDocinfo[1])<<32);
#else
	return uint64_t(pDocinfo[1]) + (uint64_t(pDocinfo[0])<<32);
#endif
}

static SphDocID_t DOCINFO2ID ( const DWORD * pDocinfo )
{
	return DOCINFO2ID_T<SphDocID_t> ( pDocinfo );
}

template < typename DOCID > inline DWORD *			DOCINFO2ATTRS_T ( DWORD * pDocinfo )		{ return pDocinfo + DWSIZEOF(DOCID); }
template < typename DOCID > inline const DWORD *	DOCINFO2ATTRS_T ( const DWORD * pDocinfo )	{ return pDocinfo + DWSIZEOF(DOCID); }
inline 			DWORD *	DOCINFO2ATTRS ( DWORD * pDocinfo )			{ return DOCINFO2ATTRS_T<SphDocID_t>(pDocinfo); }
inline const	DWORD *	DOCINFO2ATTRS ( const DWORD * pDocinfo )	{ return DOCINFO2ATTRS_T<SphDocID_t>(pDocinfo); }


enum ESphDocinfo
{
	SPH_DOCINFO_NONE		= 0,	///< no docinfo available
	SPH_DOCINFO_INLINE		= 1,	///< inline docinfo into index (specifically, into doclists)
	SPH_DOCINFO_EXTERN		= 2		///< store docinfo separately
};

enum IndexType_e
{
	INDEX_UNKNOWN,
	INDEX_PLAIN,
	INDEX_RT,
	INDEX_PQ
};

static const char * g_sIndexType[] = { "none", "plain", "rt", "percolate" };

/// unpack string attr from row storage (22 bits length max)
/// returns unpacked length; stores pointer to string data if required
static int sphUnpackStr ( const BYTE * pRow, const BYTE ** ppStr )
{
	int v = *pRow++;
	if ( v & 0x80 )
	{
		if ( v & 0x40 )
		{
			v = ( int ( v & 0x3f )<<16 ) + ( int ( *pRow++ )<<8 );
			v += ( *pRow++ ); // MUST be separate statement; cf. sequence point
		} else
		{
			v = ( int ( v & 0x3f )<<8 ) + ( *pRow++ );
		}
	}
	if ( ppStr )
		*ppStr = pRow;
	return v;
}

static CSphString GetIndexFileName ( const CSphString & sPath, const char * sExt )
{
	CSphString sRes;
	sRes.SetSprintf ( "%s.%s", sPath.cstr(), sExt );
	return sRes;
}

struct IndexSettings_t : public CSphSourceSettings
{
	ESphDocinfo		m_eDocinfo = SPH_DOCINFO_EXTERN;
	ESphHitFormat	m_eHitFormat = SPH_HIT_FORMAT_PLAIN;
	bool			m_bHtmlStrip = false;
	CSphString		m_sHtmlIndexAttrs;
	CSphString		m_sHtmlRemoveElements;
	CSphString		m_sZones;
	ESphHitless		m_eHitless = SPH_HITLESS_NONE;
	int				m_iEmbeddedLimit = 0;
	int64_t			m_tBlobUpdateSpace = 0;
	int				m_iSkiplistBlockSize = 0;

	ESphBigram				m_eBigramIndex = SPH_BIGRAM_NONE;
	CSphString				m_sBigramWords;
	StrVec_t				m_dBigramWords;

	DWORD			m_uAotFilterMask = 0;
	Preprocessor_e	m_ePreprocessor = Preprocessor_e::ICU;

	CSphString		m_sIndexTokenFilter;
};

struct Wordlist_t : public ISphWordlist
{
public:
	SphOffset_t		m_iWordsEnd = 0;		///< end of wordlist

	int				m_iDictCheckpoints = 0;			///< how many dict checkpoints (keyword blocks) are there
	SphOffset_t		m_iDictCheckpointsOffset = 0;	///< dict checkpoints file position
	int				m_iInfixCodepointBytes = 0;		///< max bytes per infix codepoint (0 means no infixes)
	int64_t			m_iInfixBlocksOffset = 0;		///< infix blocks file position (stored as unsigned 32bit int as keywords dictionary is pretty small)
	int				m_iInfixBlocksWordsSize = 0;	///< infix checkpoints size

										Wordlist_t () {}
	virtual								~Wordlist_t () override {}
	bool								Preread ( const char * sName, DWORD uVersion, bool bWordDict, CSphString & sError );

	void GetPrefixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const override {}
	void GetInfixedWords ( const char * sSubstring, int iSubLen, const char * sWildcard, Args_t & tArgs ) const override {}

private:
	bool								m_bWordDict = false;
};

struct Index_t
{
	DWORD m_uVersion = 0;	
	int64_t m_iMinMaxIndex = 0;
	int64_t m_iDocinfo = 0;
	
	bool m_bArenaProhibit = false;
	DWORD m_iTotalDocuments = 0;
	int64_t m_iTotalBytes = 0;

	SphDocID_t m_uMinDocid;

	CSphVector<CSphColumnInfo> m_dSchemaFields;
	CSphVector<CSphColumnInfo> m_dSchemaAttrs;

	IndexSettings_t m_tSettings;
	CSphAutoreader m_tDoclistFile;
	CSphAutoreader m_tHitlistFile;

	CSphMappedBuffer<DWORD> m_tAttr;
	CSphMappedBuffer<DWORD> m_tMva;
	CSphMappedBuffer<DWORD> m_tMvaArena;
	CSphMappedBuffer<BYTE> m_tString;
	CSphMappedBuffer<SphDocID_t> m_tKillList;
	CSphMappedBuffer<BYTE> m_tSkiplists;
	Wordlist_t m_tWordlist;
	CSphFixedVector<int64_t> m_dFieldLens { SPH_MAX_FIELDS };

	CSphDictSettings m_tDictSettings;
	DictRefPtr_c  m_pDict;
	CSphTokenizerSettings m_tTokSettings;
	TokenizerRefPtr_c m_pTokenizer;
	CSphFieldFilterSettings m_tFieldFilterSettings;
	CSphEmbeddedFiles m_tEmbeddedTok;
	CSphEmbeddedFiles m_tEmbeddedDict;

	CSphString m_sName;
	CSphString m_sPath;
	CSphString m_sPathOut;
	bool m_bStripPath = false;
	KillListTargets_c m_tKlistTargets;

	// RT specific
	int64_t m_iTID = 0;
	int m_iSegmentSeq = 0;
	int m_iWordsCheckpoint = 0;
	int m_iMaxCodepointLength = 0;
	CSphFixedVector<int> m_dRtChunkNames {0};
	CSphSchema m_tSchema;

	// PQ specific
	CSphFixedVector<StoredQueryDesc_t> m_dStored { 0 };

	bool IsSeparateOutDir () const { return m_sPath!=m_sPathOut; }

	CSphString GetFilename ( ESphExt eExt ) const
	{
		CSphString sName;
		if ( IsSeparateOutDir() )
			sName.SetSprintf ( "%s%s", m_sPathOut.cstr(), sphGetExt(eExt) );
		else
			sName.SetSprintf ( "%s.new%s", m_sPathOut.cstr(), sphGetExt(eExt) );

		return sName;
	}
};

static bool LoadPersistentMVA ( Index_t & tIndex, const CSphString & sPath, CSphString & sError )
{
	// prepare the file to load
	if ( !sphIsReadable ( GetIndexFileName( sPath, "mvp" ).cstr() ) )
	{
		// no mvp means no saved attributes.
		return true;
	}

	if ( tIndex.m_bArenaProhibit )
	{
		sError.SetSprintf ( "MVA update disabled (already so many MVA " INT64_FMT ", should be less %d)", tIndex.m_tMva.GetLength64(), INT_MAX );
		return false;
	}

	if ( !tIndex.m_tMvaArena.Setup ( GetIndexFileName( sPath, "mvp" ).cstr(), sError, false ) )
		return false;

	return true;
}

static void LoadIndexSettings ( IndexSettings_t & tSettings, CSphReader & tReader, DWORD uVersion )
{
	tSettings.SetMinPrefixLen ( tReader.GetDword() );
	tSettings.m_iMinInfixLen = tReader.GetDword ();

	if ( uVersion>=38 )
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

	if ( uVersion>=32 )
	{
		tSettings.m_eBigramIndex = (ESphBigram)tReader.GetByte();
		tSettings.m_sBigramWords = tReader.GetString();
	}

	if ( uVersion>=35 )
		tSettings.m_bIndexFieldLens = ( tReader.GetByte()!=0 );

	if ( uVersion>=39 )
	{
		tSettings.m_ePreprocessor = tReader.GetByte()==1 ? Preprocessor_e::ICU : Preprocessor_e::NONE;
		tReader.GetString();	// was: RLP context
	}

	if ( uVersion>=41 )
		tSettings.m_sIndexTokenFilter = tReader.GetString();

	if ( uVersion>55 )
		tSettings.m_tBlobUpdateSpace = tReader.GetOffset();

	if ( uVersion<56 )
		tSettings.m_iSkiplistBlockSize = 128;
	else
		tSettings.m_iSkiplistBlockSize = (int)tReader.GetDword();
}


static bool LoadTokenizerSettings ( CSphReader & tReader, CSphTokenizerSettings & tSettings, CSphEmbeddedFiles & tEmbeddedFiles, DWORD uVersion, CSphString & sWarning )
{
	tSettings.m_iType = tReader.GetByte ();
	if ( tSettings.m_iType!=TOKENIZER_UTF8 && tSettings.m_iType!=TOKENIZER_NGRAM )
	{
		sWarning = "can't load an old index with SBCS tokenizer";
		return false;
	}

	tSettings.m_sCaseFolding = tReader.GetString ();
	tSettings.m_iMinWordLen = tReader.GetDword ();
	tEmbeddedFiles.m_bEmbeddedSynonyms = false;
	tEmbeddedFiles.m_bEmbeddedSynonyms = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedSynonyms )
	{
		int nSynonyms = (int)tReader.GetDword();
		tEmbeddedFiles.m_dSynonyms.Resize ( nSynonyms );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dSynonyms )
			tEmbeddedFiles.m_dSynonyms[i] = tReader.GetString();
	}

	tSettings.m_sSynonymsFile = tReader.GetString ();
	tEmbeddedFiles.m_tSynonymFile.Read ( tReader, tSettings.m_sSynonymsFile.cstr (), false, tEmbeddedFiles.m_bEmbeddedSynonyms ? NULL : &sWarning );
	tSettings.m_sBoundary = tReader.GetString ();
	tSettings.m_sIgnoreChars = tReader.GetString ();
	tSettings.m_iNgramLen = tReader.GetDword ();
	tSettings.m_sNgramChars = tReader.GetString ();
	tSettings.m_sBlendChars = tReader.GetString ();
	tSettings.m_sBlendMode = tReader.GetString();

	return true;
}

static void LoadDictionarySettings ( CSphReader & tReader, CSphDictSettings & tSettings, CSphEmbeddedFiles & tEmbeddedFiles, DWORD uVersion, CSphString & sWarning )
{
	tSettings.m_sMorphology = tReader.GetString ();
	if ( uVersion>=43 )
		tSettings.m_sMorphFields = tReader.GetString();

	tEmbeddedFiles.m_bEmbeddedStopwords = false;
	tEmbeddedFiles.m_bEmbeddedStopwords = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedStopwords )
	{
		int nStopwords = (int)tReader.GetDword();
		tEmbeddedFiles.m_dStopwords.Resize ( nStopwords );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dStopwords )
			tEmbeddedFiles.m_dStopwords[i] = (SphWordID_t)tReader.UnzipOffset();
	}

	tSettings.m_sStopwords = tReader.GetString ();
	int nFiles = tReader.GetDword ();

	CSphString sFile;
	tEmbeddedFiles.m_dStopwordFiles.Resize ( nFiles );
	for ( int i = 0; i < nFiles; i++ )
	{
		sFile = tReader.GetString ();
		tEmbeddedFiles.m_dStopwordFiles[i].Read ( tReader, sFile.cstr (), false, tEmbeddedFiles.m_bEmbeddedSynonyms ? NULL : &sWarning );
	}

	tEmbeddedFiles.m_bEmbeddedWordforms = false;
	tEmbeddedFiles.m_bEmbeddedWordforms = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedWordforms )
	{
		int nWordforms = (int)tReader.GetDword();
		tEmbeddedFiles.m_dWordforms.Resize ( nWordforms );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dWordforms )
			tEmbeddedFiles.m_dWordforms[i] = tReader.GetString();
	}

	tSettings.m_dWordforms.Resize ( tReader.GetDword() );
	tEmbeddedFiles.m_dWordformFiles.Resize ( tSettings.m_dWordforms.GetLength() );
	ARRAY_FOREACH ( i, tSettings.m_dWordforms )
	{
		tSettings.m_dWordforms[i] = tReader.GetString();
		tEmbeddedFiles.m_dWordformFiles[i].Read ( tReader, tSettings.m_dWordforms[i].cstr(), false, tEmbeddedFiles.m_bEmbeddedWordforms ? NULL : &sWarning );
	}

	tSettings.m_iMinStemmingLen = tReader.GetDword ();
	tSettings.m_bWordDict = false; // default to crc for old indexes
	tSettings.m_bWordDict = ( tReader.GetByte()!=0 );

	if ( uVersion>=36 )
		tSettings.m_bStopwordsUnstemmed = ( tReader.GetByte()!=0 );

	if ( uVersion>=37 )
		tSettings.m_sMorphFingerprint = tReader.GetString();
}

static void LoadFieldFilterSettings ( CSphReader & tReader, CSphFieldFilterSettings & tFieldFilterSettings )
{
	int nRegexps = tReader.GetDword();
	if ( !nRegexps )
		return;

	tFieldFilterSettings.m_dRegexps.Resize ( nRegexps );
	ARRAY_FOREACH ( i, tFieldFilterSettings.m_dRegexps )
		tFieldFilterSettings.m_dRegexps[i] = tReader.GetString();

	tReader.GetByte(); // deprecated utf-8 flag
}

static void ReadSchemaColumn ( CSphReader & rdInfo, CSphColumnInfo & tCol )
{
	tCol.m_sName = rdInfo.GetString ();
	if ( tCol.m_sName.IsEmpty () )
		tCol.m_sName = "@emptyname";

	tCol.m_sName.ToLower ();
	tCol.m_eAttrType = (ESphAttr) rdInfo.GetDword ();

	rdInfo.GetDword (); // ignore rowitem
	tCol.m_tLocator.m_iBitOffset = rdInfo.GetDword ();
	tCol.m_tLocator.m_iBitCount = rdInfo.GetDword ();
	tCol.m_bPayload = ( rdInfo.GetByte()!=0 );
}

static void ReadSchema ( CSphReader & rdInfo, CSphVector<CSphColumnInfo> & dFields, CSphVector<CSphColumnInfo> & dAttrs )
{
	int iNumFields = rdInfo.GetDword();
	for ( int i=0; i<iNumFields; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumn ( rdInfo, tCol );
		dFields.Add ( tCol );
	}

	int iNumAttrs = rdInfo.GetDword();
	for ( int i=0; i<iNumAttrs; i++ )
	{
		CSphColumnInfo tCol;
		ReadSchemaColumn ( rdInfo, tCol );
		dAttrs.Add ( tCol );
	}
}

static int GetRowSize ( const CSphVector<CSphColumnInfo> & dAttrs )
{
	int iMaxBitSize = 0;
	for ( const auto & i : dAttrs )
		iMaxBitSize = Max ( iMaxBitSize, i.m_tLocator.m_iBitOffset + i.m_tLocator.m_iBitCount );

	return (iMaxBitSize+ROWITEM_BITS-1) / ROWITEM_BITS;
}

static bool SetupWordProcessors ( Index_t & tIndex, CSphString & sError )
{
	StrVec_t dWarnings;
	TokenizerRefPtr_c pTokenizer = Tokenizer::Create ( tIndex.m_tTokSettings, &tIndex.m_tEmbeddedTok, nullptr, dWarnings, sError );
	if ( !pTokenizer )
		return false;

	DictRefPtr_c pDict { tIndex.m_tDictSettings.m_bWordDict
		? sphCreateDictionaryKeywords ( tIndex.m_tDictSettings, &tIndex.m_tEmbeddedDict, pTokenizer, tIndex.m_sName.cstr(), false, tIndex.m_tSettings.m_iSkiplistBlockSize, nullptr, sError )
		: sphCreateDictionaryCRC ( tIndex.m_tDictSettings, &tIndex.m_tEmbeddedDict, pTokenizer, tIndex.m_sName.cstr(), false, tIndex.m_tSettings.m_iSkiplistBlockSize, nullptr, sError ) };

	if ( !pDict )
		return false;
	tIndex.m_pDict = pDict;

	Tokenizer::AddToMultiformFilterTo ( pTokenizer, tIndex.m_pDict->GetMultiWordforms () );

	// initialize AOT if needed
	tIndex.m_tSettings.m_uAotFilterMask = sphParseMorphAot ( tIndex.m_tDictSettings.m_sMorphology.cstr() );
	// aot filter
	if ( tIndex.m_tSettings.m_uAotFilterMask )
		sphAotTransformFilter ( pTokenizer, tIndex.m_pDict, tIndex.m_tSettings.m_bIndexExactWords, tIndex.m_tSettings.m_uAotFilterMask );
	tIndex.m_pTokenizer = pTokenizer;

	return true;
}

static bool LoadHeader ( const char * sHeaderName, Index_t & tIndex, CSphString & sError )
{
	CSphAutoreader rdInfo;
	if ( !rdInfo.Open ( sHeaderName, sError ) )
		return false;

	// magic header
	const char * sFmt = CheckFmtMagic ( rdInfo.GetDword () );
	if ( sFmt )
	{
		sError.SetSprintf ( sFmt, sHeaderName );
		return false;
	}

	// version
	tIndex.m_uVersion = rdInfo.GetDword();
	if ( tIndex.m_uVersion<=1 || tIndex.m_uVersion>INDEX_FORMAT_VERSION || tIndex.m_uVersion<34 )
	{
		sError.SetSprintf ( "%s is v.%d, binary is v.%d", tIndex.m_sName.cstr(), tIndex.m_uVersion, INDEX_FORMAT_VERSION );
		return false;
	}

	if ( tIndex.m_uVersion>=50 )
	{
		sError.SetSprintf ( "already a v3 index; nothing to do" );
		return false;
	}

	// bits
	bool bUse64 = !!rdInfo.GetDword();
	if ( !bUse64 )
	{
		sError = "indexes with 32-bit docids are no longer supported";
		return false;
	}

	// skiplists
	if ( tIndex.m_uVersion<31 )
	{
		sError = "indexes without skiplist unsupported";
		return false;
	}

	// docinfo
	ESphDocinfo eDocinfo = (ESphDocinfo)rdInfo.GetDword();
	if ( eDocinfo!=SPH_DOCINFO_EXTERN )
	{
		sError.SetSprintf ( "index without docinfo extern unsupported, docinfo is %d", (int)eDocinfo );
		return false;
	}

	ReadSchema ( rdInfo, tIndex.m_dSchemaFields, tIndex.m_dSchemaAttrs );

	tIndex.m_uMinDocid = (SphDocID_t)rdInfo.GetOffset ();

	tIndex.m_tWordlist.m_iDictCheckpointsOffset = rdInfo.GetOffset();
	tIndex.m_tWordlist.m_iDictCheckpoints = rdInfo.GetDword();
	tIndex.m_tWordlist.m_iInfixCodepointBytes = rdInfo.GetByte();
	tIndex.m_tWordlist.m_iInfixBlocksOffset = rdInfo.GetDword();
	tIndex.m_tWordlist.m_iInfixBlocksWordsSize = rdInfo.GetDword();

	// index stats
	tIndex.m_iTotalDocuments = rdInfo.GetDword (); // m_iTotalDocuments
	tIndex.m_iTotalBytes = rdInfo.GetOffset (); // m_iTotalBytes
	if ( tIndex.m_uVersion>=40 )
		rdInfo.GetDword(); // m_iTotalDups

	legacy::LoadIndexSettings ( tIndex.m_tSettings, rdInfo, tIndex.m_uVersion );

	// tokenizer stuff
	if ( !LoadTokenizerSettings ( rdInfo, tIndex.m_tTokSettings, tIndex.m_tEmbeddedTok, tIndex.m_uVersion, sError ) )
		return false;

	if ( tIndex.m_bStripPath )
		StripPath ( tIndex.m_tTokSettings.m_sSynonymsFile );
	
	// dictionary stuff
	legacy::LoadDictionarySettings ( rdInfo, tIndex.m_tDictSettings, tIndex.m_tEmbeddedDict, tIndex.m_uVersion, sError );
	if ( !sError.IsEmpty() )
	{
		sphWarning ( "%s", sError.cstr() );
		sError = "";
	}

	if ( tIndex.m_bStripPath )
	{
		StripPath ( tIndex.m_tDictSettings.m_sStopwords );
		ARRAY_FOREACH ( i, tIndex.m_tDictSettings.m_dWordforms )
			StripPath ( tIndex.m_tDictSettings.m_dWordforms[i] );
	}

	if ( !SetupWordProcessors ( tIndex, sError ) )
		return false;

	rdInfo.GetDword ();

	tIndex.m_iMinMaxIndex = rdInfo.GetOffset ();

	legacy::LoadFieldFilterSettings ( rdInfo, tIndex.m_tFieldFilterSettings );

	if ( tIndex.m_uVersion>=35 && tIndex.m_tSettings.m_bIndexFieldLens )
		for ( int i=0; i < tIndex.m_dSchemaFields.GetLength(); i++ )
			tIndex.m_dFieldLens[i] = rdInfo.GetOffset();

	if ( rdInfo.GetErrorFlag() )
	{
		sError.SetSprintf ( "%s: failed to parse header (unexpected eof)", sHeaderName );
		return false;
	}

	if ( rdInfo.GetPos()!=rdInfo.GetFilesize() )
	{
		sError.SetSprintf ( "%s: unexpected tail left; position " INT64_FMT ", file size " INT64_FMT , sHeaderName, (int64_t)rdInfo.GetPos(), (int64_t)rdInfo.GetFilesize() );
		return false;
	}

	return true;
}

bool Wordlist_t::Preread ( const char * sName, DWORD uVersion, bool bWordDict, CSphString & sError )
{
	assert ( ( uVersion>=21 && bWordDict ) || !bWordDict );
	assert ( m_iDictCheckpointsOffset>0 );

	m_bWordDict = bWordDict;
	m_iWordsEnd = m_iDictCheckpointsOffset; // set wordlist end

	////////////////////////////
	// preload word checkpoints
	////////////////////////////

	////////////////////////////
	// regular path that loads checkpoints data

	CSphAutoreader tReader;
	if ( !tReader.Open ( sName, sError ) )
		return false;

	int64_t iFileSize = tReader.GetFilesize();
	if ( iFileSize-m_iDictCheckpointsOffset>=UINT_MAX )
	{
		sError.SetSprintf ( "dictionary meta overflow: meta size=" INT64_FMT ", total size=" INT64_FMT ", meta offset=" INT64_FMT,
			iFileSize-m_iDictCheckpointsOffset, iFileSize, (int64_t)m_iDictCheckpointsOffset );
		return false;
	}

	////////////////////////
	// preload infix blocks
	////////////////////////

	if ( m_iInfixCodepointBytes && m_iInfixBlocksOffset )
	{
		SphOffset_t uInfixOffset = 0;
		tReader.SeekTo ( m_iInfixBlocksOffset, (int)(iFileSize-m_iInfixBlocksOffset) );
		int iInfixCount = tReader.UnzipInt();
		if ( iInfixCount )
		{
			int iBytes = tReader.UnzipInt();
			tReader.SkipBytes ( iBytes );
			uInfixOffset = tReader.UnzipInt();
		}

		// FIXME!!! store and load that explicitly
		if ( iInfixCount )
			m_iWordsEnd = uInfixOffset - strlen ( g_sTagInfixEntries );
		else
			m_iWordsEnd -= strlen ( g_sTagInfixEntries );
	}

	if ( tReader.GetErrorFlag() )
	{
		sError = tReader.GetErrorMessage();
		return false;
	}

	tReader.Close();

	return true;
}

static bool LoadPlainIndexChunk ( Index_t & tIndex, CSphString & sError )
{
	const CSphString & sPath = tIndex.m_sPath;

	// preload schema
	if ( !LoadHeader ( GetIndexFileName ( sPath, "sph" ).cstr(), tIndex, sError ) )
		return false;

	if ( tIndex.m_tSettings.m_eDocinfo!=SPH_DOCINFO_EXTERN )
	{
		sError.SetSprintf ( "only docinfo extern supported, docinfo=%d", (int)tIndex.m_tSettings.m_eDocinfo );
		return false;
	}

	// verify that data files are readable
	if ( !sphIsReadable ( GetIndexFileName (  sPath,  "spd" ).cstr(), &sError ) )
		return false;

	if ( !sphIsReadable ( GetIndexFileName (  sPath,  "spp" ).cstr(), &sError ) )
		return false;

	if ( !sphIsReadable ( GetIndexFileName (  sPath,  "spe" ).cstr(), &sError ) )
		return false;

	// preopen
	if ( !tIndex.m_tDoclistFile.Open ( GetIndexFileName (  sPath,  "spd" ), sError ) )
		return false;

	if ( !tIndex.m_tHitlistFile.Open ( GetIndexFileName (  sPath, "spp" ), sError ) )
		return false;

	/////////////////////
	// prealloc wordlist
	/////////////////////

	if ( !sphIsReadable ( GetIndexFileName ( sPath,  "spi" ).cstr(), &sError ) )
		return false;

	// only checkpoint and wordlist infixes are actually read here; dictionary itself is just mapped
	if ( !tIndex.m_tWordlist.Preread ( GetIndexFileName ( sPath, "spi" ).cstr(), tIndex.m_uVersion, tIndex.m_tDictSettings.m_bWordDict, sError ) )
		return false;

	{
		CSphAutofile tDocinfo ( GetIndexFileName (  sPath, "spa" ), SPH_O_READ, sError );
		if ( tDocinfo.GetFD()<0 )
			return false;
	}

	/////////////////////
	// prealloc docinfos
	/////////////////////

	/////////////
	// attr data
	/////////////

	int iStride = DOCINFO_IDSIZE + GetRowSize ( tIndex.m_dSchemaAttrs );

	if ( !tIndex.m_tAttr.Setup ( GetIndexFileName (  sPath, "spa" ).cstr(), sError, false ) )
		return false;

	int64_t iDocinfoSize = tIndex.m_tAttr.GetLengthBytes();
	if ( iDocinfoSize<0 )
		return false;
	iDocinfoSize = iDocinfoSize / sizeof(DWORD);
	int64_t iRealDocinfoSize = tIndex.m_iMinMaxIndex ? tIndex.m_iMinMaxIndex : iDocinfoSize;
	tIndex.m_iDocinfo = iRealDocinfoSize / iStride;

	if ( iDocinfoSize < iRealDocinfoSize )
	{
		sError.SetSprintf ( "precomputed chunk size check mismatch (size=" INT64_FMT ", real=" INT64_FMT ", min-max=" INT64_FMT ", count=" INT64_FMT ")",
			iDocinfoSize, iRealDocinfoSize, tIndex.m_iMinMaxIndex, tIndex.m_iDocinfo );
		return false;
	}

	////////////
	// MVA data
	////////////

	if ( !tIndex.m_tMva.Setup ( GetIndexFileName (  sPath, "spm" ).cstr(), sError, false ) )
		return false;

	if ( tIndex.m_tMva.GetLength64()>INT_MAX )
	{
		tIndex.m_bArenaProhibit = true;
		sphWarning ( "MVA update disabled (loaded MVA " INT64_FMT ", should be less %d)", tIndex.m_tMva.GetLength64(), INT_MAX );
	}

	///////////////
	// string data
	///////////////

	if ( !tIndex.m_tString.Setup ( GetIndexFileName (  sPath, "sps" ).cstr(), sError, false ) )
		return false;

	// prealloc killlist
	if ( !tIndex.m_tKillList.Setup ( GetIndexFileName (  sPath, "spk" ).cstr(), sError, false ) )
		return false;

	// prealloc skiplist
	if ( !tIndex.m_tSkiplists.Setup ( GetIndexFileName (  sPath, "spe" ).cstr(), sError, false ) )
		return false;

	// almost done
	bool bPersistMVA = sphIsReadable ( GetIndexFileName (  sPath, "mvp" ).cstr() );
	if ( bPersistMVA )
	{
		if ( tIndex.m_bArenaProhibit )
		{
			sError.SetSprintf ( "MVA update disabled (already so many MVA " INT64_FMT ", should be less %d)", tIndex.m_tMva.GetLength64(), INT_MAX );
			return false;
		}

		if ( !LoadPersistentMVA ( tIndex, sPath, sError ) )
			return false;
	}

	return true;
}

struct AttrConverter_t
{
	AttrConverter_t ( const Index_t & tSrc, const CSphSchema & tDst, BlobRowBuilder_i * pBlob );
	CSphRowitem * NextRow();
	SphDocID_t GetRowDocid() { return m_tCurDocID; }

	CSphFixedVector<CSphRowitem> m_dDstRow {0};
	CSphVector<int64_t> m_dMVA;

	const int m_iSrcStride = 0;
	const int64_t m_iRows = 0;
	int64_t m_iCurRow = -1;
	SphDocID_t m_tCurDocID = 0;

	const Index_t & m_tIndex;
	BlobRowBuilder_i * m_pBlob = nullptr;
	CSphString m_sError;

	CSphAttrLocator m_tLocID;
	CSphFixedVector<CSphAttrLocator> m_dDstLoc { 0 };

	const DWORD * m_pMvaUpdates = nullptr;
};

AttrConverter_t::AttrConverter_t ( const Index_t & tSrc, const CSphSchema & tDst, BlobRowBuilder_i * pBlob )
	: m_iSrcStride ( DOCINFO_IDSIZE + GetRowSize ( tSrc.m_dSchemaAttrs ) )
	, m_iRows ( tSrc.m_iDocinfo )
	, m_tIndex ( tSrc )
	, m_pBlob ( pBlob )
{
	m_dDstRow.Reset ( tDst.GetRowSize() );
	const CSphColumnInfo * pColumnID = tDst.GetAttr ( sphGetDocidName() );		
	assert ( pColumnID );
	assert ( !pColumnID->m_tLocator.m_bDynamic );
	m_tLocID = pColumnID->m_tLocator;

	const CSphVector<CSphColumnInfo> & dAttrs = m_tIndex.m_dSchemaAttrs;
	m_dDstLoc.Reset ( dAttrs.GetLength() );

	ARRAY_FOREACH ( i, dAttrs )
	{
		const CSphColumnInfo & tColumnSrc = dAttrs[i];
		if ( tColumnSrc.m_eAttrType==SPH_ATTR_STRING || tColumnSrc.m_eAttrType==SPH_ATTR_JSON ||
			tColumnSrc.m_eAttrType==SPH_ATTR_UINT32SET || tColumnSrc.m_eAttrType==SPH_ATTR_INT64SET )
			continue;

		const CSphColumnInfo * pColumnDst = tDst.GetAttr ( tColumnSrc.m_sName.cstr() );
		assert ( pColumnDst );
		m_dDstLoc[i]= pColumnDst->m_tLocator;
	}

	// persist MVA
	if ( !tSrc.m_tMvaArena.IsEmpty() )
	{
		const DWORD * pMvaArena = tSrc.m_tMvaArena.GetReadPtr();
		DWORD uDocs = *pMvaArena;
		if ( uDocs )
			m_pMvaUpdates = (pMvaArena+1) + uDocs * sizeof(SphDocID_t)/sizeof(DWORD);
	}
}

CSphRowitem * AttrConverter_t::NextRow()
{
	if ( m_iCurRow+1<m_iRows )
		m_iCurRow++;
	else
		return nullptr;

	const CSphRowitem * pSrcRow = m_tIndex.m_tAttr.GetReadPtr() + m_iCurRow * m_iSrcStride;
	m_tCurDocID = DOCINFO2ID ( pSrcRow );
	const CSphRowitem * pAttrs = DOCINFO2ATTRS ( pSrcRow );

	m_dDstRow.Fill ( 0 );
	sphSetRowAttr ( m_dDstRow.Begin(), m_tLocID, m_tCurDocID );

	int iBlobAttr = 0;
	const CSphVector<CSphColumnInfo> & dAttrs = m_tIndex.m_dSchemaAttrs;
	ARRAY_FOREACH ( i, dAttrs )
	{
		const CSphColumnInfo & tColumnSrc = dAttrs[i];		

		if ( tColumnSrc.m_eAttrType==SPH_ATTR_STRING || tColumnSrc.m_eAttrType==SPH_ATTR_JSON )
		{
			const DWORD uOff = (DWORD)sphGetRowAttr ( pAttrs, tColumnSrc.m_tLocator );
			const BYTE * pStr = nullptr;
			int iLen = 0;
			if ( uOff )
				iLen = sphUnpackStr ( m_tIndex.m_tString.GetReadPtr() + uOff, &pStr );

			assert ( m_pBlob );
			m_pBlob->SetAttr( iBlobAttr++, (const BYTE*)pStr, iLen, m_sError );

		} else if ( tColumnSrc.m_eAttrType==SPH_ATTR_UINT32SET || tColumnSrc.m_eAttrType==SPH_ATTR_INT64SET )
		{
			DWORD uOff = (DWORD)sphGetRowAttr ( pAttrs, tColumnSrc.m_tLocator );
			const DWORD * pMva = nullptr;
			if ( uOff )
			{
				if ( !m_tIndex.m_bArenaProhibit && ( uOff & MVA_ARENA_FLAG ) )
				{
					assert ( m_pMvaUpdates && m_pMvaUpdates<m_tIndex.m_tMvaArena.GetReadPtr() + m_tIndex.m_tMvaArena.GetLength64() );
					pMva = m_pMvaUpdates;
					int iCount = *m_pMvaUpdates;
					m_pMvaUpdates += iCount + 1;
				} else
				{
					pMva = m_tIndex.m_tMva.GetReadPtr() + uOff;
				}
			}

			int iValues = 0;
			if ( pMva )
				iValues = *pMva++;

			if ( tColumnSrc.m_eAttrType==SPH_ATTR_UINT32SET )
			{
				// blob packer expect all types of MVA as int64
				m_dMVA.Resize ( iValues );
				for ( int iValue=0; iValue<iValues; iValue++ )
					m_dMVA[iValue] = pMva[iValue];

				pMva = (const DWORD *)m_dMVA.Begin();
			} else
			{
				iValues /= 2;
			}

			m_pBlob->SetAttr ( iBlobAttr++, (const BYTE*)pMva, iValues*sizeof(int64_t), m_sError );
		} else
		{
			SphAttr_t tValue = sphGetRowAttr ( pAttrs, tColumnSrc.m_tLocator );
			const CSphAttrLocator & tDstLoc = m_dDstLoc[i];
			sphSetRowAttr ( m_dDstRow.Begin(), tDstLoc, tValue );
		}
	}


	return m_dDstRow.Begin();
}

struct DoclistOffsets_t
{
	SphOffset_t m_uDoclist;
	SphOffset_t m_uSkiplist;
};

struct Checkpoint_t
{
	uint64_t m_uWord;
	uint64_t m_uOffset;
};

struct ConverterPlain_t
{
	bool Save ( const CSphVector<SphDocID_t> & dKilled, Index_t & tIndex, bool bIgnoreKlist, CSphString & sError );
	const CSphSchema & GetSchema() const { return m_tSchema; }
	bool ConvertSchema ( Index_t & tIndex, CSphString & sError );

private:
	std::unique_ptr<ISphInfixBuilder> m_pInfixer;
	CSphSchema m_tSchema;

	SphOffset_t						m_tCheckpointsPosition = 0;
	SphOffset_t						m_tDocinfoIndex = 0;
	SphOffset_t						m_tMinMaxPos = 0;
	int64_t							m_iInfixBlockOffset = 0;
	int								m_iInfixCheckpointWordsSize = 0;
	CSphVector<Checkpoint_t>		m_dCheckpoints;
	CSphVector<BYTE>				m_dKeywordCheckpoints;
	
	OpenHash_T<RowID_t, SphDocID_t, HashFunc_Int64_t> m_hDoc2Row;
	OpenHash_T<DoclistOffsets_t, SphOffset_t, HashFunc_Int64_t> m_hDoclist;

	bool WriteLookup ( Index_t & tIndex, CSphString & sError );
	bool WriteAttributes ( Index_t & tIndex, CSphString & sError );
	void WriteCheckpoints ( const Index_t & tIndex, CSphWriter & tWriterDict );
	bool WriteKillList ( const Index_t & tIndex, bool bIgnoreKlist, CSphString & sError );

	void SaveHeader ( const Index_t & tIndex, DWORD uKillListSize ) const;

	bool Init ( Index_t & tIndex, CSphString & sError );
	bool ConvertDoclist ( Index_t & tIndex, CSphString & sError );
	bool ConvertDictionary ( Index_t & tIndex, CSphString & sError );
};

struct CmpDocidLookup_fn
{
	static inline bool IsLess ( const DocidRowidPair_t & a, const DocidRowidPair_t & b )
	{
		if ( a.m_tDocID==b.m_tDocID )
			return a.m_tRowID < b.m_tRowID;

		return (uint64_t)a.m_tDocID < (uint64_t)b.m_tDocID;
	}
};

bool ConverterPlain_t::WriteLookup ( Index_t & tIndex, CSphString & sError )
{
	CSphString sSPA = tIndex.GetFilename(SPH_EXT_SPA);
	CSphAutofile tSPA ( sSPA.cstr(), SPH_O_READ, sError );
	if ( tSPA.GetFD()==-1 )
		return false;

	CSphReader tSPAReader;
	tSPAReader.SetFile(tSPA);

	HistogramContainer_c tHistogramContainer;
	CSphVector<Histogram_i *> dHistograms;
	CSphVector<CSphColumnInfo> dPOD;
	for ( int i = 0; i < m_tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
		std::unique_ptr<Histogram_i> pHistogram = CreateHistogram ( tAttr.m_sName, tAttr.m_eAttrType );
		if ( pHistogram )
		{
			dHistograms.Add ( pHistogram.get() );
			Verify ( tHistogramContainer.Add ( std::move ( pHistogram ) ) );
			dPOD.Add ( tAttr );
		}
	}

	int iStride = m_tSchema.GetRowSize();
	CSphVector<CSphRowitem> dRow ( iStride );
	CSphRowitem * pRow = dRow.Begin();

	CSphFixedVector<DocidRowidPair_t> dDocidLookup ( tIndex.m_iTotalDocuments );
	for ( RowID_t tRowID = 0; tRowID < tIndex.m_iTotalDocuments; tRowID++ )
	{
		tSPAReader.GetBytes ( pRow, iStride*sizeof(CSphRowitem) );
		if ( tSPAReader.GetErrorFlag() )
		{
			sError = tSPAReader.GetErrorMessage();
			return false;
		}

		ARRAY_FOREACH ( i, dHistograms )
			dHistograms[i]->Insert ( sphGetRowAttr ( pRow, dPOD[i].m_tLocator ) );

		dDocidLookup[tRowID].m_tDocID = sphGetDocID(pRow);
		dDocidLookup[tRowID].m_tRowID = tRowID;
	}

	dDocidLookup.Sort ( CmpDocidLookup_fn() );

	CSphString sSPT = tIndex.GetFilename(SPH_EXT_SPT);
	if ( !::WriteDocidLookup ( sSPT, dDocidLookup, sError ) )
		return false;

	CSphString sSPHI = tIndex.GetFilename(SPH_EXT_SPHI);
	if ( !tHistogramContainer.Save ( sSPHI, sError ) )
		return false;

	return true;
}


bool ConverterPlain_t::WriteAttributes ( Index_t & tIndex, CSphString & sError )
{
	CSphWriter tWriterSPA;

	CSphString sSPA = tIndex.GetFilename(SPH_EXT_SPA);
	CSphString sSPB = tIndex.GetFilename(SPH_EXT_SPB);
		 
	if ( !tWriterSPA.OpenFile ( sSPA.cstr(), sError ) )
		return false;
	
	const CSphColumnInfo * pBlobLocatorAttr = m_tSchema.GetAttr ( sphGetBlobLocatorName() );
	AttrIndexBuilder_c tMinMaxBuilder ( m_tSchema );

	std::unique_ptr<BlobRowBuilder_i> pBlobRowBuilder;
	if ( pBlobLocatorAttr )
	{
		pBlobRowBuilder = sphCreateBlobRowJsonBuilder ( m_tSchema, sSPB, tIndex.m_tSettings.m_tBlobUpdateSpace, sError );
		if ( !pBlobRowBuilder )
			return false;
	}

	RowID_t tNextRowID = 0;
	int iStride = m_tSchema.GetRowSize();
	AttrConverter_t tConv ( tIndex, m_tSchema, pBlobRowBuilder.get() );
	CSphRowitem * pRow = nullptr;
	while ( ( pRow = tConv.NextRow() )!=nullptr )
	{
		if ( pBlobLocatorAttr )
		{
			SphOffset_t tOffset = pBlobRowBuilder->Flush();
			sphSetRowAttr ( pRow, pBlobLocatorAttr->m_tLocator, tOffset );
		}
		tMinMaxBuilder.Collect ( pRow );
		tWriterSPA.PutBytes ( pRow, iStride*sizeof(CSphRowitem) );

		SphDocID_t uDocid = tConv.GetRowDocid();
		if ( uDocid>INT64_MAX )
		{
			sError.SetSprintf ( "unable to convert document ID " UINT64_FMT " greater than " INT64_FMT " at row %d", uDocid, INT64_MAX, tNextRowID );
			if ( !g_bLargeDocid )
				return false;
			else
			{
				sphWarning ( "%s, wrapping value", sError.cstr() );
				sError = "";
			}
		}

		m_hDoc2Row.Acquire ( uDocid ) = tNextRowID;

		tNextRowID++;
	}

	if ( pBlobRowBuilder && !pBlobRowBuilder->Done ( sError ) )
		return false;		

	tMinMaxBuilder.FinishCollect();
	const CSphTightVector<CSphRowitem> & dMinMaxRows = tMinMaxBuilder.GetCollected();
	m_tMinMaxPos = tWriterSPA.GetPos();
	tWriterSPA.PutBytes ( dMinMaxRows.Begin(), dMinMaxRows.GetLength()*sizeof(CSphRowitem) );
	tWriterSPA.CloseFile();

	tIndex.m_iTotalDocuments = tNextRowID;
	m_tDocinfoIndex = ( dMinMaxRows.GetLength() / m_tSchema.GetRowSize() / 2 ) - 1;

	if ( !WriteLookup ( tIndex, sError ) )
		return false;

	return true;
}

bool ConverterPlain_t::ConvertDoclist ( Index_t & tIndex, CSphString & sError )
{
	CSphWriter tWriterDocs, tWriterSkips;
	if ( !tWriterDocs.OpenFile ( tIndex.GetFilename(SPH_EXT_SPD).cstr(), sError ) )
		return false;

	if ( !tWriterSkips.OpenFile ( tIndex.GetFilename(SPH_EXT_SPE).cstr(), sError ) )
		return false;

	CSphAutoreader & tDoclist = tIndex.m_tDoclistFile;
	tDoclist.SeekTo ( 1, 0 );

	tWriterDocs.PutByte(1);
	tWriterSkips.PutByte(1);

	const SphOffset_t uDoclistEnd = tDoclist.GetFilesize();
	const bool bInlineHits = ( tIndex.m_tSettings.m_eHitFormat==SPH_HIT_FORMAT_INLINE );
	CSphVector<SkiplistEntry_t> dSkiplist;
	m_hDoclist.Reset ( tIndex.m_iDocinfo );

	DWORD uSkiplistBlock = tIndex.m_tSettings.m_iSkiplistBlockSize;

	while ( uDoclistEnd!=tDoclist.GetPos() )
	{
		const SphOffset_t uOldDoclist = tDoclist.GetPos();
		const SphOffset_t uNewDoclist = tWriterDocs.GetPos();

		SphDocID_t uOldDocid = tIndex.m_uMinDocid;
		SphDocID_t uDelta = 0;
		SphOffset_t uLastHitpos = 0;
		RowID_t tLastRowID = INVALID_ROWID;
		RowID_t tSkiplistRowID = INVALID_ROWID;
		int iDocs = 0;
		dSkiplist.Resize(0);

		while ( true )
		{
			uDelta = tDoclist.UnzipOffset();
			if ( !uDelta )
			{
				tWriterDocs.ZipOffset ( 0 );
				break;
			}

			uOldDocid += uDelta;
			const RowID_t * pRow = m_hDoc2Row.Find ( uOldDocid );
			if ( pRow )
			{
				// build skiplist, aka save decoder state as needed
				if ( ( iDocs & ( uSkiplistBlock-1 ) )==0 )
				{
					SkiplistEntry_t & t = dSkiplist.Add();
					t.m_tBaseRowIDPlus1 = tSkiplistRowID+1;
					t.m_iOffset = tWriterDocs.GetPos();
					t.m_iBaseHitlistPos = uLastHitpos;
				}

				tWriterDocs.ZipOffset ( *pRow - tLastRowID );

				tLastRowID = *pRow;
				tSkiplistRowID = *pRow;
				iDocs++;
			}

			if ( bInlineHits )
			{
				const DWORD uMatchHits = tDoclist.UnzipInt();
				const DWORD uFirst = tDoclist.UnzipInt();
				if ( pRow )
				{
					 tWriterDocs.ZipInt ( uMatchHits );
					 tWriterDocs.ZipInt ( uFirst );
				}
				if ( uMatchHits==1 )
				{
					const DWORD uField = tDoclist.UnzipInt();
					if ( pRow )
						tWriterDocs.ZipInt ( uField );
				} else
				{
					const SphOffset_t uHitPosDelta = tDoclist.UnzipOffset();
					assert ( uHitPosDelta>=0 );
					uLastHitpos += uHitPosDelta;
					if ( pRow )
						tWriterDocs.ZipOffset ( uHitPosDelta );
				}
			} else
			{
				const SphOffset_t uHitPosDelta = tDoclist.UnzipOffset();
				assert ( uHitPosDelta>=0 );
				const DWORD uMatchHits = tDoclist.UnzipInt();
				uLastHitpos += uHitPosDelta;
				if ( pRow )
				{
					tWriterDocs.ZipOffset ( uHitPosDelta );
					tWriterDocs.ZipInt ( uMatchHits );
				}
			}
		}

		// write skiplist
		SphOffset_t uSkip = (int)tWriterSkips.GetPos();
		for ( int i=1; i<dSkiplist.GetLength(); i++ )
		{
			const SkiplistEntry_t & tPrev = dSkiplist[i-1];
			const SkiplistEntry_t & tCur = dSkiplist[i];
			assert ( tCur.m_tBaseRowIDPlus1 - tPrev.m_tBaseRowIDPlus1>=uSkiplistBlock );
			assert ( tCur.m_iOffset - tPrev.m_iOffset>=4*uSkiplistBlock );
			tWriterSkips.ZipInt ( tCur.m_tBaseRowIDPlus1 - tPrev.m_tBaseRowIDPlus1 - uSkiplistBlock );
			tWriterSkips.ZipOffset ( tCur.m_iOffset - tPrev.m_iOffset - 4*uSkiplistBlock );
			tWriterSkips.ZipOffset ( tCur.m_iBaseHitlistPos - tPrev.m_iBaseHitlistPos );
		}

		DoclistOffsets_t tOffsets;
		tOffsets.m_uDoclist = uNewDoclist;
		tOffsets.m_uSkiplist = uSkip;
		m_hDoclist.Add ( uOldDoclist, tOffsets );
	}

	return true;
}

bool ConverterPlain_t::ConvertDictionary ( Index_t & tIndex, CSphString & sError )
{
	CSphAutoreader tReaderDict;
	if ( !tReaderDict.Open ( GetIndexFileName ( tIndex.m_sPath,  "spi" ).cstr(), sError ) )
		return false;
	tReaderDict.SeekTo ( 1, 0 );

	CSphWriter tWriterDict;
	CSphString sDictName = tIndex.GetFilename(SPH_EXT_SPI);
	tWriterDict.OpenFile ( sDictName.cstr(), sError );
	tWriterDict.PutByte ( 1 );

	const SphOffset_t iEndDict = tIndex.m_tWordlist.m_iWordsEnd;
	const bool bWordDict = tIndex.m_tDictSettings.m_bWordDict;
	const bool bHasMorphology = tIndex.m_pDict->HasMorphology();

	SphOffset_t uDoclistOffCurLast = 0;
	SphOffset_t uDoclistOffNewLast = 0;
	SphWordID_t uWordid = 0;
	int iWords = 0;
	BYTE sDictWord[MAX_KEYWORD_BYTES+1];

	while ( tReaderDict.GetPos()<iEndDict )
	{
		SphOffset_t uDictPos = tWriterDict.GetPos();
		SphWordID_t iDeltaWord = 0;
		if ( bWordDict )
			iDeltaWord = tReaderDict.GetByte();
		else
			iDeltaWord = tReaderDict.UnzipWordid();

		// checkpoint encountered, handle it
		if ( !iDeltaWord )
		{
			int iDeltaDocs = tReaderDict.UnzipOffset();
			uWordid = 0;
			uDoclistOffCurLast = 0;
			uDoclistOffNewLast = 0;
			if ( bWordDict )
				tWriterDict.PutByte ( 0 );
			else
				tWriterDict.ZipOffset ( 0 );
			tWriterDict.ZipOffset ( iDeltaDocs );
			continue;
		}

		int iDocs = 0;
		int iHits = 0;
		DoclistOffsets_t * pOff = nullptr;

		if ( bWordDict )
			tWriterDict.PutByte ( iDeltaWord );
		else
			tWriterDict.ZipOffset ( iDeltaWord );

		if ( bWordDict )
		{
			// unpack next word
			// must be in sync with DictEnd()!
			BYTE uPack = (BYTE)iDeltaWord;
			int iMatch, iDelta;
			if ( uPack & 0x80 )
			{
				iDelta = ( ( uPack>>4 ) & 7 ) + 1;
				iMatch = uPack & 15;
			} else
			{
				iDelta = uPack & 127;
				iMatch = tReaderDict.GetByte();
				tWriterDict.PutByte ( iMatch );
			}

			tReaderDict.GetBytes ( sDictWord+1+iMatch, iDelta );
			sDictWord [ iMatch+iDelta+1 ] = '\0';
			sDictWord[0] = iMatch+iDelta;
			tWriterDict.PutBytes ( sDictWord+1+iMatch, iDelta );

			SphOffset_t uDoclistOffset = tReaderDict.UnzipOffset();
			pOff = m_hDoclist.Find ( uDoclistOffset );
			assert ( pOff );
			tWriterDict.ZipOffset ( pOff->m_uDoclist );

			iDocs = tReaderDict.UnzipInt();
			iHits = tReaderDict.UnzipInt();
			tWriterDict.ZipInt ( iDocs );
			tWriterDict.ZipInt ( iHits );
			if ( iDocs>=DOCLIST_HINT_THRESH )
			{
				BYTE uHint = tReaderDict.GetByte();
				tWriterDict.PutByte ( uHint );
			}

			// build infixes
			if ( m_pInfixer )
				m_pInfixer->AddWord ( sDictWord+1, sDictWord[0], m_dCheckpoints.GetLength(), bHasMorphology );
		} else
		{
			// finish reading the entire entry
			uWordid = uWordid + iDeltaWord;
			uDoclistOffCurLast = uDoclistOffCurLast + tReaderDict.UnzipOffset();
			iDocs = tReaderDict.UnzipInt();
			iHits = tReaderDict.UnzipInt();

			pOff = m_hDoclist.Find ( uDoclistOffCurLast );
			assert ( pOff );
			assert ( pOff->m_uDoclist>uDoclistOffNewLast );
			SphOffset_t uOffDelta = pOff->m_uDoclist - uDoclistOffNewLast;
			uDoclistOffNewLast = pOff->m_uDoclist;
			tWriterDict.ZipOffset ( uOffDelta );
			tWriterDict.ZipInt ( iDocs );
			tWriterDict.ZipInt ( iHits );
		}

		// skiplist
		if ( iDocs>(int)SPH_SKIPLIST_BLOCK )
			tReaderDict.UnzipInt();

		if ( iDocs>tIndex.m_tSettings.m_iSkiplistBlockSize )
			tWriterDict.ZipInt ( pOff->m_uSkiplist );

		if ( ( iWords%SPH_WORDLIST_CHECKPOINT )==0 )
		{
			// begin new wordlist entry
			Checkpoint_t & tCP = m_dCheckpoints.Add();
			if ( bWordDict )
			{
				tCP.m_uOffset = uDictPos;
				tCP.m_uWord = sphPutBytes ( &m_dKeywordCheckpoints, sDictWord, sDictWord[0]+1 ); // copy word len + word itself to checkpoint storage
			} else
			{
				tCP.m_uOffset = uDictPos;
				tCP.m_uWord = uWordid;
			}
		}

		iWords++;
	}

	WriteCheckpoints ( tIndex, tWriterDict );

	return true;
}


void ConverterPlain_t::WriteCheckpoints ( const Index_t & tIndex, CSphWriter & tWriterDict )
{
	const bool bKeywordDict = tIndex.m_tDictSettings.m_bWordDict;

	// flush infix hash entries, if any
	if ( m_pInfixer )
		m_pInfixer->SaveEntries ( tWriterDict );

	m_tCheckpointsPosition = tWriterDict.GetPos();
	if ( bKeywordDict )
	{
		const char * pCheckpoints = (const char *)m_dKeywordCheckpoints.Begin();
		for ( const auto & i : m_dCheckpoints )
		{
			const char * pPacked = pCheckpoints + i.m_uWord;
			int iLen = *pPacked;
			assert ( iLen && (int)i.m_uWord+1+iLen<=m_dKeywordCheckpoints.GetLength() );
			tWriterDict.PutDword ( iLen );
			tWriterDict.PutBytes ( pPacked+1, iLen );
			tWriterDict.PutOffset ( i.m_uOffset );
		}
	} else
	{
		for ( const auto & i : m_dCheckpoints )
		{
			tWriterDict.PutOffset ( i.m_uWord );
			tWriterDict.PutOffset ( i.m_uOffset );
		}
	}

	// flush infix hash blocks
	if ( m_pInfixer )
	{
		m_iInfixBlockOffset = m_pInfixer->SaveEntryBlocks ( tWriterDict );
		m_iInfixCheckpointWordsSize = m_pInfixer->GetBlocksWordsSize();

		if ( m_iInfixBlockOffset>UINT_MAX )
			sphWarning ( "INTERNAL ERROR: dictionary size " INT64_FMT " overflow at infix save", m_iInfixBlockOffset );
	}

	// flush header
	// mostly for debugging convenience
	// primary storage is in the index wide header
	if ( bKeywordDict )
	{
		tWriterDict.PutBytes ( "dict-header", 11 );
		tWriterDict.ZipInt ( m_dCheckpoints.GetLength() );
		tWriterDict.ZipOffset ( m_tCheckpointsPosition );
		tWriterDict.ZipInt ( tIndex.m_pTokenizer->GetMaxCodepointLength() );
		tWriterDict.ZipInt ( (DWORD)m_iInfixBlockOffset );
	}
}


bool ConverterPlain_t::Save ( const CSphVector<SphDocID_t> & dKilled, Index_t & tIndex, bool bIgnoreKlist, CSphString & sError )
{
	if ( !Init ( tIndex, sError ) )
		return false;

	if ( !WriteAttributes ( tIndex, sError ) )
		return false;

	if ( !ConvertDoclist ( tIndex, sError ) )
		return false;

	if ( !ConvertDictionary ( tIndex, sError ) )
		return false;

	if ( !WriteKillList ( tIndex, bIgnoreKlist, sError ) )
		return false;

	// dead row map save
	CSphBitvec dRowmap ( tIndex.m_iTotalDocuments );
	ARRAY_FOREACH ( i, dKilled )
	{
		RowID_t * pRow = m_hDoc2Row.Find ( dKilled[i] );
		if ( pRow )
			dRowmap.BitSet ( *pRow );
	}
	CSphString sRowMapName = tIndex.GetFilename(SPH_EXT_SPM);
	CSphWriter tRowMapWriter;
	if ( !tRowMapWriter.OpenFile ( sRowMapName, sError ) )
		return false;

	tRowMapWriter.PutBytes ( dRowmap.Begin(), dRowmap.GetSizeBytes() );
	tRowMapWriter.CloseFile();

	SaveHeader ( tIndex, 0 );

	return true;
}


void ConverterPlain_t::SaveHeader ( const Index_t & tIndex, DWORD uKillListSize ) const
{
	CSphWriter tWriter;
	CSphString sError;

	CSphString sName = tIndex.GetFilename(SPH_EXT_SPH);
	tWriter.OpenFile ( sName.cstr(), sError );

	// format
	tWriter.PutDword ( INDEX_MAGIC_HEADER );
	tWriter.PutDword ( INDEX_FORMAT_VERSION );

	// schema
	WriteSchema ( tWriter, m_tSchema );

	// wordlist checkpoints
	tWriter.PutOffset ( m_tCheckpointsPosition );
	tWriter.PutDword ( m_dCheckpoints.GetLength() );

	int iInfixCodepointBytes = ( tIndex.m_tSettings.m_iMinInfixLen && tIndex.m_pDict->GetSettings().m_bWordDict ? tIndex.m_pTokenizer->GetMaxCodepointLength() : 0 );
	tWriter.PutByte ( iInfixCodepointBytes ); // m_iInfixCodepointBytes, v.27+
	tWriter.PutDword ( m_iInfixBlockOffset ); // m_iInfixBlocksOffset, v.27+
	tWriter.PutDword ( m_iInfixCheckpointWordsSize ); // m_iInfixCheckpointWordsSize, v.34+

	// stats
	tWriter.PutDword ( tIndex.m_iTotalDocuments );
	tWriter.PutOffset ( tIndex.m_iTotalBytes );

	// index settings
	tWriter.PutDword ( tIndex.m_tSettings.RawMinPrefixLen() );
	tWriter.PutDword ( tIndex.m_tSettings.m_iMinInfixLen );
	tWriter.PutDword ( tIndex.m_tSettings.m_iMaxSubstringLen );
	tWriter.PutByte ( tIndex.m_tSettings.m_bHtmlStrip ? 1 : 0 );
	tWriter.PutString ( tIndex.m_tSettings.m_sHtmlIndexAttrs.cstr () );
	tWriter.PutString ( tIndex.m_tSettings.m_sHtmlRemoveElements.cstr () );
	tWriter.PutByte ( tIndex.m_tSettings.m_bIndexExactWords ? 1 : 0 );
	tWriter.PutDword ( tIndex.m_tSettings.m_eHitless );
	tWriter.PutDword ( SPH_HIT_FORMAT_INLINE );
	tWriter.PutByte ( tIndex.m_tSettings.m_bIndexSP ? 1 : 0 );
	tWriter.PutString ( tIndex.m_tSettings.m_sZones );
	tWriter.PutDword ( tIndex.m_tSettings.m_iBoundaryStep );
	tWriter.PutDword ( tIndex.m_tSettings.m_iStopwordStep );
	tWriter.PutDword ( tIndex.m_tSettings.m_iOvershortStep );
	tWriter.PutDword ( tIndex.m_tSettings.m_iEmbeddedLimit );
	tWriter.PutByte ( tIndex.m_tSettings.m_eBigramIndex );
	tWriter.PutString ( tIndex.m_tSettings.m_sBigramWords );
	tWriter.PutByte ( tIndex.m_tSettings.m_bIndexFieldLens );
	tWriter.PutByte ( tIndex.m_tSettings.m_ePreprocessor==Preprocessor_e::ICU ? 1 : 0 );
	tWriter.PutString("");	// was: rlp context
	tWriter.PutString ( tIndex.m_tSettings.m_sIndexTokenFilter );
	tWriter.PutOffset ( tIndex.m_tSettings.m_tBlobUpdateSpace );
	tWriter.PutDword ( tIndex.m_tSettings.m_iSkiplistBlockSize );
	tWriter.PutString ( "" ); // tSettings.m_sHitlessFiles

	// tokenizer
	SaveTokenizerSettings ( tWriter, tIndex.m_pTokenizer, tIndex.m_tSettings.m_iEmbeddedLimit );

	// dictionary
	SaveDictionarySettings ( tWriter, tIndex.m_pDict, tIndex.m_pDict->GetSettings().m_bWordDict, tIndex.m_tSettings.m_iEmbeddedLimit );

	tWriter.PutOffset ( tIndex.m_iTotalDocuments );
	tWriter.PutOffset ( m_tDocinfoIndex );
	tWriter.PutOffset ( m_tMinMaxPos/sizeof(CSphRowitem) );

	// field filter
	tIndex.m_tFieldFilterSettings.Save(tWriter);

	// field lengths
	if ( tIndex.m_tSettings.m_bIndexFieldLens )
		for ( int i=0; i <m_tSchema.GetFieldsCount(); i++ )
			tWriter.PutOffset ( tIndex.m_dFieldLens[i] );

	// done
	tWriter.CloseFile ();
}

static void CopyAndUpdateSchema ( const Index_t & tIndex, CSphSchema & tSchema )
{
	ARRAY_FOREACH ( i, tIndex.m_dSchemaFields )
		tSchema.AddField ( tIndex.m_dSchemaFields[i] );

	CSphColumnInfo tCol ( sphGetDocidName() );
	tCol.m_eAttrType = SPH_ATTR_BIGINT;
	tSchema.InsertAttr ( 0, tCol, false );

	ARRAY_FOREACH ( i, tIndex.m_dSchemaAttrs )
		tSchema.AddAttr ( tIndex.m_dSchemaAttrs[i], false );

	if ( tSchema.HasBlobAttrs() )
	{
		CSphColumnInfo tBlobLocatorCol ( sphGetBlobLocatorName() );
		tBlobLocatorCol.m_eAttrType = SPH_ATTR_BIGINT;

		// should be right after docid
		tSchema.InsertAttr ( 1, tBlobLocatorCol, false );

		// rebuild locators in the schema
		const char * szTmpColName = "$_tmp";
		CSphColumnInfo tColTmp ( szTmpColName, SPH_ATTR_BIGINT );
		tSchema.AddAttr ( tColTmp, false );
		tSchema.RemoveAttr ( szTmpColName, false );
	}
}

bool ConverterPlain_t::Init ( Index_t & tIndex, CSphString & sError )
{
	// merge index settings with new defaults
	CSphConfigSection hIndex;
	CSphIndexSettings tDefaultSettings;
	CSphString sWarning;
	if ( !tDefaultSettings.Setup ( hIndex, tIndex.m_sName.cstr(), sWarning, sError ) )
		return false;

	if ( !sWarning.IsEmpty() )
		sphWarning ( "%s", sWarning.cstr() );

	tIndex.m_tSettings.m_tBlobUpdateSpace = tDefaultSettings.m_tBlobUpdateSpace;
	tIndex.m_tSettings.m_iSkiplistBlockSize = tDefaultSettings.m_iSkiplistBlockSize;

	// old schema to new schema
	CopyAndUpdateSchema ( tIndex, m_tSchema );

	if ( tIndex.m_tSettings.m_iMinInfixLen && tIndex.m_pDict->GetSettings().m_bWordDict )
		m_pInfixer = sphCreateInfixBuilder ( tIndex.m_pTokenizer->GetMaxCodepointLength(), &sError );

	return ( sError.IsEmpty() );
}

bool ConverterPlain_t::ConvertSchema ( Index_t & tIndex, CSphString & sError )
{
	if ( !Init ( tIndex, sError ) )
		return false;

	tIndex.m_tSchema = m_tSchema;
	return true;
}

bool ConverterPlain_t::WriteKillList ( const Index_t & tIndex, bool bIgnoreKlist, CSphString & sError )
{
	CSphVector<DocID_t> dKillList;
	if ( !bIgnoreKlist )
	{
		dKillList.Resize ( tIndex.m_tKillList.GetLength () );
		ARRAY_FOREACH ( i, dKillList )
			dKillList[i] = tIndex.m_tKillList.GetReadPtr()[i];
	}

	CSphString sName = tIndex.GetFilename(SPH_EXT_SPK);

	if ( !::WriteKillList ( sName, dKillList.Begin(), dKillList.GetLength(), tIndex.m_tKlistTargets, sError ) )
		return false;

	WarnAboutKillList ( dKillList, tIndex.m_tKlistTargets );

	return true;
}

// hitlist (spp file) is same
static const char * g_dExtsOld[] = { ".sph", ".spa", ".spi", ".spd", ".spm", ".spk", ".sps", ".spe", ".mvp" };

struct ExtInfo_t
{
	CSphString	m_sExt;
	bool		m_bOptional;
};


static CSphVector<ExtInfo_t> GetExts ( const char * szPrefix, bool bOldFormat )
{
	CSphVector<ExtInfo_t> dResult;
	if ( bOldFormat )
	{
		int iCount = sizeof(g_dExtsOld) / sizeof ( g_dExtsOld[0] );
		for ( int i = 0; i<iCount; i++ )
		{
			ExtInfo_t & tExt = dResult.Add();
			tExt.m_sExt.SetSprintf ( "%s%s", szPrefix, g_dExtsOld[i] );
			tExt.m_bOptional = i==iCount-1;	// MVP
		}
	}
	else
	{
		auto dExts = sphGetExts();
		for ( const auto & i : dExts )
			if ( i.m_eExt!=SPH_EXT_SPP )
			{
				ExtInfo_t & tExt = dResult.Add();
				tExt.m_sExt.SetSprintf ( "%s%s", szPrefix, i.m_szExt );
				tExt.m_bOptional = i.m_bOptional;
			}
	}

	return dResult;
}


static bool TryRename ( const char * sPrefix, const char * sFromPostfix, const char * sToPostfix, const char * sAction, CSphString & sError )
{
	char sFrom [ SPH_MAX_FILENAME_LEN ];
	char sTo [ SPH_MAX_FILENAME_LEN ];

	snprintf ( sFrom, sizeof(sFrom), "%s%s", sPrefix, sFromPostfix );
	snprintf ( sTo, sizeof(sTo), "%s%s", sPrefix, sToPostfix );

#if _WIN32
	::unlink ( sTo );
#endif

	if ( rename ( sFrom, sTo ) )
	{
		sError.SetSprintf ( "%s: rename '%s' to '%s' failed: %s", sAction, sFrom, sTo, strerror(errno) );
		return false;
	}

	return true;
}


static void RollbackRename ( const CSphBitvec & dProcessed, const CSphString & sPath, const CSphVector<ExtInfo_t> & dCur, const CSphVector<ExtInfo_t> & dTo, CSphString & sError )
{
	CSphString sFilename;
	for ( int i=0; i<dCur.GetLength(); i++ )
	{
		if ( !dProcessed.BitGet ( i ) )
			continue;

		const char * sCurExt = dCur[i].m_sExt.cstr();
		const char * sToExt = dTo[i].m_sExt.cstr();

		sFilename.SetSprintf ( "%s%s", sPath.cstr(), sCurExt );
		TryRename ( sPath.cstr(), sCurExt, sToExt, "rollback", sError );
	}
}

static bool RenameIndex ( const CSphString & sPath, const char * sAction, bool bOldFormat, const char * szPrefix1, const char * szPrefix2, CSphString & sError )
{
	CSphVector<ExtInfo_t> dNextExts = GetExts ( szPrefix2, bOldFormat );
	CSphVector<ExtInfo_t> dCurExts = GetExts ( szPrefix1, bOldFormat );
	assert ( dCurExts.GetLength()==dNextExts.GetLength() );

	bool bError = false;
	CSphString sFilename;
	CSphBitvec dProcessed ( dCurExts.GetLength() );

	for ( int i=0; i<dCurExts.GetLength(); i++ )
	{
		const char * sCurExt = dCurExts[i].m_sExt.cstr();
		const char * sNextExt = dNextExts[i].m_sExt.cstr();

		sFilename.SetSprintf ( "%s%s", sPath.cstr(), sCurExt );
		if ( sphIsReadable ( sFilename.cstr() ) )
		{
			if ( !TryRename ( sPath.cstr(), sCurExt, sNextExt, sAction, sError ) )
			{
				bError = true;
				break;
			}

			dProcessed.BitSet(i);
		}
		else if ( !dCurExts[i].m_bOptional )
		{
			bError = true;
			break;
		}
	}

	if ( bError )
	{
		RollbackRename ( dProcessed, sPath, dNextExts, dCurExts, sError );
		return false;
	}

	return true;
}

static bool RotateIndexFiles ( const CSphString & sPathIn, const CSphString & sPathOut, CSphString & sError )
{
	assert ( sPathIn==sPathOut );

	// rename current to old
	if ( !RenameIndex ( sPathIn, "cur2old", true, "", ".old", sError ) )
		return false;

	// rename new to current
	if ( !RenameIndex ( sPathOut, "new2cur", false, ".new", "", sError ) )
		return false;

	return true;
}

static bool CopyHitlist ( const CSphString & sPathIn, const CSphString & sPathOut, CSphString & sError )
{
	CSphString sFrom;
	CSphString sTo;

	sFrom.SetSprintf ( "%s.spp", sPathIn.cstr() );
	sTo.SetSprintf ( "%s.spp", sPathOut.cstr() );

	CSphAutoreader tFromHit;
	if ( !tFromHit.Open ( sFrom, sError ) )
		return false;

	CSphWriter tToHit;
	if ( !tToHit.OpenFile ( sTo, sError ) )
		return false;

	int64_t iSize = tFromHit.GetFilesize();
	CSphFixedVector<BYTE> dBuf ( Min ( iSize, 4096 ) );

	while ( iSize )
	{
		int iReadSize = Min ( iSize, dBuf.GetLength() );
		tFromHit.GetBytes ( dBuf.Begin(), iReadSize );
		tToHit.PutBytes ( dBuf.Begin(), iReadSize );

		if ( tFromHit.GetErrorFlag() )
		{
			sError = tFromHit.GetErrorMessage();
			return false;
		}
		if ( tToHit.IsError() ) // sError already set as error buffer at writer
			return false;

		iSize -= iReadSize;
	}

	return true;
}


static bool LoadRtIndex ( Index_t & tIndex, CSphString & sError )
{
	// load meta
	CSphString sMetaName = GetIndexFileName ( tIndex.m_sPath, "meta" );

	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMetaName.cstr(), sError ) )
		return false;

	if ( rdMeta.GetDword()!=META_HEADER_MAGIC )
	{
		sError.SetSprintf ( "invalid meta file %s", sMetaName.cstr() );
		return false;
	}
	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion==0 || uVersion>META_VERSION || uVersion<6 )
	{
		sError.SetSprintf ( "%s is v.%d, binary is v.%d", sMetaName.cstr(), uVersion, META_VERSION );
		return false;
	}

	if ( uVersion>13 )
	{
		sError.SetSprintf ( "already a v3 index; nothing to do" );
		return false;
	}

	const int iDiskChunks = rdMeta.GetDword();
	int iDiskBase = rdMeta.GetDword();
	tIndex.m_iTotalDocuments = rdMeta.GetDword();
	tIndex.m_iTotalBytes = rdMeta.GetOffset();
	tIndex.m_iTID = rdMeta.GetOffset();

	CSphEmbeddedFiles tEmbeddedFiles;
	CSphString sWarning;

	// load them settings
	DWORD uSettingsVer = rdMeta.GetDword();
	ReadSchema ( rdMeta, tIndex.m_dSchemaFields, tIndex.m_dSchemaAttrs );
	LoadIndexSettings ( tIndex.m_tSettings, rdMeta, uSettingsVer );

	if ( !LoadTokenizerSettings ( rdMeta, tIndex.m_tTokSettings, tEmbeddedFiles, uSettingsVer, sError ) )
		return false;

	if ( tIndex.m_bStripPath )
		StripPath ( tIndex.m_tTokSettings.m_sSynonymsFile );

	LoadDictionarySettings ( rdMeta, tIndex.m_tDictSettings, tEmbeddedFiles, uSettingsVer, sWarning );
	if ( !sWarning.IsEmpty() )
	{
		sphWarning ( "%s", sWarning.cstr() );
		sWarning = "";
	}

	if ( tIndex.m_bStripPath )
	{
		StripPath ( tIndex.m_tDictSettings.m_sStopwords );
		ARRAY_FOREACH ( i, tIndex.m_tDictSettings.m_dWordforms )
			StripPath ( tIndex.m_tDictSettings.m_dWordforms[i] );
	}

	if ( !SetupWordProcessors ( tIndex, sError ) )
		return false;

	// meta v.5 checkpoint freq
	tIndex.m_iWordsCheckpoint = rdMeta.GetDword();

	// check that infixes definition changed - going to rebuild infixes
	if ( uVersion>=7 )
	{
		tIndex.m_iMaxCodepointLength = rdMeta.GetDword();
		rdMeta.GetByte(); // iBloomKeyLen
		rdMeta.GetByte(); // iBloomHashesCount
	}

	if ( uVersion>=11 )
		legacy::LoadFieldFilterSettings ( rdMeta, tIndex.m_tFieldFilterSettings );

	if ( uVersion>=12 )
	{
		int iLen = (int)rdMeta.GetDword();
		tIndex.m_dRtChunkNames.Reset ( iLen );
		rdMeta.GetBytes ( tIndex.m_dRtChunkNames.Begin(), iLen*sizeof(int) );
	}

	// prior to v.12 use iDiskBase + iDiskChunks
	// v.12 stores chunk list but wrong
	if ( uVersion<13 )
	{
		tIndex.m_dRtChunkNames.Reset ( iDiskChunks );
		ARRAY_FOREACH ( iChunk, tIndex.m_dRtChunkNames )
			tIndex.m_dRtChunkNames[iChunk] = iChunk + iDiskBase;
	}

	// load ram

	CSphString sRamName = GetIndexFileName ( tIndex.m_sPath, "ram" );
	if ( sphIsReadable ( sRamName.cstr(), &sError ) )
	{
		CSphAutoreader rdChunk;
		if ( !rdChunk.Open ( sRamName, sError ) )
			return false;

		if ( !rdChunk.GetDword () ) // !Id64
		{
			sError = "indexes with 32-bit docids are no longer supported";
			return false;
		}

		tIndex.m_iSegmentSeq = rdChunk.GetDword();
		int iSegmentCount = rdChunk.GetDword();
		if ( iSegmentCount )
		{
			sError = "RT index could not be converted; run FLUSH RAMCHUNK <index_name> before conversion";
			return false;
		}

		// field lengths
		int iFields = rdChunk.GetDword();
		for ( int i=0; i<iFields; i++ )
			tIndex.m_dFieldLens[i] = rdChunk.GetOffset();
	}

	return true;
}


static bool RenameRtIndex ( Index_t & tIndex, CSphString & sError )
{
	CSphString sMetaNew;
	sMetaNew.SetSprintf ( "%s.new.meta", tIndex.m_sPathOut.cstr() );

	CSphString sChunkNew;
	sChunkNew.SetSprintf ( "%s.new.ram", tIndex.m_sPathOut.cstr() );

	CSphString sMetaTo, sChunkTo, sKillTo;
	sMetaTo.SetSprintf ( "%s.meta", tIndex.m_sPathOut.cstr() );
	sChunkTo.SetSprintf ( "%s.ram", tIndex.m_sPathOut.cstr() );
	sKillTo.SetSprintf ( "%s.kill", tIndex.m_sPathOut.cstr() );

	CSphString sMetaOld, sChunkOld, sKillOld;
	sMetaOld.SetSprintf ( "%s.old.meta", tIndex.m_sPathOut.cstr() );
	sChunkOld.SetSprintf ( "%s.old.ram", tIndex.m_sPathOut.cstr() );
	sKillOld.SetSprintf ( "%s.old.kill", tIndex.m_sPathOut.cstr() );

	// cur to old
	bool bHasRamChunk = sphIsReadable ( sChunkTo.cstr() );
	if ( bHasRamChunk && ::rename ( sChunkTo.cstr(), sChunkOld.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename ram chunk (src=%s, dst=%s, errno=%d, error=%s)", sChunkTo.cstr(), sChunkOld.cstr(), errno, strerror(errno) );
		return false;
	}

	if ( ::rename ( sMetaTo.cstr(), sMetaOld.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)", sMetaTo.cstr(), sMetaOld.cstr(), errno, strerror(errno) );
		return false;
	}

	if ( ::rename ( sKillTo.cstr(), sKillOld.cstr() ) && bHasRamChunk )
	{
		sError.SetSprintf ( "failed to rename killlist (src=%s, dst=%s, errno=%d, error=%s)", sKillTo.cstr(), sKillOld.cstr(), errno, strerror(errno) );
		return false;
	}

	// new to cur
	if ( ::rename ( sChunkNew.cstr(), sChunkTo.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename ram chunk (src=%s, dst=%s, errno=%d, error=%s)", sChunkNew.cstr(), sChunkTo.cstr(), errno, strerror(errno) );
		return false;
	}
	if ( ::rename ( sMetaNew.cstr(), sMetaTo.cstr() ) )
	{
		sError.SetSprintf ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)", sMetaNew.cstr(), sMetaTo.cstr(), errno, strerror(errno) );
		return false;
	}

	return true;
}


static bool SaveRtIndex ( Index_t & tIndex, CSphString & sWarning, CSphString & sError )
{
	// no disk chunks - need to copy old schema from meta and update it if necessary
	if ( !tIndex.m_dRtChunkNames.GetLength() )
		CopyAndUpdateSchema ( tIndex, tIndex.m_tSchema );

	// merge index settings with new defaults
	CSphConfigSection hIndex;
	CSphIndexSettings tDefaultSettings;
	if ( !tDefaultSettings.Setup ( hIndex, tIndex.m_sName.cstr(), sWarning, sError ) )
		return false;

	// write new meta
	CSphString sMetaNew;
	sMetaNew.SetSprintf ( "%s.new.meta", tIndex.m_sPathOut.cstr() );

	CSphWriter wrMeta;
	if ( !wrMeta.OpenFile ( sMetaNew, sError ) )
		return false;
	wrMeta.PutDword ( META_HEADER_MAGIC );
	wrMeta.PutDword ( META_VERSION );
	wrMeta.PutDword ( (DWORD)tIndex.m_iTotalDocuments ); // FIXME? we don't expect over 4G docs per just 1 local index
	wrMeta.PutOffset ( tIndex.m_iTotalBytes ); // FIXME? need PutQword ideally
	wrMeta.PutOffset ( tIndex.m_iTID );

	// meta v.4, save disk index format and settings, too
	wrMeta.PutDword ( INDEX_FORMAT_VERSION );
	WriteSchema ( wrMeta, tIndex.m_tSchema );

	// index settings
	wrMeta.PutDword ( tIndex.m_tSettings.RawMinPrefixLen() );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iMinInfixLen );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iMaxSubstringLen );
	wrMeta.PutByte ( tIndex.m_tSettings.m_bHtmlStrip ? 1 : 0 );
	wrMeta.PutString ( tIndex.m_tSettings.m_sHtmlIndexAttrs.cstr () );
	wrMeta.PutString ( tIndex.m_tSettings.m_sHtmlRemoveElements.cstr () );
	wrMeta.PutByte ( tIndex.m_tSettings.m_bIndexExactWords ? 1 : 0 );
	wrMeta.PutDword ( tIndex.m_tSettings.m_eHitless );
	wrMeta.PutDword ( SPH_HIT_FORMAT_INLINE );
	wrMeta.PutByte ( tIndex.m_tSettings.m_bIndexSP ? 1 : 0 );
	wrMeta.PutString ( tIndex.m_tSettings.m_sZones );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iBoundaryStep );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iStopwordStep );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iOvershortStep );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iEmbeddedLimit );
	wrMeta.PutByte ( tIndex.m_tSettings.m_eBigramIndex );
	wrMeta.PutString ( tIndex.m_tSettings.m_sBigramWords );
	wrMeta.PutByte ( tIndex.m_tSettings.m_bIndexFieldLens );
	wrMeta.PutByte ( tIndex.m_tSettings.m_ePreprocessor==Preprocessor_e::ICU ? 1 : 0 );
	wrMeta.PutString (""); // was: RLP context
	wrMeta.PutString ( tIndex.m_tSettings.m_sIndexTokenFilter );
	wrMeta.PutOffset ( tDefaultSettings.m_tBlobUpdateSpace );
	wrMeta.PutDword ( tDefaultSettings.m_iSkiplistBlockSize );
	wrMeta.PutString ( "" ); // tSettings.m_sHitlessFiles

	// tokenizer
	SaveTokenizerSettings ( wrMeta, tIndex.m_pTokenizer, tIndex.m_tSettings.m_iEmbeddedLimit );

	// dictionary
	SaveDictionarySettings ( wrMeta, tIndex.m_pDict, tIndex.m_pDict->GetSettings().m_bWordDict, tIndex.m_tSettings.m_iEmbeddedLimit );

	// meta v.5
	wrMeta.PutDword ( tIndex.m_iWordsCheckpoint );

	// meta v.7
	wrMeta.PutDword ( tIndex.m_iMaxCodepointLength );
	// should be rebuild on load
	wrMeta.PutByte ( 0 ); // BLOOM_PER_ENTRY_VALS_COUNT
	wrMeta.PutByte ( 0 ); // BLOOM_HASHES_COUNT

	// meta v.11
	tIndex.m_tFieldFilterSettings.Save(wrMeta);

	// meta v.12
	wrMeta.PutDword ( tIndex.m_dRtChunkNames.GetLength () );
	wrMeta.PutBytes ( tIndex.m_dRtChunkNames.Begin(), tIndex.m_dRtChunkNames.GetLengthBytes64 () );
	
	// meta v.17
	wrMeta.PutOffset ( DEFAULT_RT_MEM_LIMIT );

	wrMeta.CloseFile();

	// ram chunk
	CSphString sChunkNew;
	sChunkNew.SetSprintf ( "%s.new.ram", tIndex.m_sPathOut.cstr() );

	CSphWriter wrChunk;
	if ( !wrChunk.OpenFile ( sChunkNew, sError ) )
		return false;

	wrChunk.PutDword ( tIndex.m_iSegmentSeq );
	wrChunk.PutDword ( 0 ); // N of RAM segs

	// field lengths
	wrChunk.PutDword ( tIndex.m_dSchemaFields.GetLength() );
	ARRAY_FOREACH ( i, tIndex.m_dSchemaFields )
		wrChunk.PutOffset ( tIndex.m_dFieldLens[i] );

	wrChunk.CloseFile();

	if ( tIndex.m_sPath==tIndex.m_sPathOut && !RenameRtIndex ( tIndex, sError ) )
		return false;

	return true;
}


static bool ConvertPlain ( const CSphString & sName, const CSphString & sPath, bool bStripPath, CSphString & sError, const CSphVector<SphDocID_t> & dKilled, const CSphString & sPathOut, const KillListTargets_c & tKlistTargets, Index_t * pRtIndex, bool bIgnoreKlist=false )
{
	// need scope for destructor
	{
		Index_t tIndex;
		tIndex.m_sName = sName;
		tIndex.m_sPath = sPath;
		tIndex.m_sPathOut = sPathOut;
		tIndex.m_bStripPath = bStripPath;
		tIndex.m_tKlistTargets = tKlistTargets;

		bool bLoaded = LoadPlainIndexChunk ( tIndex, sError );
		if ( !bLoaded )
		{
			sError.SetSprintf ( "failed to load index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}

		ConverterPlain_t tConverter;
		if ( !tConverter.Save ( dKilled, tIndex, bIgnoreKlist, sError ) )
		{
			sError.SetSprintf ( "failed to convert index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}

		if ( pRtIndex )
			pRtIndex->m_tSchema = tConverter.GetSchema();
	}

	// rename only in case output-dir set
	if ( sPath==sPathOut )
	{
		if ( !RotateIndexFiles ( sPath, sPathOut, sError ) )
		{
			sError.SetSprintf ( "failed to rename index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}
	} else 
	{
		if ( !CopyHitlist ( sPath, sPathOut, sError ) )
		{
			sError.SetSprintf ( "failed to copy hitlist index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}
	}

	return true;
}

static void GetKilledDocs ( const CSphFixedVector<int> & dRtChunkNames, int iCurrentChunk, const CSphString & sPath, CSphVector<SphDocID_t> & dKilled )
{
	dKilled.Resize ( 0 );

	CSphString sError;
	CSphString sKillName;
	for ( int i=iCurrentChunk+1; i<dRtChunkNames.GetLength(); i++ )
	{
		sKillName.SetSprintf ( "%s.%d.spk", sPath.cstr(), dRtChunkNames[i] );

		CSphAutoreader tKill;
		if ( sphIsReadable ( sKillName.cstr() ) && !tKill.Open ( sKillName.cstr(), sError ) )
		{
			sphWarning ( "%s", sError.cstr() );
			continue;
		}

		auto iCount = (int) (tKill.GetFilesize() / sizeof(SphDocID_t));
		if ( !iCount )
			continue;

		int iOff = dKilled.GetLength();
		dKilled.Resize ( iOff + (int) iCount );
		for ( int iElem=0; iElem<iCount; iElem++ )
			dKilled[iOff+iElem] = tKill.GetOffset();
	}

	dKilled.Uniq(); // get rid of duplicates
}

static bool LoadPqIndex ( Index_t & tIndex, CSphString & sError )
{
	// load meta
	CSphString sMetaName = GetIndexFileName ( tIndex.m_sPath, "meta" );

	CSphAutoreader rdMeta;
	if ( !rdMeta.Open ( sMetaName.cstr(), sError ) )
		return false;

	if ( rdMeta.GetDword()!=PQ_META_HEADER_MAGIC )
	{
		sError.SetSprintf ( "invalid meta file %s", sMetaName.cstr() );
		return false;
	}
	DWORD uVersion = rdMeta.GetDword();
	if ( uVersion>PQ_META_VERSION )
	{
		sError.SetSprintf ( "already a v3 index; nothing to do" );
		return false;
	}
	if ( uVersion==0 || uVersion>PQ_META_VERSION )
	{
		sError.SetSprintf ( "%s is v.%d, binary is v.%d", sMetaName.cstr(), uVersion, PQ_META_VERSION );
		return false;
	}

	DWORD uSettingsVer = rdMeta.GetDword();

	CSphTokenizerSettings tTokenizerSettings;
	CSphDictSettings tDictSettings;

	// load settings
	legacy::ReadSchema ( rdMeta, tIndex.m_dSchemaFields, tIndex.m_dSchemaAttrs );
	legacy::LoadIndexSettings ( tIndex.m_tSettings, rdMeta, uSettingsVer );
	if ( !legacy::LoadTokenizerSettings ( rdMeta, tIndex.m_tTokSettings, tIndex.m_tEmbeddedTok, uSettingsVer, sError ) )
		return false;

	legacy::LoadDictionarySettings ( rdMeta, tIndex.m_tDictSettings, tIndex.m_tEmbeddedDict, uSettingsVer, sError );

	if ( !SetupWordProcessors ( tIndex, sError ) )
		return false;

	if ( uVersion>=6 )
		legacy::LoadFieldFilterSettings ( rdMeta, tIndex.m_tFieldFilterSettings );

	// queries
	DWORD uQueries = rdMeta.GetDword();
	tIndex.m_dStored.Reset ( uQueries );
	ARRAY_FOREACH ( i, tIndex.m_dStored )
	{
		StoredQueryDesc_t & tQuery = tIndex.m_dStored[i];
		if ( uVersion<7 )
			LoadStoredQueryV6 ( uVersion, tQuery, rdMeta );
		else
			LoadStoredQuery ( uVersion, tQuery, rdMeta );
	}
	if ( uVersion>=7 )
		tIndex.m_iTID = rdMeta.GetOffset();

	if ( rdMeta.GetErrorFlag() )
	{
		sError = rdMeta.GetErrorMessage();
		return false;
	}

	return true;
}

static bool SavePqIndex ( Index_t & tIndex, CSphString & sWarning, CSphString & sError )
{
	ConverterPlain_t tConverter;
	if ( !tConverter.ConvertSchema ( tIndex, sError ) )
		return false;

	// merge index settings with new defaults
	CSphConfigSection hIndex;
	CSphIndexSettings tDefaultSettings;
	if ( !tDefaultSettings.Setup ( hIndex, tIndex.m_sName.cstr(), sWarning, sError ) )
		return false;

	// write new meta
	CSphString sMetaNew;
	sMetaNew.SetSprintf ( "%s.new.meta", tIndex.m_sPathOut.cstr() );

	CSphWriter wrMeta;
	if ( !wrMeta.OpenFile ( sMetaNew, sError ) )
		return false;
	wrMeta.PutDword ( PQ_META_HEADER_MAGIC );
	wrMeta.PutDword ( PQ_META_VERSION+1 );
	wrMeta.PutDword ( INDEX_FORMAT_VERSION );

	WriteSchema ( wrMeta, tIndex.m_tSchema );

	// index settings
	wrMeta.PutDword ( tIndex.m_tSettings.RawMinPrefixLen() );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iMinInfixLen );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iMaxSubstringLen );
	wrMeta.PutByte ( tIndex.m_tSettings.m_bHtmlStrip ? 1 : 0 );
	wrMeta.PutString ( tIndex.m_tSettings.m_sHtmlIndexAttrs.cstr () );
	wrMeta.PutString ( tIndex.m_tSettings.m_sHtmlRemoveElements.cstr () );
	wrMeta.PutByte ( tIndex.m_tSettings.m_bIndexExactWords ? 1 : 0 );
	wrMeta.PutDword ( tIndex.m_tSettings.m_eHitless );
	wrMeta.PutDword ( tIndex.m_tSettings.m_eHitFormat );
	wrMeta.PutByte ( tIndex.m_tSettings.m_bIndexSP );
	wrMeta.PutString ( tIndex.m_tSettings.m_sZones );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iBoundaryStep );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iStopwordStep );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iOvershortStep );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iEmbeddedLimit );
	wrMeta.PutByte ( tIndex.m_tSettings.m_eBigramIndex );
	wrMeta.PutString ( tIndex.m_tSettings.m_sBigramWords );
	wrMeta.PutByte ( tIndex.m_tSettings.m_bIndexFieldLens );
	wrMeta.PutByte ( tIndex.m_tSettings.m_ePreprocessor==Preprocessor_e::ICU ? 1 : 0 );
	wrMeta.PutString("");	// was: RLP context
	wrMeta.PutString ( tIndex.m_tSettings.m_sIndexTokenFilter );
	wrMeta.PutOffset ( tIndex.m_tSettings.m_tBlobUpdateSpace );
	wrMeta.PutDword ( tIndex.m_tSettings.m_iSkiplistBlockSize );

	SaveTokenizerSettings ( wrMeta, tIndex.m_pTokenizer, tIndex.m_tSettings.m_iEmbeddedLimit );
	SaveDictionarySettings ( wrMeta, tIndex.m_pDict, false, tIndex.m_tSettings.m_iEmbeddedLimit );

	tIndex.m_tFieldFilterSettings.Save(wrMeta);

	wrMeta.PutDword ( tIndex.m_dStored.GetLength() );

	ARRAY_FOREACH ( i, tIndex.m_dStored )
		SaveStoredQuery ( tIndex.m_dStored[i], wrMeta );

	wrMeta.PutOffset ( tIndex.m_iTID );

	wrMeta.CloseFile();

	if ( tIndex.m_sPath==tIndex.m_sPathOut )
	{
		CSphString sMetaTo;
		sMetaTo.SetSprintf ( "%s.meta", tIndex.m_sPathOut.cstr() );
		CSphString sMetaOld;
		sMetaOld.SetSprintf ( "%s.old.meta", tIndex.m_sPathOut.cstr() );

		if ( ::rename ( sMetaTo.cstr(), sMetaOld.cstr() ) )
		{
			sError.SetSprintf ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)", sMetaTo.cstr(), sMetaOld.cstr(), errno, strerror(errno) );
			return false;
		}
		if ( ::rename ( sMetaNew.cstr(), sMetaTo.cstr() ) )
		{
			sError.SetSprintf ( "failed to rename meta (src=%s, dst=%s, errno=%d, error=%s)", sMetaNew.cstr(), sMetaTo.cstr(), errno, strerror(errno) );
			return false;
		}
	}

	return true;
}


static bool Convert ( const CSphString & sName, const CSphString & sPath, IndexType_e eType, bool bStripPath, const CSphString & sPathOut, const KillListTargets_c & dKlistTargets, CSphString & sError )
{
	if ( eType==INDEX_UNKNOWN )
	{
		if ( sphIsReadable ( GetIndexFileName( sPath, "spa" ).cstr() ) )
			eType = INDEX_PLAIN;
		else if ( sphIsReadable ( GetIndexFileName( sPath, "meta" ).cstr() ) && sphIsReadable ( GetIndexFileName( sPath, "ram" ).cstr() ) )
			eType = INDEX_RT;
		else if ( sphIsReadable ( GetIndexFileName( sPath, "meta" ).cstr() ) )
			eType = INDEX_PQ;
	}

	if ( eType==INDEX_UNKNOWN )
	{
		sError.SetSprintf ( "unknown index type '%s'", sName.cstr() );
		return false;
	}

	printf ( "converting %s index '%s'\n", g_sIndexType[eType], sName.cstr() );

	if ( eType==INDEX_RT )
	{
		Index_t tIndex;
		tIndex.m_sName = sName;
		tIndex.m_sPath = sPath;
		tIndex.m_sPathOut = sPathOut;
		tIndex.m_bStripPath = bStripPath;

		if ( !LoadRtIndex ( tIndex, sError ) )
		{
			sError.SetSprintf ( "failed to load index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}

		CSphVector<SphDocID_t> dKilled;
		int iChunk = 0;
		CSphString sChunkInPath, sChunkOutPath;
		for ( ; iChunk<tIndex.m_dRtChunkNames.GetLength(); iChunk++ )
		{
			GetKilledDocs ( tIndex.m_dRtChunkNames, iChunk, sPath, dKilled );

			sChunkInPath.SetSprintf ( "%s.%d", sPath.cstr(), tIndex.m_dRtChunkNames[iChunk] );
			sChunkOutPath.SetSprintf ( "%s.%d", sPathOut.cstr(), tIndex.m_dRtChunkNames[iChunk] );
			Index_t * pSchema = ( iChunk==tIndex.m_dRtChunkNames.GetLength() - 1 ? &tIndex : nullptr );
			if ( !ConvertPlain ( sName, sChunkInPath, bStripPath, sError, dKilled, sChunkOutPath, KillListTargets_c(), pSchema, true ) )
			{
				sphWarning ( "failed to convert %d disk chunk, error: %s, renaming original disk chunks back ...", iChunk, sError.cstr() );
				break;
			}
		}

		if ( iChunk!=tIndex.m_dRtChunkNames.GetLength() )
		{
			// rename back converted files from old to current up to failed iChunk but not iChunk itself
			for ( int iRoll=0; iRoll<iChunk && tIndex.IsSeparateOutDir(); iRoll++ )
			{
				sChunkInPath.SetSprintf ( "%s.%d", sPath.cstr(), tIndex.m_dRtChunkNames[iRoll] );
				// rename current to old
				if ( !RenameIndex ( sPath, "old2cur", true, ".old", "", sError ) )
				{
					sphWarning ( "failed to rename old back to current at %d disk chunk, fix it manually, error: %s", iRoll, sError.cstr() );
					break;
				}
			}

			sError.SetSprintf ( "conversion failed for index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}

		CSphString sWarning;
		if ( !SaveRtIndex ( tIndex, sWarning, sError ) )
		{
			sError.SetSprintf ( "conversion failed for index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}

		if ( !sWarning.IsEmpty() )
			sphWarning ( "%s", sWarning.cstr() );
	} else if ( eType==INDEX_PQ )
	{
		Index_t tIndex;
		tIndex.m_sName = sName;
		tIndex.m_sPath = sPath;
		tIndex.m_sPathOut = sPathOut;

		if ( !LoadPqIndex ( tIndex, sError ) )
		{
			sError.SetSprintf ( "failed to load index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}

		CSphString sWarning;
		if ( !SavePqIndex ( tIndex, sWarning, sError ) )
		{
			sError.SetSprintf ( "conversion failed for index '%s', error: %s", sName.cstr(), sError.cstr() );
			return false;
		}

		if ( !sWarning.IsEmpty() )
			sphWarning ( "%s", sWarning.cstr() );

	} else
	{
		CSphVector<SphDocID_t> dKilled;
		if ( !ConvertPlain ( sName, sPath, bStripPath, sError, dKilled, sPathOut, dKlistTargets, nullptr ) )
			return false;
	}

	return true;
}


}

static void ShowVersion ()
{
	fprintf ( stdout, "%s", szMANTICORE_BANNER );
}

static void ShowHelp ()
{
	printf (
		"index_converter, a tool to convert index files from 2.X to 3.0 format\n"
		"\n"
		"Usage:\n"
		"index_converter --config manticore.conf --index test\n"
		"index_converter --path path_to_index_files --killlist-target main_idx:id\n"
		"index_converter --config manticore.conf --all --output-dir converted\n"
		"\n"
		"Options are:\n"
		"-c, --config <file>\t\tread configuration from specified file\n"
		"--index <name>\t\t\tconvert index defined in config file\n"
		"--path <path_to_index_files>\tconvert index from path provided\n"
		"--strip-path\t\t\tstrip path from filenames referenced by index: stopwords, exceptions and wordforms\n"
		"--large-docid\t\t\tallows to convert documents with ids larger than 2^63 and display a warning, otherwise it will just exit on the large id with an error\n"
		"--output-dir <dir>\t\toutput directory for converted files\n"
		"--all\t\t\t\tconvert all indexes in config file\n"
		"--killlist-target <targets>\tsets the indexes that the kill-list will be applied to\n"
	);
}

int main ( int argc, char ** argv )
{
	if ( argc<2 )
	{
		ShowVersion();
		ShowHelp();
		exit ( 0 );
	}

	CSphString sError;
	CSphString sConfig;
	CSphString sIndexName;
	CSphString sIndexIn;
	CSphString sIndexOut;
	CSphString sIndexFile;
	CSphString sKlistTarget;
	bool bKlistTargetCLI = false;
	bool bStripPath = false;
	bool bAll = false;

	for ( int i=1; i<argc; i++ )
	{
		if ( strcmp ( argv[i], "-c" )==0 || strcmp ( argv[i], "--config" )==0 )
		{
			if ( ++i>=argc )
				sphDie ( "config requires an argument" );

			sConfig = argv[i];

		} else if ( strcmp ( argv[i], "-i" )==0 || strcmp ( argv[i], "--index" )==0 )
		{
			if ( ++i>=argc )
				sphDie ( "index name requires an argument" );

			sIndexName = argv[i];

		} else if ( strcmp ( argv[i], "--path")==0 )
		{
			if ( ++i>=argc )
				sphDie ( "path to index requires an argument" );

			sIndexIn = argv[i];

		} else if ( strcmp ( argv[i], "--strip-path")==0 )
		{
			bStripPath = true;

		} else if ( strcmp ( argv[i], "--large-docid")==0 )
		{
			legacy::g_bLargeDocid = true;

		} else if ( strcmp ( argv[i], "-v" )==0 )
		{
			ShowVersion();
			exit(0);

		} else if (  strcmp ( argv[i], "-h" )==0 || strcmp ( argv[i], "--help" )==0 )
		{
			ShowVersion();
			ShowHelp();
			exit(0);

		} else if ( strcmp ( argv[i], "--output-dir")==0 )
		{
			if ( ++i>=argc )
				sphDie ( "output directory requires an argument" );

			legacy::g_sOutDir = argv[i];

		} else if ( strcmp ( argv[i], "--all")==0 )
		{
			bAll = true;

		} else if ( strcmp ( argv[i], "--killlist-target" )==0 )
		{
			if ( ++i>=argc )
				sphDie ( "killlist target requires an argument" );

			bKlistTargetCLI = true;
			sKlistTarget = argv[i];

		} else
		{
			sphDie ( "unknown switch: %s", argv[i] );
		}
	}

	if ( sIndexIn.IsEmpty() && ( sConfig.IsEmpty() || sIndexName.IsEmpty() ) && !bAll )
		sphDie ( "nothing to do" );

	if ( bAll && !sKlistTarget.IsEmpty() )
		sphDie ( "killlist-target not compatible with --all option" );

	KillListTargets_c tKlistTargets;
	if ( !sKlistTarget.IsEmpty() && !tKlistTargets.Parse ( sKlistTarget, sIndexFile.cstr(), sError ) )
		sphDie ( "failed to parse killlist-target, '%s'", sError.cstr() );

	if ( !sphInitCharsetAliasTable ( sError ) )
		sphDie ( "failed to init charset alias table: %s", sError.cstr() );

	const CSphConfigType * pIndexes = nullptr;
	if ( !sConfig.IsEmpty() )
	{
		CSphConfig hConfig = sphLoadConfig ( sConfig.cstr(), false, false );
		pIndexes = hConfig ( "index" );

		if ( ( bAll || !sIndexName.IsEmpty() ) && !pIndexes )
			sphDie ( "no indexes found in config" );

		sphConfigureCommon ( hConfig );
	}

	int iConvertedCount = 0;
	int iIndexTotal = 0;
	StrVec_t dNameParts;
	auto pItt = pIndexes->begin();
	while ( true )
	{
		if ( !bAll && iIndexTotal )
			break;

		if ( bAll )
		{
			++pItt;
			if ( pItt==pIndexes->end() )
				break;

			sIndexName = pItt->first;
			tKlistTargets.m_dTargets.Resize(0);
			sKlistTarget = "";
		}

		sError = "";
		dNameParts.Resize ( 0 );
		iIndexTotal++;
		legacy::IndexType_e eIndex = legacy::INDEX_UNKNOWN;

		if ( bAll || !sIndexName.IsEmpty() )
		{
			if ( !bAll && !pIndexes->Exists ( sIndexName ) )
			{
				sphWarning ( "no such index '%s', skipped", sIndexName.cstr() );
				continue;
			}

			const CSphConfigSection& tIndex = ( bAll ? pItt->second : ( *pIndexes )[sIndexName] );
			if ( tIndex.Exists ( "type" ) )
			{
				const CSphString sType = tIndex.GetStr ( "type", NULL );
				if ( sType.IsEmpty() )
				{
					sphWarning ( "unknown index '%s' type '%s', skipped", sIndexName.cstr(), sType.cstr() );
					continue;
				}
				if ( sType!="rt" && sType!="plain" && sType!="percolate" )
				{
					sphWarning ( "index '%s' type '%s', only 'plain' or 'rt' or 'percolate' types supported, skipped", sIndexName.cstr(), sType.cstr() );
					continue;
				}

				eIndex = ( sType=="rt" ? legacy::INDEX_RT : ( sType=="percolate" ? legacy::INDEX_PQ : legacy::INDEX_PLAIN ) );
			}

			if ( !tIndex.Exists ( "path" ) )
			{
				sphWarning ( "no index path '%s', skipped", sIndexName.cstr() );
				continue;
			}
			sIndexIn = tIndex["path"].cstr();

			sphSplit ( dNameParts, sIndexIn.cstr(), "/\\" );
			sIndexFile = dNameParts.Last();

			if ( tIndex.Exists( "killlist_target" ) )
			{
				if ( bKlistTargetCLI )
					sphWarning ( "--killlist-target specified in command line overrides killlist_target from config '%s'", sConfig.cstr() );
				else
				{
					sKlistTarget = tIndex["killlist_target"].cstr();
					if ( !tKlistTargets.Parse ( sKlistTarget, sIndexName.cstr(), sError ) )
					{
						sphWarning ( "failed to parse killlist_target, '%s'", sError.cstr() );
						tKlistTargets.m_dTargets.Resize(0);
						sKlistTarget = "";
						sError = "";
					}
				}
			}

		} else
		{
			sphSplit ( dNameParts, sIndexIn.cstr(), "/\\" );
			sIndexName = dNameParts.Last();
			sIndexFile = dNameParts.Last();
		}

		if ( !legacy::g_sOutDir.IsEmpty() )
		{
			CSphWriter tDir;
			CSphString sTmpName;
			sTmpName.SetSprintf ( "%s/%s.tmp", legacy::g_sOutDir.cstr(), sIndexFile.cstr() );
			if ( !tDir.OpenFile ( sTmpName, sError ) )
			{
				sphWarning ( "output-dir failed '%s', skipped", sError.cstr() );
				continue;
			}

			tDir.UnlinkFile();

			sIndexOut.SetSprintf ( "%s/%s", legacy::g_sOutDir.cstr(), sIndexFile.cstr() );
		} else
		{
			sIndexOut = sIndexIn;
		}

		bool bOk = legacy::Convert ( sIndexName, sIndexIn, eIndex, bStripPath, sIndexOut, tKlistTargets, sError );
		if ( !bOk )
		{
			sphWarning ( "%s", sError.cstr() );
		} else
		{
			printf ( "converted index '%s'%s%s\n", sIndexName.cstr(),
				( sKlistTarget.IsEmpty() ? "" : " with killlist_target=" ), ( sKlistTarget.IsEmpty() ? "" : sKlistTarget.cstr() ) );
			iConvertedCount++;
		}
	}

	printf ( "converted indexes %d(%d)\n", iConvertedCount, iIndexTotal );

	return 0;
}

