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

#include "searchdtask.h"
#include "netreceive_ql.h"
#include "client_session.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/process.hpp>
#if _WIN32
#include <boost/winapi/process.hpp>
#endif
#include "replication/portrange.h"

#include "netfetch.h"
#include "searchdbuddy.h"

static std::unique_ptr<boost::process::child> g_pBuddy;
static CSphString g_sPath;
static CSphString g_sListener4Buddy;
static CSphString g_sUrlBuddy;
static CSphString g_sStartArgs;

static const int PIPE_BUF_SIZE = 2048;
static std::unique_ptr<boost::asio::io_context> g_pIOS;
static std::vector<char> g_dPipeBuf ( PIPE_BUF_SIZE );
static CSphVector<char> g_dLogBuf ( PIPE_BUF_SIZE );
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
static const int g_iBuddyLoopSleep = 15;

static const int g_iRestartMax = 3;
static const int g_iStartMaxTimeout = val_from_env ( "MANTICORE_BUDDY_TIMEOUT", 3 ); // max start timeout 3 sec

static int g_iBuddyVersion = 3;
static bool g_bBuddyVersion = false;
extern CSphString g_sStatusVersion;
static CSphString g_sContainerName;

// windows docker needs port XXX:9999 port mapping
static std::unique_ptr<FreePortList_i> g_pBuddyPortList { nullptr };
ScopedPort_c g_tBuddyPort;

static BuddyState_e TryToStart ( const char * sArgs, CSphString & sError );
static CSphString GetUrl ( const ListenerDesc_t & tDesc );
static CSphString BuddyGetPath ( const CSphString & sPath, const CSphString & sPluginDir, bool bHasBuddyPath, int iHostPort, const CSphString & sDataDir );
static void BuddyStop ();

#if _WIN32
static CSphString g_sBuddyBind = "--bind=0.0.0.0:9999";
#else
static CSphString g_sBuddyBind = "";
#endif

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

static std::pair<Str_t, Str_t> CheckLine ( Str_t sLine )
{
	CSphVector<Str_t> dWords;
	sphSplitApply ( sLine.first, sLine.second, " \n\r", [&dWords] ( const char * pLine, int iLen ) { dWords.Add ( Str_t { pLine, iLen } ); } );

	if ( dWords.GetLength()>=4 && StrEq ( dWords[0], "Buddy" ) && StrEq ( dWords[2], "started" ) )
		return std::make_pair ( dWords[1], dWords[3] );

	return std::make_pair ( Str_t(), Str_t() );
}

static std::pair<Str_t, Str_t> GetLines ( Str_t tSrc )
{
	std::pair<Str_t, Str_t> tLines { tSrc, Str_t() };

	char * sStart = const_cast<char *>( tSrc.first );
	if ( !tSrc.second )
	{
		sStart[tSrc.second] = '\0';
		return tLines;
	}

	const char * sEnd = sStart + tSrc.second - 1;
	char * sCur = sStart;
	while ( sCur<=sEnd && *sCur!='\n' && *sCur!='\r' )
		sCur++;

	tLines.first = Str_t( tSrc.first, sCur - sStart + 1 );

	while ( sCur<=sEnd && sphIsSpace ( *sCur ) )
		sCur++;

	tLines.second = Str_t( sCur, sEnd - sCur + 1 );

	// check the rest of text for only empty lines
	while ( sCur<=sEnd && !sphIsSpace ( *sCur ) )
		sCur++;

	if ( sCur==sEnd )
		tLines.second.second = 0;

	return tLines;
}

static void AddTail ( Str_t tLine )
{
	char * pDst = g_dLogBuf.AddN ( tLine.second );
	memcpy ( pDst, tLine.first, tLine.second );
}

static bool HasLineEnd ( Str_t tBuf, Str_t tLine )
{
	const char * sEnd = tBuf.first + tBuf.second;
	return ( ( tLine.first + tLine.second )<sEnd );
}

