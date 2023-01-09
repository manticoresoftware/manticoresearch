//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

// elementary typedefs, enums and inline funcs, project-wide.
// Note! No specific non-standard types here, so inclusion of a header should not make circular dependencies

#pragma once

#include "config.h"
#include "sphinxstd.h"

using RowID_t = DWORD;
const RowID_t INVALID_ROWID = 0xFFFFFFFF;
using DocID_t = int64_t;

/// row entry (storage only, does not necessarily map 1:1 to attributes)
using CSphRowitem = DWORD;

#define DOCID_MIN        (INT64_MIN)
#define WORDID_MAX        U64C(0xffffffffffffffff)

STATIC_SIZE_ASSERT ( DocID_t, 8 );
STATIC_SIZE_ASSERT ( RowID_t, 4 );

#define DWSIZEOF(a) ( sizeof(a) / sizeof(DWORD) )

//////////////////////////////////////////////////////////////////////////

typedef const BYTE *	CSphRowitemPtr;

/// widest integer type that can be be stored as an attribute (ideally, fully decoupled from rowitem size!)
typedef int64_t			SphAttr_t;

const CSphRowitem		ROWITEM_MAX		= UINT_MAX;
const int				ROWITEM_BITS	= 8*sizeof(CSphRowitem);
const int				ROWITEMPTR_BITS	= 8*sizeof(CSphRowitemPtr);
const int				ROWITEM_SHIFT	= 5;

STATIC_ASSERT ( ( 1 << ROWITEM_SHIFT )==ROWITEM_BITS, INVALID_ROWITEM_SHIFT );

#ifndef USE_LITTLE_ENDIAN
#error Please define endianness
#endif

/////////////////////////////////////////////////////////////////////////////

#define SPH_MAX_WORD_LEN		42		// so that any UTF-8 word fits 127 bytes
#define SPH_MAX_FILENAME_LEN	512
#define SPH_MAX_FIELDS			256

const int MAX_KEYWORD_BYTES = SPH_MAX_WORD_LEN*3+4;

/////////////////////////////////////////////////////////////////////////////
int sphUTF8Len ( const char * pStr );

/// check for valid attribute name char
inline int sphIsAttr ( int c )
{
	// different from sphIsAlpha() in that we don't allow minus
	return ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_';
}

class CSphDict;
using DictRefPtr_c = CSphRefcountedPtr<CSphDict>;

/////////////////////////////////////////////////////////////////////////////
// DATASOURCES
/////////////////////////////////////////////////////////////////////////////

typedef uint64_t SphWordID_t;
STATIC_SIZE_ASSERT ( SphWordID_t, 8 );

/// hit position storage type
typedef DWORD Hitpos_t;

/// empty hit value
#define EMPTY_HIT 0

/// hit info
struct CSphWordHit
{
	RowID_t		m_tRowID;		///< document ID
	SphWordID_t	m_uWordID = WORDID_MAX;		///< word ID in current dictionary
	Hitpos_t	m_uWordPos = EMPTY_HIT;		///< word position in current document
	CSphWordHit () = default;
	CSphWordHit ( RowID_t tRowID, SphWordID_t uWordID, Hitpos_t uWordPos )
		: m_tRowID { tRowID }
		, m_uWordID { uWordID }
		, m_uWordPos { uWordPos }
	{}
};

inline bool operator== ( const CSphWordHit& lhs, const CSphWordHit& rhs )
{
	return lhs.m_tRowID==rhs.m_tRowID && lhs.m_uWordID==rhs.m_uWordID && lhs.m_uWordPos == rhs.m_uWordPos;
}

/// hit vector interface
/// because specific position type might vary (dword, qword, etc)
/// but we don't want to template and instantiate everything because of that
using ISphHits = CSphVector<CSphWordHit>;

/// field filter
class ISphFieldFilter;

class ISphTokenizer;
using TokenizerRefPtr_c = CSphRefcountedPtr<ISphTokenizer>;

/////////////////////////////////////////////////////////////////////////////
// SEARCH QUERIES
/////////////////////////////////////////////////////////////////////////////

/// search query sorting orders
enum ESphSortOrder
{
	SPH_SORT_RELEVANCE		= 0,	///< sort by document relevance desc, then by date
	SPH_SORT_ATTR_DESC		= 1,	///< sort by document date desc, then by relevance desc
	SPH_SORT_ATTR_ASC		= 2,	///< sort by document date asc, then by relevance desc
	SPH_SORT_TIME_SEGMENTS	= 3,	///< sort by time segments (hour/day/week/etc) desc, then by relevance desc
	SPH_SORT_EXTENDED		= 4,	///< sort by SQL-like expression (eg. "@relevance DESC, price ASC, @id DESC")
	SPH_SORT_EXPR			= 5,	///< sort by arithmetic expression in descending order (eg. "@id + max(@weight,1000)*boost + log(price)")

