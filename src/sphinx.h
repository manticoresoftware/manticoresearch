//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
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
	#define USE_MYSQL		1	/// whether to compile with MySQL support
	#define USE_WINDOWS		1	/// whether to compile for Windows
#else
	#define USE_WINDOWS		0	/// whether to compile for Windows
#endif

/////////////////////////////////////////////////////////////////////////////

#include "sphinxstd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	#define strcasecmp			strcmpi
	#define strncasecmp			_strnicmp
	#define snprintf			_snprintf

	typedef __int64				SphOffset_t;
	#define STDOUT_FILENO		fileno(stdout)
#else
	typedef unsigned int		DWORD;
	typedef unsigned short		WORD;
	typedef unsigned char		BYTE;
	typedef off_t				SphOffset_t;
#endif

/////////////////////////////////////////////////////////////////////////////

#define SPHINX_VERSION			"0.9.7-RC2"
#define SPHINX_BANNER			"Sphinx " SPHINX_VERSION "\nCopyright (c) 2001-2006, Andrew Aksyonoff\n\n"
#define SPHINX_SEARCHD_PROTO	1

#define SPH_MAX_QUERY_WORDS		10
#define SPH_MAX_WORD_LEN		64
#define SPH_MAX_FILENAME_LEN	512
#define SPH_MAX_FIELDS			32

#define SPH_CACHE_WRITE			1048576

/////////////////////////////////////////////////////////////////////////////

/// crash with an error message
void			sphDie ( char * sMessage, ... );

/// time, in seconds
float			sphLongTimer ();

/// Sphinx CRC32 implementation
DWORD			sphCRC32 ( const BYTE * pString );

/// replaces all occurences of sMacro in sTemplate with textual representation of iValue
char *			sphStrMacro ( const char * sTemplate, const char * sMacro, int iValue );

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

	void		SetRemap ( const CSphRemapRange * pRemaps, int iRemaps );
	void		SetRemap ( const CSphLowercaser * pLC );
	bool		SetRemap ( const char * sConfig );
	void		AddSpecials ( const char * sSpecials );

public:
	const CSphLowercaser &		operator = ( const CSphLowercaser & rhs );

public:
	inline int	ToLower ( int iCode )
	{
		assert ( iCode>=0 );
		if ( iCode>=MAX_CODE )
			return 0;
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

/// generic tokenizer
class ISphTokenizer
{
public:
	/// trivial ctor
									ISphTokenizer() : m_iMinWordLen ( 1 ), m_iLastTokenLen ( 0 ) {}

	/// trivial dtor
	virtual							~ISphTokenizer () {}

	/// set new translation table
	/// returns true on success, false on failure
	virtual bool					SetCaseFolding ( const char * sConfig );

	/// add special chars to translation table (SBCS only, for now)
	/// updates lowercaser so that these remap to -1
	virtual void					AddSpecials ( const char * sSpecials );

	/// get lowercaser
	virtual const CSphLowercaser *	GetLowercaser () const { return &m_tLC; }

	/// set min word length
	virtual void					SetMinWordLen ( int iLen ) { m_iMinWordLen = Max ( iLen, 1 ); }

	/// get last token length, in codepoints
	int								GetLastTokenLen () { return m_iLastTokenLen; }

public:
	/// pass next buffer
	virtual void					SetBuffer ( BYTE * sBuffer, int iLength, bool bLast ) = 0;

	/// get next token
	virtual BYTE *					GetToken () = 0;

	/// spawn a clone of my own
	virtual ISphTokenizer *			Clone () const = 0;

	/// SBCS or UTF-8?
	virtual bool					IsUtf8 () const = 0;

	/// calc codepoint length
	virtual int						GetCodepointLength ( int iCode ) const = 0;

protected:
	CSphLowercaser		m_tLC;			///< my lowercaser
	int					m_iMinWordLen;	///< minimal word length, in codepoints
	int					m_iLastTokenLen;///< last token length, in codepoints
};

/// create SBCS tokenizer
ISphTokenizer *			sphCreateSBCSTokenizer ();

/// create UTF-8 tokenizer
ISphTokenizer *			sphCreateUTF8Tokenizer ();

/////////////////////////////////////////////////////////////////////////////
// DICTIONARIES
/////////////////////////////////////////////////////////////////////////////

/// morphology flags
enum ESphMorphology
{
	SPH_MORPH_NONE				= 0,
	SPH_MORPH_STEM_EN			= (1UL<<1),
	SPH_MORPH_STEM_RU_CP1251	= (1UL<<2),
	SPH_MORPH_STEM_RU_UTF8		= (1UL<<3),
	SPH_MORPH_SOUNDEX			= (1UL<<4),
	SPH_MORPH_UNKNOWN			= (1UL<<30)
};


/// abstract word dictionary interface
struct CSphDict
{
	/// virtualizing dtor
	virtual				~CSphDict () {}

	/// get word ID by word, "text" version
	/// returns 0 for stopwords
	virtual DWORD		GetWordID ( BYTE * pWord ) = 0;

	/// get word ID by word, "binary" version
	/// returns 0 for stopwords
	virtual DWORD		GetWordID ( const BYTE * pWord, int iLen ) = 0;

	/// load stopwords from given files
	virtual void		LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer ) = 0;
};