static void DaemonLogBuddyLine ( Str_t tLine )
{
	// if some message already at the buffer - lets copy the tail there and print the whole message from the buffer - not from the message
	if ( g_dLogBuf.GetLength() )
	{
		AddTail ( tLine );
		tLine = g_dLogBuf;
	}

	const int LOG_LINE_HEADER = 60; // daemon adds timestamp, tid then our buddy header
	const int iBufMax = GetDaemonLogBufSize() - LOG_LINE_HEADER;

	while ( tLine.second>0 )
	{
		int iLen = Min ( iBufMax, tLine.second );
		sphInfo ( "[BUDDY] %.*s", iLen, tLine.first );
		tLine.first += iLen;
		tLine.second -= iLen;
	}

	if ( g_dLogBuf.GetLength() )
		g_dLogBuf.Resize ( 0 );
}

static void LogPipe ( Str_t tSrc )
{
	CSphVector<Str_t> dLines;
	sphSplitApply ( tSrc.first, tSrc.second, "\n\r", [&dLines] ( const char * pLine, int iLen ) { dLines.Add ( Str_t { pLine, iLen } ); } );

	if ( !dLines.GetLength() )
		return;

	// whole pipe buffer without line end - collect into line buffer
	Str_t tLine0 = dLines[0];
	if ( !HasLineEnd ( tSrc, tLine0 ) )
	{
		AddTail ( tLine0 );
		return;
	}

	DaemonLogBuddyLine ( tLine0 );
	if ( dLines.GetLength()==1 )
		return;

	for ( int i=1; i<dLines.GetLength()-1; i++ )
		DaemonLogBuddyLine ( dLines[i] );

	Str_t tLineLast = dLines.Last();
	// last line could be without line end - collect into line buffer
	if ( HasLineEnd ( tSrc, tLineLast ) )
	{
		DaemonLogBuddyLine ( tLineLast );
	} else
	{
		AddTail ( tLineLast );
	}
}

static void ReadFromPipe ( const boost::system::error_code & tGotCode, std::size_t iSize )
{
	if ( tGotCode.failed() )
		return;

	if ( !iSize )
		return;

	Str_t sLineRef ( g_dPipeBuf.data(), (int)iSize );

	// regular work log all lines from the buddy output
	if ( g_eBuddy!=BuddyState_e::STARTING )
	{
		LogPipe ( sLineRef );
		return;
	}

	auto [sLine, sLinesTail] = GetLines ( sLineRef );

	// at the BuddyState_e::STARTING parsing buddy output
	auto [sBuddyVer, sBuddyAddr] = CheckLine ( sLine );
	if ( IsEmpty ( sBuddyAddr ) )
	{
		g_eBuddy = BuddyState_e::FAILED;
		sphWarning ( "[BUDDY] invalid output, should be 'Buddy ver, started address:port', got '%.*s'", sLineRef.second, sLineRef.first );
		return;
	}

	CSphString sError;
	CSphString sAddr ( sBuddyAddr );
	ListenerDesc_t tListen = ParseListener ( sAddr.cstr(), &sError );
	if ( tListen.m_eProto==Proto_e::UNKNOWN || !sError.IsEmpty() )
	{
		g_eBuddy = BuddyState_e::FAILED;
		sphWarning ( "[BUDDY] invalid output, should be 'Buddy ver, started address:port', got '%.*s', parse error: %s", sLineRef.second, sLineRef.first, sError.cstr() );
		return;
	}

	// buddy really started and ready to serve queries
#ifdef _WIN32
	tListen.m_iPort = g_tBuddyPort;
#endif
	g_sUrlBuddy = GetUrl( tListen );
	g_eBuddy = BuddyState_e::WORK;
	g_iRestartCount = 0;
	sphInfo ( "[BUDDY] started %.*s '%s' at %s", sBuddyVer.second, sBuddyVer.first, g_sStartArgs.cstr(), g_sUrlBuddy.cstr() );
	if ( sLinesTail.second )
		LogPipe ( sLinesTail );

	if ( !g_bBuddyVersion )
	{
		g_bBuddyVersion = true;
		g_sStatusVersion.SetSprintf ( "%s (buddy %.*s)", g_sStatusVersion.cstr(), sBuddyVer.second, sBuddyVer.first );
	}
}

