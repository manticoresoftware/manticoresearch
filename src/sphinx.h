//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinx_
#define _sphinx_

/////////////////////////////////////////////////////////////////////////////

#include "sphinxstd.h"
#include "sphinxexpr.h" // to remove?

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if USE_PGSQL
#include <libpq-fe.h>
#endif

#if USE_WINDOWS
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#if USE_MYSQL
#include <mysql.h>
#endif

#if USE_WINDOWS
#define STDOUT_FILENO		fileno(stdout)
#define STDERR_FILENO		fileno(stderr)
#endif

#if USE_ODBC
#include <sqlext.h>
#endif

/////////////////////////////////////////////////////////////////////////////

using RowID_t = DWORD;
using DocID_t = int64_t;

const RowID_t INVALID_ROWID = 0xFFFFFFFF;
#define WORDID_MAX		U64C(0xffffffffffffffff)

typedef uint64_t		SphWordID_t;

STATIC_SIZE_ASSERT ( SphWordID_t, 8 );
STATIC_SIZE_ASSERT ( DocID_t, 8 );
STATIC_SIZE_ASSERT ( RowID_t, 4 );

#define DWSIZEOF(a) ( sizeof(a) / sizeof(DWORD) )

//////////////////////////////////////////////////////////////////////////

/// row entry (storage only, does not necessarily map 1:1 to attributes)
typedef DWORD			CSphRowitem;
typedef const BYTE *	CSphRowitemPtr;

/// widest integer type that can be be stored as an attribute (ideally, fully decoupled from rowitem size!)
typedef int64_t			SphAttr_t;

const CSphRowitem		ROWITEM_MAX		= UINT_MAX;
const int				ROWITEM_BITS	= 8*sizeof(CSphRowitem);
const int				ROWITEMPTR_BITS	= 8*sizeof(CSphRowitemPtr);
const int				ROWITEM_SHIFT	= 5;

STATIC_ASSERT ( ( 1 << ROWITEM_SHIFT )==ROWITEM_BITS, INVALID_ROWITEM_SHIFT );

#ifndef USE_LITTLE_ENDIAN
#error Please define endianness
#endif

/////////////////////////////////////////////////////////////////////////////

// defined in sphinxversion.cpp in order to isolate from total rebuild on minor changes
extern const char * szMANTICORE_VERSION;
extern const char * szMANTICORE_NAME;
extern const char * szMANTICORE_BANNER;
extern const char * szGIT_COMMIT_ID;
extern const char * szGIT_BRANCH_ID;
extern const char * szGDB_SOURCE_DIR;

#define SPHINX_SEARCHD_PROTO	1
#define SPHINX_CLIENT_VERSION	1

#define SPH_MAX_WORD_LEN		42		// so that any UTF-8 word fits 127 bytes
#define SPH_MAX_FILENAME_LEN	512
#define SPH_MAX_FIELDS			256

/////////////////////////////////////////////////////////////////////////////

extern int64_t g_iIndexerCurrentDocID;
extern int64_t g_iIndexerCurrentHits;
extern int64_t g_iIndexerCurrentRangeMin;
extern int64_t g_iIndexerCurrentRangeMax;
extern int64_t g_iIndexerPoolStartDocID;
extern int64_t g_iIndexerPoolStartHit;

/////////////////////////////////////////////////////////////////////////////

/// Sphinx CRC32 implementation
extern DWORD	g_dSphinxCRC32 [ 256 ];
DWORD			sphCRC32 ( const void * pString );
DWORD			sphCRC32 ( const void * pString, int iLen );
DWORD			sphCRC32 ( const void * pString, int iLen, DWORD uPrevCRC );

/// Fast check if our endianess is correct
const char*		sphCheckEndian();

/// Sphinx FNV64 implementation
const uint64_t	SPH_FNV64_SEED = 0xcbf29ce484222325ULL;
uint64_t		sphFNV64 ( const void * pString );
uint64_t		sphFNV64 ( const void * s, int iLen, uint64_t uPrev = SPH_FNV64_SEED );
uint64_t		sphFNV64cont ( const void * pString, uint64_t uPrev );

/// calculate file crc32
bool			sphCalcFileCRC32 ( const char * szFilename, DWORD & uCRC32 );

/// try to obtain an exclusive lock on specified file
/// bWait specifies whether to wait
bool			sphLockEx ( int iFile, bool bWait );

/// remove existing locks
void			sphLockUn ( int iFile );

/// millisecond-precision sleep
void			sphSleepMsec ( int iMsec );

/// check if file exists and is a readable file
bool			sphIsReadable ( const char * sFilename, CSphString * pError=NULL );
bool			sphIsReadable ( const CSphString& sFilename, CSphString * pError = NULL );


/// set throttling options
void			sphSetThrottling ( int iMaxIOps, int iMaxIOSize );

/// write blob to file honoring throttling
bool			sphWriteThrottled ( int iFD, const void* pBuf, int64_t iCount, const char* sName, CSphString& sError );

/// read blob from file honoring throttling
size_t			sphReadThrottled ( int iFD, void* pBuf, size_t iCount );

/// immediately interrupt current query
void			sphInterruptNow();

/// check if we got interrupted
bool			sphInterrupted();

/// initialize IO statistics collecting
void			sphInitIOStats ();

/// clean up IO statistics collector
void			sphDoneIOStats ();


class CSphIOStats
{
public:
	int64_t		m_iReadTime = 0;
	DWORD		m_iReadOps = 0;
	int64_t		m_iReadBytes = 0;
	int64_t		m_iWriteTime = 0;
	DWORD		m_iWriteOps = 0;
	int64_t		m_iWriteBytes = 0;

	~CSphIOStats ();

	void		Start();
	void		Stop();

	void		Add ( const CSphIOStats & b );
	bool		IsEnabled() { return m_bEnabled; }

private:
	bool		m_bEnabled = false;
	CSphIOStats * m_pPrev = nullptr;
};


//////////////////////////////////////////////////////////////////////////

#if UNALIGNED_RAM_ACCESS

/// pass-through wrapper
template < typename T > inline T sphUnalignedRead ( const T & tRef )
{
	return tRef;
}

/// pass-through wrapper
template < typename T > void sphUnalignedWrite ( void * pPtr, const T & tVal )
{
	*(T*)pPtr = tVal;
}

#else

/// unaligned read wrapper for some architectures (eg. SPARC)
template < typename T >
inline T sphUnalignedRead ( const T & tRef )
{
	T uTmp;
	BYTE * pSrc = (BYTE *) &tRef;
	BYTE * pDst = (BYTE *) &uTmp;
	for ( int i=0; i<(int)sizeof(T); i++ )
		*pDst++ = *pSrc++;
	return uTmp;
}

/// unaligned write wrapper for some architectures (eg. SPARC)
template < typename T >
void sphUnalignedWrite ( void * pPtr, const T & tVal )
{
	BYTE * pDst = (BYTE *) pPtr;
	BYTE * pSrc = (BYTE *) &tVal;
	for ( int i=0; i<(int)sizeof(T); i++ )
		*pDst++ = *pSrc++;
}

#endif // unalgined


#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
/// get a dword from memory, intel version
inline DWORD sphGetDword ( const BYTE * p )
{
	return *(const DWORD*)p;
}
#else
/// get a dword from memory, non-intel version
inline DWORD sphGetDword ( const BYTE * p )
{
	return p[0] + ( p[1]<<8 ) + ( p[2]<<16 ) + ( p[3]<<24 );
}
#endif


int sphUTF8Len ( const char * pStr );

/// check for valid attribute name char
inline int sphIsAttr ( int c )
{
	// different from sphIsAlpha() in that we don't allow minus
	return ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_';
}

/////////////////////////////////////////////////////////////////////////////
// TOKENIZERS
/////////////////////////////////////////////////////////////////////////////

extern const char *		SPHINX_DEFAULT_UTF8_TABLE;

/////////////////////////////////////////////////////////////////////////////

/// lowercaser remap range
struct CSphRemapRange
{
	int			m_iStart;
	int			m_iEnd;
	int			m_iRemapStart;

	CSphRemapRange ()
		: m_iStart		( -1 )
		, m_iEnd		( -1 )
		, m_iRemapStart	( -1 )
	{}

	CSphRemapRange ( int iStart, int iEnd, int iRemapStart )
		: m_iStart		( iStart )
		, m_iEnd		( iEnd )
		, m_iRemapStart	( iRemapStart )
	{}
};


inline bool operator < ( const CSphRemapRange & a, const CSphRemapRange & b )
{
	return a.m_iStart < b.m_iStart;
}


/// lowercaser
class CSphLowercaser
{
	friend class ISphTokenizer;
	friend class CSphTokenizerBase;
	friend class CSphTokenizerBase2;

public:
				~CSphLowercaser ();

	void		Reset ();
	void		SetRemap ( const CSphLowercaser * pLC );
	void		AddRemaps ( const CSphVector<CSphRemapRange> & dRemaps, DWORD uFlags );
	void		AddSpecials ( const char * sSpecials );
	uint64_t	GetFNV () const;

public:
	CSphLowercaser &		operator = ( const CSphLowercaser & rhs );

public:
	inline int	ToLower ( int iCode ) const
	{
		if ( iCode<0 || iCode>=MAX_CODE )
			return iCode;
		register int * pChunk = m_pChunk [ iCode >> CHUNK_BITS ];
		if ( pChunk )
			return pChunk [ iCode & CHUNK_MASK ];
		return 0;
	}

	int GetMaxCodepointLength () const;

protected:
	static const int	CHUNK_COUNT	= 0x300;
	static const int	CHUNK_BITS	= 8;

	static const int	CHUNK_SIZE	= 1 << CHUNK_BITS;
	static const int	CHUNK_MASK	= CHUNK_SIZE - 1;
	static const int	MAX_CODE	= CHUNK_COUNT * CHUNK_SIZE;

	int					m_iChunks = 0;					///< how much chunks are actually allocated
	int *				m_pData = nullptr;					///< chunks themselves
	int *				m_pChunk [ CHUNK_COUNT ] { nullptr };	///< pointers to non-empty chunks
};

/////////////////////////////////////////////////////////////////////////////
enum
{
	// where was TOKENIZER_SBCS=1 once
	TOKENIZER_UTF8 = 2,
	TOKENIZER_NGRAM = 3
};

struct CSphSavedFile
{
	CSphString			m_sFilename;
	SphOffset_t			m_uSize = 0;
	SphOffset_t			m_uCTime = 0;
	SphOffset_t			m_uMTime = 0;
	DWORD				m_uCRC32 = 0;
};


struct CSphEmbeddedFiles
{
	bool						m_bEmbeddedSynonyms = false;
	bool						m_bEmbeddedStopwords = false;
	bool						m_bEmbeddedWordforms = false;
	CSphSavedFile				m_tSynonymFile;
	StrVec_t					m_dSynonyms;
	CSphVector<CSphSavedFile>	m_dStopwordFiles;
	CSphVector<SphWordID_t>		m_dStopwords;
	StrVec_t					m_dWordforms;
	CSphVector<CSphSavedFile>	m_dWordformFiles;
	void						Reset();
};


struct CSphTokenizerSettings
{
	int					m_iType { TOKENIZER_UTF8 };
	CSphString			m_sCaseFolding;
	int					m_iMinWordLen = 1;
	CSphString			m_sSynonymsFile;
	CSphString			m_sBoundary;
	CSphString			m_sIgnoreChars;
	int					m_iNgramLen = 0;
	CSphString			m_sNgramChars;
	CSphString			m_sBlendChars;
	CSphString			m_sBlendMode;
};


enum ESphBigram
{
	SPH_BIGRAM_NONE			= 0,	///< no bigrams
	SPH_BIGRAM_ALL			= 1,	///< index all word pairs
	SPH_BIGRAM_FIRSTFREQ	= 2,	///< only index pairs where one of the words is in a frequent words list
	SPH_BIGRAM_BOTHFREQ		= 3		///< only index pairs where both words are in a frequent words list
};


enum ESphTokenizerClone
{
	SPH_CLONE_INDEX,				///< clone tokenizer and set indexing mode
	SPH_CLONE_QUERY,				///< clone tokenizer and set querying mode
	SPH_CLONE_QUERY_LIGHTWEIGHT		///< lightweight clone for querying (can parse, can NOT modify settings, shares pointers to the original lowercaser table)
};


enum ESphTokenMorph
{
	SPH_TOKEN_MORPH_RAW,			///< no morphology applied, tokenizer does not handle morphology
	SPH_TOKEN_MORPH_ORIGINAL,		///< no morphology applied, but tokenizer handles morphology
	SPH_TOKEN_MORPH_GUESS			///< morphology applied
};


struct CSphMultiformContainer;
class CSphWriter;

/// generic tokenizer
class ISphTokenizer : public ISphRefcountedMT
{
	/// trivial dtor - inherited from Refcounted
protected:
	~ISphTokenizer() override {};
public:
	/// set new translation table
	/// returns true on success, false on failure
	virtual bool					SetCaseFolding ( const char * sConfig, CSphString & sError );

	/// add additional character as valid (with folding to itself)
	virtual void					AddPlainChar ( char c );

	/// add special chars to translation table
	/// updates lowercaser so that these remap to -1
	virtual void					AddSpecials ( const char * sSpecials );

	/// set ignored characters
	virtual bool					SetIgnoreChars ( const char * sIgnored, CSphString & sError );

	/// set n-gram characters (for CJK n-gram indexing)
	virtual bool					SetNgramChars ( const char *, CSphString & ) { return true; }

	/// set n-gram length (for CJK n-gram indexing)
	virtual void					SetNgramLen ( int ) {}

	/// load synonyms list
	virtual bool					LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, CSphString & sError ) = 0;

	/// write synonyms to file
	virtual void					WriteSynonyms ( CSphWriter & tWriter ) const = 0;

	/// set phrase boundary chars
	virtual bool					SetBoundary ( const char * sConfig, CSphString & sError );

	/// set blended characters
	virtual bool					SetBlendChars ( const char * sConfig, CSphString & sError );

	/// set blended tokens processing mode
	virtual bool					SetBlendMode ( const char * sMode, CSphString & sError );

	/// setup tokenizer using given settings
	virtual void					Setup ( const CSphTokenizerSettings & tSettings );

	/// create a tokenizer using the given settings
	static ISphTokenizer *			Create ( const CSphTokenizerSettings & tSettings, const CSphEmbeddedFiles * pFiles, CSphString & sError );

	/// create a token filter
	static ISphTokenizer *			CreateMultiformFilter ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer );

	/// create a token filter
	static ISphTokenizer *			CreateBigramFilter ( ISphTokenizer * pTokenizer, ESphBigram eBigramIndex, const CSphString & sBigramWords, CSphString & sError );

	/// create a plugin filter
	/// sSspec is a library, name, and options specification string, eg "myplugins.dll:myfilter1:arg1=123"
	static ISphTokenizer *			CreatePluginFilter ( ISphTokenizer * pTokenizer, const CSphString & sSpec, CSphString & sError );

	/// save tokenizer settings to a stream
	virtual const CSphTokenizerSettings &	GetSettings () const { return m_tSettings; }

	/// get synonym file info
	virtual const CSphSavedFile &	GetSynFileInfo () const { return m_tSynFileInfo; }

public:
	/// pass next buffer
	virtual void					SetBuffer ( const BYTE * sBuffer, int iLength ) = 0;

	/// set current index schema (only intended for the token filter plugins)
	virtual bool					SetFilterSchema ( const CSphSchema &, CSphString & ) { return true; }

	/// set per-document options from INSERT
	virtual bool					SetFilterOptions ( const char *, CSphString & ) { return true; }

	/// notify tokenizer that we now begin indexing a field with a given number (only intended for the token filter plugins)
	virtual void					BeginField ( int ) {}

	/// get next token
	virtual BYTE *					GetToken () = 0;

	/// calc codepoint length
	virtual int						GetCodepointLength ( int iCode ) const = 0;

	/// get max codepoint length
	virtual int						GetMaxCodepointLength () const = 0;

	/// enable indexing-time sentence boundary detection, and paragraph indexing
	virtual bool					EnableSentenceIndexing ( CSphString & sError );

	/// enable zone indexing
	virtual bool					EnableZoneIndexing ( CSphString & sError );

	/// enable tokenized multiform tracking
	virtual void					EnableTokenizedMultiformTracking () {}

	/// get last token length, in codepoints
	virtual int						GetLastTokenLen () const { return m_iLastTokenLen; }

	/// get last token boundary flag (true if there was a boundary before the token)
	virtual bool					GetBoundary () { return m_bTokenBoundary; }

	/// get byte offset of the last boundary character
	virtual int						GetBoundaryOffset () { return m_iBoundaryOffset; }

	/// was last token a special one?
	virtual bool					WasTokenSpecial () { return m_bWasSpecial; }

	virtual bool					WasTokenSynonym () const { return m_bWasSynonym; }

	/// get amount of overshort keywords skipped before this token
	virtual int						GetOvershortCount () { return ( !m_bBlended && m_bBlendedPart ? 0 : m_iOvershortCount ); }

	/// get original tokenized multiform (if any); NULL means there was none
	virtual BYTE *					GetTokenizedMultiform () { return NULL; }

	/// was last token a part of multi-wordforms destination
	/// head parameter might be useful to distinguish between sequence of different multi-wordforms
	virtual bool					WasTokenMultiformDestination ( bool & bHead, int & iDestCount ) const = 0;

	/// check whether this token is a generated morphological guess
	ESphTokenMorph					GetTokenMorph() const { return m_eTokenMorph; }

	virtual bool					TokenIsBlended () const { return m_bBlended; }
	virtual bool					TokenIsBlendedPart () const { return m_bBlendedPart; }
	virtual int						SkipBlended () { return 0; }

