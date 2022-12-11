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

#include <boost/process.hpp>
#if _WIN32
#include <boost/winapi/process.hpp>
#endif

#include "netfetch.h"
#include "searchdbuddy.h"

static std::unique_ptr<boost::process::child> g_pBuddy;
static std::unique_ptr<boost::process::ipstream> g_tBuddyOut;
static CSphString g_sPath;
static CSphString g_sListener4Buddy;
static int g_iBuddyVersion = 1;
static bool g_bTelemetry { true };

static CSphString g_sUrlBuddy;

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

static int64_t g_iTM = 0;
static int g_iTask = 0;
static void NextLog();
static bool BuddyStart();

static void BuddyCheck()
{
	auto pDesc = PublishSystemInfo ( "buddy check" );

	//sphInfo ( "at log " INT64_FMT, g_iTM );
	std::string sLine;
	std::getline ( *g_tBuddyOut, sLine );
	//std::string sLine ( std::istreambuf_iterator<char> ( *g_tBuddyOut ), {} );
	if ( !sLine.empty() )
		sphInfo ( "[BUDDY] %s", sLine.c_str() );

	std::error_code tErrorCode;
	if ( g_pBuddy && g_pBuddy->running ( tErrorCode ) )
		NextLog ();
	else if ( g_pBuddy ) // restart buddy
	{
		sphWarning ( "[BUDDY] terminated, exit code %d", tErrorCode.value() );
		BuddyStart ();
		if ( g_pBuddy && g_pBuddy->running ( tErrorCode ) )
		{
			sphInfo ( "[BUDDY] restarted" );
			NextLog ();
		} else
		{
			BuddyStop();
		}
	}
}

void NextLog()
{
	g_iTM = sphMicroTimer();
	TaskManager::ScheduleJob ( g_iTask, g_iTM + 15000, BuddyCheck );
}

typedef std::function<bool ( const std::string & )> StartCnd_fn;

static bool TryToStart ( const char * sArgs, CSphString & sError, StartCnd_fn tCnd )
{
	std::string sCmd = sArgs;
	g_pBuddy.reset();
	g_tBuddyOut.reset ( new boost::process::ipstream );

	std::unique_ptr<boost::process::child> pBuddy;
	std::error_code tErrorCode;

#if _WIN32
	BuddyWindow_t tWnd;
	//pBuddy.reset ( new boost::process::child ( sCmd, tWnd ) );
	//pBuddy.reset ( new boost::process::child ( sCmd, ( boost::process::std_out & boost::process::std_err ) > *g_tBuddyOut, tWnd ) );
	pBuddy.reset ( new boost::process::child ( sCmd, ( boost::process::std_out & boost::process::std_err ) > *g_tBuddyOut, tWnd, boost::process::limit_handles, boost::process::error ( tErrorCode ) ) );
#else
	PreservedStd_t tPreserveStd;
	pBuddy.reset ( new boost::process::child ( sCmd, ( boost::process::std_out & boost::process::std_err ) > *g_tBuddyOut, boost::process::limit_handles, boost::process::error ( tErrorCode ) , tPreserveStd ) );
#endif

	//sphInfo ( "[BUDDY] prior line" ); // !COMMIT
	std::string sLine;
	while ( !tErrorCode && pBuddy->running ( tErrorCode ) )
	{
		//sphInfo ( "[BUDDY] prior get-line" ); // !COMMIT
		if ( std::getline ( *g_tBuddyOut, sLine ) && tCnd ( sLine ) )
			break;

		//sphInfo ( "[BUDDY] got-line %s, len %d", sLine.c_str(), (int)sLine.length() ); // !COMMIT
		if ( !sLine.empty() )
			sphInfo ( "[BUDDY] %s", sLine.c_str() );
	}

	//sphInfo ( "[BUDDY] aftert line %s, code %d, run %d", sLine.c_str(), (int)tErrorCode.value(), (int)pBuddy->running ( tErrorCode ) ); // !COMMIT
	if ( !pBuddy->running ( tErrorCode ) || !tCnd ( sLine ) )
	{
		std::string sTmp ( std::istreambuf_iterator<char> ( *g_tBuddyOut ), {} );
		sError.SetSprintf ( "'%s' terminated with exit code %d, %s", sArgs, tErrorCode.value(), ( sTmp.empty() ? sLine.c_str() : sTmp.c_str() ) );
		return false;
	}

	g_pBuddy = std::move ( pBuddy );

	return g_pBuddy->running ( tErrorCode );
}

struct BuddyListenOut_t
{
	const char * m_sBuddyAddr { "started" };
	CSphString m_sOutput;
	bool GotLine ( const std::string & sLine )
	{
		//sphInfo ( "[BUDDY] line %s", sLine.c_str() ); // !COMMIT

		int iPos = sLine.find ( m_sBuddyAddr );
		if ( iPos!=std::string::npos )
		{
			int iHeaderLen = strlen ( m_sBuddyAddr );
			int iAddrStart = iPos + iHeaderLen;
			int iAddrLen = sLine.length() - iAddrStart;
			assert ( iAddrLen>0 );
			m_sOutput.SetBinary ( sLine.c_str() + iAddrStart, iAddrLen );
		}

		return ( iPos!=std::string::npos );
	}
};
static CSphString GetUrl ( const ListenerDesc_t & tDesc )
{
	char sAddrBuf [ SPH_ADDRESS_SIZE ];
	sphFormatIP ( sAddrBuf, sizeof(sAddrBuf), tDesc.m_uIP );
	
	CSphString sURI;
	sURI.SetSprintf ( "http://%s:%d", sAddrBuf, tDesc.m_iPort );

	return sURI;
}


