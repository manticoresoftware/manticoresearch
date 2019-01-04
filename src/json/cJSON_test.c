//
// Created by Alexey on 2019-01-05.
//

// main purpose of this wrapper is to make some internal stuff of cJSON visible from outside
// for the sake of testing/benching. And keep original file untouched for the sake of no-conflicts merging on update.

#include "cJSON.c"

// stuff below is only for benching purpuses (used in gtests_functions.cpp)
int cJsonunescape ( char ** buf, cJSON * item )
{
	parse_buffer buffer = { 0, 0, 0, 0, { 0, 0, 0 } };
	char * value = *buf;

	/* reset error position */
	global_error.json = NULL;
	global_error.position = 0;
	if ( !value )
		return 0;

	buffer.content = ( const unsigned char * ) value;
	buffer.length = strlen ( ( const char * ) value ) + sizeof ( "" );
	buffer.offset = 0;
	buffer.hooks = global_hooks;

	parse_string ( item, &buffer );
	return 0;
}