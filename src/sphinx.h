//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
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
	#define USE_LIBEXPAT	1	/// whether to compile libexpat support
	#define USE_LIBICONV	1	/// whether to compile iconv support
	#define USE_LIBXML		0	/// whether to compile libxml support
	#define USE_WINDOWS		1	/// whether to compile for Windows
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
#else
typedef off_t				SphOffset_t;
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
#define DOCID_FMT		"%" PRIu64
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

//////////////////////////////////////////////////////////////////////////

/// row entry (storage only, does not necessarily map 1:1 to attributes)
typedef DWORD			CSphRowitem;

/// widest integer type that can be be stored as an attribute (ideally, fully decoupled from rowitem size!)
typedef int64_t			SphAttr_t;

const CSphRowitem		ROWITEM_MAX		= UINT_MAX;
const int				ROWITEM_BITS	= 8*sizeof(CSphRowitem);
const int				ROWITEM_SHIFT	= 5;

STATIC_ASSERT ( (1<<ROWITEM_SHIFT)==ROWITEM_BITS, INVALID_ROWITEM_SHIFT );

inline SphDocID_t &		DOCINFO2ID ( const DWORD * pDocinfo )	{ return *(SphDocID_t*)pDocinfo; }
inline DWORD *			DOCINFO2ATTRS ( DWORD * pDocinfo )		{ return pDocinfo+DOCINFO_IDSIZE; }
inline const DWORD *	DOCINFO2ATTRS ( const DWORD * pDocinfo ){ return pDocinfo+DOCINFO_IDSIZE; }

/////////////////////////////////////////////////////////////////////////////

#include "sphinxversion.h"

#define SPHINX_VERSION			"0.9.9" SPHINX_BITS_TAG "-dev (" SPH_SVN_TAGREV ")"
#define SPHINX_BANNER			"Sphinx " SPHINX_VERSION "\nCopyright (c) 2001-2008, Andrew Aksyonoff\n\n"
#define SPHINX_SEARCHD_PROTO	1

#define SPH_MAX_QUERY_WORDS		10
#define SPH_MAX_WORD_LEN		64
#define SPH_MAX_FILENAME_LEN	512
#define SPH_MAX_FIELDS			32

#define SPH_CACHE_WRITE			1048576

/////////////////////////////////////////////////////////////////////////////

/// time, in seconds
float			sphLongTimer ();

/// Sphinx CRC32 implementation
DWORD			sphCRC32 ( const BYTE * pString );

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
void			sphUsleep ( int iMsec );

/// check if file exists and is a readable file
bool			sphIsReadable ( const char * sFilename, CSphString * pError=NULL );

/// set throttling options
void			sphSetThrottling ( int iMaxIOps, int iMaxIOSize );

#if !USE_WINDOWS
/// set process info
void			sphSetProcessInfo ( bool bHead );
#endif

struct CSphIOStats
{
	float		m_fReadTime;
	DWORD		m_iReadOps;
	float		m_fReadKBytes;
	float		m_fWriteTime;
	DWORD		m_iWriteOps;
	float		m_fWriteKBytes;
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
	void		AddRemaps ( const CSphVector<CSphRemapRange> & dRemaps, DWORD uFlags, DWORD uFlagsIfExists );
	void		AddSpecials ( const char * sSpecials );

public:
	const CSphLowercaser &		operator = ( const CSphLowercaser & rhs );

public:
	inline int	ToLower ( int iCode ) const
	{
		if ( iCode<0 || iCode>=MAX_CODE )
			return iCode;
		register int * pChunk = m_pChunk [ iCode>>CHUNK_BITS ];
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
/// parser to build lowercaser from textual config
class CSphCharsetDefinitionParser
{
public:
						CSphCharsetDefinitionParser ();
	bool				Parse ( const char * sConfig, CSphVector<CSphRemapRange> & dRanges );
	const char *		GetLastError ();

protected:
	bool				m_bError;
	char				m_sError [ 1024 ];
	const char *		m_pCurrent;

	bool				Error ( const char * sMessage );
	void				SkipSpaces ();
	bool				IsEof ();
	bool				CheckEof ();
	int					HexDigit ( int c );
	int					ParseCharsetCode ();
};


/////////////////////////////////////////////////////////////////////////////

/// synonym list entry
struct CSphSynonym
{
	CSphString	m_sFrom;	///< specially packed list of map-from tokens
	CSphString	m_sTo;		///< map-to string
	int			m_iFromLen;	///< cached m_sFrom length 
	int			m_iToLen;	///< cached m_sTo length 

	inline bool operator < ( const CSphSynonym & rhs ) const
	{
		return strcmp ( m_sFrom.cstr(), rhs.m_sFrom.cstr() ) < 0;
	}
};

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
	virtual bool					LoadSynonyms ( const char * sFilename, CSphString & sError );

	/// set phrase boundary chars
	virtual bool					SetBoundary ( const char * sConfig, CSphString & sError );

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
	virtual void					EnableQueryParserMode ( bool bEnable ) { m_bShortTokenFilter = bEnable; }

	/// get last token length, in codepoints
	virtual int						GetLastTokenLen () const { return m_iLastTokenLen; }

	/// get last token boundary flag (true if there was a boundary before the token)
	virtual bool					GetBoundary () { return m_bTokenBoundary; }

	/// was last token a special one?
	virtual bool					WasTokenSpecial () { return m_bWasSpecial; }

	/// get amount of overshort keywords skipped before this token
	virtual int						GetOvershortCount () { return m_iOvershortCount; }

public:
	/// get lowercaser
	virtual const CSphLowercaser *	GetLowercaser () const { return &m_tLC; }

	/// spawn a clone of my own
	virtual ISphTokenizer *			Clone ( bool bEscaped ) const = 0;

	/// clone base stuff
	virtual void					CloneBase ( const ISphTokenizer * pFrom, bool bEscaped );

	/// SBCS or UTF-8?
	virtual bool					IsUtf8 () const = 0;

	/// start buffer point of last token
	virtual const char *			GetTokenStart () const = 0;

	/// end buffer point of last token
	virtual const char *			GetTokenEnd () const = 0;

	/// current buffer ptr
	virtual const char *			GetBufferPtr () const = 0;

	/// buffer end
	virtual const char *			GetBufferEnd () const = 0;

	/// set new buffer ptr (must be within current bounds)
	virtual void					SetBufferPtr ( const char * sNewPtr ) = 0;

protected:
	static const int				MAX_SYNONYM_LEN		= 1024;	///< max synonyms map-from length, bytes

	CSphLowercaser					m_tLC;						///< my lowercaser
	int								m_iLastTokenLen;			///< last token length, in codepoints
	bool							m_bTokenBoundary;			///< last token boundary flag (true after boundary codepoint followed by separator)
	bool							m_bBoundary;				///< boundary flag (true immediately after boundary codepoint)
	bool							m_bWasSpecial;				///< special token flag
	bool							m_bEscaped;					///< backslash handling flag
	int								m_iOvershortCount;			///< skipped overshort tokens count
	bool							m_bShortTokenFilter;		///< short token filter flag

	CSphTokenizerSettings			m_tSettings;				///< tokenizer settings
	CSphSavedFile					m_tSynFileInfo;				///< synonyms file info

	CSphVector<CSphSynonym>			m_dSynonyms;				///< active synonyms
	CSphVector<int>					m_dSynStart;				///< map 1st byte to candidate range start
	CSphVector<int>					m_dSynEnd;					///< map 1st byte to candidate range end
};

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
};


