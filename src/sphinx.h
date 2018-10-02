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

#ifndef _sphinx_
#define _sphinx_

/////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
	#define USE_MYSQL		1	/// whether to compile MySQL support
	#define USE_PGSQL		0	/// whether to compile PgSQL support
	#define USE_ODBC		1	/// whether to compile ODBC support
	#define USE_LIBEXPAT	1	/// whether to compile libexpat support
	#define USE_LIBICONV	1	/// whether to compile iconv support
	#define	USE_LIBSTEMMER	0	/// whether to compile libstemmber support
	#define	USE_RE2			0	/// whether to compile RE2 support
	#define USE_RLP			0	/// whether to compile RLP support
	#define USE_WINDOWS		1	/// whether to compile for Windows
	#define USE_SYSLOG		0	/// whether to use syslog for logging
	#define HAVE_STRNLEN	1	

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
#include <WS2tcpip.h>
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

typedef uint64_t		SphWordID_t;
typedef uint64_t		SphDocID_t;

#define DOCID_MAX		U64C(0xffffffffffffffff)
#define WORDID_MAX		U64C(0xffffffffffffffff)
#define DOCID_FMT		UINT64_FMT
#define DOCINFO_IDSIZE	2

STATIC_SIZE_ASSERT ( SphWordID_t, 8 );
STATIC_SIZE_ASSERT ( SphDocID_t, 8 );

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

template < typename DOCID >
inline DOCID DOCINFO2ID_T ( const DWORD * pDocinfo );

template<> inline DWORD DOCINFO2ID_T ( const DWORD * pDocinfo )
{
	return pDocinfo[0];
}

template<> inline uint64_t DOCINFO2ID_T ( const DWORD * pDocinfo )
{
#if USE_LITTLE_ENDIAN
	return *(uint64_t *) pDocinfo;
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
	*( uint64_t * ) pDocinfo = uValue;
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
template < typename DOCID > inline DWORD *			STATIC2DOCINFO_T ( DWORD * pAttrs )		{ assert ( pAttrs ); return pAttrs-DWSIZEOF(DOCID); }
template < typename DOCID > inline const DWORD *	STATIC2DOCINFO_T ( const DWORD * pAttrs )	{ assert ( pAttrs ); return pAttrs-DWSIZEOF(DOCID); }
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

#ifdef BUILD_WITH_CMAKE
	#include "gen_sphinxversion.h"
#else
	#include "sphinxversion.h"
#endif

#ifndef SPHINX_TAG
#define BANNER_TAG "dev"
#else
#define BANNER_TAG SPHINX_TAG
#endif

#ifndef GIT_TIMESTAMP_ID
#define GIT_TIMESTAMP_ID "000101"
#endif

// this line is deprecated and no more used. Leaved here for a while.
// numbers now to be defined via sphinxversion.h
#ifndef VERNUMBERS
	#define VERNUMBERS    "7.7.7"
#endif

#define SPHINX_VERSION          VERNUMBERS " " SPH_GIT_COMMIT_ID "@" GIT_TIMESTAMP_ID " " BANNER_TAG
#define SPHINX_BANNER			"Manticore " SPHINX_VERSION "\nCopyright (c) 2001-2016, Andrew Aksyonoff\n" \
	"Copyright (c) 2008-2016, Sphinx Technologies Inc (http://sphinxsearch.com)\n" \
	"Copyright (c) 2017-2018, Manticore Software LTD (http://manticoresearch.com)\n\n"
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

/// immediately interrupt current query
void			sphInterruptNow();

/// check if we got interrupted
bool			sphInterrupted();

/// initialize IO statistics collecting
bool			sphInitIOStats ();

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
	friend class CSphTokenizer_UTF8_Base;
	friend class CSphTokenizerBase2;

public:
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
	static const int				MAX_SYNONYM_LEN		= 1024;	///< max synonyms map-from length, bytes

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
	bool							m_bEscaped = false;			///< backslash handling flag
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

using ISphTokenizerRefPtr_c = CSphRefcountedPtr<ISphTokenizer>;

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
	virtual void		LoadStopwords ( const char * sFiles, const ISphTokenizer * pTokenizer ) = 0;

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

using CSphDictRefPtr_c = CSphRefcountedPtr<CSphDict>;

/// returns pDict, if stateless. Or it's clone, if not
CSphDict * GetStatelessDict ( CSphDict * pDict );

/// traits dictionary factory (no storage, only tokenizing, lemmatizing, etc.)
CSphDict * sphCreateDictionaryTemplate ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, CSphString & sError );

/// CRC32/FNV64 dictionary factory
CSphDict * sphCreateDictionaryCRC ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, CSphString & sError );

/// keyword-storing dictionary factory
CSphDict * sphCreateDictionaryKeywords ( const CSphDictSettings & tSettings, const CSphEmbeddedFiles * pFiles, const ISphTokenizer * pTokenizer, const char * sIndex, CSphString & sError );

/// clear wordform cache
void sphShutdownWordforms ();

