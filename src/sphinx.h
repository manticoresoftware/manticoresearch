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

#ifndef _sphinx_
#define _sphinx_

/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
	#define USE_MYSQL		1	/// whether to compile MySQL support
	#define USE_PGSQL		0	/// whether to compile PgSQL support
	#define USE_ODBC		1	/// whether to compile ODBC support
	#define USE_LIBEXPAT	1	/// whether to compile libexpat support
	#define USE_LIBICONV	1	/// whether to compile iconv support
	#define USE_LIBXML		0	/// whether to compile libxml support
	#define	USE_LIBSTEMMER	0	/// whether to compile libstemmber support
	#define USE_WINDOWS		1	/// whether to compile for Windows
	#define USE_SYSLOG		0	/// whether to use syslog for logging

	#define UNALIGNED_RAM_ACCESS	1
	#define USE_LITTLE_ENDIAN		1
#else
	#define USE_WINDOWS		0	/// whether to compile for Windows
#endif

/////////////////////////////////////////////////////////////////////////////

#include "sphinxstd.h"
#include "sphinxexpr.h" // to remove?

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if USE_PGSQL
#include <libpq-fe.h>
#endif

#if USE_WINDOWS
#include <winsock2.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#if USE_MYSQL
#include <mysql.h>
#endif

#if USE_WINDOWS
typedef __int64				SphOffset_t;
#define STDOUT_FILENO		fileno(stdout)
#define STDERR_FILENO		fileno(stderr)
#else
typedef off_t				SphOffset_t;
#endif

#if USE_ODBC
#include <sqlext.h>
#endif

/////////////////////////////////////////////////////////////////////////////

#ifndef USE_64BIT
#define USE_64BIT 0
#endif

#if USE_64BIT

// use 64-bit unsigned integers to store document and word IDs
#define SPHINX_BITS_TAG	"-id64"
typedef uint64_t		SphWordID_t;
typedef uint64_t		SphDocID_t;

#define DOCID_MAX		U64C(0xffffffffffffffff)
#define DOCID_FMT		UINT64_FMT
#define DOCINFO_IDSIZE	2

STATIC_SIZE_ASSERT ( SphWordID_t, 8 );
STATIC_SIZE_ASSERT ( SphDocID_t, 8 );

#else

// use 32-bit unsigned integers to store document and word IDs
#define SPHINX_BITS_TAG	""
typedef DWORD			SphWordID_t;
typedef DWORD			SphDocID_t;

#define DOCID_MAX		0xffffffffUL
#define DOCID_FMT		"%u"
#define DOCINFO_IDSIZE	1

STATIC_SIZE_ASSERT ( SphWordID_t, 4 );
STATIC_SIZE_ASSERT ( SphDocID_t, 4 );

#endif // USE_64BIT

#define DWSIZEOF(a) ( sizeof(a) / sizeof(DWORD) )

//////////////////////////////////////////////////////////////////////////

/// row entry (storage only, does not necessarily map 1:1 to attributes)
typedef DWORD			CSphRowitem;

/// widest integer type that can be be stored as an attribute (ideally, fully decoupled from rowitem size!)
typedef int64_t			SphAttr_t;

const CSphRowitem		ROWITEM_MAX		= UINT_MAX;
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

inline void DOCINFOSETID ( DWORD * pDocinfo, DWORD uValue )
{
	*pDocinfo = uValue;
}

inline void DOCINFOSETID ( DWORD * pDocinfo, uint64_t uValue )
{
#if USE_LITTLE_ENDIAN
	pDocinfo[0] = (DWORD)uValue;
	pDocinfo[1] = (DWORD)(uValue>>32);
#else
	pDocinfo[0] = (DWORD)(uValue>>32);
	pDocinfo[1] = (DWORD)uValue;
#endif
}

inline SphDocID_t DOCINFO2ID ( const DWORD * pDocinfo )
{
	return DOCINFO2ID_T<SphDocID_t> ( pDocinfo );
}

#if PARANOID
template < typename DOCID > inline DWORD *			DOCINFO2ATTRS_T ( DWORD * pDocinfo )		{ assert ( pDocinfo ); return pDocinfo+DWSIZEOF(DOCID); }
template < typename DOCID > inline const DWORD *	DOCINFO2ATTRS_T ( const DWORD * pDocinfo )	{ assert ( pDocinfo ); return pDocinfo+DWSIZEOF(DOCID); }
template < typename DOCID > inline DWORD *			STATIC2DOCINFO_T ( DWORD * pAttrs )		{ assert ( pDocinfo ); return pAttrs-DWSIZEOF(DOCID); }
template < typename DOCID > inline const DWORD *	STATIC2DOCINFO_T ( const DWORD * pAttrs )	{ assert ( pDocinfo ); return pAttrs-DWSIZEOF(DOCID); }
#else
template < typename DOCID > inline DWORD *			DOCINFO2ATTRS_T ( DWORD * pDocinfo )		{ return pDocinfo + DWSIZEOF(DOCID); }
template < typename DOCID > inline const DWORD *	DOCINFO2ATTRS_T ( const DWORD * pDocinfo )	{ return pDocinfo + DWSIZEOF(DOCID); }
template < typename DOCID > inline DWORD *			STATIC2DOCINFO_T ( DWORD * pAttrs )		{ return pAttrs - DWSIZEOF(DOCID); }
template < typename DOCID > inline const DWORD *	STATIC2DOCINFO_T ( const DWORD * pAttrs )	{ return pAttrs - DWSIZEOF(DOCID); }
#endif

inline 			DWORD *	DOCINFO2ATTRS ( DWORD * pDocinfo )			{ return DOCINFO2ATTRS_T<SphDocID_t>(pDocinfo); }
inline const	DWORD *	DOCINFO2ATTRS ( const DWORD * pDocinfo )	{ return DOCINFO2ATTRS_T<SphDocID_t>(pDocinfo); }
inline 			DWORD *	STATIC2DOCINFO ( DWORD * pAttrs )			{ return STATIC2DOCINFO_T<SphDocID_t>(pAttrs); }
inline const	DWORD *	STATIC2DOCINFO ( const DWORD * pAttrs )	{ return STATIC2DOCINFO_T<SphDocID_t>(pAttrs); }


/////////////////////////////////////////////////////////////////////////////

#include "sphinxversion.h"

#ifndef SPHINX_TAG
#define SPHINX_TAG "-dev"
#endif

#define SPHINX_VERSION			"2.0.2" SPHINX_BITS_TAG SPHINX_TAG " (" SPH_SVN_TAGREV ")"
#define SPHINX_BANNER			"Sphinx " SPHINX_VERSION "\nCopyright (c) 2001-2011, Andrew Aksyonoff\nCopyright (c) 2008-2011, Sphinx Technologies Inc (http://sphinxsearch.com)\n\n"
#define SPHINX_SEARCHD_PROTO	1

#define SPH_MAX_WORD_LEN		42		// so that any UTF-8 word fits 127 bytes
#define SPH_MAX_FILENAME_LEN	512
#define SPH_MAX_FIELDS			256

/////////////////////////////////////////////////////////////////////////////

/// microsecond precision timestamp
/// current UNIX timestamp in seconds multiplied by 1000000, plus microseconds since the beginning of current second
int64_t			sphMicroTimer ();

/// Sphinx CRC32 implementation
DWORD			sphCRC32 ( const BYTE * pString );
DWORD			sphCRC32 ( const BYTE * pString, int iLen );
DWORD			sphCRC32 ( const BYTE * pString, int iLen, DWORD uPrevCRC );

/// Sphinx FNV64 implementation
const uint64_t	SPH_FNV64_SEED = 0xcbf29ce484222325ULL;
uint64_t		sphFNV64 ( const BYTE * pString );
uint64_t		sphFNV64 ( const BYTE * s, int iLen, uint64_t uPrev = SPH_FNV64_SEED );

/// calculate file crc32
bool			sphCalcFileCRC32 ( const char * szFilename, DWORD & uCRC32 );

/// replaces all occurences of sMacro in sTemplate with textual representation of uValue
char *			sphStrMacro ( const char * sTemplate, const char * sMacro, SphDocID_t uValue );

/// try to obtain an exclusive lock on specified file
/// bWait specifies whether to wait
bool			sphLockEx ( int iFile, bool bWait );

/// remove existing locks
void			sphLockUn ( int iFile );

/// millisecond-precision sleep
void			sphSleepMsec ( int iMsec );

/// check if file exists and is a readable file
bool			sphIsReadable ( const char * sFilename, CSphString * pError=NULL );

/// set throttling options
void			sphSetThrottling ( int iMaxIOps, int iMaxIOSize );

/// immediately interrupt current query
void			sphInterruptNow();

#if !USE_WINDOWS
/// set process info
void			sphSetProcessInfo ( bool bHead );
#endif

struct CSphIOStats
{
	int64_t		m_iReadTime;
	DWORD		m_iReadOps;
	int64_t		m_iReadBytes;
	int64_t		m_iWriteTime;
	DWORD		m_iWriteOps;
	int64_t		m_iWriteBytes;
};

/// clear stats, starts collecting
void				sphStartIOStats ();

/// stops collecting stats, returns results
const CSphIOStats &	sphStopIOStats ();

/// startup mva updates arena
DWORD *				sphArenaInit ( int iMaxBytes );

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

extern const char *		SPHINX_DEFAULT_SBCS_TABLE;
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
public:
				CSphLowercaser ();
				~CSphLowercaser ();

	void		Reset ();
	void		SetRemap ( const CSphLowercaser * pLC );
	void		AddRemaps ( const CSphVector<CSphRemapRange> & dRemaps, DWORD uFlags );
	void		AddSpecials ( const char * sSpecials );
	uint64_t	GetFNV () const;