/// abstract word dictionary interface
struct CSphDict
{
	/// virtualizing dtor
	virtual				~CSphDict () {}

	/// get word ID by word, "text" version
	/// may apply stemming and modify word inplac
	/// returns 0 for stopwords
	virtual SphWordID_t	GetWordID ( BYTE * pWord ) = 0;

	/// get word ID by word, "text" version
	/// may apply stemming and modify word inplace
	/// accepts words with already prepended MAGIC_WORD_HEAD
	/// appends MAGIC_WORD_TAIL
	/// returns 0 for stopwords
	virtual SphWordID_t	GetWordIDWithMarkers ( BYTE * pWord ) { return GetWordID ( pWord ); }

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
	virtual bool		LoadWordforms ( const char * sFile, ISphTokenizer * pTokenizer ) = 0;

	/// set morphology
	virtual bool		SetMorphology ( const char * szMorph, bool bUseUTF8, CSphString & sError ) = 0;

	/// setup dictionary using settings
	virtual void		Setup ( const CSphDictSettings & tSettings ) = 0;

	/// get dictionary settings
	virtual const CSphDictSettings & GetSettings () const = 0;

	/// stopwords file infos
	virtual const CSphVector <CSphSavedFile> & GetStopwordsFileInfos () = 0;

	/// wordforms file infos
	virtual const CSphSavedFile & GetWordformsFileInfo () = 0;

	/// get multiwordforms
	virtual const CSphMultiformContainer *  GetMultiWordforms () const = 0;
};


/// dictionary factory
CSphDict * sphCreateDictionaryCRC ( const CSphDictSettings & tSettings, ISphTokenizer * pTokenizer, CSphString & sError );

/// clear wordform cache
void sphShutdownWordforms ();

/////////////////////////////////////////////////////////////////////////////
// DATASOURCES
/////////////////////////////////////////////////////////////////////////////

/// hit info
struct CSphWordHit
{
	SphDocID_t		m_iDocID;		///< document ID
	SphWordID_t		m_iWordID;		///< word ID in current dictionary
	DWORD			m_iWordPos;		///< word position in current document
};

/// attribute locator within the row
struct CSphAttrLocator
{
	int				m_iBitOffset;
	int				m_iBitCount;

	CSphAttrLocator ()
		: m_iBitOffset ( -1 )
		, m_iBitCount ( -1 )
	{}

	inline bool IsBitfield () const
	{
		return ( m_iBitCount<ROWITEM_BITS || ( m_iBitOffset%ROWITEM_BITS )!=0 );
	}

	int CalcRowitem () const
	{
		return IsBitfield() ? - 1 : ( m_iBitOffset / ROWITEM_BITS );
	}
};


/// getter
inline SphAttr_t sphGetRowAttr ( const CSphRowitem * pRow, const CSphAttrLocator & tLoc )
{
	int iItem = tLoc.m_iBitOffset>>ROWITEM_SHIFT;

	if ( tLoc.m_iBitCount==ROWITEM_BITS )
		return pRow[iItem];

	if ( tLoc.m_iBitCount==2*ROWITEM_BITS ) // FIXME? write a generalized version, perhaps
		return SphAttr_t(pRow[iItem]) + ( SphAttr_t(pRow[iItem+1])<<ROWITEM_BITS );

	int iShift = tLoc.m_iBitOffset & ((1<<ROWITEM_SHIFT)-1);
	return ( pRow[iItem]>>iShift ) & ( (1UL<<tLoc.m_iBitCount)-1 );
}


