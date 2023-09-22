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

#ifndef _sphinx_
#define _sphinx_

/////////////////////////////////////////////////////////////////////////////

#include "sphinxstd.h"
#include "indexsettings.h"
#include "fileutils.h"
#include "collation.h"
#include "binlog_defs.h"
#include "task_dispatcher.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#if _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#if _WIN32
#define STDOUT_FILENO		fileno(stdout)
#define STDERR_FILENO		fileno(stderr)
#endif

#include "sphinxdefs.h"
#include "schema/locator.h"
#include "schema/schema.h"
#include "indexfilebase.h"

/////////////////////////////////////////////////////////////////////////////

// defined in sphinxversion.cpp in order to isolate from total rebuild on minor changes
extern const char * szMANTICORE_VERSION;
extern const char * szMANTICORE_NAME;
extern const char * szMANTICORE_BANNER;
extern const char * szMANTICORE_BANNER_TEXT;
extern const char * szGIT_COMMIT_ID;
extern const char * szGIT_BRANCH_ID;
extern const char * szGDB_SOURCE_DIR;

#define SPHINX_SEARCHD_PROTO	1
#define SPHINX_CLIENT_VERSION	1

/////////////////////////////////////////////////////////////////////////////

extern int64_t g_iIndexerCurrentDocID;
extern int64_t g_iIndexerCurrentHits;
extern int64_t g_iIndexerCurrentRangeMin;
extern int64_t g_iIndexerCurrentRangeMax;
extern int64_t g_iIndexerPoolStartDocID;
extern int64_t g_iIndexerPoolStartHit;

/////////////////////////////////////////////////////////////////////////////

/// millisecond-precision sleep
void			sphSleepMsec ( int iMsec );

/// immediately interrupt current query
void			sphInterruptNow();

/// check if we got interrupted
bool			sphInterrupted();

//////////////////////////////////////////////////////////////////////////

struct CSphMultiformContainer;
class CSphWriter;


/// wordforms container
struct CSphWordforms;

// converts stopword/wordform/exception file paths for configless mode
class FilenameBuilder_i
{
public:
	virtual				~FilenameBuilder_i() {}

	virtual CSphString	GetFullPath ( const CSphString & sName ) const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// DATASOURCES
/////////////////////////////////////////////////////////////////////////////

/// hit processing tools
/// Hitpos_t consists of three things:
/// 1) high bits store field number
/// 2) middle bit - field end marker
/// 3) lower bits store hit position in field
template < int FIELD_BITS >
class Hitman_c
{
protected:
	enum
	{
		FIELD_OFF		= 32 - FIELD_BITS,			// 24
		POS_BITS		= FIELD_OFF - 1,			// 23
		FIELDEND_OFF	= POS_BITS,					// 23
		FIELDEND_MASK	= (1UL << POS_BITS),		// 0x00800000
		POS_MASK		= FIELDEND_MASK - 1,		// 0x007FFFFF
		FIELD_MASK		= ~(FIELDEND_MASK|POS_MASK),// 0xFF000000
	};

public:
	static Hitpos_t Create ( int iField, int iPos )
	{
		return ( iField << FIELD_OFF ) + ( iPos & POS_MASK );
	}

	static Hitpos_t Create ( int iField, int iPos, bool bEnd )
	{
		return ( iField << FIELD_OFF ) + ( ((int)bEnd) << FIELDEND_OFF ) + ( iPos & POS_MASK );
	}

	static inline int GetField ( Hitpos_t uHitpos )
	{
		return uHitpos >> FIELD_OFF;
	}

	static inline void DecrementField ( Hitpos_t& uHitpos )
	{
		assert ( uHitpos & FIELD_MASK );
		uHitpos -= (1UL << FIELD_OFF);
	}

	static inline int GetPos ( Hitpos_t uHitpos )
	{
		return uHitpos & POS_MASK;
	}

	static inline bool IsEnd ( Hitpos_t uHitpos )
	{
		return ( uHitpos & FIELDEND_MASK )!=0;
	}

	static inline DWORD GetPosWithField ( Hitpos_t uHitpos )
	{
		return uHitpos & ~FIELDEND_MASK;
	}

	static void AddPos ( Hitpos_t * pHitpos, int iAdd )
	{
		// FIXME! add range checks (eg. so that 0:0-1 does not overflow)
		*pHitpos += iAdd;
	}

	static Hitpos_t CreateSum ( Hitpos_t uHitpos, int iAdd )
	{
		// FIXME! add range checks (eg. so that 0:0-1 does not overflow)
		return ( uHitpos+iAdd ) & ~FIELDEND_MASK;
	}

	static void SetEndMarker ( Hitpos_t * pHitpos )
	{
		*pHitpos |= FIELDEND_MASK;
	}
};

// this could be just DWORD[] but it's methods are very handy
// used to store field information e.g. which fields do we need to search in
struct FieldMask_t
{
	static const int SIZE = SPH_MAX_FIELDS/32;
	STATIC_ASSERT ( ( SPH_MAX_FIELDS%32 )==0, ASSUME_MAX_FIELDS_ARE_REPRESENTABLE_BY_DWORD );
	DWORD m_dMask [ SIZE ];

	// no custom cstr and d-tor - to be usable from inside unions
	// deep copy for it is ok - so, no explicit copying constructor and operator=

	// old-fashion layer to work with DWORD (32-bit) mask.
	// all bits above 32 assumed to be unset.
	void Assign32 ( DWORD uMask )
	{
		UnsetAll();
		m_dMask[0] = uMask;
	}

	DWORD GetMask32 () const
	{
		return m_dMask[0];
	}

	DWORD operator[] ( int iIdx ) const
	{
		assert ( 0<=iIdx && iIdx<SIZE );
		return m_dMask [ iIdx ];
	}

	DWORD & operator[] ( int iIdx )
	{
		assert ( 0<=iIdx && iIdx<SIZE );
		return m_dMask [ iIdx ];
	}

	// set n-th bit
	void Set ( int iIdx )
	{
		assert ( 0<=iIdx && iIdx<(int)sizeof(m_dMask)*8 );
		m_dMask [ iIdx/32 ] |= 1 << ( iIdx%32 );
	}

	// set all bits
	void SetAll()
	{
		memset ( m_dMask, 0xff, sizeof(m_dMask) );
	}

	// unset n-th bit, or all
	void Unset ( int iIdx )
	{
		assert ( 0<=iIdx && iIdx<(int)sizeof(m_dMask)*8 );
		m_dMask [ iIdx/32 ] &= ~(1 << ( iIdx%32 ));
	}

	void UnsetAll()
	{
		memset ( m_dMask, 0, sizeof(m_dMask) );
	}

	// test if n-th bit is set
	bool Test ( int iIdx ) const
	{
		assert ( iIdx>=0 && iIdx<(int)sizeof(m_dMask)*8 );
		return ( m_dMask [ iIdx/32 ] & ( 1 << ( iIdx%32 ) ) )!=0;
	}

	// test if all bits are set or unset
	bool TestAll ( bool bSet ) const
	{
		DWORD uTest = bSet ? 0xffffffff : 0;
		for ( auto uMask : m_dMask )
			if ( uMask!=uTest )
				return false;
		return true;
	}

	void Negate()
	{
		for ( auto& uMask : m_dMask )
			uMask = ~uMask;
	}

