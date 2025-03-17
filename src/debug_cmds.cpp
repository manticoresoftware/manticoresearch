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

#include "sphinxql_debug.h"

#include "searchdaemon.h"
#include "searchdreplication.h"
#include "searchdtask.h"
#include "digest_sha1.h"
#include "tracer.h"
#include "netfetch.h"

#include "taskmalloctrim.h"
#include "tasksavestate.h"

#include <csignal>

/////////////////////////////////////////////////////////////////////////////
namespace DebugCmd {
enum Traits_e : BYTE
{
	NONE = 0,
	NEED_VIP = 1,
	NO_WIN = 2,
	MALLOC_STATS = 4,
	MALLOC_TRIM = 8,
};

struct CmdNotice_t
{
	BYTE m_uTraits;
	const char * m_szExample;
	const char * m_szExplanation;
};

//static constexpr std::array<CmdNotice_t, (BYTE)Cmd_e::INVALID_CMD> dCommands {
static constexpr CmdNotice_t dCommands[(BYTE) Cmd_e::INVALID_CMD] {
	{ NEED_VIP, "debug shutdown <password>", "emulate TERM signal" },
	{ NEED_VIP, "debug crash <password>", "crash daemon (make SIGSEGV action)" },
	{ NONE, "debug token <password>", "calculate token for password" },
	{ MALLOC_STATS, "debug malloc_stats", "perform 'malloc_stats', result in searchd.log" },
	{ MALLOC_TRIM, "debug malloc_trim", "pefrorm 'malloc_trim' call" },
	{ NEED_VIP | NO_WIN, "debug procdump", "ask watchdog to dump us" },
	{ NEED_VIP | NO_WIN, "debug setgdb on|off", "enable or disable potentially dangerous crash dumping with gdb" },
	{ NEED_VIP | NO_WIN, "debug setgdb status", "show current mode of gdb dumping" },
	{ NONE, "debug sleep <N>", "sleep for <N> seconds" },
	{ NONE, "debug tasks", "display global tasks stat (use select from @@system.tasks instead)" },
	{ NONE, "debug sched", "display task manager schedule (use select from @@system.sched instead)" },
	{ NONE, "debug merge <TBL> [chunk] <X> [into] [chunk] <Y> [option sync=1,byid=0]",
			"For RT table <TBL> merge disk chunk X into disk chunk Y" },
	{ NONE, "debug drop [chunk] <X> [from] <TBL> [option sync=1]",
			"For RT table <TBL> drop disk chunk X" },
	{ NONE, "debug files <TBL> [option format=all|external]",
			"list files belonging to <TBL>. 'all' - including external (wordforms, stopwords, etc.)" },
	{ NONE, "debug close", "ask server to close connection from it's side" },
	{ NONE, "debug compress <TBL> [chunk] <X> [option sync=1]",
			"Compress disk chunk X of RT table <TBL> (wipe out deleted documents)" },
	{ NONE, "debug dedup <TBL> [chunk] <X>",
			"Kill duplicates in disk chunk X of RT table <TBL> (mark duplicates as killed)" },
	{ NONE, "debug split <TBL> [chunk] <X> on @<uservar> [option sync=1]",
			"Split disk chunk X of RT table <TBL> using set of DocIDs from @uservar" },
	{ NO_WIN, "debug wait <cluster> [like 'xx'] [option timeout=3]", "wait <cluster> ready, but no more than 3 secs." },
	{ NO_WIN, "debug wait <cluster> status <N> [like 'xx'] [option timeout=13]", "wait <cluster> commit achieve <N>, but no more than 13 secs" },
	{ NONE, "debug meta", "Show max_matches/pseudo_shards. Needs set profiling=1" },
	{ NONE, "debug trace OFF|'path/to/file' [<N>]", "trace flow to file until N bytes written, or 'trace OFF'" },
	{ NONE, "debug curl <URL>", "request given url via libcurl" },
	{ NONE, "debug pause <ID> on|off", "switch named breakpoint [dev only]" },
};

}
/////////////////////////////////////////////////////////////////////////////
namespace {
	CSphString g_sShutdownToken;
}
void SetShutdownToken ( CSphString sToken ) noexcept
{
	g_sShutdownToken = std::move ( sToken );
}

