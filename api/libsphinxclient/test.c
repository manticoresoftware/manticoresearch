//
// $Id$
//

//
// Copyright (c) 2001-2014, Andrew Aksyonoff
// Copyright (c) 2008-2014, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License. You should
// have received a copy of the LGPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if _WIN32
#include <winsock2.h>
#endif

#include "sphinxclient.h"

static sphinx_bool g_smoke = SPH_FALSE;
static int g_failed = 0;


void die ( const char * template, ... )
{
	va_list ap;
	va_start ( ap, template );
	printf ( "FATAL: " );
	vprintf ( template, ap );
	printf ( "\n" );
	va_end ( ap );
	exit ( 1 );
}


void net_init ()
{
#if _WIN32
	// init WSA on Windows
	WSADATA wsa_data;
	int wsa_startup_err;

	wsa_startup_err = WSAStartup ( WINSOCK_VERSION, &wsa_data );
	if ( wsa_startup_err )
		die ( "failed to initialize WinSock2: error %d", wsa_startup_err );
#endif
}


void test_query ( sphinx_client * client, const char * query, const char * index )
{
	sphinx_result * res;
	int i, j, k, mva_len;
	unsigned int * mva;
	const char * field_names[2];
	int field_weights[2];

	field_names[0] = "title";
	field_names[1] = "content";
	field_weights[0] = 100;
	field_weights[1] = 1;
	sphinx_set_field_weights ( client, 2, field_names, field_weights );
	field_weights[0] = 1;
	field_weights[1] = 1;

	res = sphinx_query ( client, query, index, NULL );

	if ( !res )
	{
		g_failed += ( res==NULL );

		if ( !g_smoke )
			die ( "query failed: %s", sphinx_error(client) );
	}

	if ( g_smoke )
		printf ( "Query '%s' retrieved %d of %d matches.\n", query, res->total, res->total_found );
	else
		printf ( "Query '%s' retrieved %d of %d matches in %d.%03d sec.\n",
			query, res->total, res->total_found, res->time_msec/1000, res->time_msec%1000 );

	printf ( "Query stats:\n" );
	for ( i=0; i<res->num_words; i++ )
		printf ( "\t'%s' found %d times in %d documents\n",
		res->words[i].word, res->words[i].hits, res->words[i].docs );

	printf ( "\nMatches:\n" );
	for ( i=0; i<res->num_matches; i++ )
	{
		printf ( "%d. doc_id=%d, weight=%d", 1+i,
			(int)sphinx_get_id ( res, i ), sphinx_get_weight ( res, i ) );

		for ( j=0; j<res->num_attrs; j++ )
		{
			printf ( ", %s=", res->attr_names[j] );
			switch ( res->attr_types[j] )
			{
			case SPH_ATTR_MULTI64:
			case SPH_ATTR_MULTI:
				mva = sphinx_get_mva ( res, i, j );
				mva_len = *mva++;
				printf ( "(" );
				for ( k=0; k<mva_len; k++ )
					printf ( k ? ",%u" : "%u", ( res->attr_types[j]==SPH_ATTR_MULTI ? mva[k] : (unsigned int)sphinx_get_mva64_value ( mva, k ) ) );
				printf ( ")" );
				break;

			case SPH_ATTR_FLOAT:	printf ( "%f", sphinx_get_float ( res, i, j ) ); break;
			case SPH_ATTR_STRING:	printf ( "%s", sphinx_get_string ( res, i, j ) ); break;
			default:				printf ( "%u", (unsigned int)sphinx_get_int ( res, i, j ) ); break;
			}
		}

		printf ( "\n" );
	}
	printf ( "\n" );
}


void test_excerpt ( sphinx_client * client )
{
	const char * docs[] =
	{
		"this is my test text to be highlighted, and for the sake of the testing we need to pump its length somewhat",
		"another test text to be highlighted, below limit",
		"test number three, without phrase match",
		"final test, not only without phrase match, but also above limit and with swapped phrase text test as well"
	};
	const int ndocs = sizeof(docs)/sizeof(docs[0]);
	const char * words = "test text";
	const char * index = "test1";
	sphinx_excerpt_options opts;
	char ** res;
	int i, j;

	sphinx_init_excerpt_options ( &opts );
	opts.limit = 60;
	opts.around = 3;
	opts.allow_empty = SPH_FALSE;

	for ( j=0; j<2; j++ )
	{
		opts.exact_phrase = j;
		printf ( "exact_phrase=%d\n", j );

		res = sphinx_build_excerpts ( client, ndocs, docs, index, words, &opts );

		if ( !res )
		{
			g_failed += ( res==NULL );
			if ( !g_smoke )
				die ( "query failed: %s", sphinx_error(client) );
		}

		for ( i=0; i<ndocs; i++ )
			printf ( "n=%d, res=%s\n", 1+i, res[i] );
		printf ( "\n" );
	}
}


