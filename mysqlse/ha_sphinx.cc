//
// $Id$
//

#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation // gcc: Class implementation
#endif

#if _MSC_VER>=1400
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#include <mysql_version.h>

#if MYSQL_VERSION_ID>50100
#include "mysql_priv.h"
#include <mysql/plugin.h>
#else
#include "../mysql_priv.h"
#endif

#include <mysys_err.h>
#include <my_sys.h>

#ifndef __WIN__
	// UNIX-specific
	#include <my_net.h>
	#include <netdb.h>
#else
	// Windows-specific
	#include <io.h>
	#define strcasecmp	stricmp
	#define snprintf	_snprintf
#endif

#include <ctype.h>
#include "ha_sphinx.h"

#ifndef MSG_WAITALL
#define MSG_WAITALL 0
#endif

#if _MSC_VER>=1400
#pragma warning(push,4)
#endif

/////////////////////////////////////////////////////////////////////////////

// FIXME! make this all dynamic
#define SPHINXSE_MAX_FILTERS		32

#define SPHINXSE_DEFAULT_HOST		"127.0.0.1"
#define SPHINXSE_DEFAULT_PORT		3312
#define SPHINXSE_DEFAULT_INDEX		"*"

#define SPHINXSE_SYSTEM_COLUMNS		3

#define SPHINXSE_MAX_ALLOC			(16*1024*1024)

// FIXME! all the following is cut-n-paste from sphinx.h and searchd.cpp
#define SPHINX_VERSION		"0.9.7"

enum
{
	SPHINX_SEARCHD_PROTO	= 1,
	SEARCHD_COMMAND_SEARCH	= 0,
	VER_COMMAND_SEARCH		= 0x109	,
};

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

/// known attribute types
enum ESphAttrType
{
	SPH_ATTR_NONE		= 0,	///< not an attribute at all
	SPH_ATTR_INTEGER	= 1,	///< this attr is just an integer
	SPH_ATTR_TIMESTAMP	= 2		///< this attr is a timestamp
};

/// known answers
enum
{
	SEARCHD_OK		= 0,	///< general success, command-specific reply follows
	SEARCHD_ERROR	= 1,	///< general failure, error message follows
	SEARCHD_RETRY	= 2,	///< temporary failure, error message follows, client should retry later
	SEARCHD_WARNING	= 3		///< general success, warning message and command-specific reply follow
};

//////////////////////////////////////////////////////////////////////////////

#define SPHINX_DEBUG_OUTPUT		0
#define SPHINX_DEBUG_CALLS		0

#include <stdarg.h>

inline void SPH_DEBUG ( const char * format, ... )
{
	#if SPHINX_DEBUG_OUTPUT
	va_list ap;
	va_start ( ap, format );
	fprintf ( stderr, "SphinxSE: " );
	vfprintf ( stderr, format, ap );
	fprintf ( stderr, "\n" );
	va_end ( ap );
	#endif
}

#if SPHINX_DEBUG_CALLS

#define SPH_ENTER_FUNC() { SPH_DEBUG ( "enter %s", __FUNCTION__ ); }
#define SPH_ENTER_METHOD() { SPH_DEBUG ( "enter %s(this=%08x)", __FUNCTION__, this ); }
#define SPH_RET(_arg) { SPH_DEBUG ( "leave %s", __FUNCTION__ ); return _arg; }
#define SPH_VOID_RET() { SPH_DEBUG ( "leave %s", __FUNCTION__ ); return; }

#else

#define SPH_ENTER_FUNC() DBUG_ENTER(__FUNCTION__)
#define SPH_ENTER_METHOD() DBUG_ENTER(__FUNCTION__)
#define SPH_RET(_arg) { DBUG_RETURN(_arg); }
#define SPH_VOID_RET() { DBUG_VOID_RETURN; }

#endif


#define SafeDelete(_arg)		{ if ( _arg ) delete ( _arg );		(_arg) = NULL; }
#define SafeDeleteArray(_arg)	{ if ( _arg ) delete [] ( _arg );	(_arg) = NULL; }

//////////////////////////////////////////////////////////////////////////////

/// a structure that will be shared among all open Sphinx SE handlers
struct CSphSEShare
{
	pthread_mutex_t	m_tMutex;
	THR_LOCK		m_tLock;

	char *			m_sTable;
	char *			m_sScheme;
	char *			m_sHost;	///< points into m_sScheme buffer, DO NOT FREE EXPLICITLY
	char *			m_sSocket;	///< points into m_sScheme buffer, DO NOT FREE EXPLICITLY
	char *			m_sIndex;	///< points into m_sScheme buffer, DO NOT FREE EXPLICITLY
	ushort			m_iPort;
	uint			m_iTableNameLen;
	uint			m_iUseCount;

	int					m_iTableFields;
	char **				m_sTableField;
	enum_field_types *	m_eTableFieldType;

	CSphSEShare ()
		: m_sTable ( NULL )
		, m_sScheme ( NULL )
		, m_sHost ( NULL )
		, m_sSocket ( NULL )
		, m_sIndex ( NULL )
		, m_iPort ( 0 )
		, m_iTableNameLen ( 0 )
		, m_iUseCount ( 1 )

		, m_iTableFields ( 0 )
		, m_sTableField ( NULL )
		, m_eTableFieldType ( NULL )
	{
		thr_lock_init ( &m_tLock );
		pthread_mutex_init ( &m_tMutex, MY_MUTEX_INIT_FAST );
	}

	~CSphSEShare ()
	{
		pthread_mutex_destroy ( &m_tMutex );
		thr_lock_delete ( &m_tLock );

		SafeDeleteArray ( m_sTable );
		SafeDeleteArray ( m_sScheme );
		ResetTable ();
	}

	void ResetTable ()
	{
		for ( int i=0; i<m_iTableFields; i++ )
			SafeDeleteArray ( m_sTableField[i] );
		SafeDeleteArray ( m_sTableField );
		SafeDeleteArray ( m_eTableFieldType );
	}
};

/// schema attribute
struct CSphSEAttr
{
	char *			m_sName;		///< attribute name (received from Sphinx)
	ESphAttrType	m_eType;		///< attribute type (received from Sphinx)
	int				m_iField;		///< field index in current table (-1 if none)

	CSphSEAttr()
		: m_sName ( NULL )
		, m_eType ( SPH_ATTR_NONE )
		, m_iField ( -1 )
	{}

	~CSphSEAttr ()
	{
		SafeDeleteArray ( m_sName );
	}
};

/// word stats
struct CSphSEWordStats
{
	char *			m_sWord;
	int				m_iDocs;
	int				m_iHits;

	CSphSEWordStats ()
		: m_sWord ( NULL )
		, m_iDocs ( 0 )
		, m_iHits ( 0 )
	{}

	~CSphSEWordStats ()
	{
		SafeDeleteArray ( m_sWord );
	}
};

/// request stats
struct CSphSEStats
{
public:
	int					m_iMatchesTotal;
	int					m_iMatchesFound;
	int					m_iQueryMsec;
	int					m_iWords;
	CSphSEWordStats *	m_dWords;
	bool				m_bLastError;
	char				m_sLastMessage[1024];

	CSphSEStats()
		: m_dWords ( NULL )
	{
		Reset ();
	}

	void Reset ()
	{
		m_iMatchesTotal = 0;
		m_iMatchesFound = 0;
		m_iQueryMsec = 0;
		m_iWords = 0;
		SafeDeleteArray ( m_dWords );
		m_bLastError = false;
		m_sLastMessage[0] = '\0';
	}

	~CSphSEStats()
	{
		Reset ();
	}
};

/// search query filter
struct CSphSEFilter
{
public:
	char *			m_sAttrName;
	uint32			m_uMinValue;
	uint32			m_uMaxValue;
	int				m_iValues;
	uint32 *		m_pValues;
	int				m_bExclude;

public:
	CSphSEFilter ()
		: m_sAttrName ( NULL )
		, m_uMinValue ( 0 )
		, m_uMaxValue ( UINT_MAX )
		, m_iValues ( 0 )
		, m_pValues ( NULL )
		, m_bExclude ( 0 )
	{
	}

	~CSphSEFilter ()
	{
		SafeDeleteArray ( m_pValues );
	}
};

/// client-side search query
struct CSphSEQuery
{
private:
	char *			m_sQueryBuffer;

	const char *	m_sIndex;
	int				m_iOffset;
	int				m_iLimit;

	char *			m_sQuery;
	uint32 *		m_pWeights;
	int				m_iWeights;
	ESphMatchMode	m_eMode;
	ESphSortOrder	m_eSort;
	char *			m_sSortBy;
	int				m_iMaxMatches;
	uint32			m_iMinID;
	uint32			m_iMaxID;

	int				m_iFilters;
	CSphSEFilter	m_dFilters[SPHINXSE_MAX_FILTERS];