public:
	/// spawn a clone of my own
	virtual ISphTokenizer *			Clone ( ESphTokenizerClone eMode ) const = 0;

	/// start buffer point of last token
	virtual const char *			GetTokenStart () const = 0;

	/// end buffer point of last token (exclusive, ie. *GetTokenEnd() is already NOT part of a token!)
	virtual const char *			GetTokenEnd () const = 0;

	/// current buffer ptr
	virtual const char *			GetBufferPtr () const = 0;

	/// buffer end
	virtual const char *			GetBufferEnd () const = 0;

	/// set new buffer ptr (must be within current bounds)
	virtual void					SetBufferPtr ( const char * sNewPtr ) = 0;

	/// get settings hash
	virtual uint64_t				GetSettingsFNV () const;

	/// get (readonly) lowercaser
	const CSphLowercaser &			GetLowercaser() const { return m_tLC; }

protected:
	virtual bool					RemapCharacters ( const char * sConfig, DWORD uFlags, const char * sSource, bool bCanRemap, CSphString & sError );
	virtual bool					AddSpecialsSPZ ( const char * sSpecials, const char * sDirective, CSphString & sError );

protected:

	static const BYTE				BLEND_TRIM_NONE		= 1;
	static const BYTE				BLEND_TRIM_HEAD		= 2;
	static const BYTE				BLEND_TRIM_TAIL		= 4;
	static const BYTE				BLEND_TRIM_BOTH		= 8;
	static const BYTE				BLEND_TRIM_ALL		= 16;

	CSphLowercaser					m_tLC;						///< my lowercaser
	int								m_iLastTokenLen = 0;		///< last token length, in codepoints
	bool							m_bTokenBoundary = false;	///< last token boundary flag (true after boundary codepoint followed by separator)
	bool							m_bBoundary = false;		///< boundary flag (true immediately after boundary codepoint)
	int								m_iBoundaryOffset = 0;		///< boundary character offset (in bytes)
	bool							m_bWasSpecial = false;		///< special token flag
	bool							m_bWasSynonym = false;		///< last token is a synonym token
	int								m_iOvershortCount = 0;		///< skipped overshort tokens count
	ESphTokenMorph					m_eTokenMorph {SPH_TOKEN_MORPH_RAW}; ///< whether last token was a generated morphological guess

	bool							m_bBlended = false;			///< whether last token (as in just returned from GetToken()) was blended
	bool							m_bNonBlended = true;		///< internal, whether there were any normal chars in that blended token
	bool							m_bBlendedPart = false;		///< whether last token is a normal subtoken of a blended token
	bool							m_bBlendAdd = false;		///< whether we have more pending blended variants (of current accumulator) to return
	BYTE							m_uBlendVariants {BLEND_TRIM_NONE};	///< mask of blended variants as requested by blend_mode (see BLEND_TRIM_xxx flags)
	BYTE							m_uBlendVariantsPending = 0;///< mask of pending blended variants (we clear bits as we return variants)
	bool							m_bBlendSkipPure = false;	///< skip purely blended tokens

	bool							m_bShortTokenFilter = false;///< short token filter flag
	bool							m_bDetectSentences = false;	///< should we detect sentence boundaries?

	CSphTokenizerSettings			m_tSettings;				///< tokenizer settings
	CSphSavedFile					m_tSynFileInfo;				///< synonyms file info

public:
	bool							m_bPhrase = false;
};


struct CharsetAlias_t
{
	CSphString					m_sName;
	int							m_iNameLen;
	CSphVector<CSphRemapRange>	m_dRemaps;
};


using TokenizerRefPtr_c = CSphRefcountedPtr<ISphTokenizer>;

/// parse charset table
bool					sphParseCharset ( const char * sCharset, CSphVector<CSphRemapRange> & dRemaps );

/// create UTF-8 tokenizer
ISphTokenizer *			sphCreateUTF8Tokenizer ();

/// create UTF-8 tokenizer with n-grams support (for CJK n-gram indexing)
ISphTokenizer *			sphCreateUTF8NgramTokenizer ();

/////////////////////////////////////////////////////////////////////////////
// DICTIONARIES
/////////////////////////////////////////////////////////////////////////////

struct CSphDictSettings
{
	CSphString		m_sMorphology;
	CSphString		m_sMorphFields;
	CSphString		m_sStopwords;
	StrVec_t		m_dWordforms;
	int				m_iMinStemmingLen;
	bool			m_bWordDict;
	bool			m_bStopwordsUnstemmed;
	CSphString		m_sMorphFingerprint;		///< not used for creation; only for a check when loading

	CSphDictSettings ()
		: m_iMinStemmingLen ( 1 )
		, m_bWordDict ( true )
		, m_bStopwordsUnstemmed ( false )
	{}
};


/// dictionary entry
/// some of the fields might be unused depending on specific dictionary type
struct CSphDictEntry
{
	SphWordID_t		m_uWordID = 0;			///< keyword id (for dict=crc)
	const BYTE *	m_sKeyword = nullptr;	///< keyword text (for dict=keywords)
	int				m_iDocs = 0;			///< number of matching documents
	int				m_iHits = 0;			///< number of occurrences
	SphOffset_t		m_iDoclistOffset = 0;	///< absolute document list offset (into .spd)
	SphOffset_t		m_iDoclistLength = 0;	///< document list length in bytes
	SphOffset_t		m_iSkiplistOffset = 0;	///< absolute skiplist offset (into .spe)
	int				m_iDoclistHint = 0;		///< raw document list length hint value (0..255 range, 1 byte)
};


/// stored normal form
struct CSphStoredNF
{
	CSphString					m_sWord;
	bool						m_bAfterMorphology;
};


/// wordforms container
struct CSphWordforms
{
	int							m_iRefCount;
	CSphVector<CSphSavedFile>	m_dFiles;
	uint64_t					m_uTokenizerFNV;
	CSphString					m_sIndexName;
	bool						m_bHavePostMorphNF;
	CSphVector <CSphStoredNF>	m_dNormalForms;
	CSphMultiformContainer *	m_pMultiWordforms;
	CSphOrderedHash < int, CSphString, CSphStrHashFunc, 1048576 >	m_dHash;

	CSphWordforms ();
	~CSphWordforms ();

	bool						IsEqual ( const CSphVector<CSphSavedFile> & dFiles );
	bool						ToNormalForm ( BYTE * pWord, bool bBefore, bool bOnlyCheck ) const;
};


/// abstract word dictionary interface
struct CSphWordHit;
class CSphAutofile;
struct DictHeader_t;
class CSphDict : public ISphRefcountedMT
{
protected:
	/// virtualizing dtor. Protected to follow refcounted rules.
	virtual                ~CSphDict () {}

public:
	static const int	ST_OK = 0;
	static const int	ST_ERROR = 1;
	static const int	ST_WARNING = 2;

public:
	/// Get word ID by word, "text" version
	/// may apply stemming and modify word inplace
	/// modified word may become bigger than the original one, so make sure you have enough space in buffer which is pointer by pWord
	/// a general practice is to use char[3*SPH_MAX_WORD_LEN+4] as a buffer
	/// returns 0 for stopwords
	virtual SphWordID_t	GetWordID ( BYTE * pWord ) = 0;

	/// get word ID by word, "text" version
	/// may apply stemming and modify word inplace
	/// accepts words with already prepended MAGIC_WORD_HEAD
	/// appends MAGIC_WORD_TAIL
	/// returns 0 for stopwords
	virtual SphWordID_t	GetWordIDWithMarkers ( BYTE * pWord ) { return GetWordID ( pWord ); }

	/// get word ID by word, "text" version
	/// does NOT apply stemming
	/// accepts words with already prepended MAGIC_WORD_HEAD_NONSTEMMED
	/// returns 0 for stopwords
	virtual SphWordID_t	GetWordIDNonStemmed ( BYTE * pWord ) { return GetWordID ( pWord ); }

	/// get word ID by word, "binary" version
	/// only used with prefix/infix indexing
	/// must not apply stemming and modify anything
	/// filters stopwords on request
	virtual SphWordID_t	GetWordID ( const BYTE * pWord, int iLen, bool bFilterStops ) = 0;

	/// apply stemmers to the given word
	virtual void		ApplyStemmers ( BYTE * ) const {}

	/// load stopwords from given files
	virtual void		LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer, bool bStripFile ) = 0;

	/// load stopwords from an array
	virtual void		LoadStopwords ( const CSphVector<SphWordID_t> & dStopwords ) = 0;

	/// write stopwords to a file
	virtual void		WriteStopwords ( CSphWriter & tWriter ) const = 0;

	/// load wordforms from a given list of files
	virtual bool		LoadWordforms ( const StrVec_t &, const CSphEmbeddedFiles * pEmbedded, const ISphTokenizer * pTokenizer, const char * sIndex ) = 0;

	/// write wordforms to a file
	virtual void		WriteWordforms ( CSphWriter & tWriter ) const = 0;

	/// get wordforms
	virtual const CSphWordforms *	GetWordforms() { return NULL; }

	/// disable wordforms processing
	virtual void		DisableWordforms() {}

	/// set morphology
	/// returns 0 on success, 1 on hard error, 2 on a warning (see ST_xxx constants)
	virtual int			SetMorphology ( const char * szMorph, CSphString & sMessage ) = 0;

	/// are there any morphological processors?
	virtual bool		HasMorphology () const { return false; }

	/// morphological data fingerprint (lemmatizer filenames and crc32s)
	virtual const CSphString &	GetMorphDataFingerprint () const { return m_sMorphFingerprint; }

	/// setup dictionary using settings
	virtual void		Setup ( const CSphDictSettings & tSettings ) = 0;

	/// get dictionary settings
	virtual const CSphDictSettings & GetSettings () const = 0;

	/// stopwords file infos
	virtual const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () const = 0;

	/// wordforms file infos
	virtual const CSphVector <CSphSavedFile> & GetWordformsFileInfos () const = 0;

	/// get multiwordforms
	virtual const CSphMultiformContainer * GetMultiWordforms () const = 0;

	/// check what given word is stopword
	virtual bool IsStopWord ( const BYTE * pWord ) const = 0;

public:
	virtual void			SetSkiplistBlockSize ( int iSize ) {}

	/// enable actually collecting keywords (needed for stopwords/wordforms loading)
	virtual void			HitblockBegin () {}

	/// callback to let dictionary do hit block post-processing
	virtual void			HitblockPatch ( CSphWordHit *, int ) const {}

	/// resolve temporary hit block wide wordid (!) back to keyword
	virtual const char *	HitblockGetKeyword ( SphWordID_t ) { return NULL; }

	/// check current memory usage
	virtual int				HitblockGetMemUse () { return 0; }

	/// hit block dismissed
	virtual void			HitblockReset () {}

public:
	/// begin creating dictionary file, setup any needed internal structures
	virtual void			DictBegin ( CSphAutofile & tTempDict, CSphAutofile & tDict, int iDictLimit );

	/// add next keyword entry to final dict
	virtual void			DictEntry ( const CSphDictEntry & tEntry );

	/// flush last entry
	virtual void			DictEndEntries ( SphOffset_t iDoclistOffset );

	/// end indexing, store dictionary and checkpoints
	virtual bool			DictEnd ( DictHeader_t * pHeader, int iMemLimit, CSphString & sError );

	/// check whether there were any errors during indexing
	virtual bool			DictIsError () const;

public:
	/// check whether this dict is stateful (when it comes to lookups)
	virtual bool			HasState () const { return false; }

	/// make a clone
	virtual CSphDict *		Clone () const { return nullptr; }

	/// get settings hash
	virtual uint64_t		GetSettingsFNV () const = 0;

protected:
	CSphString				m_sMorphFingerprint;
};

using DictRefPtr_c = CSphRefcountedPtr<CSphDict>;

/// returns pDict, if stateless. Or it's clone, if not
CSphDict * GetStatelessDict ( CSphDict * pDict );

/// traits dictionary factory (no storage, only tokenizing, lemmatizing, etc.)
CSphDict * sphCreateDictionaryTemplate ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile, CSphString & sError );

/// CRC32/FNV64 dictionary factory
CSphDict * sphCreateDictionaryCRC ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile, int iSkiplistBlockSize, CSphString & sError );

/// keyword-storing dictionary factory
CSphDict * sphCreateDictionaryKeywords ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, bool bStripFile, int iSkiplistBlockSize, CSphString & sError );

/// clear wordform cache
void sphShutdownWordforms ();

/////////////////////////////////////////////////////////////////////////////
// DATASOURCES
/////////////////////////////////////////////////////////////////////////////

/// hit position storage type
typedef DWORD Hitpos_t;

/// empty hit value
#define EMPTY_HIT 0

/// hit processing tools
/// Hitpos_t consists of three things:
/// 1) high bits store field number
/// 2) middle bit - field end marker
/// 3) lower bits store hit position in field
template < int FIELD_BITS >
class Hitman_c
{
protected:
	enum
	{
		POS_BITS		= 31 - FIELD_BITS,
		FIELD_OFF		= 32 - FIELD_BITS,
		FIELDEND_OFF	= 31 - FIELD_BITS,
		FIELDEND_MASK	= (1UL << POS_BITS),
		POS_MASK		= (1UL << POS_BITS) - 1
	};

public:
	static Hitpos_t Create ( int iField, int iPos )
	{
		return ( iField << FIELD_OFF ) + ( iPos & POS_MASK );
	}

	static Hitpos_t Create ( int iField, int iPos, bool bEnd )
	{
		return ( iField << FIELD_OFF ) + ( ((int)bEnd) << FIELDEND_OFF ) + ( iPos & POS_MASK );
	}

	static inline int GetField ( Hitpos_t uHitpos )
	{
		return uHitpos >> FIELD_OFF;
	}

	static inline int GetPos ( Hitpos_t uHitpos )
	{
		return uHitpos & POS_MASK;
	}

	static inline bool IsEnd ( Hitpos_t uHitpos )
	{
		return ( uHitpos & FIELDEND_MASK )!=0;
	}

	static inline DWORD GetPosWithField ( Hitpos_t uHitpos )
	{
		return uHitpos & ~FIELDEND_MASK;
	}

	static void AddPos ( Hitpos_t * pHitpos, int iAdd )
	{
		// FIXME! add range checks (eg. so that 0:0-1 does not overflow)
		*pHitpos += iAdd;
	}

	static Hitpos_t CreateSum ( Hitpos_t uHitpos, int iAdd )
	{
		// FIXME! add range checks (eg. so that 0:0-1 does not overflow)
		return ( uHitpos+iAdd ) & ~FIELDEND_MASK;
	}

	static void SetEndMarker ( Hitpos_t * pHitpos )
	{
		*pHitpos |= FIELDEND_MASK;
	}
};

// this could be just DWORD[] but it's methods are very handy
// used to store field information e.g. which fields do we need to search in
struct FieldMask_t
{
	static const int SIZE = SPH_MAX_FIELDS/32;
	STATIC_ASSERT ( ( SPH_MAX_FIELDS%32 )==0, ASSUME_MAX_FIELDS_ARE_REPRESENTABLE_BY_DWORD );
	DWORD m_dMask [ SIZE ];

	// no custom cstr and d-tor - to be usable from inside unions
	// deep copy for it is ok - so, no explicit copying constructor and operator=

	// old-fashion layer to work with DWORD (32-bit) mask.
	// all bits above 32 assumed to be unset.
	void Assign32 ( DWORD uMask )
	{
		UnsetAll();
		m_dMask[0] = uMask;
	}

	DWORD GetMask32 () const
	{
		return m_dMask[0];
	}

	DWORD operator[] ( int iIdx ) const
	{
		assert ( 0<=iIdx && iIdx<SIZE );
		return m_dMask [ iIdx ];
	}

	DWORD & operator[] ( int iIdx )
	{
		assert ( 0<=iIdx && iIdx<SIZE );
		return m_dMask [ iIdx ];
	}

	// set n-th bit
	void Set ( int iIdx )
	{
		assert ( 0<=iIdx && iIdx<(int)sizeof(m_dMask)*8 );
		m_dMask [ iIdx/32 ] |= 1 << ( iIdx%32 );
	}

	// set all bits
	void SetAll()
	{
		memset ( m_dMask, 0xff, sizeof(m_dMask) );
	}

	// unset n-th bit, or all
	void Unset ( int iIdx )
	{
		assert ( 0<=iIdx && iIdx<(int)sizeof(m_dMask)*8 );
		m_dMask [ iIdx/32 ] &= ~(1 << ( iIdx%32 ));
	}

	void UnsetAll()
	{
		memset ( m_dMask, 0, sizeof(m_dMask) );
	}

	// test if n-th bit is set
	bool Test ( int iIdx ) const
	{
		assert ( iIdx>=0 && iIdx<(int)sizeof(m_dMask)*8 );
		return ( m_dMask [ iIdx/32 ] & ( 1 << ( iIdx%32 ) ) )!=0;
	}