	// keep bits up to iIdx; shift bits over iIdx right by 1
	void DeleteBit ( int iIdx )
	{
		const auto iDwordIdx = iIdx / 32;
		const auto iDwordBitPos = iIdx % 32;

		DWORD uCarryBit = 0;
		for ( int i = SIZE-1; i>iDwordIdx; --i )
		{
			bool bNextCarry = m_dMask[i] & 1;
			m_dMask[i] = uCarryBit | ( m_dMask[i] >> 1 );
			uCarryBit = bNextCarry ? 0x80000000 : 0;
		}

		DWORD uShiftBit = 1 << ( iDwordBitPos );	// like: 00000000 00000000 00000100 00000000
		DWORD uKeepMask = uShiftBit-1;				// like: 00000000 00000000 00000011 11111111
		DWORD uMoveMask = ~(uShiftBit | uKeepMask);	// like: 11111111 11111111 11111000 00000000

		DWORD uKept = m_dMask[iDwordIdx] & uKeepMask;
		m_dMask[iDwordIdx] = uCarryBit | ( ( m_dMask[iDwordIdx] & uMoveMask ) >> 1 ) | uKept;
	}
};

struct RowTagged_t
{
	RowID_t m_tID { INVALID_ROWID };	///< document ID
	int m_iTag {0};						///< index tag
	
	RowTagged_t() = default;
	RowTagged_t ( const CSphMatch & tMatch );
	RowTagged_t ( RowID_t tRowID, int iTag );

	bool operator== ( const RowTagged_t & tRow ) const;
	bool operator!= ( const RowTagged_t & tRow ) const;
};

//////////////////////////////////////////////////////////////////////////

// defined in stripper/html_stripper.h
class CSphHTMLStripper;

/// field filter
class ISphFieldFilter
{
public:
	virtual						~ISphFieldFilter() = default;

	virtual	int					Apply ( const BYTE * sField, int iLength, CSphVector<BYTE> & dStorage, bool bQuery ) = 0;
	int							Apply ( const void* szField, CSphVector<BYTE>& dStorage, bool bQuery )
	{
		return Apply ( (const BYTE*)szField, (int) strlen ( (const char*)szField ), dStorage, bQuery );
	}

	int Apply ( ByteBlob_t sField, CSphVector<BYTE>& dStorage, bool bQuery )
	{
		return Apply ( sField.first, sField.second, dStorage, bQuery );
	}
	virtual	void				GetSettings ( CSphFieldFilterSettings & tSettings ) const = 0;
	virtual std::unique_ptr<ISphFieldFilter>	Clone() const = 0;
};

/// create a regexp field filter
std::unique_ptr<ISphFieldFilter> sphCreateRegexpFilter ( const CSphFieldFilterSettings & tFilterSettings, CSphString & sError );

/// create an ICU field filter
std::unique_ptr<ISphFieldFilter> sphCreateFilterICU ( std::unique_ptr<ISphFieldFilter> pParent, const char * szBlendChars, CSphString & sError );

/////////////////////////////////////////////////////////////////////////////
// SEARCH QUERIES
/////////////////////////////////////////////////////////////////////////////

/// search query filter
struct CommonFilterSettings_t
{
	ESphFilter			m_eType = SPH_FILTER_VALUES;		///< filter type
	union
	{
		SphAttr_t		m_iMinValue = LLONG_MIN;	///< range min
		float			m_fMinValue;	///< range min
	};
	union
	{
		SphAttr_t		m_iMaxValue = LLONG_MAX;	///< range max
		float			m_fMaxValue;	///< range max
	};

	bool				m_bHasEqualMin = true;	///< has filter "equal" component or pure greater/less (for min)
	bool				m_bHasEqualMax = true;	///< has filter "equal" component or pure greater/less (for max)
	bool				m_bOpenLeft = false;
	bool				m_bOpenRight = false;
	bool				m_bExclude = false;		///< whether this is "include" or "exclude" filter (default is "include")
};


class CSphFilterSettings : public CommonFilterSettings_t
{
public:
	CSphString			m_sAttrName = "";	///< filtered attribute name
	bool				m_bIsNull = false;		///< for NULL or NOT NULL
	bool				m_bOptional = false;

	ESphMvaFunc			m_eMvaFunc = SPH_MVAFUNC_NONE;		///< MVA and stringlist folding function
	CSphVector<SphAttr_t> m_dValues;	///< integer values set
	StrVec_t			m_dStrings;	///< string values

public:
						CSphFilterSettings () = default;

						// fixme! Dependency from external values implies, that CsphFilterSettings is NOT standalone,
						// and it's state is no way 'undependent'. It would be good to capture external values, at least
						// with ref-counted technique, exactly here, to locate all usecases near each other.
	void				SetExternalValues ( const VecTraits_T<SphAttr_t>& dValues );

	int					GetNumValues () const		{ return GetValues().GetLength(); }
	const VecTraits_T<SphAttr_t>& GetValues () const { return m_dExtValues.IsEmpty() ? m_dValues : m_dExtValues; }

	bool				operator == ( const CSphFilterSettings & rhs ) const;
	bool				operator != ( const CSphFilterSettings & rhs ) const { return !( (*this)==rhs ); }

	uint64_t			GetHash() const;

private:
	VecTraits_T<SphAttr_t> m_dExtValues;
};


// keyword info
struct CSphKeywordInfo
{
	CSphString		m_sTokenized;
	CSphString		m_sNormalized;
	int				m_iDocs = 0;
	int				m_iHits = 0;
	int				m_iQpos = 0;
};

inline void Swap ( CSphKeywordInfo & v1, CSphKeywordInfo & v2 )
{
	v1.m_sTokenized.Swap ( v2.m_sTokenized );
	v1.m_sNormalized.Swap ( v2.m_sNormalized );
	::Swap ( v1.m_iDocs, v2.m_iDocs );
	::Swap ( v1.m_iHits, v2.m_iHits );
	::Swap ( v1.m_iQpos, v2.m_iQpos );
}


/// query selection item
struct CSphQueryItem
{
	CSphString		m_sExpr;		///< expression to compute
	CSphString		m_sAlias;		///< alias to return
	ESphAggrFunc	m_eAggrFunc { SPH_AGGR_NONE };
};

/// search query complex filter tree
struct FilterTreeItem_t
{
	int m_iLeft = -1;		// left node at parser filter operations
	int m_iRight = -1;		// right node at parser filter operations
	int m_iFilterItem = -1;	// index into query filters 
	bool m_bOr = false;

	bool operator == ( const FilterTreeItem_t & rhs ) const;
	bool operator != ( const FilterTreeItem_t & rhs ) const { return !( (*this)==rhs ); }
	uint64_t GetHash() const;
};

/// table function interface
struct CSphQuery;
struct AggrResult_t;
class ISphTableFunc
{
public:
	virtual			~ISphTableFunc() {}
	virtual bool	ValidateArgs ( const StrVec_t & dArgs, const CSphQuery & tQuery, CSphString & sError ) = 0;
	virtual bool	Process ( AggrResult_t * pResult, CSphString & sError ) = 0;
	virtual bool	LimitPushdown ( int, int ) { return false; } // FIXME! implement this
};

class QueryParser_i;

enum class SecondaryIndexType_e
{
	NONE,
	FILTER,
	LOOKUP,
	INDEX,
	ANALYZER,

	TOTAL
};


struct IndexHint_t
{
	CSphString				m_sIndex;
	SecondaryIndexType_e	m_eType = SecondaryIndexType_e::NONE;
	bool					m_bForce = true;
};

const int DEFAULT_MAX_MATCHES = 1000;
const int DEFAULT_QUERY_TIMEOUT = 0;
const int DEFAULT_QUERY_RETRY = -1;
const int DEFAULT_QUERY_EXPANSION_LIMIT = -1;

/// search query. Pure struct, no member functions
struct CSphQuery
{
	CSphString		m_sIndexes {"*"};	///< indexes to search
	CSphString		m_sQuery;			///< cooked query string for the engine (possibly transformed during legacy matching modes fixup)
	CSphString		m_sRawQuery;		///< raw query string from the client for searchd log, agents, etc

