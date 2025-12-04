//
// Sphinx UDF function example
//
// Linux
// gcc -fPIC -shared -o udfexample.so udfexample.c
// CREATE FUNCTION sequence RETURNS INT SONAME 'udfexample.so';
// CREATE FUNCTION strtoint RETURNS INT SONAME 'udfexample.so';
// CREATE FUNCTION avgmva RETURNS FLOAT SONAME 'udfexample.so';
// CREATE FUNCTION makemva RETURNS MULTI SONAME 'udfexample.so';
// CREATE FUNCTION makemva64 RETURNS MULTI64 SONAME 'udfexample.so';
// CREATE FUNCTION makefloatvec RETURNS FLOAT_VECTOR SONAME 'udfexample.so';
// Note: makemva, makemva64, and makefloatvec accept an optional integer seed parameter
// for deterministic results: makemva(seed), makemva64(seed), makefloatvec(seed)
//
// Windows
// cl /MTd /LD udfexample.c
// CREATE FUNCTION sequence RETURNS INT SONAME 'udfexample.dll';
// CREATE FUNCTION strtoint RETURNS INT SONAME 'udfexample.dll';
// CREATE FUNCTION avgmva RETURNS FLOAT SONAME 'udfexample.dll';
// CREATE FUNCTION makemva RETURNS MULTI SONAME 'udfexample.dll';
// CREATE FUNCTION makemva64 RETURNS MULTI64 SONAME 'udfexample.dll';
// CREATE FUNCTION makefloatvec RETURNS FLOAT_VECTOR SONAME 'udfexample.dll';
// Note: makemva, makemva64, and makefloatvec accept an optional integer seed parameter
// for deterministic results: makemva(seed), makemva64(seed), makefloatvec(seed)
//

#include "sphinxudf.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// ByteBlob_t typedef for MVA return values
typedef unsigned char BYTE;
typedef struct { const BYTE * first; int64_t second; } ByteBlob_t;

#ifndef nullptr
#define nullptr NULL
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// UDF logging callback
/// write any message into daemon's log.
sphinx_log_fn* sphlog = NULL;

/// UDF set logging callback
/// gets called once when the library is loaded; daemon set callback function in this call
DLLEXPORT void udfexample_setlogcb ( sphinx_log_fn* cblog )
{
	sphlog = cblog;
}

void UdfLog ( char * szMsg )
{
	if ( sphlog )
		( *sphlog ) ( szMsg, -1 );
}

/// UDF version control
/// gets called once when the library is loaded
DLLEXPORT int udfexample_ver ()
{
	UdfLog ( "Called udfexample_ver" );
	// Initialize random seed based on current time
	srand((unsigned int)time(NULL));
	return SPH_UDF_VERSION;
}


/// UDF re-initialization func
/// gets called on sighup (workers=prefork only)
DLLEXPORT void udfexample_reinit ()
{
	UdfLog ( "Called udfexample_reinit" );
}


/// UDF initialization
/// gets called on every query, when query begins
/// args are filled with values for a particular query
DLLEXPORT int sequence_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_message )
{
	UdfLog ( "Called sequence_init" );
	// check argument count
	if ( args->arg_count > 1 )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "SEQUENCE() takes either 0 or 1 arguments" );
		return 1;
	}

	// check argument type
	if ( args->arg_count && args->arg_types[0]!=SPH_UDF_TYPE_UINT32 )
    {
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "SEQUENCE() requires 1st argument to be uint" );
		return 1;
	}

	// allocate and init counter storage
	init->func_data = (void*) malloc ( sizeof(int) );
	if ( !init->func_data )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "malloc() failed" );
		return 1;
	}
	*(int*)init->func_data = 1;

	// all done
	return 0;
}


/// UDF deinitialization
/// gets called on every query, when query ends
DLLEXPORT void sequence_deinit ( SPH_UDF_INIT * init )
{
	UdfLog ( "Called sequence_deinit" );
	// deallocate storage
	if ( init->func_data )
	{
		free ( init->func_data );
		init->func_data = NULL;
	}
}