/// CRC32 dictionary
struct CSphDict_CRC32 : CSphDict
{
	/// ctor
	/// iMorph is a combination of ESphMorphology flags
						CSphDict_CRC32 ( DWORD iMorph );

	/// virtualizing dtor
	virtual				~CSphDict_CRC32 () {}

	/// get word ID by word
	/// does requested morphology and returns CRC32
	virtual DWORD		GetWordID ( BYTE * pWord );

	/// get word ID by word, "binary" version
	/// does NOT apply any morphology
	virtual DWORD		GetWordID ( const BYTE * pWord, int iLen );

	/// load stopwords from given files
	virtual void		LoadStopwords ( const char * sFiles, ISphTokenizer * pTokenizer );

protected:
	DWORD				m_iMorph;		///< morphology flags
	int					m_iStopwords;	///< stopwords count
	DWORD *				m_pStopwords;	///< stopwords ID list

	/// filter ID against stopwords list
	DWORD				FilterStopword ( DWORD uID );
};

/////////////////////////////////////////////////////////////////////////////
// DATASOURCES
/////////////////////////////////////////////////////////////////////////////

/// hit info
struct CSphWordHit
{
	DWORD	m_iDocID;		///< document ID
	DWORD	m_iWordID;		///< word ID in current dictionary
	DWORD	m_iWordPos;		///< word position in current document
};


/// document info
struct CSphDocInfo
{
	DWORD		m_iDocID;	///< document ID
	int			m_iAttrs;	///< attribute count (FIXME! invariant over index; stored for assignment operator)
	DWORD *		m_pAttrs;	///< attribute values

	/// ctor. clears everything
	CSphDocInfo ()
		: m_iDocID ( 0 )
		, m_iAttrs ( 0 )
		, m_pAttrs ( NULL )
	{
	}

	/// dtor. frees everything
	~CSphDocInfo ()
	{
		SafeDeleteArray ( m_pAttrs );
	}

	/// reset
	void Reset ( int iAttrs )
	{
		m_iDocID = 0;
		if ( iAttrs!=m_iAttrs )
		{
			m_iAttrs = iAttrs;
			SafeDeleteArray ( m_pAttrs );
			if ( m_iAttrs )
				m_pAttrs = new DWORD [ m_iAttrs ];
		}
	}