	// test if all bits are set or unset
	bool TestAll ( bool bSet ) const
	{
		DWORD uTest = bSet ? 0xffffffff : 0;
		for ( auto uMask : m_dMask )
			if ( uMask!=uTest )
				return false;
		return true;
	}

	void Negate()
	{
		for ( auto& uMask : m_dMask )
			uMask = ~uMask;
	}
};

/// hit info
struct CSphWordHit
{
	RowID_t		m_tRowID;		///< document ID
	SphWordID_t	m_uWordID = WORDID_MAX;		///< word ID in current dictionary
	Hitpos_t	m_uWordPos = EMPTY_HIT;		///< word position in current document
};


/// attribute locator within the row
struct CSphAttrLocator
{
	int				m_iBitOffset {-1};
	int				m_iBitCount {-1};
	int				m_iBlobAttrId {-1};
	int				m_nBlobAttrs {0};
	bool			m_bDynamic {true};

	CSphAttrLocator () = default;

	explicit CSphAttrLocator ( int iBitOffset, int iBitCount=ROWITEM_BITS )
		: m_iBitOffset ( iBitOffset )
		, m_iBitCount ( iBitCount )
	{}

	inline bool IsBitfield () const
	{
		return ( m_iBitCount<ROWITEM_BITS || ( m_iBitOffset%ROWITEM_BITS )!=0 );
	}

	int CalcRowitem () const
	{
		return IsBitfield() ? -1 : ( m_iBitOffset / ROWITEM_BITS );
	}

	bool IsBlobAttr() const
	{
		return m_iBlobAttrId>=0;
	}

#ifndef NDEBUG
	/// get last item touched by this attr (for debugging checks only)
	int GetMaxRowitem () const
	{
		return ( m_iBitOffset + m_iBitCount - 1 ) / ROWITEM_BITS;
	}
#endif

	bool operator == ( const CSphAttrLocator & rhs ) const
	{
		if ( m_iBlobAttrId==-1 )
			return m_bDynamic==rhs.m_bDynamic && m_iBitOffset==rhs.m_iBitOffset && m_iBitCount==rhs.m_iBitCount;
		else
			return m_bDynamic==rhs.m_bDynamic && m_iBlobAttrId==rhs.m_iBlobAttrId && m_nBlobAttrs==rhs.m_nBlobAttrs;
	}
};


/// getter
inline SphAttr_t sphGetRowAttr ( const CSphRowitem * pRow, const CSphAttrLocator & tLoc )
{
	assert ( pRow );
	int iItem = tLoc.m_iBitOffset >> ROWITEM_SHIFT;

	if ( tLoc.m_iBitCount==ROWITEM_BITS )
		return pRow[iItem];

	if ( tLoc.m_iBitCount==2*ROWITEM_BITS ) // FIXME? write a generalized version, perhaps
		return SphAttr_t ( pRow[iItem] ) + ( SphAttr_t ( pRow[iItem+1] ) << ROWITEM_BITS );

	int iShift = tLoc.m_iBitOffset & ( ( 1 << ROWITEM_SHIFT )-1 );
	return ( pRow[iItem] >> iShift ) & ( ( 1UL << tLoc.m_iBitCount )-1 );
}


/// setter
inline void sphSetRowAttr ( CSphRowitem * pRow, const CSphAttrLocator & tLoc, SphAttr_t uValue )
{
	assert(pRow);
	int iItem = tLoc.m_iBitOffset >> ROWITEM_SHIFT;
	if ( tLoc.m_iBitCount==2*ROWITEM_BITS )
	{
		// FIXME? write a generalized version, perhaps
		pRow[iItem] = CSphRowitem ( uValue & ( ( SphAttr_t(1) << ROWITEM_BITS )-1 ) );
		pRow[iItem+1] = CSphRowitem ( uValue >> ROWITEM_BITS );

	} else if ( tLoc.m_iBitCount==ROWITEM_BITS )
	{
		pRow[iItem] = CSphRowitem ( uValue );

	} else
	{
		int iShift = tLoc.m_iBitOffset & ( ( 1 << ROWITEM_SHIFT )-1);
		CSphRowitem uMask = ( ( 1UL << tLoc.m_iBitCount )-1 ) << iShift;
		pRow[iItem] &= ~uMask;
		pRow[iItem] |= ( uMask & ( uValue << iShift ) );
	}
}


/// search query match (document info plus weight/tag)
class CSphMatch
{
	friend class ISphSchema;
	friend class CSphSchema;
	friend class CSphSchemaHelper;
	friend class CSphRsetSchema;

public:
	RowID_t					m_tRowID {INVALID_ROWID};	///< document ID
	const CSphRowitem *		m_pStatic {nullptr};		///< static part (stored in and owned by the index)
	CSphRowitem *			m_pDynamic {nullptr};		///< dynamic part (computed per query; owned by the match)
	int						m_iWeight {0};				///< my computed weight
	int						m_iTag {0};					///< my index tag

public:
	CSphMatch () = default;

	/// dtor. frees everything
	~CSphMatch ()
	{
		ResetDynamic();
	}

	/// reset
	void Reset ( int iDynamic )
	{
		// check that we're either initializing a new one, or NOT changing the current size
		assert ( iDynamic>=0 );
		assert ( !m_pDynamic || iDynamic==(int)m_pDynamic[-1] );

		m_tRowID = INVALID_ROWID;
		if ( !m_pDynamic && iDynamic )
		{
#ifndef NDEBUG
			m_pDynamic = new CSphRowitem [ iDynamic+1 ];
			*m_pDynamic++ = iDynamic;
#else
			m_pDynamic = new CSphRowitem [ iDynamic ];
#endif
			// dynamic stuff might contain pointers now (STRINGPTR type)
			// so we gotta cleanup
			memset ( m_pDynamic, 0, iDynamic*sizeof(CSphRowitem) );
		}
	}

	void ResetDynamic()
	{
#ifndef NDEBUG
		if ( m_pDynamic )
			m_pDynamic--;
#endif
		SafeDeleteArray ( m_pDynamic );
	}

private:
	/// assignment
	void Combine ( const CSphMatch & rhs, int iDynamic )
	{
		// check that we're either initializing a new one, or NOT changing the current size
		assert ( iDynamic>=0 );
		assert ( !m_pDynamic || iDynamic==(int)m_pDynamic[-1] );

		if ( this!=&rhs )
		{
			m_tRowID = rhs.m_tRowID;
			m_iWeight = rhs.m_iWeight;
			m_pStatic = rhs.m_pStatic;
			m_iTag = rhs.m_iTag;
		}

		if ( iDynamic )
		{
			if ( !m_pDynamic )
			{
#ifndef NDEBUG
				m_pDynamic = new CSphRowitem [ iDynamic+1 ];
				*m_pDynamic++ = iDynamic;
#else
				m_pDynamic = new CSphRowitem [ iDynamic ];
#endif
			}

			if ( this!=&rhs )
			{
				assert ( rhs.m_pDynamic );
				assert ( m_pDynamic[-1]==rhs.m_pDynamic[-1] ); // ensure we're not changing X to Y
				memcpy ( m_pDynamic, rhs.m_pDynamic, iDynamic*sizeof(CSphRowitem) );
			}
		}
	}

public:
	/// integer getter
	SphAttr_t GetAttr ( const CSphAttrLocator & tLoc ) const
	{
		// m_pRowpart[tLoc.m_bDynamic] is 30% faster on MSVC 2005
		// same time on gcc 4.x though, ~1 msec per 1M calls, so lets avoid the hassle for now
		if ( tLoc.m_iBitOffset>=0 )
			return sphGetRowAttr ( tLoc.m_bDynamic ? m_pDynamic : m_pStatic, tLoc );
		assert ( false && "Unknown negative-bitoffset locator" );
		return 0;
	}

	/// float getter
	float GetAttrFloat ( const CSphAttrLocator & tLoc ) const
	{
		return sphDW2F ( (DWORD)sphGetRowAttr ( tLoc.m_bDynamic ? m_pDynamic : m_pStatic, tLoc ) );
	}

	/// integer setter
	void SetAttr ( const CSphAttrLocator & tLoc, SphAttr_t uValue )
	{
		assert ( tLoc.m_bDynamic );
		assert ( tLoc.GetMaxRowitem() < (int)m_pDynamic[-1] );
		sphSetRowAttr ( m_pDynamic, tLoc, uValue );
	}

	/// float setter
	void SetAttrFloat ( const CSphAttrLocator & tLoc, float fValue )
	{
		assert ( tLoc.m_bDynamic );
		assert ( tLoc.GetMaxRowitem() < (int)m_pDynamic[-1] );
		sphSetRowAttr ( m_pDynamic, tLoc, sphF2DW ( fValue ) );
	}

	/// fetches blobs from both data ptr attrs and pooled blob attrs
	const BYTE * FetchAttrData ( const CSphAttrLocator & tLoc, const BYTE * pPool, int & iLengthBytes ) const;

	/// "manually" prevent copying
	CSphMatch & operator = ( const CSphMatch & ) = delete;
	CSphMatch ( const CSphMatch &) = delete;
};

/// specialized swapper
inline void Swap ( CSphMatch & a, CSphMatch & b )
{
	Swap ( a.m_tRowID, b.m_tRowID );
	Swap ( a.m_pStatic, b.m_pStatic );
	Swap ( a.m_pDynamic, b.m_pDynamic );
	Swap ( a.m_iWeight, b.m_iWeight );
	Swap ( a.m_iTag, b.m_iTag );
}


/// source statistics
struct CSphSourceStats
{
	int64_t			m_iTotalDocuments = 0;	///< how much documents
	int64_t			m_iTotalBytes = 0;		///< how much bytes

	/// reset
	void Reset ()
	{
		m_iTotalDocuments = 0;
		m_iTotalBytes = 0;
	}
};

//////////////////////////////////////////////////////////////////////////

/// known multi-valued attr sources
enum ESphAttrSrc
{
	SPH_ATTRSRC_NONE		= 0,	///< not multi-valued
	SPH_ATTRSRC_FIELD		= 1,	///< get attr values from text field
	SPH_ATTRSRC_QUERY		= 2,	///< get attr values from SQL query
	SPH_ATTRSRC_RANGEDQUERY	= 3,		///< get attr values from ranged SQL query
	SPH_ATTRSRC_RANGEDMAINQUERY	= 4		///< get attr values from main ranged SQL query, used for MVA query
};


/// wordpart processing type
enum ESphWordpart
{
	SPH_WORDPART_WHOLE		= 0,	///< whole-word
	SPH_WORDPART_PREFIX		= 1,	///< prefix
	SPH_WORDPART_INFIX		= 2		///< infix
};


/// column unpack format
enum ESphUnpackFormat
{
	SPH_UNPACK_NONE				= 0,
	SPH_UNPACK_ZLIB				= 1,
	SPH_UNPACK_MYSQL_COMPRESS	= 2
};


/// aggregate function to apply
enum ESphAggrFunc
{
	SPH_AGGR_NONE,
	SPH_AGGR_AVG,
	SPH_AGGR_MIN,
	SPH_AGGR_MAX,
	SPH_AGGR_SUM,
	SPH_AGGR_CAT
};


/// source column info
struct CSphColumnInfo
{
	enum FieldFlag_e
	{
		FIELD_NONE		= 0,
		FIELD_STORED	= 1<<0
	};

	CSphString		m_sName;		///< column name
	ESphAttr		m_eAttrType;	///< attribute type
	ESphWordpart	m_eWordpart { SPH_WORDPART_WHOLE };	///< wordpart processing type
	bool			m_bIndexed = false;		///< whether to index this column as fulltext field too

	int				m_iIndex = -1;	///< index into source result set (-1 for joined fields)
	CSphAttrLocator	m_tLocator;		///< attribute locator in the row

	ESphAttrSrc		m_eSrc { SPH_ATTRSRC_NONE };	///< attr source (for multi-valued attrs only)
	CSphString		m_sQuery;		///< query to retrieve values (for multi-valued attrs only)
	CSphString		m_sQueryRange;	///< query to retrieve range (for multi-valued attrs only)

	ISphExprRefPtr_c m_pExpr;///< evaluator for expression items
	ESphAggrFunc	m_eAggrFunc { SPH_AGGR_NONE };	///< aggregate function on top of expression (for GROUP BY)
	ESphEvalStage	m_eStage { SPH_EVAL_STATIC };///< column evaluation stage (who and how computes this column)
	bool			m_bPayload = false;
	bool			m_bFilename = false;		///< column is a file name
	bool			m_bWeight = false;			///< is a weight column
	DWORD			m_uFieldFlags = 0;			///< stored/indexed/highlighted etc

	WORD			m_uNext = 0xFFFF;			///< next in linked list for hash in CSphSchema

	/// handy ctor
	explicit CSphColumnInfo ( const char * sName=nullptr, ESphAttr eType=SPH_ATTR_NONE );

	/// equality comparison checks name, type, and locator
	bool operator == ( const CSphColumnInfo & rhs ) const
	{
		return m_sName==rhs.m_sName
			&& m_eAttrType==rhs.m_eAttrType
			&& m_tLocator.m_iBitCount==rhs.m_tLocator.m_iBitCount
			&& m_tLocator.m_iBitOffset==rhs.m_tLocator.m_iBitOffset
			&& m_tLocator.m_bDynamic==rhs.m_tLocator.m_bDynamic;
	}

	/// returns true if this column stores a pointer to data
	bool IsDataPtr() const;
};


/// barebones schema interface
/// everything that is needed from every implementation of a schema
class ISphSchema
{
public:
	/// dtor
	virtual ~ISphSchema () {}

	/// assign current schema to rset schema (kind of a visitor operator)
	virtual void					AssignTo ( class CSphRsetSchema & lhs ) const = 0;

	// get schema name
	virtual const char *			GetName() const = 0;

	virtual void					AddAttr ( const CSphColumnInfo & tAttr, bool bDynamic ) = 0;

	/// get row size (static+dynamic combined)
	virtual int						GetRowSize() const = 0;

	/// get static row part size
	virtual int						GetStaticSize() const = 0;

	/// get dynamic row part size
	virtual int						GetDynamicSize() const = 0;

	virtual int						GetAttrsCount() const = 0;
	virtual int						GetFieldsCount() const = 0;

	virtual const CSphColumnInfo &	GetField ( int iIndex ) const = 0;
	virtual const CSphVector<CSphColumnInfo> & GetFields () const = 0;

	/// get attribute index by name, returns -1 if not found
	virtual int						GetAttrIndex ( const char * sName ) const = 0;

	/// get field index by name, returns -1 if not found
	virtual int						GetFieldIndex ( const char * sName ) const = 0;
	
	/// get attr by index
	virtual const CSphColumnInfo &	GetAttr ( int iIndex ) const = 0;

	/// get attr by name
	virtual const CSphColumnInfo *	GetAttr ( const char * sName ) const = 0;

	/// get the first one of field length attributes. return -1 if none exist
	virtual int						GetAttrId_FirstFieldLen() const = 0;

	/// get the last one of field length attributes. return -1 if none exist
	virtual int						GetAttrId_LastFieldLen() const = 0;

	virtual void					FreeDataPtrs ( CSphMatch * pMatch ) const = 0;

	/// simple copy; clones either the entire dynamic part, or a part thereof
	virtual void					CloneMatch ( CSphMatch * pDst, const CSphMatch & rhs ) const = 0;

	virtual void					SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs ) = 0;
};


/// helper class that is used by CSphSchema and CSphRsetSchema
class CSphSchemaHelper : public ISphSchema
{
public:
	void	FreeDataPtrs ( CSphMatch * pMatch ) const final;
	void	CloneMatch ( CSphMatch * pDst, const CSphMatch & rhs ) const final;

	/// clone all raw attrs and only specified ptrs
	void	CloneMatchSpecial ( CSphMatch * pDst, const CSphMatch &rhs, const CSphVector<int> &dSpecials ) const;

	/// exclude vec of rowitems from dataPtrAttrs and return diff back
	CSphVector<int> SubsetPtrs ( CSphVector<int> &dSpecials ) const ;

	/// get dynamic row part size
	int GetDynamicSize () const final { return m_dDynamicUsed.GetLength (); }

	void Swap ( CSphSchemaHelper& rhs ) noexcept;

protected:
	CSphVector<int>	m_dDataPtrAttrs;		///< rowitems of pointers to data that are stored inside matches
	CSphVector<int> m_dDynamicUsed;			///< dynamic row part map

	/// generic InsertAttr() implementation that tracks data ptr attributes
	void			InsertAttr ( CSphVector<CSphColumnInfo> & dAttrs, CSphVector<int> & dUsed, int iPos, const CSphColumnInfo & tCol, bool bDynamic );
	void			Reset();

	void			CopyPtrs ( CSphMatch * pDst, const CSphMatch & rhs ) const;

public:
	// free/copy by specified vec of rowitems, assumed to be from SubsetPtrs() call.
	static void FreeDataSpecial ( CSphMatch * pMatch, const CSphVector<int> &dSpecials );
	static void	CopyPtrsSpecial ( CSphMatch * pDst, const void* pSrc, const CSphVector<int> &dSpecials );
};