void test_excerpt_spz ( sphinx_client * client )
{
	const char * docs[] =
	{
		"<efx_unidentified_table>"
		"The institutional investment manager it. Is Filing this report and."
		"<efx_test>"
		"It is signed hereby represent. That it is all information."
		"are It or is"
		"</efx_test>"
		"<efx_2>"
		"cool It is cooler"
		"</efx_2>"
		"It is another place!"
		"</efx_unidentified_table>"
	};
	const int ndocs = sizeof(docs)/sizeof(docs[0]);
	const char * words = "it is";
	const char * index = "test1";
	sphinx_excerpt_options opts;
	char ** res;
	int i, j;

	sphinx_init_excerpt_options ( &opts );
	opts.limit = 150;
	opts.limit_passages = 8;
	opts.around = 8;
	opts.html_strip_mode = "strip";
	opts.passage_boundary = "zone";
	opts.emit_zones = SPH_TRUE;

	for ( j=0; j<2; j++ )
	{
		if ( j==1 )
		{
			opts.passage_boundary = "sentence";
			opts.emit_zones = SPH_FALSE;
		}
		printf ( "passage_boundary=%s\n", opts.passage_boundary );

		res = sphinx_build_excerpts ( client, ndocs, docs, index, words, &opts );
		if ( !res )
			die ( "query failed: %s", sphinx_error(client) );

		for ( i=0; i<ndocs; i++ )
			printf ( "n=%d, res=%s\n", 1+i, res[i] );
		printf ( "\n" );
	}
}


void test_persist_work ( sphinx_client * client )
{
	char * docs[] = { NULL };
	const char words[] = "that is";
	const char * index = "test1";
	const char filler[] = " no need to worry about ";
	sphinx_excerpt_options opts;
	char ** res;
	char * doc;
	int i ;

	// should be in sync with sphinxclient.c MAX_PACKET_LEN
	i = 8*1024*1024 + 50;
	docs[0] = malloc ( i );
	if ( !docs[0] )
		die ( "malloc failed at test_persist_work" );

	memcpy ( docs[0], words, sizeof(words)-1 );
	doc = docs[0] + sizeof(words)-1;
	while ( ( doc + sizeof(filler) )<docs[0]+i )
	{
		memcpy ( doc, filler, sizeof(filler)-1 );
		doc += sizeof(filler)-1;
	}
	*doc = '\0';

	sphinx_open ( client );

	for ( i=0; i<2; i++ )
	{
		if ( i==0 )
		{
			sphinx_init_excerpt_options ( &opts );
			opts.limit = 0;
			opts.limit_passages = 0;
			opts.around = 0;
			opts.html_strip_mode = "none";
			opts.query_mode = SPH_TRUE;
		} else
		{
			sphinx_init_excerpt_options ( &opts );
			opts.limit = 500;
			opts.limit_words = 10;
			opts.limit_passages = 2;
			opts.around = 5;
			opts.html_strip_mode = "none";
			opts.query_mode = SPH_TRUE;

			*( docs[0]+sizeof(words)+100 ) = '\0';
		}

		printf ( "n=%d,\t", i );
		res = sphinx_build_excerpts ( client, 1, docs, index, words, &opts );

		if ( !res )
		{
			g_failed += ( res==NULL && i ); // 1st query fails, 2nd works
			printf ( "query failed: %s", sphinx_error(client) );
		} else
		{
			printf ( "res=%s", res[0] );
			free ( res );
		}
		printf ( "\n" );
	}
	sphinx_close ( client );
	printf ( "\n" );
}


void test_update ( sphinx_client * client, sphinx_uint64_t id )
{
	const char * attr = "group_id";
	const sphinx_int64_t val = 123;
	int res;

	res = sphinx_update_attributes ( client, "test1", 1, &attr, 1, &id, &val );
	if ( res<0 )
		g_failed++;

	if ( res<0 )
		printf ( "update failed: %s\n\n", sphinx_error(client) );
	else
		printf ( "update success, %d rows updated\n\n", res );
}

void test_update_mva ( sphinx_client * client )
{
	const char * attr = "tag";
	const sphinx_uint64_t id = 3;
	const unsigned int vals[] = { 7, 77, 177 };
	int res;

	res = sphinx_update_attributes_mva ( client, "test1", attr, id, sizeof(vals)/sizeof(vals[0]), vals );
	if ( res<0 )
		g_failed++;

	if ( res<0 )
		printf ( "update mva failed: %s\n\n", sphinx_error(client) );
	else
		printf ( "update mva success, %d rows updated\n\n", res );
}


void test_keywords ( sphinx_client * client )
{
	int i, nwords;
	sphinx_keyword_info * words;

	words = sphinx_build_keywords ( client, "hello test one", "test1", SPH_TRUE, &nwords );

	g_failed += ( words==NULL );

	if ( !words )
	{
		printf ( "build_keywords failed: %s\n\n", sphinx_error(client) );
	} else
	{
		printf ( "build_keywords result:\n" );
		for ( i=0; i<nwords; i++ )
			printf ( "%d. tokenized=%s, normalized=%s, docs=%d, hits=%d\n", 1+i,
				words[i].tokenized, words[i].normalized,
				words[i].num_docs, words[i].num_hits );
		printf ( "\n" );
	}
}