	/// assignment
	const CSphDocInfo & operator = ( const CSphDocInfo & rhs )
	{
		m_iDocID = rhs.m_iDocID;

		if ( m_iAttrs!=rhs.m_iAttrs )
		{
			SafeDeleteArray ( m_pAttrs );
			m_iAttrs = rhs.m_iAttrs;
			if ( m_iAttrs )
				m_pAttrs = new DWORD [ m_iAttrs ]; // OPTIMIZE! pool these allocs
		}

		if ( m_iAttrs )
		{
			assert ( m_iAttrs==rhs.m_iAttrs );
			memcpy ( m_pAttrs, rhs.m_pAttrs, sizeof(DWORD)*m_iAttrs );
		}

		return *this;
	}
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
enum ESphAttrType
{
	SPH_ATTR_NONE		= 0,	///< not an attribute at all
	SPH_ATTR_INTEGER	= 1,	///< this attr is just an integer
	SPH_ATTR_TIMESTAMP	= 2		///< this attr is a timestamp
};


/// source column info
struct CSphColumnInfo
{
	CSphString		m_sName;		///< column name
	ESphAttrType	m_eAttrType;	///< attribute type
	int				m_iIndex;		///< index into the result set

	/// handy ctor
	CSphColumnInfo ( const char * sName=NULL, ESphAttrType eType=SPH_ATTR_NONE )
		: m_sName ( sName )
		, m_eAttrType ( eType )
	{
		m_sName.ToLower ();
	}
};


/// schema comparison results
enum ESphSchemaCompare
{
	SPH_SCHEMAS_EQUAL			= 0,	///< these schemas are fully identical
	SPH_SCHEMAS_COMPATIBLE		= 1,	///< these schemas are compatible, ie. attribute types match
	SPH_SCHEMAS_INCOMPATIBLE	= 2		///< these schemas are not compatible
};

/// source schema
struct CSphSchema
{
	CSphString									m_sName;		///< my human-readable name
	CSphVector<CSphColumnInfo,SPH_MAX_FIELDS>	m_dFields;		///< my fulltext-searchable fields
	CSphVector<CSphColumnInfo,8>				m_dAttrs;		///< my per-document attributes

	/// ctor
						CSphSchema ( const char * sName ) : m_sName ( sName ) {}

	/// get field index by name
	/// returns -1 if not found
	int					GetFieldIndex ( const char * sName ) const;

	/// get attribute index by name
	/// returns -1 if not found
	int					GetAttrIndex ( const char * sName ) const;

	/// checks if two schemas match
	/// if result is not SPH_SCHEMAS_EQUAL, human-readable error/warning message is put to sError
	ESphSchemaCompare	CompareTo ( const CSphSchema & rhs, CSphString & sError ) const;

	/// return non-virtual attributes count
	int					GetRealAttrCount () const;
};


/// generic data source
class CSphHTMLStripper;
class CSphSource
{
public:
	CSphVector<CSphWordHit>				m_dHits;	///< current document split into words
	CSphDocInfo							m_tDocInfo;	///< current document info

public:
	/// ctor
										CSphSource ( const char * sName );

	/// dtor
	virtual								~CSphSource ();

	/// set dictionary
	void								SetDict ( CSphDict * dict );

	/// set HTML stripping mode
	/// sExtractAttrs defines what attributes to store
	/// sExtractAttrs format is "img=alt; a=alt,title"
	/// sExtractAttrs can be empty, this means that all the HTML will be stripped
	/// sExtractAttrs can be NULL, this means that no HTML stripping will be performed
	/// returns NULL on success
	/// returns error position on sConfig parsing failure
	const char *						SetStripHTML ( const char * sExtractAttrs );

	/// set tokenizer
	void								SetTokenizer ( ISphTokenizer * pTokenizer );

	/// get stats
	virtual const CSphSourceStats &		GetStats ();

	/// updates schema fields and attributes
	/// updates pInfo if it's empty; checks for match if it's not
	/// must be called after Connect(); will always fail otherwise
	virtual bool						UpdateSchema ( CSphSchema * pInfo );

	/// configure source to emit prefixes or infixes
	/// passing zero iMinLength means to emit the words themselves
	void								SetEmitInfixes ( bool bPrefixesOnly, int iMinLength );

public:
	/// connect to the source (eg. to the database)
	/// connection settings are specific for each source type and as such
	/// are implemented in specific descendants
	virtual bool						Connect () = 0;

	/// document getter
	/// to be implemented by descendants
	virtual int							Next () = 0;

	/// post-index callback
	/// gets called when the indexing is succesfully (!) over
	virtual void						PostIndex () {}