/// plain good old schema
/// container that actually holds and owns all the fields, columns, etc
///
/// NOTE that while this one can be used everywhere where we need a schema
/// it might be huge (say 1000+ attributes) and expensive to copy, modify, etc!
/// so for most of the online query work, consider CSphRsetSchema
class CSphSchema : public CSphSchemaHelper
{
	friend class CSphRsetSchema;

public:
	/// ctor
	explicit				CSphSchema ( CSphString sName="(nameless)" );
							CSphSchema ( const CSphSchema & rhs );
							CSphSchema ( CSphSchema && rhs ) noexcept;
	void					Swap ( CSphSchema & rhs ) noexcept;

	CSphSchema &			operator = ( const ISphSchema & rhs );
	CSphSchema &			operator = ( CSphSchema rhs );

	/// visitor-style uber-virtual assignment implementation
	virtual void			AssignTo ( CSphRsetSchema & lhs ) const;

	virtual const char *	GetName() const;

	virtual void			AddAttr ( const CSphColumnInfo & tAttr, bool bDynamic );

	void					AddField ( const char * sFieldName );
	void					AddField ( const CSphColumnInfo & tField );

	/// get field index by name
	/// returns -1 if not found
	int						GetFieldIndex ( const char * sName ) const final;

	/// get attribute index by name
	/// returns -1 if not found
	int						GetAttrIndex ( const char * sName ) const final;

	/// checks if two schemas fully match (ie. fields names, attr names, types and locators are the same)
	/// describe mismatch (if any) to sError
	bool					CompareTo ( const CSphSchema & rhs, CSphString & sError, bool bFullComparison = true ) const;

	/// reset fields and attrs
	void					Reset ();

	/// get row size (static+dynamic combined)
	virtual int				GetRowSize () const				{ return GetStaticSize () + GetDynamicSize(); }

	/// get static row part size
	virtual int				GetStaticSize () const			{ return m_dStaticUsed.GetLength(); }

	virtual int				GetAttrsCount () const			{ return m_dAttrs.GetLength(); }
	virtual int				GetFieldsCount() const			{ return m_dFields.GetLength(); }

	virtual const CSphColumnInfo &	GetField ( int iIndex ) const { return m_dFields[iIndex]; }
	const CSphColumnInfo *	GetField ( const char * szName ) const;
	virtual const CSphVector<CSphColumnInfo> & GetFields () const { return m_dFields; }

	// most of the time we only need to get the field name
	const char *			GetFieldName ( int iIndex ) const { return m_dFields[iIndex].m_sName.cstr(); }

	/// get attr by index
	virtual const CSphColumnInfo &	GetAttr ( int iIndex ) const	{ return m_dAttrs[iIndex]; }

	/// get attr by name
	virtual const CSphColumnInfo *	GetAttr ( const char * sName ) const;

	/// insert attr
	void					InsertAttr ( int iPos, const CSphColumnInfo & tAggr, bool bDynamic );

	/// remove attr
	void					RemoveAttr ( const char * szAttr, bool bDynamic );

	/// get the first one of field length attributes. return -1 if none exist
	virtual int				GetAttrId_FirstFieldLen() const;

	/// get the last one of field length attributes. return -1 if none exist
	virtual int				GetAttrId_LastFieldLen() const;


	static bool				IsReserved ( const char * szToken );

	/// full copy, for purely dynamic matches
	void					CloneWholeMatch ( CSphMatch * pDst, const CSphMatch & rhs ) const;

	// update wordpart settings for a field
	void					SetFieldWordpart ( int iField, ESphWordpart eWordpart );

	virtual void			SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs );

	bool					HasBlobAttrs() const;
	int						GetCachedRowSize() const;
	void					SetupStoredFields ( const StrVec_t & tFields );
	bool					HasStoredFields() const;
	bool					IsFieldStored ( int iField ) const;

protected:
	static const int			HASH_THRESH		= 32;
	static const int			BUCKET_COUNT	= 256;

	WORD						m_dBuckets [ BUCKET_COUNT ];	///< uses indexes in m_dAttrs as ptrs; 0xffff is like NULL in this hash

	CSphString					m_sName;		///< my human-readable name

	int							m_iFirstFieldLenAttr = -1;///< position of the first field length attribute (cached on insert/delete)
	int							m_iLastFieldLenAttr = -1; ///< position of the last field length attribute (cached on insert/delete)

	int							m_iRowSize = 0;				///< cached row size

	CSphVector<CSphColumnInfo>	m_dFields;		///< my fulltext-searchable fields
	CSphVector<CSphColumnInfo>	m_dAttrs;		///< all my attributes
	CSphVector<int>				m_dStaticUsed;	///< static row part map (amount of used bits in each rowitem)


	/// returns 0xffff if bucket list is empty and position otherwise
	WORD &					GetBucketPos ( const char * sName );

	/// reset hash and re-add all attributes
	void					RebuildHash ();

	/// add iAddVal to all indexes strictly greater than iStartIdx in hash structures
	void					UpdateHash ( int iStartIdx, int iAddVal );
};


/// lightweight schema to be used in sorters, result sets, etc
/// avoids copying of static attributes part by keeping a pointer
/// manages the additional dynamic attributes on its own
///
/// NOTE that for that reason CSphRsetSchema needs the originating index to exist
/// (in case it keeps and uses a pointer to original schema in that index)
class CSphRsetSchema : public CSphSchemaHelper
{
protected:
	const CSphSchema *			m_pIndexSchema = nullptr;		///< original index schema, for the static part
	CSphVector<CSphColumnInfo>	m_dExtraAttrs;		///< additional dynamic attributes, for the dynamic one
	CSphVector<int>				m_dRemoved;			///< original indexes that are suppressed from the index schema by RemoveStaticAttr()

private:
	int					ActualLen() const;	///< len of m_pIndexSchema accounting removed stuff

public:
						CSphRsetSchema() = default;

	CSphRsetSchema &	operator = ( const ISphSchema & rhs );
	CSphRsetSchema &	operator = ( const CSphSchema & rhs );

	void Swap ( CSphRsetSchema & rhs ) noexcept;
	CSphRsetSchema ( const CSphRsetSchema& rhs );

	CSphRsetSchema ( CSphRsetSchema&& rhs ) noexcept { Swap(rhs); }
	CSphRsetSchema & operator = ( CSphRsetSchema rhs ) noexcept { Swap(rhs); return *this; }

	void				AddAttr ( const CSphColumnInfo & tCol, bool bDynamic ) final;
	void				AssignTo ( CSphRsetSchema & lhs ) const		final { lhs = *this; }
	const char *		GetName() const final;

public:
	int					GetRowSize() const final;
	int					GetStaticSize() const final;
	int					GetAttrsCount() const final;
	int					GetFieldsCount() const final;
	int					GetAttrIndex ( const char * sName ) const final;
	int					GetFieldIndex ( const char * sName ) const final;
	const CSphColumnInfo &	GetField ( int iIndex ) const final;
	const CSphVector<CSphColumnInfo> & GetFields () const final;
	const CSphColumnInfo &	GetAttr ( int iIndex ) const final;
	const CSphColumnInfo *	GetAttr ( const char * sName ) const final;

	int					GetAttrId_FirstFieldLen() const final;
	int					GetAttrId_LastFieldLen() const final;

public:
	void				RemoveStaticAttr ( int iAttr );
	void				Reset();

public:
	/// swap in a subset of current attributes, with not necessarily (!) unique names
	/// used to create a network result set (ie. rset to be sent and then discarded)
	/// WARNING, DO NOT USE THIS UNLESS ABSOLUTELY SURE!
	void				SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs ) final;
};

//////////////////////////////////////////////////////////////////////////

/// HTML stripper
class CSphHTMLStripper
{
public:
	explicit					CSphHTMLStripper ( bool bDefaultTags );
	bool						SetIndexedAttrs ( const char * sConfig, CSphString & sError );
	bool						SetRemovedElements ( const char * sConfig, CSphString & sError );
	bool						SetZones ( const char * sZones, CSphString & sError );
	void						EnableParagraphs ();
	void						Strip ( BYTE * sData ) const;

public:

	struct StripperTag_t
	{
		CSphString				m_sTag;			///< tag name
		int						m_iTagLen;		///< tag name length
		bool					m_bInline;		///< whether this tag is inline
		bool					m_bIndexAttrs;	///< whether to index attrs
		bool					m_bRemove;		///< whether to remove contents
		bool					m_bPara;		///< whether to mark a paragraph boundary
		bool					m_bZone;		///< whether to mark a zone boundary
		bool					m_bZonePrefix;	///< whether the zone name is a full name or a prefix
		StrVec_t				m_dAttrs;		///< attr names to index

		StripperTag_t ()
			: m_iTagLen ( 0 )
			, m_bInline ( false )
			, m_bIndexAttrs ( false )
			, m_bRemove ( false )
			, m_bPara ( false )
			, m_bZone ( false )
			, m_bZonePrefix ( false )
		{}

		inline bool operator < ( const StripperTag_t & rhs ) const
		{
			return strcmp ( m_sTag.cstr(), rhs.m_sTag.cstr() )<0;
		}
	};

	/// finds appropriate tag and zone name ( tags zone name could be prefix only )
	/// advances source to the end of the tag
	const BYTE * 				FindTag ( const BYTE * sSrc, const StripperTag_t ** ppTag, const BYTE ** ppZoneName, int * pZoneNameLen ) const;
	bool						IsValidTagStart ( int iCh ) const;

protected:
	static const int			MAX_CHAR_INDEX = 28;		///< max valid char index (a-z, underscore, colon)

	CSphVector<StripperTag_t>	m_dTags;					///< known tags to index attrs and/or to remove contents
	int							m_dStart[MAX_CHAR_INDEX];	///< maps index of the first tag name char to start offset in m_dTags
	int							m_dEnd[MAX_CHAR_INDEX];		///< maps index of the first tag name char to end offset in m_dTags

protected:
	int							GetCharIndex ( int iCh ) const;	///< calcs index by raw char
	void						UpdateTags ();				///< sorts tags, updates internal helpers
};


/// indexing-related source settings
/// NOTE, newly added fields should be synced with CSphSource::Setup()
struct CSphSourceSettings
{
	int		m_iMinPrefixLen = 0;		///< min indexable prefix (0 means don't index prefixes)
	int		m_iMinInfixLen = 0;			///< min indexable infix length (0 means don't index infixes)
	int		m_iMaxSubstringLen = 0;		///< max indexable infix and prefix (0 means don't limit infixes and prefixes)
	int		m_iBoundaryStep = 0;		///< additional boundary word position increment
	bool	m_bIndexExactWords = false;	///< exact (non-stemmed) word indexing flag
	int		m_iOvershortStep = 1;		///< position step on overshort token (default is 1)
	int		m_iStopwordStep = 1;		///< position step on stopword token (default is 1)
	bool	m_bIndexSP = false;			///< whether to index sentence and paragraph delimiters
	bool	m_bIndexFieldLens = false;	///< whether to index field lengths

	StrVec_t m_dPrefixFields;	///< list of prefix fields
	StrVec_t m_dInfixFields;	///< list of infix fields
	StrVec_t m_dStoredFields;	///< list of stored fields

	ESphWordpart			GetWordpart ( const char * sField, bool bWordDict );
};

/// hit vector interface
/// because specific position type might vary (dword, qword, etc)
/// but we don't want to template and instantiate everything because of that
class ISphHits
{
public:
	int Length () const
	{
		return m_dData.GetLength();
	}

	const CSphWordHit * First () const
	{
		return m_dData.Begin();
	}

	const CSphWordHit * Last () const
	{
		return &m_dData.Last();
	}

	void AddHit ( RowID_t tRowID, SphWordID_t uWordid, Hitpos_t uPos )
	{
		if ( uWordid )
		{
			CSphWordHit & tHit = m_dData.Add();
			tHit.m_tRowID = tRowID;
			tHit.m_uWordID = uWordid;
			tHit.m_uWordPos = uPos;
			assert ( uPos>0 );
		}
	}

public:
	CSphVector<CSphWordHit> m_dData;
};


struct SphRange_t
{
	int m_iStart;
	int m_iLength;
};

struct CSphFieldFilterSettings
{
	StrVec_t m_dRegexps;
};

/// field filter
class ISphFieldFilter : public ISphRefcountedMT
{
protected:
	virtual                     ~ISphFieldFilter ();
public:
								ISphFieldFilter();

	virtual	int					Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool bQuery ) = 0;
	virtual	void				GetSettings ( CSphFieldFilterSettings & tSettings ) const = 0;
	virtual ISphFieldFilter *	Clone() const = 0;

	void						SetParent ( ISphFieldFilter * pParent );

protected:
	ISphFieldFilter *		m_pParent;
};

using FieldFilterRefPtr_c = CSphRefcountedPtr<ISphFieldFilter>;


/// create a regexp field filter
ISphFieldFilter * sphCreateRegexpFilter ( const CSphFieldFilterSettings & tFilterSettings, CSphString & sError );

/// create an ICU field filter
ISphFieldFilter * sphCreateFilterICU ( ISphFieldFilter * pParent, const char * szBlendChars, CSphString & sError );

class BlobSource_i
{
public:
	BlobSource_i () {};
	virtual ~BlobSource_i () {};

	/// returns mva values for a given attribute (mva must be stored in a field)
	virtual CSphVector<int64_t> * GetFieldMVA ( int iAttr ) = 0;

	/// returns string attributes for a given attribute
	virtual const CSphString & GetStrAttr ( int iAttr ) = 0;
};


/// generic data source
class CSphSource : public CSphSourceSettings, public BlobSource_i
{
public:
	CSphMatch							m_tDocInfo;		///< current document info

	/// ctor
	explicit							CSphSource ( const char * sName );

	/// dtor
	virtual								~CSphSource ();

	/// set dictionary
	void								SetDict ( CSphDict * dict );

	/// set HTML stripping mode
	///
	/// sExtractAttrs defines what attributes to store. format is "img=alt; a=alt,title".
	/// empty string means to strip all tags; NULL means to disable stripping.
	///
	/// sRemoveElements defines what elements to cleanup. format is "style, script"
	///
	/// on failure, returns false and fills sError
	bool								SetStripHTML ( const char * sExtractAttrs, const char * sRemoveElements, bool bDetectParagraphs, const char * sZones, CSphString & sError );

	/// set field filter
	virtual void						SetFieldFilter ( ISphFieldFilter * pFilter );

	/// set tokenizer
	void								SetTokenizer ( ISphTokenizer * pTokenizer );

	/// set rows dump file
	virtual void						SetDumpRows ( FILE * ) {}

	/// get stats
	virtual const CSphSourceStats &		GetStats ();

	/// updates schema fields and attributes
	/// updates pInfo if it's empty; checks for match if it's not
	/// must be called after IterateStart(); will always fail otherwise
	virtual bool						UpdateSchema ( CSphSchema * pInfo, CSphString & sError );

	/// setup misc indexing settings (prefix/infix/exact-word indexing, position steps)
	void								Setup ( const CSphSourceSettings & tSettings );

	bool								SetupMorphFields ( CSphString & sError );

public:
	/// connect to the source (eg. to the database)
	/// connection settings are specific for each source type and as such
	/// are implemented in specific descendants
	virtual bool						Connect ( CSphString & sError ) = 0;

	/// disconnect from the source
	virtual void						Disconnect () = 0;

	/// check if there are any joined fields
	virtual bool						HasJoinedFields () { return false; }

	/// begin indexing this source
	/// to be implemented by descendants
	virtual bool						IterateStart ( CSphString & sError ) = 0;

	/// get next document
	/// to be implemented by descendants
	/// returns false on error
	/// returns true and sets bEOF
	virtual bool						IterateDocument ( bool & bEOF, CSphString & sError ) = 0;

	/// get next hits chunk for current document
	/// to be implemented by descendants
	/// returns NULL when there are no more hits
	/// returns pointer to hit vector (with at most MAX_SOURCE_HITS) on success
	/// fills out-string with error message on failure
	virtual ISphHits *					IterateHits ( CSphString & sError ) = 0;

	/// get joined hits from joined fields (w/o attached docinfos)
	/// returns false and fills out-string with error message on failure
	/// returns true and sets m_tDocInfo.m_uDocID to 0 on eof
	/// returns true and sets m_tDocInfo.m_uDocID to non-0 on success
	virtual ISphHits *					IterateJoinedHits ( CSphString & sError );

	/// begin iterating values of out-of-document multi-valued attribute iAttr
	/// will fail if iAttr is out of range, or is not multi-valued
	/// can also fail if configured settings are invalid (eg. SQL query can not be executed)
	virtual bool						IterateMultivaluedStart ( int iAttr, CSphString & sError ) = 0;

	/// fetch next multi-valued attribute value
	virtual bool						IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue ) = 0;

	/// notification that a rowid was assigned to a specific docid (used by joined fields)
	virtual void						RowIDAssigned ( DocID_t tDocID, RowID_t tRowID ) {}

	/// fetch fields for a current document
	virtual void						GetDocFields ( CSphVector<VecTraits_T<BYTE>> & dFields ) = 0;

	/// begin iterating kill list
	virtual bool						IterateKillListStart ( CSphString & sError ) = 0;

	/// get next kill list doc id
	virtual bool						IterateKillListNext ( DocID_t & uDocId ) = 0;

	/// post-index callback
	/// gets called when the indexing is succesfully (!) over
	virtual void						PostIndex () {}