public:
	const CSphLowercaser &		operator = ( const CSphLowercaser & rhs );

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

protected:
	static const int	CHUNK_COUNT	= 0x300;
	static const int	CHUNK_BITS	= 8;

	static const int	CHUNK_SIZE	= 1 << CHUNK_BITS;
	static const int	CHUNK_MASK	= CHUNK_SIZE - 1;
	static const int	MAX_CODE	= CHUNK_COUNT * CHUNK_SIZE;

	int					m_iChunks;					///< how much chunks are actually allocated
	int *				m_pData;					///< chunks themselves
	int *				m_pChunk [ CHUNK_COUNT ];	///< pointers to non-empty chunks
};

/////////////////////////////////////////////////////////////////////////////

struct CSphSavedFile
{
	CSphString			m_sFilename;
	SphOffset_t			m_uSize;
	SphOffset_t			m_uCTime;
	SphOffset_t			m_uMTime;
	DWORD				m_uCRC32;

						CSphSavedFile ();
};


struct CSphTokenizerSettings
{
	int					m_iType;
	CSphString			m_sCaseFolding;
	int					m_iMinWordLen;
	CSphString			m_sSynonymsFile;
	CSphString			m_sBoundary;
	CSphString			m_sIgnoreChars;
	int					m_iNgramLen;
	CSphString			m_sNgramChars;
	CSphString			m_sBlendChars;
	CSphString			m_sBlendMode;

						CSphTokenizerSettings ();
};

struct CSphMultiformContainer;

/// generic tokenizer
class ISphTokenizer
{
public:
	/// trivial ctor
									ISphTokenizer();

	/// trivial dtor
	virtual							~ISphTokenizer () {}

public:
	/// set new translation table
	/// returns true on success, false on failure
	virtual bool					SetCaseFolding ( const char * sConfig, CSphString & sError );

	/// add additional range to translation table
	virtual void					AddCaseFolding ( CSphRemapRange & tRange );

	/// add special chars to translation table (SBCS only, for now)
	/// updates lowercaser so that these remap to -1
	virtual void					AddSpecials ( const char * sSpecials );

	/// set ignored characters
	virtual bool					SetIgnoreChars ( const char * sIgnored, CSphString & sError );

	/// set n-gram characters (for CJK n-gram indexing)
	virtual bool					SetNgramChars ( const char *, CSphString & ) { return true; }

	/// set n-gram length (for CJK n-gram indexing)
	virtual void					SetNgramLen ( int ) {}

	/// load synonyms list
	virtual bool					LoadSynonyms ( const char * sFilename, CSphString & sError ) = 0;

	/// set phrase boundary chars
	virtual bool					SetBoundary ( const char * sConfig, CSphString & sError );

	/// set blended characters
	virtual bool					SetBlendChars ( const char * sConfig, CSphString & sError );

	/// set blended tokens processing mode
	virtual bool					SetBlendMode ( const char * sMode, CSphString & sError );

	/// setup tokenizer using given settings
	virtual void					Setup ( const CSphTokenizerSettings & tSettings );

	/// create a tokenizer using the given settings
	static ISphTokenizer *			Create ( const CSphTokenizerSettings & tSettings, CSphString & sError );

	/// create a token filter
	static ISphTokenizer *			CreateTokenFilter ( ISphTokenizer * pTokenizer, const CSphMultiformContainer * pContainer );

	/// save tokenizer settings to a stream
	virtual const CSphTokenizerSettings &	GetSettings () const { return m_tSettings; }

	/// get synonym file info
	virtual const CSphSavedFile &	GetSynFileInfo () const { return m_tSynFileInfo; }

public:
	/// pass next buffer
	virtual void					SetBuffer ( BYTE * sBuffer, int iLength ) = 0;

	/// get next token
	virtual BYTE *					GetToken () = 0;

	/// calc codepoint length
	virtual int						GetCodepointLength ( int iCode ) const = 0;

	/// handle tokens less than min_word_len if they match filter
	virtual void					EnableQueryParserMode ( bool bEnable )
									{
										m_bQueryMode = bEnable;
										m_bShortTokenFilter = bEnable;
										m_uBlendVariants = BLEND_TRIM_NONE;
									}

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

	/// get amount of overshort keywords skipped before this token
	virtual int						GetOvershortCount () { return m_iOvershortCount; }

	/// get original tokenized multiform (if any); NULL means there was none
	virtual BYTE *					GetTokenizedMultiform () { return NULL; }

	virtual bool					TokenIsBlended () { return m_bBlended; }
	virtual bool					TokenIsBlendedPart () { return m_bBlendedPart; }
	virtual int						SkipBlended () { return 0; }

public:
	/// spawn a clone of my own
	virtual ISphTokenizer *			Clone ( bool bEscaped ) const = 0;

	/// SBCS or UTF-8?
	virtual bool					IsUtf8 () const = 0;

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

	// get settings hash
	uint64_t						GetSettingsFNV () const { return m_tLC.GetFNV(); }

protected:
	virtual bool					RemapCharacters ( const char * sConfig, DWORD uFlags, const char * sSource, bool bCanRemap, CSphString & sError );
	virtual bool					AddSpecialsSPZ ( const char * sSpecials, const char * sDirective, CSphString & sError );

protected:
	static const int				MAX_SYNONYM_LEN		= 1024;	///< max synonyms map-from length, bytes

	static const BYTE				BLEND_TRIM_NONE		= 1;
	static const BYTE				BLEND_TRIM_HEAD		= 2;
	static const BYTE				BLEND_TRIM_TAIL		= 4;
	static const BYTE				BLEND_TRIM_BOTH		= 8;

	CSphLowercaser					m_tLC;						///< my lowercaser
	int								m_iLastTokenLen;			///< last token length, in codepoints
	bool							m_bTokenBoundary;			///< last token boundary flag (true after boundary codepoint followed by separator)
	bool							m_bBoundary;				///< boundary flag (true immediately after boundary codepoint)
	int								m_iBoundaryOffset;			///< boundary character offset (in bytes)
	bool							m_bWasSpecial;				///< special token flag
	bool							m_bEscaped;					///< backslash handling flag
	int								m_iOvershortCount;			///< skipped overshort tokens count

	bool							m_bBlended;					///< whether last token (as in just returned from GetToken()) was blended
	bool							m_bNonBlended;				///< internal, whether there were any normal chars in that blended token
	bool							m_bBlendedPart;				///< whether last token is a normal subtoken of a blended token
	bool							m_bBlendAdd;				///< whether we have more pending blended variants (of current accumulator) to return
	BYTE							m_uBlendVariants;			///< mask of blended variants as requested by blend_mode (see BLEND_TRIM_xxx flags)
	BYTE							m_uBlendVariantsPending;	///< mask of pending blended variants (we clear bits as we return variants)
	bool							m_bBlendSkipPure;			///< skip purely blended tokens

	bool							m_bShortTokenFilter;		///< short token filter flag
	bool							m_bQueryMode;				///< is this indexing time or searching time?
	bool							m_bDetectSentences;			///< should we detect sentence boundaries?

	CSphTokenizerSettings			m_tSettings;				///< tokenizer settings
	CSphSavedFile					m_tSynFileInfo;				///< synonyms file info

public:
	bool							m_bPhrase;
};

/// parse charset table
bool					sphParseCharset ( const char * sCharset, CSphVector<CSphRemapRange> & dRemaps );

/// create SBCS tokenizer
ISphTokenizer *			sphCreateSBCSTokenizer ();

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
	CSphString		m_sStopwords;
	CSphString		m_sWordforms;
	int				m_iMinStemmingLen;
	bool			m_bWordDict;
	bool			m_bCrc32;

	CSphDictSettings ()
		: m_iMinStemmingLen ( 1 )
		, m_bWordDict ( false )
		, m_bCrc32 ( !USE_64BIT )
	{}
};


/// abstract word dictionary interface
struct CSphWordHit;
struct CSphDict
{
	/// virtualizing dtor
	virtual				~CSphDict () {}

	/// get word ID by word, "text" version
	/// may apply stemming and modify word inplace
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
	virtual void		ApplyStemmers ( BYTE * ) {}

	/// load stopwords from given files
	virtual void		LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer ) = 0;

	/// load wordforms from a given file
	virtual bool		LoadWordforms ( const char * sFile, ISphTokenizer * pTokenizer, const char * sIndex ) = 0;

	/// set morphology
	virtual bool		SetMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sError ) = 0;

	virtual bool		HasMorphology () const { return false; }

	/// setup dictionary using settings
	virtual void		Setup ( const CSphDictSettings & tSettings ) = 0;

	/// get dictionary settings
	virtual const CSphDictSettings & GetSettings () const = 0;

	/// stopwords file infos
	virtual const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () = 0;

	/// wordforms file infos
	virtual const CSphSavedFile & GetWordformsFileInfo () = 0;

	/// get multiwordforms
	virtual const CSphMultiformContainer * GetMultiWordforms () const = 0;

	/// check what given word is stopword
	virtual bool IsStopWord ( const BYTE * pWord ) const = 0;

public:
	/// enable actually collecting keywords (needed for stopwords/wordforms loading)
	virtual void			HitblockBegin () {}

	/// callback to let dictionary do hit block post-processing
	virtual void			HitblockPatch ( CSphWordHit *, int ) {}

	/// resolve temporary hit block wide wordid (!) back to keyword
	virtual const char *	HitblockGetKeyword ( SphWordID_t ) { return NULL; }

	/// check current memory usage
	virtual int				HitblockGetMemUse () { return 0; }

	/// hit block dismissed
	virtual void			HitblockReset () {}

