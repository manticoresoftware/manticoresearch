#include "sphinxclient.c"

#include <stdio.h>

static unsigned int read_be32 ( const char * p )
{
	return ( (unsigned int)(unsigned char)p[0] << 24 )
		| ( (unsigned int)(unsigned char)p[1] << 16 )
		| ( (unsigned int)(unsigned char)p[2] << 8 )
		| (unsigned int)(unsigned char)p[3];
}

static int check_request ( sphinx_client * client, unsigned int flags, unsigned int ranker )
{
	if ( sphinx_add_query ( client, "query", "*", "" )!=0
		|| read_be32 ( client->reqs[0] )!=flags
		|| read_be32 ( client->reqs[0]+16 )!=ranker )
	{
		fprintf ( stderr, "unexpected request: flags=%u ranker=%u\n", read_be32(client->reqs[0]), read_be32(client->reqs[0]+16) );
		return 0;
	}
	sphinx_dismiss_requests ( client );
	return 1;
}

int main ( void )
{
	sphinx_client * client = sphinx_create ( SPH_TRUE );
	if ( !client || !check_request ( client, 1U<<6, SPH_RANK_PROXIMITY_BM25 )
		|| !sphinx_set_ranking_mode ( client, SPH_RANK_PROXIMITY_BM25, NULL )
		|| !check_request ( client, (1U<<6) | (1U<<15), SPH_RANK_PROXIMITY_BM25 )
		|| !sphinx_set_ranking_mode ( client, SPH_RANK_BM25, NULL )
		|| !check_request ( client, (1U<<6) | (1U<<15), SPH_RANK_BM25 )
		|| !sphinx_set_query_flags ( client, "boolean_simplify", SPH_TRUE, 0 )
		|| !sphinx_set_query_flags ( client, "boolean_simplify", SPH_FALSE, 0 )
		|| !check_request ( client, (1U<<6) | (1U<<15), SPH_RANK_BM25 ) )
		return 1;
	sphinx_reset_query_flags ( client );
	if ( !check_request ( client, (1U<<6) | (1U<<15), SPH_RANK_BM25 ) )
		return 1;
	sphinx_destroy ( client );
	return 0;
}
