//
// $Id$
//

//
// Copyright (c) 2001-2005, Andrew Aksyonoff. All rights reserved.
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
	#define USE_MYSQL		0	/// whether to compile with MySQL support
	#define USE_WINDOWS		1	/// whether to compile for Windows
#else
	#define USE_MYSQL		1	/// whether to compile with MySQL support
	#define USE_WINDOWS		0	/// whether to compile for Windows
#endif

/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#if USE_MYSQL
#include <mysql/mysql.h>
#endif

#if USE_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	#define strcasecmp			strcmpi

	typedef __int64				SphOffset_t;
#else
	typedef unsigned int		DWORD;
	typedef unsigned char		BYTE;
	typedef off_t				SphOffset_t;
#endif

/////////////////////////////////////////////////////////////////////////////

#define SPHINX_VERSION			"0.9.4-dev"
#define SPHINX_BANNER			"Sphinx " SPHINX_VERSION "\nCopyright (c) 2001-2005, Andrew Aksyonoff\n\n"
#define SPHINX_SEARCHD_PROTO	1

#define SPH_MAX_QUERY_WORDS		10
#define SPH_MAX_WORD_LEN		64
#define SPH_MAX_FILENAME_LEN	512
#define SPH_MAX_FIELD_COUNT		32

#define SPH_CLOG_BITS_DIR		10
#define SPH_CLOG_BITS_PAGE		22
#define SPH_CLOG_DIR_PAGES		(1 << SPH_CLOG_BITS_DIR)

#define SPH_CACHE_WRITE			1048576

/////////////////////////////////////////////////////////////////////////////

#define Min(a,b)			((a)<(b)?(a):(b))
#define Max(a,b)			((a)>(b)?(a):(b))
#define SafeDelete(_x)		{ if (_x) { delete (_x); (_x) = NULL; } }
#define SafeDeleteArray(_x)	{ if (_x) { delete [] (_x); (_x) = NULL; } }

void	sphDie ( char *message, ... );
void *	sphMalloc ( size_t size );
void *	sphRealloc ( void *ptr, size_t size );
void 	sphFree ( void * ptr );
char *	sphDup ( const char *s );

/// time, in seconds
float			sphLongTimer ();

/// Sphinx CRC32 implementation
DWORD			sphCRC32 ( const BYTE * pString );

/// new that never returns NULL (it crashes instead)
void *			operator new ( size_t iSize );

/// new that never returns NULL (it crashes instead)
void *			operator new [] ( size_t iSize );

/////////////////////////////////////////////////////////////////////////////
// GENERICS
/////////////////////////////////////////////////////////////////////////////

/// swap
template < typename T > inline void Swap ( T & v1, T & v2 )
{
	T temp = v1;
	v1 = v2;
	v2 = temp;
}