public:
	/// begin creating dictionary file, setup any needed internal structures
	virtual void			DictBegin ( int iTmpDictFD, int iDictFD, int iDictLimit );

	/// add next keyword entry to final dict
	virtual void			DictEntry ( SphWordID_t uWordID, BYTE * sKeyword, int iDocs, int iHits, SphOffset_t iDoclistOffset, SphOffset_t iDoclistLength );

	/// flush last entry
	virtual void			DictEndEntries ( SphOffset_t iDoclistOffset );

	/// end indexing, store dictionary and checkpoints
	virtual bool			DictEnd ( SphOffset_t * pCheckpointsPos, int * pCheckpointsCount, int iMemLimit, CSphString & sError );

	/// check whether there were any errors during indexing
	virtual bool			DictIsError () const;

	/// make clone
	virtual CSphDict *		Clone () const { return NULL; }

	virtual bool			HasState () const { return false; }
};


/// CRC32/FNV64 dictionary factory
CSphDict * sphCreateDictionaryCRC ( const CSphDictSettings & tSettings, ISphTokenizer * pTokenizer, CSphString & sError, const char * sIndex );

/// keyword-storing dictionary factory
CSphDict * sphCreateDictionaryKeywords ( const CSphDictSettings & tSettings, ISphTokenizer * pTokenizer, CSphString & sError, const char * sIndex );

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
/// (because we now allow multiple actual formats within a single storage type!)
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
		POS_MASK		= (1UL << POS_BITS) - 1,
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

	static inline DWORD GetLCS ( Hitpos_t uHitpos )
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

/// hit info
struct CSphWordHit
{
	SphDocID_t		m_iDocID;		///< document ID
	SphWordID_t		m_iWordID;		///< word ID in current dictionary
	Hitpos_t		m_iWordPos;		///< word position in current document
};


/// attribute locator within the row
struct CSphAttrLocator
{
	// OPTIMIZE? try packing these
	int				m_iBitOffset;
	int				m_iBitCount;
	bool			m_bDynamic;

	CSphAttrLocator ()
		: m_iBitOffset ( -1 )
		, m_iBitCount ( -1 )
		, m_bDynamic ( false )
	{}

	inline bool IsBitfield () const
	{
		return ( m_iBitCount<ROWITEM_BITS || ( m_iBitOffset%ROWITEM_BITS )!=0 );
	}

	int CalcRowitem () const
	{
		return IsBitfield() ? -1 : ( m_iBitOffset / ROWITEM_BITS );
	}

	bool IsID () const
	{
		return m_iBitOffset==-8*(int)sizeof(SphDocID_t) && m_iBitCount==8*sizeof(SphDocID_t);
	}

#ifndef NDEBUG
	/// get last item touched by this attr (for debugging checks only)
	int GetMaxRowitem () const
	{
		return ( m_iBitOffset + m_iBitCount - 1 ) / ROWITEM_BITS;
	}
#endif
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


/// pack length into row storage (22 bits max)
/// returns number of bytes used
inline int sphPackStrlen ( BYTE * pRow, int iLen )
{
	assert ( iLen>=0 && iLen<0x400000 );
	if ( iLen<0x80 )
	{
		pRow[0] = BYTE(iLen);
		return 1;
	} else if ( iLen<0x4000 )
	{
		pRow[0] = BYTE ( ( iLen>>8 ) | 0x80 );
		pRow[1] = BYTE ( iLen );
		return 2;
	} else
	{
		pRow[0] = BYTE ( ( iLen>>16 ) | 0xc0 );
		pRow[1] = BYTE ( iLen>>8 );
		pRow[2] = BYTE ( iLen );
		return 3;
	}
}


/// unpack string attr from row storage (22 bits length max)
/// returns unpacked length; stores pointer to string data if required
inline int sphUnpackStr ( const BYTE * pRow, const BYTE ** ppStr )
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


/// search query match (document info plus weight/tag)
class CSphMatch
{
public:
	SphDocID_t				m_iDocID;		///< document ID
	const CSphRowitem *		m_pStatic;		///< static part (stored in and owned by the index)
	CSphRowitem *			m_pDynamic;		///< dynamic part (computed per query; owned by the match)
	int						m_iWeight;		///< my computed weight
	int						m_iTag;			///< my index tag

public:
	/// ctor. clears everything
	CSphMatch ()
		: m_iDocID ( 0 )
		, m_pStatic ( NULL )
		, m_pDynamic ( NULL )
		, m_iWeight ( 0 )
		, m_iTag ( 0 )
	{
	}

	/// copy ctor. just in case
	CSphMatch ( const CSphMatch & rhs )
		: m_pStatic ( 0 )
		, m_pDynamic ( NULL )
	{
		*this = rhs;
	}

	/// dtor. frees everything
	~CSphMatch ()
	{
#ifndef NDEBUG
		if ( m_pDynamic )
			m_pDynamic--;
#endif
		SafeDeleteArray ( m_pDynamic );
	}

	/// reset
	void Reset ( int iDynamic )
	{
		// check that we're either initializing a new one, or NOT changing the current size
		assert ( iDynamic>=0 );
		assert ( !m_pDynamic || iDynamic==(int)m_pDynamic[-1] );

		m_iDocID = 0;
		if ( !m_pDynamic && iDynamic )
		{
#ifndef NDEBUG
			m_pDynamic = new CSphRowitem [ iDynamic+1 ];
			*m_pDynamic++ = iDynamic;
#else
			m_pDynamic = new CSphRowitem [ iDynamic ];
#endif
		}
	}

public:
	/// assignment
	void Clone ( const CSphMatch & rhs, int iDynamic )
	{
		// check that we're either initializing a new one, or NOT changing the current size
		assert ( iDynamic>=0 );
		assert ( !m_pDynamic || iDynamic==(int)m_pDynamic[-1] );

		m_iDocID = rhs.m_iDocID;
		m_iWeight = rhs.m_iWeight;
		m_pStatic = rhs.m_pStatic;
		m_iTag = rhs.m_iTag;

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

			assert ( rhs.m_pDynamic );
			assert ( m_pDynamic[-1]==rhs.m_pDynamic[-1] ); // ensure we're not changing X to Y
			memcpy ( m_pDynamic, rhs.m_pDynamic, iDynamic*sizeof(CSphRowitem) );
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
		if ( tLoc.IsID() )
			return m_iDocID;
		assert ( false && "Unknown negative-bitoffset locator" );
		return 0;
	}

	/// float getter
	float GetAttrFloat ( const CSphAttrLocator & tLoc ) const
	{
		return sphDW2F ( (DWORD)sphGetRowAttr ( tLoc.m_bDynamic ? m_pDynamic : m_pStatic, tLoc ) );
	};

	/// integer setter
	void SetAttr ( const CSphAttrLocator & tLoc, SphAttr_t uValue )
	{
		if ( tLoc.IsID() )
		{
			// m_iDocID = uValue;
			return;
		}
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

	/// MVA getter
	const DWORD * GetAttrMVA ( const CSphAttrLocator & tLoc, const DWORD * pPool ) const;

private:
	/// "manually" prevent copying
	const CSphMatch & operator = ( const CSphMatch & )
	{
		assert ( 0 && "internal error (CSphMatch::operator= called)" );
		return *this;
	}
};


/// specialized swapper
inline void Swap ( CSphMatch & a, CSphMatch & b )
{
	Swap ( a.m_iDocID, b.m_iDocID );
	Swap ( a.m_pStatic, b.m_pStatic );
	Swap ( a.m_pDynamic, b.m_pDynamic );
	Swap ( a.m_iWeight, b.m_iWeight );
	Swap ( a.m_iTag, b.m_iTag );
}


/// source statistics
struct CSphSourceStats
{
	int				m_iTotalDocuments;	///< how much documents
	int64_t			m_iTotalBytes;		///< how much bytes

	/// ctor
	CSphSourceStats ()
	{
		Reset ();
	}

	/// reset
	void Reset ()
	{
		m_iTotalDocuments = 0;
		m_iTotalBytes = 0;
	}
};


/// known multi-valued attr sources
enum ESphAttrSrc
{
	SPH_ATTRSRC_NONE		= 0,	///< not multi-valued
	SPH_ATTRSRC_FIELD		= 1,	///< get attr values from text field
	SPH_ATTRSRC_QUERY		= 2,	///< get attr values from SQL query
	SPH_ATTRSRC_RANGEDQUERY	= 3		///< get attr values from ranged SQL query
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
	SPH_AGGR_SUM
};


/// column evaluation stage
enum ESphEvalStage
{
	SPH_EVAL_STATIC = 0,		///< static data, no real evaluation needed
	SPH_EVAL_OVERRIDE,			///< static but possibly overridden
	SPH_EVAL_PREFILTER,			///< expression needed for full-text candidate matches filtering
	SPH_EVAL_PRESORT,			///< expression needed for final matches sorting
	SPH_EVAL_SORTER,			///< expression evaluated by sorter object
	SPH_EVAL_FINAL				///< expression not (!) used in filters/sorting; can be postponed until final result set cooking
};


/// source column info
struct CSphColumnInfo
{
	CSphString		m_sName;		///< column name
	ESphAttr		m_eAttrType;	///< attribute type
	ESphWordpart	m_eWordpart;	///< wordpart processing type
	bool			m_bIndexed;		///< whether to index this column as fulltext field too