	ESphGroupBy		m_eGroupFunc;
	char *			m_sGroupBy;
	char *			m_sGroupSortBy;

	int				m_iCutoff;

public:
	char			m_sParseError[256];

public:
	CSphSEQuery ( const char * sQuery, int iLength, const char * sIndex );
	~CSphSEQuery ();

	bool			Parse ();
	int				BuildRequest ( char ** ppBuffer );

protected:
	char *			m_pBuf;
	char *			m_pCur;
	int				m_iBufLeft;
	bool			m_bBufOverrun;

	int				ParseArray ( uint32 ** ppValues, const char * sValue );
	bool			ParseField ( char * sField );

	void			SendBytes ( const void * pBytes, int iBytes );
	void			SendWord ( short int v )		{ v = ntohs(v); SendBytes ( &v, sizeof(short int) ); }
	void			SendInt ( int v )				{ v = ntohl(v); SendBytes ( &v, sizeof(int) ); }
	void			SendDword ( uint v )			{ v = ntohl(v) ;SendBytes ( &v, sizeof(uint) ); }
	void			SendString ( const char * v )	{ int iLen = strlen(v); SendDword(iLen); SendBytes ( v, iLen ); }
};

//////////////////////////////////////////////////////////////////////////////

#if MYSQL_VERSION_ID>50100

#if MYSQL_VERSION_ID<50112
#error Sphinx SE requires MySQL 5.1.12 or higher if compiling for 5.1.x series!
#endif // <50112

static handler *	sphinx_create_handler ( handlerton * hton, TABLE_SHARE * table, MEM_ROOT * mem_root );
static int			sphinx_init_func ( void * p );
static int			sphinx_close_connection ( handlerton * hton, THD * thd );
static int			sphinx_panic ( handlerton * hton, enum ha_panic_function flag );
static bool			sphinx_show_status ( handlerton * hton, THD * thd, stat_print_fn * stat_print, enum ha_stat_type stat_type );

#else

static bool			sphinx_init_func_for_handlerton ();
static int			sphinx_close_connection ( THD * thd );
bool				sphinx_show_status ( THD * thd );

#endif // >50100

//////////////////////////////////////////////////////////////////////////////

static const char	sphinx_hton_name[]		= "SPHINX";
static const char	sphinx_hton_comment[]	= "Sphinx storage engine " SPHINX_VERSION;

#if MYSQL_VERSION_ID<50100
handlerton sphinx_hton =
{
	#ifdef MYSQL_HANDLERTON_INTERFACE_VERSION
	MYSQL_HANDLERTON_INTERFACE_VERSION,
	#endif
	sphinx_hton_name,
	SHOW_OPTION_YES,
	sphinx_hton_comment,
	DB_TYPE_SPHINX_DB,
	sphinx_init_func_for_handlerton,
	0,							// slot
	0,							// savepoint size
	sphinx_close_connection,	// close_connection
	NULL,	// savepoint
	NULL,	// rollback to savepoint
	NULL,	// release savepoint
	NULL,	// commit
	NULL,	// rollback
	NULL,	// prepare
	NULL,	// recover
	NULL,	// commit_by_xid
	NULL,	// rollback_by_xid
	NULL,	// create_cursor_read_view
	NULL,	// set_cursor_read_view
	NULL,	// close_cursor_read_view
	HTON_CAN_RECREATE
};
#endif

//////////////////////////////////////////////////////////////////////////////

// variables for Sphinx shared methods
pthread_mutex_t		sphinx_mutex;		// mutex to init the hash
static int			sphinx_init = 0;	// flag whether the hash was initialized
static HASH			sphinx_open_tables;	// hash used to track open tables

//////////////////////////////////////////////////////////////////////////////
// INITIALIZATION AND SHUTDOWN
//////////////////////////////////////////////////////////////////////////////

// hashing function
static byte * sphinx_get_key ( CSphSEShare * pShare, uint * pLength, my_bool )
{
	*pLength = pShare->m_iTableNameLen;
	return (byte*) pShare->m_sTable;
}


static int sphinx_init_func ( void * p )
{
	SPH_ENTER_FUNC();
	if ( !sphinx_init )
	{
		sphinx_init = 1;
		VOID ( pthread_mutex_init ( &sphinx_mutex, MY_MUTEX_INIT_FAST ) );
		hash_init ( &sphinx_open_tables, system_charset_info, 32, 0, 0,
			(hash_get_key) sphinx_get_key, 0, 0 );

		#if MYSQL_VERSION_ID > 50100
		handlerton * hton = (handlerton*) p;
		hton->state				= SHOW_OPTION_YES;
		hton->db_type			= DB_TYPE_DEFAULT;
		hton->create			= sphinx_create_handler;
		hton->close_connection	= sphinx_close_connection;
		hton->show_status		= sphinx_show_status;
		hton->panic				= sphinx_panic;
		hton->flags				= HTON_CAN_RECREATE;
		#endif
	}
	SPH_RET(0);
}


#if MYSQL_VERSION_ID<50100
static bool sphinx_init_func_for_handlerton ()
{
	return sphinx_init_func ( &sphinx_hton );
}
#endif


#if MYSQL_VERSION_ID>50100

static int sphinx_close_connection ( handlerton * hton, THD * thd )
{
	// deallocate common handler data
	SPH_ENTER_FUNC();
	void ** tmp = thd_ha_data ( thd, hton );
	CSphSEStats * pStats = (CSphSEStats*) (*tmp);
	SafeDelete ( pStats );
	*tmp = NULL;
	SPH_RET(0);
}


static int sphinx_done_func ( void * p )
{
	SPH_ENTER_FUNC();

	int error = 0;
	if ( sphinx_init )
	{
		sphinx_init = 0;
		if ( sphinx_open_tables.records )
			error = 1;
		hash_free ( &sphinx_open_tables );
		pthread_mutex_destroy ( &sphinx_mutex );
	}

	SPH_RET(0);
}


static int sphinx_panic ( handlerton * hton, enum ha_panic_function flag )
{
	return sphinx_done_func ( hton );
}

#else

static int sphinx_close_connection ( THD * thd )
{
	// deallocate common handler data
	SPH_ENTER_FUNC();
	CSphSEStats * pStats = (CSphSEStats*) thd->ha_data[sphinx_hton.slot];
	SafeDelete ( pStats );
	thd->ha_data[sphinx_hton.slot] = NULL;
	SPH_RET(0);
}

#endif // >50100

//////////////////////////////////////////////////////////////////////////////
// SHOW STATUS
//////////////////////////////////////////////////////////////////////////////

#if MYSQL_VERSION_ID>50100
static bool sphinx_show_status ( handlerton * hton, THD * thd, stat_print_fn * stat_print,
	enum ha_stat_type stat_type )