/// generic vector
/// (don't even ask why it's not std::vector)
template < typename T, int INITIAL_LIMIT=1024 > class CSphVector
{
public:
	/// ctor
	CSphVector ()
		: m_iLength	( 0 )
		, m_iLimit	( 0 )
		, m_pData	( NULL )
	{
	}

	/// dtor
	~CSphVector ()
	{
		Reset ();
	}

	/// add entry
	T & Add ()
	{
		if ( m_iLength>=m_iLimit )
			Grow ( 1+m_iLength );
		return m_pData [ m_iLength++ ];
	}

	/// add entry
	void Add ( const T & tValue )
	{
		if ( m_iLength>=m_iLimit )
			Grow ( 1+m_iLength );
		m_pData [ m_iLength++ ] = tValue;
	}

	/// get last entry
	T & Last ()
	{
		return (*this) [ m_iLength-1 ];
	}

	/// grow enough to hold that much entries, if needed
	void Grow ( int iNewLimit )
	{
		// check that we really need to be called
		assert ( iNewLimit>m_iLength );

		// calc new limit
		if ( !m_iLimit )
			m_iLimit = INITIAL_LIMIT;
		while ( m_iLimit<iNewLimit )
			m_iLimit *= 2;

		// realloc
		// FIXME! optimize for POD case
		T * pNew = new T [ m_iLimit ];
		for ( int i=0; i<m_iLength; i++ )
			pNew[i] = m_pData[i];
		delete [] m_pData;
		m_pData = pNew;
	}

	/// resize
	void Resize ( int iNewLength )
	{
		// only resize up for now
		assert ( iNewLength>=m_iLength );

		if ( iNewLength!=m_iLength )
		{
			Grow ( iNewLength );
			m_iLength = iNewLength;
		}
	}

	/// reset
	void Reset ()
	{
		m_iLength = 0;
		m_iLimit = 0;
		SafeDeleteArray ( m_pData );
	}

	/// sort the array
	template < typename F > void Sort ( F comp, int iStart=0, int iEnd=-1 )
	{
		if ( m_iLength<2 ) return;
		if ( iStart<0 ) iStart = m_iLength+iStart;
		if ( iEnd<0 ) iEnd = m_iLength+iEnd;
		assert ( iStart<=iEnd );

		int st0[32], st1[32], a, b, k, i, j;
		T x;

		k = 1;
		st0[0] = iStart;
		st1[0] = iEnd;
		while ( k )
		{
			k--;
			i = a = st0[k];
			j = b = st1[k];
			x = m_pData [ (a+b)/2 ]; // FIXME! add better median at least
			while ( a<b )
			{
				while ( i<=j )
				{
					while ( comp ( m_pData[i], x) > 0 ) i++;
					while ( comp ( x, m_pData[j]) > 0 ) j--;
					if (i <= j) { Swap ( m_pData[i], m_pData[j] ); i++; j--; }
				}

				if ( j-a>=b-i )
				{
					if ( a<j ) { st0[k] = a; st1[k] = j; k++; }
					a = i;
				} else
				{
					if ( i<b ) { st0[k] = i; st1[k] = b; k++; }
					b = j;
				}
			}
		}
	}

	/// query current length
	int GetLength ()
	{
		return m_iLength;
	}

	/// access
	/// FIXME! optimize for POD case
	const T & operator [] ( int iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		return m_pData [ iIndex ];
	}

	/// access
	/// FIXME! optimize for POD case
	T & operator [] ( int iIndex )
	{
		assert ( iIndex>=0 && iIndex<m_iLength );
		return m_pData [ iIndex ];
	}

private:
	int		m_iLength;		///< entries actually used
	int		m_iLimit;		///< entries allocated
	T *		m_pData;		///< entries
};


#define ARRAY_FOREACH(_index,_array) \
	for ( int _index=0; _index<_array.GetLength(); _index++ )

/////////////////////////////////////////////////////////////////////////////
// DICTIONARIES
/////////////////////////////////////////////////////////////////////////////

/// morphology flags
enum ESphMorphology
{
	SPH_MORPH_NONE		= 0,
	SPH_MORPH_STEM_EN	= (1<<1),
	SPH_MORPH_STEM_RU	= (1<<2)
};


/// abstract word dictionary interface
struct CSphDict
{
	/// get word ID by word
	virtual DWORD		GetWordID ( BYTE * pWord ) = 0;

	/// load stopwords from given files
	virtual void		LoadStopwords ( const char * sFiles ) = 0;
};


/// CRC32 dictionary
struct CSphDict_CRC32 : CSphDict
{
	/// ctor
	/// iMorph is a combination of ESphMorphology flags
						CSphDict_CRC32 ( DWORD iMorph );

	/// get word ID by word
	/// does requested morphology and returns CRC32
	virtual DWORD		GetWordID ( BYTE * pWord );

	/// load stopwords from given files
	virtual void		LoadStopwords ( const char * sFiles );

protected:
	DWORD				m_iMorph;		///< morphology flags
	int					m_iStopwords;	///< stopwords count
	DWORD *				m_pStopwords;	///< stopwords ID list
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
	DWORD	m_iDocID;		///< document ID
	DWORD	m_iGroupID;		///< documents group ID
	DWORD	m_iTimestamp;	///< document timestamp
};


/// source statistics
struct CSphSourceStats
{
	/// how much documents
	int				m_iTotalDocuments;

	/// how much bytes
	SphOffset_t		m_iTotalBytes;


	/// ctor
	CSphSourceStats () :
		m_iTotalDocuments ( 0 ),
		m_iTotalBytes ( 0 )
	{};
};


/// generic tokenizer
class CSphTokenizer
{
public:
	/// create empty tokenizer
						CSphTokenizer ();

	/// pass next buffer
	void				SetBuffer ( BYTE * sBuffer, int iLength, bool bLast );

	/// get next token
	BYTE *				GetToken ();

	/// set new translation table
	/// returns true on success, false on failure
	bool				SetTranslationTable ( const char * sConfig );