	int				m_iIndex;		///< index into source result set (-1 for joined fields)
	CSphAttrLocator	m_tLocator;		///< attribute locator in the row

	ESphAttrSrc		m_eSrc;			///< attr source (for multi-valued attrs only)
	CSphString		m_sQuery;		///< query to retrieve values (for multi-valued attrs only)
	CSphString		m_sQueryRange;	///< query to retrieve range (for multi-valued attrs only)

	CSphRefcountedPtr<ISphExpr>		m_pExpr;		///< evaluator for expression items
	ESphAggrFunc					m_eAggrFunc;	///< aggregate function on top of expression (for GROUP BY)
	ESphEvalStage					m_eStage;		///< column evaluation stage (who and how computes this column)
	bool							m_bPayload;
	bool							m_bFilename;	///< column is a file name

	/// handy ctor
	CSphColumnInfo ( const char * sName=NULL, ESphAttr eType=SPH_ATTR_NONE )
		: m_sName ( sName )
		, m_eAttrType ( eType )
		, m_eWordpart ( SPH_WORDPART_WHOLE )
		, m_bIndexed ( false )
		, m_iIndex ( -1 )
		, m_eSrc ( SPH_ATTRSRC_NONE )
		, m_pExpr ( NULL )
		, m_eAggrFunc ( SPH_AGGR_NONE )
		, m_eStage ( SPH_EVAL_STATIC )
		, m_bPayload ( false )
		, m_bFilename ( false )
	{
		m_sName.ToLower ();
	}

	/// equality comparison checks name, type, and locator
	bool operator == ( const CSphColumnInfo & rhs ) const
	{
		return m_sName==rhs.m_sName
			&& m_eAttrType==rhs.m_eAttrType
			&& m_tLocator.m_iBitCount==rhs.m_tLocator.m_iBitCount
			&& m_tLocator.m_iBitOffset==rhs.m_tLocator.m_iBitOffset
			&& m_tLocator.m_bDynamic==rhs.m_tLocator.m_bDynamic;
	}
};


/// source schema
struct CSphSchema
{
	CSphString						m_sName;		///< my human-readable name
	CSphVector<CSphColumnInfo>		m_dFields;		///< my fulltext-searchable fields
	int								m_iBaseFields;	///< how much fields are base, how much are additional (only affects indexer)

public:

	/// ctor
	explicit				CSphSchema ( const char * sName="(nameless)" ) : m_sName ( sName ), m_iBaseFields ( 0 ), m_iStaticSize ( 0 ) {}

	/// get field index by name
	/// returns -1 if not found
	int						GetFieldIndex ( const char * sName ) const;

	/// get attribute index by name
	/// returns -1 if not found
	int						GetAttrIndex ( const char * sName ) const;

	/// checks if two schemas fully match (ie. fields names, attr names, types and locators are the same)
	/// describe mismatch (if any) to sError
	bool					CompareTo ( const CSphSchema & rhs, CSphString & sError ) const;

	/// reset fields and attrs
	void					Reset ();

	/// reset attrs only
	void					ResetAttrs ();

	/// get row size (static+dynamic combined)
	int						GetRowSize () const				{ return m_iStaticSize + m_dDynamicUsed.GetLength(); }

	/// get static row part size
	int						GetStaticSize () const			{ return m_iStaticSize; }

	/// get dynamic row part size
	int						GetDynamicSize () const			{ return m_dDynamicUsed.GetLength(); }

	/// get attrs count
	int						GetAttrsCount () const			{ return m_dAttrs.GetLength(); }

	/// get attr by index
	const CSphColumnInfo &	GetAttr ( int iIndex ) const	{ return m_dAttrs[iIndex]; }

	/// get attr by name
	const CSphColumnInfo *	GetAttr ( const char * sName ) const;

	/// add attr
	void					AddAttr ( const CSphColumnInfo & tAttr, bool bDynamic );

	/// remove static attr (but do NOT recompute locations; for overrides)
	/// WARNING, THIS IS A HACK THAT WILL LIKELY BREAK THE SCHEMA, DO NOT USE THIS UNLESS ABSOLUTELY SURE!
	void					RemoveAttr ( int iIndex );

protected:
	CSphVector<CSphColumnInfo>		m_dAttrs;			///< all my attributes
	CSphVector<int>					m_dStaticUsed;		///< static row part map (amount of used bits in each rowitem)
	CSphVector<int>					m_dDynamicUsed;		///< dynamic row part map
	int								m_iStaticSize;		///< static row size (can be different from m_dStaticUsed.GetLength() because of gaps)
};


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
		CSphVector<CSphString>	m_dAttrs;		///< attr names to index

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
	int		m_iMinPrefixLen;	///< min indexable prefix (0 means don't index prefixes)
	int		m_iMinInfixLen;		///< min indexable infix length (0 means don't index infixes)
	int		m_iBoundaryStep;	///< additional boundary word position increment
	bool	m_bIndexExactWords;	///< exact (non-stemmed) word indexing flag
	int		m_iOvershortStep;	///< position step on overshort token (default is 1)
	int		m_iStopwordStep;	///< position step on stopword token (default is 1)
	bool	m_bIndexSP;			///< whether to index sentence and paragraph delimiters

	CSphVector<CSphString>	m_dPrefixFields;	///< list of prefix fields
	CSphVector<CSphString>	m_dInfixFields;		///< list of infix fields

	explicit				CSphSourceSettings ();
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

	void AddHit ( SphDocID_t uDocid, SphWordID_t uWordid, Hitpos_t uPos )
	{
		if ( uWordid )
		{
			CSphWordHit & tHit = m_dData.Add();
			tHit.m_iDocID = uDocid;
			tHit.m_iWordID = uWordid;
			tHit.m_iWordPos = uPos;
		}
	}

public:
	CSphVector<CSphWordHit> m_dData;
};


/// generic data source
class CSphSource : public CSphSourceSettings
{
public:
	CSphMatch							m_tDocInfo;		///< current document info
	CSphVector<CSphString>				m_dStrAttrs;	///< current document string attrs

public:
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

public:
	/// connect to the source (eg. to the database)
	/// connection settings are specific for each source type and as such
	/// are implemented in specific descendants
	virtual bool						Connect ( CSphString & sError ) = 0;

	/// disconnect from the source
	virtual void						Disconnect () = 0;

	/// check if there are any attributes configured
	/// note that there might be NO actual attributes in the case if configured
	/// ones do not match those actually returned by the source
	virtual bool						HasAttrsConfigured () = 0;

	/// check if there are any joined fields
	virtual bool						HasJoinedFields () { return false; }

	/// begin indexing this source
	/// to be implemented by descendants
	virtual bool						IterateStart ( CSphString & sError ) = 0;

	/// get next document
	/// to be implemented by descendants
	/// returns false on error
	/// returns true and fills m_tDocInfo on success
	/// returns true and sets m_tDocInfo.m_iDocID to 0 on eof
	virtual bool						IterateDocument ( CSphString & sError ) = 0;

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

	/// get next multi-valued (id,attr-value) tuple to m_tDocInfo
	virtual bool						IterateMultivaluedNext () = 0;

	/// begin iterating values of multi-valued attribute iAttr stored in a field
	/// will fail if iAttr is out of range, or is not multi-valued
	virtual bool						IterateFieldMVAStart ( int iAttr, CSphString & sError ) = 0;

	/// get next multi-valued (id,attr-value) tuple to m_tDocInfo
	virtual bool						IterateFieldMVANext () = 0;

	/// begin iterating kill list
	virtual bool						IterateKillListStart ( CSphString & sError ) = 0;

	/// get next kill list doc id
	virtual bool						IterateKillListNext ( SphDocID_t & tDocId ) = 0;

	/// post-index callback
	/// gets called when the indexing is succesfully (!) over
	virtual void						PostIndex () {}

protected:
	ISphTokenizer *						m_pTokenizer;	///< my tokenizer
	CSphDict *							m_pDict;		///< my dict

	CSphSourceStats						m_tStats;		///< my stats
	CSphSchema							m_tSchema;		///< my schema

	bool								m_bStripHTML;	///< whether to strip HTML
	CSphHTMLStripper *					m_pStripper;	///< my HTML stripper

	int			m_iNullIds;
	int			m_iMaxIds;

	SphDocID_t	VerifyID ( SphDocID_t uID );
};


/// how to handle IO errors in file fields
enum ESphOnFileFieldError
{
	FFE_IGNORE_FIELD,
	FFE_SKIP_DOCUMENT,
	FFE_FAIL_INDEX
};


/// generic document source
/// provides multi-field support and generic tokenizer
class CSphSource_Document : public CSphSource
{
public:
	/// ctor
	explicit				CSphSource_Document ( const char * sName );

	/// dtor
	virtual					~CSphSource_Document () { SafeDeleteArray ( m_pReadFileBuffer ); }

	/// my generic tokenizer
	virtual bool			IterateDocument ( CSphString & sError );
	virtual ISphHits *		IterateHits ( CSphString & sError );
	void					BuildHits ( CSphString & sError, bool bSkipEndMarker );

	/// field data getter
	/// to be implemented by descendants
	virtual BYTE **			NextDocument ( CSphString & sError ) = 0;

