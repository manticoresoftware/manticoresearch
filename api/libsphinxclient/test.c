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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if _WIN32
#include <winsock2.h>
#endif

#include "sphinxclient.h"

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


void test_query ( sphinx_client * client, sphinx_bool test_extras )
{
	sphinx_result * res;
	const char *query, *index;
	int i, j, k;
	unsigned int * mva;
	const char * field_names[2];
	int field_weights[2];

	sphinx_uint64_t override_docid = 4;
	unsigned int override_value = 456;

	query = "test";
	index = "test1";

	field_names[0] = "title";
	field_names[1] = "content";
	field_weights[0] = 100;
	field_weights[1] = 1;
	sphinx_set_field_weights ( client, 2, field_names, field_weights );
	field_weights[0] = 1;
	field_weights[1] = 1;

	if ( test_extras )
	{
		sphinx_add_override ( client, "group_id", &override_docid, 1, &override_value );
		sphinx_set_select ( client, "*, group_id*1000+@id*10 AS q" );
	}

	res = sphinx_query ( client, query, index, NULL );
	if ( !res )
		die ( "query failed: %s", sphinx_error(client) );

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
			case SPH_ATTR_MULTI | SPH_ATTR_INTEGER:
				mva = sphinx_get_mva ( res, i, j );
				printf ( "(" );
				for ( k=0; k<(int)mva[0]; k++ )
					printf ( k ? ",%u" : "%u", mva[1+k] );
				printf ( ")" );
				break;

			case SPH_ATTR_FLOAT:	printf ( "%f", sphinx_get_float ( res, i, j ) ); break;
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

	for ( j=0; j<2; j++ )
	{
		opts.exact_phrase = j;
		printf ( "exact_phrase=%d\n", j );

		res = sphinx_build_excerpts ( client, ndocs, docs, index, words, &opts );
		if ( !res )
			die ( "query failed: %s", sphinx_error(client) );

		for ( i=0; i<ndocs; i++ )
			printf ( "n=%d, res=%s\n", 1+i, res[i] );
		printf ( "\n" );
	}
}


void test_update ( sphinx_client * client )
{
	const char * attr = "group_id";
	const sphinx_uint64_t id = 2;
	const sphinx_int64_t val = 123;
	int res;

	res = sphinx_update_attributes ( client, "test1", 1, &attr, 1, &id, &val );
	if ( res<0 )
		printf ( "update failed: %s\n\n", sphinx_error(client) );
	else
		printf ( "update success, %d rows updated\n\n", res );
}


void test_keywords ( sphinx_client * client )
{
	int i, nwords;
	sphinx_keyword_info * words;

	words = sphinx_build_keywords ( client, "hello test one", "test1", SPH_TRUE, &nwords );
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
		printf ( "status failed: %s\n\n", sphinx_error(client) );
		return;
	}

	k = 0;
	for ( i=0; i<num_rows; i++ )
	{
		for ( j=0; j<num_cols; j++, k++ )
			printf ( ( j==0 ) ? "%s:" : " %s", status[k] );
		printf ( "\n" );
	}
	printf ( "\n" );

	sphinx_status_destroy ( status, num_rows, num_cols );
}


int main ()
{
	sphinx_client * client;

	net_init ();

	client = sphinx_create ( SPH_TRUE );
	if ( !client )
		die ( "failed to create client" );

	test_query ( client, SPH_FALSE );
	test_excerpt ( client );
	test_update ( client );
	test_query ( client, SPH_FALSE );
	test_keywords ( client );
	test_query ( client, SPH_TRUE );

	sphinx_open ( client );
	test_update ( client );
	test_update ( client );
	sphinx_close ( client );

	test_status ( client );

	sphinx_destroy ( client );
	return 0;
}

//
// $Id$
//