#else
bool sphinx_show_status ( THD * thd )
#endif
{
	SPH_ENTER_FUNC();

#if MYSQL_VERSION_ID<50100
	Protocol * protocol = thd->protocol;
	List<Item> field_list;
#endif

	char buf1[IO_SIZE];
	uint buf1len;
	char buf2[IO_SIZE];
	uint buf2len= 0;
	String words;

	buf1[0] = '\0';
	buf2[0] = '\0';

#if MYSQL_VERSION_ID>50100
	CSphSEStats * pStats = (CSphSEStats*) ( *thd_ha_data ( thd, hton ) );
#else
	if ( have_sphinx_db!=SHOW_OPTION_YES )
	{
		my_message ( ER_NOT_SUPPORTED_YET,
			"failed to call SHOW SPHINX STATUS: --skip-sphinx was specified",
			MYF(0) );
		SPH_RET(TRUE);
	}
	CSphSEStats * pStats = (CSphSEStats*) thd->ha_data[sphinx_hton.slot];
#endif

	if ( pStats )
	{
		buf1len = my_snprintf ( buf1, sizeof(buf1),
			"total: %d, total found: %d, time: %d, words: %d", 
			pStats->m_iMatchesTotal, pStats->m_iMatchesFound, pStats->m_iQueryMsec, pStats->m_iWords );

#if MYSQL_VERSION_ID>50100
		stat_print ( thd, sphinx_hton_name, strlen(sphinx_hton_name),
			STRING_WITH_LEN("stats"), buf1, buf1len );
#else
		field_list.push_back ( new Item_empty_string ( "Type",10 ) );
		field_list.push_back ( new Item_empty_string ( "Name",FN_REFLEN ) );
		field_list.push_back ( new Item_empty_string ( "Status",10 ) );
		if ( protocol->send_fields ( &field_list, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF ) )
			SPH_RET(TRUE);

		protocol->prepare_for_resend ();
		protocol->store ( STRING_WITH_LEN("SPHINX"), system_charset_info );
		protocol->store ( STRING_WITH_LEN("stats"), system_charset_info );
		protocol->store ( buf1, buf1len, system_charset_info );
		if ( protocol->write() )
			SPH_RET(TRUE);
#endif

		if ( pStats->m_iWords )
		{
			for ( int i=0; i<pStats->m_iWords; i++ )
			{
				CSphSEWordStats & tWord = pStats->m_dWords[i];
				buf2len = my_snprintf ( buf2, sizeof(buf2), "%s%s:%d:%d ",
					buf2, tWord.m_sWord, tWord.m_iDocs, tWord.m_iHits );
			}

#if MYSQL_VERSION_ID>50100
			stat_print ( thd, sphinx_hton_name, strlen(sphinx_hton_name),
				STRING_WITH_LEN("words"), buf2, buf2len );
#else
			protocol->prepare_for_resend ();
			protocol->store ( STRING_WITH_LEN("SPHINX"), system_charset_info );
			protocol->store ( STRING_WITH_LEN("words"), system_charset_info );
			protocol->store ( buf2, buf2len, system_charset_info );
			if ( protocol->write() )
				SPH_RET(TRUE);
#endif
		}

		// send last error or warning
		if ( pStats->m_sLastMessage && pStats->m_sLastMessage[0] )
		{
			const char * sMessageType = pStats->m_bLastError ? "error" : "warning";

#if MYSQL_VERSION_ID>50100
			stat_print ( thd, sphinx_hton_name, strlen(sphinx_hton_name),
				sMessageType, strlen(sMessageType), pStats->m_sLastMessage, strlen(pStats->m_sLastMessage) );
#else
			protocol->prepare_for_resend ();
			protocol->store ( STRING_WITH_LEN("SPHINX"), system_charset_info );
			protocol->store ( sMessageType, strlen(sMessageType), system_charset_info );
			protocol->store ( pStats->m_sLastMessage, strlen(pStats->m_sLastMessage), system_charset_info );
			if ( protocol->write() )
				SPH_RET(TRUE);
#endif
		}

	} else
	{
		#if MYSQL_VERSION_ID < 50100
		field_list.push_back ( new Item_empty_string ( "Type", 10 ) );
		field_list.push_back ( new Item_empty_string ( "Name", FN_REFLEN ) );
		field_list.push_back ( new Item_empty_string ( "Status", 10 ) );
		if ( protocol->send_fields ( &field_list, Protocol::SEND_NUM_ROWS | Protocol::SEND_EOF ) )
			SPH_RET(TRUE);

		protocol->prepare_for_resend ();
		protocol->store ( STRING_WITH_LEN("SPHINX"), system_charset_info );
		protocol->store ( STRING_WITH_LEN("stats"), system_charset_info );
		protocol->store ( STRING_WITH_LEN("no query has been executed yet"), system_charset_info );
		if ( protocol->write() )
			SPH_RET(TRUE);
		#endif
	}

	#if MYSQL_VERSION_ID < 50100
	send_eof(thd);
	#endif

	SPH_RET(FALSE);
}

//////////////////////////////////////////////////////////////////////////////
// HELPERS
//////////////////////////////////////////////////////////////////////////////

static char * sphDup ( const char * sSrc, int iLen=-1 )
{
	if ( !sSrc )
		return NULL;

	if ( iLen<0 )
		iLen = strlen(sSrc);

	char * sRes = new char [ 1+iLen ];
	memcpy ( sRes, sSrc, iLen );
	sRes[iLen] = '\0';
	return sRes;
}


static void sphLogError ( const char * sFmt, ... )
{
	// emit timestamp
#ifdef __WIN__
	SYSTEMTIME t;
	GetLocalTime ( &t );

	fprintf ( stderr, "%02d%02d%02d %2d:%02d:%02d SphinxSE: internal error: ",
		(int)t.wYear % 100, (int)t.wMonth, (int)t.wDay,
		(int)t.wHour, (int)t.wMinute, (int)t.wSecond );
#else
	// Unix version
	time_t tStamp;
	time ( &tStamp );

	struct tm * pParsed;
#ifdef HAVE_LOCALTIME_R
	struct tm tParsed;
	localtime_r ( &tStamp, &tParsed );
	pParsed = &tParsed;
#else
	pParsed = localtime ( &tStamp );
#endif // HAVE_LOCALTIME_R

	fprintf ( stderr, "%02d%02d%02d %2d:%02d:%02d SphinxSE: internal error: ",
		pParsed->tm_year % 100, pParsed->tm_mon + 1, pParsed->tm_mday,
		pParsed->tm_hour, pParsed->tm_min, pParsed->tm_sec);
#endif // __WIN__

	// emit message
	va_list ap;
	va_start ( ap, sFmt );
	vfprintf ( stderr, sFmt, ap );
	va_end ( ap );

	// emit newline
	fprintf ( stderr, "\n" );
}



// the following scheme variants are recognized
//
// sphinx://host/index
// sphinx://host:port/index
static bool ParseUrl ( CSphSEShare * share, TABLE * table, bool bCreate )
{
	SPH_ENTER_FUNC();

	if ( share )
	{
		// check incoming stuff
		if ( !table )
		{
			sphLogError ( "table==NULL in ParseUrl()" ); 
			return false;
		}
		if ( !table->s )
		{
			sphLogError ( "(table->s)==NULL in ParseUrl()" ); 
			return false;
		}

		// free old stuff
		share->ResetTable ();

		// fill new stuff
		share->m_iTableFields = table->s->fields;
		if ( share->m_iTableFields )
		{
			share->m_sTableField = new char * [ share->m_iTableFields ];
			share->m_eTableFieldType = new enum_field_types [ share->m_iTableFields ];

			for ( int i=0; i<share->m_iTableFields; i++ )
			{
				share->m_sTableField[i] = sphDup ( table->field[i]->field_name );
				share->m_eTableFieldType[i] = table->field[i]->type();
			}
		}
	}

	char * sScheme = NULL;
	char * sHost = SPHINXSE_DEFAULT_HOST;
	char * sIndex = SPHINXSE_DEFAULT_INDEX;
	int iPort = SPHINXSE_DEFAULT_PORT;

	bool bOk = true;
	while ( table->s->connect_string.length!=0 )
	{
		bOk = false;
		sScheme = sphDup ( table->s->connect_string.str, table->s->connect_string.length );

		sHost = strstr ( sScheme, "://" );
		if ( !sHost )
			break;
		sHost[0] = '\0';
		sHost += 3;

		if ( strcmp ( sScheme, "sphinx" )!=0 )
			break;

		char * sPort = strchr ( sHost, ':' );
		if ( sPort )
		{
			*sPort++ = '\0';
			if ( *sPort )
			{
				sIndex = strchr ( sPort, '/' );
				if ( sIndex )
					*sIndex++ = '\0'; 
				else
					sIndex = SPHINXSE_DEFAULT_INDEX;

				iPort = atoi(sPort);
				if ( !iPort )
					iPort = SPHINXSE_DEFAULT_PORT;
			}
		} else
		{
			sIndex = strchr ( sHost, '/' );
			if ( sIndex )
				*sIndex++ = '\0';
			else
				sIndex = SPHINXSE_DEFAULT_INDEX;
		}

		bOk = true;
		break;
	}

	if ( !bOk )
	{
		my_error ( bCreate ? ER_FOREIGN_DATA_STRING_INVALID_CANT_CREATE : ER_FOREIGN_DATA_STRING_INVALID,
			MYF(0), table->s->connect_string );
	} else
	{
		if ( share )
		{
			SafeDeleteArray ( share->m_sScheme );
			share->m_sScheme = sScheme;
			share->m_sHost = sHost;
			share->m_sIndex = sIndex;
			share->m_iPort = (ushort)iPort;
		}
	}
	if ( !bOk && !share )
		SafeDeleteArray ( sScheme );

	SPH_RET(bOk);
}


// Example of simple lock controls. The "share" it creates is structure we will
// pass to each sphinx handler. Do you have to have one of these? Well, you have
// pieces that are used for locking, and they are needed to function.
static CSphSEShare * get_share ( const char * table_name, TABLE * table )
{
	SPH_ENTER_FUNC();
	pthread_mutex_lock ( &sphinx_mutex );

	CSphSEShare * pShare = NULL;
	for ( ;; )
	{
		// check if we already have this share
		pShare = (CSphSEShare*) hash_search ( &sphinx_open_tables, (byte*)table_name, strlen(table_name) );
		if ( pShare )
		{
			pShare->m_iUseCount++;
			break;
		}

		// try to allocate new share
		pShare = new CSphSEShare ();
		if ( !pShare )
			break;

		// try to setup it
		if ( !ParseUrl ( pShare, table, false ) )
		{
			SafeDelete ( pShare );
			break;
		}

		// try to hash it
		pShare->m_iTableNameLen = strlen(table_name);
		pShare->m_sTable = sphDup ( table_name );
		if ( my_hash_insert ( &sphinx_open_tables, (byte*) pShare ) )
		{
			SafeDelete ( pShare );
			break;
		}

		// all seems fine
		break;
	}

	pthread_mutex_unlock ( &sphinx_mutex );
	SPH_RET(pShare);
}