	virtual void			SetDumpRows ( FILE * fpDumpRows ) { m_fpDumpRows = fpDumpRows; }

protected:
	void					ParseFieldMVA ( CSphVector < CSphVector < DWORD > > & dFieldMVAs, int iFieldMVA, const char * szValue );
	bool					CheckFileField ( const BYTE * sField );
	int						LoadFileField ( BYTE ** ppField, CSphString & sError );
	void					BuildSubstringHits ( SphDocID_t uDocid, bool bPayload, ESphWordpart eWordpart, bool bSkipEndMarker );
	void					BuildRegularHits ( SphDocID_t uDocid, bool bPayload, bool bSkipEndMarker );

protected:
	ISphHits				m_tHits;				///< my hitvector

protected:
	char *					m_pReadFileBuffer;
	int						m_iReadFileBufferSize;	///< size of read buffer for the 'sql_file_field' fields
	int						m_iMaxFileBufferSize;	///< max size of read buffer for the 'sql_file_field' fields
	ESphOnFileFieldError	m_eOnFileFieldError;
	FILE *					m_fpDumpRows;

protected:
	struct CSphBuildHitsState_t
	{
		bool m_bProcessingHits;
		bool m_bDocumentDone;

		BYTE ** m_dFields;

		int m_iStartPos;
		Hitpos_t m_iHitPos;
		int m_iField;
		int m_iStartField;
		int m_iEndField;

		int m_iBuildLastStep;

		CSphBuildHitsState_t ();
	};

	CSphBuildHitsState_t	m_tState;
	int						m_iMaxHits;
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
};


/// generic SQL source params
struct CSphSourceParams_SQL
{
	// query params
	CSphString						m_sQuery;
	CSphString						m_sQueryRange;
	CSphString						m_sQueryKilllist;
	int								m_iRangeStep;
	int								m_iRefRangeStep;
	bool							m_bPrintQueries;

	CSphVector<CSphString>			m_dQueryPre;
	CSphVector<CSphString>			m_dQueryPost;
	CSphVector<CSphString>			m_dQueryPostIndex;
	CSphVector<CSphColumnInfo>		m_dAttrs;
	CSphVector<CSphString>			m_dFileFields;

	int								m_iRangedThrottle;
	int								m_iMaxFileBufferSize;
	ESphOnFileFieldError			m_eOnFileFieldError;

	CSphVector<CSphUnpackInfo>		m_dUnpack;
	DWORD							m_uUnpackMemoryLimit;

	CSphVector<CSphJoinedField>		m_dJoinedFields;

	// connection params
	CSphString						m_sHost;
	CSphString						m_sUser;
	CSphString						m_sPass;
	CSphString						m_sDB;
	int								m_iPort;

	CSphSourceParams_SQL ();
};


/// generic SQL source
/// multi-field plain-text documents fetched from given query
struct CSphSource_SQL : CSphSource_Document
{
	explicit			CSphSource_SQL ( const char * sName );
	virtual				~CSphSource_SQL () {}

	bool				Setup ( const CSphSourceParams_SQL & pParams );
	virtual bool		Connect ( CSphString & sError );
	virtual void		Disconnect ();

	virtual bool		IterateStart ( CSphString & sError );
	virtual BYTE **		NextDocument ( CSphString & sError );
	virtual void		PostIndex ();

	virtual bool		HasAttrsConfigured () { return m_tParams.m_dAttrs.GetLength()!=0; }
	virtual bool		HasJoinedFields () { return m_tSchema.m_iBaseFields!=m_tSchema.m_dFields.GetLength(); }

	virtual ISphHits *	IterateJoinedHits ( CSphString & sError );

	virtual bool		IterateMultivaluedStart ( int iAttr, CSphString & sError );
	virtual bool		IterateMultivaluedNext ();

	virtual bool		IterateFieldMVAStart ( int iAttr, CSphString & sError );
	virtual bool		IterateFieldMVANext ();

	virtual bool		IterateKillListStart ( CSphString & sError );
	virtual bool		IterateKillListNext ( SphDocID_t & tDocId );

private:
	bool				m_bSqlConnected;	///< am i connected?

protected:
	CSphString			m_sSqlDSN;

	BYTE *				m_dFields [ SPH_MAX_FIELDS ];
	ESphUnpackFormat	m_dUnpack [ SPH_MAX_FIELDS ];

	SphDocID_t			m_uMinID;			///< grand min ID
	SphDocID_t			m_uMaxID;			///< grand max ID
	SphDocID_t			m_uCurrentID;		///< current min ID
	SphDocID_t			m_uMaxFetchedID;	///< max actually fetched ID
	int					m_iMultiAttr;		///< multi-valued attr being currently fetched
	int					m_iSqlFields;		///< field count (for row dumper)

	int					m_iFieldMVA;
	int					m_iFieldMVAIterator;
	CSphVector < CSphVector <DWORD> > m_dFieldMVAs;
	CSphVector < int >	m_dAttrToFieldMVA;

	CSphSourceParams_SQL		m_tParams;

	bool				m_bCanUnpack;
	bool				m_bUnpackFailed;
	bool				m_bUnpackOverflow;
	CSphVector<char>	m_dUnpackBuffers [ SPH_MAX_FIELDS ];

	int					m_iJoinedHitField;	///< currently pulling joined hits from this field (index into schema; -1 if not pulling)
	SphDocID_t			m_iJoinedHitID;		///< last document id
	int					m_iJoinedHitPos;	///< last hit position

	static const int			MACRO_COUNT = 2;
	static const char * const	MACRO_VALUES [ MACRO_COUNT ];

protected:
	bool					SetupRanges ( const char * sRangeQuery, const char * sQuery, const char * sPrefix, CSphString & sError );
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

	const char *	SqlUnpackColumn ( int iIndex, ESphUnpackFormat eFormat );
	void			ReportUnpackError ( int iIndex, int iError );
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
	MYSQL_RES *				m_pMysqlResult;
	MYSQL_FIELD *			m_pMysqlFields;
	MYSQL_ROW				m_tMysqlRow;
	MYSQL					m_tMysqlDriver;
	unsigned long *			m_pMysqlLengths;

	CSphString				m_sMysqlUsock;
	int						m_iMysqlConnectFlags;
	CSphString				m_sSslKey;
	CSphString				m_sSslCert;
	CSphString				m_sSslCA;

protected:
	virtual void			SqlDismissResult ();
	virtual bool			SqlQuery ( const char * sQuery );
	virtual bool			SqlIsError ();
	virtual const char *	SqlError ();
	virtual bool			SqlConnect ();
	virtual void			SqlDisconnect ();
	virtual int				SqlNumFields();
	virtual bool			SqlFetchRow();
	virtual DWORD			SqlColumnLength ( int iIndex );
	virtual const char *	SqlColumn ( int iIndex );
	virtual const char *	SqlFieldName ( int iIndex );
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
	virtual bool			IterateStart ( CSphString & sError );

protected:
	PGresult * 				m_pPgResult;	///< postgresql execution restult context
	PGconn *				m_tPgDriver;	///< postgresql connection context

	int						m_iPgRows;		///< how much rows last step returned
	int						m_iPgRow;		///< current row (0 based, as in PQgetvalue)

	CSphString				m_sPgClientEncoding;
	CSphVector<bool>		m_dIsColumnBool;

protected:
	virtual void			SqlDismissResult ();
	virtual bool			SqlQuery ( const char * sQuery );
	virtual bool			SqlIsError ();
	virtual const char *	SqlError ();
	virtual bool			SqlConnect ();
	virtual void			SqlDisconnect ();
	virtual int				SqlNumFields();
	virtual bool			SqlFetchRow();
	virtual DWORD	SqlColumnLength ( int iIndex );
	virtual const char *	SqlColumn ( int iIndex );
	virtual const char *	SqlFieldName ( int iIndex );
};
#endif // USE_PGSQL

#if USE_ODBC
struct CSphSourceParams_ODBC: CSphSourceParams_SQL
{
	CSphString	m_sOdbcDSN;			///< ODBC DSN
	CSphString	m_sColBuffers;		///< column buffer sizes (eg "col1=2M, col2=4M")
	bool		m_bWinAuth;			///< auth type (MS SQL only)
	bool		m_bUnicode;			///< whether to ask for Unicode or SBCS (C char) data (MS SQL only)

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
		int					m_iBufferSize;	///< size of m_dContents and m_dRaw buffers, in bytes
		bool				m_bUnicode;		///< whether this column needs UCS-2 to UTF-8 translation
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


/// MS SQL source implemenation
struct CSphSource_MSSQL : public CSphSource_ODBC
{
	explicit				CSphSource_MSSQL ( const char * sName ) : CSphSource_ODBC ( sName ) {}
	virtual void			OdbcPostConnect ();
};
#endif // USE_ODBC


/// XML pipe source implementation
class CSphSource_XMLPipe : public CSphSource
{
public:
					CSphSource_XMLPipe ( BYTE * dInitialBuf, int iBufLen, const char * sName );	///< ctor
					~CSphSource_XMLPipe ();						///< dtor

	bool			Setup ( FILE * pPipe, const char * sCommand );			///< memorize the command
	virtual bool	Connect ( CSphString & sError );			///< run the command and open the pipe
	virtual void	Disconnect ();								///< close the pipe

	virtual bool		IterateStart ( CSphString & ) { return true; }	///< Connect() starts getting documents automatically, so this one is empty
	virtual bool		IterateDocument ( CSphString & sError );		///< parse incoming chunk and emit document
	virtual ISphHits *	IterateHits ( CSphString & sError );									///< parse incoming chunk and emit some hits