/// update/clear global IDF cache
bool sphPrereadGlobalIDF ( const CSphString & sPath, CSphString & sError );
void sphUpdateGlobalIDFs ( const StrVec_t & dFiles );
void sphInitGlobalIDFs ();
void sphShutdownGlobalIDFs ();

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
	SphDocID_t		m_uDocID = DOCID_MAX;		///< document ID
	SphWordID_t		m_uWordID = WORDID_MAX;		///< word ID in current dictionary
	Hitpos_t		m_uWordPos = EMPTY_HIT;		///< word position in current document
};


/// attribute locator within the row
struct CSphAttrLocator
{
	// OPTIMIZE? try packing these
	int				m_iBitOffset = -1;
	int				m_iBitCount = -1;
	bool			m_bDynamic = false;

	CSphAttrLocator () = default;

	explicit CSphAttrLocator ( int iBitOffset, int iBitCount=ROWITEM_BITS )
		: m_iBitOffset ( iBitOffset )
		, m_iBitCount ( iBitCount )
		, m_bDynamic ( true )
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

	bool operator == ( const CSphAttrLocator & rhs ) const
	{
		return m_iBitOffset==rhs.m_iBitOffset && m_iBitCount==rhs.m_iBitCount && m_bDynamic==rhs.m_bDynamic;
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
	friend class ISphSchema;
	friend class CSphSchema;
	friend class CSphSchemaHelper;
	friend class CSphRsetSchema;

public:
	SphDocID_t				m_uDocID = 0;		///< document ID
	const CSphRowitem *		m_pStatic = nullptr;		///< static part (stored in and owned by the index)
	CSphRowitem *			m_pDynamic = nullptr;		///< dynamic part (computed per query; owned by the match)
	int						m_iWeight = 0;		///< my computed weight
	int						m_iTag = 0;			///< my index tag

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

		m_uDocID = 0;
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
			m_uDocID = rhs.m_uDocID;
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
		if ( tLoc.IsID() )
			return ( SphAttr_t ) m_uDocID;
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
		if ( tLoc.IsID() )
			return;
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
	const DWORD * GetAttrMVA ( const CSphAttrLocator & tLoc, const DWORD * pPool, bool bArenaProhibit ) const;

	/// "manually" prevent copying
	CSphMatch & operator = ( const CSphMatch & ) = delete;
	CSphMatch ( const CSphMatch &) = delete;
};

/// specialized swapper
inline void Swap ( CSphMatch & a, CSphMatch & b )
{
	Swap ( a.m_uDocID, b.m_uDocID );
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
	explicit				CSphSchema ( const char * sName="(nameless)" );
							CSphSchema ( const CSphSchema & rhs );

	CSphSchema &			operator = ( const ISphSchema & rhs );
	CSphSchema &			operator = ( const CSphSchema & rhs );
	CSphSchema &			operator = ( CSphSchema && rhs ) noexcept;

	/// visitor-style uber-virtual assignment implementation
	virtual void			AssignTo ( CSphRsetSchema & lhs ) const;

	virtual const char *	GetName() const;

	virtual void			AddAttr ( const CSphColumnInfo & tAttr, bool bDynamic );

	void					AddField ( const char * sFieldName );
	void					AddField ( const CSphColumnInfo & tField );

	/// get field index by name
	/// returns -1 if not found
	virtual int				GetFieldIndex ( const char * sName ) const;

	/// get attribute index by name
	/// returns -1 if not found
	virtual int				GetAttrIndex ( const char * sName ) const;

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

protected:
	static const int			HASH_THRESH		= 32;
	static const int			BUCKET_COUNT	= 256;

	WORD						m_dBuckets [ BUCKET_COUNT ];	///< uses indexes in m_dAttrs as ptrs; 0xffff is like NULL in this hash

	CSphString					m_sName;		///< my human-readable name

	int							m_iFirstFieldLenAttr;///< position of the first field length attribute (cached on insert/delete)
	int							m_iLastFieldLenAttr; ///< position of the last field length attribute (cached on insert/delete)

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
	int							ActualLen() const;	///< len of m_pIndexSchema accounting removed stuff

public:
								~CSphRsetSchema() override {}

	CSphRsetSchema &			operator = ( const ISphSchema & rhs );
	CSphRsetSchema &			operator = ( const CSphSchema & rhs );

	virtual void				AddAttr ( const CSphColumnInfo & tCol, bool bDynamic );
	virtual void				AssignTo ( CSphRsetSchema & lhs ) const		{ lhs = *this; }
	virtual const char *		GetName() const;

public:
	virtual int					GetRowSize() const;
	virtual int					GetStaticSize() const;
	virtual int					GetAttrsCount() const;
	virtual int					GetFieldsCount() const;
	virtual int					GetAttrIndex ( const char * sName ) const;
	virtual const CSphColumnInfo &	GetField ( int iIndex ) const;
	virtual const CSphVector<CSphColumnInfo> & GetFields () const;
	virtual const CSphColumnInfo &	GetAttr ( int iIndex ) const;
	virtual const CSphColumnInfo *	GetAttr ( const char * sName ) const;

	virtual int					GetAttrId_FirstFieldLen() const;
	virtual int					GetAttrId_LastFieldLen() const;

public:
	void						RemoveStaticAttr ( int iAttr );
	void						Reset();

public:
	/// swap in a subset of current attributes, with not necessarily (!) unique names
	/// used to create a network result set (ie. rset to be sent and then discarded)
	/// WARNING, DO NOT USE THIS UNLESS ABSOLUTELY SURE!
	virtual void				SwapAttrs ( CSphVector<CSphColumnInfo> & dAttrs );
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
			tHit.m_uDocID = uDocid;
			tHit.m_uWordID = uWordid;
			tHit.m_uWordPos = uPos;
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
	virtual ISphFieldFilter *	Clone() = 0;

	void						SetParent ( ISphFieldFilter * pParent );

protected:
	ISphFieldFilter *		m_pParent;
};

using ISphFieldFilterRefPtr_c = CSphRefcountedPtr<ISphFieldFilter>;


/// create a regexp field filter
ISphFieldFilter * sphCreateRegexpFilter ( const CSphFieldFilterSettings & tFilterSettings, CSphString & sError );

/// create a RLP field filter
ISphFieldFilter * sphCreateRLPFilter ( ISphFieldFilter * pParent, const char * szRLPRoot, const char * szRLPEnv, const char * szRLPCtx, const char * szBlendChars, CSphString & sError );


/// generic data source
class CSphSource : public CSphSourceSettings
{
public:
	CSphMatch							m_tDocInfo;		///< current document info
	StrVec_t							m_dStrAttrs;	///< current document string attrs
	CSphVector<DWORD>					m_dMva;			///< MVA storage for mva64

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
	/// returns true and sets m_tDocInfo.m_uDocID to 0 on eof
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