	int				m_iOffset=0;		///< offset into result set (as X in MySQL LIMIT X,Y clause)
	int				m_iLimit=20;		///< limit into result set (as Y in MySQL LIMIT X,Y clause)
	CSphVector<DWORD>	m_dWeights;		///< user-supplied per-field weights. may be NULL. default is NULL
	ESphMatchMode	m_eMode = SPH_MATCH_EXTENDED;		///< match mode. default is "match all"
	ESphRankMode	m_eRanker = SPH_RANK_DEFAULT;		///< ranking mode, default is proximity+BM25
	CSphString		m_sRankerExpr;		///< ranking expression for SPH_RANK_EXPR
	CSphString		m_sUDRanker;		///< user-defined ranker name
	CSphString		m_sUDRankerOpts;	///< user-defined ranker options
	ESphSortOrder	m_eSort = SPH_SORT_RELEVANCE;		///< sort mode
	CSphString		m_sSortBy;			///< attribute to sort by
	int64_t			m_iRandSeed = -1;	///< random seed for ORDER BY RAND(), -1 means do not set
	int				m_iMaxMatches = DEFAULT_MAX_MATCHES;	///< max matches to retrieve, default is 1000. more matches use more memory and CPU time to hold and sort them
	bool			m_bExplicitMaxMatches = false; ///< did we specify the max_matches explicitly?

	bool			m_bSortKbuffer = false;		///< whether to use PQ or K-buffer sorting algorithm
	bool			m_bZSlist = false;			///< whether the ranker has to fetch the zonespanlist with this query
	bool			m_bSimplify = false;		///< whether to apply boolean simplification
	bool			m_bPlainIDF = false;		///< whether to use PlainIDF=log(N/n) or NormalizedIDF=log((N-n+1)/n)
	bool			m_bGlobalIDF = false;		///< whether to use local indexes or a global idf file
	bool			m_bNormalizedTFIDF = true;	///< whether to scale IDFs by query word count, so that TF*IDF is normalized
	bool			m_bLocalDF = false;			///< whether to use calculate DF among local indexes
	bool			m_bLowPriority = false;		///< set low thread priority for this query
	DWORD			m_uDebugFlags = 0;
	QueryOption_e	m_eExpandKeywords = QUERY_OPT_DEFAULT;	///< control automatic query-time keyword expansion
	int				m_iExpansionLimit = DEFAULT_QUERY_EXPANSION_LIMIT;	///< whether to limit wildcard expansion, default use index settings

	bool			m_bAccurateAggregation = false;			///< setting via options
	bool			m_bExplicitAccurateAggregation = false; ///< whether anything was set via options

	int				m_iDistinctThresh = 3500;			///< distinct accuracy thresh
	bool			m_bExplicitDistinctThresh = false;	///< whether thresh was set via options

	int				m_iMaxMatchThresh = 16384;

	CSphVector<CSphFilterSettings>	m_dFilters;	///< filters
	CSphVector<FilterTreeItem_t>	m_dFilterTree;

	CSphVector<IndexHint_t>			m_dIndexHints; ///< secondary index hints

	CSphString		m_sGroupBy;			///< group-by attribute name(s)
	CSphString		m_sFacetBy;			///< facet-by attribute name(s)
	ESphGroupBy		m_eGroupFunc = SPH_GROUPBY_ATTR;	///< function to pre-process group-by attribute value with
	CSphString		m_sGroupSortBy { "@groupby desc" };	///< sorting clause for groups in group-by mode
	CSphString		m_sGroupDistinct;		///< count distinct values for this attribute

	int				m_iCutoff = -1;			///< matches count threshold to stop searching at (<=0 means to search until all matches are found)

	int				m_iRetryCount = DEFAULT_QUERY_RETRY;			///< retry count, for distributed queries. (-1 means 'use default')
	int				m_iRetryDelay = DEFAULT_QUERY_RETRY;			///< retry delay, for distributed queries. (-1 means 'use default')
	int				m_iAgentQueryTimeoutMs = DEFAULT_QUERY_TIMEOUT;	///< agent query timeout override, for distributed queries

	bool			m_bGeoAnchor = false;	///< do we have an anchor
	CSphString		m_sGeoLatAttr;			///< latitude attr name
	CSphString		m_sGeoLongAttr;			///< longitude attr name
	float			m_fGeoLatitude = 0.0f;	///< anchor latitude
	float			m_fGeoLongitude = 0.0f;	///< anchor longitude

	CSphVector<CSphNamedInt>	m_dIndexWeights;	///< per-index weights
	CSphVector<CSphNamedInt>	m_dFieldWeights;	///< per-field weights

	DWORD			m_uMaxQueryMsec = 0;	///< max local index search time, in milliseconds (default is 0; means no limit)
	int				m_iMaxPredictedMsec = 0; ///< max predicted (!) search time limit, in milliseconds (0 means no limit)
	CSphString		m_sComment;				///< comment to pass verbatim in the log file

	CSphString		m_sSelect;				///< select-list (attributes and/or expressions)
	CSphString		m_sOrderBy;				///< order-by clause

	CSphString		m_sOuterOrderBy;		///< temporary (?) subselect hack
	int				m_iOuterOffset = 0;		///< keep and apply outer offset at master
	int				m_iOuterLimit = 0;
	bool			m_bHasOuter = false;

	bool			m_bIgnoreNonexistent = false; ///< whether to warning or not about non-existent columns in select list
	bool			m_bIgnoreNonexistentIndexes = false; ///< whether to error or not about non-existent indexes in index list
	bool			m_bStrict = false;			///< whether to warning or not about incompatible types
	bool			m_bSync = false;			///< whether or not use synchronous operations (optimize, etc.)
	bool			m_bNotOnlyAllowed = false;	///< whether allow single full-text not operator
	CSphString		m_sStore;					///< don't delete result, just store in given uservar by name

	CSphFilterSettings	m_tHaving;				///< post aggregate filtering (got applied only on master)

	int				m_iSQLSelectStart = -1;	///< SQL parser helper
	int				m_iSQLSelectEnd = -1;	///< SQL parser helper

	int				m_iGroupbyLimit = 1;	///< number of elems within group

	CSphVector<CSphQueryItem>	m_dItems;		///< parsed select-list
	CSphVector<CSphQueryItem>	m_dRefItems;	///< select-list prior replacing by facet
	ESphCollation				m_eCollation = SPH_COLLATION_DEFAULT;	///< ORDER BY collation
	bool						m_bAgent = false;	///< agent mode (may need extra cols on output)

	CSphString		m_sQueryTokenFilterLib;		///< token filter library name
	CSphString		m_sQueryTokenFilterName;	///< token filter name
	CSphString		m_sQueryTokenFilterOpts;	///< token filter options

	bool			m_bFacet = false;			///< whether this a facet query
	bool			m_bFacetHead = false;

	QueryType_e		m_eQueryType {QUERY_API};		///< queries from sphinxql require special handling
	const QueryParser_i * m_pQueryParser = nullptr;	///< queries do not own this parser

	// JSON output
	StrVec_t m_dIncludeItems;
	StrVec_t m_dExcludeItems;
	const void*		m_pCookie = nullptr;	///< opaque mark, used to manage lifetime of the vec of queries