	/// set new translation table
	void				SetTranslationTable ( const BYTE * dTable );

protected:
	BYTE				m_dTable [ 256 ];				///< my translation table
	BYTE *				m_pBuffer;						///< my buffer
	BYTE *				m_pBufferMax;					///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	BYTE *				m_pCur;							///< current position
	BYTE				m_sAccum [ 4+SPH_MAX_WORD_LEN];	///< boundary token accumulator
	int					m_iAccum;						///< boundary token size
	bool				m_bLast;						///< is this buffer the last one
};


/// generic data source
class CSphSource
{
public:
	CSphVector<CSphWordHit>				m_dHits;	///< current document split into words
	CSphDocInfo							m_tDocInfo;


public:
										CSphSource();

	/// dtor
	virtual								~CSphSource() {}

	/// set dictionary
	void								setDict ( CSphDict * dict );

	/// get stats
	virtual const CSphSourceStats *		GetStats ();

public:
	/// document getter
	/// to be implemented by descendants
	virtual int							next() = 0;

	/// field count getter
	/// to be implemented by descendants
	/// MUST be called AFTER the indexing is over
	/// because at indexing stage, we might not be sure of the exact count
	virtual int							GetFieldCount () = 0;

protected:
	CSphTokenizer						m_tTokenizer;	///< my tokenizer
	CSphDict *							m_pDict;		///< my dict
	CSphSourceStats						m_iStats;		///< my stats
};


/// generic document source
/// provides multi-field support and generic tokenizer
struct CSphSource_Document : CSphSource
{
	/// ctor
							CSphSource_Document () : m_bCallWordCallback ( false ) {}

	/// my generic tokenizer
	virtual int				next ();

	/// this is what we can call for my descendants
	virtual void			wordCallback ( char * word ) { word = word; }

	/// field data getter
	/// to be implemented by descendants
	virtual BYTE **			NextDocument () = 0;

	/// field count getter
	virtual int				GetFieldCount ();

protected:
	/// my field count
	/// MUST be filled by NextDocument ()
	int						m_iFieldCount;

	/// whether to call the callback
	bool					m_bCallWordCallback;
};


/// generic text source
/// one-field plain-text documents
struct CSphSource_Text : CSphSource_Document
{
					CSphSource_Text () { m_iFieldCount = 1; }
	BYTE **			NextDocument ();
	virtual BYTE *	NextText () = 0;
};


#if USE_MYSQL
/// MySQL source params
struct CSphSourceParams_MySQL
{
	// query params
	const char *	m_sQuery;
	const char *	m_sQueryPre;
	const char *	m_sQueryPost;
	const char *	m_sQueryRange;
	const char *	m_sGroupColumn;
	const char *	m_sDateColumn;
	int				m_iRangeStep;

	// connection params
	const char *	m_sHost;
	const char *	m_sUser;
	const char *	m_sPass;
	const char *	m_sDB;
	int				m_iPort;
	const char *	m_sUsock;

	/// ctor which sets defaults
					CSphSourceParams_MySQL ();
};


/// MySQL source implementation
/// multi-field plain-text documents fetched from given query
struct CSphSource_MySQL : CSphSource_Document
{
						CSphSource_MySQL ();
	virtual				~CSphSource_MySQL ();

	bool				Init ( CSphSourceParams_MySQL * pParams );
	virtual BYTE **		NextDocument ();

protected:
	MYSQL_RES *			m_pSqlResult;
	MYSQL_ROW			m_tSqlRow;
	MYSQL				m_tSqlDriver;
	char *				m_sSqlDSN;

	char *				m_sQuery;		///< main fetch query
	char *				m_sQueryPost;	///< post-fetch query
	int					m_iGroupColumn;	///< group_id column number
	int					m_iDateColumn;	///< date column number

	BYTE *				m_dFields [ SPH_MAX_FIELD_COUNT ];
	int					m_dRemapFields [ SPH_MAX_FIELD_COUNT ];

	int					m_iRangeStep;	///< ID range step, -1 if not using ranges
	int					m_iMinID;		///< grand min ID
	int					m_iMaxID;		///< grand max ID
	int					m_iCurrentID;	///< current min ID

	static const int			MACRO_COUNT = 2;
	static const char * const	MACRO_VALUES [ MACRO_COUNT ];

protected:
	bool				RunQueryStep ();
	int					GetColumnIndex ( const char * sColumn );
};
#endif


/// XML pipe source implementation
class CSphSource_XMLPipe : public CSphSource
{
public:
	/// ctor
					CSphSource_XMLPipe ();

	/// dtor
					~CSphSource_XMLPipe ();