	/// get next multi-valued (id,attr-value) or (id, offset) for mva64 tuple to m_tDocInfo
	virtual bool						IterateMultivaluedNext () = 0;

	/// begin iterating values of multi-valued attribute iAttr stored in a field
	/// will fail if iAttr is out of range, or is not multi-valued
	virtual SphRange_t					IterateFieldMVAStart ( int iAttr ) = 0;

	/// begin iterating kill list
	virtual bool						IterateKillListStart ( CSphString & sError ) = 0;

	/// get next kill list doc id
	virtual bool						IterateKillListNext ( SphDocID_t & uDocId ) = 0;

	/// post-index callback
	/// gets called when the indexing is succesfully (!) over
	virtual void						PostIndex () {}

protected:
	ISphTokenizerRefPtr_c				m_pTokenizer;	///< my tokenizer
	CSphDictRefPtr_c					m_pDict;		///< my dict
	ISphFieldFilterRefPtr_c				m_pFieldFilter;	///< my field filter

	CSphSourceStats						m_tStats;		///< my stats
	CSphSchema 							m_tSchema;		///< my schema

	CSphHTMLStripper *					m_pStripper = nullptr;	///< my HTML stripper
	CSphBitvec							m_tMorphFields;

	int			m_iNullIds = 0;
	int			m_iMaxIds = 0;

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
							~CSphSource_Document () override { SafeDeleteArray ( m_pReadFileBuffer ); }

	/// my generic tokenizer
	bool			IterateDocument ( CSphString & sError ) override;
	ISphHits *		IterateHits ( CSphString & sError ) override;
	void					BuildHits ( CSphString & sError, bool bSkipEndMarker );

	/// field data getter
	/// to be implemented by descendants
	virtual BYTE **			NextDocument ( CSphString & sError ) = 0;
	virtual const int *		GetFieldLengths () const = 0;

	void			SetDumpRows ( FILE * fpDumpRows ) override { m_fpDumpRows = fpDumpRows; }

	virtual SphRange_t		IterateFieldMVAStart ( int iAttr ) override;
	virtual bool			HasJoinedFields () override { return m_iPlainFieldsLength!=m_tSchema.GetFieldsCount(); }

protected:
	int						ParseFieldMVA ( CSphVector < DWORD > & dMva, const char * szValue, bool bMva64 ) const;
	bool					CheckFileField ( const BYTE * sField );
	int						LoadFileField ( BYTE ** ppField, CSphString & sError );

	bool					BuildZoneHits ( SphDocID_t uDocid, BYTE * sWord );
	void					BuildSubstringHits ( SphDocID_t uDocid, bool bPayload, ESphWordpart eWordpart, bool bSkipEndMarker );
	void					BuildRegularHits ( SphDocID_t uDocid, bool bPayload, bool bSkipEndMarker );

	/// register autocomputed attributes such as field lengths (see index_field_lengths)
	bool					AddAutoAttrs ( CSphString & sError );

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

	CSphVector<SphDocID_t>	m_dAllIds;						///< used for joined fields FIXME! unlimited RAM use
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


/// generic SQL source
/// multi-field plain-text documents fetched from given query
struct CSphSource_SQL : CSphSource_Document
{
	explicit			CSphSource_SQL ( const char * sName );
						~CSphSource_SQL () override = default;

	bool				Setup ( const CSphSourceParams_SQL & pParams );
	bool		Connect ( CSphString & sError ) override;
	void		Disconnect () override;