	/// check if there are any attributes configured
	/// note that there might be NO actual attributes in the case if configured
	/// ones do not match those actually returned by the source
	virtual bool						HasAttrsConfigured () = 0;

protected:
	ISphTokenizer *						m_pTokenizer;	///< my tokenizer
	CSphDict *							m_pDict;		///< my dict
	
	CSphSourceStats						m_tStats;		///< my stats
	CSphSchema							m_tSchema;		///< my schema

	bool								m_bStripHTML;	///< whether to strip HTML
	CSphHTMLStripper *					m_pStripper;	///< my HTML stripper

	int									m_iMinInfixLen;	///< min indexable infix length (0 means don't index infixes)
	bool								m_bPrefixesOnly;///< whether to index prefixes only or all the infixes
};


/// generic document source
/// provides multi-field support and generic tokenizer
struct CSphSource_Document : CSphSource
{
	/// ctor
							CSphSource_Document ( const char * sName ) : CSphSource ( sName ), m_bCallWordCallback ( false ) {}

	/// my generic tokenizer
	virtual int				Next ();

	/// this is what we can call for my descendants
	virtual void			WordCallback ( char * ) {}

	/// field data getter
	/// to be implemented by descendants
	virtual BYTE **			NextDocument () = 0;

protected:
	/// whether to call the callback
	bool					m_bCallWordCallback;
};


/// generic text source
/// one-field plain-text documents
struct CSphSource_Text : CSphSource_Document
{
					CSphSource_Text ( const char * sName ) : CSphSource_Document ( sName ) {};
	BYTE **			NextDocument ();

	virtual BYTE *	NextText () = 0;
};


/// generic SQL source params
struct CSphSourceParams_SQL
{
	// query params
	CSphString						m_sQuery;
	CSphString						m_sQueryRange;
	int								m_iRangeStep;

	CSphVector<CSphString,4>		m_dQueryPre;
	CSphVector<CSphString,4>		m_dQueryPost;
	CSphVector<CSphString,4>		m_dQueryPostIndex;
	CSphVector<CSphColumnInfo,4>	m_dAttrs;

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
	virtual bool		Connect ();

	virtual BYTE **		NextDocument ();
	virtual void		PostIndex ();

	virtual bool		HasAttrsConfigured () { return m_tParams.m_dAttrs.GetLength()!=0; }

protected:
	CSphString			m_sSqlDSN;

	BYTE *				m_dFields [ SPH_MAX_FIELDS ];

	int					m_iMinID;		///< grand min ID
	int					m_iMaxID;		///< grand max ID
	int					m_iCurrentID;	///< current min ID

	DWORD				m_iMaxFetchedID;///< max actually fetched ID

	bool						m_bSqlConnected;
	CSphSourceParams_SQL		m_tParams;

	static const int			MACRO_COUNT = 2;
	static const char * const	MACRO_VALUES [ MACRO_COUNT ];

protected:
	bool					RunQueryStep ();

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

protected:
	PGresult * 				m_pPgResult;	///< postgresql execution restult context
	PGconn *				m_tPgDriver;	///< postgresql connection context

	int						m_iPgRows;		///< how much rows last step returned
	int						m_iPgRow;		///< current row (0 based, as in PQgetvalue)

	CSphString				m_sPgClientEncoding;

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
					CSphSource_XMLPipe ( const char * sName );	///< ctor
					~CSphSource_XMLPipe ();						///< dtor

	bool			Setup ( const char * sCommand );			///< memorizes the command
	virtual bool	Connect ();									///< actually runs the command 
	virtual int		Next ();									///< hit chunk getter
	virtual bool	HasAttrsConfigured () { return true; }

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

	bool			m_bBody;			///< are we scanning body or expecting document?
	Tag_e			m_eTag;				///< what's our current tag
	const char *	m_pTag;				///< tag name
	int				m_iTagLength;		///< tag name length

	FILE *			m_pPipe;			///< incoming stream
	BYTE			m_sBuffer [ 4096 ];	///< buffer
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
	bool			CheckTag ( bool bOpen, bool bStrict=true );

