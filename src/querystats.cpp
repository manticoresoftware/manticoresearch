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

#include "querystats.h"

class QueryStatContainer_c: public QueryStatContainer_i
{
public:
	void Add( uint64_t uFoundRows, uint64_t uQueryTime, uint64_t uTimestamp ) final;
	QueryStatRecord_t GetRecord( int iRecord ) const noexcept final;
	int GetNumRecords() const final;

	QueryStatContainer_c();
	QueryStatContainer_c( QueryStatContainer_c&& tOther ) noexcept;
	void Swap( QueryStatContainer_c& rhs ) noexcept;
	QueryStatContainer_c& operator=( QueryStatContainer_c tOther ) noexcept;

private:
	CircularBuffer_T<QueryStatRecord_t> m_dRecords;
};

std::unique_ptr<QueryStatContainer_i> MakeStatsContainer ()
{
	return std::make_unique<QueryStatContainer_c>();
}

void QueryStatContainer_c::Add( uint64_t uFoundRows, uint64_t uQueryTime, uint64_t uTimestamp )
{
	if ( !m_dRecords.IsEmpty())
	{
		QueryStatRecord_t& tLast = m_dRecords.Last();
		const uint64_t BUCKET_TIME_DELTA = 100000;
		if ( uTimestamp - tLast.m_uTimestamp<=BUCKET_TIME_DELTA )
		{
			tLast.m_uFoundRowsMin = Min( uFoundRows, tLast.m_uFoundRowsMin );
			tLast.m_uFoundRowsMax = Max( uFoundRows, tLast.m_uFoundRowsMax );
			tLast.m_uFoundRowsSum += uFoundRows;

			tLast.m_uQueryTimeMin = Min( uQueryTime, tLast.m_uQueryTimeMin );
			tLast.m_uQueryTimeMax = Max( uQueryTime, tLast.m_uQueryTimeMax );
			tLast.m_uQueryTimeSum += uQueryTime;

			tLast.m_iCount++;

			return;
		}
	}

	const uint64_t MAX_TIME_DELTA = 15 * 60 * 1000000;
	while ( !m_dRecords.IsEmpty() && ( uTimestamp - m_dRecords[0].m_uTimestamp )>MAX_TIME_DELTA )
		m_dRecords.Pop();

	QueryStatRecord_t& tRecord = m_dRecords.Push();
	tRecord.m_uFoundRowsMin = uFoundRows;
	tRecord.m_uFoundRowsMax = uFoundRows;
	tRecord.m_uFoundRowsSum = uFoundRows;

	tRecord.m_uQueryTimeMin = uQueryTime;
	tRecord.m_uQueryTimeMax = uQueryTime;
	tRecord.m_uQueryTimeSum = uQueryTime;

	tRecord.m_uTimestamp = uTimestamp;
	tRecord.m_iCount = 1;
}

QueryStatRecord_t QueryStatContainer_c::GetRecord ( int iRecord ) const noexcept
{
	return m_dRecords[iRecord];
}


int QueryStatContainer_c::GetNumRecords() const
{
	return m_dRecords.GetLength();
}

QueryStatContainer_c::QueryStatContainer_c() = default;

QueryStatContainer_c::QueryStatContainer_c( QueryStatContainer_c&& tOther ) noexcept
	: QueryStatContainer_c()
{ Swap( tOther ); }

void QueryStatContainer_c::Swap( QueryStatContainer_c& rhs ) noexcept
{
	rhs.m_dRecords.Swap( m_dRecords );
}

QueryStatContainer_c& QueryStatContainer_c::operator=( QueryStatContainer_c tOther ) noexcept
{
	Swap( tOther );
	return *this;
}

//////////////////////////////////////////////////////////////////////////

class QueryStatContainerExact_c: public QueryStatContainer_i
{
public:
	void Add( uint64_t uFoundRows, uint64_t uQueryTime, uint64_t uTimestamp ) final;
	QueryStatRecord_t GetRecord( int iRecord ) const noexcept final;
	int GetNumRecords() const final;

	QueryStatContainerExact_c();
	QueryStatContainerExact_c( QueryStatContainerExact_c&& tOther ) noexcept;
	void Swap( QueryStatContainerExact_c& rhs ) noexcept;
	QueryStatContainerExact_c& operator=( QueryStatContainerExact_c tOther ) noexcept;

private:
	struct QueryStatRecordExact_t
	{
		uint64_t m_uQueryTime;
		uint64_t m_uFoundRows;
		uint64_t m_uTimestamp;
	};

