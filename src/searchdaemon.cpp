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

/// @file searchdaemon.cpp
/// Definitions for the stuff need by searchd to work and serve the indexes.

#include "sphinxstd.h"
#include "searchdaemon.h"
#include "coroutine.h"

#include <optional>

#if _WIN32
	#define USE_PSI_INTERFACE 1
	// for MAC address

	#include <iphlpapi.h>
	#pragma message("Automatically linking with iphlpapi.lib")
	#pragma comment(lib, "iphlpapi.lib")

	#pragma comment(linker, "/defaultlib:WS2_32.Lib")
	#pragma message("Automatically linking with WS2_32.Lib")

	// socket function definitions
	#pragma comment(linker, "/defaultlib:wsock32.lib")
	#pragma message("Automatically linking with wsock32.lib")

#else
	#include <netdb.h>
	// for MAC address
	#include <net/if.h>
	#include <sys/ioctl.h>
	#include <net/ethernet.h>

	// TCP_NODELAY, TCP_FASTOPEN, etc.
	#include <netinet/tcp.h>
#endif

// for FreeBSD
#if defined(__FreeBSD__)
#include <sys/sysctl.h>
#include <net/route.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#endif

#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// MISC GLOBALS
/////////////////////////////////////////////////////////////////////////////

const char * szCommand ( int eCmd)
{
	const char* szCommands[SEARCHD_COMMAND_TOTAL] = {"command_search", "command_excerpt", "command_update",
		"command_keywords", "command_persist", "command_status", "gap_6", "command_flushattrs", "command_sphinxql",
		"command_ping", "command_delete", "command_set", "command_insert", "command_replace", "command_commit",
		"command_suggest", "command_json", "command_callpq", "command_cluster", "command_getfield"};
	if ( eCmd<SEARCHD_COMMAND_TOTAL )
		return szCommands[eCmd];
	return "***WRONG COMMAND!***";
}

// 'like' matcher
CheckLike::CheckLike( const char* sPattern )
{
	if ( !sPattern )
		return;

	m_sPattern.Reserve( 2 * (int) strlen( sPattern ));
	char* d = const_cast<char*> ( m_sPattern.cstr());

	// remap from SQL LIKE syntax to Sphinx wildcards syntax
	// '_' maps to '?', match any single char
	// '%' maps to '*', match zero or mor chars
	for ( const char* s = sPattern; *s; ++s )
	{
		switch ( *s )
		{
			case '_': *d++ = '?';
				break;
			case '%': *d++ = '*';
				break;
			case '?': *d++ = '\\';
				*d++ = '?';
				break;
			case '*': *d++ = '\\';
				*d++ = '*';
				break;
			default: *d++ = *s;
				break;
		}
	}
	*d = '\0';
}

bool CheckLike::Match ( const char* sValue ) const noexcept
{
	return sValue && ( m_sPattern.IsEmpty() || sphWildcardMatch ( sValue, m_sPattern.cstr() ) );
}

// string vector with 'like' matcher
/////////////////////////////////////////////////////////////////////////////
VectorLike::VectorLike( int iCols )
	: CheckLike( nullptr )
{
	m_dHeadNames.Resize ( iCols );
}

VectorLike::VectorLike ( const CSphString & sPattern )
		: CheckLike ( sPattern.cstr () )
{
	m_dHeadNames.Resize ( 2 );
	SetColName ( "Variable_name" );
	SetColName ( "Value", 1 );
}

VectorLike::VectorLike( const CSphString& sPattern, int iCols )
	: CheckLike ( sPattern.cstr () )
{
	m_dHeadNames.Resize ( iCols );
}

VectorLike::VectorLike ( const CSphString & sPattern, std::initializer_list<const char *> sCols )
	: CheckLike ( sPattern.cstr () )
{
	for ( const char * szCol : sCols )
		m_dHeadNames.Add ( szCol );
}

void VectorLike::SetColNames ( std::initializer_list<const char *> sCols )
{
	for ( const char * szCol : sCols )
		m_dHeadNames.Add ( szCol );
}

void VectorLike::SetColName ( CSphString sValue, int iIdx )
{
	assert ( iIdx>=0 && iIdx<m_dHeadNames.GetLength () );
	m_dHeadNames[iIdx] = std::move(sValue);
}

const VecTraits_T<CSphString> & VectorLike::Header () const
{
	return m_dHeadNames;
}

bool VectorLike::MatchAdd( const char* sValue )
{
	assert ( m_dHeadNames.GetLength ()>=1 );
	if ( Match( sValue ))
	{
		Add( sValue );
		return true;
	}
	return false;
}

bool VectorLike::MatchAddf ( const char* sTemplate, ... )
{
	assert ( m_dHeadNames.GetLength ()>=1 );
	va_list ap;
	CSphString sValue;

	va_start ( ap, sTemplate );
	sValue.SetSprintfVa( sTemplate, ap );
	va_end ( ap );

	return MatchAdd( sValue.cstr());
}

bool VectorLike::Matchf ( const char* sTemplate, ... ) const noexcept
{
	assert ( m_dHeadNames.GetLength() >= 1 );
	va_list ap;
	CSphString sValue;

	va_start ( ap, sTemplate );
	sValue.SetSprintfVa ( sTemplate, ap );
	va_end ( ap );

	return Match ( sValue.cstr() );
}

void VectorLike::Addf ( const char * sValueTmpl, ... )
{
	va_list ap;
	StringBuilder_c sValue;
	va_start ( ap, sValueTmpl );
	sValue.vSprintf ( sValueTmpl, ap );
	va_end ( ap );

	Add ( sValue.cstr () );
}

void VectorLike::MatchTuplet ( const char * sKey, const char * sValue )
{
	assert ( m_dHeadNames.GetLength ()>=2 );
	if ( !Match ( sKey ) )
		return;

	Add ( sKey );
	Add ( sValue );
	FillTail ( 2 );
}

void VectorLike::MatchTupletf ( const char * sKey, const char * sValueTmpl, ... )
{
	assert ( m_dHeadNames.GetLength ()>=2 );
	if ( !Match ( sKey ) )
		return;

	va_list ap;
	StringBuilder_c sValue;
	va_start ( ap, sValueTmpl );
	sValue.vSprintf( sValueTmpl, ap );
	va_end ( ap );

	Add ( sKey );
	Add ( sValue.cstr() );
	FillTail ( 2 );
}