/// setter
inline void sphSetRowAttr ( CSphRowitem * pRow, const CSphAttrLocator & tLoc, SphAttr_t uValue )
{
	int iItem = tLoc.m_iBitOffset>>ROWITEM_SHIFT;
	if ( tLoc.m_iBitCount==2*ROWITEM_BITS )
	{
		// FIXME? write a generalized version, perhaps
		pRow[iItem] = CSphRowitem ( uValue & ( (SphAttr_t(1)<<ROWITEM_BITS)-1 ) );
		pRow[iItem+1] = CSphRowitem ( uValue >> ROWITEM_BITS );

	} else if ( tLoc.m_iBitCount==ROWITEM_BITS )
	{
		pRow[iItem] = CSphRowitem ( uValue );

	} else
	{
		int iShift = tLoc.m_iBitOffset & ((1<<ROWITEM_SHIFT)-1);
		CSphRowitem uMask = ( (1UL<<tLoc.m_iBitCount)-1 ) << iShift;
		pRow[iItem] &= ~uMask;
		pRow[iItem] |= ( uMask & (uValue<<iShift) );
	}
}


/// document info
struct CSphDocInfo
{
	SphDocID_t		m_iDocID;		///< document ID
	int				m_iRowitems;	///< row items count
	CSphRowitem *	m_pRowitems;	///< row data

	/// ctor. clears everything
	CSphDocInfo ()
		: m_iDocID ( 0 )
		, m_iRowitems ( 0 )
		, m_pRowitems ( NULL )
	{
	}

	/// copy ctor. just in case
	CSphDocInfo ( const CSphDocInfo & rhs )
		: m_iRowitems ( 0 )
		, m_pRowitems ( NULL )
	{
		*this = rhs;
	}

	/// dtor. frees everything
	~CSphDocInfo ()
	{
		SafeDeleteArray ( m_pRowitems );
	}

	/// reset
	void Reset ( int iNewRowitems )
	{
		m_iDocID = 0;
		if ( iNewRowitems!=m_iRowitems )
		{
			m_iRowitems = iNewRowitems;
			SafeDeleteArray ( m_pRowitems );
			if ( m_iRowitems )
				m_pRowitems = new CSphRowitem [ m_iRowitems ];
		}
	}

	/// assignment
	const CSphDocInfo & operator = ( const CSphDocInfo & rhs )
	{
		m_iDocID = rhs.m_iDocID;

		if ( m_iRowitems!=rhs.m_iRowitems )
		{
			SafeDeleteArray ( m_pRowitems );
			m_iRowitems = rhs.m_iRowitems;
			if ( m_iRowitems )
				m_pRowitems = new CSphRowitem [ m_iRowitems ]; // OPTIMIZE! pool these allocs
		}

		if ( m_iRowitems )
		{
			assert ( m_iRowitems==rhs.m_iRowitems );
			memcpy ( m_pRowitems, rhs.m_pRowitems, sizeof(CSphRowitem)*m_iRowitems );
		}

		return *this;
	}

public:
	SphAttr_t	GetAttr ( const CSphAttrLocator & tLoc ) const				{ return sphGetRowAttr ( m_pRowitems, tLoc ); }
	float		GetAttrFloat ( const CSphAttrLocator & tLoc ) const			{ return sphDW2F ( (DWORD)sphGetRowAttr ( m_pRowitems, tLoc ) ); };

	void		SetAttr ( const CSphAttrLocator & tLoc, SphAttr_t uValue )	{ sphSetRowAttr ( m_pRowitems, tLoc, uValue ); }
	void		SetAttrFloat ( const CSphAttrLocator & tLoc, float fValue )	{ sphSetRowAttr ( m_pRowitems, tLoc, sphF2DW ( fValue ) ); }

	const DWORD *	GetAttrMVA ( const CSphAttrLocator & tLoc, const DWORD * pPool ) const;
};


/// source statistics
struct CSphSourceStats
{
	/// how much documents
	int				m_iTotalDocuments;

	/// how much bytes
	SphOffset_t		m_iTotalBytes;

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


/// known attribute types
enum
{
	SPH_ATTR_NONE		= 0,			///< not an attribute at all
	SPH_ATTR_INTEGER	= 1,			///< unsigned 32-bit integer
	SPH_ATTR_TIMESTAMP	= 2,			///< this attr is a timestamp
	SPH_ATTR_ORDINAL	= 3,			///< this attr is an ordinal string number (integer at search time, specially handled at indexing time)
	SPH_ATTR_BOOL		= 4,			///< this attr is a boolean bit field
	SPH_ATTR_FLOAT		= 5,			///< floating point number (IEEE 32-bit)
	SPH_ATTR_BIGINT		= 6,			///< signed 64-bit integer

	SPH_ATTR_MULTI		= 0x40000000UL	///< this attr has multiple values (0 or more)
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


/// source column info
struct CSphColumnInfo
{
	CSphString		m_sName;		///< column name
	DWORD			m_eAttrType;	///< attribute type
	ESphWordpart	m_eWordpart;	///< wordpart processing type

	int				m_iIndex;		///< index into source result set
	CSphAttrLocator	m_tLocator;		///< attribute locator in the row

	ESphAttrSrc		m_eSrc;			///< attr source (for multi-valued attrs only)
	CSphString		m_sQuery;		///< query to retrieve values (for multi-valued attrs only)
	CSphString		m_sQueryRange;	///< query to retrieve range (for multi-valued attrs only)

	CSphRefcountedPtr<ISphExpr>		m_pExpr;		///< evaluator for expression items
	bool							m_bLateCalc;	///< early calc or late calc