protected:
	StrVec_t							m_dStrAttrs;	///< current document string attrs
	CSphVector<CSphVector<int64_t>>		m_dMvas;		///< per-attribute MVA storage

	TokenizerRefPtr_c				m_pTokenizer;	///< my tokenizer
	DictRefPtr_c					m_pDict;		///< my dict
	FieldFilterRefPtr_c				m_pFieldFilter;	///< my field filter

	CSphSourceStats						m_tStats;		///< my stats
	CSphSchema 							m_tSchema;		///< my schema

	CSphHTMLStripper *					m_pStripper = nullptr;	///< my HTML stripper
	CSphBitvec							m_tMorphFields;
};


/// how to handle IO errors in file fields
enum ESphOnFileFieldError
{
	FFE_IGNORE_FIELD,
	FFE_SKIP_DOCUMENT,
	FFE_FAIL_INDEX
};

struct IDPair_t
{
	DocID_t m_tDocID;
	RowID_t m_tRowID;
};


/// generic document source
/// provides multi-field support and generic tokenizer
class CSphSource_Document : public CSphSource
{
public:
	/// ctor
	explicit				CSphSource_Document ( const char * sName );

	/// dtor
							~CSphSource_Document () override { SafeDeleteArray ( m_pReadFileBuffer ); }

	// returns false when there are no more documents
	bool					IterateDocument ( bool & bEOF, CSphString & sError ) override;
	ISphHits *				IterateHits ( CSphString & sError ) override;
	void					BuildHits ( CSphString & sError, bool bSkipEndMarker );

	/// field data getter
	/// to be implemented by descendants
	virtual BYTE **			NextDocument ( bool & bEOF, CSphString & sError ) = 0;
	virtual const int *		GetFieldLengths () const = 0;

	void					SetDumpRows ( FILE * fpDumpRows ) override { m_fpDumpRows = fpDumpRows; }
	bool					HasJoinedFields () override { return m_iPlainFieldsLength!=m_tSchema.GetFieldsCount(); }

	CSphVector<int64_t> *	GetFieldMVA ( int iAttr ) override;
	const CSphString &		GetStrAttr ( int iAttr ) override;
	void					GetDocFields ( CSphVector<VecTraits_T<BYTE>> & dFields ) override;

	void					RowIDAssigned ( DocID_t tDocID, RowID_t tRowID ) override;

protected:
	void					ParseFieldMVA ( int iAttr, const char * szValue );
	bool					CheckFileField ( const BYTE * sField );
	int						LoadFileField ( BYTE ** ppField, CSphString & sError );

	bool					BuildZoneHits ( RowID_t tRowID, BYTE * sWord );
	void					BuildSubstringHits ( RowID_t tRowID, bool bPayload, ESphWordpart eWordpart, bool bSkipEndMarker );
	void					BuildRegularHits ( RowID_t tRowID, bool bPayload, bool bSkipEndMarker );

	/// register autocomputed attributes such as field lengths (see index_field_lengths)
	bool					AddAutoAttrs ( CSphString & sError, StrVec_t * pDefaults = nullptr );

	/// allocate m_tDocInfo storage, do post-alloc magic (compute pointer to field lengths, etc)
	void					AllocDocinfo ();

protected:
	ISphHits				m_tHits;				///< my hitvector

protected:
	char *					m_pReadFileBuffer = nullptr;
	int						m_iReadFileBufferSize = 256*1024;	///< size of read buffer for the 'sql_file_field' fields
	int						m_iMaxFileBufferSize = 2*1024*1024;	///< max size of read buffer for the 'sql_file_field' fields
	ESphOnFileFieldError	m_eOnFileFieldError = FFE_IGNORE_FIELD;
	FILE *					m_fpDumpRows = nullptr;
	int						m_iPlainFieldsLength = 0;
	DWORD *					m_pFieldLengthAttrs = nullptr;	///< pointer into the part of m_tDocInfo where field lengths are stored

	CSphVector<IDPair_t>	m_dAllIds;					///< used for joined fields FIXME! unlimited RAM use
	bool					m_bIdsSorted = false;			///< we sort array to use binary search

protected:
	struct CSphBuildHitsState_t
	{
		bool m_bProcessingHits;
		bool m_bDocumentDone;

		BYTE ** m_dFields;
		CSphVector<int> m_dFieldLengths;

		CSphVector<BYTE*> m_dTmpFieldStorage;
		CSphVector<BYTE*> m_dTmpFieldPtrs;
		CSphVector<BYTE> m_dFiltered;

		int m_iStartPos;
		Hitpos_t m_iHitPos;
		int m_iField;
		int m_iStartField;
		int m_iEndField;

		int m_iBuildLastStep;

		CSphBuildHitsState_t ();
		~CSphBuildHitsState_t ();

		void Reset ();
	};

	CSphBuildHitsState_t	m_tState;
	int						m_iMaxHits;
	CSphVector<CSphVector<BYTE>> m_dDocFields;
};

struct CSphUnpackInfo
{
	ESphUnpackFormat	m_eFormat;
	CSphString			m_sName;
};

struct CSphJoinedField
{
	CSphString			m_sName;
	CSphString			m_sQuery;
	CSphString			m_sRanged;
	bool				m_bPayload;
	bool				m_bRangedMain;
};


/// generic SQL source params
struct CSphSourceParams_SQL
{
	// query params
	CSphString						m_sQuery;
	CSphString						m_sQueryRange;
	CSphString						m_sQueryKilllist;
	int64_t							m_iRangeStep = 1024;
	int64_t							m_iRefRangeStep = 1024;
	bool							m_bPrintQueries = false;
	bool							m_bPrintRTQueries = false;
	CSphString						m_sDumpRTIndex;

	StrVec_t						m_dQueryPre;
	StrVec_t						m_dQueryPost;
	StrVec_t						m_dQueryPostIndex;
	CSphVector<CSphColumnInfo>		m_dAttrs;
	StrVec_t						m_dFileFields;

	int								m_iRangedThrottle = 0;
	int								m_iMaxFileBufferSize = 0;
	ESphOnFileFieldError			m_eOnFileFieldError {FFE_IGNORE_FIELD};

	CSphVector<CSphUnpackInfo>		m_dUnpack;
	DWORD							m_uUnpackMemoryLimit = 0;

	CSphVector<CSphJoinedField>		m_dJoinedFields;

	// connection params
	CSphString						m_sHost;
	CSphString						m_sUser;
	CSphString						m_sPass;
	CSphString						m_sDB;
	unsigned int					m_uPort = 0;

	// hooks
	CSphString						m_sHookConnect;
	CSphString						m_sHookQueryRange;
	CSphString						m_sHookPostIndex;
};

struct SqlQuotation_t
{
	static const char cQuote = '\'';
	inline static bool IsEscapeChar ( char c )
	{
		return ( c=='\\' || c=='\'' || c=='\t' );
	}
	inline static char GetEscapedChar ( char c ) { return c; }
};

using SqlEscapedBuilder_c = EscapedStringBuilder_T<SqlQuotation_t>;

/// generic SQL source
/// multi-field plain-text documents fetched from given query
struct CSphSource_SQL : CSphSource_Document
{
	explicit			CSphSource_SQL ( const char * sName );
						~CSphSource_SQL () override = default;

	bool				Setup ( const CSphSourceParams_SQL & pParams );
	bool				Connect ( CSphString & sError ) override;
	void				Disconnect () override;
	bool				IterateStart ( CSphString & sError ) override;
	BYTE **				NextDocument ( bool & bEOF, CSphString & sError ) override;
	const int *			GetFieldLengths () const override;
	void				PostIndex () override;

	ISphHits *			IterateJoinedHits ( CSphString & sError ) override;

	bool				IterateMultivaluedStart ( int iAttr, CSphString & sError ) override;
	bool				IterateMultivaluedNext ( int64_t & iDocID, int64_t & iMvaValue ) override;

	bool				IterateKillListStart ( CSphString & sError ) override;
	bool				IterateKillListNext ( DocID_t & tDocId ) override;

private:
	bool				m_bSqlConnected = false;	///< am i connected?

protected:
	CSphString			m_sSqlDSN;

	BYTE *				m_dFields [ SPH_MAX_FIELDS ] { nullptr };
	int					m_dFieldLengths [ SPH_MAX_FIELDS ];
	ESphUnpackFormat	m_dUnpack [ SPH_MAX_FIELDS ] { SPH_UNPACK_NONE };

	DocID_t				m_tMinID = 0;			///< grand min ID
	DocID_t				m_tMaxID = 0;			///< grand max ID
	DocID_t				m_tCurrentID = 0;		///< current min ID
	DocID_t				m_tMaxFetchedID = 0;	///< max actually fetched ID
	int					m_iMultiAttr = -1;		///< multi-valued attr being currently fetched
	int					m_iSqlFields = 0;		///< field count (for row dumper)

	CSphSourceParams_SQL		m_tParams;

	bool				m_bCanUnpack = false;
	bool				m_bUnpackFailed = false;
	bool				m_bUnpackOverflow = false;
	CSphVector<char>	m_dUnpackBuffers [ SPH_MAX_FIELDS ];

	int					m_iJoinedHitField = -1;	///< currently pulling joined hits from this field (index into schema; -1 if not pulling)
	DocID_t				m_iJoinedHitID = 0;		///< last document id
	int					m_iJoinedHitPos = 0;	///< last hit position

	static const int			MACRO_COUNT = 2;
	static const char * const	MACRO_VALUES [ MACRO_COUNT ];

protected:
	/// by what reason the internal SetupRanges called
	enum ERangesReason
	{
		SRE_DOCS,
		SRE_MVA,
		SRE_JOINEDHITS
	};

protected:
	bool					SetupRanges ( const char * sRangeQuery, const char * sQuery, const char * sPrefix, CSphString & sError, ERangesReason iReason );
	bool					RunQueryStep ( const char * sQuery, CSphString & sError );

protected:
	virtual void			SqlDismissResult () = 0;
	virtual bool			SqlQuery ( const char * sQuery ) = 0;
	virtual bool			SqlIsError () = 0;
	virtual const char *	SqlError () = 0;
	virtual bool			SqlConnect () = 0;
	virtual void			SqlDisconnect () = 0;
	virtual int				SqlNumFields() = 0;
	virtual bool			SqlFetchRow() = 0;
	virtual DWORD			SqlColumnLength ( int iIndex ) = 0;
	virtual const char *	SqlColumn ( int iIndex ) = 0;
	virtual const char *	SqlFieldName ( int iIndex ) = 0;

	const char *	SqlUnpackColumn ( int iIndex, DWORD & uUnpackedLen, ESphUnpackFormat eFormat );
	void			ReportUnpackError ( int iIndex, int iError );

	void DumpRowsHeader ();
	void DumpRowsHeaderSphinxql ();

	void DumpDocument ();
	void DumpDocumentSphinxql ();

	using TinyCol_t = std::pair<int,bool>; // int idx in sql resultset; bool whether it is string
	CSphVector<TinyCol_t>	m_dDumpMap;
	SqlEscapedBuilder_c			m_sCollectDump;
	int 					m_iCutoutDumpSize = 100*1024;
};


#if USE_MYSQL
/// MySQL source params
struct CSphSourceParams_MySQL : CSphSourceParams_SQL
{
	CSphString	m_sUsock;					///< UNIX socket
	int			m_iFlags;					///< connection flags
	CSphString	m_sSslKey;
	CSphString	m_sSslCert;
	CSphString	m_sSslCA;

				CSphSourceParams_MySQL ();	///< ctor. sets defaults
};


/// MySQL source implementation
/// multi-field plain-text documents fetched from given query
struct CSphSource_MySQL : CSphSource_SQL
{
	explicit				CSphSource_MySQL ( const char * sName );
	bool					Setup ( const CSphSourceParams_MySQL & tParams );

protected:
	MYSQL_RES *				m_pMysqlResult = nullptr;
	MYSQL_FIELD *			m_pMysqlFields = nullptr;
	MYSQL_ROW				m_tMysqlRow = nullptr;
	MYSQL					m_tMysqlDriver;
	unsigned long *			m_pMysqlLengths = nullptr;

	CSphString				m_sMysqlUsock;
	unsigned long			m_iMysqlConnectFlags = 0;
	CSphString				m_sSslKey;
	CSphString				m_sSslCert;
	CSphString				m_sSslCA;

protected:
	void			SqlDismissResult () override;
	bool			SqlQuery ( const char * sQuery ) override;
	bool			SqlIsError () override;
	const char *	SqlError () override;
	bool			SqlConnect () override;
	void			SqlDisconnect () override;
	int				SqlNumFields() override;
	bool			SqlFetchRow() override;
	DWORD			SqlColumnLength ( int iIndex ) override;
	const char *	SqlColumn ( int iIndex ) override;
	const char *	SqlFieldName ( int iIndex ) override;
};
#endif // USE_MYSQL


#if USE_PGSQL
/// PgSQL specific source params
struct CSphSourceParams_PgSQL : CSphSourceParams_SQL
{
	CSphString		m_sClientEncoding;
					CSphSourceParams_PgSQL ();
};


/// PgSQL source implementation
/// multi-field plain-text documents fetched from given query
struct CSphSource_PgSQL : CSphSource_SQL
{
	explicit				CSphSource_PgSQL ( const char * sName );
	bool					Setup ( const CSphSourceParams_PgSQL & pParams );
	bool					IterateStart ( CSphString & sError ) final;

protected:
	PGresult * 				m_pPgResult = nullptr;	///< postgresql execution restult context
	PGconn *				m_tPgDriver = nullptr;	///< postgresql connection context

	int						m_iPgRows = 0;		///< how much rows last step returned
	int						m_iPgRow = 0;		///< current row (0 based, as in PQgetvalue)

	CSphString				m_sPgClientEncoding;
	CSphVector<bool>		m_dIsColumnBool;

protected:
	void			SqlDismissResult () final;
	bool			SqlQuery ( const char * sQuery ) final;
	bool			SqlIsError () final;
	const char *	SqlError () final;
	bool			SqlConnect () final;
	void			SqlDisconnect () final;
	int				SqlNumFields() final;
	bool			SqlFetchRow() final;
	DWORD	SqlColumnLength ( int iIndex ) final;
	const char *	SqlColumn ( int iIndex ) final;
	const char *	SqlFieldName ( int iIndex ) final;
};
#endif // USE_PGSQL

#if USE_ODBC
struct CSphSourceParams_ODBC: CSphSourceParams_SQL
{
	CSphString	m_sOdbcDSN;			///< ODBC DSN
	CSphString	m_sColBuffers;		///< column buffer sizes (eg "col1=2M, col2=4M")
	bool		m_bWinAuth;			///< auth type (MS SQL only)

				CSphSourceParams_ODBC ();
};

/// ODBC source implementation
struct CSphSource_ODBC : CSphSource_SQL
{
	explicit				CSphSource_ODBC ( const char * sName );
	bool					Setup ( const CSphSourceParams_ODBC & tParams );

protected:
	virtual void			SqlDismissResult ();
	virtual bool			SqlQuery ( const char * sQuery );
	virtual bool			SqlIsError ();
	virtual const char *	SqlError ();
	virtual bool			SqlConnect ();
	virtual void			SqlDisconnect ();
	virtual int				SqlNumFields();
	virtual bool			SqlFetchRow();
	virtual const char *	SqlColumn ( int iIndex );
	virtual const char *	SqlFieldName ( int iIndex );
	virtual DWORD			SqlColumnLength ( int iIndex );

	virtual void			OdbcPostConnect () {}

protected:
	CSphString				m_sOdbcDSN;
	bool					m_bWinAuth;
	bool					m_bUnicode;

	SQLHENV					m_hEnv;
	SQLHDBC					m_hDBC;
	SQLHANDLE				m_hStmt;
	int						m_nResultCols;
	CSphString				m_sError;

	struct QueryColumn_t
	{
		CSphVector<char>	m_dContents;
		CSphVector<char>	m_dRaw;
		CSphString			m_sName;
		SQLLEN				m_iInd;
		int					m_iBytes;		///< size of actual data in m_dContents, in bytes
		int					m_iBufferSize;	///< size of m_dContents and m_dRaw buffers, in bytes
		bool				m_bUCS2;		///< whether this column needs UCS-2 to UTF-8 translation
		bool				m_bTruncated;	///< whether data was truncated when fetching rows
	};

	static const int		DEFAULT_COL_SIZE	= 1024;			///< default column buffer size
	static const int		VARCHAR_COL_SIZE	= 1048576;		///< default column buffer size for VARCHAR columns
	static const int		MAX_COL_SIZE		= 8*1048576;	///< hard limit on column buffer size
	static const int		WARN_ROW_SIZE		= 32*1048576;	///< warning thresh (NOT a hard limit) on row buffer size

	CSphVector<QueryColumn_t>	m_dColumns;
	SmallStringHash_T<int>		m_hColBuffers;

	void					GetSqlError ( SQLSMALLINT iHandleType, SQLHANDLE hHandle );
};


/// MS SQL source implementation
struct CSphSource_MSSQL : public CSphSource_ODBC
{
	explicit				CSphSource_MSSQL ( const char * sName ) : CSphSource_ODBC ( sName ) { m_bUnicode=true; }
	virtual void			OdbcPostConnect ();
};
#endif // USE_ODBC


#if USE_LIBEXPAT
class CSphConfigSection;
CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, FILE * pPipe, const char * szSourceName, int iMaxFieldLen, CSphString & sError );
#endif


