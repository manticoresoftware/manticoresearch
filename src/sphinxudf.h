//
// $Id$
//

//
// Copyright (c) 2011, Andrew Aksyonoff
// Copyright (c) 2011, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

//
// Sphinx UDF interface header
//
// This file will be included by UDF implementations, so it should be
// portable plain C, stay standalone, and change as rarely as possible.
//

#ifndef _sphinxudf_
#define _sphinxudf_

/// error buffer size
#define SPH_UDF_ERROR_LEN 256

/// UDF argument and result value types
enum sphinx_udf_argtype
{
	SPH_UDF_TYPE_UINT32			= 1,			///< unsigned 32-bit integer
	SPH_UDF_TYPE_UINT32SET		= 2,			///< sorted set of unsigned 32-bit integers
	SPH_UDF_TYPE_INT64			= 3,			///< signed 64-bit integer
	SPH_UDF_TYPE_FLOAT			= 4,			///< single-precision IEEE 754 float
	SPH_UDF_TYPE_STRING			= 5,			///< non-ASCIIZ string, with a separately stored length
	SPH_UDF_TYPE_UINT64SET		= 6				///< sorted set of unsigned 64-bit integers
};

/// our malloc() replacement type
/// results that are returned to searchd MUST be allocated using this replacement
typedef void * sphinx_malloc_fn(int);

/// UDF call arguments
typedef struct st_sphinx_udf_args
{
	int							arg_count;		///< number of arguments
	enum sphinx_udf_argtype *	arg_types;		///< argument types
	char **						arg_values;		///< argument values (strings are not (!) ASCIIZ; see str_lengths below)
	char **						arg_names;		///< argument names (ASCIIZ argname in 'expr AS argname' case; NULL otherwise)
	int *						str_lengths;	///< string argument lengths
	sphinx_malloc_fn *			fn_malloc;		///< malloc() replacement to allocate returned values
} SPH_UDF_ARGS;

/// UDF initialization
typedef struct st_sphinx_udf_init
{
	void *						func_data;		///< function data (will be passed to calls, deinit)
	char						is_const;		///< whether a function returns a constant
} SPH_UDF_INIT;

/// integer return types
#if defined(_MSC_VER) || defined(__WIN__)
typedef __int64					sphinx_int64_t;
typedef unsigned __int64		sphinx_uint64_t;
#else
typedef long long				sphinx_int64_t;
typedef unsigned long long		sphinx_uint64_t;
#endif

#endif // _sphinxudf_

//
// $Id$
//