/// UDF implementation
/// gets called for every row, unless optimized away
DLLEXPORT sphinx_int64_t sequence ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
	UdfLog ( "Called sequence" );
	int res = (*(int*)init->func_data)++;
	if ( args->arg_count )
		res += *(int*)args->arg_values[0];
	return res;
}

//////////////////////////////////////////////////////////////////////////

DLLEXPORT int strtoint_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_message )
{
	UdfLog ( "Called strtoint_init" );
	if ( args->arg_count!=1 || args->arg_types[0]!=SPH_UDF_TYPE_STRING )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "STRTOINT() requires 1 string argument" );
		return 1;
	}
	return 0;
}

DLLEXPORT sphinx_int64_t strtoint ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
	UdfLog ( "Called strtoint" );
	const char * s = args->arg_values[0];
	int len = args->str_lengths[0], res = 0;

	// looks strange, but let's just take sum of digits, i.e. '123' -> 1+2+3 = 6.
	while ( len>0 && *s>='0' && *s<='9' )
	{
		res += *s - '0';
		--len;
		++s;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////

DLLEXPORT int avgmva_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_message )
{
	UdfLog ( "Called avgmva_init" );
	sphinx_int64_t mvattype;

	if ( args->arg_count!=1 ||
		( args->arg_types[0]!=SPH_UDF_TYPE_UINT32SET && args->arg_types[0]!=SPH_UDF_TYPE_INT64SET ) )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "AVGMVA() requires 1 MVA argument" );
		return 1;
	}

	// store our mva vs mva64 flag to func_data
	mvattype = args->arg_types[0]==SPH_UDF_TYPE_INT64SET ? 1 : 0;
	init->func_data = (void*)mvattype;
	return 0;
}

DLLEXPORT double avgmva ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
	UdfLog ( "Called avgmva" );
	unsigned int * mva = (unsigned int *) args->arg_values[0];
	double res = 0;
	int n = args->str_lengths[0];
	int i, is64;

	if ( !n )
		return res;

	// Both MVA32 and MVA64 are stored as dword (unsigned 32-bit) arrays.
	// num of the values stored in args.str_lengths
	// In pseudocode:
	//
	// num_values = args->str_lengths[idx]
	// unsigned int * values = args->arg_values[idx]
	//
	// With MVA32, this lets you access the values pretty naturally.
	//
	// With MVA64, however, we have to assemble 64-bit values from dword pairs
	//
	// The latter is required for architectures where non-aligned
	// 64-bit access crashes. On Intel, we could have also done it
	// like this:
	//
	// sphinx_int64_t * values = (sphinx_int64_t*) args->values[idx];

	// pull "mva32 or mva64" flag (that we stored in _init) from func_data
	is64 = (sphinx_int64_t)(init->func_data) != 0;
	if ( is64 )
	{
		// handle mva64
		// consider that we're on intel arch and no special conversion is necessary
		sphinx_int64_t * mva64 = (sphinx_int64_t*)mva;
		for ( i=0; i<n; i++ )
			res += *mva64++;
	} else
	{
		// handle mva32
		for ( i=0; i<n; i++ )
			res += *mva++;
	}

	return res/n;
}

//////////////////////////////////////////////////////////////////////////

// very simple email hider with exception
// symbols @ and . should be in charset_table
// it works on indexing by processing tokens as:
// - keep emails to domain 'space.io' and transform them to 'mailto:any@space.io' then searching for query with 'mailto:*' should return all documents with emails
// - deletes all other emails, ie returns NULL for 'test@gmail.com'

DLLEXPORT int hideemail_init ( void ** userdata, int num_fields, const char ** field_names, const char * options, char * error_message )
{
	UdfLog ( "Called hideemail_init" );
	*userdata = (void*) malloc ( sizeof(char) * SPH_UDF_ERROR_LEN );
	return 0;
}