	/// initializer
	bool			Init ( const char * sCommand );

	/// hit chunk getter
	virtual int		next ();

	/// field count getter
	virtual int		GetFieldCount ();

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
	/// are we scanning body or expecting document?
	bool			m_bBody;

	/// what's our current tag
	Tag_e			m_eTag;

	/// tag name
	const char *	m_pTag;

	/// tag name length
	int				m_iTagLength;

	/// incoming stream
	FILE *			m_pPipe;

	/// buffer
	BYTE			m_sBuffer [ 4096 ];

	/// current buffer pos
	BYTE *			m_pBuffer;

	/// buffered end pos
	BYTE *			m_pBufferEnd;

private:
	/// current word position
	int				m_iWordPos;

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
	bool			CheckTag ( bool bOpen );

	/// skips whitespace and opening/closing current tag (m_pTag)
	/// returns false on failure
	bool			SkipTag ( bool bOpen, bool bWarnOnEOF=true );

	/// scan for tag with integer value
	bool			ScanInt ( const char * sTag, DWORD * pRes );

	/// scan for tag with string value
	bool			ScanStr ( const char * sTag, char * pRes, int iMaxLength );
};

/////////////////////////////////////////////////////////////////////////////
// SEARCH QUERIES
/////////////////////////////////////////////////////////////////////////////

/// search query match
struct CSphMatch : public CSphDocInfo
{
	int			m_iWeight;
};


/// search query sorting orders
enum ESphSortOrder
{
	SPH_SORT_RELEVANCE = 0,		///< sort by document relevance desc, then by date
	SPH_SORT_DATE_DESC,			///< sort by document date desc, then by relevance desc
	SPH_SORT_DATE_ASC,			///< sort by document date asc, then by relevance desc
	SPH_SORT_TIME_SEGMENTS,		///< sort by time segments (hour/day/week/etc) desc, then by relevance desc

	SPH_SORT_TOTAL
};


/// search query matching mode
enum ESphMatchMode
{
	SPH_MATCH_ALL = 0,			///< match all query words
	SPH_MATCH_ANY,				///< match any query word
	SPH_MATCH_PHRASE,			///< match this exact phrase

	SPH_MATCH_TOTAL
};


/// search query
class CSphQuery
{
public:
	const char *	m_sQuery;	///< query string. MUST be not NULL
	int *			m_pWeights;	///< user-supplied per-field weights. may be NULL. default is NULL. WILL NOT BE FREED in dtor.
	int				m_iWeights;	///< number of user-supplied weights. missing fields will be assigned weight 1. default is 0
	ESphMatchMode	m_eMode;	///< match mode. default is "match all"
	DWORD *			m_pGroups;	///< groups to match. default is NULL, which means "match all". WILL BE FREED in dtor.
	int				m_iGroups;	///< count of groups to match
	ESphSortOrder	m_eSort;	///< sorting order

public:
					CSphQuery ();	///< ctor, fills defaults
					~CSphQuery ();	///< dtor, safely frees owned fields
};


/// search query result
class CSphQueryResult
{
public:
	static const int		MAX_MATCHES = 1000;

	struct
	{
		char *				m_sWord;	///< i-th search term (normalized word form)
		int					m_iDocs;	///< document count for this term
		int					m_iHits;	///< hit count for this term
	}						m_tWordStats [ SPH_MAX_QUERY_WORDS ];

	int						m_iNumWords;		///< query word count
	float					m_fQueryTime;		///< query time, seconds
	CSphVector<CSphMatch>	m_dMatches;			///< top matching documents, no more than MAX_MATCHES
	int						m_iTotalMatches;	///< total matches count

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


/// generic fulltext index interface
class CSphIndex
{
public:
	typedef void ProgressCallback_t ( const CSphIndexProgress * pStat );

public:
								CSphIndex() : m_pProgress ( NULL ) {}
	virtual	void				SetProgressCallback ( ProgressCallback_t * pfnProgress ) { m_pProgress = pfnProgress; }

public:
	virtual int					build ( CSphDict * dict, CSphSource * source, int iMemoryLimit ) = 0;
	virtual CSphQueryResult *	query ( CSphDict * dict, CSphQuery * pQuery ) = 0;

protected:
	ProgressCallback_t *		m_pProgress;
};

/////////////////////////////////////////////////////////////////////////////

/// create phrase fulltext index implemntation
CSphIndex *		sphCreateIndexPhrase ( const char * sFilename );

#endif // _sphinx_

//
// $Id$
//