/////////////////////////////////////////////////////////////////////////////
// SEARCH QUERIES
/////////////////////////////////////////////////////////////////////////////

/// search query sorting orders
enum ESphSortOrder
{
	SPH_SORT_RELEVANCE		= 0,	///< sort by document relevance desc, then by date
	SPH_SORT_ATTR_DESC		= 1,	///< sort by document date desc, then by relevance desc
	SPH_SORT_ATTR_ASC		= 2,	///< sort by document date asc, then by relevance desc
	SPH_SORT_TIME_SEGMENTS	= 3,	///< sort by time segments (hour/day/week/etc) desc, then by relevance desc
	SPH_SORT_EXTENDED		= 4,	///< sort by SQL-like expression (eg. "@relevance DESC, price ASC, @id DESC")
	SPH_SORT_EXPR			= 5,	///< sort by arithmetic expression in descending order (eg. "@id + max(@weight,1000)*boost + log(price)")

	SPH_SORT_TOTAL
};


/// search query matching mode
enum ESphMatchMode
{
	SPH_MATCH_ALL = 0,			///< match all query words
	SPH_MATCH_ANY,				///< match any query word
	SPH_MATCH_PHRASE,			///< match this exact phrase
	SPH_MATCH_BOOLEAN,			///< match this boolean query
	SPH_MATCH_EXTENDED,			///< match this extended query
	SPH_MATCH_FULLSCAN,			///< match all document IDs w/o fulltext query, apply filters
	SPH_MATCH_EXTENDED2,		///< extended engine V2 (TEMPORARY, WILL BE REMOVED IN 0.9.8-RELEASE)

	SPH_MATCH_TOTAL
};


/// search query relevance ranking mode
enum ESphRankMode
{
	SPH_RANK_PROXIMITY_BM25		= 0,	///< default mode, phrase proximity major factor and BM25 minor one (aka SPH03)
	SPH_RANK_BM25				= 1,	///< statistical mode, BM25 ranking only (faster but worse quality)
	SPH_RANK_NONE				= 2,	///< no ranking, all matches get a weight of 1
	SPH_RANK_WORDCOUNT			= 3,	///< simple word-count weighting, rank is a weighted sum of per-field keyword occurence counts
	SPH_RANK_PROXIMITY			= 4,	///< phrase proximity (aka SPH01)
	SPH_RANK_MATCHANY			= 5,	///< emulate old match-any weighting (aka SPH02)
	SPH_RANK_FIELDMASK			= 6,	///< sets bits where there were matches
	SPH_RANK_SPH04				= 7,	///< codename SPH04, phrase proximity + bm25 + head/exact boost
	SPH_RANK_EXPR				= 8,	///< rank by user expression (eg. "sum(lcs*user_weight)*1000+bm25")
	SPH_RANK_EXPORT				= 9,	///< rank by BM25, but compute and export all user expression factors
	SPH_RANK_PLUGIN				= 10,	///< user-defined ranker

	SPH_RANK_TOTAL,
	SPH_RANK_DEFAULT			= SPH_RANK_PROXIMITY_BM25
};


/// search query grouping mode
enum ESphGroupBy
{
	SPH_GROUPBY_DAY		= 0,	///< group by day
	SPH_GROUPBY_WEEK	= 1,	///< group by week
	SPH_GROUPBY_MONTH	= 2,	///< group by month
	SPH_GROUPBY_YEAR	= 3,	///< group by year
	SPH_GROUPBY_ATTR	= 4,	///< group by attribute value
	SPH_GROUPBY_ATTRPAIR= 5,	///< group by sequential attrs pair (rendered redundant by 64bit attrs support; removed)
	SPH_GROUPBY_MULTIPLE= 6		///< group by on multiple attribute values
};


/// search query filter types
enum ESphFilter
{
	SPH_FILTER_VALUES		= 0,	///< filter by integer values set
	SPH_FILTER_RANGE		= 1,	///< filter by integer range
	SPH_FILTER_FLOATRANGE	= 2,	///< filter by float range
	SPH_FILTER_STRING		= 3,	///< filter by string value
	SPH_FILTER_NULL			= 4,	///< filter by NULL
	SPH_FILTER_USERVAR		= 5,	///< filter by @uservar
	SPH_FILTER_STRING_LIST	= 6,	///< filter by string list
	SPH_FILTER_EXPRESSION	= 7		///< filter by expression
};


/// MVA folding function
/// (currently used in filters, eg WHERE ALL(mymva) BETWEEN 1 AND 3)
enum  ESphMvaFunc
{
	SPH_MVAFUNC_NONE = 0,
	SPH_MVAFUNC_ANY,
	SPH_MVAFUNC_ALL
};


/// search query filter
class CSphFilterSettings
{
public:
	CSphString			m_sAttrName = "";	///< filtered attribute name
	bool				m_bExclude = false;		///< whether this is "include" or "exclude" filter (default is "include")
	bool				m_bHasEqualMin = true;	///< has filter "equal" component or pure greater\less (for min)
	bool				m_bHasEqualMax = true;	///< has filter "equal" component or pure greater\less (for max)
	bool				m_bOpenLeft = false;
	bool				m_bOpenRight = false;
	bool				m_bIsNull = false;		///< for NULL or NOT NULL

	ESphFilter			m_eType = SPH_FILTER_VALUES;		///< filter type
	ESphMvaFunc			m_eMvaFunc = SPH_MVAFUNC_NONE;		///< MVA and stringlist folding function
	union
	{
		SphAttr_t		m_iMinValue = LLONG_MIN;	///< range min
		float			m_fMinValue;	///< range min
	};
	union
	{
		SphAttr_t		m_iMaxValue = LLONG_MAX;	///< range max
		float			m_fMaxValue;	///< range max
	};
	CSphVector<SphAttr_t>	m_dValues;	///< integer values set
	StrVec_t				m_dStrings;	///< string values

public:
						CSphFilterSettings () = default;

	void				SetExternalValues ( const SphAttr_t * pValues, int nValues );

	SphAttr_t			GetValue ( int iIdx ) const	{ assert ( iIdx<GetNumValues() ); return m_pValues ? m_pValues[iIdx] : m_dValues[iIdx]; }
	const SphAttr_t *	GetValueArray () const		{ return m_pValues ? m_pValues : m_dValues.Begin(); }
	int					GetNumValues () const		{ return m_pValues ? m_nValues : m_dValues.GetLength (); }

	bool				operator == ( const CSphFilterSettings & rhs ) const;
	bool				operator != ( const CSphFilterSettings & rhs ) const { return !( (*this)==rhs ); }

	uint64_t			GetHash() const;

protected:
	const SphAttr_t *	m_pValues = nullptr;		///< external value array
	int					m_nValues = 0;		///< external array size
};


// keyword info
struct CSphKeywordInfo
{
	CSphString		m_sTokenized;
	CSphString		m_sNormalized;
	int				m_iDocs = 0;
	int				m_iHits = 0;
	int				m_iQpos = 0;
};

inline void Swap ( CSphKeywordInfo & v1, CSphKeywordInfo & v2 )
{
	v1.m_sTokenized.Swap ( v2.m_sTokenized );
	v1.m_sNormalized.Swap ( v2.m_sNormalized );
	::Swap ( v1.m_iDocs, v2.m_iDocs );
	::Swap ( v1.m_iHits, v2.m_iHits );
	::Swap ( v1.m_iQpos, v2.m_iQpos );
}


/// query selection item
struct CSphQueryItem
{
	CSphString		m_sExpr;		///< expression to compute
	CSphString		m_sAlias;		///< alias to return
	ESphAggrFunc	m_eAggrFunc { SPH_AGGR_NONE };
};

/// search query complex filter tree
struct FilterTreeItem_t
{
	int m_iLeft = -1;		// left node at parser filter operations
	int m_iRight = -1;		// right node at parser filter operations
	int m_iFilterItem = -1;	// index into query filters 
	bool m_bOr = false;

	bool operator == ( const FilterTreeItem_t & rhs ) const;
	bool operator != ( const FilterTreeItem_t & rhs ) const { return !( (*this)==rhs ); }
	uint64_t GetHash() const;
};

/// table function interface
struct CSphQuery;
struct AggrResult_t;
class ISphTableFunc
{
public:
	virtual			~ISphTableFunc() {}
	virtual bool	ValidateArgs ( const StrVec_t & dArgs, const CSphQuery & tQuery, CSphString & sError ) = 0;
	virtual bool	Process ( AggrResult_t * pResult, CSphString & sError ) = 0;
	virtual bool	LimitPushdown ( int, int ) { return false; } // FIXME! implement this
};

enum QueryType_e
{
	QUERY_API,
	QUERY_SQL,
	QUERY_JSON
};

class QueryParser_i;

enum QueryOption_e
{
	QUERY_OPT_DEFAULT = 0,
	QUERY_OPT_DISABLED,
	QUERY_OPT_ENABLED,
	QUERY_OPT_MORPH_NONE
};

enum IndexHint_e
{
	INDEX_HINT_USE,
	INDEX_HINT_FORCE,
	INDEX_HINT_IGNORE
};


struct IndexHint_t
{
	CSphString		m_sIndex;
	IndexHint_e		m_eHint{INDEX_HINT_USE};
};

const int DEFAULT_MAX_MATCHES = 1000;

/// search query. Pure struct, no member functions
struct CSphQuery
{
	CSphString		m_sIndexes {"*"};	///< indexes to search
	CSphString		m_sQuery;			///< cooked query string for the engine (possibly transformed during legacy matching modes fixup)
	CSphString		m_sRawQuery;		///< raw query string from the client for searchd log, agents, etc

	int				m_iOffset=0;		///< offset into result set (as X in MySQL LIMIT X,Y clause)
	int				m_iLimit=20;		///< limit into result set (as Y in MySQL LIMIT X,Y clause)
	CSphVector<DWORD>	m_dWeights;		///< user-supplied per-field weights. may be NULL. default is NULL
	ESphMatchMode	m_eMode = SPH_MATCH_EXTENDED;		///< match mode. default is "match all"
	ESphRankMode	m_eRanker = SPH_RANK_DEFAULT;		///< ranking mode, default is proximity+BM25
	CSphString		m_sRankerExpr;		///< ranking expression for SPH_RANK_EXPR
	CSphString		m_sUDRanker;		///< user-defined ranker name
	CSphString		m_sUDRankerOpts;	///< user-defined ranker options
	ESphSortOrder	m_eSort = SPH_SORT_RELEVANCE;		///< sort mode
	CSphString		m_sSortBy;			///< attribute to sort by
	int64_t			m_iRandSeed = -1;	///< random seed for ORDER BY RAND(), -1 means do not set
	int				m_iMaxMatches = DEFAULT_MAX_MATCHES;	///< max matches to retrieve, default is 1000. more matches use more memory and CPU time to hold and sort them

	bool			m_bSortKbuffer = false;		///< whether to use PQ or K-buffer sorting algorithm
	bool			m_bZSlist = false;			///< whether the ranker has to fetch the zonespanlist with this query
	bool			m_bSimplify = false;		///< whether to apply boolean simplification
	bool			m_bPlainIDF = false;		///< whether to use PlainIDF=log(N/n) or NormalizedIDF=log((N-n+1)/n)
	bool			m_bGlobalIDF = false;		///< whether to use local indexes or a global idf file
	bool			m_bNormalizedTFIDF = true;	///< whether to scale IDFs by query word count, so that TF*IDF is normalized
	bool			m_bLocalDF = false;			///< whether to use calculate DF among local indexes
	bool			m_bLowPriority = false;		///< set low thread priority for this query
	DWORD			m_uDebugFlags = 0;
	QueryOption_e	m_eExpandKeywords = QUERY_OPT_DEFAULT;	///< control automatic query-time keyword expansion

	CSphVector<CSphFilterSettings>	m_dFilters;	///< filters
	CSphVector<FilterTreeItem_t>	m_dFilterTree;

	CSphVector<IndexHint_t>			m_dIndexHints; ///< secondary index hints

	CSphString		m_sGroupBy;			///< group-by attribute name(s)
	CSphString		m_sFacetBy;			///< facet-by attribute name(s)
	ESphGroupBy		m_eGroupFunc = SPH_GROUPBY_ATTR;	///< function to pre-process group-by attribute value with
	CSphString		m_sGroupSortBy { "@groupby desc" };	///< sorting clause for groups in group-by mode
	CSphString		m_sGroupDistinct;		///< count distinct values for this attribute

	int				m_iCutoff = 0;			///< matches count threshold to stop searching at (default is 0; means to search until all matches are found)

	int				m_iRetryCount = -1;		///< retry count, for distributed queries. (-1 means 'use default')
	int				m_iRetryDelay = -1;		///< retry delay, for distributed queries. (-1 means 'use default')
	int				m_iAgentQueryTimeout = 0;	///< agent query timeout override, for distributed queries

	bool			m_bGeoAnchor = false;	///< do we have an anchor
	CSphString		m_sGeoLatAttr;			///< latitude attr name
	CSphString		m_sGeoLongAttr;			///< longitude attr name
	float			m_fGeoLatitude = 0.0f;	///< anchor latitude
	float			m_fGeoLongitude = 0.0f;	///< anchor longitude

	CSphVector<CSphNamedInt>	m_dIndexWeights;	///< per-index weights
	CSphVector<CSphNamedInt>	m_dFieldWeights;	///< per-field weights

	DWORD			m_uMaxQueryMsec = 0;	///< max local index search time, in milliseconds (default is 0; means no limit)
	int				m_iMaxPredictedMsec = 0; ///< max predicted (!) search time limit, in milliseconds (0 means no limit)
	CSphString		m_sComment;				///< comment to pass verbatim in the log file

	CSphString		m_sSelect;				///< select-list (attributes and/or expressions)
	CSphString		m_sOrderBy;				///< order-by clause

	CSphString		m_sOuterOrderBy;		///< temporary (?) subselect hack
	int				m_iOuterOffset = 0;		///< keep and apply outer offset at master
	int				m_iOuterLimit = 0;
	bool			m_bHasOuter = false;

	bool			m_bReverseScan = false;		///< perform scan in reverse order
	bool			m_bIgnoreNonexistent = false; ///< whether to warning or not about non-existent columns in select list
	bool			m_bIgnoreNonexistentIndexes = false; ///< whether to error or not about non-existent indexes in index list
	bool			m_bStrict = false;			///< whether to warning or not about incompatible types
	bool			m_bSync = false;			///< whether or not use synchronous operations (optimize, etc.)

	ISphTableFunc *	m_pTableFunc = nullptr;		///< post-query NOT OWNED, WILL NOT BE FREED in dtor.
	CSphFilterSettings	m_tHaving;				///< post aggregate filtering (got applied only on master)

	int				m_iSQLSelectStart = -1;	///< SQL parser helper
	int				m_iSQLSelectEnd = -1;	///< SQL parser helper

	int				m_iGroupbyLimit = 1;	///< number of elems within group

	CSphVector<CSphQueryItem>	m_dItems;		///< parsed select-list
	CSphVector<CSphQueryItem>	m_dRefItems;	///< select-list prior replacing by facet
	ESphCollation				m_eCollation = SPH_COLLATION_DEFAULT;	///< ORDER BY collation
	bool						m_bAgent = false;	///< agent mode (may need extra cols on output)

	CSphString		m_sQueryTokenFilterLib;		///< token filter library name
	CSphString		m_sQueryTokenFilterName;	///< token filter name
	CSphString		m_sQueryTokenFilterOpts;	///< token filter options

	bool			m_bFacet = false;			///< whether this a facet query
	bool			m_bFacetHead = false;

	QueryType_e		m_eQueryType {QUERY_API};		///< queries from sphinxql require special handling
	const QueryParser_i * m_pQueryParser = nullptr;	///< queries do not own this parser

	StrVec_t m_dIncludeItems;
	StrVec_t m_dExcludeItems;
};

/// parse select list string into items
bool ParseSelectList ( CSphString &sError, CSphQuery &pResult );

/// some low-level query stats
struct CSphQueryStats
{
	int64_t *	m_pNanoBudget = nullptr;///< pointer to max_predicted_time budget (counted in nanosec)
	DWORD		m_iFetchedDocs = 0;		///< processed documents
	DWORD		m_iFetchedHits = 0;		///< processed hits (aka positions)
	DWORD		m_iSkips = 0;			///< number of Skip() calls

	void		Add ( const CSphQueryStats & tStats );
};


/// search query meta-info
class CSphQueryResultMeta
{
public:
	int						m_iQueryTime = 0;		///< query time, milliseconds
	int						m_iRealQueryTime = 0;	///< query time, measured just from start to finish of the query. In milliseconds
	int64_t					m_iCpuTime = 0;			///< user time, microseconds
	int						m_iMultiplier = 1;		///< multi-query multiplier, -1 to indicate error

	struct WordStat_t
	{
		int64_t					m_iDocs = 0;		///< document count for this term
		int64_t					m_iHits = 0;		///< hit count for this term
	};
	SmallStringHash_T<WordStat_t>	m_hWordStats; 	///< hash of i-th search term (normalized word form)

	int						m_iMatches = 0;			///< total matches returned (upto MAX_MATCHES)
	int64_t					m_iTotalMatches = 0;	///< total matches found (unlimited)

	CSphIOStats				m_tIOStats;				///< i/o stats for the query
	int64_t					m_iAgentCpuTime = 0;	///< agent cpu time (for distributed searches)
	CSphIOStats				m_tAgentIOStats;		///< agent IO stats (for distributed searches)

