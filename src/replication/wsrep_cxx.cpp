//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "wsrep_cxx_int.h"

static const char* g_dReplicatorPatterns[] = {
	"Could not open state file for reading:",
	"No persistent state found. Bootstrapping with default state",
	"Fail to access the file (" };

bool CheckNoWarning ( const char* szMsg )
{
	if ( !szMsg || !szMsg[0] )
		return false;

	for ( const char* sPattern : g_dReplicatorPatterns )
	{
		if ( strncmp ( szMsg, sPattern, strlen ( sPattern ) ) == 0 )
			return true;
	}
	return false;
}

// callback for Galera logger_cb to log messages and errors
void WsrepLog ( LogLevel_e eLevel, const char* szMsg )
{
	// in normal flow need to skip certain messages from Galera but keep message in debug replication verbosity level
	// don't want to patch Galera source code
	if ( g_eLogLevel < SPH_LOG_RPL_DEBUG && eLevel == LogLevel_e::WARN && CheckNoWarning ( szMsg ) )
		return;

	ESphLogLevel eLevelDst = SPH_LOG_INFO;
	switch ( eLevel )
	{
	case LogLevel_e::FATAL:
	case LogLevel_e::ERROR_:
		eLevelDst = SPH_LOG_FATAL;
		break;
	case LogLevel_e::WARN:
		eLevelDst = SPH_LOG_WARNING;
		break;
	case LogLevel_e::INFO:
	case LogLevel_e::DEBUG:
	default:
		eLevelDst = SPH_LOG_RPL_DEBUG;
	}
	sphLogf ( eLevelDst, "%s", szMsg );
}

const char * Wsrep::GetViewStatus ( Wsrep::ViewStatus_e eStatus ) noexcept
{
	using namespace Wsrep;
	switch ( eStatus )
	{
	case ViewStatus_e::PRIMARY:			return "primary";
	case ViewStatus_e::NON_PRIMARY:		return "non-primary";
	case ViewStatus_e::DISCONNECTED:	return "disconnected";
	default:	return "unknown (MAX)";
	}
}

// log debug info about nodes as current nodes views that
void LogGroupView ( const Wsrep::ViewInfo_t* pView )
{
	if ( g_eLogLevel < SPH_LOG_RPL_DEBUG )
		return;

	using namespace Wsrep;

	sphLogDebugRpl ( "new cluster membership: %d(%d), global seqno: " INT64_FMT ", status %s, gap: %s",
		pView->m_iIdx, pView->m_iNMembers, pView->m_tStateId.m_iSeqNo, GetViewStatus ( pView->m_eStatus ), pView->m_bGap ? "true" : "false" );

	StringBuilder_c sBuf;
	const MemberInfo_t* pBoxes = &pView->m_tMemInfo;
	for ( int i = 0; i < pView->m_iNMembers; ++i )
		sBuf.Appendf ( "\n'%s', '%s' %s", pBoxes[i].m_sName, pBoxes[i].m_sIncoming, ( i == pView->m_iIdx ? "*" : "" ) );

	sphLogDebugRpl ( "%s", sBuf.IsEmpty() ? "no nodes" : sBuf.cstr() );
}

DEFINE_RENDER ( GaleraInfo_t )
{
	auto& tInfo = *(GaleraInfo_t*)const_cast<void*> ( pSrc );
	dDst.m_sChain << "Repl ";
	dDst.m_sClientName << "wsrep " << tInfo.m_sName.cstr();
}


CSphString Wsrep::Uuid2Str ( const Wsrep::UUID_t& tUuid )
{
	auto h = [&tUuid] ( int idx ) { return FixedNum_T<BYTE, 16, 2, 0, '0'> ( tUuid[idx] ); };
	CSphString sResult;
	StringBuilder_c{}.Sprint (h(0),h(1),h(2),h(3),'-',h(4),h(5),'-',h(6),h(7),'-',h(8),h(9),'-',h(10),h(11),h(12),h(13),h(14),h(15)).MoveTo(sResult);
	return sResult;
}

CSphString Wsrep::Gtid2Str ( const Wsrep::GlobalTid_t& tGtid )
{
	CSphString sResult;
	StringBuilder_c {}.Sprint ( Wsrep::Uuid2Str ( tGtid.m_tUuid ), ':', tGtid.m_iSeqNo ).MoveTo ( sResult );
	return sResult;
}

#if __has_include( <charconv>)
template <typename INT>
inline static std::from_chars_result from_chars_wrap ( const char* pBegin, const char* pEnd, INT& iValue, int iBase=10 )
{
	return std::from_chars ( pBegin, pEnd, iValue, iBase );
}
#else
struct from_chars_result
{
	const char* ptr;
	std::errc ec;
};
inline static from_chars_result from_chars_wrap ( const char* pBegin, const char* pEnd, int64_t& iValue )
{
	from_chars_result tRes;
	iValue = strtoll ( pBegin, (char**)&tRes.ptr, 10 );
	if ( tRes.ptr < pEnd )
		tRes.ec = std::errc::invalid_argument;
	return tRes;
}