	/// handy ctor
	CSphColumnInfo ( const char * sName=NULL, DWORD eType=SPH_ATTR_NONE )
		: m_sName ( sName )
		, m_eAttrType ( eType )
		, m_eWordpart ( SPH_WORDPART_WHOLE )
		, m_iIndex ( -1 )
		, m_eSrc ( SPH_ATTRSRC_NONE )
		, m_pExpr ( NULL )
		, m_bLateCalc ( false )
	{
		m_sName.ToLower ();
	}

	/// equality comparison checks name, type, and locator
	bool operator == ( const CSphColumnInfo & rhs ) const
	{
		return m_sName==rhs.m_sName && m_eAttrType==rhs.m_eAttrType && m_tLocator.m_iBitCount==rhs.m_tLocator.m_iBitCount && m_tLocator.m_iBitOffset==rhs.m_tLocator.m_iBitOffset;
	}
};


/// source schema
class CSphQuery;
struct CSphSchema
{
	CSphString						m_sName;		///< my human-readable name
	CSphVector<CSphColumnInfo>		m_dFields;		///< my fulltext-searchable fields

public:

	/// ctor
							CSphSchema ( const char * sName="(nameless)" ) : m_sName ( sName ) {}

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

	/// get row size
	int						GetRowSize () const				{ return m_dRowUsed.GetLength(); }

	/// get attrs count
	int						GetAttrsCount () const			{ return m_dAttrs.GetLength(); }

	/// get attr
	const CSphColumnInfo &	GetAttr ( int iIndex ) const	{ return m_dAttrs[iIndex]; }

	/// add attr
	void					AddAttr ( const CSphColumnInfo & tAttr );

protected:
	CSphVector<CSphColumnInfo>		m_dAttrs;		///< all my attributes
	CSphVector<int>					m_dRowUsed;		///< row map (amount of used bits in each rowitem)
};


/// HTML stripper
class CSphHTMLStripper
{
public:
								CSphHTMLStripper ();
	bool						SetIndexedAttrs ( const char * sConfig, CSphString & sError );
	bool						SetRemovedElements ( const char * sConfig, CSphString & sError );
	void						Strip ( BYTE * sData );

protected:
	struct StripperTag_t
	{
		CSphString				m_sTag;			///< tag name
		int						m_iTagLen;		///< tag name length
		bool					m_bInline;		///< whether this tag is inline
		bool					m_bIndexAttrs;	///< whether to index attrs
		bool					m_bRemove;		///< whethet to remove contents
		CSphVector<CSphString>	m_dAttrs;		///< attr names to index

		StripperTag_t ()
			: m_iTagLen ( 0 )
			, m_bInline ( false )
			, m_bIndexAttrs ( false )
			, m_bRemove ( false )
		{}

		inline bool operator < ( const StripperTag_t & rhs ) const
		{
			return strcmp ( m_sTag.cstr(), rhs.m_sTag.cstr() )<0; 
		}
	};

protected:
	static const int			MAX_CHAR_INDEX = 28;		///< max valid char index (a-z, underscore, colon)

	CSphVector<StripperTag_t>	m_dTags;					///< known tags to index attrs and/or to remove contents
	int							m_dStart[MAX_CHAR_INDEX];	///< maps index of the first tag name char to start offset in m_dTags
	int							m_dEnd[MAX_CHAR_INDEX];		///< maps index of the first tag name char to end offset in m_dTags

protected:
	int							GetCharIndex ( int iCh );	///< calcs index by raw char
	void						UpdateTags ();				///< sorts tags, updates internal helpers
};


/// generic data source
class CSphSource
{
public:
	CSphVector<CSphWordHit>				m_dHits;	///< current document split into words
	CSphDocInfo							m_tDocInfo;	///< current document info
	CSphVector<CSphString>				m_dStrAttrs;///< current document string attrs

public:
	/// ctor
										CSphSource ( const char * sName );

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
	bool								SetStripHTML ( const char * sExtractAttrs, const char * sRemoveElements, CSphString & sError );

	/// set tokenizer
	void								SetTokenizer ( ISphTokenizer * pTokenizer );

	/// get stats
	virtual const CSphSourceStats &		GetStats ();

	/// updates schema fields and attributes
	/// updates pInfo if it's empty; checks for match if it's not
	/// must be called after IterateHitsStart(); will always fail otherwise
	virtual bool						UpdateSchema ( CSphSchema * pInfo, CSphString & sError );

	/// configure source to emit prefixes or infixes
	/// passing zero to both iMinPrefixLen and iMinInfixLen means to emit the words themselves
	void								SetEmitInfixes ( int iMinPrefixLen, int iMinInfixLen );

	/// set boundary step
	void								SetBoundaryStep ( int iBoundaryStep ) { m_iBoundaryStep = Max ( iBoundaryStep, 0 ); }

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

	/// begin iterating document hits
	/// to be implemented by descendants
	virtual bool						IterateHitsStart ( CSphString & sError ) = 0;

	/// get next document hit
	/// to be implemented by descendants
	/// on next document, returns true and m_tDocInfo.m_uDocID is not 0
	/// on end of documents, returns true and m_tDocInfo.m_uDocID is 0
	/// on error, returns false and fills sError
	virtual bool						IterateHitsNext ( CSphString & sError ) = 0;

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

	/// setup field match mode
	virtual void						SetupFieldMatch ( const char *, const char * ) {}

protected:
	ISphTokenizer *						m_pTokenizer;	///< my tokenizer
	CSphDict *							m_pDict;		///< my dict
	
	CSphSourceStats						m_tStats;		///< my stats
	CSphSchema							m_tSchema;		///< my schema

