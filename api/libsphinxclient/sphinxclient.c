//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License. You should
// have received a copy of the LGPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#if _MSC_VER>=1400
#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include "sphinxclient_config.h"
#endif

#include "sphinxclient.h"

#if _WIN32
	// Win-specific headers, calls, libraries
	#include <io.h>
	#include <winsock2.h>

	#pragma comment(linker, "/defaultlib:wsock32.lib")
	#pragma message("Automatically linking with wsock32.lib")

	#define EWOULDBLOCK			WSAEWOULDBLOCK
	#define EINTR				WSAEINTR

#else
	// UNIX-specific headers and calls
	#include <unistd.h>
	#include <netinet/in.h>
	#include <sys/file.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/wait.h>
	#include <netdb.h>
	#include <errno.h>
#endif

//////////////////////////////////////////////////////////////////////////

#define MAX_REQS				32
#define CONNECT_TIMEOUT_MSEC	1000
#define MAX_PACKET_LEN			(8*1024*1024)

enum
{
	SEARCHD_COMMAND_SEARCH		= 0,
	SEARCHD_COMMAND_EXCERPT		= 1,
	SEARCHD_COMMAND_UPDATE		= 2,
	SEARCHD_COMMAND_KEYWORDS	= 3,
	SEARCHD_COMMAND_PERSIST		= 4,
	SEARCHD_COMMAND_STATUS		= 5
};

enum
{
	VER_COMMAND_EXCERPT			= 0x102,
	VER_COMMAND_UPDATE			= 0x102,
	VER_COMMAND_KEYWORDS		= 0x100,
	VER_COMMAND_STATUS			= 0x100
};

//////////////////////////////////////////////////////////////////////////

struct st_filter
{
	const char *			attr;
	int						filter_type;
	int						num_values;
	const sphinx_int64_t *	values;
	sphinx_int64_t			umin;
	sphinx_int64_t			umax;
	float					fmin;
	float					fmax;
	int						exclude;
};


union un_attr_value
{
	sphinx_int64_t			int_value;
	float					float_value;
	unsigned int *			mva_value;
	const char *			string;
};


struct st_override
{
	const char *			attr;
	const sphinx_uint64_t *	docids;
	int						num_values;
	const unsigned int *	uint_values;
};


struct st_sphinx_client
{
	unsigned short			ver_search;				///< compatibility mode
	sphinx_bool				copy_args;				///< whether to create a copy of each passed argument
	void *					head_alloc;				///< head of client-owned allocations list

	const char *			error;					///< last error
	const char *			warning;				///< last warning
	char					local_error_buf[256];	///< buffer to store 'local' error messages (eg. connect() error)

	const char *			host;
	int						port;
	float					timeout;
	int						offset;
	int						limit;
	int						mode;
	int						num_weights;
	const int *				weights;
	int						sort;
	const char *			sortby;
	sphinx_uint64_t			minid;
	sphinx_uint64_t			maxid;
	const char *			group_by;
	int						group_func;
	const char *			group_sort;
	const char *			group_distinct;
	int						max_matches;
	int						cutoff;
	int						retry_count;
	int						retry_delay;
	const char *			geoanchor_attr_lat;
	const char *			geoanchor_attr_long;
	float					geoanchor_lat;
	float					geoanchor_long;
	int						num_filters;
	int						max_filters;
	struct st_filter *		filters;
	int						num_index_weights;
	const char **			index_weights_names;
	const int *				index_weights_values;
	int						ranker;
	int						max_query_time;
	int						num_field_weights;
	const char **			field_weights_names;
	const int *				field_weights_values;
	int						num_overrides;
	int						max_overrides;
	struct st_override *	overrides;
	const char *			select_list;

	int						num_reqs;
	int						req_lens [ MAX_REQS ];
	char *					reqs [ MAX_REQS ];

	int						response_len;
	char *					response_buf;	///< where the buffer begins (might also contain heading warning)
	char *					response_start;	///< where the data to parse starts

	int						num_results;
	sphinx_result			results [ MAX_REQS ];

	int						sock;			///< open socket for pconns; -1 if none
};

//////////////////////////////////////////////////////////////////////////

static void *				chain ( sphinx_client * client, const void * ptr, size_t len );
static const char *			strchain ( sphinx_client * client, const char * s );
static void					unchain ( sphinx_client * client, const void * ptr );
static void					unchain_all ( sphinx_client * client );

sphinx_client * sphinx_create ( sphinx_bool copy_args )
{
	sphinx_client * client;
	int i;

	// allocate
	client = malloc ( sizeof(sphinx_client) );
	if ( !client )
		return NULL;

	// initialize defaults and return
	client->ver_search				= 0x117; // 0x113 for 0.9.8, 0x116 for 0.9.9rc2
	client->copy_args				= copy_args;
	client->head_alloc				= NULL;

	client->error					= NULL;
	client->warning					= NULL;
	client->local_error_buf[0]		= '\0';

	client->host					= strchain ( client, "localhost" );
	client->port					= 9312;
	client->timeout					= 0.0f;
	client->offset					= 0;
	client->limit					= 20;
	client->mode					= SPH_MATCH_ALL;
	client->num_weights				= 0;
	client->weights					= NULL;
	client->sort					= SPH_SORT_RELEVANCE;
	client->sortby					= NULL;
	client->minid					= 0;
	client->maxid					= 0;
	client->group_by				= NULL;
	client->group_func				= SPH_GROUPBY_ATTR;
	client->group_sort				= strchain ( client, "@groupby desc" );
	client->group_distinct			= NULL;
	client->max_matches				= 1000;
	client->cutoff					= 0;
	client->retry_count				= 0;
	client->retry_delay				= 0;
	client->geoanchor_attr_lat		= NULL;
	client->geoanchor_attr_long		= NULL;
	client->geoanchor_lat			= 0.0f;
	client->geoanchor_long			= 0.0f;
	client->num_filters				= 0;
	client->max_filters				= 0;
	client->filters					= NULL;
	client->num_index_weights		= 0;
	client->index_weights_names		= NULL;
	client->index_weights_values	= NULL;
	client->ranker					= SPH_RANK_PROXIMITY_BM25;
	client->max_query_time			= 0;
	client->num_field_weights		= 0;
	client->field_weights_names		= NULL;
	client->field_weights_values	= NULL;
	client->num_overrides			= 0;
	client->max_overrides			= 0;
	client->overrides				= NULL;
	client->select_list				= NULL;

	client->num_reqs				= 0;
	client->response_len			= 0;
	client->response_buf			= NULL;
	client->num_results				= 0;

	for ( i=0; i<MAX_REQS; i++ )
	{
		client->results[i].values_pool = NULL;
		client->results[i].words = NULL;
		client->results[i].fields = NULL;
		client->results[i].attr_names = NULL;
		client->results[i].attr_types = NULL;
	}

	client->sock = -1;
	return client;
}


static void sphinx_free_results ( sphinx_client * client )
{
	int i;
	for ( i=0; i<client->num_results; i++ )
	{
		free ( client->results[i].values_pool );
		free ( client->results[i].words );
		free ( client->results[i].fields );
		free ( client->results[i].attr_names );
		free ( client->results[i].attr_types );

		client->results[i].values_pool = NULL;
		client->results[i].words = NULL;
		client->results[i].fields = NULL;
		client->results[i].attr_names = NULL;
		client->results[i].attr_types = NULL;
	}
	client->num_results = 0;
}


void sock_close ( int sock );

void sphinx_destroy ( sphinx_client * client )
{
	int i;

	if ( !client )
		return;

	for ( i=0; i<client->num_reqs; i++ )
		free ( client->reqs[i] );

	sphinx_free_results ( client );

	unchain_all ( client );

	if ( client->filters )
		free ( client->filters );

	if ( client->response_buf )
		free ( client->response_buf );

	if ( client->sock>=0 )
		sock_close ( client->sock );

	free ( client );
}


const char * sphinx_error ( sphinx_client * client )
{
	return client->error ? client->error : "";
}


const char * sphinx_warning ( sphinx_client * client )
{
	return client->warning ? client->warning : "";
}