	/// skips whitespace and opening/closing current tag (m_pTag)
	/// returns false on failure
	bool			SkipTag ( bool bOpen, bool bWarnOnEOF=true, bool bStrict=true );

	/// scan for tag with integer value
	bool			ScanInt ( const char * sTag, DWORD * pRes, bool bStrict=true );

	/// scan for tag with string value
	bool			ScanStr ( const char * sTag, char * pRes, int iMaxLength );
};

/////////////////////////////////////////////////////////////////////////////
// SEARCH QUERIES
/////////////////////////////////////////////////////////////////////////////

/// search query match
struct CSphMatch : public CSphDocInfo
{
	int m_iWeight;

	CSphMatch ()
		: m_iWeight ( 0 )
	{
	}

	CSphMatch ( const CSphMatch & rhs )
	{
		*this = rhs;
	}

	bool operator == ( const CSphMatch & rhs ) const
	{
		return ( m_iDocID==rhs.m_iDocID );
	}

	const CSphMatch & operator = ( const CSphMatch & rhs )
	{
		CSphDocInfo::operator = ( rhs );
		m_iWeight = rhs.m_iWeight;
		return *this;
	}
};


inline void Swap ( CSphMatch & a, CSphMatch & b )
{
	Swap ( a.m_iDocID, b.m_iDocID );
	Swap ( a.m_iAttrs, b.m_iAttrs );
	Swap ( a.m_pAttrs, b.m_pAttrs );
	Swap ( a.m_iWeight, b.m_iWeight );
}


/// search query sorting orders
enum ESphSortOrder
{
	SPH_SORT_RELEVANCE		= 0,	///< sort by document relevance desc, then by date
	SPH_SORT_ATTR_DESC		= 1,	///< sort by document date desc, then by relevance desc
	SPH_SORT_ATTR_ASC		= 2,	///< sort by document date asc, then by relevance desc
	SPH_SORT_TIME_SEGMENTS	= 3,	///< sort by time segments (hour/day/week/etc) desc, then by relevance desc
	SPH_SORT_EXTENDED		= 4,	///< sort by SQL-like expression (eg. "@relevance DESC, price ASC, @id DESC")

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

	SPH_MATCH_TOTAL
};


/// search query grouping mode
enum ESphGroupBy
{
	SPH_GROUPBY_DAY		= 0,	///< group by day
	SPH_GROUPBY_WEEK	= 1,	///< group by week
	SPH_GROUPBY_MONTH	= 2,	///< group by month
	SPH_GROUPBY_YEAR	= 3,	///< group by year
	SPH_GROUPBY_ATTR	= 4		///< group by attribute value
};


/// search query filter
class CSphFilter
{
public:
	CSphString		m_sAttrName;	///< filtered attribute name
	int				m_iAttrIndex;	///< filtered attribute index
	DWORD			m_uMinValue;	///< min value, only used when m_iValues==0
	DWORD			m_uMaxValue;	///< max value, only used when m_iValues==0
	int				m_iValues;		///< values set size, default is 0
	DWORD *			m_pValues;		///< values set. OWNED, WILL BE FREED IN DTOR.

public:
					CSphFilter ();
					~CSphFilter ();
	void			SortValues ();	///< sort values in ascending order

	const CSphFilter & operator = ( const CSphFilter & rhs );
};


/// search query
class CSphQuery
{
public:
	CSphString		m_sQuery;		///< query string
	int *			m_pWeights;		///< user-supplied per-field weights. may be NULL. default is NULL. NOT OWNED, WILL NOT BE FREED in dtor.
	int				m_iWeights;		///< number of user-supplied weights. missing fields will be assigned weight 1. default is 0
	ESphMatchMode	m_eMode;		///< match mode. default is "match all"
	ESphSortOrder	m_eSort;		///< sort mode
	CSphString		m_sSortBy;		///< attribute to sort by
	ISphTokenizer *	m_pTokenizer;	///< tokenizer to use. NOT OWNED.
	int				m_iMaxMatches;	///< max matches to retrieve, default is 1000. more matches use more memory and CPU time to hold and sort them

