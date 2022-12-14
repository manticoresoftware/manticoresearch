//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchdtask.h"
#include "netreceive_ql.h"

#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/process.hpp>
#if _WIN32
#include <boost/winapi/process.hpp>
#endif

#include "netfetch.h"
#include "searchdbuddy.h"

static std::unique_ptr<boost::process::child> g_pBuddy;
static CSphString g_sPath;
static CSphString g_sListener4Buddy;
static int g_iBuddyVersion = 1;
static CSphString g_sUrlBuddy;
static CSphString g_sStartArgs;

static boost::asio::io_service g_tIOS;
static std::vector<char> g_dPipeBuf ( 4096 );
static std::unique_ptr<boost::process::async_pipe> g_pPipe;
enum class BuddyState_e
{
	NONE,
	STARTING,
	WORK,
	STOPPED,
	FAILED
};
static BuddyState_e g_eBuddy { BuddyState_e::NONE };
static int g_iRestartCount = 0;
static int64_t g_tmStarting = 0;
static int g_iTask = 0;

static const int g_iRestartMax = 3;
static const int g_iStartMaxTimeout = 3; // max start timeout 3 sec

static BuddyState_e TryToStart ( const char * sArgs, CSphString & sError );
static CSphString GetUrl ( const ListenerDesc_t & tDesc );
static void BuddyNextTick();
static CSphString BuddyGetPath ( const CSphString & sPath, bool bHasBuddyPath );

#if _WIN32
struct BuddyWindow_t : boost::process::detail::handler_base
{
    // this function will be invoked at child process constructor before spawning process
    template <class WindowsExecutor>
    void on_setup ( WindowsExecutor & e ) const
    {
        e.creation_flags = boost::winapi::CREATE_NEW_CONSOLE_;
    }
};
#endif

struct PreservedStd_t : boost::process::detail::handler, boost::process::detail::uses_handles
{
	std::vector<int> get_used_handles ()
	{
		int iStdOut = STDOUT_FILENO;
		int iStdErr = STDERR_FILENO;
		std::vector<int> dHandles { iStdOut, iStdErr };
		return dHandles;
	}
};

static CSphString CheckLine ( Str_t sLine )
{
	char sBuddyAddrHead[] = "started ";
	const char * sGot = strstr ( sLine.first, sBuddyAddrHead );
	if ( sGot )
	{
		int iHeaderLen = sizeof ( sBuddyAddrHead ) - 1;
		const char * sAddrStart = sGot + iHeaderLen;
		int iAddrLen = sLine.second - iHeaderLen;
		assert ( iAddrLen>0 );

		CSphString sAddr;
		sAddr.SetBinary ( sAddrStart, iAddrLen );
		return sAddr;
	}

	return CSphString();
}

static Str_t TrimRight ( Str_t tSrc )
{
	char * sStart = const_cast<char *>( tSrc.first );
	if ( !tSrc.second )
	{
		sStart[tSrc.second] = '\0';
		return tSrc;
	}

	const char * sEnd = sStart + tSrc.second - 1;
	while ( sStart<=sEnd && isspace ( (unsigned char)*sEnd ) )
		sEnd--;

	int iLen = sEnd - sStart + 1;
	sStart[iLen] = '\0';
	return Str_t ( tSrc.first, iLen );
}

static void BuddyPipe_fn ( const boost::system::error_code & tGotCode, std::size_t iSize )
{
	if ( tGotCode.failed() )
		return;

	if ( !iSize )
		return;

	Str_t sLine = TrimRight ( { g_dPipeBuf.data(), iSize } );
	// regular work log all lines from the buddy output
	if ( g_eBuddy!=BuddyState_e::STARTING )
	{
		sphInfo ( "[BUDDY] %s", sLine.first ); // FIXME!!! add split line by line
		return;
	}

	// at the BuddyState_e::STARTING parsing buddy output
	CSphString sAddr = CheckLine ( sLine );
	sAddr.Trim();
	if ( sAddr.IsEmpty() )
	{
		g_eBuddy = BuddyState_e::FAILED;
		sphWarning ( "[BUDDY] invalid output, should be 'address:port', got '%s'", sLine.first );
		return;
	}

	CSphString sError;
	ListenerDesc_t tListen = ParseListener ( sAddr.cstr(), &sError );
	if ( tListen.m_eProto==Proto_e::UNKNOWN || !sError.IsEmpty() )
	{
		g_eBuddy = BuddyState_e::FAILED;
		sphWarning ( "[BUDDY] invalid output, should be 'address:port', got '%s', parse error: %s", sAddr.cstr(), sError.cstr() );
		return;
	}

	// buddy really started and ready to serve queries
	g_sUrlBuddy = GetUrl( tListen );
	g_eBuddy = BuddyState_e::WORK;
	g_iRestartCount = 0;
	sphInfo ( "[BUDDY] started '%s' at %s", g_sStartArgs.cstr(), g_sUrlBuddy.cstr() );
}