static void set_error ( sphinx_client * client, const char * template, ... )
{
	va_list ap;

	if ( !client )
		return;

	va_start ( ap, template );
	vsnprintf ( client->local_error_buf, sizeof(client->local_error_buf), template, ap );
	va_end ( ap );

	client->error = client->local_error_buf;
}


//////////////////////////////////////////////////////////////////////////

struct st_memblock
{
	struct st_memblock *	prev;
	struct st_memblock *	next;
};


static void * chain ( sphinx_client * client, const void * ptr, size_t len )
{
	struct st_memblock * entry;

	if ( !client->copy_args || !ptr )
		return (void*) ptr;

	entry = malloc ( sizeof(struct st_memblock) + len );
	if ( !entry )
	{
		set_error ( client, "malloc() failed (bytes=%d)", sizeof(struct st_memblock) + len );
		return NULL;
	}

	entry->prev = NULL;
	entry->next = client->head_alloc;
	if ( entry->next )
		entry->next->prev = entry;
	client->head_alloc = entry;

	entry++;
	memcpy ( entry, ptr, len );
	return entry;
}


static const char * strchain ( sphinx_client * client, const char * s )
{
	return s ? chain ( client, s, 1+strlen(s) ) : NULL;
}


static void unchain ( sphinx_client * client, const void * ptr )
{
	struct st_memblock * entry;

	if ( !client->copy_args || !ptr )
		return;

	entry = (struct st_memblock*) ptr;
	entry--;

	if ( entry->prev )
		entry->prev->next = entry->next;
	else
		client->head_alloc = entry->next;

	if ( entry->next )
		entry->next->prev = entry->prev;

	free ( entry );
}


static void unchain_all ( sphinx_client * client )
{
	struct st_memblock *to_free, *cur;

	if ( !client || !client->copy_args )
		return;

	cur = client->head_alloc;
	while ( cur )
	{
		to_free = cur;
		cur = cur->next;
		free ( to_free );
	}
	client->head_alloc = NULL;
}

//////////////////////////////////////////////////////////////////////////