DLLEXPORT char * hideemail_push_token ( void * userdata, char * token, int * extra, int * delta )
{
	UdfLog ( "Called hideemail_push_token" );
	char * dst = (char *)userdata;
	char domain[] = "space.io";
	char prefix[] = "mailto:";
	char * pos0;
	char * pos1;
	int len0, len1 = ( sizeof(domain)-1 ), lenprefix = ( sizeof(prefix)-1 );

	*delta = 1;

	if ( !token || !*token )
		return token;

	pos0 = strchr ( token, '@' );
	pos1 = ( pos0!=0 ? strchr ( pos0, '.' ) : 0 );

	// not email case
	if ( pos0==0 || pos1==0 )
		return token;

	len0 = (int) strlen ( token );

	// domain size not same as expected - hide email
	if ( len0-(pos0+1-token)!=len1 )
		return 0;

	// domain name does not match - hide email
	if ( strncmp ( pos0 + 1, domain, len1 )!=0 )
		return 0;

	strcpy ( dst, prefix );
	strcpy ( dst + lenprefix, token );
	dst[lenprefix+len0] = '\0';

	return dst;
}

DLLEXPORT char * hideemail_get_extra_token ( void * userdata, int * delta )
{
	UdfLog ( "Called hideemail_get_extra_token" );
	*delta = 0;
	return 0;
}

DLLEXPORT int hideemail_begin_document ( void * userdata, const char * options, char * error_message )
{
	UdfLog ( "Called hideemail_begin_document" );
	return 0;
}

DLLEXPORT void hideemail_deinit ( void * userdata )
{
	UdfLog ( "Called hideemail_deinit" );
	if ( userdata )
	{
		free ( userdata );
	}
}


//////////////////////////////////////////////////////////////////////////
// MVA return example - returns a random MVA with 1-20 random values
// Optional seed parameter for deterministic results

// Simple Linear Congruential Generator for deterministic random numbers
static uint32_t lcg_next ( uint32_t * state )
{
	*state = *state * 1103515245U + 12345U;
	return *state;
}

typedef struct { uint32_t seed; uint32_t counter; } SeedData_t;

DLLEXPORT int makemva_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_message )
{
	UdfLog ( "Called makemva_init" );
	if ( args->arg_count > 1 )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "MAKEMVA() takes 0 or 1 argument (optional seed)" );
		return 1;
	}
	
	// Allocate storage for seed value and call counter
	SeedData_t * pData = (SeedData_t *) malloc ( sizeof(SeedData_t) );
	if ( !pData )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "malloc() failed" );
		return 1;
	}
	
	// Initialize: seed=0 means not set, counter starts at 0
	pData->seed = 0;
	pData->counter = 0;
	
	init->func_data = pData;
	return 0;
}

DLLEXPORT void makemva_deinit ( SPH_UDF_INIT * init )
{
	if ( init->func_data )
	{
		free ( init->func_data );
		init->func_data = NULL;
	}
}

