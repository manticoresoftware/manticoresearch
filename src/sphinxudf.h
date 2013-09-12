//
// $Id$
//

//
// Copyright (c) 2011-2013, Andrew Aksyonoff
// Copyright (c) 2011-2013, Sphinx Technologies Inc
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

/// current udf version
#define SPH_UDF_VERSION 5

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
	SPH_UDF_TYPE_UINT64SET		= 6,			///< sorted set of unsigned 64-bit integers
	SPH_UDF_TYPE_FACTORS		= 7				///< packed ranking factors
};

/// our malloc() replacement type
/// results that are returned to searchd MUST be allocated using this replacement
typedef void * sphinx_malloc_fn ( int );

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

//////////////////////////////////////////////////////////////////////////

/// unpacked representation of all the field-level ranking factors
typedef struct st_sphinx_field_factors
{
	unsigned int	hit_count;
	unsigned int	id;
	unsigned int	lcs;
	unsigned int	word_count;
	float			tf_idf;
	float			min_idf;
	float			max_idf;
	float			sum_idf;
	int				min_hit_pos;
	int				min_best_span_pos;
	char			exact_hit;
	int				max_window_hits;
	int				min_gaps;			///< added in v.3
	char			exact_order;		///< added in v.4
	float			atc;				///< added in v.4
	int				lccs;				///< added in v.5
	float			wlccs;				///< added in v.5
} SPH_UDF_FIELD_FACTORS;

/// unpacked representation of all the term-level ranking factors
typedef struct st_sphinx_term_factors
{
	unsigned int	keyword_mask;
	unsigned int	id;
	int				tf;
	float			idf;
} SPH_UDF_TERM_FACTORS;

/// unpacked representation of all the ranking factors (document, field, and term-level)
typedef struct st_sphinx_factors
{
	int					doc_bm25;
	float				doc_bm25a;
	unsigned int		matched_fields;
	int					doc_word_count;
	int					num_fields;
	int					max_uniq_qpos;

	SPH_UDF_FIELD_FACTORS *	field;
	SPH_UDF_TERM_FACTORS *	term;
	int *					field_tf;
} SPH_UDF_FACTORS;


/// helper function that must be called to initialize the SPH_UDF_FACTORS structure
/// before it is passed to sphinx_factors_unpack
/// returns 0 on success
/// returns an error code on error
int sphinx_factors_init ( SPH_UDF_FACTORS * out );

/// helper function that unpacks PACKEDFACTORS() blob into SPH_UDF_FACTORS structure
/// MUST be in sync with PackFactors() method in sphinxsearch.cpp
/// returns 0 on success
/// returns an error code on error
int sphinx_factors_unpack ( const unsigned int * in, SPH_UDF_FACTORS * out );

/// helper function that must be called to free the memory allocated by the sphinx_factors_unpack
/// function call
/// returns 0 on success
/// returns an error code on error
int sphinx_factors_deinit ( SPH_UDF_FACTORS * out );

#endif // _sphinxudf_

//
// $Id$
//