	bool		IterateStart ( CSphString & sError ) override;
	BYTE **		NextDocument ( CSphString & sError ) override;
	const int *	GetFieldLengths () const override;
	void		PostIndex () override;

	bool		HasAttrsConfigured () override { return m_tParams.m_dAttrs.GetLength()!=0; }

	ISphHits *	IterateJoinedHits ( CSphString & sError ) override;

	bool		IterateMultivaluedStart ( int iAttr, CSphString & sError ) override;
	bool		IterateMultivaluedNext () override;

	bool		IterateKillListStart ( CSphString & sError ) override;
	bool		IterateKillListNext ( SphDocID_t & tDocId ) override;

private:
	bool				m_bSqlConnected = false;	///< am i connected?

protected:
	CSphString			m_sSqlDSN;

	BYTE *				m_dFields [ SPH_MAX_FIELDS ] { nullptr };
	int					m_dFieldLengths [ SPH_MAX_FIELDS ];
	ESphUnpackFormat	m_dUnpack [ SPH_MAX_FIELDS ] { SPH_UNPACK_NONE };

	SphDocID_t			m_uMinID = 0;			///< grand min ID
	SphDocID_t			m_uMaxID = 0;			///< grand max ID
	SphDocID_t			m_uCurrentID = 0;		///< current min ID
	SphDocID_t			m_uMaxFetchedID = 0;	///< max actually fetched ID
	int					m_iMultiAttr = -1;		///< multi-valued attr being currently fetched
	int					m_iSqlFields = 0;		///< field count (for row dumper)

	CSphSourceParams_SQL		m_tParams;

	bool				m_bCanUnpack = false;
	bool				m_bUnpackFailed = false;
	bool				m_bUnpackOverflow = false;
	CSphVector<char>	m_dUnpackBuffers [ SPH_MAX_FIELDS ];

	int					m_iJoinedHitField = -1;	///< currently pulling joined hits from this field (index into schema; -1 if not pulling)
	SphDocID_t			m_iJoinedHitID = 0;		///< last document id
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
CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, FILE * pPipe, const char * szSourceName, int iMaxFieldLen, bool bProxy, CSphString & sError );
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
	ESphMvaFunc			m_eMvaFunc = SPH_MVAFUNC_NONE;		///< MVA folding function
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
	const SphAttr_t *	GetValueArray () const		{ return m_pValues ? m_pValues : &(m_dValues[0]); }
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
class CSphQuery;
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
	QUERY_OPT_ENABLED
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
	CSphVector<DWORD>	m_dWeights;		///< user-supplied per-field weights. may be NULL. default is NULL
	ESphMatchMode	m_eMode;		///< match mode. default is "match all"
	ESphRankMode	m_eRanker;		///< ranking mode, default is proximity+BM25
	CSphString		m_sRankerExpr;	///< ranking expression for SPH_RANK_EXPR
	CSphString		m_sUDRanker;	///< user-defined ranker name
	CSphString		m_sUDRankerOpts;	///< user-defined ranker options
	ESphSortOrder	m_eSort;		///< sort mode
	CSphString		m_sSortBy;		///< attribute to sort by
	int64_t			m_iRandSeed;	///< random seed for ORDER BY RAND(), -1 means do not set
	int				m_iMaxMatches;	///< max matches to retrieve, default is 1000. more matches use more memory and CPU time to hold and sort them

	bool			m_bSortKbuffer;	///< whether to use PQ or K-buffer sorting algorithm
	bool			m_bZSlist;		///< whether the ranker has to fetch the zonespanlist with this query
	bool			m_bSimplify;	///< whether to apply boolean simplification
	bool			m_bPlainIDF;		///< whether to use PlainIDF=log(N/n) or NormalizedIDF=log((N-n+1)/n)
	bool			m_bGlobalIDF;		///< whether to use local indexes or a global idf file
	bool			m_bNormalizedTFIDF;	///< whether to scale IDFs by query word count, so that TF*IDF is normalized
	bool			m_bLocalDF;			///< whether to use calculate DF among local indexes
	bool			m_bLowPriority;		///< set low thread priority for this query
	DWORD			m_uDebugFlags;
	QueryOption_e	m_eExpandKeywords;	///< control automatic query-time keyword expansion

	CSphVector<CSphFilterSettings>	m_dFilters;	///< filters
	CSphVector<FilterTreeItem_t>	m_dFilterTree;

	CSphString		m_sGroupBy;			///< group-by attribute name(s)
	CSphString		m_sFacetBy;			///< facet-by attribute name(s)
	ESphGroupBy		m_eGroupFunc;		///< function to pre-process group-by attribute value with
	CSphString		m_sGroupSortBy;		///< sorting clause for groups in group-by mode
	CSphString		m_sGroupDistinct;	///< count distinct values for this attribute

	int				m_iCutoff;			///< matches count threshold to stop searching at (default is 0; means to search until all matches are found)