	virtual bool	HasAttrsConfigured ()							{ return true; }	///< xmlpipe always has some attrs for now
	virtual bool	IterateMultivaluedStart ( int, CSphString & )	{ return false; }	///< xmlpipe does not support multi-valued attrs for now
	virtual bool	IterateMultivaluedNext ()						{ return false; }	///< xmlpipe does not support multi-valued attrs for now
	virtual bool	IterateFieldMVAStart ( int, CSphString & )		{ return false; }
	virtual bool	IterateFieldMVANext ()							{ return false; }
	virtual bool	IterateKillListStart ( CSphString & )			{ return false; }
	virtual bool	IterateKillListNext ( SphDocID_t & )			{ return false; }


private:
	enum Tag_e
	{
		TAG_DOCUMENT = 0,
		TAG_ID,
		TAG_GROUP,
		TAG_TITLE,
		TAG_BODY
	};

private:
	CSphString		m_sCommand;			///< my command

	Tag_e			m_eTag;				///< what's our current tag
	const char *	m_pTag;				///< tag name
	int				m_iTagLength;		///< tag name length
	int				m_iBufferSize;		///< buffer size
	bool			m_bEOF;				///< EOF encountered
	bool			m_bWarned;			///< warned of buffer size already
	int				m_iInitialBufLen;	///< initial buffer len

	FILE *			m_pPipe;			///< incoming stream
	BYTE *			m_sBuffer;			///< buffer
	BYTE *			m_pBuffer;			///< current buffer pos
	BYTE *			m_pBufferEnd;		///< buffered end pos

	int				m_iWordPos;			///< current word position

	ISphHits		m_tHits;			///< my hitvector
	bool			m_bHitsReady;

private:
	/// set current tag
	void			SetTag ( const char * sTag );

	/// read in some more data
	/// moves everything from current ptr (m_pBuffer) to the beginng
	/// reads in as much data as possible to the end
	/// returns false on EOF
	bool			UpdateBuffer ();

	/// skips whitespace
	/// does buffer updates
	/// returns false on EOF
	bool			SkipWhitespace ();

	/// check if what's at current pos is either opening/closing current tag (m_pTag)
	/// return false on failure
	bool			CheckTag ( bool bOpen, CSphString & sError );

	/// skips whitespace and opening/closing current tag (m_pTag)
	/// returns false on failure
	bool			SkipTag ( bool bOpen, CSphString & sError );

	/// scan for tag with integer value
	bool			ScanInt ( const char * sTag, DWORD * pRes, CSphString & sError );

	/// scan for tag with integer value
	bool			ScanInt ( const char * sTag, uint64_t * pRes, CSphString & sError );

	/// scan for tag with integer value
	bool			ScanInt ( const char * sTag, int64_t * pRes, CSphString & sError ) { return ScanInt ( sTag, (uint64_t*)pRes, sError ); }

	/// scan for tag with string value
	bool			ScanStr ( const char * sTag, char * pRes, int iMaxLength, CSphString & sError );

	/// check for hits overun hits buffer
	void			CheckHitsCount ( const char * sField );
};


#if USE_LIBEXPAT || USE_LIBXML

class CSphConfigSection;
CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, FILE * pPipe, BYTE * dInitialBuf, int iBufLen, const char * szSourceName, int iMaxFieldLen );

#endif

FILE * sphDetectXMLPipe ( const char * szCommand, BYTE * dBuf, int & iBufSize, int iMaxBufSize, bool & bUsePipe2 );


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
	SPH_GROUPBY_ATTRPAIR= 5		///< group by sequential attrs pair (rendered redundant by 64bit attrs support; removed)
};


/// search query filter types
enum ESphFilter
{
	SPH_FILTER_VALUES		= 0,	///< filter by integer values set
	SPH_FILTER_RANGE		= 1,	///< filter by integer range
	SPH_FILTER_FLOATRANGE	= 2		///< filter by float range
};


/// search query filter
class CSphFilterSettings
{
public:
	CSphString			m_sAttrName;	///< filtered attribute name
	bool				m_bExclude;		///< whether this is "include" or "exclude" filter (default is "include")

	ESphFilter			m_eType;		///< filter type
	union
	{
		SphAttr_t		m_uMinValue;	///< range min
		float			m_fMinValue;	///< range min
	};
	union
	{
		SphAttr_t		m_uMaxValue;	///< range max
		float			m_fMaxValue;	///< range max
	};
	CSphVector<SphAttr_t>	m_dValues;		///< integer values set

public:
						CSphFilterSettings ();

	void				SetExternalValues ( const SphAttr_t * pValues, int nValues );

	SphAttr_t			GetValue ( int iIdx ) const	{ assert ( iIdx<GetNumValues() ); return m_pValues ? m_pValues[iIdx] : m_dValues[iIdx]; }
	const SphAttr_t *	GetValueArray () const		{ return m_pValues ? m_pValues : &(m_dValues[0]); }
	int					GetNumValues () const		{ return m_pValues ? m_nValues : m_dValues.GetLength (); }

	bool				operator == ( const CSphFilterSettings & rhs ) const;
	bool				operator != ( const CSphFilterSettings & rhs ) const { return !( (*this)==rhs ); }



protected:
	const SphAttr_t *	m_pValues;		///< external value array
	int					m_nValues;		///< external array size

						CSphFilterSettings ( const CSphFilterSettings & rhs );
};


// keyword info
struct CSphKeywordInfo
{
	CSphString		m_sTokenized;
	CSphString		m_sNormalized;
	int				m_iDocs;
	int				m_iHits;
};


/// name+int pair
struct CSphNamedInt
{
	CSphString	m_sName;
	int			m_iValue;

	CSphNamedInt () : m_iValue ( 0 ) {}
};


/// per-attribute value overrides
class CSphAttrOverride
{
public:
	/// docid+attrvalue pair
	struct IdValuePair_t
	{
		SphDocID_t				m_uDocID;		///< document ID
		union
		{
			SphAttr_t			m_uValue;		///< attribute value
			float				m_fValue;		///< attribute value
		};

		inline bool operator < ( const IdValuePair_t & rhs ) const
		{
			return m_uDocID<rhs.m_uDocID;
		}
	};

public:
	CSphString					m_sAttr;		///< attribute name
	ESphAttr					m_eAttrType;	///< attribute type
	CSphVector<IdValuePair_t>	m_dValues;		///< id-value overrides
};


/// query selection item
struct CSphQueryItem
{
	CSphString		m_sExpr;		///< expression to compute
	CSphString		m_sAlias;		///< alias to return
	ESphAggrFunc	m_eAggrFunc;

	CSphQueryItem() : m_eAggrFunc ( SPH_AGGR_NONE ) {}
};


/// known collations
enum ESphCollation
{
	SPH_COLLATION_LIBC_CI,
	SPH_COLLATION_LIBC_CS,
	SPH_COLLATION_UTF8_GENERAL_CI,
	SPH_COLLATION_BINARY,

	SPH_COLLATION_DEFAULT = SPH_COLLATION_LIBC_CI
};


/// search query
class CSphQuery
{
public:
	CSphString		m_sIndexes;		///< indexes to search
	CSphString		m_sQuery;		///< cooked query string for the engine (possibly transformed during legacy matching modes fixup)
	CSphString		m_sRawQuery;	///< raw query string from the client for searchd log, agents, etc

	int				m_iOffset;		///< offset into result set (as X in MySQL LIMIT X,Y clause)
	int				m_iLimit;		///< limit into result set (as Y in MySQL LIMIT X,Y clause)
	DWORD *			m_pWeights;		///< user-supplied per-field weights. may be NULL. default is NULL. NOT OWNED, WILL NOT BE FREED in dtor.
	int				m_iWeights;		///< number of user-supplied weights. missing fields will be assigned weight 1. default is 0
	ESphMatchMode	m_eMode;		///< match mode. default is "match all"
	ESphRankMode	m_eRanker;		///< ranking mode, default is proximity+BM25
	CSphString		m_sRankerExpr;	///< ranking expression for SPH_RANK_EXPR
	ESphSortOrder	m_eSort;		///< sort mode
	CSphString		m_sSortBy;		///< attribute to sort by
	int				m_iMaxMatches;	///< max matches to retrieve, default is 1000. more matches use more memory and CPU time to hold and sort them

	CSphVector<CSphFilterSettings>	m_dFilters;	///< filters

	CSphString		m_sGroupBy;			///< group-by attribute name
	ESphGroupBy		m_eGroupFunc;		///< function to pre-process group-by attribute value with
	CSphString		m_sGroupSortBy;		///< sorting clause for groups in group-by mode
	CSphString		m_sGroupDistinct;	///< count distinct values for this attribute

	int				m_iCutoff;			///< matches count threshold to stop searching at (default is 0; means to search until all matches are found)

	int				m_iRetryCount;		///< retry count, for distributed queries
	int				m_iRetryDelay;		///< retry delay, for distributed queries

	bool			m_bGeoAnchor;		///< do we have an anchor
	CSphString		m_sGeoLatAttr;		///< latitude attr name
	CSphString		m_sGeoLongAttr;		///< longitude attr name
	float			m_fGeoLatitude;		///< anchor latitude
	float			m_fGeoLongitude;	///< anchor longitude

	CSphVector<CSphNamedInt>	m_dIndexWeights;	///< per-index weights
	CSphVector<CSphNamedInt>	m_dFieldWeights;	///< per-field weights

	DWORD			m_uMaxQueryMsec;	///< max local index search time, in milliseconds (default is 0; means no limit)
	CSphString		m_sComment;			///< comment to pass verbatim in the log file

	CSphVector<CSphAttrOverride>	m_dOverrides;	///< per-query attribute value overrides

	CSphString		m_sSelect;			///< select-list (attributes and/or expressions)
	CSphString		m_sOrderBy;			///< order-by clause

	bool			m_bReverseScan;		///< perform scan in reverse order

