//
// $Id$
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
#else
	typedef unsigned int		DWORD;
	typedef unsigned char		BYTE;
#endif

/////////////////////////////////////////////////////////////////////////////

#define SPH_MAX_QUERY_WORDS		10
#define SPH_MAX_WORD_LEN		64
#define SPH_MAX_FILENAME_LEN	512
#define SPH_MAX_FIELD_COUNT		32

#define SPH_RLOG_MAX_BLOCKS		128
#define SPH_RLOG_BIN_SIZE		262144
#define SPH_RLOG_BLOCK_SIZE		1048576

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
float	sphLongTimer ();

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
			Resize ( 1+m_iLength );
		return m_pData [ m_iLength++ ];
	}

	/// add entry
	void Add ( const T & tValue )
	{
		if ( m_iLength>=m_iLimit )
			Resize ( 1+m_iLength );
		m_pData [ m_iLength++ ] = tValue;
	}

	/// resize
	void Resize ( int iNewLimit )
	{
		// check that there'll be enough place
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
struct CSphHit
{
	DWORD	m_iGroupID;		///< documents group ID (from data source)
	DWORD	m_iDocID;		///< document ID (from data source)
	DWORD	m_iWordID;		///< word ID (from dictionary)
	DWORD	m_iWordPos;		///< word position in document
};


/// source statistics
struct CSphSourceStats
{
	/// how much documents
	int		m_iTotalDocuments;

	/// how much bytes
	int		m_iTotalBytes;


	/// ctor
			CSphSourceStats () :
				m_iTotalDocuments ( 0 ),
				m_iTotalBytes ( 0 )
			{};
};


/// generic data source
struct CSphSource
{
	CSphVector<CSphHit>		hits;
	CSphDict *				dict;

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
	/// my stats
	CSphSourceStats						m_iStats;
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
	virtual void			wordCallback ( char * word ) {}

	/// field data getter
	/// to be implemented by descendants
	virtual BYTE **			NextDocument () = 0;

	/// field count getter
	virtual int				GetFieldCount ();

protected:
	/// last group id
	/// MUST be filled by NextDocument ()
	DWORD					m_iLastGroupID;

	/// last document id
	/// MUST be filled by NextDocument ()
	DWORD					m_iLastID;

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
	const char *	m_sGroupColumn;

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

private:
	MYSQL_RES *			m_pSqlResult;
	MYSQL_ROW			m_tSqlRow;
	MYSQL				m_tSqlDriver;

	char *				m_sQueryPost;	///< query which is issued after main fetch
	int					m_iGroupColumn;
	BYTE *				m_dFields [ SPH_MAX_FIELD_COUNT ];
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
	/// current document ID
	int				m_iDocID;

	/// current group ID
	int				m_iGroupID;

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
	bool			ScanInt ( const char * sTag, int * pRes );

	/// scan for tag with string value
	bool			ScanStr ( const char * sTag, char * pRes, int iMaxLength );
};

/////////////////////////////////////////////////////////////////////////////
// SEARCH QUERIES
/////////////////////////////////////////////////////////////////////////////

/// search query match
struct CSphMatch
{
	DWORD		m_iGroupID;
	DWORD		m_iDocID;
	int			m_iWeight;
};


/// search query
class CSphQuery
{
public:
	const char *	m_sQuery;	///< query string. MUST be not NULL
	int *			m_pWeights;	///< user-supplied per-field weights. may be NULL. default is NULL
	int				m_iWeights;	///< number of user-supplied weights. missing fields will be assigned weight 1. default is 0
	bool			m_bAll;		///< match all words or any word. default is "match all"
	int				m_iGroup;	///< match this group only. default is 0, which means "match all"

public:
	/// ctor. fills defaults
					CSphQuery ();
};


/// search query result
class CSphQueryResult
{
public:
	struct
	{
		char *				m_sWord;	///< i-th search term (normalized word form)
		int					m_iDocs;	///< document count for this term
		int					m_iHits;	///< hit count for this term
	}						m_tWordStats [ SPH_MAX_QUERY_WORDS ];

	int						m_iNumWords;	///< query word count
	float					m_fQueryTime;	///< query time, seconds
	CSphVector<CSphMatch>	m_dMatches;		///< matching documents/weights vector

public:
							CSphQueryResult ();		///< ctor
	virtual					~CSphQueryResult ();	///< dtor, which releases all owned stuff
};

/////////////////////////////////////////////////////////////////////////////
// FULLTEXT INDICES
/////////////////////////////////////////////////////////////////////////////

/// generic fulltext index interface
struct CSphIndex
{
	virtual int					build ( CSphDict * dict, CSphSource * source ) = 0;
	virtual CSphQueryResult *	query ( CSphDict * dict, CSphQuery * pQuery ) = 0;
};

/// create phrase fulltext index implemntation
CSphIndex *		sphCreateIndexPhrase ( const char * sFilename );

/////////////////////////////////////////////////////////////////////////////
// UTILITY CLASSES
/////////////////////////////////////////////////////////////////////////////

struct CSphHash
{
	int count;
	char **keys, **values;

	CSphHash();
	~CSphHash();

	void add(char *key, char *value);
	char *get(char *key);

private:
	int max;
};


struct CSphConfig
{
	CSphConfig();
	~CSphConfig();

	int open ( const char *file );
	CSphHash * loadSection ( const char *section );

private:
	FILE *fp;
};

#endif // _sphinx_

//
// $Id$
//