// stuff for command 'debug', isolated
inline static CSphString strSHA1 ( const CSphString& sLine )
{
	return CalcSHA1 ( sLine.cstr(), sLine.Length() );
}

// defined in searchd.cpp
int GetLogFD ();

void HandleMysqlOptimizeManual ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( "MERGE requires an existing RT table" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eMerge;
	tTask.m_iFrom = (int)tCmd.m_iPar1;
	tTask.m_iTo = (int)tCmd.m_iPar2;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );
	tTask.m_iCutoff = (int)tCmd.iOpt("cutoff");

	RIdx_T<RtIndex_i *> ( pIndex )->StartOptimize ( std::move ( tTask ) );
	if ( tCmd.bOpt ( "sync" ) && !PollOptimizeRunning ( sIndex ) )
		tOut.Error ( "RT table went away during waiting" );
	else
		tOut.Ok ();
}

// command 'drop [chunk] X [from] <IDX> [option...]'
void HandleMysqlDropManual ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( "DROP requires an existing RT table" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eDrop;
	tTask.m_iFrom = (int)tCmd.m_iPar1;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );

	RIdx_T<RtIndex_i *> ( pIndex )->StartOptimize ( std::move ( tTask ) );
	if ( tCmd.bOpt ( "sync" ) && !PollOptimizeRunning ( sIndex ) )
		tOut.Error ( "RT table went away during waiting" );
	else
		tOut.Ok ();
}

void HandleMysqlCompress ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( "COMPRESS requires an existing RT table" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eCompress;
	tTask.m_iFrom = (int) tCmd.m_iPar1;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );

	RIdx_T<RtIndex_i *> ( pIndex )->StartOptimize ( std::move ( tTask ) );
	if ( tCmd.bOpt ( "sync" ) && !PollOptimizeRunning ( sIndex ) )
		tOut.Error ( "RT table went away during waiting" );
	else
		tOut.Ok ();
}

void HandleMysqlDedup ( RowBuffer_i& tOut, const DebugCmd::DebugCommand_t& tCmd )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( "DEDUP requires an existing RT table" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eDedup;
	tTask.m_iFrom = (int)tCmd.m_iPar1;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );

	RIdx_T<RtIndex_i*> ( pIndex )->Optimize ( std::move ( tTask ) );
	tOut.Ok();
}

// command 'split <IDX> [chunk] N on @uservar [option...]'
// IDX is tCmd.m_sParam
// chunk is tCmd.m_iPar1
// uservar is tCmd.m_sParam2
void HandleMysqlSplit ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	if ( !sphCheckWeCanModify ( tOut ) )
		return;

	// check index existance
	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsMutable ( pIndex ) )
	{
		tOut.Error ( "SPLIT requires an existing RT table" );
		return;
	}

	bool bVarFound = false;
	IterateUservars ( [&tCmd, &bVarFound] ( const NamedRefVectorPair_t & dVar ) {
		if ( dVar.first == tCmd.m_sParam2
//			&& dVar.second.m_eType==USERVAR_INT_SET_TMP // uncomment this to split only by session (result of delete .. store) variables
		)
			bVarFound = true;
	} );

	if ( !bVarFound )
	{
		tOut.Error ( "SPLIT requires an existing session @uservar" );
		return;
	}

	OptimizeTask_t tTask;
	tTask.m_eVerb = OptimizeTask_t::eSplit;
	tTask.m_iFrom = (int)tCmd.m_iPar1;
	tTask.m_sUvarFilter = tCmd.m_sParam2;
	tTask.m_bByOrder = !tCmd.bOpt ( "byid", session::GetOptimizeById() );

	RIdx_T<RtIndex_i *> ( pIndex )->StartOptimize ( std::move ( tTask ) );
	if ( tCmd.bOpt ( "sync" ) && !PollOptimizeRunning ( sIndex ) )
		tOut.Error ( "RT table went away during waiting" );
	else
		tOut.Ok ();
}


