//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "sphinx.h"
#include "searchdexpr.h"
#include "failures_log.h"
#include "searchdaemon.h"
#include "searchdha.h"


struct QueryStat_t
{
	uint64_t	m_uQueryTime = 0;
	uint64_t	m_uFoundRows = 0;
	int			m_iSuccesses = 0;
};


struct StatsPerQuery_t
{
	CSphVector<QueryStat_t> m_dStats;
};

struct DistrServedByAgent_t : StatsPerQuery_t
{
	CSphString						m_sIndex;
	CSphVector<int>					m_dAgentIds;
	StrVec_t						m_dLocalNames;
};


/// manage collection of indexes (to keep them alive)
/// Get(name) - returns an index from collection.
/// AddUniqIndex(name) - add local idx to collection, addref is implied by design
/// AddIndex(name,pidx) - add custom idx, to make it available with Get()
class KeepCollection_c
{
	SmallStringHash_T<cServedIndexRefPtr_c> m_hIndexes;

public:
	// add from globally served
	bool AddUniqIndex ( const CSphString& sName );

	// add custom
	void AddIndex ( const CSphString& sName, cServedIndexRefPtr_c pIdx );

	// use idx
	cServedIndexRefPtr_c Get ( const CSphString &sName ) const;
};

struct LocalIndex_t
{
	CSphString	m_sName;
	CSphString	m_sParentIndex;
	int			m_iOrderTag = 0;
	int			m_iWeight = 1;
	int64_t		m_iMass = 0;
};

struct LocalSearchRef_t;
class GlobalSorters_c;


class SearchHandler_c final
{
public:
									SearchHandler_c (SearchHandler_c&&) = default;
									SearchHandler_c ( int iQueries, std::unique_ptr<QueryParser_i> pParser, QueryType_e eQueryType, bool bMaster=true ) noexcept;
									SearchHandler_c ( CSphFixedVector<CSphQuery>, bool bMaster=true ) noexcept;

									~SearchHandler_c();

	void							RunQueries ();					///< run all queries, get all results
	void							RunCollect ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors, CSphVector<BYTE> * pCollectedDocs );
	void							SetQuery ( int iQuery, const CSphQuery & tQuery, std::unique_ptr<ISphTableFunc> pTableFunc );
	void							SetJoinQueryOptions ( int iQuery, const CSphQuery & tJoinQueryOptions ) const { m_dJoinQueryOptions[iQuery] = tJoinQueryOptions; }
	void							SetQueryParser ( std::unique_ptr<QueryParser_i> pParser, QueryType_e eQueryType );
	void							SetProfile ( QueryProfile_c * pProfile );
	void							SetFederatedUser () { m_bFederatedUser = true; }

public:
	CSphFixedVector<CSphQuery>				m_dQueries;						///< queries which i need to search
	CSphFixedVector<CSphQuery>				m_dJoinQueryOptions;			///< join query options
	CSphFixedVector<AggrResult_t>			m_dAggrResults;					///< results which i obtained

	CSphFixedVector<SearchFailuresLog_c>	m_dFailuresSet;					///< failure logs for each query
	CSphFixedVector<CSphVector<int64_t>>	m_dAgentTimes;					///< per-agent time stats
	KeepCollection_c				m_dAcquired;					/// locked indexes
	CSphFixedVector<std::unique_ptr<ISphTableFunc>>	m_dTables;
	SqlStmt_t *						m_pStmt = nullptr;				///< original (one) statement to take extra options