	int				m_iRetryCount = -1;		///< retry count, for distributed queries. (-1 means 'use default')
	int				m_iRetryDelay = -1;		///< retry delay, for distributed queries. (-1 means 'use default')
	int				m_iAgentQueryTimeout;	///< agent query timeout override, for distributed queries

	bool			m_bGeoAnchor;		///< do we have an anchor
	CSphString		m_sGeoLatAttr;		///< latitude attr name
	CSphString		m_sGeoLongAttr;		///< longitude attr name
	float			m_fGeoLatitude;		///< anchor latitude
	float			m_fGeoLongitude;	///< anchor longitude

	CSphVector<CSphNamedInt>	m_dIndexWeights;	///< per-index weights
	CSphVector<CSphNamedInt>	m_dFieldWeights;	///< per-field weights

	DWORD			m_uMaxQueryMsec;	///< max local index search time, in milliseconds (default is 0; means no limit)
	int				m_iMaxPredictedMsec; ///< max predicted (!) search time limit, in milliseconds (0 means no limit)
	CSphString		m_sComment;			///< comment to pass verbatim in the log file

	CSphVector<CSphAttrOverride>	m_dOverrides;	///< per-query attribute value overrides

	CSphString		m_sSelect;			///< select-list (attributes and/or expressions)
	CSphString		m_sOrderBy;			///< order-by clause

	CSphString		m_sOuterOrderBy;	///< temporary (?) subselect hack
	int				m_iOuterOffset;		///< keep and apply outer offset at master
	int				m_iOuterLimit;
	bool			m_bHasOuter;

	bool			m_bReverseScan;		///< perform scan in reverse order
	bool			m_bIgnoreNonexistent; ///< whether to warning or not about non-existent columns in select list
	bool			m_bIgnoreNonexistentIndexes; ///< whether to error or not about non-existent indexes in index list
	bool			m_bStrict;			///< whether to warning or not about incompatible types
	bool			m_bSync;			///< whether or not use synchronous operations (optimize, etc.)

	CSphFilterSettings	m_tHaving;		///< post aggregate filtering (got applied only on master)

public:
	int				m_iSQLSelectStart;	///< SQL parser helper
	int				m_iSQLSelectEnd;	///< SQL parser helper

	int				m_iGroupbyLimit;	///< number of elems within group

public:
	CSphVector<CSphQueryItem>	m_dItems;		///< parsed select-list
	CSphVector<CSphQueryItem>	m_dRefItems;	///< select-list prior replacing by facet
	ESphCollation				m_eCollation;	///< ORDER BY collation
	bool						m_bAgent;		///< agent mode (may need extra cols on output)

	CSphString		m_sQueryTokenFilterLib;		///< token filter library name
	CSphString		m_sQueryTokenFilterName;	///< token filter name
	CSphString		m_sQueryTokenFilterOpts;	///< token filter options

public:
					CSphQuery ();		///< ctor, fills defaults
					~CSphQuery ();		///< dtor, frees owned stuff

	/// parse select list string into items
	bool			ParseSelectList ( CSphString & sError );
	bool			m_bFacet;			///< whether this a facet query
	bool			m_bFacetHead;

	QueryType_e		m_eQueryType {QUERY_API};		///< queries from sphinxql require special handling
	const QueryParser_i * m_pQueryParser {nullptr};	///< queries do not own this parser

	StrVec_t m_dIncludeItems;
	StrVec_t m_dExcludeItems;
};


/// some low-level query stats
struct CSphQueryStats
{
	int64_t *	m_pNanoBudget;		///< pointer to max_predicted_time budget (counted in nanosec)
	DWORD		m_iFetchedDocs;		///< processed documents
	DWORD		m_iFetchedHits;		///< processed hits (aka positions)
	DWORD		m_iSkips;			///< number of Skip() calls

				CSphQueryStats();

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
class CSphQueryResult : public CSphQueryResultMeta
{
public:
	CSphSwapVector<CSphMatch>	m_dMatches;			///< top matching documents, no more than MAX_MATCHES
	CSphSchema				m_tSchema;				///< result schema
	const DWORD *			m_pMva = nullptr;		///< pointer to MVA storage
	const BYTE *			m_pStrings = nullptr;	///< pointer to strings storage
	bool					m_bArenaProhibit = false;
	int						m_iOffset = 0;			///< requested offset into matches array
	int						m_iCount = 0;			///< count which will be actually served (computed from total, offset and limit)
	int						m_iSuccesses = 0;
	CSphQueryProfile *		m_pProfile = nullptr;	///< filled when query profiling is enabled; NULL otherwise

							~CSphQueryResult () override; 	///< dtor, which releases all owned stuff
};

/////////////////////////////////////////////////////////////////////////////
// ATTRIBUTE UPDATE QUERY
/////////////////////////////////////////////////////////////////////////////

struct CSphAttrUpdate
{
	CSphVector<char*>				m_dAttrs;		///< update schema, attr names to update
	CSphVector<ESphAttr>			m_dTypes;		///< update schema, attr types to update
	CSphVector<DWORD>				m_dPool;		///< update values pool
	CSphVector<SphDocID_t>			m_dDocids;		///< document IDs vector
	CSphVector<const CSphRowitem*>	m_dRows;		///< document attribute's vector, used instead of m_dDocids.
	CSphVector<int>					m_dRowOffset;	///< document row offsets in the pool (1 per doc, i.e. the length is the same as of m_dDocids)
	bool							m_bIgnoreNonexistent = false;	///< whether to warn about non-existen attrs, or just silently ignore them
	bool							m_bStrict = false;				///< whether to check for incompatible types first, or just ignore them