// Free lock controls. We call this whenever we close a table. If the table had
// the last reference to the share then we free memory associated with it.
static int free_share ( CSphSEShare * pShare )
{
	SPH_ENTER_FUNC();
	pthread_mutex_lock ( &sphinx_mutex );

	if ( !--pShare->m_iUseCount )
	{
		hash_delete ( &sphinx_open_tables, (byte*) pShare );
		SafeDelete ( pShare );
	}

	pthread_mutex_unlock ( &sphinx_mutex );
	SPH_RET(0);
}


#if MYSQL_VERSION_ID>50100
static handler * sphinx_create_handler ( handlerton * hton, TABLE_SHARE * table, MEM_ROOT * mem_root )
{
	return new ( mem_root ) ha_sphinx ( hton, table );
}
#endif

//////////////////////////////////////////////////////////////////////////////
// CLIENT-SIDE REQUEST STUFF
//////////////////////////////////////////////////////////////////////////////

CSphSEQuery::CSphSEQuery ( const char * sQuery, int iLength, const char * sIndex )
	: m_sIndex ( sIndex ? sIndex : "*" )
	, m_iOffset ( 0 )
	, m_iLimit ( 20 )
	, m_sQuery ( "" )
	, m_pWeights ( NULL )
	, m_iWeights ( 0 )
	, m_eMode ( SPH_MATCH_ALL )
	, m_eSort ( SPH_SORT_RELEVANCE )
	, m_sSortBy ( "" )
	, m_iMaxMatches ( 1000 )
	, m_iMinID ( 0 )
	, m_iMaxID ( UINT_MAX )
	, m_iFilters ( 0 )
	, m_eGroupFunc ( SPH_GROUPBY_DAY )
	, m_sGroupBy ( "" )
	, m_sGroupSortBy ( "@group desc" )
	, m_iCutoff ( 0 )

	, m_pBuf ( NULL )
	, m_pCur ( NULL )
	, m_iBufLeft ( 0 )
	, m_bBufOverrun ( false )
{
	m_sQueryBuffer = new char [ iLength+2 ];
	memcpy ( m_sQueryBuffer, sQuery, iLength );
	m_sQueryBuffer[iLength]= ';';
	m_sQueryBuffer[iLength+1]= '\0';

}


CSphSEQuery::~CSphSEQuery ()
{
	SPH_ENTER_METHOD();
	SafeDeleteArray ( m_sQueryBuffer );
	SafeDeleteArray ( m_pWeights );
	SafeDeleteArray ( m_pBuf );
	SPH_VOID_RET();
}


int CSphSEQuery::ParseArray ( uint32 ** ppValues, const char * sValue )
{
	SPH_ENTER_METHOD();

//	assert ( ppValues );
//	assert ( !(*ppValues) );

	const char * p;
	bool bPrevDigit = false;
	int iValues = 0;

	// count the values
	for ( p=sValue; *p; p++ )
	{
		bool bDigit = ( (*p)>='0' && (*p)<='9' );
		if ( bDigit && !bPrevDigit )
			iValues++;
		bPrevDigit = bDigit;
	}
	if ( !iValues )
		SPH_RET(0);

	// extract the values
	uint32 * pValues = new uint32 [ iValues ];
	*ppValues = pValues;

	int iIndex = 0;
	uint32 uValue = 0;

	bPrevDigit = false;
	for ( p=sValue; ; p++ )
	{
		bool bDigit = ( (*p)>='0' && (*p)<='9' );

		if ( bDigit )
		{
			if ( !bPrevDigit )
				uValue = 0;
			uValue = uValue*10 + ( (*p)-'0' );
		}

		if ( !bDigit && bPrevDigit )
		{
			assert ( iIndex<iValues );
			pValues [ iIndex++ ] = uValue;
		}

		bPrevDigit = bDigit;

		if ( !(*p) )
			break;
	}

	SPH_RET(iValues);
}


static char * chop ( char * s )
{
	while ( *s && isspace(*s) )
		s++;

	char * p = s + strlen(s);
	while ( p>s && isspace(p[-1]) )
		p--;
	*p = '\0';

	return s;
}


static bool myisattr ( char c )
{
	return
		( c>='0' && c<='9' ) ||
		( c>='a' && c<='z' ) ||
		( c>='A' && c<='Z' ) ||
		c=='_';
}


bool CSphSEQuery::ParseField ( char * sField )
{
	SPH_ENTER_METHOD();

	// look for option name/value separator
	char * sValue = strchr ( sField, '=' );
	if ( !sValue )
	{
		// by default let's assume it's just query
		if ( sField[0] )
			m_sQuery = sField;
		SPH_RET(true);
	}

	// split
	*sValue++ = '\0';
	sValue = chop ( sValue );
	int iValue = atoi ( sValue );

	// handle options
	char * sName = chop ( sField );

	if ( !strcmp ( sName, "query" ) )			m_sQuery = sValue;
	else if ( !strcmp ( sName, "index" ) )		m_sIndex = sValue;
	else if ( !strcmp ( sName, "offset" ) )		m_iOffset = iValue;
	else if ( !strcmp ( sName, "limit" ) )		m_iLimit = iValue;
	else if ( !strcmp ( sName, "weights" ) )	m_iWeights = ParseArray ( &m_pWeights, sValue );
	else if ( !strcmp ( sName, "minid" ) )		m_iMinID = iValue;
	else if ( !strcmp ( sName, "maxid" ) )		m_iMaxID = iValue;
	else if ( !strcmp ( sName, "maxmatches" ) )	m_iMaxMatches = iValue;
	else if ( !strcmp ( sName, "groupsort" ) )	m_sGroupSortBy = sValue;
	else if ( !strcmp ( sName, "cutoff" ) )		m_iCutoff = iValue;

	else if ( !strcmp ( sName, "mode" ) )
	{

		m_eMode = SPH_MATCH_ALL;
		if ( !strcmp ( sValue, "any") )				m_eMode = SPH_MATCH_ANY;
		else if ( !strcmp ( sValue, "phrase" ) )	m_eMode = SPH_MATCH_PHRASE;
		else if ( !strcmp ( sValue, "boolean") )	m_eMode = SPH_MATCH_BOOLEAN;
		else if ( !strcmp ( sValue, "extended") )	m_eMode = SPH_MATCH_EXTENDED;
		else if ( !strcmp ( sValue, "all") )		m_eMode = SPH_MATCH_ALL;
		else
		{
			snprintf ( m_sParseError, sizeof(m_sParseError), "unknown matching mode '%s'", sValue );
			SPH_RET(false);
		}

	} else if ( !strcmp ( sName, "sort" ) )
	{
		static const struct 
		{
			const char *	m_sName;
			ESphSortOrder	m_eSort;
		} dSortModes[] = 
		{
			{ "relevance",		SPH_SORT_RELEVANCE },
			{ "attr_desc:",		SPH_SORT_ATTR_DESC },
			{ "attr_asc:",		SPH_SORT_ATTR_ASC },
			{ "time_segments:",	SPH_SORT_TIME_SEGMENTS },
			{ "extended:",		SPH_SORT_EXTENDED },
		};

		int i;
		const int nModes = sizeof(dSortModes)/sizeof(dSortModes[0]);
		for ( i=0; i<nModes; i++ )
			if ( !strncmp ( sValue, dSortModes[i].m_sName, strlen(dSortModes[i].m_sName) ) )
		{
			m_eSort = dSortModes[i].m_eSort;
			m_sSortBy = sValue + strlen(dSortModes[i].m_sName);
			break;
		}
		if ( i==nModes )
		{
			snprintf ( m_sParseError, sizeof(m_sParseError), "unknown sorting mode '%s'", sValue );
			SPH_RET(false);
		}

	} else if ( !strcmp ( sName, "groupby" ) )
	{
		static const struct 
		{
			const char *	m_sName;
			ESphGroupBy		m_eFunc;
		} dGroupModes[] = 
		{
			{ "day:",	SPH_GROUPBY_DAY },
			{ "week:",	SPH_GROUPBY_WEEK },
			{ "month:",	SPH_GROUPBY_MONTH },
			{ "year:",	SPH_GROUPBY_YEAR },
			{ "attr:",	SPH_GROUPBY_ATTR },
		};

		int i;
		const int nModes = sizeof(dGroupModes)/sizeof(dGroupModes[0]);
		for ( i=0; i<nModes; i++ )
			if ( !strncmp ( sValue, dGroupModes[i].m_sName, strlen(dGroupModes[i].m_sName) ) )
		{
			m_eGroupFunc = dGroupModes[i].m_eFunc;
			m_sGroupBy = sValue + strlen(dGroupModes[i].m_sName);
			break;
		}
		if ( i==nModes )
		{
			snprintf ( m_sParseError, sizeof(m_sParseError), "unknown groupby mode '%s'", sValue );
			SPH_RET(false);
		}

	} else if ( m_iFilters<SPHINXSE_MAX_FILTERS &&
		( !strcmp ( sName, "range" ) || !strcmp ( sName, "!range" ) ) )
	{
		for ( ;; )
		{
			CSphSEFilter & tFilter = m_dFilters [ m_iFilters ];
			tFilter.m_bExclude = ( strcmp ( sName, "!range")==0 );
			char * p;

			if (!( p = strchr ( sValue, ',' ) ))
				break;
			*p++ = '\0';

			tFilter.m_sAttrName = chop ( sValue );
			sValue = p;

			if (!( p = strchr ( sValue, ',' ) ))
				break;
			*p++ = '\0';

			tFilter.m_uMinValue = atoi(sValue);
			tFilter.m_uMaxValue = atoi(p);

			// all ok
			m_iFilters++;
			break;
		}

	} else if ( m_iFilters<SPHINXSE_MAX_FILTERS &&
		( !strcmp ( sName, "filter" ) || !strcmp ( sName, "!filter" ) ) )
	{
		for ( ;; )
		{
			CSphSEFilter & tFilter = m_dFilters [ m_iFilters ];
			tFilter.m_bExclude = ( strcmp ( sName, "!filter")==0 );

			// get the attr name
			while ( (*sValue) && !myisattr(*sValue) )
				sValue++;
			if ( !*sValue )
				break;

			tFilter.m_sAttrName = sValue;
			while ( (*sValue) && myisattr(*sValue) )
				sValue++;
			if ( !*sValue )
				break;
			*sValue++ = '\0';

			// get the values
			tFilter.m_iValues = ParseArray ( &tFilter.m_pValues, sValue );
			if ( !tFilter.m_iValues )
			{
				assert ( !tFilter.m_pValues );
				break;
			}

			// all ok
			m_iFilters++;
			break;
		}

	} else
	{
		snprintf ( m_sParseError, sizeof(m_sParseError), "unknown parameter '%s'", sName );
		SPH_RET(false);
	}

	// !COMMIT handle syntax errors

	SPH_RET(true);
}