	int				m_iConcurrency = 0;    ///< limit N of threads to run query with. 0 means 'no limit'
	CSphVector<CSphString>	m_dStringSubkeys;
	CSphVector<int64_t>		m_dIntSubkeys;
	Dispatcher::Template_t	m_tMainDispatcher;
	Dispatcher::Template_t	m_tPseudoShardingDispatcher;
};

/// parse select list string into items
bool ParseSelectList ( CSphString &sError, CSphQuery &pResult );

/// some low-level query stats
struct CSphQueryStats
{
	int64_t *	m_pNanoBudget = nullptr;///< pointer to max_predicted_time budget (counted in nanosec)
	DWORD		m_iFetchedDocs = 0;		///< processed documents
	DWORD		m_iFetchedHits = 0;		///< processed hits (aka positions)
	DWORD		m_iSkips = 0;			///< number of Skip() calls

	void		Add ( const CSphQueryStats & tStats );
};


struct IteratorDesc_t
{
	CSphString	m_sAttr;
	CSphString	m_sType;
	int			m_iUsed = 1;
};


struct IteratorStats_t
{
	CSphVector<IteratorDesc_t> m_dIterators;
	int		m_iTotal = 0;

	void	Merge ( const IteratorStats_t & tSrc );
};


/// search query meta-info
class CSphQueryResultMeta
{
public:
	int						m_iQueryTime = 0;		///< query time, milliseconds
	int						m_iRealQueryTime = 0;	///< query time, measured just from start to finish of the query. In milliseconds
	int64_t					m_iCpuTime = 0;			///< user time, microseconds
	int						m_iMultiplier = 1;		///< multi-query multiplier, -1 to indicate error

	using WordStat_t = std::pair<int64_t, int64_t>;
	SmallStringHash_T<WordStat_t>	m_hWordStats; 	///< hash of i-th search term (normalized word form)

	int						m_iMatches = 0;			///< total matches returned (upto MAX_MATCHES)
	int64_t					m_iTotalMatches = 0;	///< total matches found (unlimited)
	bool					m_bTotalMatchesApprox = false; ///< whether m_iTotalMatches shows exact or approximate numbers

	CSphIOStats				m_tIOStats;				///< i/o stats for the query
	int64_t					m_iAgentCpuTime = 0;	///< agent cpu time (for distributed searches)
	CSphIOStats				m_tAgentIOStats;		///< agent IO stats (for distributed searches)

	int64_t					m_iPredictedTime = 0;		///< local predicted time
	int64_t					m_iAgentPredictedTime = 0;	///< distributed predicted time
	DWORD					m_iAgentFetchedDocs = 0;	///< distributed fetched docs
	DWORD					m_iAgentFetchedHits = 0;	///< distributed fetched hits
	DWORD					m_iAgentFetchedSkips = 0;	///< distributed fetched skips

	CSphQueryStats 			m_tStats;					///< query prediction counters
	bool					m_bHasPrediction = false;	///< is prediction counters set?

	CSphString				m_sError;				///< error message
	CSphString				m_sWarning;				///< warning message
	QueryProfile_c *		m_pProfile		= nullptr;	///< filled when query profiling is enabled; NULL otherwise

	IteratorStats_t			m_tIteratorStats;		///< iterators used while calculating the query
	bool					m_bBigram = false;		///< whatever to remove bigram symbol on adding word to stat

	virtual					~CSphQueryResultMeta () {}					///< dtor
	void					AddStat ( const CSphString & sWord, int64_t iDocs, int64_t iHits );

	void					MergeWordStats ( const CSphQueryResultMeta& tOther );// sort wordstat to achieve reproducable result over different runs
	CSphFixedVector<SmallStringHash_T<CSphQueryResultMeta::WordStat_t>::KeyValue_t *>	MakeSortedWordStat () const;
};


/// search query result (meta-info)
class QueryProfile_c;
class DocstoreReader_i;
class CSphQueryResult
{
public:
	CSphQueryResultMeta *	m_pMeta = nullptr; 		///< not owned
	const BYTE *			m_pBlobPool = nullptr;	///< pointer to blob attr storage. Used only during calculations.
	const DocstoreReader_i* m_pDocstore = nullptr;	///< pointer to docstore reader fixme! not need in aggr
	columnar::Columnar_i *	m_pColumnar = nullptr;
};

/////////////////////////////////////////////////////////////////////////////
// ATTRIBUTE UPDATE QUERY
/////////////////////////////////////////////////////////////////////////////

struct TypedAttribute_t
{
	CSphString	m_sName;
	ESphAttr	m_eType;
};


struct CSphAttrUpdate
{
	CSphVector<TypedAttribute_t>	m_dAttributes;	///< update schema, attributes to update
	CSphVector<DWORD>				m_dPool;		///< update values pool
	CSphVector<BYTE>				m_dBlobs;		///< update pool for blob attrs
	CSphVector<DocID_t>				m_dDocids;		///< document IDs vector
	CSphVector<int>					m_dRowOffset;	///< document row offsets in the pool (1 per doc, or empty, means 0 always)
	bool							m_bIgnoreNonexistent = false;	///< whether to warn about non-existen attrs, or just silently ignore them
	bool							m_bStrict = false;				///< whether to check for incompatible types first, or just ignore them
	bool							m_bReusable = true;				///< whether update is standalone and never rewritten, or need deep-copy

	inline int GetRowOffset (int i) const
	{
		return m_dRowOffset.IsEmpty() ? 0 : m_dRowOffset[i];
	}
};

using AttrUpdateSharedPtr_t = SharedPtr_t<CSphAttrUpdate>;

inline AttrUpdateSharedPtr_t MakeReusableUpdate ( AttrUpdateSharedPtr_t& pUpdate )
{
	if ( pUpdate->m_bReusable )
		return pUpdate;

	AttrUpdateSharedPtr_t pNewUpdate { new CSphAttrUpdate };
	*pNewUpdate = *pUpdate;
	pNewUpdate->m_bReusable = true;
	return pNewUpdate;
}

struct AttrUpdateInc_t // for cascade (incremental) update
{
	AttrUpdateSharedPtr_t			m_pUpdate;	///< the unchangeable update pool
	CSphBitvec						m_dUpdated;			///< bitmask of updated rows
	int								m_iAffected = 0;	///< num of updated rows.

	explicit AttrUpdateInc_t ( AttrUpdateSharedPtr_t pUpd )
		: m_pUpdate ( std::move(pUpd) )
		, m_dUpdated ( m_pUpdate->m_dDocids.GetLength() )
	{}

	void MarkUpdated ( int iUpd )
	{
		if ( m_dUpdated.BitGet ( iUpd ) )
			return;

		++m_iAffected;
		m_dUpdated.BitSet ( iUpd );
	}

	bool AllApplied () const
	{
		assert ( m_dUpdated.GetSize() >= m_iAffected );
		return m_dUpdated.GetSize() == m_iAffected;
	}
};

/////////////////////////////////////////////////////////////////////////////
// FULLTEXT INDICES
/////////////////////////////////////////////////////////////////////////////

/// progress info
class MergeCb_c
{
	std::atomic<bool> * m_pStop = nullptr;

public:
	enum Event_e : BYTE
	{
		E_IDLE,
		E_COLLECT_START,		// begin collecting alive docs on merge; payload is chunk ID
		E_COLLECT_FINISHED,		// collecting alive docs on merge is finished; payload is chunk ID
		E_MERGEATTRS_START,
		E_MERGEATTRS_FINISHED,
		E_KEYWORDS,
		E_FINISHED,
	};

	explicit MergeCb_c ( std::atomic<bool>* pStop = nullptr )
		: m_pStop ( pStop )
	{}
	virtual ~MergeCb_c() = default;

	virtual void SetEvent ( Event_e eEvent, int64_t iPayload ) {}

