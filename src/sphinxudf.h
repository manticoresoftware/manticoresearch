//
// $Id$
//

//
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

//
// Sphinx plugin interface header
//
// This file will be included by plugin implementations, so it should be
// portable plain C, stay standalone, and change as rarely as possible.
//
// Refer to src/udfexample.c for a working UDF example, and refer to
// doc/sphinx.html#extending-sphinx for more information on writing
// plugins and UDFs.
//

#ifndef _sphinxudf_
#define _sphinxudf_

#ifdef __cplusplus
extern "C" {
#endif

/// current udf version
#define SPH_UDF_VERSION 8

/// error buffer size
#define SPH_UDF_ERROR_LEN 256

//////////////////////////////////////////////////////////////////////////
// UDF PLUGINS
//////////////////////////////////////////////////////////////////////////

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

/// ranking factors interface, v1
/// functions that unpack PACKEDFACTORS() blob into a few helper C structures
/// slower because of malloc()s and copying, but easier to use

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

//////////////////////////////////////////////////////////////////////////

/// ranking factors interface, v2
/// functions that access factor values directly in the PACKEDFACTORS() blob
///
/// faster, as no memory allocations are guaranteed, but type-punned
/// meaning that you have to call a proper get_xxx_factor_int() or xxx_float() variant
/// the accessor functions themselves will NOT perform any type checking or conversions
/// or in other words, sphinx_get_field_factor_int() on a float factor like min_idf is legal,
/// but returns "garbage" (floating value from the blob reinterpreted as an integer)

enum sphinx_doc_factor
{
	SPH_DOCF_BM25					= 1,	///< int
	SPH_DOCF_BM25A					= 2,	///< float
	SPH_DOCF_MATCHED_FIELDS			= 3,	///< unsigned int
	SPH_DOCF_DOC_WORD_COUNT			= 4,	///< int
	SPH_DOCF_NUM_FIELDS				= 5,	///< int
	SPH_DOCF_MAX_UNIQ_QPOS			= 6,	///< int
	SPH_DOCF_EXACT_HIT_MASK			= 7,	///< unsigned int
	SPH_DOCF_EXACT_ORDER_MASK		= 8		///< v.4, unsigned int
};

enum sphinx_field_factor
{
	SPH_FIELDF_HIT_COUNT			= 1,	///< unsigned int
	SPH_FIELDF_LCS					= 2,	///< unsigned int
	SPH_FIELDF_WORD_COUNT			= 3,	///< unsigned int
	SPH_FIELDF_TF_IDF				= 4,	///< float
	SPH_FIELDF_MIN_IDF				= 5,	///< float
	SPH_FIELDF_MAX_IDF				= 6,	///< float
	SPH_FIELDF_SUM_IDF				= 7,	///< float
	SPH_FIELDF_MIN_HIT_POS			= 8,	///< int
	SPH_FIELDF_MIN_BEST_SPAN_POS	= 9,	///< int
	SPH_FIELDF_MAX_WINDOW_HITS		= 10,	///< int
	SPH_FIELDF_MIN_GAPS				= 11,	///< v.3, int
	SPH_FIELDF_ATC					= 12,	///< v.4, float
	SPH_FIELDF_LCCS					= 13,	///< v.5, int
	SPH_FIELDF_WLCCS				= 14	///< v.5, float
};

enum sphinx_term_factor
{
	SPH_TERMF_KEYWORD_MASK			= 1,	///< unsigned int
	SPH_TERMF_TF					= 2,	///< int
	SPH_TERMF_IDF					= 3		///< float
};

/// returns a pointer to the field factors, or NULL for a non-matched field index
const unsigned int * sphinx_get_field_factors ( const unsigned int * in, int field );

/// returns a pointer to the term factors, or NULL for a non-matched field index
const unsigned int * sphinx_get_term_factors ( const unsigned int * in, int term );

/// returns a document factor value, interpreted as integer
int sphinx_get_doc_factor_int ( const unsigned int * in, enum sphinx_doc_factor f );

/// returns a document factor value, interpreted as float
float sphinx_get_doc_factor_float ( const unsigned int * in, enum sphinx_doc_factor f );

/// returns a field factor value, interpreted as integer
int sphinx_get_field_factor_int ( const unsigned int * in, enum sphinx_field_factor f );

/// returns a field factor value, interpreted as float
float sphinx_get_field_factor_float ( const unsigned int * in, enum sphinx_field_factor f );

/// returns a term factor value, interpreted as integer
int sphinx_get_term_factor_int ( const unsigned int * in, enum sphinx_term_factor f );

/// returns a term factor value, interpreted as float
float sphinx_get_term_factor_float ( const unsigned int * in, enum sphinx_term_factor f );

/// returns a pointer to document factor value, interpreted as vector of integers
const unsigned int * sphinx_get_doc_factor_ptr ( const unsigned int * in, enum sphinx_doc_factor f );

//////////////////////////////////////////////////////////////////////////
// RANKER PLUGINS
//////////////////////////////////////////////////////////////////////////

/// ranker plugin intialization info
typedef struct st_plugin_rankerinfo
{
	int					num_field_weights;
	int *				field_weights;
	const char *		options;
	unsigned int		payload_mask;
	int					num_query_words;
	int					max_qpos;
} SPH_RANKER_INIT;

/// a structure that represents a hit
typedef struct st_plugin_hit
{
	sphinx_uint64_t		doc_id;
	unsigned int		hit_pos;
	unsigned short		query_pos;
	unsigned short		node_pos;
	unsigned short		span_length;
	unsigned short		match_length;
	unsigned int		weight;
	unsigned int		query_pos_mask;
} SPH_RANKER_HIT;

#ifdef __cplusplus
}
#endif

#endif // _sphinxudf_

//
// $Id$
//
