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
// Sphinx plugin interface header
//
// This file will be included by plugin implementations, so it should be
// portable plain C, stay standalone, and change as rarely as possible.
//

#ifndef _sphinxudf_
#define _sphinxudf_

#ifdef __cplusplus
extern "C" {
#endif

/// current udf version
#define SPH_UDF_VERSION 6

/// error buffer size
#define SPH_UDF_ERROR_LEN 256

//////////////////////////////////////////////////////////////////////////
// UDF PLUGINS
//////////////////////////////////////////////////////////////////////////

/// UDF (User Defined Function) plugins let you add functions to use
/// in the SELECT expressions, like this:
///
/// SELECT id, attr1, myudf(attr2, attr3+attr4) ...
///
/// They can be loaded and unloaded dynamically using CREATE FUNCTION
/// and DROP FUNCTION statements, respectively.
///
/// The workflow is as follows:
///
/// 1) XXX_init() gets called once per query, in the very beginning.
/// It receives a list of argument types (and optional names, if any)
/// for validation. Init() can raise an error; the query will then
/// not be processed any further.
///
/// 2) XXX() gets called every time when Sphinx needs to actually
/// compute an UDF value.
///
/// 3) XXX_deinit() gets called once per query, in the very end.
///
/// Depending on the query, UDFs can be called very frequently or very
/// rarely. Indeed, when the UDF result is used for filtering or sorting,
/// Sphinx computes it for every single matched document, as it should.
/// However, when the UDF result is only needed in the final result set,
/// Sphinx can postpone that computation.
///
/// The possible uses are pretty wide: implementing fast custom math
/// calculations; or complex ranking functions; or fetching documents
/// contents from the database, etc.
///
/// UDFs can return integers, floats, or strings. In the latter case,
/// the return values need to be C strings, and they MUST be allocated
/// using the provided SPH_UDF_ARGS::fn_malloc() function.
///
/// To access PACKEDFACTORS() arguments from within the UDF, you will
/// need to use one of the two provided helper function sets, see below.
/// Basically, sphinx_factors_YYY() functions are slow, but easier to use;
/// sphinx_get_ZZZ_factor() functions are the faster streamlined interface.


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


//////////////////////////////////////////////////////////////////////////
// RANKER PLUGINS
//////////////////////////////////////////////////////////////////////////

/// Ranker plugins let you implement a custom ranker that receives
/// all the occurrences of the keywords matched in the document, and
/// computes a WEIGHT() value.
///
/// Custom rankers can be used like this:
/// SELECT id, attr1 FROM test WHERE match('hello') OPTION ranker=myranker('option1=1');
///
/// Rankers should be loaded before use with CREATE RANKER statement.
/// They can be unloaded using the DROP RANKER statement.
///
/// The workflow is as follows:
///
/// 1) XXX_init() gets called once per query in the very beginning.
/// A few query-wide options are passed to it, as well as the userdata
/// pointer, so that a stateful ranker plugin could store its state.
///
/// 2) XXX_update() gets called multiple times per matched document,
/// with every matched keyword occurrence passed as its parameter.
/// The occurrences are guaranteed to be in the "hitpos asc" order.
///
/// 3) XXX_finalize() gets called once per matched document, once there
/// are no more keyword occurrences. It must return the WEIGHT() value.
/// It is the only mandatory function for a custom ranker.
///
/// 4) XXX_deinit() gets called once per query in the very end.


/// a structure that represents a hit
typedef struct st_plugin_hit
{
	sphinx_uint64_t		docid;
	unsigned int		hitpos;
	unsigned short		querypos;
	unsigned short		nodepos;
	unsigned short		spanlen;
	unsigned short		matchlen;
	unsigned int		weight;
	unsigned int		qposmask;
} SPH_PLUGIN_HIT;


/// a structure that represents a match
typedef struct st_plugin_match
{
	sphinx_uint64_t		docid;
	int					weight;
} SPH_PLUGIN_MATCH;


// ranker info (used by user-defined rankers)
typedef struct st_plugin_rankerinfo
{
	unsigned int		payload_mask;
	int					qwords;
	int					max_qpos;
} SPH_PLUGIN_RANKERINFO;


#ifdef __cplusplus
}
#endif

#endif // _sphinxudf_

//
// $Id$
//
