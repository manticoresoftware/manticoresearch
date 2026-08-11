// Copyright (c) 2001-2026, Manticore Software LTD (https://manticoresearch.com)

#include "searchdaemon.h"

#if !_WIN32
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#endif

#include <string>
#include <vector>

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

static bool IsLocalhost ( DWORD uAddress )
{
	return ( ntohl(uAddress)>>24 )==127;
}

DWORD SelectListenerAddress ( const DWORD * pAddresses, int iCount )
{
	if ( !pAddresses || iCount<=0 )
		return 0;
	DWORD uAddress = pAddresses[0];
	if ( IsLocalhost(uAddress) )
		for ( int i=1; i<iCount; ++i )
			if ( !IsLocalhost(pAddresses[i]) )
				return pAddresses[i];
	return uAddress;
}

static DWORD ResolveIPv4 ( const char * szHost, CSphString * pFatal )
{
	addrinfo tHints {};
	tHints.ai_family = AF_INET;
	tHints.ai_socktype = SOCK_STREAM;
	addrinfo * pResult = nullptr;
	int iError = getaddrinfo ( szHost, nullptr, &tHints, &pResult );
	if ( iError || !pResult )
	{
		MaybeFatalLog ( pFatal, "failed to resolve host '%s': %s", szHost, iError ? gai_strerror(iError) : "no IPv4 address" );
		if ( pResult ) freeaddrinfo ( pResult );
		return 0;
	}

	std::vector<DWORD> dAddresses;
	int iAddresses = 0;
	std::string sAddresses;
	for ( addrinfo * pAddress=pResult; pAddress; pAddress=pAddress->ai_next )
	{
		DWORD uNext = reinterpret_cast<sockaddr_in *>(pAddress->ai_addr)->sin_addr.s_addr;
		dAddresses.push_back ( uNext );
		char szAddress[INET_ADDRSTRLEN] {};
		in_addr tAddress { uNext };
		const char * szText = inet_ntop ( AF_INET, &tAddress, szAddress, sizeof(szAddress) );
		if ( iAddresses++ ) sAddresses += "; ip=";
		else sAddresses = "ip=";
		sAddresses += szText ? szText : "unknown";
	}
	DWORD uAddress = SelectListenerAddress ( dAddresses.data(), (int)dAddresses.size() );
	if ( iAddresses>1 )
	{
		char szSelected[INET_ADDRSTRLEN] {};
		in_addr tSelected { uAddress };
		inet_ntop ( AF_INET, &tSelected, szSelected, sizeof(szSelected) );
		sphWarning ( "multiple addresses (%s) found for '%s', using first one (%s)", sAddresses.c_str(), szHost, szSelected );
	}
	freeaddrinfo ( pResult );
	return uAddress;
}

bool CheckPort ( int iPort, CSphString * pFatal )
{
	if ( iPort<1 || iPort>65535 )
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

	// split by colon. The explicit unix: prefix permits a relative socket path;
	// this is useful after searchd changes into data_dir and avoids sun_path limits.
	bool bExplicitUnix = strncmp ( sSpec, "unix:", 5 )==0;
	if ( bExplicitUnix && !sSpec[5] )
	{
		MaybeFatalLog ( pFatal, "UNIX socket path must not be empty" );
		return {};
	}
	auto dParts = sphSplit ( bExplicitUnix ? sSpec+5 : sSpec, ":" ); // diff. parts are :-separated

	int iParts = dParts.GetLength();
	if ( iParts>3 )
	{
		MaybeFatalLog ( pFatal, "invalid listen format (too many fields)" );
		return {};
	}

	assert ( iParts>=1 && iParts<=3 );

	// handle UNIX socket case
	// might be either an absolute path, or a relative path after the unix: prefix;
	// protocol may be omitted for backward compatibility.
	if ( bExplicitUnix || *dParts[0].scstr()=='/' )
	{
		if ( iParts>2 )
		{
			MaybeFatalLog ( pFatal, "invalid listen format (too many fields)" );
			return {};
		}
		if ( dParts[0].IsEmpty() )
		{
			MaybeFatalLog ( pFatal, "UNIX socket path must not be empty" );
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
			tRes.m_uIP = bResolve ? ResolveIPv4 ( sSpec, pFatal ) : 0;
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
		tRes.m_uIP = bResolve ? ResolveIPv4 ( dParts[0].cstr(), pFatal ) : 0;
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