DLLEXPORT ByteBlob_t makemva ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
	SeedData_t * pData = (SeedData_t *)init->func_data;
	uint32_t state;
	
	// Check if seed was explicitly provided (arg_values not available during _init)
	if ( args->arg_count == 1 && args->arg_values && args->arg_values[0] )
	{
		// Seed was provided - use it deterministically for all rows
		if ( pData->seed == 0 )
		{
			// First call with seed - initialize it
			if ( args->arg_types[0] == SPH_UDF_TYPE_UINT32 )
				pData->seed = *(uint32_t*)args->arg_values[0];
			else if ( args->arg_types[0] == SPH_UDF_TYPE_INT64 )
				pData->seed = (uint32_t)(*(sphinx_int64_t*)args->arg_values[0]);
			else
				pData->seed = (uint32_t)time(NULL);
			// Ensure seed is non-zero (0 is our "not initialized" marker)
			if ( pData->seed == 0 )
				pData->seed = 1;
		}
		// Use the stored seed for deterministic results
		state = pData->seed;
	}
	else
	{
		// No seed provided - generate new random seed for each row
		// Use time + counter + pointer address for variation
		pData->counter++;
		state = (uint32_t)time(NULL) ^ (uint32_t)((uintptr_t)init->func_data) ^ pData->counter;
	}
	
	// Generate random array length from 1 to 20
	int count = (lcg_next(&state) % 20) + 1;
	
	// Allocate memory for random number of uint32 values using the provided malloc function
	uint32_t * pValues = (uint32_t *) args->fn_malloc ( count * sizeof(uint32_t) );
	if ( !pValues )
	{
		*error_flag = 1;
		return (ByteBlob_t){nullptr, 0};
	}
	
	// Fill with random values
	for ( int i = 0; i < count; i++ )
		pValues[i] = (lcg_next(&state) % 1000) + 1; // Random values from 1 to 1000
	
	// Sort values (required for any() function to work with binary search)
	// Simple bubble sort for small arrays (typically 1-20 elements)
	for ( int i = 0; i < count - 1; i++ )
	{
		for ( int j = 0; j < count - i - 1; j++ )
		{
			if ( pValues[j] > pValues[j+1] )
			{
				uint32_t temp = pValues[j];
				pValues[j] = pValues[j+1];
				pValues[j+1] = temp;
			}
		}
	}
	
	// Return as ByteBlob_t
	return (ByteBlob_t){(const BYTE*)pValues, count * sizeof(uint32_t)};
}

//////////////////////////////////////////////////////////////////////////
// MVA64 return example - returns a random MVA64 with 1-20 random values
// Optional seed parameter for deterministic results

DLLEXPORT int makemva64_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_message )
{
	if ( args->arg_count > 1 )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "MAKEMVA64() takes 0 or 1 argument (optional seed)" );
		return 1;
	}
	
	// Allocate storage for seed value and call counter
	SeedData_t * pData = (SeedData_t *) malloc ( sizeof(SeedData_t) );
	if ( !pData )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "malloc() failed" );
		return 1;
	}
	
	// Initialize: seed=0 means not set, counter starts at 0
	pData->seed = 0;
	pData->counter = 0;
	
	init->func_data = pData;
	return 0;
}

DLLEXPORT void makemva64_deinit ( SPH_UDF_INIT * init )
{
	if ( init->func_data )
	{
		free ( init->func_data );
		init->func_data = NULL;
	}
}

DLLEXPORT ByteBlob_t makemva64 ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
	SeedData_t * pData = (SeedData_t *)init->func_data;
	uint32_t state;
	
	// Check if seed was explicitly provided (arg_values not available during _init)
	if ( args->arg_count == 1 && args->arg_values && args->arg_values[0] )
	{
		// Seed was provided - use it deterministically for all rows
		if ( pData->seed == 0 )
		{
			// First call with seed - initialize it
			if ( args->arg_types[0] == SPH_UDF_TYPE_UINT32 )
				pData->seed = *(uint32_t*)args->arg_values[0];
			else if ( args->arg_types[0] == SPH_UDF_TYPE_INT64 )
				pData->seed = (uint32_t)(*(sphinx_int64_t*)args->arg_values[0]);
			else
				pData->seed = (uint32_t)time(NULL);
			// Ensure seed is non-zero (0 is our "not initialized" marker)
			if ( pData->seed == 0 )
				pData->seed = 1;
		}
		// Use the stored seed for deterministic results
		state = pData->seed;
	}
	else
	{
		// No seed provided - generate new random seed for each row
		pData->counter++;
		state = (uint32_t)time(NULL) ^ (uint32_t)((uintptr_t)init->func_data) ^ pData->counter;
	}
	
	// Generate random array length from 1 to 20
	int count = (lcg_next(&state) % 20) + 1;
	
	// Allocate memory for random number of int64 values using the provided malloc function
	int64_t * pValues = (int64_t *) args->fn_malloc ( count * sizeof(int64_t) );
	if ( !pValues )
	{
		*error_flag = 1;
		return (ByteBlob_t){nullptr, 0};
	}
	
	// Fill with random values
	for ( int i = 0; i < count; i++ )
		pValues[i] = (int64_t)(lcg_next(&state) % 10000) + 1; // Random values from 1 to 10000
	
	// Sort values (required for any() function to work with binary search)
	// Simple bubble sort for small arrays (typically 1-20 elements)
	for ( int i = 0; i < count - 1; i++ )
	{
		for ( int j = 0; j < count - i - 1; j++ )
		{
			if ( pValues[j] > pValues[j+1] )
			{
				int64_t temp = pValues[j];
				pValues[j] = pValues[j+1];
				pValues[j+1] = temp;
			}
		}
	}
	
	// Return as ByteBlob_t
	return (ByteBlob_t){(const BYTE*)pValues, count * sizeof(int64_t)};
}