inline static from_chars_result from_chars_wrap ( const char* pBegin, const char* pEnd, BYTE& uValue, int iBase )
{
	assert ( iBase == 16 );
	from_chars_result tRes { pBegin, std::errc::invalid_argument };

	auto Char2Hex = [] ( BYTE uChar ) -> int {
		switch ( uChar ) {
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a':
		case 'A': return 10;
		case 'b':
		case 'B': return 11;
		case 'c':
		case 'C': return 12;
		case 'd':
		case 'D': return 13;
		case 'e':
		case 'E': return 14;
		case 'f':
		case 'F': return 15;
		default: return -1;
		}
	};

	auto iVal = Char2Hex ( *tRes.ptr );
	if ( iVal < 0 )
		return tRes;
	uValue = iVal << 4;
	++tRes.ptr;
	iVal = Char2Hex ( *tRes.ptr );
	if ( iVal < 0 )
		return tRes;
	uValue += iVal;
	++tRes.ptr;
	tRes.ec = static_cast<std::errc> ( 0 );
	return tRes;
}
#endif


const char * Str2UuidImpl ( Wsrep::UUID_t& dRes, const CSphString & sUuid )
{
	int iUuidBytes = 0;
	const char* pSrc = sUuid.cstr();
	const char* pDigitEnd = pSrc + 2;
	const char* pEnd = pSrc + sUuid.Length();
	while ( pDigitEnd <= pEnd && iUuidBytes < 16 )
	{
		auto tChars = from_chars_wrap ( pSrc, pDigitEnd, dRes[iUuidBytes++], 16 );
		if ( tChars.ec == std::errc::invalid_argument )
			break;
		pSrc = tChars.ptr + ( iUuidBytes != 16 && tChars.ptr < pEnd && *tChars.ptr == '-' );
		pDigitEnd = pSrc + 2;
	}
	if ( iUuidBytes < 16 )
		dRes = {};
	return pSrc;
}

Wsrep::UUID_t Wsrep::Str2Uuid ( const CSphString & sUuid )
{
	Wsrep::UUID_t dRes;
	Str2UuidImpl ( dRes, sUuid );
	return dRes;
}

Wsrep::GlobalTid_t Wsrep::Str2Gtid ( const CSphString& sGtid )
{
	Wsrep::GlobalTid_t dRes;
	const char* pEnd = sGtid.cstr() + sGtid.Length();
	const char* pSrc = Str2UuidImpl ( dRes.m_tUuid, sGtid );
	if ( pSrc+1 < pEnd && *pSrc == ':' )
	{
		++pSrc;
		auto tChars = from_chars_wrap ( pSrc, pEnd, dRes.m_iSeqNo );
		if ( tChars.ec == std::errc::invalid_argument )
			dRes = {};
	} else dRes = {};
	return dRes;
}

std::atomic<uint64_t> uWritesetConnIds {1};

WsrepLoader_t TryWsrep()
{
	WsrepLoader_t tRes;
	static const char* VANILLA_GALERA = "/usr/lib/galera/libgalera_smm.so";

	tRes.m_pLibrary = std::make_unique<CSphDynamicLibrary> ( GET_GALERA_FULLPATH().cstr(), false );
	if ( !tRes.m_pLibrary->GetLib() )
		tRes.m_pLibrary->CSphDynamicLibraryAlternative ( VANILLA_GALERA, false );
	if ( !tRes.m_pLibrary->GetLib() )
	{
		TlsMsg::Err ( "no wsrep provider available. Tried %s, then %s", GET_GALERA_FULLPATH().cstr(), VANILLA_GALERA );
		return tRes;
	}

	TlsMsg::ResetErr();
	auto eLogLvl = LogLevel_e::ERROR_;
	AT_SCOPE_EXIT ( [&eLogLvl] { if ( TlsMsg::HasErr() ) WsrepLog ( eLogLvl, TlsMsg::szError()); TlsMsg::ResetErr(); } );

	std::array<const char*, 2> sSyms { "wsrep_loader", "wsrep_interface_version" };
	const char** pszIfaceVer = nullptr;
	std::array<void**, 2> ppSyms { (void**)&tRes.m_WsrepLoaderFn, (void**)&pszIfaceVer };
	if ( !tRes.m_pLibrary->LoadSymbols ( (const char**)sSyms.data(), (void***)ppSyms.data(), 2 ) )
	{
		TlsMsg::Err ( "can't load wsrep provider" );
		return tRes;
	}

	tRes.m_iIfaceVer = atoi ( *pszIfaceVer );
	return tRes;
}

Wsrep::Provider_i* Wsrep::MakeProvider ( Wsrep::Cluster_i* pCluster, CSphString sName, const char* szListenAddr, const char* szIncoming, const char* szPath, const char* szOptions )
{
	auto tLoader = TryWsrep();
	if ( !tLoader.m_pLibrary )
		return nullptr;

	if ( tLoader.m_iIfaceVer == 25 )
		return MakeProviderV25 ( std::move ( tLoader ), pCluster, std::move ( sName ), szListenAddr, szIncoming, szPath, szOptions );

	if ( tLoader.m_iIfaceVer == 31 )
		return MakeProviderV31 ( std::move ( tLoader ), pCluster, std::move ( sName ), szListenAddr, szIncoming, szPath, szOptions );

	TlsMsg::Err ( "Wrong galera interface version. Got %d", tLoader.m_iIfaceVer );
	return nullptr;
}