	int				m_iSQLSelectStart;	///< SQL parser helper
	int				m_iSQLSelectEnd;	///< SQL parser helper

public:
	int				m_iOldVersion;		///< version, to fixup old queries
	int				m_iOldGroups;		///< 0.9.6 group filter values count
	DWORD *			m_pOldGroups;		///< 0.9.6 group filter values
	DWORD			m_iOldMinTS;		///< 0.9.6 min timestamp
	DWORD			m_iOldMaxTS;		///< 0.9.6 max timestamp
	DWORD			m_iOldMinGID;		///< 0.9.6 min group id
	DWORD			m_iOldMaxGID;		///< 0.9.6 max group id

public:
	CSphVector<CSphQueryItem>	m_dItems;		///< parsed select-list
	ESphCollation				m_eCollation;	///< ORDER BY collation
	bool						m_bAgent;		///< agent mode (may need extra cols on output)

public:
					CSphQuery ();		///< ctor, fills defaults
					~CSphQuery ();		///< dtor, frees owned stuff

	/// return index weight from m_dIndexWeights; or 1 by default
	int				GetIndexWeight ( const char * sName ) const;

	/// parse select list string into items
	bool			ParseSelectList ( CSphString & sError );
};


/// search query meta-info
class CSphQueryResultMeta
{
public:
	int						m_iQueryTime;		///< query time, milliseconds
	int64_t					m_iCpuTime;			///< user time, microseconds
	int						m_iMultiplier;		///< multi-query multiplier, -1 to indicate error

	struct WordStat_t
	{
		int					m_iDocs;			///< document count for this term
		int					m_iHits;			///< hit count for this term
		bool				m_bExpanded;		///< is this term from query itself or was expanded

		WordStat_t()
			: m_iDocs ( 0 )
			, m_iHits ( 0 )
			, m_bExpanded ( false )
		{}
	};
	SmallStringHash_T<WordStat_t>	m_hWordStats; ///< hash of i-th search term (normalized word form)

	int						m_iMatches;			///< total matches returned (upto MAX_MATCHES)
	int						m_iTotalMatches;	///< total matches found (unlimited)

	CSphString				m_sError;			///< error message
	CSphString				m_sWarning;			///< warning message

	CSphQueryResultMeta ();													///< ctor
	virtual					~CSphQueryResultMeta () {}						///< dtor
	void					AddStat ( const CSphString & sWord, int iDocs, int iHits, bool bExpanded );

	CSphQueryResultMeta ( const CSphQueryResultMeta & tMeta );				///< copy ctor
	CSphQueryResultMeta & operator= ( const CSphQueryResultMeta & tMeta );	///< copy
};


/// search query result (meta-info plus actual matches)
class CSphQueryResult : public CSphQueryResultMeta
{
public:
	CSphSwapVector<CSphMatch>	m_dMatches;			///< top matching documents, no more than MAX_MATCHES

	CSphSchema				m_tSchema;			///< result schema
	const DWORD *			m_pMva;				///< pointer to MVA storage
	const BYTE *			m_pStrings;			///< pointer to strings storage

	CSphVector<void *>		m_dStorage2Free;	/// < aggregated external storage from rt indexes

	int						m_iOffset;			///< requested offset into matches array
	int						m_iCount;			///< count which will be actually served (computed from total, offset and limit)

	int						m_iSuccesses;

public:
							CSphQueryResult ();		///< ctor
	virtual					~CSphQueryResult ();	///< dtor, which releases all owned stuff

	void					LeakStorages ( CSphQueryResult & tDst );
};

/////////////////////////////////////////////////////////////////////////////
// ATTRIBUTE UPDATE QUERY
/////////////////////////////////////////////////////////////////////////////

struct CSphAttrUpdate
{
	CSphVector<CSphColumnInfo>		m_dAttrs;		///< update schema (ie. what attrs to update)
	CSphVector<DWORD>				m_dPool;		///< update values pool
	CSphVector<SphDocID_t>			m_dDocids;		///< document IDs vector
	CSphVector<const CSphRowitem*>	m_dRows;		///< document attribute's vector, used instead of m_dDocids.
	CSphVector<int>					m_dRowOffset;	///< document row offsets in the pool (1 per doc, i.e. the length is the same as of m_dDocids)
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
		PHASE_COLLECT_MVA,			///< multi-valued attributes collection phase
		PHASE_SORT_MVA,				///< multi-valued attributes collection phase
		PHASE_MERGE,				///< index merging

		PHASE_PREREAD,				///< searchd startup, prereading data
		PHASE_PRECOMPUTE			///< searchd startup, indexing attributes
	};

	Phase_e			m_ePhase;		///< current indexing phase

	int				m_iDocuments;	///< PHASE_COLLECT: documents collected so far
	int64_t			m_iBytes;		///< PHASE_COLLECT: bytes collected so far;
									///< PHASE_PREREAD: bytes read so far;
	int64_t			m_iBytesTotal;	///< PHASE_PREREAD: total bytes to read;

	int64_t			m_iAttrs;		///< PHASE_COLLECT_MVA, PHASE_SORT_MVA: attrs processed so far
	int64_t			m_iAttrsTotal;	///< PHASE_SORT_MVA: attrs total

	SphOffset_t		m_iHits;		///< PHASE_SORT: hits sorted so far
	SphOffset_t		m_iHitsTotal;	///< PHASE_SORT: hits total

	int				m_iWords;		///< PHASE_MERGE: words merged so far

	int				m_iDone;		///< generic percent, 0..1000 range

	CSphIndexProgress ()
		: m_ePhase ( PHASE_COLLECT )
		, m_iDocuments ( 0 )
		, m_iBytes ( 0 )
		, m_iBytesTotal ( 0 )
		, m_iAttrs ( 0 )
		, m_iAttrsTotal ( 0 )
		, m_iHits ( 0 )
		, m_iHitsTotal ( 0 )
		, m_iWords ( 0 )
	{}

	/// builds a message to print
	/// WARNING, STATIC BUFFER, NON-REENTRANT
	const char * BuildMessage() const;
};


/// sorting key part types
enum ESphSortKeyPart
{
	SPH_KEYPART_ID,
	SPH_KEYPART_WEIGHT,
	SPH_KEYPART_INT,
	SPH_KEYPART_FLOAT,
	SPH_KEYPART_STRING
};

typedef int ( *SphStringCmp_fn )( const BYTE * pStr1, const BYTE * pStr2 );

/// match comparator state
struct CSphMatchComparatorState
{
	static const int	MAX_ATTRS = 5;

	ESphSortKeyPart		m_eKeypart[MAX_ATTRS];		///< sort-by key part type
	CSphAttrLocator		m_tLocator[MAX_ATTRS];		///< sort-by attr locator

	DWORD				m_uAttrDesc;				///< sort order mask (if i-th bit is set, i-th attr order is DESC)
	DWORD				m_iNow;						///< timestamp (for timesegments sorting mode)
	SphStringCmp_fn		m_fnStrCmp;					///< string comparator

	/// create default empty state
	CSphMatchComparatorState ()
		: m_uAttrDesc ( 0 )
		, m_iNow ( 0 )
		, m_fnStrCmp ( NULL )
	{
		for ( int i=0; i<MAX_ATTRS; i++ )
			m_eKeypart[i] = SPH_KEYPART_ID;
	}

	/// check if any of my attrs are bitfields
	bool UsesBitfields ()
	{
		for ( int i=0; i<MAX_ATTRS; i++ )
			if ( m_eKeypart[i]==SPH_KEYPART_INT && m_tLocator[i].IsBitfield() )
				return true;
		return false;
	}

	inline int CmpStrings ( const CSphMatch & a, const CSphMatch & b, int iAttr ) const
	{
		assert ( iAttr>=0 && iAttr<MAX_ATTRS );
		assert ( m_eKeypart[iAttr]==SPH_KEYPART_STRING );
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
		return m_fnStrCmp ( aa, bb );
	}
};


/// generic match sorter interface
class ISphMatchSorter
{
public:
	bool				m_bRandomize;
	int					m_iTotal;

protected:
	CSphSchema			m_tSchema;		///< sorter schema (adds dynamic attributes on top of index schema)

public:
	/// ctor
						ISphMatchSorter () : m_bRandomize ( false ), m_iTotal ( 0 ) {}

	/// virtualizing dtor
	virtual				~ISphMatchSorter () {}

	/// check if this sorter needs attr values
	virtual bool		UsesAttrs () const = 0;

	/// check if this sorter does groupby
	virtual bool		IsGroupby () const = 0;

	/// set match comparator state
	virtual void		SetState ( const CSphMatchComparatorState & ) = 0;

	/// set group comparator state
	virtual void		SetGroupState ( const CSphMatchComparatorState & ) {}

	/// set MVA pool pointer (for MVA+groupby sorters)
	virtual void		SetMVAPool ( const DWORD * ) {}

	/// set string pool pointer (for string+groupby sorters)
	virtual void		SetStringPool ( const BYTE * ) {}

	/// set schemas
	virtual void				SetSchema ( const CSphSchema & tSchema ) { m_tSchema = tSchema; }

	/// get incoming schema
	virtual const CSphSchema &	GetSchema () const { return m_tSchema; }

	/// base push
	/// returns false if the entry was rejected as duplicate
	/// returns true otherwise (even if it was not actually inserted)
	virtual bool		Push ( const CSphMatch & tEntry ) = 0;

	/// submit pre-grouped match
	virtual bool		PushGrouped ( const CSphMatch & tEntry ) = 0;

	/// get entries count
	virtual int			GetLength () const = 0;