	bool								m_bStripHTML;	///< whether to strip HTML
	CSphHTMLStripper *					m_pStripper;	///< my HTML stripper

	int									m_iMinPrefixLen;///< min indexable prefix (0 means don't index prefixes)
	int									m_iMinInfixLen;	///< min indexable infix length (0 means don't index infixes)
	int									m_iBoundaryStep;///< additional boundary word position increment
};


/// generic document source
/// provides multi-field support and generic tokenizer
class CSphSource_Document : public CSphSource
{
public:
	/// ctor
							CSphSource_Document ( const char * sName ) : CSphSource ( sName ) {}

	/// my generic tokenizer
	virtual bool			IterateHitsNext ( CSphString & sError );

	/// field data getter
	/// to be implemented by descendants
	virtual BYTE **			NextDocument ( CSphString & sError ) = 0;

	virtual void			SetupFieldMatch ( const char * szPrefixFields, const char * szInfixFields );

protected:
	bool					IsPrefixMatch ( const char * szField ) const;
	bool					IsInfixMatch ( const char * szField ) const;

	void					ParseFieldMVA ( CSphVector < CSphVector < DWORD > >  & dFieldMVAs, int iFieldMVA, const char * szValue );

private:
	CSphString				m_sPrefixFields;
	CSphString				m_sInfixFields;

	bool					IsFieldInStr ( const char * szField, const char * szString ) const;
};


/// generic SQL source params
struct CSphSourceParams_SQL
{
	// query params
	CSphString						m_sQuery;
	CSphString						m_sQueryRange;
	CSphString						m_sQueryKilllist;
	int								m_iRangeStep;

	CSphVector<CSphString>			m_dQueryPre;
	CSphVector<CSphString>			m_dQueryPost;
	CSphVector<CSphString>			m_dQueryPostIndex;
	CSphVector<CSphColumnInfo>		m_dAttrs;

	int								m_iRangedThrottle;

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
						CSphSource_SQL ( const char * sName );
	virtual				~CSphSource_SQL () {}

	bool				Setup ( const CSphSourceParams_SQL & pParams );
	virtual bool		Connect ( CSphString & sError );
	virtual void		Disconnect ();

	virtual bool		IterateHitsStart ( CSphString & sError );
	virtual BYTE **		NextDocument ( CSphString & sError );
	virtual void		PostIndex ();

	virtual bool		HasAttrsConfigured () { return m_tParams.m_dAttrs.GetLength()!=0; }

	virtual bool		IterateMultivaluedStart ( int iAttr, CSphString & sError );
	virtual bool		IterateMultivaluedNext ();

	virtual bool		IterateFieldMVAStart ( int iAttr, CSphString & sError );
	virtual bool		IterateFieldMVANext ();

	virtual bool		IterateKillListStart ( CSphString & sError );
	virtual bool		IterateKillListNext ( SphDocID_t & tDocId );

private:
	bool				m_bSqlConnected;///< am i connected?

protected:
	CSphString			m_sSqlDSN;

	BYTE *				m_dFields [ SPH_MAX_FIELDS ];

	SphDocID_t			m_uMinID;		///< grand min ID
	SphDocID_t			m_uMaxID;		///< grand max ID
	SphDocID_t			m_uCurrentID;	///< current min ID
	SphDocID_t			m_uMaxFetchedID;///< max actually fetched ID
	int					m_iMultiAttr;	///< multi-valued attr being currently fetched

	int					m_iFieldMVA;
	int					m_iFieldMVAIterator;
	CSphVector < CSphVector <DWORD> > m_dFieldMVAs;
	CSphVector < int >	m_dAttrToFieldMVA;

	CSphSourceParams_SQL		m_tParams;

	bool				m_bWarnedNull;
	bool				m_bWarnedMax;

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
	virtual const char *	SqlColumn ( int iIndex ) = 0;
	virtual const char *	SqlFieldName ( int iIndex ) = 0;
};


#if USE_MYSQL
/// MySQL source params
struct CSphSourceParams_MySQL : CSphSourceParams_SQL
{
	CSphString	m_sUsock;					///< UNIX socket
	int			m_iFlags;					///< connection flags

				CSphSourceParams_MySQL ();	///< ctor. sets defaults
};


/// MySQL source implementation
/// multi-field plain-text documents fetched from given query
struct CSphSource_MySQL : CSphSource_SQL
{
							CSphSource_MySQL ( const char * sName );
	bool					Setup ( const CSphSourceParams_MySQL & tParams );

protected:
	MYSQL_RES *				m_pMysqlResult;
	MYSQL_FIELD *			m_pMysqlFields;
	MYSQL_ROW				m_tMysqlRow;
	MYSQL					m_tMysqlDriver;

	CSphString				m_sMysqlUsock;
	int						m_iMysqlConnectFlags;

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
							CSphSource_PgSQL ( const char * sName );
	bool					Setup ( const CSphSourceParams_PgSQL & pParams );
	virtual bool			IterateHitsStart ( CSphString & sError );

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
	virtual const char *	SqlColumn ( int iIndex );
	virtual const char *	SqlFieldName ( int iIndex );
};
#endif // USE_PGSQL


/// XML pipe source implementation
class CSphSource_XMLPipe : public CSphSource
{
public:
					CSphSource_XMLPipe ( BYTE * dInitialBuf, int iBufLen, const char * sName );	///< ctor
					~CSphSource_XMLPipe ();						///< dtor

	bool			Setup ( FILE * pPipe, const char * sCommand );			///< memorize the command
	virtual bool	Connect ( CSphString & sError );			///< run the command and open the pipe
	virtual void	Disconnect ();								///< close the pipe