	inline bool NeedStop () const
	{
		return sphInterrupted() || ( m_pStop && m_pStop->load ( std::memory_order_relaxed ) );
	}
};

class CSphIndexProgress : private MergeCb_c
{
	MergeCb_c * m_pMergeHook;

private:
	virtual void ShowImpl ( bool bPhaseEnd ) const {};

public:
	enum Phase_e
	{
		PHASE_COLLECT,				///< document collection phase
		PHASE_SORT,					///< final sorting phase
		PHASE_LOOKUP,				///< docid lookup construction
		PHASE_MERGE,				///< index merging
		PHASE_SI_BUILD,				///< secondary index build
		PHASE_UNKNOWN,
	};

	Phase_e			m_ePhase;		///< current indexing phase

	union {
		int64_t m_iDocuments;		///< PHASE_COLLECT: documents collected so far
		int64_t m_iDocids;			///< PHASE_LOOKUP: docids added to lookup so far
		int64_t m_iHits;			///< PHASE_SORT: hits sorted so far
		int64_t m_iWords;			///< PHASE_MERGE: words merged so far
	};

	union {
		int64_t m_iBytes;			///< PHASE_COLLECT: bytes collected so far;
		int64_t m_iDocidsTotal;		///< PHASE_LOOKUP: total docids
		int64_t m_iHitsTotal;		///< PHASE_SORT: hits total
	};

public:
	explicit CSphIndexProgress( MergeCb_c * pMergeHook = nullptr )
	{
		if ( pMergeHook )
			m_pMergeHook = pMergeHook;
		else
			m_pMergeHook = static_cast<MergeCb_c *>(this);
		PhaseBegin ( PHASE_UNKNOWN );
	}

	inline void PhaseBegin ( Phase_e ePhase )
	{
		m_ePhase = ePhase;
		m_iDocuments = m_iBytes = 0;
	}

	inline void PhaseEnd() const
	{
		if ( m_ePhase!=PHASE_UNKNOWN )
			ShowImpl ( true );
	}

	inline void Show() const
	{
		ShowImpl ( false );
	}

	// cb
	MergeCb_c& GetMergeCb() const { return *m_pMergeHook; }
};

/// JSON key lookup stuff
struct JsonKey_t
{
	CSphString		m_sKey;			///< name string
	DWORD			m_uMask = 0;	///< Bloom mask for this key
	int				m_iLen = 0;		///< name length, in bytes

	JsonKey_t () = default;
	explicit JsonKey_t ( const char * sKey, int iLen );
};

/// forward refs to internal searcher classes
class ISphQword;
class ISphQwordSetup;
class CSphQueryContext;
class ISphFilter;
struct GetKeywordsSettings_t;
struct SuggestArgs_t;
struct SuggestResult_t;


struct ISphKeywordsStat
{
	virtual			~ISphKeywordsStat() {}
	virtual bool	FillKeywords ( CSphVector <CSphKeywordInfo> & dKeywords ) const = 0;
};


struct CSphIndexStatus
{
	int64_t			m_iRamUse = 0;
	int64_t			m_iRamRetired = 0;
	int64_t			m_iMapped = 0; // total size of mmapped files
	union {
		int64_t			m_iMappedResident = 0;	// size of mmaped which are in core
		int64_t			m_iStackNeed;			// for pq - max size of stack for eval node over all index on *this* node
	};
	union {
		int64_t			m_iMappedDocs = 0; // size of mmapped doclists
		int64_t			m_iStackBase;	   // for pq - base size over necessary
	};
	int64_t			m_iMappedResidentDocs = 0; // size of mmaped resident doclists
	int64_t			m_iMappedHits = 0; // size of mmapped hitlists
	int64_t			m_iMappedResidentHits = 0; // size of mmaped resident doclists
	int64_t			m_iDiskUse = 0; // place occupied by index on disk (despite if it fetched into mem or not)
	int64_t			m_iRamChunkSize = 0; // not used for plain
	int				m_iNumRamChunks = 0; // not used for plain
	int				m_iNumChunks = 0; // not used for plain
	int64_t			m_iMemLimit = 0; // not used for plain
	int64_t			m_iTID = 0;
	int64_t			m_iSavedTID = 0;
	int64_t 		m_iDead = 0;
	double			m_fSaveRateLimit {0.0};	 // not used for plain. Part of m_iMemLimit to be achieved before flushing
};


struct CSphMultiQueryArgs : public ISphNoncopyable
{
	const int								m_iIndexWeight;
	int										m_iTag = 0;
	DWORD									m_uPackedFactorFlags { SPH_FACTOR_DISABLE };
	bool									m_bLocalDF = false;
	const SmallStringHash_T<int64_t> *		m_pLocalDocs = nullptr;
	int64_t									m_iTotalDocs = 0;
	bool									m_bModifySorterSchemas = true;
	bool									m_bFinalizeSorters = true;
	int										m_iThreads = 1;
	int										m_iTotalThreads = 1;

	CSphMultiQueryArgs ( int iIndexWeight );
};


struct RowToUpdateData_t
{
	RowID_t 			m_tRow; /// row in the index
	int					m_iIdx;	/// idx in updateset
};

using RowsToUpdateData_t = CSphVector<RowToUpdateData_t>;
using RowsToUpdate_t = VecTraits_T<RowToUpdateData_t>;

struct PostponedUpdate_t
{
	AttrUpdateSharedPtr_t	m_pUpdate;
	RowsToUpdateData_t		m_dRowsToUpdate;
};

struct UpdateContext_t;
using BlockerFn = std::function<bool()>;

// common attribute update code for both RT and plain indexes
// an index or a part of an index that has its own row ids
class IndexSegment_c
{
	mutable IndexSegment_c* m_pKillHook = nullptr; // if set, killed docids will be emerged also here.

protected:
	enum
	{
		ATTRS_UPDATED			= ( 1UL<<0 ),
		ATTRS_BLOB_UPDATED		= ( 1UL<<1 ),
		ATTRS_ROWMAP_UPDATED	= ( 1UL<<2 )
	};

private:
	virtual bool		Update_WriteBlobRow ( UpdateContext_t & tCtx, RowID_t tRowID, ByteBlob_t tBlob,
			int nBlobAttrs, const CSphAttrLocator & tBlobRowLoc, bool & bCritical, CSphString & sError ) {return false;};

	static bool			Update_InplaceJson ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx, CSphString & sError, bool bDryRun );
	bool				Update_Blobs ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx, bool & bCritical, CSphString & sError );
	static void			Update_Plain ( const RowsToUpdate_t& dRows, UpdateContext_t & tCtx );

public:
	virtual int			Kill ( DocID_t  /*tDocID*/ ) { return 0; }
	virtual int			KillMulti ( const VecTraits_T<DocID_t> &  /*dKlist*/ ) { return 0; };
	virtual int			CheckThenKillMulti ( const VecTraits_T<DocID_t>& dKlist, BlockerFn&& /*fnWatcher*/ ) { return KillMulti ( dKlist ); };
	virtual				~IndexSegment_c() = default;

	inline void			SetKillHook ( IndexSegment_c * pKillHook ) const noexcept
	{
		m_pKillHook = pKillHook;
	}

	inline bool HasKillHook () const noexcept
	{
		return m_pKillHook!=nullptr;
	}

	inline void KillHook ( DocID_t tDocID ) const noexcept
	{
		if ( HasKillHook() )
			m_pKillHook->Kill ( tDocID );
	}

public:
	bool Update_UpdateAttributes ( const RowsToUpdate_t& dRows, UpdateContext_t& tCtx, bool& bCritical, CSphString& sError );

	/// apply serie of updates, assuming them prepared (no need to full-scan attributes), and index is offline, i.e. no concurrency
	virtual void UpdateAttributesOffline ( VecTraits_T<PostponedUpdate_t>& dPostUpdates ) {}

	inline void ResetPostponedUpdates()
	{
		m_bAttrsBusy = false;
		m_dPostponedUpdates.Reset();
	}