void HandleMysqlDebugMeta ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd, const QueryProfile_c & tProfile )
{
	VectorLike tLike ( tCmd.sOpt ( "like" ) );
	tLike.MatchTupletf ( "pseudo_shards", "%d", tProfile.m_iPseudoShards );
	tLike.MatchTupletf ( "max_matches", "%d", tProfile.m_iMaxMatches );
	tOut.DataTable(tLike);
}


void HandleMysqlfiles ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	auto sIndex = tCmd.m_sParam;
	auto pIndex = GetServed ( sIndex );
	if ( !ServedDesc_t::IsLocal ( pIndex ) )
	{
		tOut.Error ( "FILES requires an existing local table" );
		return;
	}

	StrVec_t dFiles;
	StrVec_t dExt;
	RIdx_c ( pIndex )->GetIndexFiles ( dFiles, dExt );

	VectorLike dOut ( 0 );
	dOut.SetColNames ( { "file" } );

	auto sFormat = tCmd.sOpt ( "format" );
	if ( sFormat!="external" )
		dFiles.Apply ( [&dOut] ( const CSphString & a ) { dOut.Add ( a ); } );

	if ( sFormat=="all" || sFormat=="external" )
	{
		dExt.Uniq ();
		dExt.Apply ( [&dOut] ( const CSphString & a ) { dOut.Add ( a ); } );
	}

	tOut.DataTable ( dOut );
}

void HandleMysqlclose ( RowBuffer_i & tOut )
{
	auto iSocket = session::Info().GetSocket();
	if ( iSocket >= 0 )
		sphSockClose ( iSocket );
}


void HandleShutdownCrash ( RowBuffer_i & tOut, const CSphString & sPasswd, DebugCmd::Cmd_e eCmd )
{
	const char * szCmd = DebugCmd::dCommands[(BYTE) eCmd].m_szExample;
	if ( g_sShutdownToken.IsEmpty () )
	{
		tOut.Error ( "shutdown_token is empty. Provide it in searchd config section." );
		return;
	}

	if ( strSHA1 ( sPasswd )!=g_sShutdownToken )
	{
		tOut.Error ( "FAIL" );
		return;
	}

	tOut.HeadTuplet ( "command", "result" );
	tOut.DataTuplet ( szCmd, "SUCCESS" );
	tOut.Eof ();
	if ( eCmd==DebugCmd::Cmd_e::SHUTDOWN )
	{
		sphInterruptNow ();
	} else // crash
	{
		BYTE * pSegv = (BYTE *) ( 0 );
		*pSegv = 'a';
	}
}

#if !_WIN32
void HandleProcDump ( RowBuffer_i & tOut )
{
	auto & iParentPid = getParentPID ();
	tOut.HeadTuplet ( "command", "result" );
	if ( iParentPid<=0 )
		tOut.DataTuplet ( "procdump", "Unavailable (no watchdog)" );
	else
	{
		kill ( iParentPid, SIGUSR1 );
		tOut.DataTupletf ( "procdump", "Sent USR1 to wathcdog (%d)", iParentPid );
	}
	tOut.Eof ();
}

void HandleGdbStatus ( RowBuffer_i & tOut )
{
	auto& iParentPid = getParentPID ();
	tOut.HeadTuplet ( "command", "result" );
	const auto & g_bSafeGDB = getSafeGDB ();
	if ( iParentPid>0 )
		tOut.DataTupletf ( "setgdb", "Enabled, managed by watchdog (pid=%d)", iParentPid );
	else if ( g_bSafeGDB )
		tOut.DataTupletf ( "setgdb", "Enabled, managed locally because of jemalloc", iParentPid );
	else if ( iParentPid==-1 )
		tOut.DataTuplet ( "setgdb", "Enabled locally, MAY HANG!" );
	else
		tOut.DataTuplet ( "setgdb", "Disabled" );
	tOut.Eof ();
}