void VectorLike::MatchTupletFn ( const char * sKey, Generator_fn && fnValuePrinter )
{
	assert ( m_dHeadNames.GetLength ()>=2 );
	if ( !Match ( sKey ) )
		return;

	Add ( sKey );
	Add ( fnValuePrinter () );
	FillTail ( 2 );
}

void VectorLike::MatchTupletFn ( const char * sKey, GeneratorS_fn && fnValuePrinter )
{
	assert ( m_dHeadNames.GetLength ()>=2 );
	if ( !Match ( sKey ) )
		return;

	Add ( sKey );
	Add ( CSphString ( fnValuePrinter () ) );
	FillTail ( 2 );
}


void VectorLike::FillTail ( int iHas )
{
	for ( auto iLen = m_dHeadNames.GetLength (); iHas<iLen; ++iHas )
		Add("");
}


const char* GetIndexTypeName ( IndexType_e eType )
{
	switch ( eType )
	{
	case IndexType_e::PLAIN : return "plain";
	case IndexType_e::TEMPLATE: return "template";
	case IndexType_e::RT: return "rt";
	case IndexType_e::PERCOLATE: return "percolate";
	case IndexType_e::DISTR: return "distributed";
	default: return "invalid";
	}
}

IndexType_e TypeOfIndexConfig( const CSphString& sType )
{
	if ( sType=="distributed" )
		return IndexType_e::DISTR;

	if ( sType=="rt" )
		return IndexType_e::RT;

	if ( sType=="percolate" )
		return IndexType_e::PERCOLATE;

	if ( sType=="template" )
		return IndexType_e::TEMPLATE;

	if (( sType.IsEmpty() || sType=="plain" ))
		return IndexType_e::PLAIN;

	return IndexType_e::ERROR_;
}

static void MaybeFatalLog ( CSphString * pFatal, const char * sTemplate, ... )
{
	va_list ap;

	va_start ( ap, sTemplate );
	if ( pFatal )
		pFatal->SetSprintfVa ( sTemplate, ap );
	else
		sphFatalVa ( sTemplate, ap );
	va_end ( ap );
}

bool CheckPort ( int iPort, CSphString * pFatal )
{
	if ( !IsPortInRange ( iPort ) )
	{
		MaybeFatalLog ( pFatal, "port %d is out of range", iPort );
		return false;
	}
	return true;
}

// check only proto name in lowcase, no '_vip'
static Proto_e SimpleProtoByName ( const CSphString& sProto, CSphString * pFatal )
{
	if ( sProto=="" )
		return Proto_e::SPHINX;
	if ( sProto=="mysql41" || sProto=="mysql" )
		return Proto_e::MYSQL41;
	if ( sProto=="http" )
		return Proto_e::HTTP;
	if ( sProto=="https" )
		return Proto_e::HTTPS;
	if ( sProto=="replication" )
		return Proto_e::REPLICATION;
	if ( sProto=="sphinx" )
		return Proto_e::SPHINXSE;
	
	MaybeFatalLog ( pFatal, "unknown listen protocol type '%s'", sProto.scstr());
	return Proto_e::UNKNOWN;
}

static bool ProtoByName ( CSphString sFullProto, ListenerDesc_t & tDesc, CSphString * pFatal )
{
	sFullProto.ToLower();
	StrVec_t dParts;
	sphSplit( dParts, sFullProto.cstr(), "_" );

	if ( !dParts.IsEmpty() )
	{
		tDesc.m_eProto = SimpleProtoByName( dParts[0], pFatal );
		if ( tDesc.m_eProto==Proto_e::UNKNOWN )
			return false;
	}

	if ( dParts.GetLength() == 1 )
		return true;

	if ( dParts.GetLength() >= 2 )
	{
		bool bOk = dParts.GetLength() == 2;
		if ( dParts[1] == "vip" )
			tDesc.m_bVIP = true;
		else if ( dParts[1] == "readonly" )
			tDesc.m_bReadOnly = true;
		else
			bOk = false;
		if ( bOk )
			return true;
	}

	if ( dParts.GetLength() == 3 && dParts[2] == "readonly" )
	{
		tDesc.m_bReadOnly = true;
		return true;
	}

	MaybeFatalLog ( pFatal, "unknown listen protocol type '%s'", sFullProto.scstr() );
	return false;
}