public:
	// stuff for dispatch races between changes and updates
	mutable std::atomic<bool>		m_bAttrsBusy { false };
	CSphVector<PostponedUpdate_t>	m_dPostponedUpdates;
};

bool Update_CheckAttributes ( const CSphAttrUpdate& tUpd, const ISphSchema& tSchema, CSphString& sError );

// helper - collects killed documents
struct KillAccum_t final : public IndexSegment_c
{
	CSphVector<DocID_t> m_dDocids;

	int Kill ( DocID_t tDocID ) final
	{
		m_dDocids.Add ( tDocID );
		return 1;
	}
};

class Histogram_i;
class HistogramContainer_c;

struct UpdatedAttribute_t
{
	CSphAttrLocator		m_tLocator;
	CSphRefcountedPtr<ISphExpr>	m_pExpr;
	Histogram_i *		m_pHistogram {nullptr};
	ESphAttr			m_eAttrType {SPH_ATTR_NONE};
	TypeConversion_e	m_eConversion {CONVERSION_NONE};
	bool				m_bExisting {false};
	int					m_iSchemaAttr = -1;
};

struct UpdateContext_t
{
	AttrUpdateInc_t &						m_tUpd;
	const ISphSchema &						m_tSchema;
	const HistogramContainer_c *			m_pHistograms {nullptr};
	CSphRowitem *							m_pAttrPool {nullptr};
	BYTE *									m_pBlobPool {nullptr};
	int 									m_iStride {0};

	CSphFixedVector<UpdatedAttribute_t>		m_dUpdatedAttrs;	// manipulation schema (1 item per column of schema)

	CSphBitvec			m_dSchemaUpdateMask;
	DWORD				m_uUpdateMask {0};
	bool				m_bBlobUpdate {false};
	int					m_iJsonWarnings {0};


	UpdateContext_t ( AttrUpdateInc_t & tUpd, const ISphSchema & tSchema );

	void PrepareListOfUpdatedAttributes ( CSphString& sError );
	bool HandleJsonWarnings ( int iUpdated, CSphString& sWarning, CSphString& sError ) const;
	CSphRowitem* GetDocinfo ( RowID_t tRowID ) const;
};

class DocstoreAddField_i;
void SetupDocstoreFields ( DocstoreAddField_i & tFields, const CSphSchema & tSchema );
bool CheckStoredFields ( const CSphSchema & tSchema, const CSphIndexSettings & tSettings, CSphString & sError );

class DiskIndexQwordTraits_c;
DiskIndexQwordTraits_c * sphCreateDiskIndexQword ( bool bInlineHits );

/// returns ranker name as string
const char * sphGetRankerName ( ESphRankMode eRanker );

struct DocstoreDoc_t
{
	CSphVector<CSphVector<BYTE>> m_dFields;
};

// used to fetch documents from docstore by docids
class DocstoreReader_i
{
public:
	virtual			~DocstoreReader_i() = default;

	virtual void	CreateReader ( int64_t iSessionId ) const {}
	virtual bool	GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const = 0;
	virtual int		GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const = 0;
};

bool IsMlock ( FileAccess_e eType );
bool IsOndisk ( FileAccess_e eType );

Bson_t EmptyBson();

// returns correct size even if iBuf is 0
int GetReadBuffer ( int iBuf );

class ISphMatchSorter;
class CSphSource;
struct CSphSourceStats;
class DebugCheckError_i;
struct AttrAddRemoveCtx_t;
class Docstore_i;

namespace SI
{
	class Index_i;
}

enum ESphExt : BYTE;

/// generic fulltext index interface
class CSphIndex : public ISphKeywordsStat, public IndexSegment_c, public DocstoreReader_i, public IndexFileBase_c
{
public:
								CSphIndex ( CSphString sIndexName, CSphString sFilename );
								~CSphIndex() override;

	const CSphString &			GetLastError() const { return m_sLastError; }
	const CSphString &			GetLastWarning() const { return m_sLastWarning; }
	virtual const CSphSchema &	GetMatchSchema() const { return m_tSchema; }			///< match schema as returned in result set (possibly different from internal storage schema!)

	void						SetInplaceSettings ( int iHitGap, float fRelocFactor, float fWriteFactor ); // fixme! build only
	void						SetFieldFilter ( std::unique_ptr<ISphFieldFilter> pFilter );
	const ISphFieldFilter *		GetFieldFilter() const { return m_pFieldFilter.get(); }
	void						SetTokenizer ( TokenizerRefPtr_c pTokenizer );
	void						SetupQueryTokenizer();
	TokenizerRefPtr_c			GetTokenizer () const;
	TokenizerRefPtr_c			GetQueryTokenizer () const;
	TokenizerRefPtr_c&			ModifyTokenizer ();
	void						SetDictionary ( DictRefPtr_c pDict );
	DictRefPtr_c				GetDictionary () const;
	virtual void				SetKeepAttrs ( const CSphString & , const StrVec_t & ) {} // fixme! build only
	virtual void				Setup ( const CSphIndexSettings & tSettings );
	const CSphIndexSettings &	GetSettings () const { return m_tSettings; }
	virtual bool				IsRT() const { return false; }
	virtual bool				IsPQ() const { return false; }
	void						SetBinlog ( bool bBinlog ) { m_bBinlog = bBinlog; }
	virtual int64_t *			GetFieldLens() const { return nullptr; }
	virtual bool				IsStarDict ( bool bWordDict ) const;
	int64_t						GetIndexId() const { return m_iIndexId; }
	void						SetMutableSettings ( const MutableIndexSettings_c & tSettings );
	const MutableIndexSettings_c & GetMutableSettings () const { return m_tMutableSettings; }

	virtual std::pair<int64_t,int> GetPseudoShardingMetric ( const VecTraits_T<const CSphQuery> & dQueries, const VecTraits_T<int64_t> & dMaxCountDistinct, int iThreads, bool & bForceSingleThread ) const { return { 0, 0 }; }
	virtual bool				MustRunInSingleThread ( const VecTraits_T<const CSphQuery> & dQueries, bool bHasSI, const VecTraits_T<int64_t> & dMaxCountDistinct, bool & bForceSingleThread ) const;
	virtual int64_t				GetCountDistinct ( const CSphString & sAttr ) const { return -1; }	// returns values if index has some meta on its attributes
	virtual int64_t				GetCountFilter ( const CSphFilterSettings & tFilter ) const { return -1; }	// returns values if index has some meta on its attributes
	virtual int64_t				GetCount() const { return -1; }

public:
	/// build index by indexing given sources
	virtual int					Build ( const CSphVector<CSphSource*> & dSources, int iMemoryLimit, int iWriteBuffer, CSphIndexProgress & ) = 0; // fixme! build only

	/// build index by mering current index with given index
	virtual bool				Merge ( CSphIndex * pSource, const VecTraits_T<CSphFilterSettings> & dFilters, bool bSupressDstDocids, CSphIndexProgress & tProgress ) = 0; // fixme! build only

public:
	/// check all data files, preload schema, and preallocate enough RAM to load memory-cached data
	virtual bool				Prealloc ( bool bStripPath, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings ) = 0;

	/// deallocate all previously preallocated shared data
	virtual void				Dealloc () = 0;

	/// precache everything which needs to be precached
	virtual void				Preread () = 0;

	/// set new index base path, and physically rename index files too
	enum RenameResult_e { RE_OK, RE_FAIL, RE_FATAL };
	virtual RenameResult_e		RenameEx ( CSphString sNewBase ) = 0;

	bool 						Rename ( CSphString sNewBase ) { return RenameEx (std::move(sNewBase))==RE_OK; }