void HandleSetGdb ( RowBuffer_i & tOut, bool bParam )
{
	auto & iParentPid = getParentPID ();
	tOut.HeadTuplet ( "command", "result" );
	const auto & g_bSafeGDB = getSafeGDB ();

	if ( iParentPid>0 )
		tOut.DataTupletf ( "setgdb", "Enabled by watchdog (pid=%d)", iParentPid );
	else if ( g_bSafeGDB )
		tOut.DataTuplet ( "setgdb", "Enabled locally because of jemalloc" );
	else if ( bParam )
	{
		iParentPid = -1;
		tOut.DataTuplet ( "setgdb", "Ok, enabled locally, MAY HANG!" );
	} else if ( !bParam )
	{
		iParentPid = 0;
		tOut.DataTuplet ( "setgdb", "Ok, disabled" );
	}
	tOut.Eof ();
}

void HandleWait ( RowBuffer_i& tOutBuf, const DebugCmd::DebugCommand_t& tCmd )
{
	auto iTimeoutS = tCmd.iOpt ( "timeout" );
	auto sCluster = tCmd.m_sParam;
	auto iTime = -sphMicroTimer();
	auto sState = WaitClusterReady ( sCluster, iTimeoutS );
	iTime += sphMicroTimer();
	VectorLike tOut { tCmd.sOpt ( "like" ) };
	tOut.SetColName("name");
	tOut.MatchTuplet ( "cluster", sCluster.cstr() );
	tOut.MatchTuplet ( "state", sState.cstr() );
	tOut.MatchTupletf ( "time", "%.2t", iTime );
	tOutBuf.DataTable ( tOut );
}

void HandleWaitStatus ( RowBuffer_i& tOutBuf, const DebugCmd::DebugCommand_t& tCmd )
{
	auto iTimeoutS = tCmd.iOpt ( "timeout" );
	auto sCluster = tCmd.m_sParam;
	auto iTxn = (int)tCmd.m_iPar1;
	auto iTime = -sphMicroTimer();
	auto tAchieved = WaitClusterCommit ( sCluster, iTxn, iTimeoutS );
	iTime += sphMicroTimer();
	VectorLike tOut { tCmd.sOpt ( "like" ) };
	tOut.SetColName ( "name" );
	tOut.MatchTuplet ( "cluster", sCluster.cstr() );
	tOut.MatchTupletf ( "wanted", "%d", iTxn );
	if ( tAchieved.first>=0 )
		tOut.MatchTupletf ( "state", "%d", tAchieved.first );
	else
		tOut.MatchTuplet ( "achieved", tAchieved.second.cstr() );
	tOut.MatchTupletf ( "time", "%.2t", iTime );
	tOutBuf.DataTable ( tOut );
}
#endif

void HandleTrace ( RowBuffer_i& tOut, const DebugCmd::DebugCommand_t& tCmd )
{
	tOut.HeadTuplet ( "command", "result" );
#ifdef PERFETTO
	if ( tCmd.m_sParam.IsEmpty() )
	{
		if ( !tCmd.m_iPar1 )
		{
			Tracer::Stop();
		}
	} else
	{
		Tracer::Start ( tCmd.m_sParam, tCmd.m_iPar1 );
	}
	tOut.DataTuplet ( "debug trace ...", "SUCCESS" );
#else
	tOut.DataTuplet ( "debug trace ...", "FAIL, need to rebuild with Perfetto, look to src/perfetto/README.txt" );
#endif
	tOut.Eof();
}

void HandleToken ( RowBuffer_i & tOut, const CSphString & sParam )
{
	auto sSha = strSHA1 ( sParam );
	tOut.HeadTuplet ( "command", "result" );
	tOut.DataTuplet ( "debug token", sSha.cstr () );
	tOut.Eof ();
}