/// listen = ( address ":" port | port | path | address ":" port start - port end ) [ ":" protocol ] [ "_vip" ]
ListenerDesc_t ParseResolveListener ( const char* sSpec, bool bResolve, CSphString* pFatal )
{
	ListenerDesc_t tRes;
	tRes.m_eProto = Proto_e::SPHINX;
	tRes.m_uIP = htonl(INADDR_ANY);
	tRes.m_iPort = SPHINXAPI_PORT;
	tRes.m_iPortsCount = 0;
	tRes.m_bVIP = false;
	tRes.m_bReadOnly = false;

	// split by colon
	auto dParts = sphSplit( sSpec, ":" ); // diff. parts are :-separated

	int iParts = dParts.GetLength();
	if ( iParts>3 )
	{
		MaybeFatalLog ( pFatal, "invalid listen format (too many fields)" );
		return {};
	}

	assert ( iParts>=1 && iParts<=3 );

	// handle UNIX socket case
	// might be either name on itself (1 part), or name+protocol (2 parts)
	if ( *dParts[0].scstr()=='/' )
	{
		if ( iParts>2 )
		{
			MaybeFatalLog ( pFatal, "invalid listen format (too many fields)" );
			return {};
		}

		if ( iParts==2 && !ProtoByName ( dParts[1], tRes, pFatal ) )
			return {};

		tRes.m_sUnix = dParts[0];

		// MOVED!!! check outside ParseListener in order to make tests consistent despite platforms
#if _WIN32
		MaybeFatalLog ( pFatal, "UNIX sockets are not supported on Windows" );
		return {};
#else
		return tRes;
#endif
	}

	// check if it all starts with a valid port number
	auto sPart = dParts[0].cstr();
	auto iLen = (int) strlen( sPart );

	bool bAllDigits = true;
	for ( int i = 0; i<iLen && bAllDigits; ++i )
		if ( !isdigit( sPart[i] ))
			bAllDigits = false;

	int iPort = 0;
	if ( bAllDigits && iLen<=5 ) // if we have num from only digits, it may be only port, nothing else!
	{
		iPort = atol( sPart );
		if ( !CheckPort ( iPort, pFatal ) ) // lets forbid ambiguous magic like 0:sphinx or 99999:mysql41
			return {};
	}

	// handle TCP port case
	// one part. might be either port name, or host name (unix socked case is already parsed)
	if ( iParts==1 )
	{
		if ( iPort )
		{
			// port name on itself
			tRes.m_iPort = iPort;
		} else
		{
			// host name on itself
			tRes.m_sAddr = sSpec;
			tRes.m_uIP = bResolve ? sphGetAddress ( sSpec, ( pFatal==nullptr ), false, pFatal  ) : 0;
			if ( pFatal && !pFatal->IsEmpty() )
				return {};
		}
		return tRes;
	}

	// two or three parts
	if ( iPort )
	{
		// 1st part is a valid port number; must be port:proto
		if ( iParts!=2 )
		{
			MaybeFatalLog ( pFatal, "invalid listen format (expected port:proto, got extra trailing part in listen=%s)", sSpec );
			return {};
		}

		tRes.m_iPort = iPort;
		if ( !ProtoByName ( dParts[1], tRes, pFatal ) )
			return {};
		return tRes;
	}

	// 1st part must be a host name; must be: host:port[:proto]
	if ( iParts==3 && !ProtoByName ( dParts[2], tRes, pFatal ) )
		return {};

	if ( dParts[0].IsEmpty() )
	{
		tRes.m_uIP = htonl(INADDR_ANY);
	} else
	{
		tRes.m_sAddr = dParts[0];
		tRes.m_uIP = bResolve ? sphGetAddress ( dParts[0].cstr(), ( pFatal==nullptr ), false, pFatal ) : 0;
		if ( pFatal && !pFatal->IsEmpty() )
			return {};
	}


	auto dPorts = sphSplit( dParts[1].scstr(), "-" );
	tRes.m_iPort = atoi( dPorts[0].cstr());
	if ( !CheckPort( tRes.m_iPort, pFatal ) )
		return {};

	if ( dPorts.GetLength()==2 )
	{
		int iPortsEnd = atoi( dPorts[1].scstr() );
		if ( !CheckPort ( iPortsEnd, pFatal ) )
			return {};

		int iPortsCount = iPortsEnd - tRes.m_iPort + 1;

		if ( iPortsEnd<=tRes.m_iPort )
		{
			MaybeFatalLog ( pFatal, "ports range invalid %d-%d", tRes.m_iPort, iPortsEnd );
			return {};
		}
		if ( iPortsCount<2 )
		{
			MaybeFatalLog( pFatal, "ports range %d-%d count should be at least 2, got %d", tRes.m_iPort, iPortsEnd, iPortsCount );
			return {};
		}

		tRes.m_iPortsCount = iPortsCount;
	}
	return tRes;
}

ListenerDesc_t ParseListener ( const char* sSpec, CSphString* pFatal )
{
	return ParseResolveListener ( sSpec, true, pFatal );
}

/////////////////////////////////////////////////////////////////////////////
// NETWORK SOCKET WRAPPERS
/////////////////////////////////////////////////////////////////////////////

#if _WIN32
const char * sphSockError ( int iErr )
{
	if ( iErr==0 )
		iErr = WSAGetLastError ();

	static char sBuf [ 256 ];
	_snprintf ( sBuf, sizeof(sBuf), "WSA error %d", iErr );
	return sBuf;
}
#else

const char* sphSockError( int )
{
	return strerrorm(errno);
}

#endif