void BuddyStart ( const CSphString & sPath, const VecTraits_T<ListenerDesc_t> & dListeners, bool bTelemetry )
{
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
	g_bTelemetry = bTelemetry;

	if ( BuddyStart() )
	{
		g_iTask = TaskManager::RegisterGlobal ( "buddy service" );
		assert ( g_iTask>=0 && "failed to create buddy service task" );
		NextLog();
	} else
	{
		BuddyStop();
	}
}

bool BuddyStart ()
{
	CSphString sErorr;

	StringBuilder_c sArgs;
	sArgs.Appendf (
		"%s --listen=%s ",
		g_sPath.cstr(),
		g_sListener4Buddy.cstr()
	);
	if ( !g_bTelemetry )
		sArgs += " --disable-telemetry ";
	
	BuddyListenOut_t tOut;
	StartCnd_fn fnOut = [&tOut]( const std::string & sLine ) { return tOut.GotLine ( sLine ); };

	if ( !TryToStart ( sArgs.cstr(), sErorr, fnOut ) )
	{
		sphWarning ( "[BUDDY] failed to start buddy, %s", sErorr.cstr() );
		return false;
	}

	tOut.m_sOutput.Trim();
	if ( tOut.m_sOutput.IsEmpty() )
	{
		sphWarning ( "[BUDDY] empty output from buddy, should be 'address:port'" );
		return false;
	}

	ListenerDesc_t tListen = ParseListener ( tOut.m_sOutput.cstr(), &sErorr );
	if ( tListen.m_eProto==Proto_e::UNKNOWN || !sErorr.IsEmpty() )
	{
		sphWarning ( "[BUDDY] invalid output from buddy, should be 'address:port', got '%s', parse error: %s", tOut.m_sOutput.cstr(), sErorr.cstr() );
		return false;
	}

	g_sUrlBuddy = GetUrl( tListen );
	sphInfo ( "[BUDDY] started '%s' at %s", sArgs.cstr(), g_sUrlBuddy.cstr() );
	return true;
}

void BuddyStop ()
{
	if ( g_pBuddy )
	{
		std::error_code tErrorCode;
		g_pBuddy->terminate ( tErrorCode );
		if ( tErrorCode )
			sphWarning ( "[BUDDY] buddy stop exit code %d", tErrorCode.value() );
	}

	g_pBuddy.reset();
	g_tBuddyOut.reset();
}

bool HasBuddy()
{
	return ( g_pBuddy && !g_sUrlBuddy.IsEmpty() );
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

	return FetchHelperUrl ( g_sUrlBuddy, (Str_t)tBuddyQuery );
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
		sError.SetSprintf ( "budy reply version (%d) greater daemon version (%d), upgrade daemon binary", iVer, g_iBuddyVersion );
		return false;
	}
	if ( iVer<1 )
	{
		sError.SetSprintf ( "wrong budy reply version (%d), daemon version (%d), upgrade buddy", iVer, g_iBuddyVersion );
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
		sphWarning ( "[BUDDY] error: %s", tReplyRaw.second.cstr() );
		return tRes;
	}

	CSphString sError;
	BuddyReply_t tReplyParsed;
	if ( !ParseReply ( const_cast<char *>( tReplyRaw.second.cstr() ), tReplyParsed, sError ) )
	{
		sphWarning ( "[BUDDY] %s: %s", sError.cstr(), tReplyRaw.second.cstr() );
		return tRes;
	}
	if ( bson::String ( tReplyParsed.m_tType )!="json response" )
	{
		sphWarning ( "[BUDDY] wrong response type %s: %s", bson::String ( tReplyParsed.m_tType ).cstr(), tReplyRaw.second.cstr() );
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
		sphWarning ( "[BUDDY] error: %s", tReplyRaw.second.cstr() );
		return bKeepProfile;
	}

	CSphString sError;
	BuddyReply_t tReplyParsed;
	if ( !ParseReply ( const_cast<char *>( tReplyRaw.second.cstr() ), tReplyParsed, sError ) )
	{
		sphWarning ( "[BUDDY] %s: %s", sError.cstr(), tReplyRaw.second.cstr() );
		return bKeepProfile;
	}
	if ( bson::String ( tReplyParsed.m_tType )!="sql response" )
	{
		sphWarning ( "[BUDDY] wrong response type %s: %s", bson::String ( tReplyParsed.m_tType ).cstr(), tReplyRaw.second.cstr() );
		return bKeepProfile;
	}

	if ( bson::IsNullNode ( tReplyParsed.m_tMessage ) || !bson::IsArray ( tReplyParsed.m_tMessage ) )
	{
		const char * sReplyType = ( bson::IsNullNode ( tReplyParsed.m_tMessage ) ? "empty" : "not cli reply array" );
		sphWarning ( "[BUDDY] wrong reply format - %s: %s", sReplyType, tReplyRaw.second.cstr() );
		return bKeepProfile;
	}

	// reset back out buff and packet
	uPacketID = uRefPacketID;
	tOut.Rewind ( iRefPos );
	std::unique_ptr<RowBuffer_i> tBuddyRows ( CreateSqlRowBuffer ( &uPacketID, &tOut ) );

	ConvertJsonDataset ( tReplyParsed.m_tMessage, sQuery.first, *tBuddyRows );
	return bKeepProfile;
}