	SPH_SORT_TOTAL
};


/// search query matching mode
enum ESphMatchMode
{
	SPH_MATCH_ALL = 0,			///< match all query words
	SPH_MATCH_ANY,				///< match any query word
	SPH_MATCH_PHRASE,			///< match this exact phrase
	SPH_MATCH_BOOLEAN,			///< match this boolean query
	SPH_MATCH_EXTENDED,			///< match this extended query
	SPH_MATCH_FULLSCAN,			///< match all document IDs w/o fulltext query, apply filters
	SPH_MATCH_EXTENDED2,		///< extended engine V2 (TEMPORARY, WILL BE REMOVED IN 0.9.8-RELEASE)

	SPH_MATCH_TOTAL
};


/// search query relevance ranking mode
enum ESphRankMode
{
	SPH_RANK_PROXIMITY_BM25		= 0,	///< default mode, phrase proximity major factor and BM25 minor one (aka SPH03)
	SPH_RANK_BM25				= 1,	///< statistical mode, BM25 ranking only (faster but worse quality)
	SPH_RANK_NONE				= 2,	///< no ranking, all matches get a weight of 1
	SPH_RANK_WORDCOUNT			= 3,	///< simple word-count weighting, rank is a weighted sum of per-field keyword occurence counts
	SPH_RANK_PROXIMITY			= 4,	///< phrase proximity (aka SPH01)
	SPH_RANK_MATCHANY			= 5,	///< emulate old match-any weighting (aka SPH02)
	SPH_RANK_FIELDMASK			= 6,	///< sets bits where there were matches
	SPH_RANK_SPH04				= 7,	///< codename SPH04, phrase proximity + bm25 + head/exact boost
	SPH_RANK_EXPR				= 8,	///< rank by user expression (eg. "sum(lcs*user_weight)*1000+bm25")
	SPH_RANK_EXPORT				= 9,	///< rank by BM25, but compute and export all user expression factors
	SPH_RANK_PLUGIN				= 10,	///< user-defined ranker

	SPH_RANK_TOTAL,
	SPH_RANK_DEFAULT			= SPH_RANK_PROXIMITY_BM25
};

/// known multi-valued attr sources
enum ESphAttrSrc {
	SPH_ATTRSRC_NONE = 0,			///< not multi-valued
	SPH_ATTRSRC_FIELD = 1,			///< get attr values from text field
	SPH_ATTRSRC_QUERY = 2,			///< get attr values from SQL query
	SPH_ATTRSRC_RANGEDQUERY = 3,	///< get attr values from ranged SQL query
	SPH_ATTRSRC_RANGEDMAINQUERY = 4 ///< get attr values from main ranged SQL query, used for MVA query
};


/// aggregate function to apply
enum ESphAggrFunc {
	SPH_AGGR_NONE,
	SPH_AGGR_AVG,
	SPH_AGGR_MIN,
	SPH_AGGR_MAX,
	SPH_AGGR_SUM,
	SPH_AGGR_CAT
};


/// search query grouping mode
enum ESphGroupBy
{
	SPH_GROUPBY_DAY		= 0,	///< group by day
	SPH_GROUPBY_WEEK	= 1,	///< group by week
	SPH_GROUPBY_MONTH	= 2,	///< group by month
	SPH_GROUPBY_YEAR	= 3,	///< group by year
	SPH_GROUPBY_ATTR	= 4,	///< group by attribute value
	SPH_GROUPBY_ATTRPAIR= 5,	///< group by sequential attrs pair (rendered redundant by 64bit attrs support; removed)
	SPH_GROUPBY_MULTIPLE= 6		///< group by on multiple attribute values
};


/// search query filter types
enum ESphFilter
{
	SPH_FILTER_VALUES		= 0,	///< filter by integer values set
	SPH_FILTER_RANGE		= 1,	///< filter by integer range
	SPH_FILTER_FLOATRANGE	= 2,	///< filter by float range
	SPH_FILTER_STRING		= 3,	///< filter by string value
	SPH_FILTER_NULL			= 4,	///< filter by NULL
	SPH_FILTER_USERVAR		= 5,	///< filter by @uservar
	SPH_FILTER_STRING_LIST	= 6,	///< filter by string list
	SPH_FILTER_EXPRESSION	= 7		///< filter by expression
};