	CircularBuffer_T<QueryStatRecordExact_t> m_dRecords;
};

void QueryStatContainerExact_c::Add( uint64_t uFoundRows, uint64_t uQueryTime, uint64_t uTimestamp )
{
	const uint64_t MAX_TIME_DELTA = 15 * 60 * 1000000;
	while ( !m_dRecords.IsEmpty() && ( uTimestamp - m_dRecords[0].m_uTimestamp )>MAX_TIME_DELTA )
		m_dRecords.Pop();

	QueryStatRecordExact_t& tRecord = m_dRecords.Push();
	tRecord.m_uFoundRows = uFoundRows;
	tRecord.m_uQueryTime = uQueryTime;
	tRecord.m_uTimestamp = uTimestamp;
}


int QueryStatContainerExact_c::GetNumRecords() const
{
	return m_dRecords.GetLength();
}


QueryStatRecord_t QueryStatContainerExact_c::GetRecord ( int iRecord ) const noexcept
{
	QueryStatRecord_t tRecord;
	const QueryStatRecordExact_t& tExact = m_dRecords[iRecord];

	tRecord.m_uQueryTimeMin = tExact.m_uQueryTime;
	tRecord.m_uQueryTimeMax = tExact.m_uQueryTime;
	tRecord.m_uQueryTimeSum = tExact.m_uQueryTime;
	tRecord.m_uFoundRowsMin = tExact.m_uFoundRows;
	tRecord.m_uFoundRowsMax = tExact.m_uFoundRows;
	tRecord.m_uFoundRowsSum = tExact.m_uFoundRows;

	tRecord.m_uTimestamp = tExact.m_uTimestamp;
	tRecord.m_iCount = 1;
	return tRecord;
}

QueryStatContainerExact_c::QueryStatContainerExact_c() = default;

QueryStatContainerExact_c::QueryStatContainerExact_c( QueryStatContainerExact_c&& tOther ) noexcept
	: QueryStatContainerExact_c()
{ Swap( tOther ); }

void QueryStatContainerExact_c::Swap( QueryStatContainerExact_c& rhs ) noexcept
{
	rhs.m_dRecords.Swap( m_dRecords );
}

QueryStatContainerExact_c& QueryStatContainerExact_c::operator=( QueryStatContainerExact_c tOther ) noexcept
{
	Swap( tOther );
	return *this;
}

std::unique_ptr<QueryStatContainer_i> MakeExactStatsContainer()
{
	return std::make_unique<QueryStatContainerExact_c>();
}

CommandStats_t::CommandStats_t()
{
	for ( auto & i : m_iCommands )
		i = 0;

	for ( auto & i: m_dDetailed )
		i.m_tStats = MakeStatsContainer();
}

void CommandStats_t::AddDetailed ( EDETAILS eCmd, uint64_t uFoundRows, uint64_t tmStart )
{
	auto tmNow = sphMicroTimer ();
	auto & tDetail = m_dDetailed[eCmd];
	ScWL_t wLock ( tDetail.m_tStatsLock );
	tDetail.m_tStats->Add ( uFoundRows, tmNow-tmStart, tmNow );
}

void CommandStats_t::AddDeltaDetailed ( EDETAILS eCmd, uint64_t uFoundRows, uint64_t tmDelta )
{
	auto tmNow = sphMicroTimer ();
	auto & tDetail = m_dDetailed[eCmd];
	ScWL_t wLock ( tDetail.m_tStatsLock );
	tDetail.m_tStats->Add ( uFoundRows, tmDelta, tmNow );
}

void CommandStats_t::CalcDetailed ( EDETAILS eCmd, QueryStats_t & tRowsFoundStats, QueryStats_t & tQueryTimeStats ) const
{
	const auto & tDetail = m_dDetailed[eCmd];
	ScRL_t rLock ( tDetail.m_tStatsLock );
	CalcSimpleStats ( tDetail.m_tStats.get (), tRowsFoundStats, tQueryTimeStats );
}

void  CommandStats_t::Inc ( SearchdCommand_e eCmd )
{
	if ( eCmd<SEARCHD_COMMAND_TOTAL )
		m_iCommands[eCmd].fetch_add ( 1, std::memory_order_relaxed );
}

int64_t CommandStats_t::Get ( SearchdCommand_e eCmd ) const
{
	return ( ( eCmd<SEARCHD_COMMAND_TOTAL ) ? m_iCommands[eCmd].load ( std::memory_order_relaxed ) : 0 );
}