bool CSphSEQuery::Parse ()
{
	SPH_ENTER_METHOD();
	SPH_DEBUG ( "query [[ %s ]]", m_sQueryBuffer );

	char * pNext;
	char * pCur = m_sQueryBuffer;

	while (( pNext = strchr(  pCur, ';' ) ))
	{
		*pNext++ = '\0';
		if ( !ParseField ( pCur ) )
			SPH_RET(false);
		pCur = pNext;
	}

	SPH_RET(true);
}


void CSphSEQuery::SendBytes ( const void * pBytes, int iBytes )
{
	SPH_ENTER_METHOD();
	if ( m_iBufLeft<iBytes )
	{
		m_bBufOverrun = true;
		SPH_VOID_RET();
	}

	memcpy ( m_pCur, pBytes, iBytes );

	m_pCur += iBytes;
	m_iBufLeft -= iBytes;
	SPH_VOID_RET();
}


int CSphSEQuery::BuildRequest ( char ** ppBuffer )
{
	SPH_ENTER_METHOD();

	// calc request length
	int iReqSize = 76 + 4*m_iWeights
		+ strlen ( m_sSortBy )
		+ strlen ( m_sQuery )
		+ strlen ( m_sIndex )
		+ strlen ( m_sGroupBy )
		+ strlen ( m_sGroupSortBy );

	for ( int i=0; i<m_iFilters; i++ )
	{
		const CSphSEFilter & tFilter = m_dFilters[i];
		iReqSize +=
			12
			+ strlen ( tFilter.m_sAttrName )
			+ 4*tFilter.m_iValues
			+ ( tFilter.m_iValues ? 0 : 8 );
	}

	m_iBufLeft = 0;
	SafeDeleteArray ( m_pBuf );

	m_pBuf = new char [ iReqSize ];
	if ( !m_pBuf )
		SPH_RET(-1);

	m_pCur = m_pBuf;
	m_iBufLeft = iReqSize;
	m_bBufOverrun = false;
	(*ppBuffer) = m_pBuf;

	// build request
	SendWord ( SEARCHD_COMMAND_SEARCH ); // command id
	SendWord ( VER_COMMAND_SEARCH ); // command version
	SendInt ( iReqSize-8 ); // request body length

	SendInt ( m_iOffset );
	SendInt ( m_iLimit );
	SendInt ( m_eMode );
	SendInt ( m_eSort );
	SendString ( m_sSortBy ); // sort attr
	SendString ( m_sQuery ); // query
	SendInt ( m_iWeights );
	for ( int j=0; j<m_iWeights; j++ )
		SendInt ( m_pWeights[j] ); // weights
	SendString ( m_sIndex ); // indexes
	SendInt ( 0 ); // id32 range follows
	SendInt ( m_iMinID ); // id/ts ranges
	SendInt ( m_iMaxID );

	SendInt ( m_iFilters );
	for ( int j=0; j<m_iFilters; j++ )
	{
		const CSphSEFilter & tFilter = m_dFilters[j];
		SendString ( tFilter.m_sAttrName );
		SendInt ( tFilter.m_iValues );
		for ( int k=0; k<tFilter.m_iValues; k++ )
			SendInt ( tFilter.m_pValues[k] );
		if ( !tFilter.m_iValues )
		{
			SendDword ( tFilter.m_uMinValue );
			SendDword ( tFilter.m_uMaxValue );
		}
		SendInt ( tFilter.m_bExclude );
	}

	SendInt ( m_eGroupFunc );
	SendString ( m_sGroupBy );
	SendInt ( m_iMaxMatches );
	SendString ( m_sGroupSortBy );
	SendInt ( m_iCutoff );

	// detect buffer overruns and underruns, and report internal error
	if ( m_bBufOverrun || m_iBufLeft!=0 || m_pCur-m_pBuf!=iReqSize )
		SPH_RET(-1);

	// all fine
	SPH_RET(iReqSize);
}

//////////////////////////////////////////////////////////////////////////////
// SPHINX HANDLER
//////////////////////////////////////////////////////////////////////////////

static const char * ha_sphinx_exts[] = { NullS };


#if MYSQL_VERSION_ID<50100
ha_sphinx::ha_sphinx ( TABLE_ARG * table )
	: handler ( &sphinx_hton, table )
#else
ha_sphinx::ha_sphinx ( handlerton * hton, TABLE_ARG * table )
	: handler ( hton, table )
#endif
	, m_pShare ( NULL )
	, m_iStartOfScan ( 0 )
	, m_iMatchesTotal ( 0 )
	, m_iCurrentPos ( 0 )
	, m_pCurrentKey ( NULL )
	, m_iCurrentKeyLen ( 0 )
	, m_pResponse ( NULL )
	, m_pResponseEnd ( NULL )
	, m_pCur ( NULL )
	, m_bUnpackError ( false )
	, m_iFields ( 0 )
	, m_dFields ( NULL )
	, m_iAttrs ( 0 )
	, m_dAttrs ( NULL )
	, m_bId64 ( 0 )
	, m_dUnboundFields ( NULL )
{
	SPH_ENTER_METHOD();
	SPH_VOID_RET();
}


// If frm_error() is called then we will use this to to find out what file extentions
// exist for the storage engine. This is also used by the default rename_table and
// delete_table method in handler.cc.
const char ** ha_sphinx::bas_ext() const
{
	return ha_sphinx_exts;
}


// Used for opening tables. The name will be the name of the file.
// A table is opened when it needs to be opened. For instance
// when a request comes in for a select on the table (tables are not
// open and closed for each request, they are cached).
//
// Called from handler.cc by handler::ha_open(). The server opens all tables by
// calling ha_open() which then calls the handler specific open().
int ha_sphinx::open ( const char * name, int, uint )
{
	SPH_ENTER_METHOD();
	m_pShare = get_share ( name, table );
	if ( !m_pShare )
		SPH_RET(1);

	thr_lock_data_init ( &m_pShare->m_tLock, &m_tLock, NULL );

	#if MYSQL_VERSION_ID>50100
	*thd_ha_data ( table->in_use, ht ) = NULL;
	#else
	table->in_use->ha_data [ sphinx_hton.slot ] = NULL;
	#endif

	SPH_RET(0);
}