/// MVA folding function
/// (currently used in filters, eg WHERE ALL(mymva) BETWEEN 1 AND 3)
enum  ESphMvaFunc
{
	SPH_MVAFUNC_NONE = 0,
	SPH_MVAFUNC_ANY,
	SPH_MVAFUNC_ALL
};

enum QueryType_e
{
	QUERY_API,
	QUERY_SQL,
	QUERY_JSON
};

enum QueryOption_e
{
	QUERY_OPT_DEFAULT = 0,
	QUERY_OPT_DISABLED,
	QUERY_OPT_ENABLED,
	QUERY_OPT_MORPH_NONE
};

/// match sorting functions
enum ESphSortFunc
{
	FUNC_REL_DESC,
	FUNC_ATTR_DESC,
	FUNC_ATTR_ASC,
	FUNC_TIMESEGS,
	FUNC_GENERIC1,
	FUNC_GENERIC2,
	FUNC_GENERIC3,
	FUNC_GENERIC4,
	FUNC_GENERIC5,
	FUNC_EXPR
};


/// match sorting clause parsing outcomes
enum ESortClauseParseResult
{
	SORT_CLAUSE_OK,
	SORT_CLAUSE_ERROR,
	SORT_CLAUSE_RANDOM
};


/// sorting key part types
enum ESphSortKeyPart
{
	SPH_KEYPART_ROWID,
	SPH_KEYPART_WEIGHT,
	SPH_KEYPART_INT,
	SPH_KEYPART_FLOAT,
	SPH_KEYPART_DOUBLE,
	SPH_KEYPART_STRING,
	SPH_KEYPART_STRINGPTR
};

enum KeywordExpansion_e
{
	KWE_DISABLED	=	0,
	KWE_EXACT	=		( 1UL << 0 ),
	KWE_STAR	=		( 1UL << 1 ),
	KWE_MORPH_NONE	=	( 1UL << 2 ),

	KWE_ENABLED = ( KWE_EXACT | KWE_STAR ),
};

enum TypeConversion_e
{
	CONVERSION_NONE,
	CONVERSION_BIGINT2FLOAT,
	CONVERSION_FLOAT2BIGINT,
};

enum DocstoreDataType_e
{
	DOCSTORE_TEXT,
	DOCSTORE_BIN,
	DOCSTORE_ATTR,
	DOCSTORE_TOTAL
};

/// wordpart processing type
enum ESphWordpart
{
	SPH_WORDPART_WHOLE		= 0,	///< whole-word
	SPH_WORDPART_PREFIX		= 1,	///< prefix
	SPH_WORDPART_INFIX		= 2		///< infix
};

enum class AttrEngine_e
{
	DEFAULT,
	ROWWISE,
	COLUMNAR
};

using Bson_t = CSphVector<BYTE>;

#if UNALIGNED_RAM_ACCESS

/// pass-through wrapper
template<typename T>
inline T sphUnalignedRead ( const T& tRef )
{
	return tRef;
}

/// pass-through wrapper
template<typename T>
void sphUnalignedWrite ( void* pPtr, const T& tVal )
{
	*(T*)pPtr = tVal;
}

#else

/// unaligned read wrapper for some architectures (eg. SPARC)
template<typename T>
inline T sphUnalignedRead ( const T& tRef )
{
	T uTmp;
	BYTE* pSrc = (BYTE*)&tRef;
	BYTE* pDst = (BYTE*)&uTmp;
	for ( int i = 0; i < (int)sizeof ( T ); i++ )
		*pDst++ = *pSrc++;
	return uTmp;
}

/// unaligned write wrapper for some architectures (eg. SPARC)
template<typename T>
void sphUnalignedWrite ( void* pPtr, const T& tVal )
{
	BYTE* pDst = (BYTE*)pPtr;
	BYTE* pSrc = (BYTE*)&tVal;
	for ( int i = 0; i < (int)sizeof ( T ); i++ )
		*pDst++ = *pSrc++;
}

#endif // unalgined


#if UNALIGNED_RAM_ACCESS && USE_LITTLE_ENDIAN
/// get a dword from memory, intel version
inline DWORD sphGetDword ( const BYTE* p )
{
	return *(const DWORD*)p;
}
#else
/// get a dword from memory, non-intel version
inline DWORD sphGetDword ( const BYTE* p )
{
	return p[0] + ( p[1] << 8 ) + ( p[2] << 16 ) + ( p[3] << 24 );
}
#endif