static BuddyState_e BuddyCheckLive()
{
	assert ( g_eBuddy==BuddyState_e::WORK );

	std::error_code tErrorCode;
	if ( g_pBuddy && g_pBuddy->running ( tErrorCode ) )
		return BuddyState_e::WORK;

	// need to restart buddy as curent buddy got killed
	if ( g_pBuddy )
		sphWarning ( "[BUDDY] terminated, exit code %d", tErrorCode.value() );
	return BuddyState_e::FAILED;
}

static void BuddyTryRestart()
{
	if ( g_eBuddy!=BuddyState_e::FAILED )
		return;

	BuddyStop();
	g_iRestartCount++;
	if ( g_iRestartCount>=g_iRestartMax )
	{
		sphInfo ( "[BUDDY] restart amount of attempts (%d) has been exceeded", g_iRestartMax );
		return;
	}

	CSphString sErorr;
	g_eBuddy = TryToStart ( g_sStartArgs.cstr(), sErorr );
	if ( g_eBuddy!=BuddyState_e::STARTING )
	{
		sphWarning ( "[BUDDY] failed to restart: %s", sErorr.cstr() );
		BuddyStop();
	} else
	{
		sphInfo ( "[BUDDY] restarting" );
	}
}

static void NextShedule()
{
	int64_t tmCur = sphMicroTimer();
	TaskManager::ScheduleJob ( g_iTask, tmCur + 15000, BuddyNextTick );
}

void BuddyNextTick()
{
	auto pDesc = PublishSystemInfo ( "buddy check" );

	if ( g_pPipe )
		g_pPipe->async_read_some( boost::asio::buffer ( g_dPipeBuf ), BuddyPipe_fn );
	g_tIOS.poll();
	g_tIOS.restart();

	if ( g_eBuddy==BuddyState_e::STARTING && sphMicroTimer()>( g_tmStarting + g_iStartMaxTimeout * 1000 * 1000  ) )
	{
		sphWarning ( "[BUDDY] failed to start after %d sec", g_iStartMaxTimeout );
		BuddyStop();
		return;
	}

	if ( g_eBuddy==BuddyState_e::WORK )
		g_eBuddy = BuddyCheckLive();

	BuddyTryRestart();

	if ( g_eBuddy==BuddyState_e::STARTING || g_eBuddy==BuddyState_e::WORK )
		NextShedule();
}

BuddyState_e TryToStart ( const char * sArgs, CSphString & sError )
{
	std::string sCmd = sArgs;
	g_pBuddy.reset();
	g_pPipe.reset ( new boost::process::async_pipe ( g_tIOS ) );

	std::unique_ptr<boost::process::child> pBuddy;
	std::error_code tErrorCode;

#if _WIN32
	BuddyWindow_t tWnd;
	pBuddy.reset ( new boost::process::child ( sCmd, ( boost::process::std_out & boost::process::std_err ) > *g_pPipe, tWnd, boost::process::limit_handles, boost::process::error ( tErrorCode ) ) );
#else
	PreservedStd_t tPreserveStd;
	pBuddy.reset ( new boost::process::child ( sCmd, ( boost::process::std_out & boost::process::std_err ) > *g_pPipe, boost::process::limit_handles, boost::process::error ( tErrorCode ) , tPreserveStd ) );
#endif

	if ( !pBuddy->running ( tErrorCode ) )
	{
		sError.SetSprintf ( "'%s' terminated with exit code %d", sArgs, tErrorCode.value() );
		return BuddyState_e::FAILED;
	}

	g_tmStarting = sphMicroTimer();
	g_pBuddy = std::move ( pBuddy );
	return BuddyState_e::STARTING;
}

CSphString GetUrl ( const ListenerDesc_t & tDesc )
{
	char sAddrBuf [ SPH_ADDRESS_SIZE ];
	sphFormatIP ( sAddrBuf, sizeof(sAddrBuf), tDesc.m_uIP );
	
	CSphString sURI;
	sURI.SetSprintf ( "http://%s:%d", sAddrBuf, tDesc.m_iPort );

	return sURI;
}