//////////////////////////////////////////////////////////////////////////
// Float vector return example - returns a random float vector with 128 values
// Optional seed parameter for deterministic results

DLLEXPORT int makefloatvec_init ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_message )
{
	if ( args->arg_count > 1 )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "MAKEFLOATVEC() takes 0 or 1 argument (optional seed)" );
		return 1;
	}
	
	// Allocate storage for seed value and call counter
	SeedData_t * pData = (SeedData_t *) malloc ( sizeof(SeedData_t) );
	if ( !pData )
	{
		snprintf ( error_message, SPH_UDF_ERROR_LEN, "malloc() failed" );
		return 1;
	}
	
	// Initialize: seed=0 means not set, counter starts at 0
	pData->seed = 0;
	pData->counter = 0;
	
	init->func_data = pData;
	return 0;
}

DLLEXPORT void makefloatvec_deinit ( SPH_UDF_INIT * init )
{
	if ( init->func_data )
	{
		free ( init->func_data );
		init->func_data = NULL;
	}
}

DLLEXPORT ByteBlob_t makefloatvec ( SPH_UDF_INIT * init, SPH_UDF_ARGS * args, char * error_flag )
{
	SeedData_t * pData = (SeedData_t *)init->func_data;
	uint32_t state;
	
	// Check if seed was explicitly provided (arg_values not available during _init)
	if ( args->arg_count == 1 && args->arg_values && args->arg_values[0] )
	{
		// Seed was provided - use it deterministically for all rows
		if ( pData->seed == 0 )
		{
			// First call with seed - initialize it
			if ( args->arg_types[0] == SPH_UDF_TYPE_UINT32 )
				pData->seed = *(uint32_t*)args->arg_values[0];
			else if ( args->arg_types[0] == SPH_UDF_TYPE_INT64 )
				pData->seed = (uint32_t)(*(sphinx_int64_t*)args->arg_values[0]);
			else
				pData->seed = (uint32_t)time(NULL);
			// Ensure seed is non-zero (0 is our "not initialized" marker)
			if ( pData->seed == 0 )
				pData->seed = 1;
		}
		// Use the stored seed for deterministic results
		state = pData->seed;
	}
	else
	{
		// No seed provided - generate new random seed for each row
		pData->counter++;
		state = (uint32_t)time(NULL) ^ (uint32_t)((uintptr_t)init->func_data) ^ pData->counter;
	}
	
	// Constant array length of 128 for float vector
	const int count = 128;
	
	// Allocate memory for 128 float values using the provided malloc function
	float * pValues = (float *) args->fn_malloc ( count * sizeof(float) );
	if ( !pValues )
	{
		*error_flag = 1;
		return (ByteBlob_t){nullptr, 0};
	}
	
	// Fill with random values
	for ( int i = 0; i < count; i++ )
		pValues[i] = (float)(lcg_next(&state) % 1000) / 10.0f; // Random values from 0.0 to 99.9
	
	// Return as ByteBlob_t
	return (ByteBlob_t){(const BYTE*)pValues, count * sizeof(float)};
}

// FIXME! add a string function example?
// FIXME! add a ranker plugin example?

