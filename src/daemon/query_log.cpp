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

#include "logger.h"
#include "fileutils.h"
#include "searchdaemon.h"
#include "client_session.h"

#include <sys/stat.h>

// for USE_SYSLOG
#include "config.h"
#if USE_SYSLOG
#include <syslog.h>
#endif

constexpr int LOG_COMPACT_IN = 128; // upto this many IN(..) values allowed in query_log

static bool g_bQuerySyslog = false;
static int g_iQueryLogFile = -1;
static CSphString g_sQueryLogFile;
static LOG_FORMAT g_eLogFormat = LOG_FORMAT::SPHINXQL;
static bool g_bLogCompactIn = false; // whether to cut list in IN() clauses.
extern int g_iQueryLogMinMs; // log 'slow' threshold for query, defined in searchd.cpp

static bool g_bLogIOStats = false;
static bool g_bLogCPUStats = false;
static CSphBitvec g_tLogStatements;



bool SlowTimeExceeded ( int iTimeMs ) noexcept
{
	return !g_iQueryLogMinMs || iTimeMs > g_iQueryLogMinMs;
}


void SetLogFormat ( LOG_FORMAT eFormat ) noexcept
{
	g_eLogFormat = eFormat;
}


LOG_FORMAT LogFormat () noexcept
{
	return g_eLogFormat;
}


void SetLogCompact ( bool bCompact ) noexcept
{
	g_bLogCompactIn = bCompact;
}


void SetLogStatsFlags ( bool bIOStats, bool bCPUStats ) noexcept
{
	g_bLogIOStats = bIOStats;
	g_bLogCPUStats = bCPUStats;
}