	~CSphAttrUpdate()
	{
		for ( auto & pAttrs : m_dAttrs )
			SafeDeleteArray ( pAttrs );
	}
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

	Phase_e			m_ePhase { PHASE_COLLECT };		///< current indexing phase

	int64_t			m_iDocuments = 0;	///< PHASE_COLLECT: documents collected so far
	int64_t			m_iBytes = 0;		///< PHASE_COLLECT: bytes collected so far;
										///< PHASE_PREREAD: bytes read so far;
	int64_t			m_iBytesTotal = 0;	///< PHASE_PREREAD: total bytes to read;

	int64_t			m_iAttrs = 0;		///< PHASE_COLLECT_MVA, PHASE_SORT_MVA: attrs processed so far
	int64_t			m_iAttrsTotal = 0;	///< PHASE_SORT_MVA: attrs total

	SphOffset_t		m_iHits {0};		///< PHASE_SORT: hits sorted so far
	SphOffset_t		m_iHitsTotal {0};	///< PHASE_SORT: hits total

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
	SPH_KEYPART_ID,
	SPH_KEYPART_WEIGHT,
	SPH_KEYPART_INT,
	SPH_KEYPART_FLOAT,
	SPH_KEYPART_STRING,
	SPH_KEYPART_STRINGPTR
};


/// JSON key lookup stuff
struct JsonKey_t
{
	CSphString		m_sKey;		///< name string
	DWORD			m_uMask = 0;	///< Bloom mask for this key
	int				m_iLen = 0;		///< name length, in bytes

	JsonKey_t () = default;
	explicit JsonKey_t ( const char * sKey, int iLen );
};


enum StringSource_e
{
	STRING_STATIC,		// static packed string, comes from index, no trailing zero
	STRING_DATAPTR,		// packed string, comes from memory (_PTR attrs), no tailing zero
	STRING_PLAIN		// plain string, can be zero-terminated or not
};

// iLen1 and iLen2 should be specified only in case of STRING_PLAIN
using SphStringCmp_fn =  int ( * )( const BYTE * pStr1, const BYTE * pStr2, StringSource_e eStrSource, int iLen1, int iLen2 );

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
			m_eKeypart[i] = SPH_KEYPART_ID;
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
		for ( int i=0; i<MAX_ATTRS; i++ )
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

		StringSource_e eStrSource = m_eKeypart[iAttr]==SPH_KEYPART_STRING ? STRING_STATIC : STRING_DATAPTR;
		return m_fnStrCmp ( aa, bb, eStrSource, 0, 0 );
	}

	void FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema );
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

	SphDocID_t			m_iJustPushed;
	int					m_iMatchCapacity;
	CSphTightVector<SphDocID_t> m_dJustPopped;

protected:
	ISphSchema *		m_pSchema { nullptr };	///< sorter schema (adds dynamic attributes on top of index schema)
	CSphMatchComparatorState	m_tState;		///< protected to set m_iNow automatically on SetState() calls

public:
	/// ctor
						ISphMatchSorter () : m_bRandomize ( false ), m_iTotal ( 0 ), m_iJustPushed ( 0 ), m_iMatchCapacity ( 0 ) {}

	/// virtualizing dtor
	virtual				~ISphMatchSorter ();

	/// check if this sorter needs attr values
	virtual bool		UsesAttrs () const = 0;

	// check if sorter might be used in multi-queue
	virtual bool		CanMulti () const = 0;

	/// check if this sorter does groupby
	virtual bool		IsGroupby () const = 0;

	/// set match comparator state
	virtual void		SetState ( const CSphMatchComparatorState & tState );

	/// get match comparator stat
	virtual CSphMatchComparatorState &	GetState() { return m_tState; }

	/// set group comparator state
	virtual void		SetGroupState ( const CSphMatchComparatorState & ) {}

	/// set MVA pool pointer (for MVA+groupby sorters)
	virtual void SetMVAPool ( const DWORD *, bool ) {}

	/// set string pool pointer (for string+groupby sorters)
	virtual void		SetStringPool ( const BYTE * ) {}

	/// set sorter schema
	virtual void		SetSchema ( ISphSchema * pSchema );

	/// get incoming schema
	virtual const ISphSchema * GetSchema () const { return m_pSchema; }

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
	virtual int64_t		GetTotalCount () const { return m_iTotal; }

	/// process collected entries up to length count
	virtual void		Finalize ( ISphMatchProcessor & tProcessor, bool bCallProcessInResultSetOrder ) = 0;

	/// store all entries into specified location and remove them from the queue
	/// entries are stored in properly sorted order,
	/// if iTag is non-negative, entries are also tagged; otherwise, their tag's unchanged
	/// return sored entries count, might be less than length due of aggregate filtering phase
	virtual int			Flatten ( CSphMatch * pTo, int iTag ) = 0;

	/// get a pointer to the worst element, NULL if there is no fixed location
	virtual const CSphMatch *	GetWorst() const { return NULL; }
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
	SPH_HITLESS_ALL			= 2	///< no hits in this index
};