int sphSockGetErrno()
{
#if _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}


void sphSockSetErrno( int iErr )
{
#if _WIN32
	WSASetLastError ( iErr );
#else
	errno = iErr;
#endif
}


int sphSockPeekErrno()
{
	int iRes = sphSockGetErrno();
	sphSockSetErrno( iRes );
	return iRes;
}


int sphSetSockNB( int iSock )
{
#if _WIN32
	u_long uMode = 1;
		return ioctlsocket ( iSock, FIONBIO, &uMode );
#else
	return fcntl( iSock, F_SETFL, O_NONBLOCK );
#endif
}

void sphSetSockNodelay ( int iSock )
{
#ifdef TCP_NODELAY
	int iOn = 1;
	if ( setsockopt ( iSock, IPPROTO_TCP, TCP_NODELAY, (char*)&iOn, sizeof ( iOn ) )<0 )
		sphWarning ( "failed to set nodelay option: %s", sphSockError() );
#endif
}

void sphSetSockReuseAddr ( int iSock )
{
	int iOn = 1;
	if ( setsockopt ( iSock, SOL_SOCKET, SO_REUSEADDR, (char *) &iOn, sizeof ( iOn ) ) )
		sphWarning ( "setsockopt(SO_REUSEADDR) failed: %s", sphSockError () );
}

void sphSetSockReusePort ( int iSock )
{
#if HAVE_SO_REUSEPORT
	int iOn = 1;
	if ( setsockopt ( iSock, SOL_SOCKET, SO_REUSEPORT, (char *) &iOn, sizeof ( iOn ) ) )
		sphWarning ( "setsockopt(SO_REUSEPORT) failed: %s", sphSockError () );
#endif
}

void sphSetSockTFO ( int iSock)
{
#if defined (TCP_FASTOPEN)
	int iOn = 1;
	if ( setsockopt ( iSock, IPPROTO_TCP, TCP_FASTOPEN, (char *) &iOn, sizeof ( iOn ) ) )
		sphLogDebug ( "setsockopt(TCP_FASTOPEN) failed: %s", sphSockError () );
#endif
}

#if _WIN32

/// on Windows, the wrapper just prevents the warnings

#pragma warning(push) // store current warning values
#pragma warning(disable:4127) // conditional expr is const
#pragma warning(disable:4389) // signed/unsigned mismatch

static void FDSet ( int fd, fd_set * fdset )
{
	FD_SET ( fd, fdset );
}

#pragma warning(pop) // restore warnings

#else // !_WIN32
#if !HAVE_POLL
#define SPH_FDSET_OVERFLOW( _fd ) ( (_fd)<0 || (_fd)>=(int)FD_SETSIZE )

/// on UNIX, we also check that the descript won't corrupt the stack
static void FDSet( int fd, fd_set* set )
{
	if ( SPH_FDSET_OVERFLOW( fd ))
		sphFatal( "FDSet() failed fd=%d, FD_SETSIZE=%d", fd, FD_SETSIZE );
	else
		FD_SET ( fd, set );
}

#endif // !HAVE_POLL
#endif // _WIN32

/// wait until socket is readable or writable
int sphPoll( int iSock, int64_t tmTimeout, bool bWrite )
{
	// don't need any epoll/kqueue here, since we check only 1 socket
#if HAVE_POLL
	struct pollfd pfd;
	pfd.fd = iSock;
	pfd.events = bWrite ? POLLOUT : POLLIN;

	return ::poll( &pfd, 1, int( tmTimeout / 1000 ));
#else
	fd_set fdSet;
	FD_ZERO ( &fdSet );
	FDSet ( iSock, &fdSet );

	struct timeval tv;
	tv.tv_sec = (int)( tmTimeout / 1000000 );
	tv.tv_usec = (int)( tmTimeout % 1000000 );

	return ::select ( iSock+1, bWrite ? NULL : &fdSet, bWrite ? &fdSet : NULL, NULL, &tv );
#endif
}

static bool IsLocalhost ( DWORD uAddr )
{
	return ( ( ntohl ( uAddr )>>24)==127 );
}

DWORD sphGetAddress ( const char * sHost, bool bFatal, bool bIP, CSphString * pFatal )
{
	struct addrinfo tHints, * pResult = nullptr;
	memset( &tHints, 0, sizeof( tHints ));
	tHints.ai_family = AF_INET;
	tHints.ai_socktype = SOCK_STREAM;
	if ( bIP )
		tHints.ai_flags = AI_NUMERICHOST;

	int iResult = getaddrinfo( sHost, nullptr, &tHints, &pResult );
	auto pResFree = AtScopeExit ( [pResult] { if (pResult) freeaddrinfo( pResult ); } );
	if ( iResult!=0 || !pResult )
	{
		if ( pFatal )
			pFatal->SetSprintf ( "no AF_INET address found for: %s, error %d: %s", sHost, iResult, gai_strerror(iResult) );
		else if ( bFatal )
			sphFatal( "no AF_INET address found for: %s, error %d: %s", sHost, iResult, gai_strerror(iResult) );
		else
			sphLogDebugv( "no AF_INET address found for: %s, error %d: %s", sHost, iResult, gai_strerror(iResult) );
		return 0;
	}

	assert ( pResult );
	auto * pSockaddr_ipv4 = ( struct sockaddr_in* ) pResult->ai_addr;
	DWORD uAddr = pSockaddr_ipv4->sin_addr.s_addr;

	if ( pResult->ai_next )
	{
		const bool bLocalHost = IsLocalhost ( uAddr );
		std::array<char, SPH_ADDRESS_SIZE + 1> sAddrBuf{};
		StringBuilder_c sBuf( "; ip=", "ip=" );

		while ( pResult )
		{
			auto * pAddr = ( struct sockaddr_in *)pResult->ai_addr;
			DWORD uNextAddr = pAddr->sin_addr.s_addr;
			sphFormatIP( sAddrBuf.data(), sAddrBuf.size(), uNextAddr );
			sBuf += sAddrBuf.data(); // can not use << as builder appends string buffer with tail '\0' and next chunks are invisible
			pResult = pResult->ai_next;

			if ( bLocalHost && !IsLocalhost ( uNextAddr ) )
				uAddr = uNextAddr;
		}

		sphFormatIP( sAddrBuf.data(), sAddrBuf.size(), uAddr );
		sphWarning( "multiple addresses (%s) found for '%s', using first one (%s)", sBuf.cstr(), sHost, sAddrBuf.data() );
	}

	return uAddr;
}


/// formats IP address given in network byte order into sBuffer
/// returns the buffer
char* sphFormatIP( char* sBuffer, int iBufferSize, DWORD uAddress )
{
	const BYTE* a = ( const BYTE* ) &uAddress;
	snprintf( sBuffer, iBufferSize, "%u.%u.%u.%u", a[0], a[1], a[2], a[3] );
	return sBuffer;
}


bool IsPortInRange( int iPort )
{
	return ( iPort>0 ) && ( iPort<=0xFFFF );
}


/////////////////////////////////////////////////////////////////////////////
// NETWORK BUFFERS
/////////////////////////////////////////////////////////////////////////////

ISphOutputBuffer::ISphOutputBuffer()
{
	m_dBuf.Reserve( NETOUTBUF );
}

// construct via adopting external buf
ISphOutputBuffer::ISphOutputBuffer( CSphVector<BYTE>& dChunk )
{
	m_dBuf.SwapData( dChunk );
}


void ISphOutputBuffer::SendString( const char* sStr )
{
	int iLen = sStr ? (int) strlen( sStr ) : 0;
	SendInt( iLen );
	SendBytes( sStr, iLen );
}

void ISphOutputBuffer::SendString ( const Str_t& sStr )
{
	SendInt ( sStr.second );
	SendBytes ( sStr );
}

void SendString ( const Str_t& sStr );

/// SmartOutputBuffer_t : chain of blobs could be used in scattered sending
/////////////////////////////////////////////////////////////////////////////
SmartOutputBuffer_t::~SmartOutputBuffer_t()
{
	m_dChunks.Apply( []( ISphOutputBuffer*& pChunk ) {
		SafeDelete ( pChunk );
	} );
}

void SmartOutputBuffer_t::StartNewChunk()
{
	m_dChunks.Add( new ISphOutputBuffer( m_dBuf ));
	m_dBuf.Reserve( NETOUTBUF );
}

/*
void SmartOutputBuffer_t::AppendBuf ( SmartOutputBuffer_t &dBuf )
{
	if ( !dBuf.m_dBuf.IsEmpty () )
		dBuf.StartNewChunk ();
	for ( auto * pChunk : dBuf.m_dChunks )
	{
		pChunk->AddRef ();
		m_dChunks.Add ( pChunk );
	}
}

void SmartOutputBuffer_t::PrependBuf ( SmartOutputBuffer_t &dBuf )
{
	CSphVector<ISphOutputBuffer *> dChunks;
	if ( !dBuf.m_dBuf.IsEmpty () )
		dBuf.StartNewChunk ();
	for ( auto * pChunk : dBuf.m_dChunks )
	{
		pChunk->AddRef ();
		dChunks.Add ( pChunk );
	}
	dChunks.Append ( m_dChunks );
	m_dChunks.SwapData ( dChunks );
}
*/

#ifndef UIO_MAXIOV
#define UIO_MAXIOV (1024)
#endif

// makes vector of chunks suitable to direct using in Send() or WSASend()
// returns federated size of the chunks
size_t SmartOutputBuffer_t::GetIOVec( CSphVector<sphIovec>& dOut ) const
{
	size_t iOutSize = 0;
	dOut.Reset();
	m_dChunks.Apply( [ &dOut, &iOutSize ]( const ISphOutputBuffer* pChunk ) {
		auto& dIovec = dOut.Add();
		IOPTR( dIovec ) = IOBUFTYPE ( pChunk->GetBufPtr());
		IOLEN ( dIovec ) = pChunk->GetSentCount();
		iOutSize += IOLEN ( dIovec );
	} );
	if ( !m_dBuf.IsEmpty())
	{
		auto& dIovec = dOut.Add();
		IOPTR ( dIovec ) = IOBUFTYPE ( GetBufPtr());
		IOLEN ( dIovec ) = (int) m_dBuf.GetLengthBytes();
		iOutSize += IOLEN ( dIovec );
	}
	assert ( dOut.GetLength()<UIO_MAXIOV );
	return iOutSize;
};

void SmartOutputBuffer_t::Reset()
{
	m_dChunks.Apply( []( ISphOutputBuffer*& pChunk ) {
		SafeDelete ( pChunk );
	} );
	m_dChunks.Reset();
	m_dBuf.Reset();
	m_dBuf.Reserve( NETOUTBUF );
};

#if _WIN32
void SmartOutputBuffer_t::LeakTo ( CSphVector<ISphOutputBuffer *> dOut )
{
	for ( auto & pChunk : m_dChunks )
		dOut.Add ( pChunk );
	m_dChunks.Reset ();
	dOut.Add ( new ISphOutputBuffer ( m_dBuf ) );
	m_dBuf.Reserve ( NETOUTBUF );
}
#endif


/////////////////////////////////////////////////////////////////////////////

InputBuffer_c::InputBuffer_c( const BYTE* pBuf, int iLen )
	: m_pBuf( pBuf ), m_pCur( pBuf ), m_iLen( iLen )
{
	if ( !pBuf || iLen<0 )
		SetError ( "empty input buffer" );
}

InputBuffer_c::InputBuffer_c ( const VecTraits_T<BYTE> & dBuf )
	: m_pBuf ( dBuf.begin() ), m_pCur ( dBuf.begin () ), m_iLen ( dBuf.GetLength() )
{
	if ( dBuf.IsEmpty() )
		SetError ( "empty input buffer" );
}

CSphString InputBuffer_c::GetString()
{
	int iLen = GetInt();
	return GetRawString ( iLen );
}


CSphString InputBuffer_c::GetRawString( int iLen )
{
	CSphString sRes;
	if ( m_bError || !IsDataSizeValid ( iLen ) )
		return sRes;

	if ( iLen )
		sRes.SetBinary( (const char*) m_pCur, iLen );

	m_pCur += iLen;
	return sRes;
}


bool InputBuffer_c::GetString( CSphVector<BYTE>& dBuffer )
{
	int iLen = GetInt();
	if ( m_bError || !IsDataSizeValid ( iLen ) )
		return false;

	if ( !iLen )
		return true;

	return GetBytes( dBuffer.AddN( iLen ), iLen );
}


bool InputBuffer_c::GetBytes( void* pBuf, int iLen )
{
	assert ( pBuf );

	if ( m_bError || !IsDataSizeValid ( iLen ) )
		return false;

	memcpy( pBuf, m_pCur, iLen );
	m_pCur += iLen;
	return true;
}

bool InputBuffer_c::GetBytesZerocopy( const BYTE** ppData, int iLen )
{
	assert ( ppData );

	if ( m_bError || !IsDataSizeValid ( iLen ) )
		return false;

	*ppData = m_pCur;
	m_pCur += iLen;
	return true;
}


bool InputBuffer_c::GetDwords( CSphVector<DWORD>& dBuffer, int& iGot, int iMax )
{
	iGot = GetInt();
	if ( iGot<0 || iGot>iMax )
	{
		SetError( "length %d (should be in 0..%d range)", iGot, iMax );
		return false;
	}

	dBuffer.Resize( iGot );
	ARRAY_FOREACH ( i, dBuffer )
		dBuffer[i] = GetDword();

	if ( m_bError )
		dBuffer.Reset();

	return !m_bError;
}


bool InputBuffer_c::GetQwords( CSphVector<SphAttr_t>& dBuffer, int& iGot, int iMax )
{
	iGot = GetInt();
	if ( iGot<0 || iGot>iMax )
	{
		SetError( "length %d (should be in 0..%d range)", iGot, iMax );
		return false;
	}

	dBuffer.Resize( iGot );
	ARRAY_FOREACH ( i, dBuffer )
		dBuffer[i] = GetUint64();

	if ( m_bError )
		dBuffer.Reset();

	return !m_bError;
}

void InputBuffer_c::SetError( const char * sTemplate, ... )
{
	m_bError = true;

	va_list ap;
	va_start ( ap, sTemplate );
	m_sError.SetSprintfVa ( sTemplate, ap );
	va_end ( ap );
}

bool InputBuffer_c::IsDataSizeValid ( int iSize )
{
	if ( !IsLessMaxPacket ( iSize ) )
	{
		return false;
	} else if ( m_pCur + iSize>m_pBuf + m_iLen )
	{
		SetError( "read overflows buffer by %d byte, data size %d", (int)( ( m_pCur + iSize ) - ( m_pBuf + m_iLen ) ), iSize );
		return false;
	}

	return true;
}

bool InputBuffer_c::IsLessMaxPacket ( int iSize )
{
	if ( iSize<0 )
	{
		SetError( "negative data length %d", iSize );
		return false;
	} else if ( iSize>m_iMaxPacketSize )
	{
		SetError( "length out of bounds %d(%d)", iSize, m_iMaxPacketSize );
		return false;
	}

	return true;
}

void InputBuffer_c::ResetError()
{
	m_bError = false;
	m_sError = "";
}

void GenericOutputBuffer_c::ResetError()
{
	m_bError = false;
	m_sError = "";
}

void InputBuffer_c::SetMaxPacketSize( int iMaxPacketSize )
{
	m_iMaxPacketSize = Max ( g_iMaxPacketSize, iMaxPacketSize );
}

/////////////////////////////////////////////////////////////////////////////
// SERVED INDEX DESCRIPTORS STUFF
/////////////////////////////////////////////////////////////////////////////


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

#ifndef NDEBUG

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

#endif


//////////////////////////////////////////////////////////////////////////
ServedStats_c::ServedStats_c()
	: m_pQueryStatRecords { std::make_unique<QueryStatContainer_c>() }
#ifndef NDEBUG
	, m_pQueryStatRecordsExact { std::make_unique<QueryStatContainerExact_c>() }
#endif

{}

void ServedStats_c::AddQueryStat( uint64_t uFoundRows, uint64_t uQueryTime )
{
	ScWL_t wLock( m_tStatsLock );

	m_tRowsFoundDigest.Add(( double ) uFoundRows );
	m_tQueryTimeDigest.Add(( double ) uQueryTime );

	uint64_t uTimeStamp = sphMicroTimer();
	m_pQueryStatRecords->Add( uFoundRows, uQueryTime, uTimeStamp );

#ifndef NDEBUG
	m_pQueryStatRecordsExact->Add( uFoundRows, uQueryTime, uTimeStamp );
#endif

	m_uTotalFoundRowsMin = Min( uFoundRows, m_uTotalFoundRowsMin );
	m_uTotalFoundRowsMax = Max( uFoundRows, m_uTotalFoundRowsMax );
	m_uTotalFoundRowsSum += uFoundRows;

	m_uTotalQueryTimeMin = Min( uQueryTime, m_uTotalQueryTimeMin );
	m_uTotalQueryTimeMax = Max( uQueryTime, m_uTotalQueryTimeMax );
	m_uTotalQueryTimeSum += uQueryTime;

	++m_uTotalQueries;
}


static const uint64_t g_dStatsIntervals[] =
	{
		1 * 60 * 1000000,
		5 * 60 * 1000000,
		15 * 60 * 1000000
	};


void ServedStats_c::CalculateQueryStats( QueryStats_t& tRowsFoundStats, QueryStats_t& tQueryTimeStats ) const
{
	ScRL_t rLock { m_tStatsLock };
	DoStatCalcStats ( m_pQueryStatRecords.get(), tRowsFoundStats, tQueryTimeStats );
}


#ifndef NDEBUG

void ServedStats_c::CalculateQueryStatsExact( QueryStats_t& tRowsFoundStats, QueryStats_t& tQueryTimeStats ) const
{
	ScRL_t rLock { m_tStatsLock };
	DoStatCalcStats ( m_pQueryStatRecordsExact.get(), tRowsFoundStats, tQueryTimeStats );
}

#endif // !NDEBUG


static void CalcStatsForInterval( const QueryStatContainer_i* pContainer, QueryStatElement_t& tRowResult, QueryStatElement_t& tTimeResult, uint64_t uTimestamp, uint64_t uInterval, int iRecords )
{
	assert ( pContainer );
	using namespace QueryStats;

	tRowResult.m_dData[TYPE_AVG] = 0;
	tRowResult.m_dData[TYPE_MIN] = UINT64_MAX;
	tRowResult.m_dData[TYPE_MAX] = 0;

	tTimeResult.m_dData[TYPE_AVG] = 0;
	tTimeResult.m_dData[TYPE_MIN] = UINT64_MAX;
	tTimeResult.m_dData[TYPE_MAX] = 0;

	CSphTightVector<uint64_t> dFound, dTime;
	dFound.Reserve( iRecords );
	dTime.Reserve( iRecords );

	DWORD uTotalQueries = 0;

	for ( int i = 0; i<pContainer->GetNumRecords(); ++i )
	{
		auto tRecord = pContainer->GetRecord ( i );
		if ( uTimestamp - tRecord.m_uTimestamp<=uInterval )
		{
			tRowResult.m_dData[TYPE_MIN] = Min( tRecord.m_uFoundRowsMin, tRowResult.m_dData[TYPE_MIN] );
			tRowResult.m_dData[TYPE_MAX] = Max( tRecord.m_uFoundRowsMax, tRowResult.m_dData[TYPE_MAX] );

			tTimeResult.m_dData[TYPE_MIN] = Min( tRecord.m_uQueryTimeMin, tTimeResult.m_dData[TYPE_MIN] );
			tTimeResult.m_dData[TYPE_MAX] = Max( tRecord.m_uQueryTimeMax, tTimeResult.m_dData[TYPE_MAX] );

			dFound.Add( tRecord.m_uFoundRowsSum / tRecord.m_iCount );
			dTime.Add( tRecord.m_uQueryTimeSum / tRecord.m_iCount );

			tRowResult.m_dData[TYPE_AVG] += tRecord.m_uFoundRowsSum;
			tTimeResult.m_dData[TYPE_AVG] += tRecord.m_uQueryTimeSum;
			uTotalQueries += tRecord.m_iCount;
		}
	}

	dFound.Sort();
	dTime.Sort();

	tRowResult.m_uTotalQueries = uTotalQueries;
	tTimeResult.m_uTotalQueries = uTotalQueries;

	if ( !dFound.GetLength())
		return;

	tRowResult.m_dData[TYPE_AVG] /= uTotalQueries;
	tTimeResult.m_dData[TYPE_AVG] /= uTotalQueries;

	int u95 = Max( 0, Min( int( ceilf( dFound.GetLength() * 0.95f ) + 0.5f ) - 1, dFound.GetLength() - 1 ));
	int u99 = Max( 0, Min( int( ceilf( dFound.GetLength() * 0.99f ) + 0.5f ) - 1, dFound.GetLength() - 1 ));

	tRowResult.m_dData[TYPE_95] = dFound[u95];
	tRowResult.m_dData[TYPE_99] = dFound[u99];

	tTimeResult.m_dData[TYPE_95] = dTime[u95];
	tTimeResult.m_dData[TYPE_99] = dTime[u99];
}

void CalcSimpleStats ( const QueryStatContainer_i * pContainer, QueryStats_t & tRowsFoundStats, QueryStats_t & tQueryTimeStats )
{
	assert ( pContainer );
	using namespace QueryStats;
	auto uTimestamp = sphMicroTimer ();
	int iRecords = pContainer->GetNumRecords ();
	for ( int i = INTERVAL_1MIN; i<=INTERVAL_15MIN; ++i )
		CalcStatsForInterval ( pContainer, tRowsFoundStats.m_dStats[i], tQueryTimeStats.m_dStats[i], uTimestamp, g_dStatsIntervals[i], iRecords );
}


void ServedStats_c::DoStatCalcStats( const QueryStatContainer_i* pContainer, QueryStats_t& tRowsFoundStats, QueryStats_t& tQueryTimeStats ) const
{
	assert ( pContainer );
	using namespace QueryStats;

	auto uTimestamp = sphMicroTimer();

	int iRecords = m_pQueryStatRecords->GetNumRecords();
	for ( int i = INTERVAL_1MIN; i<=INTERVAL_15MIN; ++i )
		CalcStatsForInterval( pContainer, tRowsFoundStats.m_dStats[i], tQueryTimeStats.m_dStats[i], uTimestamp, g_dStatsIntervals[i], iRecords );

	auto& tRowsAllStats = tRowsFoundStats.m_dStats[INTERVAL_ALLTIME];
	tRowsAllStats.m_dData[TYPE_AVG] = m_uTotalQueries ? m_uTotalFoundRowsSum / m_uTotalQueries : 0;
	tRowsAllStats.m_dData[TYPE_MIN] = m_uTotalFoundRowsMin;
	tRowsAllStats.m_dData[TYPE_MAX] = m_uTotalFoundRowsMax;
	tRowsAllStats.m_dData[TYPE_95] = ( uint64_t ) m_tRowsFoundDigest.Percentile( 95 );
	tRowsAllStats.m_dData[TYPE_99] = ( uint64_t ) m_tRowsFoundDigest.Percentile( 99 );
	tRowsAllStats.m_uTotalQueries = m_uTotalQueries;

	auto& tQueryAllStats = tQueryTimeStats.m_dStats[INTERVAL_ALLTIME];
	tQueryAllStats.m_dData[TYPE_AVG] = m_uTotalQueries ? m_uTotalQueryTimeSum / m_uTotalQueries : 0;
	tQueryAllStats.m_dData[TYPE_MIN] = m_uTotalQueryTimeMin;
	tQueryAllStats.m_dData[TYPE_MAX] = m_uTotalQueryTimeMax;
	tQueryAllStats.m_dData[TYPE_95] = ( uint64_t ) m_tQueryTimeDigest.Percentile( 95 );
	tQueryAllStats.m_dData[TYPE_99] = ( uint64_t ) m_tQueryTimeDigest.Percentile( 99 );
	tQueryAllStats.m_uTotalQueries = m_uTotalQueries;
}

//////////////////////////////////////////////////////////////////////////
RunningIndex_c::~RunningIndex_c()
{
	if ( m_bLeaked )
		auto VARIABLE_IS_NOT_USED p = m_pIndex.release();
	if ( m_pIndex )
		m_pIndex->Dealloc();
	if ( !m_sUnlink.IsEmpty() )
	{
		sphLogDebug ( "unlink %s", m_sUnlink.cstr() );
		sphUnlinkIndex ( m_sUnlink.cstr(), false );
	}
}

void ServedIndex_c::UpdateMass () const NO_THREAD_SAFETY_ANALYSIS
{
	CSphIndexStatus tStatus;
	m_pIndex->m_pIndex->GetStatus ( &tStatus );
	// break const, since mass value is not critical for races
	m_iMass = (int) CalculateMass ( tStatus );
}

// Get index mass
uint64_t ServedIndex_c::GetIndexMass ( const ServedIndex_c* pServed )
{
	return pServed ? pServed->m_iMass : 0;
}

void ServedIndex_c::SetIdx ( std::unique_ptr<CSphIndex>&& pIndex ) NO_THREAD_SAFETY_ANALYSIS
{
	assert ( !m_pIndex );
	m_pIndex = new RunningIndex_c;
	m_pIndex->m_pIndex = std::move ( pIndex );
	if ( !m_pStats )
		m_pStats = new ServedStats_c;
}

void ServedIndex_c::ReleaseIdx() const NO_THREAD_SAFETY_ANALYSIS
{
	if ( m_pIndex )
		m_pIndex->m_bLeaked = true;
}

void ServedIndex_c::SetIdxAndStatsFrom ( const ServedIndex_c& tIndex ) NO_THREAD_SAFETY_ANALYSIS
{
	m_pIndex = tIndex.m_pIndex;
	m_pStats = tIndex.m_pStats;
}

void ServedIndex_c::SetStatsFrom ( const ServedIndex_c& tIndex ) NO_THREAD_SAFETY_ANALYSIS
{
	m_pStats = tIndex.m_pStats;
}

void ServedIndex_c::SetUnlink ( CSphString sUnlink ) const
{
	if ( m_pIndex )
		m_pIndex->m_sUnlink = std::move ( sUnlink );
}

void LightClone ( ServedIndexRefPtr_c& pTarget, const cServedIndexRefPtr_c& pSource )
{
	assert ( pTarget );
	assert ( pSource );
	auto& tDesc = (ServedDesc_t&)*pTarget;
	tDesc = *pSource;
}

void FullClone ( ServedIndexRefPtr_c& pTarget, const cServedIndexRefPtr_c& pSource )
{
	LightClone ( pTarget, pSource );
	pTarget->SetIdxAndStatsFrom ( *pSource );
}

//////////////////////////////////////////////////////////////////////////
ServedIndexRefPtr_c& ServedClone_c::LightCloneOnce()
{
	if ( !m_pTarget )
		m_pTarget = MakeLightClone ( m_pSource );
	return m_pTarget;
}

ServedIndexRefPtr_c& ServedClone_c::FullCloneOnce()
{
	if ( !m_pTarget )
		m_pTarget = MakeFullClone ( m_pSource );
	return m_pTarget;
}

HashedServedClone_c::HashedServedClone_c ( CSphString sIndex, ReadOnlyServedHash_c* pHash )
	: ServedClone_c { pHash->Get ( sIndex ) }
	, m_sIndex { std::move ( sIndex ) }
	, m_pHash { pHash }
{}

HashedServedClone_c::~HashedServedClone_c()
{
	if ( !CloneRef() )
		return;
	m_pHash->Replace ( CloneRef(), m_sIndex );
}

//////////////////////////////////////////////////////////////////////////

ServedIndexRefPtr_c MakeServedIndex()
{
	return ServedIndexRefPtr_c { new ServedIndex_c };
}

ServedIndexRefPtr_c MakeLightClone( const cServedIndexRefPtr_c& pSource )
{
	auto pRes = MakeServedIndex();
	LightClone ( pRes, pSource );
	return pRes;
}

ServedIndexRefPtr_c MakeFullClone ( const cServedIndexRefPtr_c& pSource )
{
	auto pRes = MakeServedIndex();
	FullClone ( pRes, pSource );
	return pRes;
}

//////////////////////////////////////////////////////////////////////////

CSphString GetMacAddress()
{
	StringBuilder_c sMAC( ":" );

#if _WIN32
	CSphFixedVector<IP_ADAPTER_ADDRESSES> dAdapters ( 128 );
	PIP_ADAPTER_ADDRESSES pAdapter = dAdapters.Begin();
	auto uSize = (DWORD) dAdapters.GetLengthBytes();
	if ( GetAdaptersAddresses ( 0, 0, nullptr, pAdapter, &uSize )==NO_ERROR )
	{
		while ( pAdapter )
		{
			if ( pAdapter->IfType == IF_TYPE_ETHERNET_CSMACD && pAdapter->PhysicalAddressLength>=6 )
			{
				const BYTE * pMAC = pAdapter->PhysicalAddress;
				for ( DWORD i=0; i<pAdapter->PhysicalAddressLength; i++ )
				{
					sMAC.Appendf ( "%02x", *pMAC );
					pMAC++;
				}
				break;
			}
			pAdapter = pAdapter->Next;
		}
	}
#elif defined(__FreeBSD__)
	size_t iLen = 0;
	const int iMibLen = 6;
	int dMib[iMibLen] = { CTL_NET, AF_ROUTE, 0, AF_LINK, NET_RT_IFLIST, 0 };

	if ( sysctl ( dMib, iMibLen, NULL, &iLen, NULL, 0 )!=-1 )
	{
		CSphFixedVector<char> dBuf ( iLen );
		if ( sysctl ( dMib, iMibLen, dBuf.Begin(), &iLen, NULL, 0 )>=0 )
		{
			if_msghdr * pIf = nullptr;
			for ( const char * pNext = dBuf.Begin(); pNext<dBuf.Begin() + iLen; pNext+=pIf->ifm_msglen )
			{
				pIf = (if_msghdr *)pNext;
				if ( pIf->ifm_type==RTM_IFINFO )
				{
					bool bAllZero = true;
					const sockaddr_dl * pSdl= (const sockaddr_dl *)(pIf + 1);
					const BYTE * pMAC = (const BYTE *)LLADDR(pSdl);
					for ( int i=0; i<ETHER_ADDR_LEN; i++ )
					{
						BYTE uPart = *pMAC;
						pMAC++;
						bAllZero &= ( uPart==0 );
						sMAC.Appendf ( "%02x", uPart );
					}

					if ( !bAllZero )
						break;

					sMAC.Clear();
					sMAC.StartBlock ( ":" );
				}
			}
		}
	}
#elif defined ( __APPLE__ )
	// no MAC address for OSX

#else
	int iFD = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( iFD>=0 )
	{
		ifreq dIf[64];
		ifconf tIfConf;
		tIfConf.ifc_len = sizeof( dIf );
		tIfConf.ifc_req = dIf;

		if ( ioctl( iFD, SIOCGIFCONF, &tIfConf )>=0 )
		{
			const ifreq* pIfEnd = dIf + ( tIfConf.ifc_len / sizeof( dIf[0] ));
			for ( const ifreq* pIfCur = tIfConf.ifc_req; pIfCur<pIfEnd; pIfCur++ )
			{
				if ( pIfCur->ifr_addr.sa_family==AF_INET )
				{
					ifreq tIfCur;
					memset( &tIfCur, 0, sizeof( tIfCur ));
					memcpy( tIfCur.ifr_name, pIfCur->ifr_name, sizeof( tIfCur.ifr_name ));
					if ( ioctl( iFD, SIOCGIFHWADDR, &tIfCur )>=0 )
					{
						bool bAllZero = true;
						const BYTE* pMAC = ( const BYTE* ) tIfCur.ifr_hwaddr.sa_data;
						for ( int i = 0; i<ETHER_ADDR_LEN; i++ )
						{
							BYTE uPart = *pMAC;
							pMAC++;
							bAllZero &= ( uPart==0 );
							sMAC.Appendf( "%02x", uPart );
						}

						if ( !bAllZero )
							break;

						sMAC.Clear();
						sMAC.StartBlock( ":" );
					}
				}
			}
		}
	}
	SafeClose( iFD );
#endif

	return sMAC.cstr();
}


volatile bool& sphGetSeamlessRotate() noexcept
{
#if _WIN32
	static bool bSeamlessRotate = false;
#else
	static bool bSeamlessRotate = true;
#endif
	return bSeamlessRotate;
}

bool PollOptimizeRunning ( const CSphString & sIndex )
{
	while ( true )
	{
		Threads::Coro::SleepMsec ( 500 );
		auto pTmpIndex = GetServed ( sIndex );
		if ( !ServedDesc_t::IsMutable ( pTmpIndex ) )
			return false;

		RIdx_T<RtIndex_i *> pRtIndex { pTmpIndex };
		if ( !pRtIndex->OptimizesRunning () )
			return true;
	}
}