	virtual bool	IterateHitsStart ( CSphString & ) { return true; }	///< Connect() starts getting documents automatically, so this one is empty
	virtual bool	IterateHitsNext ( CSphString & sError );			///< parse incoming chunk and emit some hits

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
};


#if USE_LIBEXPAT || USE_LIBXML

class CSphConfigSection;
CSphSource * sphCreateSourceXmlpipe2 ( const CSphConfigSection * pSource, FILE * pPipe, BYTE * dInitialBuf, int iBufLen, const char * szSourceName );

#endif

FILE * sphDetectXMLPipe ( const char * szCommand, BYTE * dBuf, int & iBufSize, int iMaxBufSize, bool & bUsePipe2 );


/////////////////////////////////////////////////////////////////////////////
// SEARCH QUERIES
/////////////////////////////////////////////////////////////////////////////

/// search query match
struct CSphMatch : public CSphDocInfo
{
	int		m_iWeight;	///< my computed weight
	int		m_iTag;		///< my index tag

	CSphMatch () : m_iWeight ( 0 ), m_iTag ( 0 ) {}
	bool operator == ( const CSphMatch & rhs ) const { return ( m_iDocID==rhs.m_iDocID ); }
};


/// specialized swapper
inline void Swap ( CSphMatch & a, CSphMatch & b )
{
	Swap ( a.m_iDocID, b.m_iDocID );
	Swap ( a.m_iRowitems, b.m_iRowitems );
	Swap ( a.m_pRowitems, b.m_pRowitems );
	Swap ( a.m_iWeight, b.m_iWeight );
	Swap ( a.m_iTag, b.m_iTag );
}


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
	SPH_RANK_PROXIMITY_BM25		= 0,	///< default mode, phrase proximity major factor and BM25 minor one
	SPH_RANK_BM25				= 1,	///< statistical mode, BM25 ranking only (faster but worse quality)
	SPH_RANK_NONE				= 2,	///< no ranking, all matches get a weight of 1
	SPH_RANK_WORDCOUNT			= 3,	///< simple word-count weighting, rank is a weighted sum of per-field keyword occurence counts
	SPH_RANK_PROXIMITY			= 4,	///< phrase proximity
	SPH_RANK_MATCHANY			= 5,	///< emulate old match-any weighting

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
class CSphFilter
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
	bool				m_bMva;			///< whether this filter is against multi-valued attribute
	CSphAttrLocator		m_tLocator;		///< attr locator

public:
						CSphFilter ();

	bool				Setup ( CSphSchema * pSchema );
	bool				IsValid () const;

	/// returns true all rows within given bounds are guaranteed to match not
	/// returns false otherwise, if some rows can possibly match
	bool				CheckBoundsReject ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo, int iSchemaSize ) const;

	void				SetExternalValues ( const SphAttr_t * pValues, int nValues );

	inline int					GetAttrType () const		{ return m_iAttrType; }
	inline SphAttr_t			GetValue ( int iIdx ) const	{ assert ( iIdx<GetNumValues() ); return m_pValues ? m_pValues[iIdx] : m_dValues[iIdx]; }
	inline const SphAttr_t *	GetValueArray () const		{ return m_pValues ? m_pValues : &(m_dValues [0]); }
	inline int					GetNumValues () const		{ return m_pValues ? m_nValues : m_dValues.GetLength (); }

	bool				operator == ( const CSphFilter & rhs ) const;
	bool				operator != ( const CSphFilter & rhs ) const { return !( (*this)==rhs ); }

protected:
	int					m_iAttrType;	///< filter attr type
	const SphAttr_t *	m_pValues;		///< external value array
	int					m_nValues;		///< external array size

						CSphFilter ( const CSphFilter & rhs );
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
	DWORD						m_uAttrType;	///< attribute type
	CSphAttrLocator				m_tLocator;		///< attribute locator
	CSphVector<IdValuePair_t>	m_dValues;		///< id-value overrides
};


/// query selection item
struct CSphQueryItem
{
	CSphString		m_sExpr;		///< expression to compute
	CSphString		m_sAlias;		///< alias to return
};


/// search query
class CSphQuery
{
public:
	CSphString		m_sIndexes;		///< indexes to search
	CSphString		m_sQuery;		///< query string

	int				m_iOffset;		///< offset into result set (as X in MySQL LIMIT X,Y clause)
	int				m_iLimit;		///< limit into result set (as Y in MySQL LIMIT X,Y clause)
	DWORD *			m_pWeights;		///< user-supplied per-field weights. may be NULL. default is NULL. NOT OWNED, WILL NOT BE FREED in dtor.
	int				m_iWeights;		///< number of user-supplied weights. missing fields will be assigned weight 1. default is 0
	ESphMatchMode	m_eMode;		///< match mode. default is "match all"
	ESphRankMode	m_eRanker;		///< ranking mode, default is proximity+BM25
	ESphSortOrder	m_eSort;		///< sort mode
	CSphString		m_sSortBy;		///< attribute to sort by
	int				m_iMaxMatches;	///< max matches to retrieve, default is 1000. more matches use more memory and CPU time to hold and sort them

	SphDocID_t		m_iMinID;		///< min ID to match, 0 by default
	SphDocID_t		m_iMaxID;		///< max ID to match, UINT_MAX by default

	CSphVector<CSphFilter>	m_dFilters;	///< filters