void BuddyStart ( const CSphString & sConfigPath, bool bHasBuddyPath, const VecTraits_T<ListenerDesc_t> & dListeners, bool bTelemetry )
{
	CSphString sPath = BuddyGetPath ( sConfigPath, bHasBuddyPath );
	if ( sPath.IsEmpty() )
		return;

	ARRAY_FOREACH ( i, dListeners )
	{
		const ListenerDesc_t & tDesc = dListeners[i];
		if ( tDesc.m_eProto==Proto_e::SPHINX || tDesc.m_eProto==Proto_e::HTTP )
		{
			g_sListener4Buddy = GetUrl ( tDesc );
			break;
		}
	}
	if ( g_sListener4Buddy.IsEmpty() )
	{
		sphWarning ( "[BUDDY] no SPHINX or HTTP listeners found, disabled" );
		return;
	}

	if ( !IsCurlAvailable() )
	{
		sphWarning ( "[BUDDY] no curl found, disabled" );
		return;
	}

	g_sPath = sPath;

	g_sStartArgs.SetSprintf ( "%s --listen=%s %s",
		g_sPath.cstr(),
		g_sListener4Buddy.cstr(),
		( bTelemetry ? "" : "--disable-telemetry" ) );

	CSphString sErorr;
	BuddyState_e eBuddy = TryToStart ( g_sStartArgs.cstr(), sErorr );
	if ( eBuddy!=BuddyState_e::STARTING )
	{
		sphWarning ( "[BUDDY] failed to start: %s", sErorr.cstr() );
		BuddyStop();
		return;
	}

	g_eBuddy = eBuddy;
	g_iTask = TaskManager::RegisterGlobal ( "buddy service" );
	assert ( g_iTask>=0 && "failed to create buddy service task" );
	BuddyNextTick();
}

void BuddyStop ()
{
	if ( g_pBuddy )
	{
		std::error_code tErrorCode;
		g_pBuddy->terminate ( tErrorCode );
		if ( tErrorCode )
			sphWarning ( "[BUDDY] stopped, exit code: %d", tErrorCode.value() );
	}

	g_eBuddy = BuddyState_e::STOPPED;
	g_pBuddy.reset();
}

bool HasBuddy()
{
	return ( g_eBuddy==BuddyState_e::WORK );
}

static std::pair<bool, CSphString> BuddyQuery ( bool bHttp, Str_t sQueryError, Str_t sPathQuery, Str_t sQuery )
{
	if ( !HasBuddy() )
		return { false, {} };

	JsonEscapedBuilder tBuddyQuery;
	{
		auto tRoot = tBuddyQuery.Object();
		tBuddyQuery.NamedString ( "type", bHttp ? "unknown json request" : "unknown sql request" );
		tBuddyQuery.NamedString ( "error", sQueryError );
		tBuddyQuery.NamedVal ( "version", g_iBuddyVersion );

		{
			tBuddyQuery.Named ( "message" );
			auto tMessageRoot = tBuddyQuery.Object();
			tBuddyQuery.NamedString ( "path_query", sPathQuery );
			tBuddyQuery.NamedString ( "body", sQuery );
		}
	}

	CSphString sConnID;
	sConnID.SetSprintf ( "Request-ID: %d", session::GetConnID() );
	CSphFixedVector<const char *> dHeaders ( 1 );
	dHeaders[0] = sConnID.cstr();

	return FetchHelperUrl ( g_sUrlBuddy, (Str_t)tBuddyQuery, dHeaders );
}

static bool HasProhibitBuddy ( const OptionsHash_t & hOptions )
{
	CSphString * pProhibit = hOptions ( "User-Agent" );
	if ( !pProhibit )
		return false;

	return ( pProhibit->Begins ( "Manticore Buddy" ) );
}

struct BuddyReply_t
{
	CSphVector<BYTE> m_dParsedData;
	bson::Bson_c m_tBJSON;

	bson::NodeHandle_t m_tType { bson::nullnode };
	bson::NodeHandle_t m_tError { bson::nullnode };
	bson::NodeHandle_t m_tMessage { bson::nullnode };
};

bson::NodeHandle_t GetNode ( const bson::Bson_c & tReply, const char * sName, CSphString & sError )
{
	bson::NodeHandle_t tNode = tReply.ChildByName ( sName );
	if ( bson::IsNullNode ( tNode ) )
		sError.SetSprintf ( "missed %s", sName );

	return tNode;
}