	DWORD			m_iMinID;		///< min ID to match, 0 by default
	DWORD			m_iMaxID;		///< max ID to match, UINT_MAX by default

	CSphVector<CSphFilter,8>	m_dFilters;	///< filters

	CSphString		m_sGroupBy;		///< group-by attribute name
	ESphGroupBy		m_eGroupFunc;	///< function to pre-process group-by attribute value with
	bool			m_bSortByGroup;	///< whether to sort found groups by group key or by current sorting func

protected:
	int				m_iAttrs;		///< attribute count (necessary to instantiate group-by queues)
	int				m_iGroupBy;		///< group-by attribute index

public:
					CSphQuery ();								///< ctor, fills defaults
					~CSphQuery ();								///< dtor, frees owned stuff
	bool			SetSchema ( const CSphSchema & tSchema );	///< calc m_iAttrs and m_iGroupBy from schema

	int				GetAttrsCount () const { return m_iAttrs;}
	int				GetGroupByAttr () const { return m_iGroupBy; }

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

public:
							CSphQueryResult ();		///< ctor
	virtual					~CSphQueryResult ();	///< dtor, which releases all owned stuff
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
		PHASE_COLLECT_END,			///< document collection phase end
		PHASE_SORT,					///< final sorting phase
		PHASE_SORT_END				///< final sorting phase end
	};

	Phase_e			m_ePhase;		///< current indexing phase

	int				m_iDocuments;	///< PHASE_COLLECT: documents collected so far
	SphOffset_t		m_iBytes;		///< PHASE_COLLECT: bytes collected so far

	SphOffset_t		m_iHits;		///< PHASE_SORT: hits sorted so far
	SphOffset_t		m_iHitsTotal;	///< PHASE_SORT: hits total
};


/// match queue interface
typedef ISphQueue<CSphMatch>	ISphMatchQueue;


/// available docinfo storage strategies
enum ESphDocinfo
{
	SPH_DOCINFO_NONE		= 0,	///< no docinfo available
	SPH_DOCINFO_INLINE		= 1,	///< inline docinfo into index (specifically, into doclists)
	SPH_DOCINFO_EXTERN		= 2		///< store docinfo separately
};


/// generic fulltext index interface
class CSphIndex
{
public:
	typedef void ProgressCallback_t ( const CSphIndexProgress * pStat );

public:
								CSphIndex ( const char * sName ) : m_pProgress ( NULL ), m_tSchema ( sName ) {}
	virtual						~CSphIndex () {}
	virtual	void				SetProgressCallback ( ProgressCallback_t * pfnProgress ) { m_pProgress = pfnProgress; }

public:
	virtual int					Build ( CSphDict * dict, const CSphVector < CSphSource * > & dSources, int iMemoryLimit, ESphDocinfo eDocinfo ) = 0;
	virtual CSphQueryResult *	Query ( CSphDict * dict, CSphQuery * pQuery ) = 0;
	virtual bool				QueryEx ( CSphDict * dict, CSphQuery * pQuery, CSphQueryResult * pResult, ISphMatchQueue * pTop ) = 0;
	virtual bool				Merge( CSphIndex * pSource ) = 0;

	virtual const CSphSchema *	Preload () = 0;

protected:
	ProgressCallback_t *		m_pProgress;
	CSphSchema					m_tSchema;
};

/////////////////////////////////////////////////////////////////////////////

/// create phrase fulltext index implemntation
CSphIndex *			sphCreateIndexPhrase ( const char * sFilename );

/// tell libsphinx to be quiet or not (logs and loglevels to come later)
void				sphSetQuiet ( bool bQuiet );

/// create proper queue for given query
/// may return NULL on error; in this case, error message is placed in sError
ISphMatchQueue *	sphCreateQueue ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sError );

/// convert queue to sorted array, and add its entries to result's matches array
void				sphFlattenQueue ( ISphMatchQueue * pQueue, CSphQueryResult * pResult );

#endif // _sphinx_

//
// $Id$
//