void test_status ( sphinx_client * client )
{
	int num_rows, num_cols, i, j, k;
	char ** status;

	status = sphinx_status ( client, &num_rows, &num_cols );
	if ( !status )
	{
		g_failed++;
		printf ( "status failed: %s\n\n", sphinx_error(client) );
		return;
	}

	k = 0;
	for ( i=0; i<num_rows; i++ )
	{
		if ( !g_smoke || ( strstr ( status[k], "time" )==NULL && strstr ( status[k], "wall" )==NULL && strstr ( status[k], "wait" )==NULL ) )
		{
			for ( j=0; j<num_cols; j++, k++ )
				printf ( ( j==0 ) ? "%s:" : " %s", status[k] );
			printf ( "\n" );
		} else
			k += num_cols;
	}
	printf ( "\n" );

	sphinx_status_destroy ( status, num_rows, num_cols );
}

void test_group_by ( sphinx_client * client, const char * attr )
{
	sphinx_set_groupby ( client, attr, SPH_GROUPBY_ATTR, "@group asc" );
	test_query ( client, "is", "test1" );

	sphinx_reset_groupby ( client );
}

void test_filter ( sphinx_client * client )
{
	const char * attr_group = "group_id";
	const char * attr_mva = "tag";
	sphinx_int64_t filter_group = { 1 };
	sphinx_int64_t filter_mva = { 7 };
	int i;
	sphinx_bool mva;

	for ( i=0; i<2; i++ )
	{
		mva = ( i==1 );
		sphinx_add_filter ( client, mva ? attr_mva : attr_group, 1, mva ? &filter_mva : &filter_group, SPH_FALSE );
		test_query ( client, "is", "test1" );

		sphinx_reset_filters ( client );
	}
}

void title ( const char * name )
{
	if ( g_smoke || !name )
		return;

	printf ( "-> % s <-\n\n", name );
}

int main ( int argc, char ** argv )
{
	int i, port = 0;
	sphinx_client * client;
	sphinx_uint64_t override_docid = 2;
	unsigned int override_value = 2000;

	for ( i=1; i<argc; i++ )
	{
		if ( strcmp ( argv[i], "--smoke" )==0 )
			g_smoke = SPH_TRUE;
		else if ( strcmp ( argv[i], "--port" )==0 && i+1<argc )
			port = (int)strtoul ( argv[i+1], NULL, 10 );
	}

	net_init ();

	client = sphinx_create ( SPH_TRUE );
	if ( !client )
		die ( "failed to create client" );

	if ( port )
		sphinx_set_server ( client, "127.0.0.1", port );

	sphinx_set_match_mode ( client, SPH_MATCH_EXTENDED2 );
	sphinx_set_sort_mode ( client, SPH_SORT_RELEVANCE, NULL );

	// excerpt + keywords
	title ( "excerpt" );
	test_excerpt ( client );
	test_excerpt_spz ( client );

	title ( "keywords" );
	test_keywords ( client );

	// search phase 0
	title ( "search phase 0" );
	test_query ( client, "is", "test1" );
	test_query ( client, "is test", "test1" );
	test_query ( client, "test number", "test1" );
	test_query ( client, "is", "dist" );

	// group_by (attr; mva) + filter
	title ( "group_by (attr; mva) + filter" );
	title ( "group_by attr" );
	test_group_by ( client, "group_id" );
	// group_by mva
	title ( "group_by mva" );
	test_group_by ( client, "tag" );
	// filter
	title ( "filter" );
	test_filter ( client );

	// update (attr; mva) + sort (descending id)
	title ( "update (attr; mva) + sort (descending id)" );
	test_update ( client, 2 );
	test_update_mva ( client );
	sphinx_set_sort_mode ( client, SPH_SORT_EXTENDED, "idd desc" );
	test_query ( client, "is", "test1" );

	// persistence connection
	sphinx_open ( client );

	// update (attr) + sort (default)
	title ( "update (attr) + sort (default)" );
	test_update ( client, 4 );
	test_update ( client, 3 );
	sphinx_set_sort_mode ( client, SPH_SORT_RELEVANCE, NULL );
	test_query ( client, "is", "test1" );

	sphinx_cleanup ( client );

	// group_by (attr; mva) + filter + post update
	title ( "group_by (attr; mva) +  filter + post update" );
	title ( "group_by attr" );
	test_group_by ( client, "group_id" );
	title ( "group_by mva" );
	test_group_by ( client, "tag" );
	title ( "filter" );
	test_filter ( client );

	// select
	title ( "select" );
	sphinx_set_select ( client, "*, group_id*1000+@id*10 AS q" );
	test_query ( client, "is", "test1" );

	// override
	title ( "override" );
	sphinx_add_override ( client, "group_id", &override_docid, 1, &override_value );
	test_query ( client, "is", "test1" );

	// group_by (override attr)
	title ( "group_by (override attr)" );
	test_group_by ( client, "group_id" );

	sphinx_close ( client );

	test_status ( client );

	// long queries
	title ( "long queries vs persist connection" );
	test_persist_work ( client );

	sphinx_destroy ( client );

	if ( g_smoke && g_failed )
	{
		printf ( "%d error(s)\n", g_failed );
		exit ( 1 );
	}

	return 0;
}

//
// $Id$
//