int ha_sphinx::ConnectToSearchd ()
{
	SPH_ENTER_METHOD();

	struct sockaddr_in sa;
	in_addr_t ip_addr;
	int version;
	uint uClientVersion = htonl ( SPHINX_SEARCHD_PROTO );

	memset ( &sa, 0, sizeof(sa) );
	sa.sin_family = AF_INET;

	// prepare host address
	if ( (int)( ip_addr=inet_addr(m_pShare->m_sHost) ) != (int)INADDR_NONE )
	{ 
		memcpy ( &sa.sin_addr, &ip_addr, sizeof(ip_addr) );
	} else
	{
		int tmp_errno;
		struct hostent tmp_hostent, *hp;
		char buff2 [ GETHOSTBYNAME_BUFF_SIZE ];

		hp = my_gethostbyname_r ( m_pShare->m_sHost, &tmp_hostent,
			buff2, sizeof(buff2), &tmp_errno );
		if ( !hp )
		{ 
			my_gethostbyname_r_free();

			char sError[256];
			my_snprintf ( sError, sizeof(sError), "failed to resolve searchd host (name=%s)", m_pShare->m_sHost );

			my_error ( ER_CONNECT_TO_FOREIGN_DATA_SOURCE, MYF(0), sError );
			SPH_RET(-1);
		}

		memcpy ( &sa.sin_addr, hp->h_addr,
			min ( sizeof(sa.sin_addr), (size_t)hp->h_length ) );
		my_gethostbyname_r_free();
	}

	sa.sin_port = htons(m_pShare->m_iPort);

	char sError[256];
	int iSocket = socket ( AF_INET, SOCK_STREAM, 0 );

	if ( iSocket<0 )
	{
		my_error ( ER_CONNECT_TO_FOREIGN_DATA_SOURCE, MYF(0), "failed to create client socket" );
		SPH_RET(-1);
	}

	if ( connect ( iSocket, (struct sockaddr *) &sa, sizeof(sa) )<0 )
	{
		my_snprintf ( sError, sizeof(sError), "failed to connect to searchd (host=%s, port=%d)",
			m_pShare->m_sHost, m_pShare->m_iPort );
		my_error ( ER_CONNECT_TO_FOREIGN_DATA_SOURCE, MYF(0), sError );
		SPH_RET(-1);
	}

	if ( ::recv ( iSocket, (char *)&version, sizeof(version), 0 )!=sizeof(version) )
	{
		::close ( iSocket );
		my_snprintf ( sError, sizeof(sError), "failed to receive searchd version (host=%s, port=%d)",
			m_pShare->m_sHost, m_pShare->m_iPort );
		my_error ( ER_CONNECT_TO_FOREIGN_DATA_SOURCE, MYF(0), sError );
		SPH_RET(-1);
	}

	if ( ::send ( iSocket, (char*)&uClientVersion, sizeof(uClientVersion), 0 )!=sizeof(uClientVersion) )
	{
		::close ( iSocket );
		my_snprintf ( sError, sizeof(sError), "failed to send client version (host=%s, port=%d)",
			m_pShare->m_sHost, m_pShare->m_iPort );
		my_error ( ER_CONNECT_TO_FOREIGN_DATA_SOURCE, MYF(0), sError );
		SPH_RET(-1);
	}

	SPH_RET(iSocket);
}


// Closes a table. We call the free_share() function to free any resources
// that we have allocated in the "shared" structure.
//
// Called from sql_base.cc, sql_select.cc, and table.cc.
// In sql_select.cc it is only used to close up temporary tables or during
// the process where a temporary table is converted over to being a
// myisam table.
// For sql_base.cc look at close_data_tables().
int ha_sphinx::close()
{
	SPH_ENTER_METHOD();
	SPH_RET ( free_share(m_pShare) );
}


int ha_sphinx::write_row ( byte * )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_WRONG_COMMAND );
}


int ha_sphinx::update_row ( const byte *, byte * )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_WRONG_COMMAND );
}


int ha_sphinx::delete_row ( const byte * )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_WRONG_COMMAND );
}


// keynr is key (index) number
// sorted is 1 if result MUST be sorted according to index
int ha_sphinx::index_init ( uint keynr, bool )
{
	SPH_ENTER_METHOD();
	m_iStartOfScan = 1;
	active_index = keynr;
	SPH_RET(0);
}


int ha_sphinx::index_end()
{
	SPH_ENTER_METHOD();
	SPH_RET(0);
}


uint32 ha_sphinx::UnpackDword ()
{
	if ( m_pCur+sizeof(uint32)>m_pResponseEnd )
	{
		m_pCur = m_pResponseEnd;
		m_bUnpackError = true;
		return 0;
	}

	uint32 uRes = ntohl ( *( (uint32*)m_pCur ) );
	m_pCur += sizeof(uint32);
	return uRes;
}


char * ha_sphinx::UnpackString ()
{
	uint32 iLen = UnpackDword ();
	if ( !iLen )
		return NULL;

	if ( m_pCur+iLen>m_pResponseEnd )
	{
		m_pCur = m_pResponseEnd;
		m_bUnpackError = true;
		return NULL;
	}

	char * sRes = new char [ 1+iLen ];
	memcpy ( sRes, m_pCur, iLen );
	sRes[iLen] = '\0';
	m_pCur += iLen;
	return sRes;
}


static inline const char * FixNull ( const char * s )
{
	return s ? s : "(null)";
}


bool ha_sphinx::UnpackSchema ()
{
	SPH_ENTER_METHOD();

	// unpack network packet
	if ( m_dFields )
		for ( int i=0; i<(int)m_iFields; i++ )
			SafeDeleteArray ( m_dFields[i] );
	SafeDeleteArray ( m_dFields );

	m_iFields = UnpackDword ();
	m_dFields = new char * [ m_iFields ];
	if ( !m_dFields )
		SPH_RET(false);

	for ( uint32 i=0; i<m_iFields; i++ )
		m_dFields[i] = UnpackString ();

	SafeDeleteArray ( m_dAttrs );
	m_iAttrs = UnpackDword ();
	m_dAttrs = new CSphSEAttr [ m_iAttrs ];
	if ( !m_dAttrs )
	{
		for ( int i=0; i<(int)m_iFields; i++ )
			SafeDeleteArray ( m_dFields[i] );
		SafeDeleteArray ( m_dFields );
		SPH_RET(false);
	}

	for ( uint32 i=0; i<m_iAttrs; i++ )
	{
		m_dAttrs[i].m_sName = UnpackString ();
		m_dAttrs[i].m_eType = (ESphAttrType) UnpackDword ();

		m_dAttrs[i].m_iField = -1;
		for ( int j=SPHINXSE_SYSTEM_COLUMNS; j<m_pShare->m_iTableFields; j++ )
		{
			const char * sTableField = m_pShare->m_sTableField[j];
			const char * sAttrField = m_dAttrs[i].m_sName;
			if ( m_dAttrs[i].m_sName[0]=='@' )
			{
				const char * sAtPrefix = "_sph_";
				if ( strncmp ( sTableField, sAtPrefix, strlen(sAtPrefix) ) )
					continue;
				sTableField += strlen(sAtPrefix);
				sAttrField++;
			}

			if ( !strcasecmp ( sAttrField, sTableField ) )
			{
				m_dAttrs[i].m_iField = j;
				break;
			}
		}
	}

	m_iMatchesTotal = UnpackDword ();
	m_bId64 = UnpackDword ();

	// network packet unpacked; build unbound fields map
	SafeDeleteArray ( m_dUnboundFields );
	m_dUnboundFields = new int [ m_pShare->m_iTableFields ];

	for ( int i=0; i<m_pShare->m_iTableFields; i++ )
	{
		if ( i<SPHINXSE_SYSTEM_COLUMNS )
			m_dUnboundFields[i] = SPH_ATTR_NONE;

		else if ( m_pShare->m_eTableFieldType[i]==MYSQL_TYPE_TIMESTAMP )
			m_dUnboundFields[i] = SPH_ATTR_TIMESTAMP;

		else
			m_dUnboundFields[i] = SPH_ATTR_INTEGER;
	}

	for ( uint32 i=0; i<m_iAttrs; i++ )
		if ( m_dAttrs[i].m_iField>=0 )
			m_dUnboundFields [ m_dAttrs[i].m_iField ] = SPH_ATTR_NONE;

	SPH_RET(!m_bUnpackError);
}


bool ha_sphinx::UnpackStats ( CSphSEStats * pStats )
{
	assert ( pStats );

	char * pCurSave = m_pCur;
	m_pCur = m_pCur + m_iMatchesTotal*(2+m_bId64+m_iAttrs)*sizeof(uint); // id+weight+attrs

	pStats->m_iMatchesTotal = UnpackDword ();
	pStats->m_iMatchesFound = UnpackDword ();
	pStats->m_iQueryMsec = UnpackDword ();
	pStats->m_iWords = UnpackDword ();

	if ( m_bUnpackError )
		return false;

	SafeDeleteArray ( pStats->m_dWords );
	pStats->m_dWords = new CSphSEWordStats [ pStats->m_iWords ]; // !COMMIT not bad-value safe
	if ( !pStats->m_dWords )
		return false;

	for ( int i=0; i<pStats->m_iWords; i++ )
	{
		CSphSEWordStats & tWord = pStats->m_dWords[i];
		tWord.m_sWord = UnpackString ();
		tWord.m_iDocs = UnpackDword ();
		tWord.m_iHits = UnpackDword ();
	}

	if ( m_bUnpackError )
		return false;

	m_pCur = pCurSave;
	return true;
}