void HandleCurl ( RowBuffer_i & tOut, const CSphString & sParam )
{
	auto sRes = FetchUrl ( sParam );
	tOut.HeadTuplet ( "command", "result" );
	tOut.DataTuplet ( "curl", sRes.cstr() );
	tOut.Eof();
}


void HandlePause ( RowBuffer_i & tOut, const DebugCmd::DebugCommand_t & tCmd )
{
	tOut.HeadTuplet ( "command", "result" );
	auto bPause = tCmd.m_iPar1!=0;
	PauseAt ( tCmd.m_sParam, bPause );
	tOut.DataTuplet ( "debug pause ...", bPause ? "Set" : "Unset" );
	tOut.Eof ();
}

#if HAVE_MALLOC_STATS
void HandleMallocStats ( RowBuffer_i & tOut, const CSphString& sParam )
{
	tOut.HeadTuplet ( "command", "result" );
	// check where is stderr...
	int iOldErr = ::dup ( STDERR_FILENO );
	::dup2 ( GetLogFD (), STDERR_FILENO );
	sphMallocStats ( sParam.cstr() );
	::close ( STDERR_FILENO );
	::dup2 ( iOldErr, STDERR_FILENO );
	::close ( iOldErr );
	tOut.DataTuplet ( "malloc_stats", sphGetLogFile().cstr () );
	tOut.Eof ();
}
#endif

#if HAVE_MALLOC_TRIM
void HandleMallocTrim ( RowBuffer_i & tOut )
{
	tOut.HeadTuplet ( "command", "result" );
	CSphString sResult;
	sResult.SetSprintf ( "%d", PerformMallocTrim ( 0 ) );
	tOut.DataTuplet ( "malloc_trim", sResult.cstr () );
	tOut.Eof ();
}
#endif

void HandleSleep ( RowBuffer_i & tOut, int64_t iParam )
{
	int64_t tmStart = sphMicroTimer ();
	Threads::Coro::SleepMsec ( Max ( iParam/1000, 1 ) );
	int64_t tmDelta = sphMicroTimer ()-tmStart;

	tOut.HeadTuplet ( "command", "result" );
	CSphString sResult;
	sResult.SetSprintf ( "%.3f", (float) tmDelta / 1000000.0f );
	tOut.DataTuplet ( "sleep", sResult.cstr () );
	tOut.Eof ();
}

void HandleTasks ( RowBuffer_i & tOut )
{
	if (!tOut.HeadOfStrings ( { "Name", "MaxRunners", "CurrentRunners", "TotalSpent", "LastFinished", "Executed", "Dropped", "Enqueued" } ))
		return;

	auto dTasks = TaskManager::GetTaskInfo ();
	for ( const auto & dTask : dTasks )
	{
		tOut.PutString ( dTask.m_sName );
		if ( dTask.m_iMaxRunners > 0 )
			tOut.PutNumAsString ( dTask.m_iMaxRunners );
		else
			tOut.PutString ( "unlimited" );
		tOut.PutNumAsString ( dTask.m_iCurrentRunners );
		tOut.PutTimeAsString ( dTask.m_iTotalSpent );
		tOut.PutTimestampAsString ( dTask.m_iLastFinished );
		tOut.PutNumAsString ( dTask.m_iTotalRun );
		tOut.PutNumAsString ( dTask.m_iTotalDropped );
		tOut.PutNumAsString ( dTask.m_iAllRunners );
		if ( !tOut.Commit () )
			return;
	}
	tOut.Eof ();
}

void HandleSched ( RowBuffer_i & tOut )
{
	if (!tOut.HeadOfStrings ( { "Time rest", "Task" } ))
		return;
	auto dTasks = sph::GetSchedInfo ();
	for ( auto& dTask : dTasks )
	{
		tOut.PutTimestampAsString ( dTask.m_iTimeoutStamp );
		tOut.PutString ( dTask.m_sTask );
		if (!tOut.Commit ())
			return;
	}
	tOut.Eof ();
}