static bool ParseReply ( char * sReplyRaw, BuddyReply_t & tParsed, CSphString & sError )
{
	if ( !sphJsonParse ( tParsed.m_dParsedData, sReplyRaw, false, false, false, sError ) )
		return false;

	tParsed.m_tBJSON = bson::Bson_c ( tParsed.m_dParsedData );

	if ( tParsed.m_tBJSON.IsEmpty() || !tParsed.m_tBJSON.IsAssoc() )
	{
		const char * sReplyType = ( tParsed.m_tBJSON.IsEmpty() ? "empty" : "not object" );
		sError.SetSprintf ( "wrong reply format - %s", sReplyType );
		return false;
	}

	bson::NodeHandle_t tVer = GetNode ( tParsed.m_tBJSON, "version", sError );
	if ( bson::IsNullNode ( tVer ) )
		return false;
	int iVer = bson::Int ( tVer );
	if ( iVer>g_iBuddyVersion )
	{
		sError.SetSprintf ( "buddy reply version (%d) greater daemon version (%d), upgrade daemon binary", iVer, g_iBuddyVersion );
		return false;
	}
	if ( iVer<1 )
	{
		sError.SetSprintf ( "wrong buddy reply version (%d), daemon version (%d), upgrade buddy", iVer, g_iBuddyVersion );
		return false;
	}

	tParsed.m_tType = GetNode ( tParsed.m_tBJSON, "type", sError );
	tParsed.m_tMessage = GetNode ( tParsed.m_tBJSON, "message", sError );

	if ( !bson::IsNullNode ( tParsed.m_tError ) )
	{
		sError.SetSprintf ( "wrong budy reply version (%d), daemon version (%d), upgrade buddy", iVer, g_iBuddyVersion );
		return false;
	}
	if ( bson::IsNullNode ( tParsed.m_tType ) || bson::IsNullNode ( tParsed.m_tMessage ) )
		return false;

	return true;
}

HttpProcessResult_t ProcessHttpQueryBuddy ( CharStream_c & tSource, OptionsHash_t & hOptions, CSphVector<BYTE> & dResult, bool bNeedHttpResponse, http_method eRequestType )
{
	Str_t sSrcQuery = dEmptyStr;
	HttpProcessResult_t tRes = ProcessHttpQuery ( tSource, sSrcQuery, hOptions, dResult, bNeedHttpResponse, eRequestType );

	if ( tRes.m_bOk || tRes.m_eEndpoint==SPH_HTTP_ENDPOINT_INDEX || !HasBuddy() || HasProhibitBuddy ( hOptions ) || IsEmpty ( sSrcQuery ) )
		return tRes;

	auto tReplyRaw = BuddyQuery ( true, FromStr ( tRes.m_sError ), FromStr ( hOptions["full_url"] ), FromStr ( sSrcQuery ) );

	if ( !tReplyRaw.first )
	{
		sphWarning ( "[BUDDY] [%d] error: %s", session::GetConnID(), tReplyRaw.second.cstr() );
		return tRes;
	}

	CSphString sError;
	BuddyReply_t tReplyParsed;
	if ( !ParseReply ( const_cast<char *>( tReplyRaw.second.cstr() ), tReplyParsed, sError ) )
	{
		sphWarning ( "[BUDDY] [%d] %s: %s", session::GetConnID(), sError.cstr(), tReplyRaw.second.cstr() );
		return tRes;
	}
	if ( bson::String ( tReplyParsed.m_tType )!="json response" )
	{
		sphWarning ( "[BUDDY] [%d] wrong response type %s: %s", session::GetConnID(), bson::String ( tReplyParsed.m_tType ).cstr(), tReplyRaw.second.cstr() );
		return tRes;
	}

	CSphString sDump;
	bson::Bson_c ( tReplyParsed.m_tMessage ).BsonToJson ( sDump );

	dResult.Resize ( 0 );
	ReplyBuf ( FromStr ( sDump ), SPH_HTTP_STATUS_200, bNeedHttpResponse, dResult );
	tRes.m_bOk = true;
	tRes.m_sError = "";
	return tRes;
}