enum ESphHitFormat
{
	SPH_HIT_FORMAT_PLAIN	= 0,	///< all hits are stored in hitlist
	SPH_HIT_FORMAT_INLINE	= 1	///< hits can be split and inlined into doclist (aka 9-23)
};


enum ESphRLPFilter
{
	SPH_RLP_NONE			= 0,	///< rlp not used
	SPH_RLP_PLAIN			= 1,	///< rlp used to tokenize every document
	SPH_RLP_BATCHED			= 2		///< rlp used to batch documents and tokenize several documents at once
};


struct CSphIndexSettings : CSphSourceSettings
{
	ESphDocinfo		m_eDocinfo = SPH_DOCINFO_NONE;
	ESphHitFormat	m_eHitFormat = SPH_HIT_FORMAT_PLAIN;
	bool			m_bHtmlStrip = false;
	CSphString		m_sHtmlIndexAttrs;
	CSphString		m_sHtmlRemoveElements;
	CSphString		m_sZones;
	ESphHitless		m_eHitless = SPH_HITLESS_NONE;
	CSphString		m_sHitlessFiles;
	bool			m_bVerbose = false;
	int				m_iEmbeddedLimit = 0;

	ESphBigram		m_eBigramIndex = SPH_BIGRAM_NONE;
	CSphString		m_sBigramWords;
	StrVec_t		m_dBigramWords;

	DWORD			m_uAotFilterMask = 0;			///< lemmatize_XX_all forces us to transform queries on the index level too
	ESphRLPFilter	m_eChineseRLP = SPH_RLP_NONE;	///< chinese RLP filter
	CSphString		m_sRLPContext;					///< path to RLP context file

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
};

struct KillListTrait_t
{
	const SphDocID_t *	m_pBegin;
	int					m_iLen;
};
typedef CSphVector<KillListTrait_t> KillListVector;

struct CSphMultiQueryArgs : public ISphNoncopyable
{
	const KillListVector &					m_dKillList;
	const int								m_iIndexWeight;
	int										m_iTag;
	DWORD									m_uPackedFactorFlags;
	bool									m_bLocalDF;
	const SmallStringHash_T<int64_t> *		m_pLocalDocs;
	int64_t									m_iTotalDocs;
	bool									m_bModifySorterSchemas {true};

	CSphMultiQueryArgs ( const KillListVector & dKillList, int iIndexWeight );
};

enum KeywordExpansion_e
{
	KWE_DISABLED	=	0,
	KWE_EXACT	=		( 1UL << 0 ),
	KWE_STAR	=		( 1UL << 1 ),

	KWE_ENABLED = ( KWE_EXACT | KWE_STAR ),
};


/// generic fulltext index interface
class CSphIndex : public ISphKeywordsStat
{
public:

	enum
	{
		ATTRS_UPDATED			= ( 1UL<<0 ),
		ATTRS_MVA_UPDATED		= ( 1UL<<1 ),
		ATTRS_STRINGS_UPDATED	= ( 1UL<<2 )
	};

public:
	explicit					CSphIndex ( const char * sIndexName, const char * sFilename );
	virtual						~CSphIndex ();

	virtual const CSphString &	GetLastError () const { return m_sLastError; }
	virtual const CSphString &	GetLastWarning () const { return m_sLastWarning; }
	virtual const CSphSchema &	GetMatchSchema () const { return m_tSchema; }			///< match schema as returned in result set (possibly different from internal storage schema!)

	virtual	void				SetProgressCallback ( CSphIndexProgress::IndexingProgress_fn pfnProgress ) = 0;
	virtual void				SetInplaceSettings ( int iHitGap, int iDocinfoGap, float fRelocFactor, float fWriteFactor );
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
	virtual SphDocID_t *		GetKillList () const = 0;
	virtual int					GetKillListSize () const = 0;
	virtual bool				HasDocid ( SphDocID_t uDocid ) const = 0;
	virtual bool				IsRT() const { return false; }
	void						SetBinlog ( bool bBinlog ) { m_bBinlog = bBinlog; }
	virtual int64_t *			GetFieldLens() const { return NULL; }
	virtual bool				IsStarDict() const { return true; }
	int64_t						GetIndexId() const { return m_iIndexId; }

public:
	/// build index by indexing given sources
	virtual int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer ) = 0;

	/// build index by mering current index with given index
	virtual bool				Merge ( CSphIndex * pSource, const CSphVector<CSphFilterSettings> & dFilters, bool bMergeKillLists ) = 0;

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
	virtual bool				FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const = 0;
	virtual void				GetSuggest ( const SuggestArgs_t & , SuggestResult_t & ) const {}

