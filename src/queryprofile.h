//
// Copyright (c) 2017-2022, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _queryprofile_
#define _queryprofile_

#include "sphinxdefs.h"

class CSphSchema;

#define SPH_QUERY_STATES \
	SPH_QUERY_STATE ( UNKNOWN,		"unknown" ) \
	SPH_QUERY_STATE ( NET_READ,		"net_read" ) \
	SPH_QUERY_STATE ( IO,			"io" ) \
	SPH_QUERY_STATE ( DIST_CONNECT,	"dist_connect" ) \
	SPH_QUERY_STATE ( LOCAL_DF,		"local_df" ) \
	SPH_QUERY_STATE ( LOCAL_SEARCH,	"local_search" ) \
	SPH_QUERY_STATE ( SQL_PARSE,	"sql_parse" ) \
	SPH_QUERY_STATE ( SETUP_ITER,	"setup_iter" ) \
	SPH_QUERY_STATE ( FULLSCAN,		"fullscan" ) \
	SPH_QUERY_STATE ( DICT_SETUP,	"dict_setup" ) \
	SPH_QUERY_STATE ( PARSE,		"parse" ) \
	SPH_QUERY_STATE ( TRANSFORMS,	"transforms" ) \
	SPH_QUERY_STATE ( INIT,			"init" ) \
	SPH_QUERY_STATE ( INIT_SEGMENT,	"init_segment" ) \
	SPH_QUERY_STATE ( OPEN,			"open" ) \
	SPH_QUERY_STATE ( READ_DOCS,	"read_docs" ) \
	SPH_QUERY_STATE ( READ_HITS,	"read_hits" ) \
	SPH_QUERY_STATE ( GET_DOCS,		"get_docs" ) \
	SPH_QUERY_STATE ( GET_HITS,		"get_hits" ) \
	SPH_QUERY_STATE ( FILTER,		"filter" ) \
	SPH_QUERY_STATE ( RANK,			"rank" ) \
	SPH_QUERY_STATE ( QCACHE_UP,	"qcache_update" ) \
	SPH_QUERY_STATE ( QCACHE_FINAL,	"qcache_final" ) \
	SPH_QUERY_STATE ( SORT,			"sort" ) \
	SPH_QUERY_STATE ( FINALIZE,		"finalize" ) \
	SPH_QUERY_STATE ( DYNAMIC,		"clone_attrs" ) \
	SPH_QUERY_STATE ( DIST_WAIT,	"dist_wait" ) \
	SPH_QUERY_STATE ( AGGREGATE,	"aggregate" ) \
	SPH_QUERY_STATE ( NET_WRITE,	"net_write" ) \
	SPH_QUERY_STATE ( EVAL_POST,	"eval_post" ) \
	SPH_QUERY_STATE ( EVAL_GETFIELD,"eval_getfield" ) \
	SPH_QUERY_STATE ( SNIPPET,		"eval_snippet" ) \
	SPH_QUERY_STATE ( EVAL_UDF,		"eval_udf" ) \
	SPH_QUERY_STATE ( TABLE_FUNC,	"table_func" )


/// possible query states, used for profiling
enum ESphQueryState
{
	SPH_QSTATE_INFINUM = -1,

#define SPH_QUERY_STATE(_name,_desc) SPH_QSTATE_##_name,
	SPH_QUERY_STATES
#undef SPH_QUERY_STATE

	SPH_QSTATE_TOTAL
};
STATIC_ASSERT ( SPH_QSTATE_UNKNOWN==0, BAD_QUERY_STATE_ENUM_BASE );

struct XQNode_t;

/// search query profile
class QueryProfile_c
{
public:
	ESphQueryState	m_eState;							///< current state
	int64_t			m_tmStamp;							///< timestamp when we entered the current state

	int				m_dSwitches [ SPH_QSTATE_TOTAL+1 ];	///< number of switches to given state
	int64_t			m_tmTotal [ SPH_QSTATE_TOTAL+1 ];	///< total time spent per state
	CSphVector<BYTE> m_dPlan; 							///< bson with plan

	int				m_iMaxMatches = 0;
	int				m_iPseudoShards = 1;
															/// create empty and stopped profile
					QueryProfile_c();
	virtual 		~QueryProfile_c() {}

	/// switch to a new query state, and record a timestamp
	/// returns previous state, to simplify Push/Pop like scenarios
	ESphQueryState Switch ( ESphQueryState eNew );

	/// reset everything and start profiling from a given state
	void			Start ( ESphQueryState eNew );
	/// stop profiling
	void			Stop();
	void			AddMetric ( const QueryProfile_c & tData );

	void			BuildResult ( XQNode_t * pRoot, const CSphSchema & tSchema, const StrVec_t & dZones );
};


class CSphScopedProfile
{
public:
						CSphScopedProfile ( QueryProfile_c * pProfile, ESphQueryState eNewState );
						~CSphScopedProfile();

private:
	QueryProfile_c *	m_pProfile;
	ESphQueryState		m_eOldState;
};


// acquire common pattern 'check, then switch if not null'
inline void SwitchProfile ( QueryProfile_c * pProfile, ESphQueryState eState )
{
	if ( pProfile )
		pProfile->Switch ( eState );
}


#endif // _queryprofile_