bool ProcessSqlQueryBuddy ( Str_t sQuery, BYTE & uPacketID, ISphOutputBuffer & tOut )
{
	BYTE uRefPacketID = uPacketID;
	int iRefPos = tOut.GetSentCount();

	std::unique_ptr<RowBuffer_i> tRows ( CreateSqlRowBuffer ( &uPacketID, &tOut ) );
	bool bKeepProfile = session::Execute ( sQuery, *tRows );
	bool bOk = !tRows->IsError();

	if ( bOk || !HasBuddy() )
		return bKeepProfile;

	auto tReplyRaw = BuddyQuery ( false, FromStr ( tRows->GetError() ), Str_t(), sQuery );

	if ( !tReplyRaw.first )
	{
		sphWarning ( "[BUDDY] [%d] error: %s", session::GetConnID(), tReplyRaw.second.cstr() );
		return bKeepProfile;
	}

	CSphString sError;
	BuddyReply_t tReplyParsed;
	if ( !ParseReply ( const_cast<char *>( tReplyRaw.second.cstr() ), tReplyParsed, sError ) )
	{
		sphWarning ( "[BUDDY] [%d] %s: %s", session::GetConnID(), sError.cstr(), tReplyRaw.second.cstr() );
		return bKeepProfile;
	}
	if ( bson::String ( tReplyParsed.m_tType )!="sql response" )
	{
		sphWarning ( "[BUDDY] [%d] wrong response type %s: %s", session::GetConnID(), bson::String ( tReplyParsed.m_tType ).cstr(), tReplyRaw.second.cstr() );
		return bKeepProfile;
	}

	if ( bson::IsNullNode ( tReplyParsed.m_tMessage ) || !bson::IsArray ( tReplyParsed.m_tMessage ) )
	{
		const char * sReplyType = ( bson::IsNullNode ( tReplyParsed.m_tMessage ) ? "empty" : "not cli reply array" );
		sphWarning ( "[BUDDY] [%d] wrong reply format - %s: %s", session::GetConnID(), sReplyType, tReplyRaw.second.cstr() );
		return bKeepProfile;
	}

	// reset back out buff and packet
	uPacketID = uRefPacketID;
	tOut.Rewind ( iRefPos );
	std::unique_ptr<RowBuffer_i> tBuddyRows ( CreateSqlRowBuffer ( &uPacketID, &tOut ) );

	ConvertJsonDataset ( tReplyParsed.m_tMessage, sQuery.first, *tBuddyRows );
	return bKeepProfile;
}

#ifdef _WIN32
static CSphString g_sDefaultBuddyName ( "manticore-buddy.phar" );
#else
static CSphString g_sDefaultBuddyName ( "manticore-buddy" );
#endif
static CSphString g_sDefaultBuddyExecName ( "manticore-executor.exe" );

static CSphString GetFullBuddyPath ( const CSphString & sExecPath, const CSphString & sBuddyPath )
{
#ifdef _WIN32
		assert ( !sExecPath.IsEmpty() );
		CSphString sFullPath;
		sFullPath.SetSprintf ( "\"%s\" \"%s\"", sExecPath.cstr(), sBuddyPath.cstr() );
		return sFullPath;
#else
		return sBuddyPath.cstr();
#endif
}

CSphString BuddyGetPath ( const CSphString & sConfigPath, bool bHasBuddyPath )
{
	if ( bHasBuddyPath )
		return sConfigPath;

	CSphString sExecPath;
	CSphString sPathToDaemon = GetPathOnly ( GetExecutablePath() );
	// check executor first
#ifdef _WIN32
	sExecPath.SetSprintf ( "%s%s", sPathToDaemon.cstr(), g_sDefaultBuddyExecName.cstr() );
	if ( !sphFileExists ( sExecPath.cstr() ) )
	{
		sphWarning ( "[BUDDY] no %s found at '%s', disabled", g_sDefaultBuddyExecName.cstr(), sExecPath.cstr() );
		return CSphString();
	}
#endif

	CSphString sPathBuddy2Module;
	sPathBuddy2Module.SetSprintf ( "%s/%s", GET_MANTICORE_MODULES(), g_sDefaultBuddyName.cstr() );
	if ( sphFileExists ( sPathBuddy2Module.cstr() ) )
		return GetFullBuddyPath ( sExecPath, sPathBuddy2Module );

	// check at the daemon location / cwd
	CSphString sPathBuddy2Cwd;
	sPathBuddy2Cwd.SetSprintf ( "%s%s", sPathToDaemon.cstr(), g_sDefaultBuddyName.cstr() );
	if ( sphFileExists ( sPathBuddy2Cwd.cstr() ) )
		return GetFullBuddyPath ( sExecPath, sPathBuddy2Cwd );

	sphWarning ( "[BUDDY] no %s found at '%s', disabled", g_sDefaultBuddyName.cstr(), sPathBuddy2Module.cstr() );

	return CSphString();
}