	int64_t					m_iPredictedTime = 0;		///< local predicted time
	int64_t					m_iAgentPredictedTime = 0;	///< distributed predicted time
	DWORD					m_iAgentFetchedDocs = 0;	///< distributed fetched docs
	DWORD					m_iAgentFetchedHits = 0;	///< distributed fetched hits
	DWORD					m_iAgentFetchedSkips = 0;	///< distributed fetched skips

	CSphQueryStats 			m_tStats;					///< query prediction counters
	bool					m_bHasPrediction = false;	///< is prediction counters set?

	CSphString				m_sError;				///< error message
	CSphString				m_sWarning;				///< warning message
	int64_t					m_iBadRows = 0;

	virtual					~CSphQueryResultMeta () {}					///< dtor
	void					AddStat ( const CSphString & sWord, int64_t iDocs, int64_t iHits );
};


/// search query result (meta-info plus actual matches)
class CSphQueryProfile;
class DocstoreReader_i;
class CSphQueryResult : public CSphQueryResultMeta
{
public:
	CSphSwapVector<CSphMatch>	m_dMatches;			///< top matching documents, no more than MAX_MATCHES
	CSphSchema				m_tSchema;				///< result schema
	const BYTE *			m_pBlobPool = nullptr;	///< pointer to blob attr storage
	const DocstoreReader_i* m_pDocstore = nullptr;	///< pointer to docstore reader
	int						m_iOffset = 0;			///< requested offset into matches array
	int						m_iCount = 0;			///< count which will be actually served (computed from total, offset and limit)
	int						m_iSuccesses = 0;
	CSphQueryProfile *		m_pProfile = nullptr;	///< filled when query profiling is enabled; NULL otherwise

							~CSphQueryResult () override; 	///< dtor, which releases all owned stuff
};

/////////////////////////////////////////////////////////////////////////////
// ATTRIBUTE UPDATE QUERY
/////////////////////////////////////////////////////////////////////////////

struct TypedAttribute_t
{
	CSphString	m_sName;
	ESphAttr	m_eType;
};


struct CSphAttrUpdate
{
	CSphVector<TypedAttribute_t>	m_dAttributes;	///< update schema, attributes to update
	CSphVector<DWORD>				m_dPool;		///< update values pool
	CSphVector<BYTE>				m_dBlobs;		///< update pool for blob attrs
	CSphVector<DocID_t>				m_dDocids;		///< document IDs vector
	CSphVector<int>					m_dRowOffset;	///< document row offsets in the pool (1 per doc, i.e. the length is the same as of m_dDocids)
	bool							m_bIgnoreNonexistent = false;	///< whether to warn about non-existen attrs, or just silently ignore them
	bool							m_bStrict = false;				///< whether to check for incompatible types first, or just ignore them
};

/////////////////////////////////////////////////////////////////////////////
// FULLTEXT INDICES
/////////////////////////////////////////////////////////////////////////////

/// progress info
struct CSphIndexProgress
{
	enum Phase_e
	{
		PHASE_COLLECT,				///< document collection phase
		PHASE_SORT,					///< final sorting phase
		PHASE_LOOKUP,				///< docid lookup construction
		PHASE_HISTOGRAMS,			///< creating histograms for POD attrs
		PHASE_MERGE					///< index merging
	};

	Phase_e			m_ePhase { PHASE_COLLECT };		///< current indexing phase

	int64_t			m_iDocuments = 0;	///< PHASE_COLLECT: documents collected so far
	int64_t			m_iBytes = 0;		///< PHASE_COLLECT: bytes collected so far;
										///< PHASE_PREREAD: bytes read so far;
	int64_t			m_iBytesTotal = 0;	///< PHASE_PREREAD: total bytes to read;

	SphOffset_t		m_iHits {0};		///< PHASE_SORT: hits sorted so far
	SphOffset_t		m_iHitsTotal {0};	///< PHASE_SORT: hits total

	int64_t			m_iDocids {0};		///< PHASE_LOOKUP: docids added to lookup so far
	int64_t			m_iDocidsTotal {0};	///< PHASE_LOOKUP: total docids

	int				m_iWords = 0;		///< PHASE_MERGE: words merged so far

	int				m_iDone = 0;		///< generic percent, 0..1000 range

	typedef void ( *IndexingProgress_fn ) ( const CSphIndexProgress * pStat, bool bPhaseEnd );
	IndexingProgress_fn m_fnProgress {nullptr};

	/// builds a message to print
	/// WARNING, STATIC BUFFER, NON-REENTRANT
	const char * BuildMessage() const;

	void Show ( bool bPhaseEnd ) const;
};


/// match sorting functions
enum ESphSortFunc
{
	FUNC_REL_DESC,
	FUNC_ATTR_DESC,
	FUNC_ATTR_ASC,
	FUNC_TIMESEGS,
	FUNC_GENERIC1,
	FUNC_GENERIC2,
	FUNC_GENERIC3,
	FUNC_GENERIC4,
	FUNC_GENERIC5,
	FUNC_EXPR
};


/// match sorting clause parsing outcomes
enum ESortClauseParseResult
{
	SORT_CLAUSE_OK,
	SORT_CLAUSE_ERROR,
	SORT_CLAUSE_RANDOM
};


/// sorting key part types
enum ESphSortKeyPart
{
	SPH_KEYPART_ROWID,
	SPH_KEYPART_DOCID_S,
	SPH_KEYPART_DOCID_D,
	SPH_KEYPART_WEIGHT,
	SPH_KEYPART_INT,
	SPH_KEYPART_FLOAT,
	SPH_KEYPART_STRING,
	SPH_KEYPART_STRINGPTR
};


/// JSON key lookup stuff
struct JsonKey_t
{
	CSphString		m_sKey;			///< name string
	DWORD			m_uMask = 0;	///< Bloom mask for this key
	int				m_iLen = 0;		///< name length, in bytes

	JsonKey_t () = default;
	explicit JsonKey_t ( const char * sKey, int iLen );
};


// iLen1 and iLen2 don't need to be specified for STRINGPTR attrs
typedef int ( *SphStringCmp_fn )( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 );


/// match comparator state
struct CSphMatchComparatorState
{
	static const int	MAX_ATTRS = 5;

	ESphSortKeyPart		m_eKeypart[MAX_ATTRS];		///< sort-by key part type
	CSphAttrLocator		m_tLocator[MAX_ATTRS];		///< sort-by attr locator
	JsonKey_t			m_tSubKeys[MAX_ATTRS];		///< sort-by attr sub-locator
	ISphExpr *			m_tSubExpr[MAX_ATTRS];		///< sort-by attr expression
	ESphAttr			m_tSubType[MAX_ATTRS];		///< sort-by expression type
	int					m_dAttrs[MAX_ATTRS];		///< sort-by attr index

	DWORD				m_uAttrDesc = 0;			///< sort order mask (if i-th bit is set, i-th attr order is DESC)
	DWORD				m_iNow = 0;					///< timestamp (for timesegments sorting mode)
	SphStringCmp_fn		m_fnStrCmp = nullptr;		///< string comparator


	/// create default empty state
	CSphMatchComparatorState ()
	{
		for ( int i=0; i<MAX_ATTRS; ++i )
		{
			m_eKeypart[i] = SPH_KEYPART_ROWID;
			m_tSubExpr[i] = nullptr;
			m_tSubType[i] = SPH_ATTR_NONE;
			m_dAttrs[i] = -1;
		}
	}

	~CSphMatchComparatorState ()
	{
		for ( ISphExpr *&pExpr :  m_tSubExpr ) SafeRelease( pExpr );
	}

	/// check if any of my attrs are bitfields
	bool UsesBitfields ()
	{
		for ( int i=0; i<MAX_ATTRS; ++i )
			if ( m_eKeypart[i]==SPH_KEYPART_INT && m_tLocator[i].IsBitfield() )
				return true;
		return false;
	}

	inline int CmpStrings ( const CSphMatch & a, const CSphMatch & b, int iAttr ) const
	{
		assert ( iAttr>=0 && iAttr<MAX_ATTRS );
		assert ( m_eKeypart[iAttr]==SPH_KEYPART_STRING || m_eKeypart[iAttr]==SPH_KEYPART_STRINGPTR );
		assert ( m_fnStrCmp );

		const BYTE * aa = (const BYTE*) a.GetAttr ( m_tLocator[iAttr] );
		const BYTE * bb = (const BYTE*) b.GetAttr ( m_tLocator[iAttr] );
		if ( aa==NULL || bb==NULL )
		{
			if ( aa==bb )
				return 0;
			if ( aa==NULL )
				return -1;
			return 1;
		}

		return m_fnStrCmp ( aa, bb, m_eKeypart[iAttr]==SPH_KEYPART_STRINGPTR, 0, 0 );
	}

	void FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, bool bRemapKeyparts );
};


/// match processor interface
struct ISphMatchProcessor
{
	virtual ~ISphMatchProcessor () {}
	virtual void Process ( CSphMatch * pMatch ) = 0;
};


/// generic match sorter interface
class ISphMatchSorter
{
public:
	bool				m_bRandomize;
	int64_t				m_iTotal;

	RowID_t				m_iJustPushed {INVALID_ROWID};
	int					m_iMatchCapacity;
	CSphTightVector<RowID_t> m_dJustPopped;

protected:
	ISphSchema *		m_pSchema { nullptr };	///< sorter schema (adds dynamic attributes on top of index schema)
	CSphMatchComparatorState	m_tState;		///< protected to set m_iNow automatically on SetState() calls

public:
	/// ctor
						ISphMatchSorter () : m_bRandomize ( false ), m_iTotal ( 0 ), m_iMatchCapacity ( 0 ) {}

	/// virtualizing dtor
	virtual				~ISphMatchSorter ();

	/// check if this sorter does groupby
	virtual bool		IsGroupby () const = 0;

	/// set match comparator state
	void		SetState ( const CSphMatchComparatorState & tState );

	/// get match comparator stat
	const CSphMatchComparatorState &	GetState() const { return m_tState; }

	/// set group comparator state
	virtual void		SetGroupState ( const CSphMatchComparatorState & ) {}

	/// set blob pool pointer (for string+groupby sorters)
	virtual void		SetBlobPool ( const BYTE * ) {}

	/// set sorter schema
	virtual void		SetSchema ( ISphSchema * pSchema, bool bRemapCmp );

	/// get incoming schema
	const ISphSchema * GetSchema () const { return m_pSchema; }

	/// base push
	/// returns false if the entry was rejected as duplicate
	/// returns true otherwise (even if it was not actually inserted)
	virtual bool		Push ( const CSphMatch & tEntry ) = 0;

	/// submit pre-grouped match. bNewSet indicates that the match begins the bunch of matches got from one source
	virtual bool		PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) = 0;

	/// get	rough entries count, due of aggregate filtering phase
	virtual int			GetLength () const = 0;

	/// get internal buffer length
	virtual int			GetDataLength () const = 0;

	/// get total count of non-duplicates Push()ed through this queue
	int64_t		GetTotalCount () const { return m_iTotal; }

	/// process collected entries up to length count
	virtual void		Finalize ( ISphMatchProcessor & tProcessor, bool bCallProcessInResultSetOrder ) = 0;

	/// store all entries into specified location and remove them from the queue
	/// entries are stored in properly sorted order,
	/// if iTag is non-negative, entries are also tagged; otherwise, their tag's unchanged
	/// return sorted entries count, might be less than length due of aggregate filtering phase
	virtual int			Flatten ( CSphMatch * pTo, int iTag ) = 0;

	/// get a pointer to the worst element, NULL if there is no fixed location
	virtual const CSphMatch *	GetWorst() const { return nullptr; }
};

struct CmpPSortersByRandom_fn
{
	inline static bool IsLess ( const ISphMatchSorter * a, const ISphMatchSorter * b )
	{
		assert ( a );
		assert ( b );
		return a->m_bRandomize<b->m_bRandomize;
	}
};


enum ESphHitless
{
	SPH_HITLESS_NONE		= 0,	///< all hits are present
	SPH_HITLESS_SOME		= 1,	///< some of the hits might be omitted (check the flag bit)
	SPH_HITLESS_ALL			= 2	///< no hits in this index
};


enum ESphHitFormat
{
	SPH_HIT_FORMAT_PLAIN	= 0,	///< all hits are stored in hitlist
	SPH_HIT_FORMAT_INLINE	= 1	///< hits can be split and inlined into doclist (aka 9-23)
};


enum class Preprocessor_e
{
	NONE,			///< no preprocessor
	ICU				///< ICU chinese preprocessor
};


struct KillListTarget_t
{
	enum
	{
		USE_KLIST	= 1 << 0,
		USE_DOCIDS	= 1 << 1
	};

	CSphString		m_sIndex;
	DWORD			m_uFlags {USE_KLIST|USE_DOCIDS};
};


enum class Compression_e
{
	NONE,
	LZ4,
	LZ4HC
};


const DWORD DEFAULT_DOCSTORE_BLOCK = 16384;
const int DEFAULT_COMPRESSION_LEVEL = 9;

struct DocstoreSettings_t
{
	Compression_e	m_eCompression		= Compression_e::LZ4;
	int				m_iCompressionLevel	= DEFAULT_COMPRESSION_LEVEL;
	DWORD			m_uBlockSize		= DEFAULT_DOCSTORE_BLOCK;
};


struct CSphIndexSettings : CSphSourceSettings, DocstoreSettings_t
{
	ESphHitFormat	m_eHitFormat = SPH_HIT_FORMAT_PLAIN;
	bool			m_bHtmlStrip = false;
	CSphString		m_sHtmlIndexAttrs;
	CSphString		m_sHtmlRemoveElements;
	CSphString		m_sZones;
	ESphHitless		m_eHitless = SPH_HITLESS_NONE;
	CSphString		m_sHitlessFiles;
	bool			m_bVerbose = false;
	int				m_iEmbeddedLimit = 0;
	SphOffset_t		m_tBlobUpdateSpace {0};
	int				m_iSkiplistBlockSize {32};

	CSphVector<KillListTarget_t> m_dKlistTargets;	///< list of indexes to apply killlist to

	ESphBigram		m_eBigramIndex = SPH_BIGRAM_NONE;
	CSphString		m_sBigramWords;
	StrVec_t		m_dBigramWords;

	DWORD			m_uAotFilterMask = 0;			///< lemmatize_XX_all forces us to transform queries on the index level too
	Preprocessor_e	m_ePreprocessor = Preprocessor_e::NONE;

	CSphString		m_sIndexTokenFilter;	///< indexing time token filter spec string (pretty useless for disk, vital for RT)
};


/// forward refs to internal searcher classes
class ISphQword;
class ISphQwordSetup;
class CSphQueryContext;
struct ISphFilter;
struct GetKeywordsSettings_t;
struct SuggestArgs_t;
struct SuggestResult_t;


struct ISphKeywordsStat
{
	virtual			~ISphKeywordsStat() {}
	virtual bool	FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const = 0;
};


struct CSphIndexStatus
{
	int64_t			m_iRamUse = 0;
	int64_t			m_iRamRetired = 0;
	int64_t			m_iDiskUse = 0;
	int64_t			m_iRamChunkSize = 0; // not used for plain
	int				m_iNumChunks = 0; // not used for plain
	int64_t			m_iMemLimit = 0; // not used for plain
	int64_t			m_iTID = 0;
	int64_t			m_iSavedTID = 0;
};


struct CSphMultiQueryArgs : public ISphNoncopyable
{
	const int								m_iIndexWeight;
	int										m_iTag = 0;
	DWORD									m_uPackedFactorFlags { SPH_FACTOR_DISABLE };
	bool									m_bLocalDF = false;
	const SmallStringHash_T<int64_t> *		m_pLocalDocs = nullptr;
	int64_t									m_iTotalDocs = 0;
	bool									m_bModifySorterSchemas {true};

	CSphMultiQueryArgs ( int iIndexWeight );
};

enum KeywordExpansion_e
{
	KWE_DISABLED	=	0,
	KWE_EXACT	=		( 1UL << 0 ),
	KWE_STAR	=		( 1UL << 1 ),
	KWE_MORPH_NONE	=	( 1UL << 2 ),

	KWE_ENABLED = ( KWE_EXACT | KWE_STAR ),
};


// an index or a part of an index that has its own row ids
class IndexSegment_c
{
public:
	virtual int		Kill ( DocID_t tDocID ) = 0;
	virtual int		KillMulti ( const VecTraits_T<DocID_t> & dKlist ) = 0;
};


struct UpdatedRowData_t
{
	CSphRowitem *		m_pRow {nullptr};
	IndexSegment_c *	m_pSegment {nullptr};
	CSphRowitem *		m_pAttrPool {nullptr};
	BYTE *				m_pBlobPool {nullptr};
	bool				m_bUpdated {false};
};

class Histogram_i;
class HistogramContainer_c;

enum TypeConversion_e
{
	CONVERSION_NONE,
	CONVERSION_BIGINT2FLOAT,
	CONVERSION_FLOAT2BIGINT,
};

struct UpdatedAttribute_t
{
	CSphAttrLocator		m_tLocator;
	CSphRefcountedPtr<ISphExpr>	m_pExpr;
	Histogram_i *		m_pHistogram {nullptr};
	ESphAttr			m_eAttrType {SPH_ATTR_NONE};
	TypeConversion_e	m_eConversion {CONVERSION_NONE};
	bool				m_bExisting {false};
};