	CSphString		m_sGroupBy;		///< group-by attribute name
	ESphGroupBy		m_eGroupFunc;	///< function to pre-process group-by attribute value with
	CSphString		m_sGroupSortBy;	///< sorting clause for groups in group-by mode
	CSphString		m_sGroupDistinct;///< count distinct values for this attribute

	int				m_iCutoff;		///< matches count threshold to stop searching at (defualt is 0; means to search until all matches are found)

	int				m_iRetryCount;	///< retry count, for distributed queries
	int				m_iRetryDelay;	///< retry delay, for distributed queries

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

public:
	int				m_iOldVersion;		///< version, to fixup old queries
	int				m_iOldGroups;		///< 0.9.6 group filter values count
	DWORD *			m_pOldGroups;		///< 0.9.6 group filter values
	DWORD			m_iOldMinTS;		///< 0.9.6 min timestamp
	DWORD			m_iOldMaxTS;		///< 0.9.6 max timestamp
	DWORD			m_iOldMinGID;		///< 0.9.6 min group id
	DWORD			m_iOldMaxGID;		///< 0.9.6 max group id

public:
	CSphVector<CSphQueryItem>		m_dItems;	///< parsed select-list

public:
					CSphQuery ();		///< ctor, fills defaults
					~CSphQuery ();		///< dtor, frees owned stuff

	/// return index weight from m_dIndexWeights; or 1 by default
	int				GetIndexWeight ( const char * sName ) const;

	/// parse select list string into items
	bool			ParseSelectList ( CSphString & sError );
};


/// search query result
class CSphQueryResult
{
public:
	struct WordStat_t
	{
		CSphString			m_sWord;	///< i-th search term (normalized word form)
		int					m_iDocs;	///< document count for this term
		int					m_iHits;	///< hit count for this term
	}						m_tWordStats [ SPH_MAX_QUERY_WORDS ];

	int						m_iNumWords;		///< query word count
	int						m_iQueryTime;		///< query time, ms
	CSphVector<CSphMatch>	m_dMatches;			///< top matching documents, no more than MAX_MATCHES
	int						m_iTotalMatches;	///< total matches count

	CSphSchema				m_tSchema;			///< result schema
	const DWORD *			m_pMva;				///< pointer to MVA storage

	CSphString				m_sError;			///< error message
	CSphString				m_sWarning;			///< warning message

	int						m_iOffset;			///< requested offset into matches array
	int						m_iCount;			///< count which will be actually served (computed from total, offset and limit)

	int						m_iSuccesses;

public:
							CSphQueryResult ();		///< ctor
	virtual					~CSphQueryResult ();	///< dtor, which releases all owned stuff
};

/////////////////////////////////////////////////////////////////////////////
// ATTRIBUTE UPDATE QUERY
/////////////////////////////////////////////////////////////////////////////

struct CSphAttrUpdate
{
	CSphVector<CSphColumnInfo>		m_dAttrs;		///< update schema (ie. what attrs to update)
	CSphVector<DWORD>				m_dPool;		///< update values pool
	CSphVector<SphDocID_t>			m_dDocids;		///< document IDs vector
	CSphVector<int>					m_dRowOffset;	///< document row offsets in the pool
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
		PHASE_MERGE					///< index merging phase
	};

	Phase_e			m_ePhase;		///< current indexing phase

	int				m_iDocuments;	///< PHASE_COLLECT: documents collected so far
	SphOffset_t		m_iBytes;		///< PHASE_COLLECT: bytes collected so far

	int64_t			m_iAttrs;		///< PHASE_COLLECT_MVA, PHASE_SORT_MVA: attrs processed so far
	int64_t			m_iAttrsTotal;	///< PHASE_SORT_MVA: attrs total

	SphOffset_t		m_iHits;		///< PHASE_SORT: hits sorted so far
	SphOffset_t		m_iHitsTotal;	///< PHASE_SORT: hits total

	int				m_iWords;		///< PHASE_MERGE: words merged so far

	CSphIndexProgress ()
		: m_ePhase ( PHASE_COLLECT )
		, m_iDocuments ( 0 )
		, m_iBytes ( 0 )
		, m_iAttrs ( 0 )
		, m_iAttrsTotal ( 0 )
		, m_iHits ( 0 )
		, m_iHitsTotal ( 0 )
		, m_iWords ( 0 )
	{}
};


/// match comparator state
struct CSphMatchComparatorState
{
	static const int	MAX_ATTRS = 5;

	int					m_iAttr[MAX_ATTRS];			///< sort-by attr index
	CSphAttrLocator		m_tLocator[MAX_ATTRS];		///< sort-by attr locator 

	DWORD				m_uAttrDesc;				///< sort order mask (if i-th bit is set, i-th attr order is DESC)
	DWORD				m_iNow;						///< timestamp (for timesegments sorting mode)

	/// create default empty state
	CSphMatchComparatorState ()
		: m_uAttrDesc ( 0 )
		, m_iNow ( 0 )
	{
		for ( int i=0; i<MAX_ATTRS; i++ )
			m_iAttr[i] = -1;
	}

	/// check if any of my attrs are bitfields
	bool UsesBitfields ()
	{
		for ( int i=0; i<MAX_ATTRS; i++ )
			if ( m_iAttr[i]>=0 )
				if ( m_tLocator[i].IsBitfield() )
					return true;
		return false;
	}
};


/// generic match sorter interface
class ISphMatchSorter
{
public:
	bool				m_bRandomize;
	int					m_iTotal;
	CSphSchema			m_tIncomingSchema;		///< incoming schema (adds computed attributes on top of index schema)
	CSphSchema			m_tOutgoingSchema;		///< outgoing schema (adds @groupby etc if needed on top of incoming)

public:
	/// ctor
						ISphMatchSorter () : m_bRandomize ( false ), m_iTotal ( 0 ) {}