	/// get total count of non-duplicates Push()ed through this queue
	virtual int			GetTotalCount () const { return m_iTotal; }

	/// get first entry ptr
	/// used for docinfo lookup
	/// entries order does NOT matter and is NOT guaranteed
	/// however top GetLength() entries MUST be stored linearly starting from First()
	virtual CSphMatch *	First () = 0;

	/// store all entries into specified location and remove them from the queue
	/// entries are stored in properly sorted order,
	/// if iTag is non-negative, entries are also tagged; otherwise, their tag's unchanged
	virtual void		Flatten ( CSphMatch * pTo, int iTag ) = 0;
};


/// available docinfo storage strategies
enum ESphDocinfo
{
	SPH_DOCINFO_NONE		= 0,	///< no docinfo available
	SPH_DOCINFO_INLINE		= 1,	///< inline docinfo into index (specifically, into doclists)
	SPH_DOCINFO_EXTERN		= 2		///< store docinfo separately
};


enum ESphHitless
{
	SPH_HITLESS_NONE		= 0,	///< all hits are present
	SPH_HITLESS_SOME		= 1,	///< some of the hits might be omitted (check the flag bit)
	SPH_HITLESS_ALL			= 2,	///< no hits in this index
};


enum ESphHitFormat
{
	SPH_HIT_FORMAT_PLAIN	= 0,	///< all hits are stored in hitlist
	SPH_HIT_FORMAT_INLINE	= 1,	///< hits can be split and inlined into doclist (aka 9-23)
};


struct CSphIndexSettings : public CSphSourceSettings
{
	ESphDocinfo		m_eDocinfo;
	ESphHitFormat	m_eHitFormat;
	bool			m_bHtmlStrip;
	CSphString		m_sHtmlIndexAttrs;
	CSphString		m_sHtmlRemoveElements;
	CSphString		m_sZones;

	ESphHitless		m_eHitless;
	CSphString		m_sHitlessFile;

	bool			m_bVerbose;

					CSphIndexSettings ();
};


/// forward refs to internal searcher classes
class ISphQword;
class ISphQwordSetup;
class CSphQueryContext;


/// generic fulltext index interface
class CSphIndex
{
public:
	typedef void ProgressCallback_t ( const CSphIndexProgress * pStat, bool bPhaseEnd );

	enum
	{
		ATTRS_UPDATED			= ( 1UL<<0 ),
		ATTRS_MVA_UPDATED		= ( 1UL<<1 )
	};

public:
	explicit					CSphIndex ( const char * sIndexName, const char * sFilename );
	virtual						~CSphIndex ();

	virtual const CSphString &	GetLastError () const { return m_sLastError; }
	virtual const CSphString &	GetLastWarning () const { return m_sLastWarning; }
	virtual const CSphSchema &	GetMatchSchema () const { return m_tSchema; }			///< match schema as returned in result set (possibly different from internal storage schema!)

	virtual	void				SetProgressCallback ( ProgressCallback_t * pfnProgress ) { m_pProgress = pfnProgress; }
	virtual void				SetInplaceSettings ( int iHitGap, int iDocinfoGap, float fRelocFactor, float fWriteFactor );
	virtual void				SetPreopen ( bool bValue ) { m_bKeepFilesOpen = bValue; }
	virtual void				SetWordlistPreload ( bool bValue ) { m_bPreloadWordlist = bValue; }
	void						SetTokenizer ( ISphTokenizer * pTokenizer );
	ISphTokenizer *				GetTokenizer () const { return m_pTokenizer; }
	ISphTokenizer *				LeakTokenizer ();
	void						SetDictionary ( CSphDict * pDict );
	CSphDict *					GetDictionary () const { return m_pDict; }
	CSphDict *					LeakDictionary ();
	void						Setup ( const CSphIndexSettings & tSettings );
	const CSphIndexSettings &	GetSettings () const { return m_tSettings; }
	bool						IsStripperInited () const { return m_bStripperInited; }
	virtual SphAttr_t *			GetKillList () const = 0;
	virtual int					GetKillListSize () const = 0;
	virtual bool				HasDocid ( SphDocID_t uDocid ) const = 0;
	virtual bool				IsRT() const { return false; }

public:
	/// build index by indexing given sources
	virtual int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer ) = 0;

	/// build index by mering current index with given index
	virtual bool				Merge ( CSphIndex * pSource, CSphVector<CSphFilterSettings> & dFilters, bool bMergeKillLists ) = 0;

public:
	/// check all data files, preload schema, and preallocate enough shared RAM to load memory-cached data
	virtual bool				Prealloc ( bool bMlock, bool bStripPath, CSphString & sWarning ) = 0;

	/// deallocate all previously preallocated shared data
	virtual void				Dealloc () = 0;

	/// precache everything which needs to be precached
	// WARNING, WILL BE CALLED FROM DIFFERENT PROCESS, MUST ONLY MODIFY SHARED MEMORY
	virtual bool				Preread () = 0;

	/// set new index base path
	virtual void				SetBase ( const char * sNewBase ) = 0;

	/// set new index base path, and physically rename index files too
	virtual bool				Rename ( const char * sNewBase ) = 0;

	/// obtain exclusive lock on this index
	virtual bool				Lock () = 0;

	/// dismiss exclusive lock and unlink lock file
	virtual void				Unlock () = 0;

	/// relock shared RAM (only on daemonization)
	virtual bool				Mlock () = 0;

	/// called when index is loaded and prepared to work
	virtual void				PostSetup() = 0;

public:
	virtual bool						EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const = 0;
	virtual const CSphSourceStats &		GetStats () const = 0;
	void						SetCacheSize ( int iMaxCachedDocs, int iMaxCachedHits );
	virtual bool				MultiQuery ( const CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag=0 ) const = 0;
	virtual bool				MultiQueryEx ( int iQueries, const CSphQuery * ppQueries, CSphQueryResult ** ppResults, ISphMatchSorter ** ppSorters, const CSphVector<CSphFilterSettings> * pExtraFilters, int iTag=0 ) const = 0;
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats, CSphString & sError ) const = 0;

public:
	/// updates memory-cached attributes in real time
	/// returns non-negative amount of actually found and updated records on success
	/// on failure, -1 is returned and GetLastError() contains error message
	virtual int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError ) = 0;

	/// saves memory-cached attributes, if there were any updates to them
	/// on failure, false is returned and GetLastError() contains error message
	virtual bool				SaveAttributes () = 0;

	virtual DWORD				GetAttributeStatus () const = 0;

public:
	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpHeader ( FILE * fp, const char * sHeaderName, bool bConfig ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpDocids ( FILE * fp ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual int					DebugCheck ( FILE * fp ) = 0;

	/// getter for name
	const char * GetName () { return m_sIndexName.cstr(); }

public:
	int64_t						m_iTID;

	bool						m_bEnableStar;			///< enable star-syntax
	bool						m_bExpandKeywords;		///< enable automatic query-time keyword expansion (to "( word | =word | *word* )")
	int							m_iExpansionLimit;

protected:
	ProgressCallback_t *		m_pProgress;
	CSphSchema					m_tSchema;
	CSphString					m_sLastError;
	CSphString					m_sLastWarning;

	bool						m_bInplaceSettings;
	int							m_iHitGap;
	int							m_iDocinfoGap;
	float						m_fRelocFactor;
	float						m_fWriteFactor;

	bool						m_bKeepFilesOpen;		///< keep files open to avoid race on seamless rotation
	bool						m_bPreloadWordlist;		///< preload wordlists or keep them on disk

	bool						m_bStripperInited;		///< was stripper initialized (old index version (<9) handling)

public:
	bool						m_bId32to64;			///< did we convert id32 to id64 on startup

protected:
	CSphIndexSettings			m_tSettings;

	ISphTokenizer *				m_pTokenizer;
	CSphDict *					m_pDict;

	int							m_iMaxCachedDocs;
	int							m_iMaxCachedHits;
	CSphString					m_sIndexName;
};

// update attributes with index pointer attached
struct CSphAttrUpdateEx
{
	const CSphAttrUpdate*	m_pUpdate;	///< the unchangeable update pool
	CSphIndex *			m_pIndex;		///< the index on which the update should happen
	CSphString *		m_pError;		///< the error, if any
	int					m_iAffected;	///< num of updated rows.
	CSphAttrUpdateEx()
		: m_pUpdate ( NULL )
		, m_pIndex ( NULL )
		, m_pError ( NULL )
		, m_iAffected ( 0 )
	{}
};

/////////////////////////////////////////////////////////////////////////////

/// create phrase fulltext index implemntation
CSphIndex *			sphCreateIndexPhrase ( const char* szIndexName, const char * sFilename );

/// tell libsphinx to be quiet or not (logs and loglevels to come later)
void				sphSetQuiet ( bool bQuiet );

/// creates proper queue for given query
/// may return NULL on error; in this case, error message is placed in sError
/// if the pUpdate is given, creates the updater's queue and perform the index update
/// instead of searching
ISphMatchSorter *	sphCreateQueue ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sError, bool bComputeItems=true, CSphSchema * pExtra=NULL, CSphAttrUpdateEx* pUpdate=NULL );

/// convert queue to sorted array, and add its entries to result's matches array
void				sphFlattenQueue ( ISphMatchSorter * pQueue, CSphQueryResult * pResult, int iTag );

/// setup per-keyword read buffer sizes
void				sphSetReadBuffers ( int iReadBuffer, int iReadUnhinted );

/// check query for expressions
bool				sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema );

/// initialize collation tables
void				sphCollationInit ();

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

//
// $Id$
//
