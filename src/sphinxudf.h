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
	SPH_UDF_TYPE_UINT64SET		= 6,			///< sorted set of unsigned 64-bit integers
	SPH_UDF_TYPE_FACTORS		= 7				///< packed ranking factors
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
	unsigned int	exact_hit;
	int				max_window_hits;
} SPH_UDF_FIELD_FACTORS;

/// unpacked representation of all the term-level ranking factors
typedef struct st_sphinx_term_factors
{
	unsigned int	keyword_mask;
	unsigned int	id;
	int				tf;
	float			idf;
} SPH_UDF_TERM_FACTORS;

#define SPH_UDF_MAX_FIELD_FACTORS	256
#define SPH_UDF_MAX_TERM_FACTORS	2048

/// unpacked representation of all the ranking factors (document, field, and term-level)
typedef struct st_sphinx_factors
{
	int					doc_bm25;
	float				doc_bm25a;
	unsigned int		matched_fields;
	int					doc_word_count;
	int					num_fields;
	int					max_uniq_qpos;

	SPH_UDF_FIELD_FACTORS	field [ SPH_UDF_MAX_FIELD_FACTORS ];
	SPH_UDF_TERM_FACTORS	term [ SPH_UDF_MAX_TERM_FACTORS ];
} SPH_UDF_FACTORS;

/// helper function that unpacks PACKEDFACTORS() blob into SPH_UDF_FACTORS structure
/// MUST be in sync with PackFactors() method in sphinxsearch.cpp
/// returns 0 on success
/// returns an error code on error
int sphinx_unpack_factors ( const unsigned int * in, SPH_UDF_FACTORS * out )
{
	const unsigned int * pack = in;
	SPH_UDF_FIELD_FACTORS * f;
	SPH_UDF_TERM_FACTORS * t;
	int i, size;

	if ( !in )
		return 1;

	// extract size, extract document-level factors
	size = *in++;
	out->doc_bm25 = *in++;
	out->doc_bm25a = (float)*in++;
	out->matched_fields = *in++;
	out->doc_word_count = *in++;
	out->num_fields = *in++;

	// extract field-level factors
	if ( out->num_fields > SPH_UDF_MAX_FIELD_FACTORS )
		return 1;
	for ( i=0; i<out->num_fields; i++ )
	{
		out->field[i].hit_count = *in++;
		if ( !out->field[i].hit_count )
			continue;

		f = &(out->field[i]);
		f->id = *in++;
		f->lcs = *in++;
		f->word_count = *in++;
		f->tf_idf = (float)*in++;
		f->min_idf = (float)*in++;
		f->max_idf = (float)*in++;
		f->sum_idf = (float)*in++;
		f->min_hit_pos = (int)*in++;
		f->min_best_span_pos = (int)*in++;
		f->exact_hit = *in++;
		f->max_window_hits = (int)*in++;
	}

	// extract term-level factors
	out->max_uniq_qpos = *in++;
	if ( out->max_uniq_qpos > SPH_UDF_MAX_TERM_FACTORS )
		return 1;
	for ( i=0; i<out->max_uniq_qpos; i++ )
	{
		t = &(out->term[i]);
		t->keyword_mask = *in++;
		if ( t->keyword_mask )
		{
			t->id = *in++;
			t->tf = (int)*in++;
			t->idf = (float)*in++;
		}
	}

	// do a safety check, and return
	return ( size!=(in-pack)*sizeof(unsigned int) ) ? 1 : 0;
}

#endif // _sphinxudf_

//
// $Id$
//