public:
	/// updates memory-cached attributes in real time
	/// returns non-negative amount of actually found and updated records on success
	/// on failure, -1 is returned and GetLastError() contains error message
	virtual int					UpdateAttributes ( const CSphAttrUpdate & tUpd, int iIndex, CSphString & sError, CSphString & sWarning ) = 0;

	/// saves memory-cached attributes, if there were any updates to them
	/// on failure, false is returned and GetLastError() contains error message
	virtual bool				SaveAttributes ( CSphString & sError ) const = 0;

	virtual DWORD				GetAttributeStatus () const = 0;

	virtual bool				AddRemoveAttribute ( bool bAddAttr, const CSphString & sAttrName, ESphAttr eAttrType, CSphString & sError ) = 0;

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
	virtual void				SetDebugCheck () {}

	/// getter for name
	const char *				GetName () const { return m_sIndexName.cstr(); }

	void						SetName ( const char * sName ) { m_sIndexName = sName; }

	/// get for the base file name
	const char *				GetFilename () const { return m_sFilename.cstr(); }

	/// internal make document id list from external docinfo, DO NOT USE
	virtual bool BuildDocList ( SphAttr_t ** ppDocList, int64_t * pCount, CSphString * pError ) const;

	/// internal replace kill-list and rewrite spk file, DO NOT USE
	virtual bool				ReplaceKillList ( const SphDocID_t *, int ) { return true; }

	virtual void				SetMemorySettings ( bool bMlock, bool bOndiskAttrs, bool bOndiskPool ) = 0;

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
	int							m_iDocinfoGap = 0;
	float						m_fRelocFactor { 0.0f };
	float						m_fWriteFactor { 0.0f };

	bool						m_bKeepFilesOpen = false;	///< keep files open to avoid race on seamless rotation
	bool						m_bBinlog = true;

	bool						m_bStripperInited = true;	///< was stripper initialized (old index version (<9) handling)

protected:
	CSphIndexSettings			m_tSettings;

	ISphFieldFilterRefPtr_c		m_pFieldFilter;
	ISphTokenizerRefPtr_c		m_pTokenizer;
	ISphTokenizerRefPtr_c		m_pQueryTokenizer;
	ISphTokenizerRefPtr_c		m_pQueryTokenizerJson;
	CSphDictRefPtr_c			m_pDict;

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
	const CSphQuery &			m_tQuery;
	const ISphSchema &			m_tSchema;
	CSphString &				m_sError;
	CSphQueryProfile *			m_pProfiler;
	bool						m_bComputeItems = true;
	sph::StringSet *			m_pExtra = nullptr;
	CSphAttrUpdateEx *			m_pUpdate = nullptr;
	CSphVector<SphDocID_t> *	m_pCollection = nullptr;
	bool						m_bZonespanlist = false;
	DWORD						m_uPackedFactorFlags { SPH_FACTOR_DISABLE };
	ISphExprHook *				m_pHook = nullptr;
	const CSphFilterSettings *	m_pAggrFilter = nullptr;

	SphQueueSettings_t ( const CSphQuery & tQuery, const ISphSchema & tSchema, CSphString & sError, CSphQueryProfile * pProfiler = nullptr )
		: m_tQuery ( tQuery )
		, m_tSchema ( tSchema )
		, m_sError ( sError )
		, m_pProfiler ( pProfiler )
	{ }
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
	ESphSortFunc & eFunc, CSphMatchComparatorState & tState, CSphString & sError );

/// creates proper queue for given query
/// may return NULL on error; in this case, error message is placed in sError
/// if the pUpdate is given, creates the updater's queue and perform the index update
/// instead of searching
ISphMatchSorter *	sphCreateQueue ( SphQueueSettings_t & tQueue );

/// convert queue to sorted array, and add its entries to result's matches array
int					sphFlattenQueue ( ISphMatchSorter * pQueue, CSphQueryResult * pResult, int iTag );

/// setup per-keyword read buffer sizes
void				sphSetReadBuffers ( int iReadBuffer, int iReadUnhinted );

/// check query for expressions
bool				sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema );

/// initialize collation tables
void				sphCollationInit ();

//////////////////////////////////////////////////////////////////////////

// pack data pointer attr (length+data), return allocated storage
BYTE *				sphPackPtrAttr ( const BYTE * pData, int iLengthBytes );

// pack data pointer attr to preallocated storage
void				sphPackPtrAttr ( BYTE * pPrealloc, const BYTE * pData, int iLengthBytes );

// allocate buffer, store zipped length, set pointer to free space in buffer
BYTE *				sphPackPtrAttr ( int iLengthBytes, BYTE * & pData );

// unpack data pointer attr, return length
int					sphUnpackPtrAttr ( const BYTE * pData, const BYTE ** ppUnpacked=NULL );

// calculate packed data attr length
int					sphCalcPackedLength ( int iLengthBytes );

// convert plain attr type to corresponding in-memort (_PTR) attr type
ESphAttr			sphPlainAttrToPtrAttr ( ESphAttr eAttrType );

// is this a data ptr attribute?
bool				sphIsDataPtrAttr ( ESphAttr eAttrType );

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