// Positions an index cursor to the index specified in the handle. Fetches the
// row if available. If the key value is null, begin at the first key of the
// index.
int ha_sphinx::index_read ( byte * buf, const byte * key, uint key_len, enum ha_rkey_function )
{
	SPH_ENTER_METHOD();
	char sError[256];

	m_pCurrentKey = key;
	m_iCurrentKeyLen = key_len;
	m_iStartOfScan = 1;

	// parse query
	CSphSEQuery q ( (char*)key+HA_KEY_BLOB_LENGTH, uint2korr(key), m_pShare->m_sIndex );
	if ( !q.Parse () )
	{
		my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), q.m_sParseError );
		SPH_RET ( HA_ERR_END_OF_FILE );
	}

	// do connect
	int iSocket = ConnectToSearchd ();
	if ( iSocket<0 )
		SPH_RET ( HA_ERR_END_OF_FILE );

	// my buffer
	char * pBuffer; // will be free by CSphSEQuery dtor; do NOT free manually
	int iReqLen = q.BuildRequest ( &pBuffer );

	if ( iReqLen<=0 )
	{
		my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), "INTERNAL ERROR: q.BuildRequest() failed" );
		SPH_RET ( HA_ERR_END_OF_FILE );
	}

	// send request
	::send ( iSocket, pBuffer, iReqLen, 0 );

	// receive reply
	char sHeader[8];
	if ( ::recv ( iSocket, sHeader, sizeof(sHeader), MSG_WAITALL )!=sizeof(sHeader) )
	{
		my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), "failed to receive response header (searchd went away?)" );
		SPH_RET ( HA_ERR_END_OF_FILE );
	}

	short int uRespStatus = ntohs ( *(short int*)( &sHeader[0] ) );
	short int uRespVersion = ntohs ( *(short int*)( &sHeader[2] ) );
	uint uRespLength = ntohl ( *(uint *)( &sHeader[4] ) );
	SPH_DEBUG ( "got response header (status=%d version=%d length=%d)",
		uRespStatus, uRespVersion, uRespLength );

	SafeDeleteArray ( m_pResponse );
	if ( uRespLength<=SPHINXSE_MAX_ALLOC )
		m_pResponse = new char [ uRespLength+1 ];

	if ( !m_pResponse )
	{
		my_snprintf ( sError, sizeof(sError), "bad searchd response length (length=%u)", uRespLength );
		my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), sError );
		SPH_RET ( HA_ERR_END_OF_FILE );
	}

	int iRecvLength = 0;
	while ( iRecvLength<(int)uRespLength )
	{
		int iRecv = ::recv ( iSocket, m_pResponse+iRecvLength, uRespLength-iRecvLength, MSG_WAITALL );
		if ( iRecv<0 )
			break;
		iRecvLength += iRecv;
	}

	::close ( iSocket );
	iSocket = -1;

	if ( iRecvLength!=(int)uRespLength )
	{
		my_snprintf ( sError, sizeof(sError), "net read error (expected=%d, got=%d)", uRespLength, iRecvLength );
		my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), sError );
		SPH_RET ( HA_ERR_END_OF_FILE );
	}

	// set new data for thd->ha_data, it is used in show_status
	#if MYSQL_VERSION_ID>50100
	void ** tmp_ha_data = thd_ha_data ( table->in_use, ht );
	#define TARGET (*tmp_ha_data)
	#else
	#define TARGET (current_thd->ha_data[sphinx_hton.slot])
	#endif // >50100

	CSphSEStats * pStats = (CSphSEStats *) TARGET;
	if ( !pStats )
	{
		pStats = new CSphSEStats ();
		TARGET = pStats;
		if ( !pStats )
		{
			my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), "INTERNAL ERROR: malloc() failed" );
			SPH_RET ( HA_ERR_END_OF_FILE );
		}
	} else
	{
		pStats->Reset ();
	}

	// parse reply
	m_iCurrentPos = 0;
	m_pCur = m_pResponse;
	m_pResponseEnd = m_pResponse + uRespLength;
	m_bUnpackError = false;

	if ( uRespStatus!=SEARCHD_OK )
	{
		char * sMessage = UnpackString ();
		if ( !sMessage )
		{
			my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), "no valid response from searchd (status=%d, resplen=%d)",
				uRespStatus, uRespLength );
			SPH_RET ( HA_ERR_END_OF_FILE );
		}

		strncpy ( pStats->m_sLastMessage, sMessage, sizeof(pStats->m_sLastMessage) );
		SafeDeleteArray ( sMessage );

		if ( uRespStatus!=SEARCHD_WARNING )
		{
			my_snprintf ( sError, sizeof(sError), "searchd error: %s", pStats->m_sLastMessage );
			my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), sError );

			pStats->m_bLastError = true;
			SPH_RET ( HA_ERR_END_OF_FILE );
		}
	}

	if ( !UnpackSchema () )
	{
		my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), "INTERNAL ERROR: UnpackSchema() failed" );
		SPH_RET ( HA_ERR_END_OF_FILE );
	}

	if ( !UnpackStats ( pStats ) )
	{
		my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), "INTERNAL ERROR: UnpackStats() failed" );
		SPH_RET ( HA_ERR_END_OF_FILE );
	}

	SPH_RET ( get_rec ( buf, key, key_len ) );
}


// Positions an index cursor to the index specified in key. Fetches the
// row if any. This is only used to read whole keys.
int ha_sphinx::index_read_idx ( byte *, uint, const byte *, uint, enum ha_rkey_function )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_WRONG_COMMAND );
}


// Used to read forward through the index.
int ha_sphinx::index_next ( byte * buf )
{
	SPH_ENTER_METHOD();
	SPH_RET ( get_rec ( buf, m_pCurrentKey, m_iCurrentKeyLen ) );
}


int ha_sphinx::index_next_same ( byte * buf, const byte * key, uint keylen )
{
	SPH_ENTER_METHOD();
	SPH_RET ( get_rec ( buf, key, keylen ) );
}


int ha_sphinx::get_rec ( byte * buf, const byte * key, uint keylen )
{
	SPH_ENTER_METHOD();

	if ( m_iCurrentPos>=m_iMatchesTotal )
	{
		SafeDeleteArray ( m_pResponse );
		SPH_RET ( HA_ERR_END_OF_FILE ); 
	}

	#if MYSQL_VERSION_ID>50100
	my_bitmap_map * org_bitmap = dbug_tmp_use_all_columns ( table, table->write_set );
	#endif
	Field ** field = table->field;

	// unpack and return the match
	if ( m_bId64 )
		UnpackDword (); // FIXME! add full id64 support later here; for now, it ignores high bits
	uint32 uMatchID = UnpackDword ();
	uint32 uMatchWeight = UnpackDword ();

	field[0]->store ( uMatchID, 1 );
	field[1]->store ( uMatchWeight, 1 );
	field[2]->set_key_image ( (char*)key, keylen ); // store requested query. it's necessary, otherwise mysql goes crazy

	for ( uint32 i=0; i<m_iAttrs; i++ )
	{
		uint32 uValue = UnpackDword ();
		if ( m_dAttrs[i].m_iField<0 )
			continue;

		Field * af = field [ m_dAttrs[i].m_iField ];
		switch ( m_dAttrs[i].m_eType )
		{
			case SPH_ATTR_INTEGER:		af->store ( uValue, 1 ); break;
			case SPH_ATTR_TIMESTAMP:	longstore ( af->ptr, uValue ); break;
			default:
				my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0),
					"INTERNAL ERROR: unhandled attr type %d", m_dAttrs[i].m_eType );
				SafeDeleteArray ( m_pResponse );
				SPH_RET ( HA_ERR_END_OF_FILE ); 
		}
	}

	if ( m_bUnpackError )
	{
		my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0), "INTERNAL ERROR: response unpacker failed" );
		SafeDeleteArray ( m_pResponse );
		SPH_RET ( HA_ERR_END_OF_FILE ); 
	}

	// zero out unmapped fields
	for ( int i=SPHINXSE_SYSTEM_COLUMNS; i<(int)table->s->fields; i++ )
		if ( m_dUnboundFields[i]!=SPH_ATTR_NONE )
			switch ( m_dUnboundFields[i] )
	{
		case SPH_ATTR_INTEGER:		table->field[i]->store ( 0, 1 ); break;
		case SPH_ATTR_TIMESTAMP:	longstore ( table->field[i]->ptr, 0 ); break;
		default:
			my_error ( ER_QUERY_ON_FOREIGN_DATA_SOURCE, MYF(0),
				"INTERNAL ERROR: unhandled unbound field type %d", m_dUnboundFields[i] );
			SafeDeleteArray ( m_pResponse );
			SPH_RET ( HA_ERR_END_OF_FILE );
	}

	memset ( buf, 0, table->s->null_bytes );
	m_iCurrentPos++;

	#if MYSQL_VERSION_ID > 50100
	dbug_tmp_restore_column_map(table->write_set, org_bitmap);
	#endif

	SPH_RET(0);
}