	/// virtualizing dtor
	virtual				~ISphMatchSorter () {}

	/// check if this sorter needs attr values
	virtual bool		UsesAttrs () = 0;

	/// check if this sorter does groupby
	virtual bool		IsGroupby () = 0;

	/// set match comparator state
	virtual void		SetState ( const CSphMatchComparatorState & ) = 0;

	/// set group comparator state
	virtual void		SetGroupState ( const CSphMatchComparatorState & ) {}

	/// set MVA pool pointer (for MVA+groupby sorters)
	virtual void		SetMVAPool ( const DWORD * ) {}

	/// base push
	/// returns false if the entry was rejected as duplicate
	/// returns true otherwise (even if it was not actually inserted)
	virtual bool		Push ( const CSphMatch & tEntry ) = 0;

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


struct CSphIndexSettings
{
	ESphDocinfo		m_eDocinfo;
	int				m_iMinPrefixLen;
	int				m_iMinInfixLen;
	bool			m_bHtmlStrip;
	CSphString		m_sHtmlIndexAttrs;
	CSphString		m_sHtmlRemoveElements;
					
					CSphIndexSettings ();
};


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
								CSphIndex ( const char * sName );
	virtual						~CSphIndex ();

	virtual const CSphString &	GetLastError () const { return m_sLastError; }
	virtual const CSphSchema *	GetSchema () const { return &m_tSchema; }

	virtual	void				SetProgressCallback ( ProgressCallback_t * pfnProgress ) { m_pProgress = pfnProgress; }
	virtual void				SetBoundaryStep ( int iBoundaryStep );
	virtual void				SetInplaceSettings ( int iHitGap, int iDocinfoGap, float fRelocFactor, float fWriteFactor );
	virtual void				SetStar ( bool bValue ) { m_bEnableStar = bValue; }
	virtual bool				GetStar () const { return m_bEnableStar; }
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

public:
	/// build index by indexing given sources
	virtual int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit ) = 0;

	/// build index by mering current index with given index
	virtual bool				Merge ( CSphIndex * pSource, CSphVector<CSphFilter> & dFilters, bool bMergeKillLists ) = 0;

public:
	/// dump human-readable header info to given file
	virtual void				DumpHeader ( FILE * fp, const char * sHeaderName ) = 0;

	/// check all data files, preload schema, and preallocate enough shared RAM to load memory-cached data
	virtual const CSphSchema *	Prealloc ( bool bMlock, CSphString & sWarning ) = 0;

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

public:
	virtual CSphQueryResult *	Query ( CSphQuery * pQuery ) = 0;
	virtual bool				QueryEx ( CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchSorter * pTop ) = 0;
	virtual bool				MultiQuery ( CSphQuery * pQuery, CSphQueryResult * pResult, int iSorters, ISphMatchSorter ** ppSorters ) = 0;
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, bool bGetStats ) = 0;

public:
	/// updates memory-cached attributes in real time
	/// returns non-negative amount of actually found and updated records on success
	/// on failure, -1 is returned and GetLastError() contains error message
	virtual int					UpdateAttributes ( const CSphAttrUpdate & tUpd ) = 0;

	/// saves memory-cached attributes, if there were any updates to them
	/// on failure, false is returned and GetLastError() contains error message
	virtual bool				SaveAttributes () = 0;

public:
	DWORD						m_uAttrsStatus;			///< whether in-memory attrs are updated (compared to disk state)

protected:
	ProgressCallback_t *		m_pProgress;
	CSphSchema					m_tSchema;
	CSphString					m_sLastError;

	int							m_iBoundaryStep;		///< on-boundary additional word position step (0 means index all words continuously)

	bool						m_bInplaceSettings;
	int							m_iHitGap;
	int							m_iDocinfoGap;
	float						m_fRelocFactor;
	float						m_fWriteFactor;

	bool						m_bEnableStar;			///< enable star-syntax
	bool						m_bKeepFilesOpen;		///< keep files open to avoid race on seamless rotation
	bool						m_bPreloadWordlist;		///< preload wordlists or keep them on disk

	bool						m_bStripperInited;		///< was stripper initialized (old index version (<9) handling)
	CSphIndexSettings			m_tSettings;

	ISphTokenizer *				m_pTokenizer;
	CSphDict *					m_pDict;
};

/////////////////////////////////////////////////////////////////////////////

/// create phrase fulltext index implemntation
CSphIndex *			sphCreateIndexPhrase ( const char * sFilename );

/// tell libsphinx to be quiet or not (logs and loglevels to come later)
void				sphSetQuiet ( bool bQuiet );

/// creates proper queue for given query
/// may return NULL on error; in this case, error message is placed in sError
ISphMatchSorter *	sphCreateQueue ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sError );

/// convert queue to sorted array, and add its entries to result's matches array
void				sphFlattenQueue ( ISphMatchSorter * pQueue, CSphQueryResult * pResult, int iTag );

/////////////////////////////////////////////////////////////////////////////

/// callback type
typedef void		(*SphErrorCallback_fn) ( const char * );

/// register application-level internal error callback
void				sphSetInternalErrorCallback ( SphErrorCallback_fn fnCallback );

#endif // _sphinx_

/////////////////////////////////////////////////////////////////////////////

/// workaround to suppress C4511/C4512 warnings (copy ctor and assignment operator) in VS 2003
#if _MSC_VER>=1300 && _MSC_VER<1400
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#endif

//
// $Id$
//