	/// obtain exclusive lock on this index
	virtual bool				Lock () = 0;

	/// dismiss exclusive lock and unlink lock file
	virtual void				Unlock () = 0;

	/// called when index is loaded and prepared to work
	virtual void				PostSetup();

public:
	/// return index document, bytes totals (FIXME? remove this in favor of GetStatus() maybe?)
	virtual const CSphSourceStats &		GetStats () const = 0;

	/// return additional index info
	virtual void				GetStatus ( CSphIndexStatus* ) const = 0;

public:
	virtual bool				EarlyReject ( CSphQueryContext * pCtx, CSphMatch & tMatch ) const = 0;
	void						SetCacheSize ( int iMaxCachedDocs, int iMaxCachedHits );

	/// one regular query vs many sorters (like facets, or similar for common-tree optimization)
	virtual bool				MultiQuery ( CSphQueryResult & tResult, const CSphQuery & tQuery, const VecTraits_T<ISphMatchSorter *> & dSorters, const CSphMultiQueryArgs & tArgs ) const = 0;

	/// many regular queries with one sorter attached to each query.
	/// returns true if at least one query succeeded. The failed queries indicated with pResult->m_iMultiplier==-1
	virtual bool				MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult* pResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const = 0;
	virtual bool				GetKeywords ( CSphVector <CSphKeywordInfo> & dKeywords, const char * szQuery, const GetKeywordsSettings_t & tSettings, CSphString * pError ) const = 0;
	virtual void				GetSuggest ( const SuggestArgs_t & , SuggestResult_t & ) const {}
	virtual Bson_t				ExplainQuery ( const CSphString & sQuery ) const { return EmptyBson(); }

public:
	/// returns non-negative amount of actually found and updated records on success
	/// on failure, -1 is returned and GetLastError() contains error message
	int							UpdateAttributes ( AttrUpdateSharedPtr_t pUpd, bool & bCritical, CSphString & sError, CSphString & sWarning );
	int							UpdateAttributes ( AttrUpdateInc_t & tUpd, bool & bCritical, CSphString & sError, CSphString & sWarning );

	/// update accumulating state
	virtual int					CheckThenUpdateAttributes ( AttrUpdateInc_t& tUpd, bool& bCritical, CSphString& sError, CSphString& sWarning, BlockerFn&& /*fnWatcher*/ ) = 0;

	virtual Binlog::CheckTnxResult_t ReplayTxn ( Binlog::Blop_e eOp, CSphReader & tReader, CSphString & sError, Binlog::CheckTxn_fn&& fnCheck ) = 0;
	/// saves memory-cached attributes, if there were any updates to them
	/// on failure, false is returned and GetLastError() contains error message
	virtual bool				SaveAttributes ( CSphString & sError ) const = 0;

	virtual DWORD				GetAttributeStatus () const = 0;

	virtual bool				AddRemoveAttribute ( bool bAddAttr, const AttrAddRemoveCtx_t & tCtx, CSphString & sError ) = 0;

	virtual bool				AddRemoveField ( bool bAdd, const CSphString & sFieldName, DWORD, CSphString & sError ) = 0;

	virtual void				FlushDeadRowMap ( bool bWaitComplete ) const {}
	virtual bool				LoadKillList ( CSphFixedVector<DocID_t> * pKillList, KillListTargets_c & tTargets, CSphString & sError ) const { return true; }
	virtual bool				AlterKillListTarget ( KillListTargets_c & tTargets, CSphString & sError ) { return false; }
	virtual void				KillExistingDocids ( CSphIndex * pTarget ) const {}
	virtual bool				IsAlive ( DocID_t tDocID ) const { return false; }