// Used to read backwards through the index.
int ha_sphinx::index_prev ( byte * )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_WRONG_COMMAND );
}


// index_first() asks for the first key in the index.
//
// Called from opt_range.cc, opt_sum.cc, sql_handler.cc,
// and sql_select.cc.
int ha_sphinx::index_first ( byte * )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_END_OF_FILE );
}

// index_last() asks for the last key in the index.
//
// Called from opt_range.cc, opt_sum.cc, sql_handler.cc,
// and sql_select.cc.
int ha_sphinx::index_last ( byte * )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_WRONG_COMMAND );
}


int ha_sphinx::rnd_init ( bool )
{
	SPH_ENTER_METHOD();
	SPH_RET(0);
}


int ha_sphinx::rnd_end()
{
	SPH_ENTER_METHOD();
	SPH_RET(0);
}


int ha_sphinx::rnd_next ( byte * )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_END_OF_FILE );
}


void ha_sphinx::position ( const byte * )
{
	SPH_ENTER_METHOD();
	SPH_VOID_RET();
}


// This is like rnd_next, but you are given a position to use
// to determine the row. The position will be of the type that you stored in
// ref. You can use ha_get_ptr(pos,ref_length) to retrieve whatever key
// or position you saved when position() was called.
// Called from filesort.cc records.cc sql_insert.cc sql_select.cc sql_update.cc.
int ha_sphinx::rnd_pos ( byte *, byte * )
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_WRONG_COMMAND );
}


#if MYSQL_VERSION_ID>=50030
int ha_sphinx::info ( uint )
#else
void ha_sphinx::info ( uint )
#endif
{
	SPH_ENTER_METHOD();

	if ( table->s->keys>0 )
		table->key_info[0].rec_per_key[0] = 1;

	#if MYSQL_VERSION_ID>50100
	stats.records = 20;
	#else
	records = 20;
	#endif

#if MYSQL_VERSION_ID>=50030
	SPH_RET(0);
#else
	SPH_VOID_RET();
#endif
}


int ha_sphinx::extra ( enum ha_extra_function )
{
	SPH_ENTER_METHOD();
	SPH_RET(0);
}


int ha_sphinx::reset()
{
	SPH_ENTER_METHOD();
	SPH_RET(0);
}


int ha_sphinx::delete_all_rows()
{
	SPH_ENTER_METHOD();
	SPH_RET ( HA_ERR_WRONG_COMMAND );
}


// First you should go read the section "locking functions for mysql" in
// lock.cc to understand this.
// This create a lock on the table. If you are implementing a storage engine
// that can handle transacations look at ha_berkely.cc to see how you will
// want to go about doing this. Otherwise you should consider calling flock()
// here.
//
// Called from lock.cc by lock_external() and unlock_external(). Also called
// from sql_table.cc by copy_data_between_tables().
int ha_sphinx::external_lock ( THD *, int )
{
	SPH_ENTER_METHOD();
	SPH_RET(0);
}


THR_LOCK_DATA ** ha_sphinx::store_lock ( THD *, THR_LOCK_DATA ** to,
	enum thr_lock_type lock_type )
{
	SPH_ENTER_METHOD();

	if ( lock_type!=TL_IGNORE && m_tLock.type==TL_UNLOCK )
		m_tLock.type=lock_type;

	*to++ = &m_tLock;
	SPH_RET(to);
}


int ha_sphinx::delete_table ( const char * )
{
	SPH_ENTER_METHOD();
	SPH_RET(0);
}


// Renames a table from one name to another from alter table call.
//
// If you do not implement this, the default rename_table() is called from
// handler.cc and it will delete all files with the file extentions returned
// by bas_ext().
//
// Called from sql_table.cc by mysql_rename_table().
int ha_sphinx::rename_table ( const char *, const char * )
{
	SPH_ENTER_METHOD();
	SPH_RET(0);
}


// Given a starting key, and an ending key estimate the number of rows that
// will exist between the two. end_key may be empty which in case determine
// if start_key matches any rows.
//
// Called from opt_range.cc by check_quick_keys().
ha_rows ha_sphinx::records_in_range ( uint, key_range * min_key, key_range * max_key )
{
	SPH_ENTER_METHOD();

#if SPHINX_DEBUG
	String varchar;
	uint var_length = uint2korr(min_key->key);
	varchar.set_quick ( (char*)min_key->key+HA_KEY_BLOB_LENGTH, var_length, &my_charset_bin );
	SPH_DEBUG ( __FUNCTION__ ": key_val=%s, key_len=%d", varchar.ptr(), var_length );
#endif

	SPH_RET(3); // low number to force index usage
}


static inline bool IsIntegerFieldType ( enum_field_types eType )
{
	return eType==MYSQL_TYPE_LONG;
}


// create() is called to create a database. The variable name will have the name
// of the table. When create() is called you do not need to worry about opening
// the table. Also, the FRM file will have already been created so adjusting
// create_info will not do you any good. You can overwrite the frm file at this
// point if you wish to change the table definition, but there are no methods
// currently provided for doing that.
//
// Called from handle.cc by ha_create_table().
int ha_sphinx::create ( const char * name, TABLE * table, HA_CREATE_INFO * )
{
	SPH_ENTER_METHOD();
	char sError[256];

	if ( !ParseUrl ( NULL, table, true ) )
		SPH_RET(-1);

	for ( ;; )
	{
		// check system fields (count and types)
		if ( table->s->fields<SPHINXSE_SYSTEM_COLUMNS )
		{
			my_snprintf ( sError, sizeof(sError), "%s: there MUST be at least %d columns",
				name, SPHINXSE_SYSTEM_COLUMNS );
			break;
		}

		if ( !IsIntegerFieldType ( table->field[0]->type() ) )
		{
			my_snprintf ( sError, sizeof(sError), "%s: 1st column (docid) MUST be integer", name );
			break;
		}

		if ( !IsIntegerFieldType ( table->field[1]->type() ) )
		{
			my_snprintf ( sError, sizeof(sError), "%s: 2nd column (weight) MUST be integer", name );
			break;
		}

		if ( table->field[2]->type()!=MYSQL_TYPE_VARCHAR )
		{
			my_snprintf ( sError, sizeof(sError), "%s: 3rd column (search query) MUST be varchar", name );
			break;
		}

		// check attributes
		int i;
		for ( i=3; i<(int)table->s->fields; i++ )
		{
			enum_field_types eType = table->field[i]->type();
			if ( eType!=MYSQL_TYPE_TIMESTAMP && !IsIntegerFieldType(eType) )
			{
				my_snprintf ( sError, sizeof(sError), "%s: %dth column (attribute %s) MUST be integer or timestamp",
					name, i+1, table->field[i]->field_name );
				break;
			}
		}

		if ( i!=(int)table->s->fields )
			break;

		// check index
		if (
			table->s->keys!=1 ||
			table->key_info[0].key_parts!=1 ||
			strcasecmp ( table->key_info[0].key_part[0].field->field_name, table->field[2]->field_name ) )
		{
			my_snprintf ( sError, sizeof(sError), "%s: there must be an index on '%s' column",
				name, table->field[2]->field_name );
			break;
		}

		// all good
		sError[0] = '\0';
		break;
	}
	if ( sError[0] )
	{
		my_error ( ER_CANT_CREATE_TABLE, MYF(0), sError, -1 );
		SPH_RET(-1);
	}


/*
		MYSQL_TYPE_DECIMAL,
		MYSQL_TYPE_LONG,
		MYSQL_TYPE_LONGLONG,

		MYSQL_TYPE_TIMESTAMP,
		MYSQL_TYPE_DATETIME,
		MYSQL_TYPE_NEWDATE,
		
		MYSQL_TYPE_VARCHAR,
		MYSQL_TYPE_VAR_STRING,
		MYSQL_TYPE_STRING,
*/

	SPH_RET(0);
}


#if MYSQL_VERSION_ID>50100
struct st_mysql_storage_engine sphinx_storage_engine =
{
	MYSQL_HANDLERTON_INTERFACE_VERSION
};


mysql_declare_plugin(sphinx)
{
	MYSQL_STORAGE_ENGINE_PLUGIN,
	&sphinx_storage_engine,
	sphinx_hton_name,
	"Sphinx developers",
	sphinx_hton_comment,
	PLUGIN_LICENSE_GPL,
	sphinx_init_func, // Plugin Init
	sphinx_done_func, // Plugin Deinit
	0x0001, // 0.1
	NULL, NULL, NULL
}
mysql_declare_plugin_end;

#endif // >50100

//
// $Id$
//