struct UpdateContext_t
{
	const CSphAttrUpdate &					m_tUpd;
	const ISphSchema &						m_tSchema;
	const HistogramContainer_c *			m_pHistograms {nullptr};

	CSphVector<UpdatedRowData_t>			m_dUpdatedRows;
	CSphFixedVector<UpdatedAttribute_t>		m_dUpdatedAttrs;

	CSphBitvec			m_dSchemaUpdateMask;
	DWORD				m_uUpdateMask {0};

	int					m_iFirst {0};
	int					m_iLast {0};
	bool				m_bBlobUpdate {false};
	int					m_iJsonWarnings {0};


						UpdateContext_t ( const CSphAttrUpdate & tUpd, const ISphSchema & tSchema, const HistogramContainer_c * pHistograms, int iFirst, int iLast );

	UpdatedRowData_t &	GetRowData ( int iUpd );
};


// common attribute update code for both RT and plain indexes
class IndexUpdateHelper_c
{
protected:
	enum
	{
		ATTRS_UPDATED			= ( 1UL<<0 ),
		ATTRS_BLOB_UPDATED		= ( 1UL<<1 ),
		ATTRS_ROWMAP_UPDATED	= ( 1UL<<2 )
	};

	virtual bool		Update_WriteBlobRow ( UpdateContext_t & tCtx, int iUpd, CSphRowitem * pDocinfo, const BYTE * pBlob, int iLength, int nBlobAttrs, bool & bCritical, CSphString & sError ) = 0;

	bool				Update_FixupData ( UpdateContext_t & tCtx, CSphString & sError );
	bool				Update_InplaceJson ( UpdateContext_t & tCtx, CSphString & sError, bool bDryRun );
	bool				Update_Blobs ( UpdateContext_t & tCtx, bool & bCritical, CSphString & sError );
	void				Update_Plain ( UpdateContext_t & tCtx );
	bool				Update_HandleJsonWarnings ( UpdateContext_t & tCtx, int iUpdated, CSphString & sWarning, CSphString & sError );
};


class WriteWrapper_c
{
public:
	virtual void		PutBytes ( const BYTE * pData, int iSize ) = 0;
	virtual SphOffset_t	GetPos() const = 0;
	virtual bool		IsError() const = 0;
};

// common add/remove attribute code for both RT and plain indexes
class IndexAlterHelper_c
{
protected:
	bool				Alter_AddRemoveAttr ( const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphRowitem * pDocinfo, const CSphRowitem * pDocinfoMax, const BYTE * pBlobPool, WriteWrapper_c & tSPAWriter, WriteWrapper_c & tSPBWriter, bool bAddAttr, const CSphString & sAttrName );
	bool 				Alter_AddRemoveFromSchema ( CSphSchema & tSchema, const CSphString & sAttrName, ESphAttr eAttrType, bool bAdd, CSphString & sError );

	virtual bool		Alter_IsMinMax ( const CSphRowitem * pDocinfo, int iStride ) const;

private:
	void				CreateAttrMap ( CSphVector<int> & dAttrMap, const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, int iAttrToRemove );
	const CSphRowitem *	CopyRow ( const CSphRowitem * pDocinfo, DWORD * pTmpDocinfo, const CSphColumnInfo * pNewAttr, int iOldStride );
	const CSphRowitem * CopyRowAttrByAttr ( const CSphRowitem * pDocinfo, DWORD * pTmpDocinfo, const CSphSchema & tOldSchema, const CSphSchema & tNewSchema, const CSphVector<int> & dAttrMap, int iOldStride );
};


// simple error reporter for debug checks
class DebugCheckError_c
{
public:
			DebugCheckError_c ( FILE * pFile );

	void	Fail ( const char * szFmt, ... );
	void	Msg ( const char * szFmt, ... );
	void	Progress ( const char * szFmt, ... );
	void	Done();

	void	SetSegment ( int iSegment );
	int64_t	GetNumFails() const;

private:
	FILE *	m_pFile {nullptr};
	bool	m_bProgress {false};
	int64_t m_tStartTime {0};
	int64_t	m_nFails {0};
	int64_t	m_nFailsPrinted {0};
	int		m_iSegment {-1};
};


class DocstoreFields_i;
void SetupDocstoreFields ( DocstoreFields_i & tFields, const CSphSchema & tSchema );


struct DocstoreDoc_t
{
	CSphVector<CSphVector<BYTE>> m_dFields;
};


enum DocstoreDataType_e
{
	DOCSTORE_TEXT,
	DOCSTORE_BIN
};


// used to fetch documents from docstore by docids
class DocstoreReader_i
{
public:
	virtual void	CreateReader ( int64_t iSessionId ) const {}
	virtual bool	GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const = 0;
	virtual int		GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const = 0;
};


enum class FileAccess_e
{
	FILE,
	MMAP,
	MMAP_PREREAD,
	MLOCK,
	UNKNOWN
};

bool IsMlock ( FileAccess_e eType );
bool IsOndisk ( FileAccess_e eType );

// returns correct size even if iBuf is 0
int GetReadBuffer ( int iBuf );

struct FileAccessSettings_t
{
	FileAccess_e	m_eAttr;
	FileAccess_e	m_eBlob;
	FileAccess_e	m_eDoclist;
	FileAccess_e	m_eHitlist;
	int				m_iReadBufferDocList;
	int				m_iReadBufferHitList;

	bool operator== ( const FileAccessSettings_t & tOther ) const;
	bool operator!= ( const FileAccessSettings_t & tOther ) const;
};

/// generic fulltext index interface
class CSphIndex : public ISphKeywordsStat, public IndexSegment_c, public DocstoreReader_i
{
public:
	explicit					CSphIndex ( const char * sIndexName, const char * sFilename );
	virtual						~CSphIndex ();

	virtual const CSphString &	GetLastError () const { return m_sLastError; }
	virtual const CSphString &	GetLastWarning () const { return m_sLastWarning; }
	virtual const CSphSchema &	GetMatchSchema () const { return m_tSchema; }			///< match schema as returned in result set (possibly different from internal storage schema!)

	virtual	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn pfnProgress ) = 0;
	virtual void				SetInplaceSettings ( int iHitGap, float fRelocFactor, float fWriteFactor );
	virtual void				SetPreopen ( bool bValue ) { m_bKeepFilesOpen = bValue; }
	void						SetFieldFilter ( ISphFieldFilter * pFilter );
	const ISphFieldFilter *		GetFieldFilter() const { return m_pFieldFilter; }
	void						SetTokenizer ( ISphTokenizer * pTokenizer );
	void						SetupQueryTokenizer();
	const ISphTokenizer *		GetTokenizer () const { return m_pTokenizer; }
	const ISphTokenizer *		GetQueryTokenizer () const { return m_pQueryTokenizer; }
	ISphTokenizer *				LeakTokenizer ();
	void						SetDictionary ( CSphDict * pDict );
	CSphDict *					GetDictionary () const { return m_pDict; }
	CSphDict *					LeakDictionary ();
	virtual void				SetKeepAttrs ( const CSphString & , const StrVec_t & ) {}
	virtual void				Setup ( const CSphIndexSettings & tSettings );
	const CSphIndexSettings &	GetSettings () const { return m_tSettings; }
	bool						IsStripperInited () const { return m_bStripperInited; }
	virtual bool				IsRT() const { return false; }
	virtual bool				IsPQ() const { return false; }
	void						SetBinlog ( bool bBinlog ) { m_bBinlog = bBinlog; }
	virtual int64_t *			GetFieldLens() const { return NULL; }
	virtual bool				IsStarDict() const { return true; }
	int64_t						GetIndexId() const { return m_iIndexId; }

public:
	/// build index by indexing given sources
	virtual int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer ) = 0;

	/// build index by mering current index with given index
	virtual bool				Merge ( CSphIndex * pSource, const CSphVector<CSphFilterSettings> & dFilters, bool bSupressDstDocids ) = 0;

public:
	/// check all data files, preload schema, and preallocate enough RAM to load memory-cached data
	virtual bool				Prealloc ( bool bStripPath ) = 0;

	/// deallocate all previously preallocated shared data
	virtual void				Dealloc () = 0;

	/// precache everything which needs to be precached
	virtual void				Preread () = 0;

	/// set new index base path
	virtual void				SetBase ( const char * sNewBase ) = 0;

	/// set new index base path, and physically rename index files too
	virtual bool				Rename ( const char * sNewBase ) = 0;

	/// obtain exclusive lock on this index
	virtual bool				Lock () = 0;

	/// dismiss exclusive lock and unlink lock file
	virtual void				Unlock () = 0;

	/// called when index is loaded and prepared to work
	virtual void				PostSetup();

public:
	/// return index document, bytes totals (FIXME? remove this in favor of GetStatus() maybe?)
	virtual const CSphSourceStats &		GetStats () const = 0;

	/// return additional index info
	virtual void				GetStatus ( CSphIndexStatus* ) const = 0;

public:
	virtual bool				EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const = 0;
	void						SetCacheSize ( int iMaxCachedDocs, int iMaxCachedHits );
	virtual bool				MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const = 0;
	virtual bool				MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const = 0;
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const = 0;
	virtual void				GetSuggest ( const SuggestArgs_t & , SuggestResult_t & ) const {}

public:
	/// returns non-negative amount of actually found and updated records on success
	/// on failure, -1 is returned and GetLastError() contains error message
	virtual int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, bool & bCritical, CSphString & sError, CSphString & sWarning ) = 0;

	/// saves memory-cached attributes, if there were any updates to them
	/// on failure, false is returned and GetLastError() contains error message
	virtual bool				SaveAttributes ( CSphString & sError ) const = 0;

	virtual DWORD				GetAttributeStatus () const = 0;

	virtual bool				AddRemoveAttribute ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError ) = 0;

	virtual void				FlushDeadRowMap ( bool bWaitComplete ) const {};
	virtual bool				LoadKillList ( CSphFixedVector<DocID_t> * pKillList, CSphVector<KillListTarget_t> & dTargets, CSphString & sError ) const { return true; }
	virtual bool				AlterKillListTarget ( CSphVector<KillListTarget_t> & dTargets, CSphString & sError ) { return false; }
	virtual void				KillExistingDocids ( CSphIndex * pTarget ) {}
	int							KillMulti ( const VecTraits_T<DocID_t> & dKlist ) override { return 0; }

	bool						GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const override { return false; }
	int							GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const override { return -1; }

public:
	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpHeader ( FILE * fp, const char * sHeaderName, bool bConfig ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpDocids ( FILE * fp ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpDict ( FILE * fp ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual int					DebugCheck ( FILE * fp ) = 0;
	virtual void				SetDebugCheck ( bool bCheckIdDups ) {}

	/// getter for name
	const char *				GetName () const { return m_sIndexName.cstr(); }

	void						SetName ( const char * sName ) { m_sIndexName = sName; }

	/// get for the base file name
	const char *				GetFilename () const { return m_sFilename.cstr(); }

	/// get actual index files list
	virtual void				GetIndexFiles ( CSphVector<CSphString> & dFiles ) const {};

	/// internal make document id list from external docinfo, DO NOT USE
	virtual CSphFixedVector<SphAttr_t> BuildDocList () const;

	virtual void				SetMemorySettings ( const FileAccessSettings_t & tFileAccessSettings ) = 0;

	virtual void				GetFieldFilterSettings ( CSphFieldFilterSettings & tSettings );

public:
	int64_t						m_iTID = 0;				///< last committed transaction id

	int							m_iExpandKeywords = KWE_DISABLED;	///< enable automatic query-time keyword expansion (to "( word | =word | *word* )")
	int							m_iExpansionLimit = 0;

protected:
	static CSphAtomic			m_tIdGenerator;

	int64_t						m_iIndexId;				///< internal (per daemon) unique index id, introduced for caching

	CSphSchema					m_tSchema;
	CSphString					m_sLastError;
	CSphString					m_sLastWarning;

	bool						m_bInplaceSettings = false;
	int							m_iHitGap = 0;
	float						m_fRelocFactor { 0.0f };
	float						m_fWriteFactor { 0.0f };

	bool						m_bKeepFilesOpen = false;	///< keep files open to avoid race on seamless rotation
	bool						m_bBinlog = true;

	bool						m_bStripperInited = true;	///< was stripper initialized (old index version (<9) handling)

protected:
	CSphIndexSettings			m_tSettings;

	FieldFilterRefPtr_c		m_pFieldFilter;
	TokenizerRefPtr_c		m_pTokenizer;
	TokenizerRefPtr_c		m_pQueryTokenizer;
	TokenizerRefPtr_c		m_pQueryTokenizerJson;
	DictRefPtr_c			m_pDict;

	int							m_iMaxCachedDocs = 0;
	int							m_iMaxCachedHits = 0;
	CSphString					m_sIndexName;			///< index ID in binlogging; otherwise used only in messages.
	CSphString					m_sFilename;

public:
	void						SetGlobalIDFPath ( const CSphString & sPath ) { m_sGlobalIDFPath = sPath; }
	float						GetGlobalIDF ( const CSphString & sWord, int64_t iDocsLocal, bool bPlainIDF ) const;

protected:
	CSphString					m_sGlobalIDFPath;
};

// update attributes with index pointer attached
struct CSphAttrUpdateEx
{
	const CSphAttrUpdate *	m_pUpdate = nullptr;	///< the unchangeable update pool
	CSphIndex *				m_pIndex = nullptr;		///< the index on which the update should happen
	CSphString *			m_pError = nullptr;		///< the error, if any
	CSphString *			m_pWarning = nullptr;	///< the warning, if any
	int						m_iAffected = 0;		///< num of updated rows.
};

struct SphQueueSettings_t : public ISphNoncopyable
{
	const ISphSchema &			m_tSchema;
	CSphQueryProfile *			m_pProfiler;
	bool						m_bComputeItems = true;
	CSphAttrUpdateEx *			m_pUpdate = nullptr;
	CSphVector<DocID_t> *		m_pCollection = nullptr;
	ISphExprHook *				m_pHook = nullptr;
	const CSphFilterSettings *	m_pAggrFilter = nullptr;

	SphQueueSettings_t ( const ISphSchema & tSchema, CSphQueryProfile * pProfiler = nullptr )
		: m_tSchema ( tSchema )
		, m_pProfiler ( pProfiler )
	{}
};

struct SphQueueRes_t : public ISphNoncopyable
{
	bool						m_bZonespanlist = false;
	DWORD						m_uPackedFactorFlags { SPH_FACTOR_DISABLE };

	bool						m_bAlowMulti = true;
};

/////////////////////////////////////////////////////////////////////////////

/// create phrase fulltext index implementation
CSphIndex *			sphCreateIndexPhrase ( const char* szIndexName, const char * sFilename );

/// create template (tokenizer) index implementation
CSphIndex *			sphCreateIndexTemplate ( );

/// set JSON attribute indexing options
/// bStrict is whether to stop indexing on error, or just ignore the attribute value
/// bAutoconvNumbers is whether to auto-convert eligible (!) strings to integers and floats, or keep them as strings
/// bKeynamesToLowercase is whether to convert all key names to lowercase
void				sphSetJsonOptions ( bool bStrict, bool bAutoconvNumbers, bool bKeynamesToLowercase );

/// parses sort clause, using a given schema
/// fills eFunc and tState and optionally sError, returns result code
ESortClauseParseResult	sphParseSortClause ( const CSphQuery * pQuery, const char * sClause, const ISphSchema & tSchema,
	ESphSortFunc & eFunc, CSphMatchComparatorState & tState, bool bComputeItems, CSphString & sError );

/// creates proper queue for given query
/// may return NULL on error; in this case, error message is placed in sError
/// if the pUpdate is given, creates the updater's queue and perform the index update
/// instead of searching
ISphMatchSorter *	sphCreateQueue ( const CSphQuery & tQuery, const SphQueueSettings_t & tQueue, CSphString & sError, SphQueueRes_t & tRes, StrVec_t * pExtra );
bool sphCreateMultiQueue ( const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, VecTraits_T<StrVec_t> & dExtras );

/// convert queue to sorted array, and add its entries to result's matches array
int					sphFlattenQueue ( ISphMatchSorter * pQueue, CSphQueryResult * pResult, int iTag );

/// setup per-keyword read buffer sizes
void SetUnhintedBuffer ( int iReadUnhinted );

/// check query for expressions
bool				sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema );

/// initialize collation tables
void				sphCollationInit ();

//////////////////////////////////////////////////////////////////////////

extern CSphString g_sLemmatizerBase;

// Get global shutdown flag
volatile bool& sphGetShutdown();

// Access to global TFO settings
volatile int& sphGetTFO();
#define TFO_CONNECT 1
#define TFO_LISTEN 2
#define TFO_ABSENT (-1)
/////////////////////////////////////////////////////////////////////////////
// workaround to suppress C4511/C4512 warnings (copy ctor and assignment operator) in VS 2003
#if _MSC_VER>=1300 && _MSC_VER<1400
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#endif

// suppress C4201 (nameless struct/union is a nonstandard extension) because even min-spec gcc 3.4.6 works ok
#if defined(_MSC_VER)
#pragma warning(disable:4201)
#endif

#endif // _sphinx_