sphinx_bool sphinx_set_server ( sphinx_client * client, const char * host, int port )
{
	if ( !client || !host || !host[0] )
	{
		set_error ( client, "invalid arguments (host must not be empty)" );
		return SPH_FALSE;
	}

	unchain ( client, client->host );
	client->host = strchain ( client, host );
	client->port = port;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_connect_timeout ( sphinx_client * client, float seconds )
{
	if ( !client )
		return SPH_FALSE;

	client->timeout = seconds;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_limits ( sphinx_client * client, int offset, int limit, int max_matches, int cutoff )
{
	if ( !client || offset<0 || limit<=0 || max_matches<0 || cutoff<0 )
	{
		if ( offset<0 )				set_error ( client, "invalid arguments (offset must be >= 0)" );
		else if ( limit<=0 )		set_error ( client, "invalid arguments (limit must be > 0)" );
		else if ( max_matches<0 )	set_error ( client, "invalid arguments (max_matches must be >= 0)" );
		else if ( cutoff<0 )		set_error ( client, "invalid arguments (cutoff must be >= 0)" );
		else						set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	client->offset = offset;
	client->limit = limit;
	if ( max_matches>0 )
		client->max_matches = max_matches;
	if ( cutoff>0 )
		client->cutoff = cutoff;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_max_query_time ( sphinx_client * client, int max_query_time )
{
	if ( !client || max_query_time<=0 )
	{
		set_error ( client, "invalid arguments (max_query_time must be > 0)" );
		return SPH_FALSE;
	}

	client->max_query_time = max_query_time;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_match_mode ( sphinx_client * client, int mode )
{
	if ( !client || mode<SPH_MATCH_ALL || mode>SPH_MATCH_EXTENDED2 ) // FIXME?
	{
		set_error ( client, "invalid arguments (matching mode %d out of bounds)", mode );
		return SPH_FALSE;
	}

	client->mode = mode;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_ranking_mode ( sphinx_client * client, int ranker )
{
	if ( !client || ranker<SPH_RANK_PROXIMITY_BM25 || ranker>SPH_RANK_WORDCOUNT ) // FIXME?
	{
		set_error ( client, "invalid arguments (ranking mode %d out of bounds)", ranker );
		return SPH_FALSE;
	}

	client->ranker = ranker;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_sort_mode ( sphinx_client * client, int mode, const char * sortby )
{
	if ( !client
		|| mode<SPH_SORT_RELEVANCE
		|| mode>SPH_SORT_EXPR
		|| ( mode!=SPH_SORT_RELEVANCE && ( !sortby || !sortby[0] ) ) )
	{
		if ( mode<SPH_SORT_RELEVANCE || mode>SPH_SORT_EXPR )
		{
			set_error ( client, "invalid arguments (sorting mode %d out of bounds)", mode );

		} else if ( mode!=SPH_SORT_RELEVANCE && ( !sortby || !sortby[0] ) )
		{
			set_error ( client, "invalid arguments (sortby clause must not be empty)", mode );

		} else
		{
			set_error ( client, "invalid arguments", mode );
		}
		return SPH_FALSE;
	}

	client->sort = mode;
	unchain ( client, client->sortby );
	client->sortby = strchain ( client, sortby );
	return SPH_TRUE;
}


sphinx_bool sphinx_set_field_weights ( sphinx_client * client, int num_weights, const char ** field_names, const int * field_weights )
{
	int i;

	if ( !client || num_weights<=0 || !field_names || !field_weights )
	{
		if ( num_weights<=0 )		set_error ( client, "invalid arguments (num_weights must be > 0)" );
		else if ( !field_names )	set_error ( client, "invalid arguments (field_names must not be NULL)" );
		else if ( !field_names )	set_error ( client, "invalid arguments (field_weights must not be NULL)" );
		else						set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	if ( client->copy_args )
	{
		for ( i=0; i<client->num_field_weights; i++ )
			unchain ( client, client->field_weights_names[i] );
		unchain ( client, client->field_weights_names );
		unchain ( client, client->field_weights_values );

		field_names = chain ( client, field_names, num_weights*sizeof(const char*) );
		for ( i=0; i<num_weights; i++ )
			field_names[i] = strchain ( client, field_names[i] );
		field_weights = chain ( client, field_weights, num_weights*sizeof(int) );
	}

	client->num_field_weights = num_weights;
	client->field_weights_names = field_names;
	client->field_weights_values = field_weights;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_index_weights ( sphinx_client * client, int num_weights, const char ** index_names, const int * index_weights )
{
	int i;

	if ( !client || num_weights<=0 || !index_names || !index_weights )
	{
		if ( num_weights<=0 )		set_error ( client, "invalid arguments (num_weights must be > 0)" );
		else if ( !index_names )	set_error ( client, "invalid arguments (index_names must not be NULL)" );
		else if ( !index_names )	set_error ( client, "invalid arguments (index_weights must not be NULL)" );
		else						set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	if ( client->copy_args )
	{
		for ( i=0; i<client->num_index_weights; i++ )
			unchain ( client, client->index_weights_names[i] );
		unchain ( client, client->index_weights_names );
		unchain ( client, client->index_weights_values );

		index_names = chain ( client, index_names, num_weights*sizeof(const char*) );
		for ( i=0; i<num_weights; i++ )
			index_names[i] = strchain ( client, index_names[i] );
		index_weights = chain ( client, index_weights, num_weights*sizeof(int) );
	}

	client->num_index_weights = num_weights;
	client->index_weights_names = index_names;
	client->index_weights_values = index_weights;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_id_range ( sphinx_client * client, sphinx_uint64_t minid, sphinx_uint64_t maxid )
{
	if ( !client || minid>maxid )
	{
		set_error ( client, "invalid arguments (minid must be <= maxid)" );
		return SPH_FALSE;
	}

	client->minid = minid;
	client->maxid = maxid;
	return SPH_TRUE;
}


static struct st_filter * sphinx_add_filter_entry ( sphinx_client * client )
{
	int len;
	if ( client->num_filters>=client->max_filters )
	{
		len = ( client->max_filters<=0 ) ? client->num_filters + 8 : 2*client->max_filters;
		len *= sizeof(struct st_filter);
		client->filters = realloc ( client->filters, len );
		if ( !client->filters )
		{
			set_error ( client, "realloc() failed (bytes=%d)", len );
			return NULL;
		}
	}
	return client->filters + client->num_filters++;
}


sphinx_bool sphinx_add_filter ( sphinx_client * client, const char * attr, int num_values, const sphinx_int64_t * values, sphinx_bool exclude )
{
	struct st_filter * filter;

	if ( !client || !attr || num_values<=0 || !values )
	{
		if ( !attr )				set_error ( client, "invalid arguments (attr must not be empty)" );
		else if ( num_values<=0 )	set_error ( client, "invalid arguments (num_values must be > 0)" );
		else if ( !values )			set_error ( client, "invalid arguments (values must not be NULL)" );
		else						set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	filter = sphinx_add_filter_entry ( client );
	if ( !filter )
		return SPH_FALSE;

	filter->attr = strchain ( client, attr );
	filter->filter_type = SPH_FILTER_VALUES;
	filter->num_values = num_values;
	filter->values = chain ( client, values, num_values*sizeof(sphinx_int64_t) );
	filter->exclude = exclude;
	return SPH_TRUE;
}


sphinx_bool sphinx_add_filter_range ( sphinx_client * client, const char * attr, sphinx_int64_t umin, sphinx_int64_t umax, sphinx_bool exclude )
{
	struct st_filter * filter;

	if ( !client || !attr || umin>umax )
	{
		if ( !attr )			set_error ( client, "invalid arguments (attr must not be empty)" );
		else if ( umin>umax )	set_error ( client, "invalid arguments (umin must be <= umax)" );
		else					set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	filter = sphinx_add_filter_entry ( client );
	if ( !filter )
		return SPH_FALSE;

	filter->attr = strchain ( client, attr );
	filter->filter_type = SPH_FILTER_RANGE;
	filter->umin = umin;
	filter->umax = umax;
	filter->exclude = exclude;
	return SPH_TRUE;
}


sphinx_bool sphinx_add_filter_float_range ( sphinx_client * client, const char * attr, float fmin, float fmax, sphinx_bool exclude )
{
	struct st_filter * filter;

	if ( !client || !attr || fmin>fmax )
	{
		if ( !attr )			set_error ( client, "invalid arguments (attr must not be empty)" );
		else if ( fmin>fmax )	set_error ( client, "invalid arguments (fmin must be <= fmax)" );
		else					set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	filter = sphinx_add_filter_entry ( client );
	if ( !filter )
		return SPH_FALSE;

	filter->attr = strchain ( client, attr );
	filter->filter_type = SPH_FILTER_FLOATRANGE;
	filter->fmin = fmin;
	filter->fmax = fmax;
	filter->exclude = exclude;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_geoanchor ( sphinx_client * client, const char * attr_latitude, const char * attr_longitude, float latitude, float longitude )
{
	if ( !client || !attr_latitude || !attr_latitude[0] || !attr_longitude || !attr_longitude[0] )
	{
		if ( !attr_latitude || !attr_latitude[0] )			set_error ( client, "invalid arguments (attr_latitude must not be empty)" );
		else if ( !attr_longitude || !attr_longitude[0] )	set_error ( client, "invalid arguments (attr_longitude must not be empty)" );
		else												set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	unchain ( client, client->geoanchor_attr_lat );
	unchain ( client, client->geoanchor_attr_long );
	client->geoanchor_attr_lat = strchain ( client, attr_latitude );
	client->geoanchor_attr_long = strchain ( client, attr_longitude );
	client->geoanchor_lat = latitude;
	client->geoanchor_long = longitude;
	return SPH_TRUE;
}


sphinx_bool sphinx_set_groupby ( sphinx_client * client, const char * attr, int groupby_func, const char * group_sort )
{
	if ( !client || !attr || groupby_func<SPH_GROUPBY_DAY || groupby_func>SPH_GROUPBY_ATTRPAIR )
	{
		if ( !attr )
		{
			set_error ( client, "invalid arguments (attr must not be empty)" );

		} else if ( groupby_func<SPH_GROUPBY_DAY || groupby_func>SPH_GROUPBY_ATTRPAIR )
		{
			set_error ( client, "invalid arguments (groupby_func %d out of bounds)", groupby_func );

		} else
		{
			set_error ( client, "invalid arguments" );
		}
		return SPH_FALSE;
	}

	unchain ( client, client->group_by );
	unchain ( client, client->group_sort );
	client->group_by = strchain ( client, attr );
	client->group_func = groupby_func;
	client->group_sort = strchain ( client, group_sort ? group_sort : "@groupby desc" );
	return SPH_TRUE;
}


sphinx_bool sphinx_set_groupby_distinct ( sphinx_client * client, const char * attr )
{
	if ( !client || !attr )
	{
		if ( !attr)		set_error ( client, "invalid arguments (attr must not be empty)" );
		else			set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	unchain ( client, client->group_distinct );
	client->group_distinct = strchain ( client, attr );
	return SPH_TRUE;
}


sphinx_bool sphinx_set_retries ( sphinx_client * client, int count, int delay )
{
	if ( !client || count<0 || count>1000 || delay<0 || delay>100000 )
	{
		if ( count<0 || count>1000 )		set_error ( client, "invalid arguments (count value %d out of bounds)", count );
		else if ( delay<0 || delay>100000 )	set_error ( client, "invalid arguments (delay value %d out of bounds)", delay );
		else								set_error ( client, "invalid arguments" );
		return SPH_FALSE;
	}

	client->retry_count = count;
	client->retry_delay = delay;
	return SPH_TRUE;
}


sphinx_bool sphinx_add_override ( sphinx_client * client, const char * attr, const sphinx_uint64_t * docids, int num_values, const unsigned int * values )
{
	struct st_override * p;

	if ( !client )
		return SPH_FALSE;

	if ( client->ver_search<0x115 )
	{
		set_error ( client, "sphinx_add_override not supported by chosen protocol version" );
		return SPH_FALSE;
	}

	if ( client->num_overrides>=client->max_overrides )
	{
		client->max_overrides = ( client->max_overrides<=0 ) ? 8 : 2*client->max_overrides;
		client->overrides = realloc ( client->overrides, client->max_overrides *sizeof(struct st_override) );
	}

	p = client->overrides + client->num_overrides;
	client->num_overrides++;

	p->attr = strchain ( client, attr );
	p->docids = chain ( client, docids, sizeof(sphinx_uint64_t)*num_values );
	p->num_values = num_values;
	p->uint_values = chain ( client, values, sizeof(unsigned int)*num_values );
	return SPH_TRUE;
}


sphinx_bool sphinx_set_select ( sphinx_client * client, const char * select_list )
{
	if ( !client )
		return SPH_FALSE;

	if ( client->ver_search<0x116 )
	{
		set_error ( client, "sphinx_set_select not supported by chosen protocol version" );
		return SPH_FALSE;
	}

	unchain ( client, client->select_list );
	client->select_list = strchain ( client, select_list );
	return SPH_TRUE;
}


void sphinx_reset_filters ( sphinx_client * client )
{
	int i;

	if ( !client )
		return;

	if ( client->filters )
	{
		if ( client->copy_args )
			for ( i=0; i<client->num_filters; i++ )
		{
			unchain ( client, client->filters[i].attr );
			if ( client->filters[i].filter_type==SPH_FILTER_VALUES )
				unchain ( client, client->filters[i].values );
		}

		free ( client->filters );
		client->filters = NULL;
	}
	client->num_filters = client->max_filters = 0;
}


void sphinx_reset_groupby ( sphinx_client * client )
{
	if ( !client )
		return;

	unchain ( client, client->group_by );
	unchain ( client, client->group_sort );
	client->group_by = NULL;
	client->group_func = SPH_GROUPBY_ATTR;
	client->group_sort = strchain ( client, "@groupby desc" );
	client->group_distinct = NULL;
}

//////////////////////////////////////////////////////////////////////////

sphinx_result * sphinx_query ( sphinx_client * client, const char * query, const char * index_list, const char * comment )
{
	sphinx_result * res;

	if ( !client )
		return NULL;

	if ( client->num_reqs!=0 )
	{
		set_error ( client, "sphinx_query() must not be called after sphinx_add_query()" );
		return NULL;
	}

	if ( sphinx_add_query ( client, query, index_list, comment )!=0 )
		return NULL;

	res = sphinx_run_queries ( client ); // just a shortcut for client->results[0]
	if ( !res )
		return NULL;

	client->error = res->error;
	client->warning = res->warning;
	return ( res->status==SEARCHD_ERROR ) ? NULL : res;
}


static size_t safestrlen ( const char * s )
{
	return s ? strlen(s) : 0;
}


static int calc_req_len ( sphinx_client * client, const char * query, const char * index_list, const char * comment )
{
	int i, filter_val_size;
	size_t res;

	res = 96 + 2*(int)sizeof(sphinx_uint64_t) + 4*client->num_weights
		+ safestrlen ( client->sortby )
		+ safestrlen ( query )
		+ safestrlen ( index_list )
		+ safestrlen ( client->group_by )
		+ safestrlen ( client->group_sort )
		+ safestrlen ( client->group_distinct )
		+ safestrlen ( comment );

	filter_val_size = ( client->ver_search>=0x114 ) ? 8 : 4;
	for ( i=0; i<client->num_filters; i++ )
	{
		const struct st_filter * filter = &client->filters[i];
		res += 12 + safestrlen ( filter->attr ); // string attr-name; int type; int exclude-flag

		switch ( filter->filter_type )
		{
			case SPH_FILTER_VALUES:		res += 4 + filter_val_size*filter->num_values; break; // int values-count; uint32/int64[] values
			case SPH_FILTER_RANGE:		res += 2*filter_val_size; break; // uint32/int64 min-val, max-val
			case SPH_FILTER_FLOATRANGE:	res += 8; break; // float min-val,max-val
		}
	}

	if ( client->geoanchor_attr_lat && client->geoanchor_attr_long )
		res += 16 + safestrlen ( client->geoanchor_attr_lat ) + safestrlen ( client->geoanchor_attr_long ); // string lat-attr, long-attr; float lat, long

	for ( i=0; i<client->num_index_weights; i++ )
		res += 8 + safestrlen ( client->index_weights_names[i] ); // string index-name; int index-weight

	for ( i=0; i<client->num_field_weights; i++ )
		res += 8 + safestrlen ( client->field_weights_names[i] ); // string field-name; int field-weight

	if ( client->ver_search>=0x115 )
	{
		res += 4; // int overrides-count
		for ( i=0; i<client->num_overrides; i++ )
		{
			res += 8 + safestrlen ( client->overrides[i].attr ); // string attr, int attr-type
			res += 4 + 12*client->overrides[i].num_values; // int values-count, { uint64 docid, uint32 value }[] override
		}
	}

	if ( client->ver_search>=0x116 )
		res += 4 + safestrlen ( client->select_list ); // string select_list

	return (int)res;
}


static void send_bytes ( char ** pp, const char * bytes, int len )
{
	char * ptr;
	int i;

	ptr = *pp;
	if ( ptr )
		for ( i=0; i<len; i++ )
			*ptr++ = bytes[i];
	*pp = ptr;
}


static void send_int ( char ** pp, unsigned int value )
{
	union
	{
		unsigned int n;
		char c[sizeof(int)];
	} u;

	u.n = htonl ( value );
	send_bytes ( pp, u.c, (int)sizeof(int) );
}


static void send_word ( char ** pp, unsigned short value )
{
	union
	{
		unsigned short n;
		char c[sizeof(short)];
	} u;

	u.n = htons ( value );
	send_bytes ( pp, u.c, (int)sizeof(short) );
}


static void send_str ( char ** pp, const char * s )
{
	int len;
	len = s ? (int)strlen(s) : 0;
	send_int ( pp, len );
	send_bytes ( pp, s, len );
}


static void send_qword ( char ** pp, sphinx_uint64_t value )
{
	send_int ( pp, (int)( value >> 32 ) );
	send_int ( pp, (int)( value & ((sphinx_uint64_t)0xffffffffL) ) );
}


static void send_float ( char ** pp, float value )
{
	union
	{
		float f;
		int i;
	} u;

	u.f = value;
	send_int ( pp, u.i );
}


int sphinx_add_query ( sphinx_client * client, const char * query, const char * index_list, const char * comment )
{
	int i, j, req_len;
	char * req;

	if ( client->num_reqs<0 || client->num_reqs>=MAX_REQS )
	{
		set_error ( client, "num_reqs=%d out of bounds (too many queries?)", client->num_reqs );
		return -1;
	}

	req_len = calc_req_len ( client, query, index_list, comment );

	req = malloc ( req_len );
	if ( !req )
	{
		set_error ( client, "malloc() failed (bytes=%d)", req_len );
		return -1;
	}

	client->reqs[client->num_reqs] = req;
	client->req_lens[client->num_reqs] = req_len;
	client->num_reqs++;

	send_int ( &req, client->offset );
	send_int ( &req, client->limit );
	send_int ( &req, client->mode );
	send_int ( &req, client->ranker );
	send_int ( &req, client->sort );
	send_str ( &req, client->sortby );
	send_str ( &req, query );
	send_int ( &req, client->num_weights );
	for ( i=0; i<client->num_weights; i++ )
		send_int ( &req, client->weights[i] );
	send_str ( &req, index_list );
	send_int ( &req, 1 ); // id range bits
	send_qword ( &req, client->minid );
	send_qword ( &req, client->maxid );
	send_int ( &req, client->num_filters );
	for ( i=0; i<client->num_filters; i++ )
	{
		send_str ( &req, client->filters[i].attr );
		send_int ( &req, client->filters[i].filter_type );

		switch ( client->filters[i].filter_type )
		{
			case SPH_FILTER_VALUES:
				send_int ( &req, client->filters[i].num_values );
				if ( client->ver_search>=0x114 )
				{
					for ( j=0; j<client->filters[i].num_values; j++ )
						send_qword ( &req, client->filters[i].values[j] );
				} else
				{
					for ( j=0; j<client->filters[i].num_values; j++ )
						send_int ( &req, (unsigned int)client->filters[i].values[j] );
				}
				break;

			case SPH_FILTER_RANGE:
				if ( client->ver_search>=0x114 )
				{
					send_qword ( &req, client->filters[i].umin );
					send_qword ( &req, client->filters[i].umax );
				} else
				{
					send_int ( &req, (unsigned int)client->filters[i].umin );
					send_int ( &req, (unsigned int)client->filters[i].umax );
				}
				break;

			case SPH_FILTER_FLOATRANGE:
				send_float ( &req, client->filters[i].fmin );
				send_float ( &req, client->filters[i].fmax );
				break;
		}

		send_int ( &req, client->filters[i].exclude );
	}
	send_int ( &req, client->group_func );
	send_str ( &req, client->group_by );
	send_int ( &req, client->max_matches );
	send_str ( &req, client->group_sort );
	send_int ( &req, client->cutoff );
	send_int ( &req, client->retry_count );
	send_int ( &req, client->retry_delay );
	send_str ( &req, client->group_distinct );
	if ( client->geoanchor_attr_lat && client->geoanchor_attr_long )
	{
		send_int ( &req, 1 );
		send_str ( &req, client->geoanchor_attr_lat );
		send_str ( &req, client->geoanchor_attr_long );
		send_float ( &req, client->geoanchor_lat );
		send_float ( &req, client->geoanchor_long );
	} else
	{
		send_int ( &req, 0 );
	}
	send_int ( &req, client->num_index_weights );
	for ( i=0; i<client->num_index_weights; i++ )
	{
		send_str ( &req, client->index_weights_names[i] );
		send_int ( &req, client->index_weights_values[i] );
	}
	send_int ( &req, client->max_query_time );
	send_int ( &req, client->num_field_weights );
	for ( i=0; i<client->num_field_weights; i++ )
	{
		send_str ( &req, client->field_weights_names[i] );
		send_int ( &req, client->field_weights_values[i] );
	}
	send_str ( &req, comment );

	if ( client->ver_search>=0x115 )
	{
		send_int ( &req, client->num_overrides );
		for ( i=0; i<client->num_overrides; i++ )
		{
			send_str ( &req, client->overrides[i].attr );
			send_int ( &req, SPH_ATTR_INTEGER );
			send_int ( &req, client->overrides[i].num_values );
			for ( j=0; j<client->overrides[i].num_values; j++ )
			{
				send_qword ( &req, client->overrides[i].docids[j] );
				send_int ( &req, client->overrides[i].uint_values[j] );
			}
		}
	}

	if ( client->ver_search>=0x116 )
		send_str ( &req, client->select_list );

	if ( !req )
	{
		set_error ( client, "internal error, failed to build request" );
		free ( client->reqs [ --client->num_reqs ] );
		return -1;
	}

	return client->num_reqs-1;
}


static const char * sock_error ()
{
#if _WIN32
	static char sBuf [ 256 ];
	int iErr;

	iErr = WSAGetLastError ();
	_snprintf ( sBuf, sizeof(sBuf), "WSA error %d", iErr );
	return sBuf;
#else
	return strerror ( errno );
#endif
}


static int sock_errno ()
{
#ifdef _WIN32
	return WSAGetLastError ();
#else
	return errno;
#endif
}


static int sock_set_nonblocking ( int sock )
{
#if _WIN32
	u_long uMode = 1;
	return ioctlsocket ( sock, FIONBIO, &uMode );
#else
	return fcntl ( sock, F_SETFL, O_NONBLOCK );
#endif
}


static int sock_set_blocking ( int sock )
{
#if _WIN32
	u_long uMode = 0;
	return ioctlsocket ( sock, FIONBIO, &uMode );
#else
	return fcntl ( sock, F_SETFL, 0 );
#endif
}


void sock_close ( int sock )
{
#if _WIN32
	closesocket ( sock );
#else
	close ( sock );
#endif
}


// wrap FD_SET to prevent warnings on Windows
#if _WIN32
#pragma warning(disable:4127) // conditional expr is const
#pragma warning(disable:4389) // signed/unsigned mismatch
void SPH_FD_SET ( int fd, fd_set * fdset ) { FD_SET ( fd, fdset ); }
#pragma warning(default:4127) // conditional expr is const
#pragma warning(default:4389) // signed/unsigned mismatch
#else // !USE_WINDOWS
#define	SPH_FD_SET FD_SET
#endif


static sphinx_bool net_write ( int fd, const char * bytes, int len, sphinx_client * client )
{
	int res;
	res = send ( fd, bytes, len, 0 );

	if ( res<0 )
	{
		set_error ( client, "send() error: %s", sock_error() );
		return SPH_FALSE;
	}

	if ( res!=len )
	{
		set_error ( client, "send() error: incomplete write (len=%d, sent=%d)", len, res );
		return SPH_FALSE;
	}

	return SPH_TRUE;
}


static sphinx_bool net_read ( int fd, char * buf, int len, sphinx_client * client )
{
	int res, err;
	for ( ;; )
	{
		res = recv ( fd, buf, len, 0 );

		if ( res<0 )
		{
			err = sock_errno();
			if ( err==EINTR || err==EWOULDBLOCK ) // FIXME! remove non-blocking mode here; add timeout
				continue;
			set_error ( client, "recv(): read error (error=%s)", sock_error() );
			return SPH_FALSE;
		}

		len -= res;
		buf += res;

		if ( len==0 )
			return SPH_TRUE;

		if ( res==0 )
		{
			set_error ( client, "recv(): incomplete read (len=%d, recv=%d)", len, res );
			return SPH_FALSE;
		}
	}
}


static int net_connect ( sphinx_client * client )
{
	struct hostent * hp;
	struct sockaddr_in sa;
	struct timeval timeout;
	fd_set fds_write;
	int sock, to_wait, res, err, my_proto, optval;

	if ( client->sock>=0 )
		return client->sock;

	hp = gethostbyname ( client->host );
	if ( !hp )
	{
		set_error ( client, "host name lookup failed (host=%s, error=%s)", client->host, sock_error() );
		return -1;
	}

	memset ( &sa, 0, sizeof(sa) );
	memcpy ( &sa.sin_addr, hp->h_addr_list[0], hp->h_length );
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons ( (unsigned short)client->port );

	sock = (int) socket ( hp->h_addrtype, SOCK_STREAM, 0 );
	if ( sock<0 )
	{
		set_error ( client, "socket() failed: %s", sock_error() );
		return -1;
	}

	if ( sock_set_nonblocking ( sock )<0 )
	{
		set_error ( client, "sock_set_nonblocking() failed: %s", sock_error() );
		return -1;
	}

	optval = 1;
#ifndef _WIN32
	if ( setsockopt ( sock, SOL_SOCKET, SO_NOSIGPIPE, (void *)&optval, (socklen_t)sizeof(optval) ) < 0 )
	{
		set_error ( client, "setsockopt() failed: %s", sock_error() );
		return -1;
	}
#endif

	res = connect ( sock, (struct sockaddr*)&sa, sizeof(sa) );
	if ( res==0 )
		return sock;

	err = sock_errno();
#ifdef EINPROGRESS
	if ( err!=EWOULDBLOCK && err!=EINPROGRESS )
#else
	if ( err!=EWOULDBLOCK )
#endif
	{
		set_error ( client, "connect() failed: %s", sock_error() );
		return -1;
	}

	to_wait = (int)( 1000*client->timeout );
	if ( to_wait<=0 )
		to_wait = CONNECT_TIMEOUT_MSEC;

	{
		timeout.tv_sec = to_wait / 1000; // full seconds
		timeout.tv_usec = ( to_wait % 1000 ) * 1000; // remainder is msec, so *1000 for usec
		FD_ZERO ( &fds_write );
		SPH_FD_SET ( sock, &fds_write );

		res = select ( 1+sock, NULL, &fds_write, NULL, &timeout );

		if ( res>=0 && FD_ISSET ( sock, &fds_write ) )
		{
			sock_set_blocking ( sock );

			// now send major client protocol version
			my_proto = htonl ( 1 ); 
			if ( !net_write ( sock, (char*)&my_proto, sizeof(my_proto), client ) )
			{
				sock_close ( sock );
				set_error ( client, "failed to send client protocol version" );
				return -1;
			}

			// check daemon version
			if ( !net_read ( sock, (char*)&my_proto, sizeof(my_proto), client ) )
			{
				sock_close ( sock );
				return -1;
			}

			my_proto = ntohl ( my_proto );
			if ( my_proto<1 )
			{
				sock_close ( sock );
				set_error ( client, "expected searchd protocol version 1+, got version %d", my_proto );
				return -1;
			}

			return sock;
		}

		/*!COMMIT handle EINTR here*/

		sock_close ( sock );
		set_error ( client, "connect() timed out" );
		return -1;
	}
}


static unsigned short unpack_short ( char ** cur )
{
	unsigned short v;
	memcpy ( &v, *cur, sizeof(unsigned short) );
	(*cur) += sizeof(unsigned short);
	return ntohs ( v );
}


static unsigned int unpack_int ( char ** cur )
{
	unsigned int v;
	memcpy ( &v, *cur, sizeof(unsigned int) );
	(*cur) += sizeof(unsigned int);
	return ntohl ( v );
}


static char * unpack_str ( char ** cur )
{
	// we play a trick
	// we move the string in-place to free space for trailing zero but avoid malloc
	unsigned int len;
	len = unpack_int ( cur );
	memmove ( (*cur)-1, (*cur), len );
	(*cur) += len;
	(*cur)[-1] = '\0';
	return (*cur)-len-1;
}


static sphinx_uint64_t unpack_qword ( char ** cur )
{
	sphinx_uint64_t hi, lo;
	hi = unpack_int ( cur );
	lo = unpack_int ( cur );
	return ( hi<<32 ) + lo;
}


static float unpack_float ( char ** cur )
{
	union
	{
		unsigned int n;
		float f;
	} u;
	u.n = unpack_int ( cur );
	return u.f;
}


static void net_get_response ( int fd, sphinx_client * client )
{
	int len;
	char header_buf[32], *cur, *response;
	unsigned short status, ver;

	// dismiss previous response
	if ( client->response_buf )
	{
		free ( client->response_buf );
		client->response_len = 0;
		client->response_buf = NULL;
	}

	// read and parse the header
	if ( !net_read ( fd, header_buf, 8, client ) )
	{
		sock_close ( fd );
		if ( client->sock>0 )
			client->sock = -1;
		return;
	}

	cur = header_buf;
	status = unpack_short ( &cur );
	ver = unpack_short ( &cur );
	len = unpack_int ( &cur );

	// sanity check the length, alloc the buffer
	if ( len<0 || len>MAX_PACKET_LEN )
	{
		set_error ( client, "response length out of bounds (len=%d)", len );
		return;
	}

	response = malloc ( len );
	if ( !response )
	{
		set_error ( client, "malloc() failed (bytes=%d)", len );
		return;
	}

	// read the response
	if ( !net_read ( fd, response, len, client ) )
	{
		sock_close ( fd );
		if ( client->sock>0 )
			client->sock = -1;
		free ( response );
		return;
	}

	// check status
	cur = response;
	switch ( status )
	{
		case SEARCHD_OK:
		case SEARCHD_WARNING:
			if ( status==SEARCHD_WARNING )
				client->warning = unpack_str ( &cur );
			client->response_len = len;
			client->response_buf = response;
			client->response_start = cur;
			break;

		case SEARCHD_ERROR:
		case SEARCHD_RETRY:
			set_error ( client, "%s", unpack_str ( &cur ) );
			free ( response );
			break;

		default:
			set_error ( client, "unknown status code (status=%d)", status );
			free ( response );
			break;
	}

	// close one-time socket on success
	if ( client->sock<0 )
		sock_close ( fd );
}


sphinx_bool sphinx_open ( sphinx_client * client )
{
	char buf[16], *pbuf;

	if ( client->sock>=0 )
	{
		set_error ( client, "already connected" );
		return SPH_FALSE;
	}

	client->sock = net_connect ( client );
	if ( client->sock<0 )
		return SPH_FALSE;

	pbuf = buf;
	send_word ( &pbuf, SEARCHD_COMMAND_PERSIST );
	send_word ( &pbuf, 0 ); // dummy version
	send_int ( &pbuf, 4 ); // dummy body len
	send_int ( &pbuf, 1 ); // dummy body
	if ( !net_write ( client->sock, buf, (int)(pbuf-buf), client ) )
	{
		sock_close ( client->sock );
		client->sock = -1;
		return SPH_FALSE;
	}
	return SPH_TRUE;
}


sphinx_bool sphinx_close ( sphinx_client * client )
{
	if ( client->sock<0 )
	{
		set_error ( client, "not connected" );
		return SPH_FALSE;
	}

	sock_close ( client->sock );
	client->sock = -1;
	return SPH_TRUE;
}


static void * sphinx_malloc ( int len, sphinx_client * client )
{
	void * res;

	if ( len<0 || len>MAX_PACKET_LEN )
	{
		set_error ( client, "malloc() length out of bounds, possibly broken response packet (len=%d)", len );
		return NULL;
	}

	res = malloc ( len );
	if ( !res )
		set_error ( client, "malloc() failed (bytes=%d)", len );

	return res;
}


sphinx_result * sphinx_run_queries ( sphinx_client * client )
{
	int i, j, k, l, fd, len, nreqs, id64;
	char req_header[32], *req, *p, *pmax;
	sphinx_result * res;
	union un_attr_value * pval;

	if ( !client )
		return NULL;

	if ( client->num_reqs<=0 || client->num_reqs>MAX_REQS )
	{
		set_error ( client, "num_reqs=%d out of bounds (too many queries?)", client->num_reqs );
		return NULL;
	}

	fd = net_connect ( client );
	if ( fd<0 )
		return NULL;

	// free previous results
	sphinx_free_results ( client );

	// send query, get response
	len = 4;
	for ( i=0; i<client->num_reqs; i++ )
		len += client->req_lens[i];

	req = req_header;
	send_word ( &req, SEARCHD_COMMAND_SEARCH );
	send_word ( &req, client->ver_search );
	send_int ( &req, len );
	send_int ( &req, client->num_reqs );

	if ( !net_write ( fd, req_header, (int)(req-req_header), client ) )
		return NULL;

	for ( i=0; i<client->num_reqs; i++ )
		if ( !net_write ( fd, client->reqs[i], client->req_lens[i], client ) )
			return NULL;

	net_get_response ( fd, client );
	if ( !client->response_buf )
		return NULL;

	// dismiss requests
	nreqs = client->num_reqs;

	for ( i=0; i<client->num_reqs; i++ )
		free ( client->reqs[i] );
	client->num_reqs = 0;

	// parse response
	p = client->response_start;
	pmax = client->response_start + client->response_len; // max position for checks, to protect against broken responses

	for ( i=0; i<nreqs && p<pmax; i++ )
	{
		res = &client->results[i];
		client->num_results++;
		res->error = NULL;
		res->warning = NULL;

		res->status = unpack_int ( &p );
		if ( res->status!=SEARCHD_OK )
		{
			if ( res->status==SEARCHD_WARNING )
			{
				res->warning = unpack_str ( &p );
			} else 
			{
				res->error = unpack_str ( &p );
				continue;
			}
		}

		// fields
		res->num_fields = unpack_int ( &p );
		res->fields = sphinx_malloc ( res->num_fields*sizeof(const char*), client );
		if ( !res->fields )
			return NULL;

		for ( j=0; j<res->num_fields; j++ )
			res->fields[j] = unpack_str ( &p );

		// attrs
		res->num_attrs = unpack_int ( &p );
		res->attr_names = sphinx_malloc ( res->num_attrs*sizeof(const char*), client  );
		if ( !res->attr_names )
			return NULL;

		res->attr_types = sphinx_malloc ( res->num_attrs*sizeof(int), client  );
		if ( !res->attr_types )
			return NULL;

		for ( j=0; j<res->num_attrs; j++ )
		{
			res->attr_names[j] = unpack_str ( &p );
			res->attr_types[j] = unpack_int ( &p );
		}

		// match count, id bits flag
		res->num_matches = unpack_int ( &p );
		id64 = unpack_int ( &p );

		res->values_pool = sphinx_malloc ( (2+res->num_attrs) * res->num_matches * sizeof(union un_attr_value), client );
		if ( !res->values_pool )
			return NULL;
		pval = res->values_pool;

		// matches
		for ( j=0; j<res->num_matches; j++ )
		{
			// id
			if ( id64 )
				pval->int_value = unpack_qword ( &p );
			else
				pval->int_value = unpack_int ( &p );
			pval++;

			// weight
			pval->int_value = unpack_int ( &p );
			pval++;

			// attrs
			for ( k=0; k<res->num_attrs; k++ )
			{
				switch ( res->attr_types[k] )
				{
					case SPH_ATTR_MULTI | SPH_ATTR_INTEGER:
						/*!COMMIT this is totally unsafe on some arches (eg. SPARC)*/
						pval->mva_value = (unsigned int *) p; 
						len = unpack_int ( &p );
						for ( l=0; l<=len; l++ ) // including first one that is len
							pval->mva_value[l] = ntohl ( pval->mva_value[l] );
						p += len*sizeof(unsigned int);
						break;

					case SPH_ATTR_FLOAT:	pval->float_value = unpack_float ( &p ); break;
					case  SPH_ATTR_BIGINT:	pval->int_value = unpack_qword ( &p ); break;
					case SPH_ATTR_STRING:	pval->string = unpack_str ( &p ); break;
					default:				pval->int_value = unpack_int ( &p ); break;
				}
				pval++;
			}
		}

		// totals
		res->total = unpack_int ( &p );
		res->total_found = unpack_int ( &p );
		res->time_msec = unpack_int ( &p );
		res->num_words = unpack_int ( &p );

		if ( res->words )
			free ( res->words );
		res->words = sphinx_malloc ( res->num_words*sizeof(struct st_sphinx_wordinfo), client );
		if ( !res->words )
			return NULL;

		// words
		for ( j=0; j<res->num_words; j++ )
		{
			res->words[j].word = unpack_str ( &p );
			res->words[j].docs = unpack_int ( &p );
			res->words[j].hits = unpack_int ( &p );
		}

		// sanity check
		// FIXME! add it to each unpack?
		if ( p>pmax )
		{
			set_error ( client, "unpack error (req=%d, reqs=%d)", i, nreqs );
			return NULL;
		}
	}

	return client->results;
}

//////////////////////////////////////////////////////////////////////////

int sphinx_get_num_results ( sphinx_client * client )
{
	return client ? client->num_results : -1;
}


sphinx_uint64_t sphinx_get_id ( sphinx_result * result, int match )
{
	return sphinx_get_int ( result, match, -2 );
}


int sphinx_get_weight ( sphinx_result * result, int match )
{
	return (int)sphinx_get_int ( result, match, -1 );
}


sphinx_int64_t sphinx_get_int ( sphinx_result * result, int match, int attr )
{
	// FIXME! add safety and type checks
	union un_attr_value * pval;
	pval = result->values_pool;
	return pval [ (2+result->num_attrs)*match+2+attr ].int_value;
}


float sphinx_get_float ( sphinx_result * result, int match, int attr )
{
	// FIXME! add safety and type checks
	union un_attr_value * pval;
	pval = result->values_pool;
	return pval [ (2+result->num_attrs)*match+2+attr ].float_value;
}


unsigned int * sphinx_get_mva ( sphinx_result * result, int match, int attr )
{
	// FIXME! add safety and type checks
	union un_attr_value * pval;
	pval = result->values_pool;
	return pval [ (2+result->num_attrs)*match+2+attr ].mva_value;
}

const char * sphinx_get_string ( sphinx_result * result, int match, int attr )
{
	// FIXME! add safety and type checks
	union un_attr_value * pval;
	pval = result->values_pool;
	return pval [ (2+result->num_attrs)*match+2+attr ].string;
}

//////////////////////////////////////////////////////////////////////////

static sphinx_bool net_simple_query ( sphinx_client * client, char * buf, int req_len )
{
	int fd;

	fd = net_connect ( client );
	if ( fd<0 )
	{
		free ( buf );
		return SPH_FALSE;
	}

	if ( !net_write ( fd, buf, 8+req_len, client ) )
	{
		free ( buf );
		return SPH_FALSE;
	}
	free ( buf );

	net_get_response ( fd, client );
	if ( !client->response_buf )
		return SPH_FALSE;

	return SPH_TRUE;
}


void sphinx_init_excerpt_options ( sphinx_excerpt_options * opts )
{
	if ( !opts )
		return;

	opts->before_match		= NULL;
	opts->after_match		= NULL;
	opts->chunk_separator	= NULL;
	opts->html_strip_mode	= "index";

	opts->limit				= 0;
	opts->limit_passages	= 0;
	opts->limit_words		= 0;
	opts->around			= 0;
	opts->start_passage_id	= 1;

	opts->exact_phrase		= SPH_FALSE;
	opts->single_passage	= SPH_FALSE;
	opts->use_boundaries	= SPH_FALSE;
	opts->weight_order		= SPH_FALSE;
	opts->load_files		= SPH_FALSE;
}


char ** sphinx_build_excerpts ( sphinx_client * client, int num_docs, const char ** docs, const char * index, const char * words, sphinx_excerpt_options * opts )
{
	sphinx_excerpt_options opt;
	int i, req_len, flags;
	char *buf, *req, *p, *pmax, **result;

	if ( !client || !docs || !index || !words || num_docs<=0 )
	{
		if ( !docs )			set_error ( client, "invalid arguments (docs must not be empty)" );
		else if ( !index )		set_error ( client, "invalid arguments (index must not be empty)" );
		else if ( !words )		set_error ( client, "invalid arguments (words must not be empty)" );
		else if ( num_docs<=0 )	set_error ( client, "invalid arguments (num_docs must be positive)" );
		return NULL;
	}

	// fixup options
	sphinx_init_excerpt_options ( &opt );
	if ( opts )
	{
		opt.before_match		= opts->before_match ? opts->before_match : "<b>";
		opt.after_match			= opts->after_match ? opts->after_match : "</b>";
		opt.chunk_separator		= opts->chunk_separator ? opts->chunk_separator : " ... ";
		opt.html_strip_mode		= opts->html_strip_mode ? opts->html_strip_mode : "index";

		opt.limit				= opts->limit>0 ? opts->limit : 256;
		opt.limit_passages		= opts->limit_passages;
		opt.limit_words			= opts->limit_words;
		opt.around				= opts->around>0 ? opts->around : 5;
		opt.start_passage_id	= opts->start_passage_id;

		opt.exact_phrase		= opts->exact_phrase;
		opt.single_passage		= opts->single_passage;
		opt.use_boundaries		= opts->use_boundaries;
		opt.weight_order		= opts->weight_order;
		opt.load_files			= opts->load_files;

	} else
	{
		opt.before_match		= "<b>";
		opt.after_match			= "</b>";
		opt.chunk_separator		= " ... ";

		opt.limit				= 256;
		opt.around				= 5;
	}

	// alloc buffer
	req_len = (int)( 56 + strlen(index) + strlen(words) + strlen(opt.before_match)
		+ strlen(opt.after_match) + strlen(opt.chunk_separator) + strlen(opt.html_strip_mode) );
	for ( i=0; i<num_docs; i++ )
		req_len += (int)( 4 + safestrlen(docs[i]) );

	buf = malloc ( 12+req_len ); // request body length plus 12 header bytes
	if ( !buf )
	{
		set_error ( client, "malloc() failed (bytes=%d)", req_len );
		return NULL;
	}

	// build request
	req = buf;

	send_word ( &req, SEARCHD_COMMAND_EXCERPT );
	send_word ( &req, VER_COMMAND_EXCERPT );
	send_int ( &req, req_len );

	flags = 1; // remove spaces
	if ( opt.exact_phrase )		flags |= 2;
	if ( opt.single_passage )	flags |= 4;
	if ( opt.use_boundaries )	flags |= 8;
	if ( opt.weight_order )		flags |= 16;
	if ( opt.query_mode )		flags |= 32;
	if ( opt.force_all_words )	flags |= 64;
	if ( opt.load_files )		flags |= 128;

	send_int ( &req, 0 );
	send_int ( &req, flags );
	send_str ( &req, index );
	send_str ( &req, words );

	send_str ( &req, opt.before_match );
	send_str ( &req, opt.after_match );
	send_str ( &req, opt.chunk_separator );
	send_int ( &req, opt.limit );
	send_int ( &req, opt.around );

	send_int ( &req, opt.limit_passages ); // v1.2
	send_int ( &req, opt.limit_words );
	send_int ( &req, opt.start_passage_id );
	send_str ( &req, opt.html_strip_mode );

	send_int ( &req, num_docs );
	for ( i=0; i<num_docs; i++ )
		send_str ( &req, docs[i] );

	if ( (int)(req-buf)!=8+req_len )
	{
		set_error ( client, "internal error: failed to build request in sphinx_build_excerpts()" );
		free ( buf );
		return NULL;
	}

	// send query, get response
	if ( !net_simple_query ( client, buf, req_len ) )
		return NULL;

	// parse response
	p = client->response_start;
	pmax = client->response_start + client->response_len; // max position for checks, to protect against broken responses

	result = malloc ( (1+num_docs)*sizeof(char*) );
	if ( !result )
	{
		set_error ( client, "malloc() failed (bytes=%d)", (1+num_docs)*sizeof(char*) );
		return NULL;
	}

	for ( i=0; i<=num_docs; i++ )
		result[i] = NULL;

	for ( i=0; i<num_docs && p<pmax; i++ )
		result[i] = strdup ( unpack_str ( &p ) );

	if ( p>pmax )
	{
		for ( i=0; i<num_docs; i++ )
			if ( result[i] )
				free ( result[i] );

		set_error ( client, "unpack error" );
		return NULL;
	}

	// done
	return result;
}

//////////////////////////////////////////////////////////////////////////

int sphinx_update_attributes ( sphinx_client * client, const char * index, int num_attrs, const char ** attrs, int num_docs, const sphinx_uint64_t * docids, const sphinx_int64_t * values )
{
	int i, j, req_len;
	char *buf, *req, *p;

	// check args
	if ( !client || num_attrs<=0 || !attrs || num_docs<=0 || !docids || !values )
	{
		if ( num_attrs<=0 )		set_error ( client, "invalid arguments (num_attrs must be positive)" );
		else if ( !index )		set_error ( client, "invalid arguments (index must not be empty)" );
		else if ( !attrs )		set_error ( client, "invalid arguments (attrs must not empty)" );
		else if ( num_docs<=0 )	set_error ( client, "invalid arguments (num_docs must be positive)" );
		else if ( !docids )		set_error ( client, "invalid arguments (docids must not be empty)" );
		else if ( !values )		set_error ( client, "invalid arguments (values must not be empty)" );
	}

	// alloc buffer
	req_len = (int)( 12 + safestrlen(index) + (8+4*num_attrs)*num_docs );
	for ( i=0; i<num_attrs; i++ )
		req_len += (int)( 4 + safestrlen(attrs[i]) );

	buf = malloc ( 12+req_len ); // request body length plus 12 header bytes
	if ( !buf )
	{
		set_error ( client, "malloc() failed (bytes=%d)", req_len );
		return -1;
	}

	// build request
	req = buf;

	send_word ( &req, SEARCHD_COMMAND_UPDATE );
	send_word ( &req, VER_COMMAND_UPDATE );
	send_int ( &req, req_len );

	send_str ( &req, index );
	send_int ( &req, num_attrs );
	for ( i=0; i<num_attrs; i++ )
		send_str ( &req, attrs[i] );

	send_int ( &req, num_docs );
	for ( i=0; i<num_docs; i++ )
	{
		send_qword ( &req, docids[i] );
		for ( j=0; j<num_attrs; j++ )
			send_int ( &req, (unsigned int)( *values++ ) );
	}

	// send query, get response
	if ( !net_simple_query ( client, buf, req_len ) )
		return -1;

	// parse response
	if ( client->response_len<4 )
	{
		set_error ( client, "incomplete reply" );
		return -1;
	}

	p = client->response_start;
	return unpack_int ( &p );
}

int sphinx_update_attributes_mva	( sphinx_client * client, const char * index, const char * attr, sphinx_uint64_t docid, int num_values, const unsigned int * values )
{
	int i, req_len;
	char *buf, *req, *p;

	// check args
	if ( !client || !index || !attr || num_values<=0 || !values )
	{
		if ( !index )			set_error ( client, "invalid arguments (index must not be empty)" );
		else if ( !attr )		set_error ( client, "invalid arguments (attr must not empty)" );
		else if ( num_values<=0 )	set_error ( client, "invalid arguments (num_values must be positive)" );
		else if ( !values )		set_error ( client, "invalid arguments (values must not be empty)" );
	}

	// alloc buffer
	req_len = (int)( 38 + safestrlen(index) + safestrlen(attr) + num_values*4 );

	buf = malloc ( 12+req_len ); // request body length plus 12 header bytes
	if ( !buf )
	{
		set_error ( client, "malloc() failed (bytes=%d)", req_len );
		return -1;
	}

	// build request
	req = buf;

	send_word	( &req, SEARCHD_COMMAND_UPDATE );
	send_word	( &req, VER_COMMAND_UPDATE );
	send_int	( &req, req_len );

	send_str	( &req, index );
	send_int	( &req, 1 );
	send_str	( &req, attr );
	send_int	( &req, 1 ); // SPH_ATTR_MULTI flag

	send_int	( &req, 1 );
	send_qword	( &req, docid );
	send_int	( &req, num_values );
	for ( i=0; i<num_values; i++ )
		send_int ( &req, values[i] );

	// send query, get response
	if ( !net_simple_query ( client, buf, req_len ) )
		return -1;

	// parse response
	if ( client->response_len<4 )
	{
		set_error ( client, "incomplete reply" );
		return -1;
	}

	p = client->response_start;
	return unpack_int ( &p );
}

//////////////////////////////////////////////////////////////////////////

sphinx_keyword_info * sphinx_build_keywords ( sphinx_client * client, const char * query, const char * index, sphinx_bool hits, int * out_num_keywords )
{
	int i, req_len, nwords, len;
	char *buf, *req, *p, *pmax;
	sphinx_keyword_info *res;

	// check args
	if ( !client || !query || !index )
	{
		if ( !query )					set_error ( client, "invalid arguments (query must not be empty)" );
		else if ( !index )				set_error ( client, "invalid arguments (index must not be empty)" );
		else if ( !out_num_keywords )	set_error ( client, "invalid arguments (out_num_keywords must not be null)" );
		return NULL;
	}

	// alloc buffer
	req_len = (int)( safestrlen(query) + safestrlen(index) + 12 );

	buf = malloc ( 12+req_len ); // request body length plus 12 header bytes
	if ( !buf )
	{
		set_error ( client, "malloc() failed (bytes=%d)", req_len );
		return NULL;
	}

	// build request
	req = buf;

	send_word ( &req, SEARCHD_COMMAND_KEYWORDS );
	send_word ( &req, VER_COMMAND_KEYWORDS );
	send_int ( &req, req_len );

	send_str ( &req, query );
	send_str ( &req, index );
	send_int ( &req, hits );

	// send query, get response
	if ( !net_simple_query ( client, buf, req_len ) )
		return NULL;

	// parse response
	p = client->response_start;
	pmax = client->response_start + client->response_len; // max position for checks, to protect against broken responses

	nwords = unpack_int ( &p );
	*out_num_keywords = nwords;

	len = nwords*sizeof(sphinx_keyword_info);
	res = (sphinx_keyword_info*) malloc ( len );
	if ( !res )
	{
		set_error ( client, "malloc() failed (bytes=%d)", len );
		return NULL;
	}
	memset ( res, 0, len );

	for ( i=0; i<nwords && p<pmax; i++ )
	{
		res[i].tokenized = strdup ( unpack_str ( &p ) );
		res[i].normalized = strdup ( unpack_str ( &p ) );
		if ( hits )
		{
			res[i].num_docs = unpack_int ( &p );
			res[i].num_hits = unpack_int ( &p );
		}
	}
	// FIXME! add check for incomplete reply

	return res;
}

//////////////////////////////////////////////////////////////////////////

char ** sphinx_status ( sphinx_client * client, int * num_rows, int * num_cols )
{
	int i, j, k, n;
	char *p, *pmax, *req, *buf, **res;

	// check args
	if ( !client || !num_rows || !num_cols )
	{
		if ( !num_rows )		set_error ( client, "invalid arguments (num_rows must not be NULL)" );
		else if ( !num_cols )	set_error ( client, "invalid arguments (num_cols must not be NULL)" );
		return NULL;
	}

	// build request
	buf = malloc ( 12 );
	if ( !buf )
	{
		set_error ( client, "malloc() failed (bytes=12)" );
		return NULL;
	}

	req = buf;
	send_word ( &req, SEARCHD_COMMAND_STATUS );
	send_word ( &req, VER_COMMAND_STATUS );
	send_int ( &req, 4 );
	send_int ( &req, 1 );

	// send query, get response
	if ( !net_simple_query ( client, buf, 12 ) )
		return NULL;

	// parse response
	p = client->response_start;
	pmax = client->response_start + client->response_len; // max position for checks, to protect against broken responses

	*num_rows = unpack_int ( &p );
	*num_cols = unpack_int ( &p );
	n = (*num_rows)*(*num_cols);

	res = (char**) malloc ( n*sizeof(char*) );
	for ( i=0; i<n; i++ )
		res[i] = NULL;

	// FIXME! error checking?
	k = 0;
	for ( i=0; i<*num_rows; i++ )
		for ( j=0; j<*num_cols; j++ )
			res[k++] = strdup ( unpack_str ( &p ) );

	return res;
}

void sphinx_status_destroy ( char ** status, int num_rows, int num_cols )
{
	int i;
	for ( i=0; i<num_rows*num_cols; i++ )
		free ( status[i] );
	free ( status );
}

//
// $Id$
//
