//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include <array>
#include "sphinxstd.h"

namespace QueryStats {
	enum
	{
		INTERVAL_1MIN,
		INTERVAL_5MIN,
		INTERVAL_15MIN,
		INTERVAL_ALLTIME,

		INTERVAL_TOTAL
	};


	enum
	{
		TYPE_AVG,
		TYPE_MIN,
		TYPE_MAX,
		TYPE_95,
		TYPE_99,

		TYPE_TOTAL,
	};
}

struct QueryStatElement_t
{
	uint64_t	m_dData[QueryStats::TYPE_TOTAL] = { 0, UINT64_MAX, 0, 0, 0, };
	uint64_t	m_uTotalQueries = 0;
};


struct QueryStats_t
{
	QueryStatElement_t	m_dStats[QueryStats::INTERVAL_TOTAL];
};


struct QueryStatRecord_t
{
	uint64_t	m_uQueryTimeMin;
	uint64_t	m_uQueryTimeMax;
	uint64_t	m_uQueryTimeSum;
	uint64_t	m_uFoundRowsMin;
	uint64_t	m_uFoundRowsMax;
	uint64_t	m_uFoundRowsSum;

	uint64_t	m_uTimestamp;
	int			m_iCount;
};


class QueryStatContainer_i
{
public:
	virtual								~QueryStatContainer_i() {}
	virtual void						Add ( uint64_t uFoundRows, uint64_t uQueryTime, uint64_t uTimestamp ) = 0;
	virtual QueryStatRecord_t			GetRecord ( int iRecord ) const noexcept = 0;
	virtual int							GetNumRecords() const = 0;
};

struct GuardedQueryStatContainer_t
{
	mutable RwLock_t m_tStatsLock;
	std::unique_ptr<QueryStatContainer_i> m_tStats GUARDED_BY ( m_tStatsLock );
};

/// known commands
/// (shared here because at least SEARCHD_COMMAND_TOTAL used outside the core)
enum SearchdCommand_e : WORD
{
	SEARCHD_COMMAND_SEARCH		= 0,
	SEARCHD_COMMAND_EXCERPT		= 1,
	SEARCHD_COMMAND_UPDATE		= 2,
	SEARCHD_COMMAND_KEYWORDS	= 3,
	SEARCHD_COMMAND_PERSIST		= 4,
	SEARCHD_COMMAND_STATUS		= 5,
	SEARCHD_COMMAND_UNUSED_6	= 6,
	SEARCHD_COMMAND_FLUSHATTRS	= 7,
	SEARCHD_COMMAND_SPHINXQL	= 8,
	SEARCHD_COMMAND_PING		= 9,
	SEARCHD_COMMAND_DELETE		= 10,
	SEARCHD_COMMAND_UVAR		= 11,
	SEARCHD_COMMAND_INSERT		= 12,
	SEARCHD_COMMAND_REPLACE		= 13,
	SEARCHD_COMMAND_COMMIT		= 14,
	SEARCHD_COMMAND_SUGGEST		= 15,
	SEARCHD_COMMAND_JSON		= 16,
	SEARCHD_COMMAND_CALLPQ 		= 17,
	SEARCHD_COMMAND_CLUSTER		= 18,
	SEARCHD_COMMAND_GETFIELD	= 19,

	SEARCHD_COMMAND_TOTAL,
	SEARCHD_COMMAND_WRONG = SEARCHD_COMMAND_TOTAL,
};

class CommandStats_t
{
public:

	enum EDETAILS : BYTE { eUpdate, eReplace, eSearch, eTotal };

	CommandStats_t();
	virtual ~CommandStats_t() = default;

	void AddDetailed ( EDETAILS eCmd, uint64_t uFoundRows, uint64_t tmStart );
	void AddDeltaDetailed ( EDETAILS eCmd, uint64_t uFoundRows, uint64_t tmDelta );
	void CalcDetailed ( EDETAILS eCmd, QueryStats_t & tRowsFoundStats, QueryStats_t & tQueryTimeStats ) const;

	void Inc ( SearchdCommand_e eCmd );
	int64_t  Get ( SearchdCommand_e eCmd ) const;

private:
	std::array<std::atomic<int64_t>, SEARCHD_COMMAND_TOTAL>	m_iCommands;
	std::array<GuardedQueryStatContainer_t, eTotal> m_dDetailed;
};

std::unique_ptr<QueryStatContainer_i> MakeStatsContainer();
std::unique_ptr<QueryStatContainer_i> MakeExactStatsContainer();
void CalcSimpleStats ( const QueryStatContainer_i * pContainer, QueryStats_t & tRowsFoundStats, QueryStats_t & tQueryTimeStats );