	bool						GetDoc ( DocstoreDoc_t & tDoc, DocID_t tDocID, const VecTraits_T<int> * pFieldIds, int64_t iSessionId, bool bPack ) const override { return false; }
	int							GetFieldId ( const CSphString & sName, DocstoreDataType_e eType ) const override { return -1; }

public:
	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpHeader ( FILE * fp, const CSphString& sHeaderName, bool bConfig ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpDocids ( FILE * fp ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpHitlist ( FILE * fp, const char * sKeyword, bool bID ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual void				DebugDumpDict ( FILE * fp ) = 0;

	/// internal debugging hook, DO NOT USE
	virtual int					DebugCheck ( DebugCheckError_i& ) = 0;
	virtual void				SetDebugCheck ( bool bCheckIdDups, int iCheckChunk ) {}

	/// getter for name. Notice, const char* returned as it is mostly used for printing name
	const char *				GetName () const { return m_sIndexName.cstr(); }

	void						SetName ( CSphString sNewName ) { m_sIndexName = std::move ( sNewName ); }

	/// get actual index files list
	virtual void				GetIndexFiles ( StrVec_t& dFiles, StrVec_t& dExt, const FilenameBuilder_i* = nullptr ) const {}


	/// internal make document id list from external docinfo, DO NOT USE
	virtual CSphVector<SphAttr_t> BuildDocList () const;

	virtual void				GetFieldFilterSettings ( CSphFieldFilterSettings & tSettings ) const;

	// put external files (if any) into index folder
	// copy the rest of the external files to index folder
	virtual bool				CopyExternalFiles ( int iPostfix, StrVec_t & dCopied ) { return true; }

	// used for query optimizer calibration
	virtual HistogramContainer_c * Debug_GetHistograms() const { return nullptr; }
	virtual SI::Index_i *		Debug_GetSI() const { return nullptr; }

	virtual Docstore_i *			GetDocstore() const { return nullptr; }
	virtual columnar::Columnar_i *	GetColumnar() const { return nullptr; }
	virtual const DWORD *			GetRawAttrs() const { return nullptr; }
	virtual const BYTE *			GetRawBlobAttrs() const { return nullptr; }
	virtual bool					AlterSI ( CSphString & sError ) { return true; }

public:
	int64_t						m_iTID = 0;				///< last committed transaction id
	int							m_iChunk = 0;

	int							m_iExpansionLimit = 0;

protected:
	int64_t						m_iIndexId;				///< internal (per daemon) unique index id, introduced for caching

	CSphSchema					m_tSchema;
	CSphString					m_sLastError;
	CSphString					m_sLastWarning;

	bool						m_bInplaceSettings = false; // fixme! build only
	int							m_iHitGap = 0; // fixme! build only
	float						m_fRelocFactor { 0.0f }; // fixme! build only
	float						m_fWriteFactor { 0.0f }; // fixme! build only

	bool						m_bBinlog = true;

protected:
	CSphIndexSettings			m_tSettings;
	MutableIndexSettings_c		m_tMutableSettings;

	std::unique_ptr<ISphFieldFilter>		m_pFieldFilter;
	TokenizerRefPtr_c		m_pTokenizer;
	TokenizerRefPtr_c		m_pQueryTokenizer;
	TokenizerRefPtr_c		m_pQueryTokenizerJson;
	DictRefPtr_c			m_pDict;

	int							m_iMaxCachedDocs = 0;
	int							m_iMaxCachedHits = 0;

private:
	CSphString					m_sIndexName;			///< index ID in binlogging; otherwise used only in messages. Use GetName()!

public:
	void						SetGlobalIDFPath ( const CSphString & sPath ) { m_sGlobalIDFPath = sPath; }
	float						GetGlobalIDF ( const CSphString & sWord, int64_t iDocsLocal, bool bPlainIDF ) const;

protected:
	CSphString					m_sGlobalIDFPath;
};

const CSphSourceStats& GetStubStats();
bool CheckDocsCount ( int64_t iDocs, CSphString & sError );

// dummy implementation which makes most of the things optional (makes all non-disk idxes much simpler)
class CSphIndexStub : public CSphIndex
{
public:
						FWD_CTOR ( CSphIndexStub, CSphIndex )
	int					Build ( const CSphVector<CSphSource *> &, int, int, CSphIndexProgress& ) override { return 0; }
	bool				Merge ( CSphIndex *, const VecTraits_T<CSphFilterSettings> &, bool, CSphIndexProgress & ) override { return false; }
	bool				Prealloc ( bool, FilenameBuilder_i *, StrVec_t & ) override { return false; }
	void				Dealloc () override {}
	void				Preread () override {}
	RenameResult_e		RenameEx ( CSphString ) override { return RE_FAIL; }
	bool				Lock () override { return true; }
	void				Unlock () override {}
	bool				EarlyReject ( CSphQueryContext * , CSphMatch & ) const override { return false; }
	const CSphSourceStats &	GetStats () const override
	{
		return GetStubStats();
	}
	void				GetStatus ( CSphIndexStatus* ) const override {}
	bool				GetKeywords ( CSphVector <CSphKeywordInfo> & , const char * , const GetKeywordsSettings_t & tSettings, CSphString * ) const override { return false; }
	bool				FillKeywords ( CSphVector <CSphKeywordInfo> & ) const override { return true; }
	int					CheckThenUpdateAttributes ( AttrUpdateInc_t&, bool &, CSphString & , CSphString &, BlockerFn&& ) override { return -1; }
	Binlog::CheckTnxResult_t ReplayTxn ( Binlog::Blop_e, CSphReader &, CSphString &, Binlog::CheckTxn_fn&& ) override { return {}; }
	bool				SaveAttributes ( CSphString & ) const override { return true; }
	DWORD				GetAttributeStatus () const override { return 0; }
	bool				AddRemoveAttribute ( bool, const AttrAddRemoveCtx_t & tCtx, CSphString & sError ) override { return true; }
	bool				AddRemoveField ( bool, const CSphString &, DWORD, CSphString & ) override { return true; }
	void				DebugDumpHeader ( FILE *, const CSphString&, bool ) override {}
	void				DebugDumpDocids ( FILE * ) override {}
	void				DebugDumpHitlist ( FILE * , const char * , bool ) override {}
	int					DebugCheck ( DebugCheckError_i& ) override { return 0; }
	void				DebugDumpDict ( FILE * ) override {}
	Bson_t				ExplainQuery ( const CSphString & sQuery ) const override { return EmptyBson (); }

	bool				MultiQuery ( CSphQueryResult & , const CSphQuery & , const VecTraits_T<ISphMatchSorter *> &, const CSphMultiQueryArgs & ) const override { return false; }
	bool 				MultiQueryEx ( int iQueries, const CSphQuery * pQueries, CSphQueryResult* pResults, ISphMatchSorter ** ppSorters, const CSphMultiQueryArgs & tArgs ) const override
	{
		// naive stub implementation without common subtree cache and/or any other optimizations
		bool bResult = false;
		for ( int i=0; i<iQueries; ++i )
			if ( MultiQuery ( pResults[i], pQueries[i], { ppSorters+i, 1}, tArgs ) )
				bResult = true;
			else
				pResults[i].m_pMeta->m_iMultiplier = -1; // -1 means 'error'

		return bResult;
	}
};

// update attributes with index pointer attached
struct CSphAttrUpdateEx
{
	AttrUpdateSharedPtr_t	m_pUpdate;				///< the unchangeable update pool
	CSphIndex *				m_pIndex = nullptr;		///< the index on which the update should happen
	CSphString *			m_pError = nullptr;		///< the error, if any
	CSphString *			m_pWarning = nullptr;	///< the warning, if any
	int						m_iAffected = 0;		///< num of updated rows.
};

class RowBuffer_i;
struct SphQueueSettings_t
{
	const ISphSchema &			m_tSchema;
	QueryProfile_c *			m_pProfiler;
	bool						m_bComputeItems = false;
	CSphVector<BYTE> *			m_pCollection = nullptr;
	ISphExprHook *				m_pHook = nullptr;
	const CSphFilterSettings *	m_pAggrFilter = nullptr;
	int							m_iMaxMatches = DEFAULT_MAX_MATCHES;
	bool						m_bNeedDocids = false;
	bool						m_bGrouped = false;	// are we going to push already grouped matches to it?
	std::function<int64_t (const CSphString &)>			m_fnGetCountDistinct;
	std::function<int64_t (const CSphFilterSettings &)>	m_fnGetCountFilter;
	std::function<int64_t ()>							m_fnGetCount;
	bool						m_bEnableFastDistinct = false;
	bool						m_bForceSingleThread = false;
	StrVec_t 					m_dCreateSchema;
	RowBuffer_i*				m_pSqlRowBuffer;
	void **						m_ppOpaque1 = nullptr;
	void **						m_ppOpaque2 = nullptr;

	explicit SphQueueSettings_t ( const ISphSchema & tSchema, QueryProfile_c * pProfiler = nullptr, RowBuffer_i * pSqlRowBuffer = nullptr, void ** ppOpaque1 = nullptr, void ** ppOpaque2 = nullptr )
		: m_tSchema ( tSchema )
		, m_pProfiler ( pProfiler )
		, m_pSqlRowBuffer ( pSqlRowBuffer )
		, m_ppOpaque1 ( ppOpaque1 )
		, m_ppOpaque2 ( ppOpaque2 )
	{}
};

struct SphQueueRes_t : public ISphNoncopyable
{
	DWORD m_uPackedFactorFlags {SPH_FACTOR_DISABLE};
	bool						m_bZonespanlist = false;
	bool						m_bAlowMulti = true;
};

/////////////////////////////////////////////////////////////////////////////

/// create phrase fulltext index implementation
std::unique_ptr<CSphIndex>		sphCreateIndexPhrase ( CSphString sIndexName, CSphString sFilename );

/// create template (tokenizer) index implementation
std::unique_ptr<CSphIndex>		sphCreateIndexTemplate ( CSphString sIndexName );

/// set JSON attribute indexing options
/// bStrict is whether to stop indexing on error, or just ignore the attribute value
/// bAutoconvNumbers is whether to auto-convert eligible (!) strings to integers and floats, or keep them as strings
/// bKeynamesToLowercase is whether to convert all key names to lowercase
void				sphSetJsonOptions ( bool bStrict, bool bAutoconvNumbers, bool bKeynamesToLowercase );

/// setup per-keyword read buffer sizes
void				SetUnhintedBuffer ( int iReadUnhinted );
int					GetUnhintedBuffer();

/// check query for expressions
bool				sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema );

void				SetPseudoSharding ( bool bSet );
bool				GetPseudoSharding();
void				SetPseudoShardingThresh ( int iThresh );

void				InitSkipCache ( int64_t iCacheSize );
void				ShutdownSkipCache();

//////////////////////////////////////////////////////////////////////////

volatile bool & sphGetbCpuStat () noexcept;

// Access to global TFO settings
volatile int& sphGetTFO() noexcept;
#define TFO_CONNECT 1
#define TFO_LISTEN 2
#define TFO_ABSENT (-1)
/////////////////////////////////////////////////////////////////////////////
// workaround to suppress C4511/C4512 warnings (copy ctor and assignment operator) in VS 2003
#if _MSC_VER>=1300 && _MSC_VER<1400
#pragma warning(disable:4511)
#pragma warning(disable:4512)
#endif

// suppress C4201 (nameless struct/union is a nonstandard extension) because even min-spec gcc 3.4.6 works ok
#if defined(_MSC_VER)
#pragma warning(disable:4201)
#endif

#endif // _sphinx_