void SetupQueryLogDrain ( CSphString sQueryLog )
{
	if ( sQueryLog == "syslog" )
		g_bQuerySyslog = true;
	else
	{
		FixPathAbsolute ( sQueryLog );
		g_iQueryLogFile = open ( sQueryLog.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
		if ( g_iQueryLogFile < 0 )
			sphFatal ( "failed to open query log file '%s': %s", sQueryLog.cstr(), strerrorm ( errno ) );

		ChangeLogFileMode ( g_iQueryLogFile );
	}
	g_sQueryLogFile = std::move ( sQueryLog );
}


void ReopenQueryLog ()
{
	if ( g_bQuerySyslog || g_iQueryLogFile == GetDaemonLogFD() || g_iQueryLogFile < 0 || isatty ( g_iQueryLogFile ) )
		return;

	int iFD = ::open ( g_sQueryLogFile.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
	if ( iFD < 0 )
	{
		sphWarning ( "failed to reopen query log file '%s': %s", g_sQueryLogFile.cstr(), strerrorm(errno) );
	} else
	{
		::close ( g_iQueryLogFile );
		g_iQueryLogFile = iFD;
		ChangeLogFileMode ( g_iQueryLogFile );
		sphInfo ( "query log reopened" );
	}
}


void RedirectQueryLogToDaemonLog ()
{
	g_bQuerySyslog = false;
	DisableLogSyslog();
	g_iQueryLogFile = GetDaemonLogFD();
}


void OpenSyslogIfNecessary ()
{
#if USE_SYSLOG
	if ( LogSyslogEnabled() || g_bQuerySyslog )
	{
		openlog ( "searchd", LOG_PID, LOG_DAEMON );
	}
#else
	if ( g_bQuerySyslog )
		sphFatal ( "Wrong query_log file! You have to reconfigure --with-syslog and rebuild daemon if you want to use syslog there." );
#endif
}

static void WriteQuery ( const StringBuilder_c & tBuf )
{
	sphSeek ( g_iQueryLogFile, 0, SEEK_END );
	sphWrite ( g_iQueryLogFile, tBuf.cstr(), tBuf.GetLength() );
}

static void LogQueryPlain ( const CSphQuery & tQuery, const CSphQueryResultMeta & tMeta )
{
	assert ( g_eLogFormat==LOG_FORMAT::_PLAIN );
	if ( (!g_bQuerySyslog && g_iQueryLogFile < 0) || !tMeta.m_sError.IsEmpty() )
		return;

	QuotationEscapedBuilder tBuf;

	// [time]
#if USE_SYSLOG
	if ( !g_bQuerySyslog )
	{
#endif

		tBuf << '[';
		sphFormatCurrentTime ( tBuf );
		tBuf << ']';

#if USE_SYSLOG
	} else
		tBuf += "[query]";
#endif

	// querytime sec
	int iQueryTime = Max ( tMeta.m_iQueryTime, 0 );
	int iRealTime = Max ( tMeta.m_iRealQueryTime, 0 );
	tBuf.Appendf ( " %d.%03d sec", iRealTime / 1000, iRealTime % 1000 );
	tBuf.Appendf ( " %d.%03d sec", iQueryTime / 1000, iQueryTime % 1000 );

	// optional multi-query multiplier
	if ( tMeta.m_iMultiplier > 1 )
		tBuf.Appendf ( " x%d", tMeta.m_iMultiplier );

	// [matchmode/numfilters/sortmode matches (offset,limit)
	constexpr const char * sModes[SPH_MATCH_TOTAL] = { "all", "any", "phr", "bool", "ext", "scan", "ext2" };
	constexpr const char * sSort[SPH_SORT_TOTAL] = { "rel", "attr-", "attr+", "tsegs", "ext", "expr" };
	tBuf.Appendf ( " [%s/%d/%s " INT64_FMT " (%d,%d)",
	               sModes[tQuery.m_eMode], tQuery.m_dFilters.GetLength(), sSort[tQuery.m_eSort], tMeta.m_iTotalMatches,
	               tQuery.m_iOffset, tQuery.m_iLimit );

	// optional groupby info
	if ( !tQuery.m_sGroupBy.IsEmpty() )
		tBuf.Appendf ( " @%s", tQuery.m_sGroupBy.cstr() );

	// ] [indexes]
	tBuf.Appendf ( "] [%s]", tQuery.m_sIndexes.cstr() );

	// optional performance counters
	if ( g_bLogIOStats || g_bLogCPUStats )
	{
		const CSphIOStats & IOStats = tMeta.m_tIOStats;

		tBuf += " [";

		if ( g_bLogIOStats )
			tBuf.Appendf ( "ios=%d kb=%d.%d ioms=%d.%d",
			               IOStats.m_iReadOps, (int) (IOStats.m_iReadBytes / 1024), (int) (IOStats.m_iReadBytes % 1024) * 10 / 1024,
			               (int) (IOStats.m_iReadTime / 1000), (int) (IOStats.m_iReadTime % 1000) / 100 );

		if ( g_bLogIOStats && g_bLogCPUStats )
			tBuf += " ";

		if ( g_bLogCPUStats )
			tBuf.Sprintf ( "cpums=%.1D", tMeta.m_iCpuTime / 100 );

		tBuf += "]";
	}

	// optional query comment
	if ( !tQuery.m_sComment.IsEmpty() )
		tBuf.Appendf ( " [%s]", tQuery.m_sComment.cstr() );

	// query
	// (m_sRawQuery is empty when using MySQL handler)
	const CSphString & sQuery = tQuery.m_sRawQuery.IsEmpty()
			? tQuery.m_sQuery
			: tQuery.m_sRawQuery;

	if ( !sQuery.IsEmpty() )
	{
		tBuf += " ";
		tBuf.FixupSpacesAndAppend ( sQuery.cstr() );
	}

#if USE_SYSLOG
	if ( !g_bQuerySyslog )
	{
#endif

		// line feed
		tBuf += "\n";

		WriteQuery ( tBuf );
#if USE_SYSLOG
	} else
	{
		syslog ( LOG_INFO, "%s", tBuf.cstr() );
	}
#endif
}


namespace {
CSphString RemoveBackQuotes ( const char * pSrc )
{
	CSphString sResult;
	if ( !pSrc )
		return sResult;

	size_t iLen = strlen ( pSrc );
	if ( !iLen )
		return sResult;

	auto szResult = new char[iLen + 1];

	auto * sMax = pSrc + iLen;
	auto d = szResult;
	while ( pSrc < sMax )
	{
		auto sQuote = (const char *) memchr ( pSrc, '`', sMax - pSrc );
		if ( !sQuote )
			sQuote = sMax;
		auto iChunk = sQuote - pSrc;
		memmove ( d, pSrc, iChunk );
		d += iChunk;
		pSrc += iChunk + 1; // +1 to skip the quote
	}
	*d = '\0';
	if ( !*szResult ) // never return allocated, but empty str. Prefer to return nullptr instead.
		SafeDeleteArray ( szResult );
	sResult.Adopt ( &szResult );
	return sResult;
}
}


static void FormatOrderBy ( StringBuilder_c * pBuf, const char * sPrefix, ESphSortOrder eSort, const CSphString & sSort )
{
	assert ( pBuf );
	if ( eSort == SPH_SORT_EXTENDED && sSort == "@weight desc" )
		return;

	const char * sSubst = "@weight";
	if ( sSort != "@relevance" )
		sSubst = sSort.cstr();

	auto sUnquoted = RemoveBackQuotes ( sSubst );
	sSubst = sUnquoted.cstr();

	// for simplicity check that sPrefix is already prefixed/suffixed by spaces.
	assert ( sPrefix && sPrefix[0]==' ' && sPrefix[strlen ( sPrefix )-1]==' ' );
	*pBuf << sPrefix;
	switch ( eSort )
	{
	case SPH_SORT_TIME_SEGMENTS: *pBuf << "TIME_SEGMENT(" << sSubst << ")";
		break;
	case SPH_SORT_EXTENDED: *pBuf << sSubst;
		break;
	case SPH_SORT_EXPR: *pBuf << "BUILTIN_EXPR()";
		break;
	case SPH_SORT_RELEVANCE: *pBuf << "weight() desc";
		if ( sSubst ) *pBuf << ", " << sSubst;
		break;
	default: pBuf->Appendf ( "mode-%d", (int) eSort );
		break;
	}
}


static const CSphQuery g_tDefaultQuery {};

static void FormatOption ( const CSphQuery & tQuery, StringBuilder_c & tBuf, const char * szOption )
{
	ScopedComma_c tOptionComma ( tBuf, ", ", szOption );

	if ( tQuery.m_iMaxMatches != DEFAULT_MAX_MATCHES )
		tBuf.Appendf ( "max_matches=%d", tQuery.m_iMaxMatches );

	if ( !tQuery.m_sComment.IsEmpty() )
		tBuf.Appendf ( "comment='%s'", tQuery.m_sComment.cstr() ); // FIXME! escape, replace newlines..

	if ( tQuery.m_eRanker != SPH_RANK_DEFAULT )
	{
		const char * sRanker = sphGetRankerName ( tQuery.m_eRanker );
		if ( !sRanker )
			sRanker = sphGetRankerName ( SPH_RANK_DEFAULT );

		if ( tQuery.m_sRankerExpr.IsEmpty() )
			tBuf.Appendf ( "ranker=%s", sRanker );
		else
			tBuf.Appendf ( "ranker=%s(\'%s\')", sRanker, tQuery.m_sRankerExpr.scstr() );
	}

	if ( tQuery.m_iAgentQueryTimeoutMs != DEFAULT_QUERY_TIMEOUT )
		tBuf.Appendf ( "agent_query_timeout=%d", tQuery.m_iAgentQueryTimeoutMs );

	if ( tQuery.m_iCutoff != g_tDefaultQuery.m_iCutoff )
		tBuf.Appendf ( "cutoff=%d", tQuery.m_iCutoff );

	if ( !tQuery.m_dFieldWeights.IsEmpty() )
	{
		tBuf.StartBlock ( nullptr, "field_weights=(", ")" );
		tBuf << Vec2Str ( tQuery.m_dFieldWeights, ", " );
		tBuf.FinishBlock();
	}

	if ( tQuery.m_bGlobalIDF != g_tDefaultQuery.m_bGlobalIDF )
		tBuf << "global_idf=1";

	if ( tQuery.m_bPlainIDF || !tQuery.m_bNormalizedTFIDF )
	{
		tBuf.StartBlock ( ",", "idf='", "'" );
		tBuf << (tQuery.m_bPlainIDF ? "plain" : "normalized")
				<< (tQuery.m_bNormalizedTFIDF ? "tfidf_normalized" : "tfidf_unnormalized");
		tBuf.FinishBlock();
	}

	if ( tQuery.m_bLocalDF.has_value() )
		tBuf.Appendf ( "local_df=%d", tQuery.m_bLocalDF.value() ? 1 : 0 );

	if ( !tQuery.m_dIndexWeights.IsEmpty() )
	{
		tBuf.StartBlock ( nullptr, "index_weights=(", ")" );
		tBuf << Vec2Str ( tQuery.m_dIndexWeights, ", " );
		tBuf.FinishBlock();
	}

	if ( tQuery.m_uMaxQueryMsec != g_tDefaultQuery.m_uMaxQueryMsec )
		tBuf.Appendf ( "max_query_time=%u", tQuery.m_uMaxQueryMsec );

	if ( tQuery.m_iMaxPredictedMsec != g_tDefaultQuery.m_iMaxPredictedMsec )
		tBuf.Appendf ( "max_predicted_time=%d", tQuery.m_iMaxPredictedMsec );

	if ( tQuery.m_iRetryCount != DEFAULT_QUERY_RETRY )
		tBuf.Appendf ( "retry_count=%d", tQuery.m_iRetryCount );

	if ( tQuery.m_iRetryDelay != DEFAULT_QUERY_RETRY )
		tBuf.Appendf ( "retry_delay=%d", tQuery.m_iRetryDelay );

	if ( tQuery.m_iRandSeed != g_tDefaultQuery.m_iRandSeed )
		tBuf.Appendf ( "rand_seed=" INT64_FMT, tQuery.m_iRandSeed );

	if ( !tQuery.m_sQueryTokenFilterLib.IsEmpty() )
	{
		if ( tQuery.m_sQueryTokenFilterOpts.IsEmpty() )
			tBuf.Appendf ( "token_filter = '%s:%s'", tQuery.m_sQueryTokenFilterLib.cstr(), tQuery.m_sQueryTokenFilterName.cstr() );
		else
			tBuf.Appendf ( "token_filter = '%s:%s:%s'", tQuery.m_sQueryTokenFilterLib.cstr(), tQuery.m_sQueryTokenFilterName.cstr(), tQuery.m_sQueryTokenFilterOpts.cstr() );
	}

	if ( tQuery.m_bIgnoreNonexistent )
		tBuf << "ignore_nonexistent_columns=1";

	if ( tQuery.m_bIgnoreNonexistentIndexes )
		tBuf << "ignore_nonexistent_indexes=1";

	if ( tQuery.m_bStrict )
		tBuf << "strict=1";

	if ( tQuery.m_eExpandKeywords != QUERY_OPT_DEFAULT && tQuery.m_eExpandKeywords != QUERY_OPT_MORPH_NONE )
		tBuf.Appendf ( "expand_keywords=%d", (tQuery.m_eExpandKeywords == QUERY_OPT_ENABLED ? 1 : 0) );
	if ( tQuery.m_eExpandKeywords == QUERY_OPT_MORPH_NONE )
		tBuf.Appendf ( "morphology=none" );
	if ( tQuery.m_iExpansionLimit != DEFAULT_QUERY_EXPANSION_LIMIT )
		tBuf.Appendf ( "expansion_limit=%d", tQuery.m_iExpansionLimit );
}


static const char* GetHintName ( const IndexHint_t & tHint )
{
	switch ( tHint.m_eType )
	{
	case SecondaryIndexType_e::FILTER: return "Filter";
	case SecondaryIndexType_e::LOOKUP: return "DocidIndex";
	case SecondaryIndexType_e::INDEX: return "SecondaryIndex";
	case SecondaryIndexType_e::ANALYZER: return "ColumnarScan";
	default: return "None";
	}
}


static void AppendHint ( const IndexHint_t & tHint, const StrVec_t & dIndexes, StringBuilder_c & tBuf )
{
	auto sName = SphSprintf ( " %s%s (", (tHint.m_bForce?"":"NO_"), GetHintName ( tHint ) );
	ScopedComma_c tComma ( tBuf, ",", sName.cstr(), ")" );
	for ( const auto & sIndex: dIndexes )
		tBuf << sIndex;
}


static void FormatIndexHints ( const CSphQuery & tQuery, StringBuilder_c & tBuf )
{
	if ( !tQuery.m_dIndexHints.GetLength() )
		return;

	ScopedComma_c sMatch ( tBuf, nullptr );
	CSphVector<bool> dUsed { tQuery.m_dIndexHints.GetLength() };
	dUsed.ZeroVec();

	tBuf << " /*+ ";

	ARRAY_FOREACH ( i, tQuery.m_dIndexHints )
	{
		if ( dUsed[i] )
			continue;

		StrVec_t dIndexes;
		dIndexes.Add ( tQuery.m_dIndexHints[i].m_sIndex );
		for ( int j = i + 1; j < tQuery.m_dIndexHints.GetLength(); j++ )
			if ( !dUsed[j] && tQuery.m_dIndexHints[i].m_eType == tQuery.m_dIndexHints[j].m_eType && tQuery.m_dIndexHints[i].m_bForce == tQuery.m_dIndexHints[j].m_bForce )
			{
				dIndexes.Add ( tQuery.m_dIndexHints[j].m_sIndex );
				dUsed[j] = true;
			}

		AppendHint ( tQuery.m_dIndexHints[i], dIndexes, tBuf );
	}

	tBuf << " */";
}


static void LogQueryJson ( const CSphQuery & q, StringBuilder_c & tBuf )
{
	if ( q.m_sRawQuery.IsEmpty() )
		tBuf << " /*" << "{\"index\":\"" << q.m_sIndexes << "\"}*/ /*" << q.m_sQuery << " */";
	else
		tBuf << " /*" << q.m_sRawQuery << " */";
}


static void FormatTimeConnClient ( StringBuilder_c & tBuf )
{
	sphFormatCurrentTime ( tBuf );
	tBuf << " conn " << session::GetConnID() << " (" << session::szClientName() << ")";
}


static const char* TypeCastToStr ( ESphAttr eAttr )
{
	switch ( eAttr )
	{
	case SPH_ATTR_INTEGER: return "INTEGER";
	case SPH_ATTR_BIGINT: return "BIGINT";
	case SPH_ATTR_STRING: return "STRING";
	default: return nullptr;
	}
}

void FormatSphinxql ( const CSphQuery & q, const CSphQuery & tJoinOptions, int iCompactIN, QuotationEscapedBuilder & tBuf )
{
	if ( q.m_bHasOuter )
		tBuf << "SELECT * FROM (";

	if ( q.m_sSelect.IsEmpty() )
		tBuf << "SELECT * FROM " << q.m_sIndexes;
	else
		tBuf << "SELECT " << RemoveBackQuotes ( q.m_sSelect.cstr() ) << " FROM " << q.m_sIndexes;

	if ( q.m_eJoinType != JoinType_e::NONE )
	{
		tBuf << (q.m_eJoinType == JoinType_e::LEFT ? " LEFT JOIN " : " INNER JOIN ")
			<< q.m_sJoinIdx << " ON ";

		ARRAY_FOREACH ( i, q.m_dOnFilters )
		{
			const auto & tOn = q.m_dOnFilters[i];

			const char* szCast1 = TypeCastToStr ( tOn.m_eTypeCast1 );
			if ( szCast1 )
				tBuf.Sprint ( szCast1, '(', tOn.m_sIdx1, '.', tOn.m_sAttr1, ')' );
			else
				tBuf.Sprint ( tOn.m_sIdx1, '.', tOn.m_sAttr1 );
			tBuf << '=';

			const char* szCast2 = TypeCastToStr ( tOn.m_eTypeCast2 );
			if ( szCast2 )
				tBuf.Sprint ( szCast2, '(', tOn.m_sIdx2, '.', tOn.m_sAttr2, ')' );
			else
				tBuf.Sprint ( tOn.m_sIdx2, '.', tOn.m_sAttr2 );

			if ( i < q.m_dOnFilters.GetLength() - 1 )
				tBuf << " AND ";
		}
	}

	// WHERE clause
	// (m_sRawQuery is empty when using MySQL handler)
	const CSphString & sQuery = q.m_sQuery;
	if ( !sQuery.IsEmpty() || !q.m_sJoinQuery.IsEmpty() || q.m_dFilters.GetLength() )
	{
		ScopedComma_c sWHERE ( tBuf, " AND ", " WHERE " );

		if ( !sQuery.IsEmpty() )
		{
			ScopedComma_c sMatch ( tBuf, nullptr, "MATCH(", ")" );
			tBuf.FixupSpacedAndAppendEscaped ( sQuery.cstr() );
		}

		if ( !q.m_sJoinQuery.IsEmpty() )
		{
			CSphString sEnd;
			sEnd.SetSprintf ( ",%s)", q.m_sJoinIdx.cstr() );
			ScopedComma_c sMatch ( tBuf, nullptr, "MATCH(", sEnd.cstr() );
			tBuf.FixupSpacedAndAppendEscaped ( q.m_sJoinQuery.cstr() );
		}

		FormatFiltersQL ( q.m_dFilters, q.m_dFilterTree, tBuf, iCompactIN );
	}

	// ORDER BY and/or GROUP BY clause
	if ( q.m_sGroupBy.IsEmpty() )
	{
		if ( !q.m_sSortBy.IsEmpty() ) // case API SPH_MATCH_EXTENDED2 - SPH_SORT_RELEVANCE
			FormatOrderBy ( &tBuf, " ORDER BY ", q.m_eSort, q.m_sSortBy );
	} else
	{
		tBuf << " GROUP BY " << q.m_sGroupBy;
		FormatOrderBy ( &tBuf, " WITHIN GROUP ORDER BY ", q.m_eSort, q.m_sSortBy );
		if ( !q.m_tHaving.m_sAttrName.IsEmpty() )
		{
			ScopedComma_c sHawing ( tBuf, nullptr, " HAVING " );
			FormatFilterQL ( q.m_tHaving, tBuf, iCompactIN );
		}
		if ( q.m_sGroupSortBy != "@group desc" )
			FormatOrderBy ( &tBuf, " ORDER BY ", SPH_SORT_EXTENDED, q.m_sGroupSortBy );
	}

	// LIMIT clause
	if ( q.m_iOffset != 0 || q.m_iLimit != 20 )
		tBuf << " LIMIT ";
	if ( q.m_iOffset )
		tBuf << q.m_iOffset << ',';
	if ( q.m_iLimit != 20 )
		tBuf << q.m_iLimit;

	// OPTION clause
	FormatOption ( q, tBuf, " OPTION " );
	if ( q.m_eJoinType != JoinType_e::NONE )
	{
		CSphString sJoinedOption;
		sJoinedOption.SetSprintf ( " OPTION(%s) ", q.m_sJoinIdx.cstr() );
		FormatOption ( tJoinOptions, tBuf, sJoinedOption.cstr() );
	}

	FormatIndexHints ( q, tBuf );

	// outer order by, limit
	if ( q.m_bHasOuter )
	{
		tBuf << ')';
		if ( !q.m_sOuterOrderBy.IsEmpty() )
			tBuf << " ORDER BY " << q.m_sOuterOrderBy;
		if ( q.m_iOuterOffset > 0 )
			tBuf << " LIMIT " << q.m_iOuterOffset << ", " << q.m_iOuterLimit;
		else if ( q.m_iOuterLimit > 0 )
			tBuf << " LIMIT " << q.m_iOuterLimit;
	}

	// finish SQL statement
	tBuf << ';';
}


static void LogQuerySphinxql ( const CSphQuery & q, const CSphQuery & tJoinOptions, const CSphQueryResultMeta & tMeta, const CSphVector<int64_t> & dAgentTimes )
{
	assert ( g_eLogFormat==LOG_FORMAT::SPHINXQL );
	if ( g_iQueryLogFile < 0 )
		return;

	QuotationEscapedBuilder tBuf;
	int iCompactIN = (g_bLogCompactIn ? LOG_COMPACT_IN : 0);

	// time, conn id, wall, found
	int iQueryTime = Max ( tMeta.m_iQueryTime, 0 );
	int iRealTime = Max ( tMeta.m_iRealQueryTime, 0 );

	tBuf << "/* ";
	FormatTimeConnClient ( tBuf );
	tBuf << " real " << FixedFrac ( iRealTime ) << " wall " << FixedFrac ( iQueryTime );

	if ( tMeta.m_iMultiplier > 1 )
		tBuf << " x" << tMeta.m_iMultiplier;
	tBuf << " found " << tMeta.m_iTotalMatches << " */ ";

	///////////////////////////////////
	// format request as SELECT query
	///////////////////////////////////

	if ( q.m_eQueryType == QUERY_JSON )
		LogQueryJson ( q, tBuf );
	else
		FormatSphinxql ( q, tJoinOptions, iCompactIN, tBuf );

	///////////////
	// query stats
	///////////////

	// next block ecnlosed in /* .. */, space-separated
	tBuf.StartBlock ( " ", " /*", " */" );
	if ( !tMeta.m_sError.IsEmpty() )
	{
		// all we have is an error
		tBuf.Appendf ( "error=%s", tMeta.m_sError.cstr() );
	} else
	{
		// performance counters
		if ( g_bLogIOStats || g_bLogCPUStats )
		{
			const CSphIOStats & IOStats = tMeta.m_tIOStats;

			if ( g_bLogIOStats )
				tBuf.Sprintf ( "ios=%d kb=%d.%d ioms=%.1D",
				               IOStats.m_iReadOps, (int) (IOStats.m_iReadBytes / 1024), (int) (IOStats.m_iReadBytes % 1024) * 10 / 1024,
				               IOStats.m_iReadTime / 100 );

			if ( g_bLogCPUStats )
				tBuf.Sprintf ( "cpums=%.1D", tMeta.m_iCpuTime / 100 );
		}

		// per-agent times
		if ( dAgentTimes.GetLength() )
		{
			ScopedComma_c dAgents ( tBuf, ", ", " agents=(", ")" );
			for ( auto iTime: dAgentTimes )
				tBuf.Appendf ( "%d.%03d",
				               (int) (iTime / 1000),
				               (int) (iTime % 1000) );
		}

		// merged stats
		if ( tMeta.m_hWordStats.GetLength() && (tMeta.m_tExpansionStats.m_iTerms || tMeta.m_tExpansionStats.m_iMerged) )
			tBuf.Appendf ( "terms expansion=(merged %d, not merged %d)", tMeta.m_tExpansionStats.m_iMerged, tMeta.m_tExpansionStats.m_iTerms );

		// warning
		if ( !tMeta.m_sWarning.IsEmpty() )
			tBuf.Appendf ( "warning=%s", tMeta.m_sWarning.cstr() );
	}
	tBuf.FinishBlock(); // close the comment

	// line feed
	tBuf += "\n";

	WriteQuery ( tBuf );
}


void LogQuery ( const CSphQuery & q, const CSphQuery & tJoinOptions, const CSphQueryResultMeta & tMeta, const CSphVector<int64_t> & dAgentTimes )
{
	if ( g_iQueryLogMinMs > 0 && tMeta.m_iQueryTime < g_iQueryLogMinMs )
		return;
	// should not log query from buddy in the info but only in debug and more verbose
	bool bNoLogQuery = ((q.m_uDebugFlags & QUERY_DEBUG_NO_LOG) == QUERY_DEBUG_NO_LOG);
	if ( bNoLogQuery && g_eLogLevel == SPH_LOG_INFO )
		return;

	switch ( g_eLogFormat )
	{
	case LOG_FORMAT::_PLAIN: LogQueryPlain ( q, tMeta );
		break;
	case LOG_FORMAT::SPHINXQL: LogQuerySphinxql ( q, tJoinOptions, tMeta, dAgentTimes );
		break;
	}
}

void LogSphinxqlError ( const char * sStmt, const Str_t & sError )
{
	if ( g_eLogFormat != LOG_FORMAT::SPHINXQL || g_iQueryLogFile < 0 || !sStmt || IsEmpty ( sError ) )
		return;

	// some mysql cli, like mysql 9.0.1, 9.1.0, 9.3.0, may be others, fire 'select $$' query after connect
	// that produces some noise in query log, so let's just filter out these queries. #2772
	if ( !strcmp (sStmt, "select $$") )
		return;

	StringBuilder_c tBuf;
	tBuf << "/* ";
	FormatTimeConnClient ( tBuf );
	tBuf << " */ " << sStmt << " # error=" << sError << '\n';

	WriteQuery ( tBuf );
}


void LogSphinxqlError ( const Str_t & sStmt, const Str_t & sError )
{
	if ( g_eLogFormat != LOG_FORMAT::SPHINXQL || g_iQueryLogFile < 0 || IsEmpty ( sStmt ) || IsEmpty ( sError ) )
		return;

	QuotationEscapedBuilder tBuf;

	tBuf << "/* ";
	FormatTimeConnClient ( tBuf );
	tBuf << " */ ";
	tBuf.AppendEscaped ( sStmt.first, EscBld::eFixupSpace, sStmt.second );
	tBuf << " # error=" << sError << '\n';

	WriteQuery ( tBuf );
}


void LogBuddyQuery ( Str_t sQuery, BuddyQuery_e tType )
{
	if ( g_eLogFormat != LOG_FORMAT::SPHINXQL || g_iQueryLogFile < 0 || IsEmpty ( sQuery ) )
		return;

	const auto & tMeta = session::GetClientSession()->m_tLastMeta;

	QuotationEscapedBuilder tBuf;

	// time, conn id, wall, found
	int iQueryTime = Max ( tMeta.m_iQueryTime, 0 );
	int iRealTime = Max ( tMeta.m_iRealQueryTime, 0 );

	tBuf << "/* ";
	FormatTimeConnClient ( tBuf );
	tBuf << " real " << FixedFrac ( iRealTime ) << " wall " << FixedFrac ( iQueryTime );

	if ( tMeta.m_iMultiplier > 1 )
		tBuf << " x" << tMeta.m_iMultiplier;
	tBuf << " found " << tMeta.m_iTotalMatches << " */ ";

	if ( tType == BuddyQuery_e::HTTP )
		tBuf << "/* ";
	tBuf.AppendEscaped ( sQuery.first, EscBld::eFixupSpace, sQuery.second );
	if ( tType == BuddyQuery_e::HTTP )
		tBuf << " */";

	tBuf << ";\n";

	WriteQuery ( tBuf );
}


static int64_t LogFilterStatementSphinxql ( Str_t sQuery, SqlStmt_e eStmt )
{
	if ( g_tLogStatements.IsEmpty() )
		return 0;

	if ( !g_tLogStatements.BitGet ( eStmt ) )
		return 0;

	int64_t tmStarted = sphMicroTimer();
	LogSphinxqlClause ( sQuery, 0 );
	return tmStarted;
}


void ConfigureQueryLogCommands ( const CSphString & sMode )
{
	if ( sMode.IsEmpty() )
		return;

	StrVec_t dOpts = sphSplit ( sMode.cstr(), "," );

	SmallStringHash_T<int> hStmt;
	for ( int i = 0; i < STMT_TOTAL; ++i )
		hStmt.Add ( i, SqlStmt2Str ( (SqlStmt_e) i ) );

	CSphBitvec tLogStatements ( STMT_TOTAL );
	StringBuilder_c sWrongModes ( "," );

	for ( const CSphString & sOpt: dOpts )
	{
		if ( sOpt == "0" ) // implicitly disable all statements
			return;

		if ( sOpt == "1" || sOpt == "*" ) // enable all statements
		{
			tLogStatements.Set();
			g_tLogStatements = tLogStatements;
			return;
		}

		// check for whole statement enumerated
		int * pMode = hStmt ( sOpt );
		if ( pMode )
		{
			tLogStatements.BitSet ( *pMode );
			continue;
		}

		bool bHasWild = false;
		for ( const char * s = sOpt.cstr(); *s && !bHasWild; s++ )
			bHasWild = sphIsWild ( *s );

		if ( bHasWild )
		{
			bool bMatched = false;
			for ( int i = 0; i < STMT_TOTAL; ++i )
			{
				if ( sphWildcardMatch ( SqlStmt2Str ( (SqlStmt_e) i ), sOpt.cstr() ) )
				{
					tLogStatements.BitSet ( i );
					bMatched = true;
					break;
				}
			}

			if ( bMatched )
				continue;
		}

		sWrongModes += sOpt.cstr();
	}

	if ( tLogStatements.BitCount() )
		g_tLogStatements = tLogStatements;

	if ( !sWrongModes.IsEmpty() )
		sphWarning ( "query_log_statements invalid values: %s", sWrongModes.cstr() );
}

void LogSphinxqlClause ( Str_t sQuery, int iRealTime )
{
	if ( g_iQueryLogFile < 0 || g_eLogFormat != LOG_FORMAT::SPHINXQL || !IsFilled ( sQuery ) )
		return;
	if ( session::IsQueryLogDisabled() && g_eLogLevel == SPH_LOG_INFO )
		return;

	StringBuilder_c tBuf;
	tBuf << "/* ";
	FormatTimeConnClient ( tBuf );
	tBuf << " real " << FixedFrac ( iRealTime ) << " */ " << sQuery << ";\n";
	WriteQuery ( tBuf );
}

void LogQueryToSphinxlLog (const CSphString& sQuery, const CSphString& sError )
{
	if ( g_eLogFormat!=LOG_FORMAT::SPHINXQL || g_iQueryLogFile<0 )
		return;

	StringBuilder_c tBuf;
	tBuf << "/* ";
	sphFormatCurrentTime ( tBuf );
	tBuf << "*/ " << sQuery << " # error=" << sError << '\n';
	WriteQuery ( tBuf );
}

constexpr Str_t g_tLogDoneStmt = FROMS ("/* DONE */");

LogStmtGuard_c::LogStmtGuard_c ( const Str_t & sQuery, SqlStmt_e eStmt, bool bMulti )
	: m_tmStarted(LogFilterStatementSphinxql ( sQuery, eStmt ))
{
	m_bLogDone = ( m_tmStarted && eStmt!=STMT_UPDATE && eStmt!=STMT_SELECT && !bMulti ); // update and select will log differently
}

LogStmtGuard_c::~LogStmtGuard_c ()
{
	if ( !m_bLogDone )
		return;

	int64_t tmDelta = sphMicroTimer() - m_tmStarted;
	LogSphinxqlClause ( g_tLogDoneStmt, (int)( tmDelta / 1000 ) );
}