void HandleMysqlDebug ( RowBuffer_i &tOut, const DebugCmd::DebugCommand_t* pCommand, const QueryProfile_c & tProfile )
{
	using namespace DebugCmd;
	bool bVipConn = session::GetVip ();
	assert ( pCommand->Valid() );

	const auto& tCmd = *pCommand;

	if ( bVipConn )
	{
		switch ( tCmd.m_eCommand )
		{
		case Cmd_e::SHUTDOWN:
		case Cmd_e::CRASH: HandleShutdownCrash ( tOut, tCmd.m_sParam, tCmd.m_eCommand ); return;
#if !_WIN32
		case Cmd_e::PROCDUMP: HandleProcDump ( tOut ); return;
		case Cmd_e::SETGDB: HandleSetGdb ( tOut, tCmd.m_iPar1!=0 ); return;
		case Cmd_e::GDBSTATUS: HandleGdbStatus ( tOut ); return;
#endif
		default: break;
		}
	}


	switch ( tCmd.m_eCommand )
	{
#if HAVE_MALLOC_STATS
	case Cmd_e::MALLOC_STATS: HandleMallocStats ( tOut, tCmd.m_sParam ); return;
#endif

#if HAVE_MALLOC_TRIM
	case Cmd_e::MALLOC_TRIM: HandleMallocTrim ( tOut ); return;
#endif
	case Cmd_e::TOKEN: HandleToken ( tOut, tCmd.m_sParam ); return;
	case Cmd_e::SLEEP: HandleSleep ( tOut, tCmd.m_iPar1 ); return;
	case Cmd_e::TASKS: HandleTasks ( tOut ); return;
	case Cmd_e::SCHED: HandleSched ( tOut ); return;
	case Cmd_e::MERGE: HandleMysqlOptimizeManual ( tOut, tCmd ); return;
	case Cmd_e::DROP: HandleMysqlDropManual ( tOut, tCmd ); return;
	case Cmd_e::FILES: HandleMysqlfiles ( tOut, tCmd ); return;
	case Cmd_e::CLOSE: HandleMysqlclose ( tOut ); return;
	case Cmd_e::COMPRESS: HandleMysqlCompress ( tOut, tCmd ); return;
	case Cmd_e::DEDUP: HandleMysqlDedup ( tOut, tCmd ); return;
	case Cmd_e::SPLIT: HandleMysqlSplit ( tOut, tCmd ); return;
	case Cmd_e::META: HandleMysqlDebugMeta ( tOut, tCmd, tProfile ); return;
#if !_WIN32
	case Cmd_e::WAIT: HandleWait ( tOut, tCmd ); return;
	case Cmd_e::WAIT_STATUS: HandleWaitStatus ( tOut, tCmd ); return;
#endif
	case Cmd_e::TRACE: HandleTrace ( tOut, tCmd );	return;
	case Cmd_e::CURL: HandleCurl ( tOut, tCmd.m_sParam ); return;
	case Cmd_e::PAUSE: HandlePause ( tOut, tCmd ); return;
	default: break;
	}

	// no known command; provide short help.
	BYTE uMask = bVipConn ? DebugCmd::NEED_VIP : DebugCmd::NONE;
#if !_WIN32
	uMask |= DebugCmd::NO_WIN;
#endif

#if HAVE_MALLOC_STATS
	uMask |= DebugCmd::MALLOC_STATS;
#endif

#if HAVE_MALLOC_TRIM
	uMask |= DebugCmd::MALLOC_TRIM;
#endif

	// display a short help
	tOut.HeadTuplet ( "command", "meaning" );
	tOut.DataTuplet ( "flush logs", "emulate USR1 signal" );
	tOut.DataTuplet ( "reload tables", "emulate HUP signal" );
	for ( const auto& dCommand : DebugCmd::dCommands )
		if ( ( dCommand.m_uTraits & uMask )==dCommand.m_uTraits )
			tOut.DataTuplet ( dCommand.m_szExample, dCommand.m_szExplanation );
	tOut.Eof ();
}