private:
									explicit SearchHandler_c ( int iQueries ) noexcept;
	void							RunSubset ( int iStart, int iEnd );	///< run queries against index(es) from first query in the subset
	void							RunLocalSearches();
	bool							AllowsMulti() const;
	void							SetupLocalDF();

	bool							m_bMultiQueue = false;	///< whether current subset is subject to multi-queue optimization
	bool							m_bFacetQueue = false;	///< whether current subset is subject to facet-queue optimization
	CSphVector<LocalIndex_t>		m_dLocal;				///< local indexes for the current subset
	CSphVector<StatsPerQuery_t>		m_dQueryIndexStats;		///< statistics for current query
	StrVec_t 						m_dExtraSchema;		 	///< the extra attrs for agents. One vec per index*threads
	CSphVector<BYTE> *				m_pCollectedDocs = nullptr;	///< this query is for deleting

	QueryProfile_c *				m_pProfile = nullptr;
	QueryType_e						m_eQueryType {QUERY_API}; ///< queries from sphinxql require special handling
	std::unique_ptr<QueryParser_i>	m_pQueryParser;	///< parser used for queries in this handler. e.g. plain or json-style

	bool							m_bNeedDocIDs = false;	///< do we need docids returned from local searches (remotes return them anyway)?

	// FIXME!!! breaks for dist threads with SNIPPETS expressions for queries to multiple indexes
	mutable ExprHook_c				m_tHook;

	SmallStringHash_T < int64_t >	m_hLocalDocs;
	int64_t							m_iTotalDocs = 0;
	bool							m_bGotLocalDF = false;
	bool							m_bMaster;
	bool							m_bFederatedUser = false;
	bool							m_bQueryLog = true;

	void							OnRunFinished ();

	CSphFixedVector<CSphQueryResult>	m_dResults;
	VecTraits_T<CSphQuery>				m_dNQueries;		///< working subset of queries
	VecTraits_T<CSphQuery>				m_dNJoinQueryOptions;///< working subset of join query options
	VecTraits_T<AggrResult_t>			m_dNAggrResults;	///< working subset of results
	VecTraits_T<CSphQueryResult>		m_dNResults;		///< working subset of result pointers
	VecTraits_T<SearchFailuresLog_c>	m_dNFailuresSet;	///< working subset of failures

	struct IndexPSInfo_t
	{
		int		m_iThreads = 0;		// threads per index
		int		m_iMaxThreads = 0;	// max threads per index (used for consistency between GetPseudoShardingMetric() and SpawnIterators()
		bool	m_bForceSingleThread = false;	// for disk chunks; means "run all disk chunk searches in a single thread"
	};

	CSphVector<IndexPSInfo_t>			m_dPSInfo;

	StringBuilder_c						m_sError;

	struct JoinedServedIndex_t
	{
		cServedIndexRefPtr_c	m_pServed;
		int						m_iDupeId = -1;
	};

	bool							ParseSysVar();
	bool							ParseIdxSubkeys();
	bool							CheckMultiQuery() const;
	bool							AcquireInvokedIndexes();
	void							UniqLocals ( VecTraits_T<LocalIndex_t>& dLocals );
	void							RunActionQuery ( const CSphQuery & tQuery, const CSphString & sIndex, CSphString * pErrors ); ///< run delete/update
	bool							BuildIndexList ( int & iDivideLimits, VecRefPtrsAgentConn_t & dRemotes, CSphVector<DistrServedByAgent_t> & dDistrServedByAgent ); // fixme!
	void							CalcTimeStats ( int64_t tmCpu, int64_t tmSubset, const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent );
	void							CalcPerIndexStats ( const CSphVector<DistrServedByAgent_t> & dDistrServedByAgent ) const;
	void							CalcGlobalStats ( int64_t tmCpu, int64_t tmSubset, int64_t tmLocal, const CSphIOStats & tIO, const VecRefPtrsAgentConn_t & dRemotes ) const;
	int								CreateSorters ( const CSphIndex * pIndex, CSphVector<const CSphIndex*> & dJoinedIndexes, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const;
	int								CreateSingleSorters ( const CSphIndex * pIndex, CSphVector<const CSphIndex*> & dJoinedIndexes, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const;
	int								CreateMultiQueryOrFacetSorters ( const CSphIndex * pIndex, CSphVector<const CSphIndex*> & dJoinedIndexes, VecTraits_T<ISphMatchSorter*> & dSorters, VecTraits_T<CSphString> & dErrors, StrVec_t * pExtra, SphQueueRes_t & tQueueRes, ISphExprHook * pHook ) const;

	SphQueueSettings_t				MakeQueueSettings ( const CSphIndex * pIndex, const CSphIndex * pJoinedIndex, int iMaxMatches, bool bForceSingleThread, ISphExprHook * pHook ) const;
	cServedIndexRefPtr_c			CheckIndexSelectable ( const CSphString& sLocal, const char * szParent, VecTraits_T<SearchFailuresLog_c> * pNFailuresSet=nullptr ) const;
	bool							PopulateJoinedIndexes ( CSphVector<JoinedServedIndex_t> & dJoinedServed, VecTraits_T<SearchFailuresLog_c> & dFailuresSet ) const;
	CSphVector<const CSphIndex*>	GetRlockedJoinedIndexes ( const CSphVector<JoinedServedIndex_t> & dJoinedServed, std::vector<RIdx_c> & dRLockedJoined ) const;
	bool							CreateValidSorters ( VecTraits_T<ISphMatchSorter *> & dSrt, SphQueueRes_t * pQueueRes, VecTraits_T<SearchFailuresLog_c> & dFlr, StrVec_t * pExtra, const CSphIndex* pIndex, CSphVector<const CSphIndex*> & dJoinedIndexes, const CSphString & sLocal, const char * szParent, ISphExprHook * pHook );

	void							PopulateCountDistinct ( CSphVector<CSphVector<int64_t>> & dCountDistinct ) const;
	int								CalcMaxThreadsPerIndex ( int iConcurrency ) const;
	void							CalcThreadsPerIndex ( int iConcurrency );

	bool							SubmitSuccess ( CSphVector<ISphMatchSorter *> & dSorters, GlobalSorters_c & tGlobalSorters, LocalSearchRef_t & tCtx, int64_t & iCpuTime, int iQuery, int iLocal, const CSphQueryResultMeta & tMqMeta, const CSphQueryResult & tMqRes );
};

 SearchHandler_c CreateMsearchHandler ( std::unique_ptr<QueryParser_i> pQueryParser, QueryType_e eQueryType, ParsedJsonQuery_t & tParsed );