static void BuddyPipe_fn ( const boost::system::error_code & tGotCode, std::size_t iSize )
{
	ReadFromPipe ( tGotCode, iSize );
	if ( g_pPipe && iSize )
		g_pPipe->async_read_some( boost::asio::buffer ( g_dPipeBuf ), BuddyPipe_fn );
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

static void BuddyWorkLoop()
{
	auto pDesc = PublishSystemInfo ( "buddy check" );

	while ( !sphInterrupted() )
	{
		while ( !g_pIOS->stopped() )
		{
			if ( !g_pIOS->poll_one() )
				break;
		}
		g_pIOS->restart();

		if ( g_eBuddy==BuddyState_e::STARTING && sphMicroTimer()>( g_tmStarting + g_iStartMaxTimeout * 1000 * 1000  ) )
		{
			sphWarning ( "[BUDDY] failed to start after %d sec", g_iStartMaxTimeout );
			BuddyStop();
			break;
		}

		if ( g_eBuddy==BuddyState_e::WORK )
			g_eBuddy = BuddyCheckLive();

		BuddyTryRestart();

		if ( g_eBuddy==BuddyState_e::STARTING || g_eBuddy==BuddyState_e::WORK )
		{
			Threads::Coro::SleepMsec ( g_iBuddyLoopSleep );
		} else
		{
			break;
		}
	}
}

BuddyState_e TryToStart ( const char * sArgs, CSphString & sError )
{
	std::string sCmd = sArgs;
	g_pBuddy.reset();
	if ( g_pIOS )
		g_pIOS->stop();

	g_pPipe.reset();
	g_pIOS.reset();

	g_pIOS.reset ( new boost::asio::io_context );
	g_pPipe.reset ( new boost::process::async_pipe ( *g_pIOS ) );

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
	if ( g_pPipe )
		g_pPipe->async_read_some( boost::asio::buffer ( g_dPipeBuf ), BuddyPipe_fn );

	return BuddyState_e::STARTING;
}

CSphString GetUrl ( const ListenerDesc_t & tDesc )
{
    CSphString sURI;

#ifdef _WIN32
    // Use the constant host for Windows
    sURI.SetSprintf ("http://host.docker.internal:%d", tDesc.m_iPort);
#else
    // Original code for other systems
    char sAddrBuf [ SPH_ADDRESS_SIZE ];
    sphFormatIP ( sAddrBuf, sizeof(sAddrBuf), tDesc.m_uIP );
    sURI.SetSprintf ( "http://%s:%d", sAddrBuf, tDesc.m_iPort );
#endif

    return sURI;
}

static void SetContainerName ( const CSphString & sConfigPath )
{
	DWORD uName = sphCRC32 ( sConfigPath.cstr() );
	g_sContainerName.SetSprintf ( "buddy_%u", uName );
}

static void BuddyStopContainer()
{
#ifdef _WIN32
	CSphString sCmd;
	sCmd.SetSprintf ( "docker kill %s", g_sContainerName.cstr() );
	std::error_code tErrorCode;
	boost::process::child tStop ( sCmd.cstr(), boost::process::limit_handles, boost::process::error ( tErrorCode ) );
	tStop.wait();
#endif
}

void BuddyStart ( const CSphString & sConfigPath, const CSphString & sPluginDir, bool bHasBuddyPath, const VecTraits_T<ListenerDesc_t> & dListeners, bool bTelemetry, int iThreads, const CSphString & sConfigFilePath, const CSphString & sDataDir )
{
	const char* szHelperUrl = getenv ( "MANTICORE_HELPER_URL" );
	if ( szHelperUrl )
	{
		// debug mode - don't start anything and consider env url valid and available
		// - can start any kind of helper externally and just route to it using provided URL
		g_sUrlBuddy = szHelperUrl;
		g_eBuddy = BuddyState_e::WORK;
		return;
	}

	SetContainerName ( sConfigFilePath );
	// should not check buddy related code if buddy disabled at config
	if ( bHasBuddyPath && sConfigPath.IsEmpty() )
		return;

	ARRAY_FOREACH ( i, dListeners )
	{
		const ListenerDesc_t & tDesc = dListeners[i];
		if ( tDesc.m_eProto==Proto_e::SPHINX || tDesc.m_eProto==Proto_e::HTTP )
		{
#ifdef _WIN32
			g_pBuddyPortList.reset ( PortRange::Create ( "127.0.0.1", tDesc.m_iPort+100, 20 ) );
			g_tBuddyPort = g_pBuddyPortList->AcquirePort();
#endif
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

	CSphString sPath = BuddyGetPath ( sConfigPath, sPluginDir, bHasBuddyPath, (int)g_tBuddyPort, sDataDir );
	if ( sPath.IsEmpty() )
		return;

	// at WINDOWS need to stop docker conteiner that could left from the previous run or after daemon got crashed
	BuddyStopContainer();

	g_dLogBuf.Resize ( 0 );
	g_sPath = sPath;

	g_sStartArgs.SetSprintf ( "%s --listen=%s %s %s --threads=%d",
		g_sPath.cstr(),
		g_sListener4Buddy.cstr(),
		g_sBuddyBind.scstr(),
		( bTelemetry ? "" : "--disable-telemetry" ),
		iThreads );

	sphLogDebug ( "[BUDDY] start args: %s", g_sStartArgs.cstr() );

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
	TaskManager::StartJob ( g_iTask, BuddyWorkLoop );
}

void BuddyStop ()
{
#if _WIN32
	if ( g_pBuddy )
	{
		std::error_code tErrorCode;
		g_pBuddy->terminate ( tErrorCode );
		if ( tErrorCode )
			sphWarning ( "[BUDDY] stopped, exit code: %d", tErrorCode.value() );
		BuddyStopContainer();
	}
#else
	if ( g_pBuddy )
	{
		// FIXME!!! migrate to boost::process::v2 and use
		// proc.request_exit();
		// proc.wait();
		kill ( g_pBuddy->id(), SIGTERM );
		std::error_code tErrorCode;
		g_pBuddy->wait ( tErrorCode );
		if ( tErrorCode )
			sphLogDebug ( "[BUDDY] stopped, exit code: %d", tErrorCode.value() );
	}
#endif

	g_eBuddy = BuddyState_e::STOPPED;
	g_pBuddy.reset();
}

void BuddyShutdown ()
{
	BuddyStop();
	g_tBuddyPort = ScopedPort_c();
	g_pBuddyPortList.reset();
}


bool HasBuddy()
{
	return ( g_eBuddy==BuddyState_e::WORK );
}

static bool BuddyQueryAddErrorBody ( JsonEscapedBuilder & tBuddyQuery, const VecTraits_T<BYTE> & dSrcHttpReply )
{
	if ( !dSrcHttpReply.GetLength() )
		return false;

	const char * sErrorStart = (const char *)dSrcHttpReply.Begin();
	const char * sBodyDel = strstr ( sErrorStart, "\r\n\r\n" );
	if ( !sBodyDel )
		return false;
	const char * sBodyStart = sBodyDel + 4;
	if ( (sBodyDel - sErrorStart )>dSrcHttpReply.GetLength() )
		return false;

	int iBodyLen = ( sErrorStart + dSrcHttpReply.GetLength() ) - sBodyStart;
	Str_t sBodyBuf ( sBodyStart, iBodyLen );

	JsonObj_c tError ( sBodyBuf  );
	if ( tError.Empty() )
		return false;

	tBuddyQuery.NamedValNE ( "body", sBodyBuf );
	return true;
}

static std::pair<bool, CSphString> BuddyQuery ( bool bHttp, Str_t sQueryError, Str_t sPathQuery, Str_t sQuery, http_method eRequestType, const VecTraits_T<BYTE> & dSrcHttpReply )
{
	if ( !HasBuddy() )
		return { false, {} };

	JsonEscapedBuilder tBuddyQuery;
	{
		auto tRoot = tBuddyQuery.Object();
		tBuddyQuery.NamedString ( "type", bHttp ? "unknown json request" : "unknown sql request" );
		{
			tBuddyQuery.Named ( "error" );
			auto tMessageRoot = tBuddyQuery.Object();

			tBuddyQuery.NamedString ( "message", sQueryError );
			if ( !BuddyQueryAddErrorBody ( tBuddyQuery, dSrcHttpReply ) )
				tBuddyQuery.NamedValNE ( "body", "null" );
		}
		tBuddyQuery.NamedVal ( "version", g_iBuddyVersion );
		if ( !bHttp )
			tBuddyQuery.NamedString ( "user", session::GetClientSession()->m_sUser );

		{
			tBuddyQuery.Named ( "message" );
			auto tMessageRoot = tBuddyQuery.Object();
			tBuddyQuery.NamedString ( "path_query", sPathQuery );
			tBuddyQuery.NamedString ( "body", sQuery );
			tBuddyQuery.NamedString ( "http_method", ( bHttp ? http_method_str ( eRequestType ) : "" ) );
		}
	}

	StrVec_t dHeaders;
	dHeaders.Add ( SphSprintf ( "Request-ID: %d_%u", session::GetConnID(), sphCRC32 ( sQuery.first, sQuery.second, sphRand() ) ) );
	// disable Expect: 100-continue
	// as Expect: 100-continue header added by curl library do not with the buddy
	dHeaders.Add ( "Expect:" );

	return PostToHelperUrl ( g_sUrlBuddy, (Str_t)tBuddyQuery, dHeaders );
}

bool IsBuddyQuery ( const OptionsHash_t & hOptions )
{
	CSphString * pProhibit = hOptions ( "user-agent" );
	return pProhibit != nullptr && ( pProhibit->Begins ( "Manticore Buddy" ) );
}

struct BuddyReply_t
{
	JsonObj_c m_tRoot;

	CSphString m_sType;
	JsonObj_c m_tMessage;
	int m_iReplyHttpCode = 0;
};

static bool ParseReply ( char * sReplyRaw, BuddyReply_t & tParsed, CSphString & sError )
{
	tParsed.m_tRoot = JsonObj_c ( sReplyRaw );
	if ( !tParsed.m_tRoot )
	{
		sError.SetSprintf ( "unable to parse: %s", tParsed.m_tRoot.GetErrorPtr() );
		return false;
	}

	if ( !tParsed.m_tRoot.IsObj() )
	{
		sError.SetSprintf ( "wrong reply format - not object" );
		return false;
	}

	int iVer = 0;
	if ( !tParsed.m_tRoot.FetchIntItem ( iVer, "version", sError, false ) )
		return false;
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

	if ( !tParsed.m_tRoot.FetchStrItem ( tParsed.m_sType, "type", sError, false ) )
		return false;

	tParsed.m_tMessage = tParsed.m_tRoot.GetItem ( "message" );
	if ( tParsed.m_tMessage.Empty() )
		return false;

	if ( !tParsed.m_tRoot.FetchIntItem ( tParsed.m_iReplyHttpCode, "error_code", sError, false ) )
		return false;

	return true;
}

static EHTTP_STATUS GetHttpStatusCode ( int iBuddyHttpCode, EHTTP_STATUS eReqHttpCode )
{
	return ( iBuddyHttpCode>0 ? HttpGetStatusCodes ( iBuddyHttpCode ) : eReqHttpCode );
}

template<typename T>
bool ConvertValue ( const char * sName, const JsonObj_c & tMeta, T & tVal )
{
	JsonObj_c tSrcVal = tMeta.GetItem ( sName );
	if ( !tSrcVal )
		return false;

	if ( !tSrcVal.IsStr() )
		return false;

	int64_t iVal = 0;
	double fVal = 0.0;
	ESphJsonType eType;
	if ( !sphJsonStringToNumber ( tSrcVal.SzVal(), strlen ( tSrcVal.SzVal() ), eType, iVal, fVal ) )
		return false;

	if ( eType==JSON_INT64 )
		tVal = (T)iVal;
	else
		tVal = (T)fVal;
	return true;
}

static bool SetSessionMeta ( const JsonObj_c & tBudyyReply )
{
	ClientSession_c * pSession = session::GetClientSession();
	if ( !pSession )
		return false;

	auto & tLastMeta = pSession->m_tLastMeta;
	tLastMeta = CSphQueryResultMeta();

	CSphString sTmpError;
	JsonObj_c tSrcMeta = tBudyyReply.GetObjItem ( "meta", sTmpError, true );
	if ( !tSrcMeta )
		return false;

	// total => m_iMatches
	// if no meta.total this is not a search query - do not log it into query.log
	if ( !ConvertValue ( "total", tSrcMeta, tLastMeta.m_iMatches ) )
		return false;
		
	// total_found => m_iTotalMatches
	ConvertValue ( "total_found", tSrcMeta, tLastMeta.m_iTotalMatches );

	// time => m_iQueryTime \ m_iRealQueryTime
	float fTime = 0.0f;
	if ( ConvertValue ( "time", tSrcMeta, fTime ) )
		tLastMeta.m_iRealQueryTime = tLastMeta.m_iQueryTime = (int)( fTime * 1000.0f );

	// total_relation => m_bTotalMatchesApprox
	CSphString sRel;
	if ( tSrcMeta.FetchStrItem ( sRel, "total_relation", sTmpError, true ) && !sRel.IsEmpty() && sRel=="gte" )
		tLastMeta.m_bTotalMatchesApprox = true;

	return true;
}

// we call it ALWAYS, because even with absolutely correct result, we still might reject it for '/cli' endpoint if buddy is not available or prohibited
bool ProcessHttpQueryBuddy ( HttpProcessResult_t & tRes, Str_t sSrcQuery, OptionsHash_t & hOptions, CSphVector<BYTE> & dResult, bool bNeedHttpResponse, http_method eRequestType )
{
	if ( tRes.m_bOk || !HasBuddy() || tRes.m_eEndpoint==EHTTP_ENDPOINT::INDEX || IsBuddyQuery ( hOptions ) )
	{
		if ( tRes.m_eEndpoint==EHTTP_ENDPOINT::CLI )
		{
			if ( !HasBuddy() )
				tRes.m_sError.SetSprintf ( "can not process /cli endpoint without buddy" );
			else if ( IsBuddyQuery ( hOptions ) )
				tRes.m_sError.SetSprintf ( "can not process /cli endpoint with User-Agent:Manticore Buddy" );
			sphHttpErrorReply ( dResult, EHTTP_STATUS::_501, tRes.m_sError.cstr() );
		}

		assert ( dResult.GetLength()>0 );
		return tRes.m_bOk;
	}

	myinfo::SetCommand ( sSrcQuery.first );
	AT_SCOPE_EXIT ( []() { myinfo::SetCommandDone(); } );

	bool bHttpEndpoint = true;
	if ( tRes.m_eEndpoint==EHTTP_ENDPOINT::SQL )
	{
		bHttpEndpoint = false;

		// sql parser put \0 at error position at the reference string
		// should use raw_query for buddy request
		CSphString * pRawQuery = hOptions ( "raw_query" );
		if ( pRawQuery && !pRawQuery->IsEmpty() )
		{
			sSrcQuery = FromStr ( *pRawQuery );

			const char sQueryHead[] = "query=";
			const int iQueryHeadLen = sizeof ( sQueryHead )-1;

			const char * sHead = strstr ( pRawQuery->cstr(), sQueryHead );
			if ( sHead )
			{
				const char * sOnlyQuery = sHead + iQueryHeadLen;
				int iLen = strlen ( sOnlyQuery );
				sSrcQuery = Str_t ( sOnlyQuery, iLen );
			}
		}
	}
	auto tReplyRaw = BuddyQuery ( bHttpEndpoint, FromStr ( tRes.m_sError ), FromStr ( hOptions["full_url"] ), sSrcQuery, eRequestType, dResult );
	if ( !tReplyRaw.first )
	{
		sphWarning ( "[BUDDY] [%d] error: %s", session::GetConnID(), tReplyRaw.second.cstr() );
		return tRes.m_bOk;
	}

	CSphString sError;
	BuddyReply_t tReplyParsed;
	if ( !ParseReply ( const_cast<char *>( tReplyRaw.second.cstr() ), tReplyParsed, sError ) )
	{
		sphWarning ( "[BUDDY] [%d] %s: %s", session::GetConnID(), sError.cstr(), tReplyRaw.second.cstr() );
		return tRes.m_bOk;
	}
	if ( ( bHttpEndpoint && tReplyParsed.m_sType!="json response" ) || ( !bHttpEndpoint && tReplyParsed.m_sType!="sql response" ) )
	{
		sphWarning ( "[BUDDY] [%d] wrong response type %s: %s", session::GetConnID(), tReplyParsed.m_sType.cstr(), tReplyRaw.second.cstr() );
		return tRes.m_bOk;
	}

	CSphString sDumpBuf;
	Str_t sDump;
	if ( tReplyParsed.m_tMessage.IsStr() )
	{
		sDump = FromSz ( tReplyParsed.m_tMessage.SzVal() );

	} else
	{
		CSphVector<BYTE> dBson;
		bson::JsonObjToBson ( tReplyParsed.m_tMessage, dBson, true, false );
		bson::Bson_c ( dBson ).BsonToJson ( sDumpBuf, false );
		sDump = FromStr ( sDumpBuf );
	}

	EHTTP_STATUS eHttpStatus = GetHttpStatusCode ( tReplyParsed.m_iReplyHttpCode, tRes.m_eReplyHttpCode );

	dResult.Resize ( 0 );
	ReplyBuf ( FromStr ( sDump ), eHttpStatus, bNeedHttpResponse, dResult );
	
	if ( SetSessionMeta ( tReplyParsed.m_tRoot ) )
		LogBuddyQuery ( sSrcQuery, BuddyQuery_e::HTTP );

	return true;
}

static bool ConvertErrorMessage ( const Str_t & sStmt, std::pair<int, BYTE> tSavedPos, BYTE & uPacketID, const JsonObj_c & tMessage, GenericOutputBuffer_c & tOut )
{
	if ( !tMessage.IsObj() )
		return false;

	CSphString sTmp;
	CSphString sMsgError;
	if ( !tMessage.FetchStrItem ( sMsgError, "error", sTmp, false ) )
		return false;

	// reset back out buff and packet
	uPacketID = tSavedPos.second;
	tOut.Rewind ( tSavedPos.first );
	std::unique_ptr<RowBuffer_i> tBuddyRows ( CreateSqlRowBuffer ( &uPacketID, &tOut ) );

	LogSphinxqlError ( sStmt, FromStr ( sMsgError ) );
	session::GetClientSession()->m_sError = sMsgError;
	session::GetClientSession()->m_tLastMeta.m_sError = sMsgError;
	tBuddyRows->Error ( sMsgError.cstr() );
	return true;
}

void ProcessSqlQueryBuddy ( Str_t sSrcQuery, Str_t tError, std::pair<int, BYTE> tSavedPos, BYTE & uPacketID, GenericOutputBuffer_c & tOut )
{
	auto tReplyRaw = BuddyQuery ( false, tError, Str_t(), sSrcQuery, HTTP_GET, VecTraits_T<BYTE>() );
	if ( !tReplyRaw.first )
	{
		LogSphinxqlError ( sSrcQuery.first, tError );
		sphWarning ( "[BUDDY] [%d] error: %s", session::GetConnID(), tReplyRaw.second.cstr() );
		return;
	}

	CSphString sError;
	BuddyReply_t tReplyParsed;
	if ( !ParseReply ( const_cast<char *>( tReplyRaw.second.cstr() ), tReplyParsed, sError ) )
	{
		LogSphinxqlError ( sSrcQuery.first, tError );
		sphWarning ( "[BUDDY] [%d] %s: %s", session::GetConnID(), sError.cstr(), tReplyRaw.second.cstr() );
		return;
	}
	if ( tReplyParsed.m_sType!="sql response" )
	{
		LogSphinxqlError ( sSrcQuery.first, tError );
		sphWarning ( "[BUDDY] [%d] wrong response type %s: %s", session::GetConnID(), tReplyParsed.m_sType.cstr(), tReplyRaw.second.cstr() );
		return;
	}

	if ( !tReplyParsed.m_tMessage.IsArray() )
	{
		if ( ConvertErrorMessage ( sSrcQuery, tSavedPos, uPacketID, tReplyParsed.m_tMessage, tOut ) )
			return;

		LogSphinxqlError ( sSrcQuery.first, tError );
		sphWarning ( "[BUDDY] [%d] wrong reply format - not cli reply array: %s", session::GetConnID(), tReplyRaw.second.cstr() );
		return;
	}

	// reset back out buff and packet
	uPacketID = tSavedPos.second;
	tOut.Rewind ( tSavedPos.first );
	std::unique_ptr<RowBuffer_i> tBuddyRows ( CreateSqlRowBuffer ( &uPacketID, &tOut ) );

	ConvertJsonDataset ( tReplyParsed.m_tMessage, sSrcQuery.first, *tBuddyRows );

	if ( SetSessionMeta ( tReplyParsed.m_tRoot ) )
		LogBuddyQuery ( sSrcQuery, BuddyQuery_e::SQL );
}

#ifdef _WIN32
CSphString BuddyGetPath ( const CSphString & sConfigPath, const CSphString & , bool bHasBuddyPath, int iHostPort, const CSphString & sDataDir )
{
	if ( bHasBuddyPath )
		return sConfigPath;

	const char * sDefaultBuddyName ( "manticore-buddy" );
	const char * sDefaultBuddyDockerImage ( "manticoresearch/manticore-executor:" BUDDY_EXECUTOR_VERNUM );

	StringBuilder_c sCmd ( " " );
	sCmd.Appendf ( "docker run --rm" ); // the head of the docker start command
	sCmd.Appendf ( "-p %d:9999", iHostPort ); // port mapping
	sCmd.Appendf ( "-v \"%s/%s\":/buddy", GET_MANTICORE_MODULES(), sDefaultBuddyName ); // volume for buddy modules
	sCmd.Appendf ( "-v manticore-usr_local_lib_manticore:/usr/local/lib/manticore -e PLUGIN_DIR=/usr/local/lib/manticore" ); // pesistent volume for buddy data
	if ( !sDataDir.IsEmpty() ) // volume for data dir into container
		sCmd.Appendf ( "-v \"%s\":/var/lib/manticore -e DATA_DIR=/var/lib/manticore", sDataDir.cstr() );
	sCmd.Appendf ( "-w /buddy" ); // workdir is buddy root dir
	sCmd.Appendf ( "--name %s", g_sContainerName.cstr() ); // the name of the buddy container is the hash of the config
	sCmd.Appendf ( "%s /buddy/src/main.php", sDefaultBuddyDockerImage ); // docker image and the buddy start command

	return CSphString ( sCmd );
}
#else
CSphString BuddyGetPath ( const CSphString & sConfigPath, const CSphString & sPluginDir, bool bHasBuddyPath, int iHostPort, const CSphString & )
{
	if ( bHasBuddyPath )
		return sConfigPath;

	const char * sExecutor = "manticore-executor";
	const char * sDefaultBuddyName = "manticore-buddy/src/main.php";

	CSphString sFullPath;
	CSphString sPathToDaemon = GetPathOnly ( GetExecutablePath() );

	CSphString sPathBuddy2Module;
	sPathBuddy2Module.SetSprintf ( "%s/%s", GET_MANTICORE_MODULES(), sDefaultBuddyName );
	if ( sphFileExists ( sPathBuddy2Module.cstr() ) )
	{
		sFullPath.SetSprintf ( "%s %s", sExecutor, sPathBuddy2Module.cstr() );
		return sFullPath;
	}

	// check at the daemon location / cwd
	CSphString sPathBuddy2Cwd;
	sPathBuddy2Cwd.SetSprintf ( "%s%s", sPathToDaemon.cstr(), sDefaultBuddyName );
	if ( sphFileExists ( sPathBuddy2Cwd.cstr() ) )
	{
		sFullPath.SetSprintf ( "%s %s", sExecutor, sPathBuddy2Cwd.cstr() );
		return sFullPath;
	}

	sphWarning ( "[BUDDY] no %s found at '%s', disabled", sDefaultBuddyName, sPathBuddy2Module.cstr() );
	return sFullPath;
}
#endif
