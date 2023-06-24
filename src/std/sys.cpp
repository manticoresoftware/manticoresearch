//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#include "sys.h"
#include "fixedvector.h"
#include "string.h"
#include <stdlib.h>
#include <set>

#if defined(__x86_64__) || defined(__i386__)
	#include <cpuid.h>
#endif

#if !_WIN32
	#include <unistd.h>
#endif


static int GetNumLogicalCPUsImpl()
{
#if _WIN32
	SYSTEM_INFO tInfo;
	GetSystemInfo ( &tInfo );
	return tInfo.dwNumberOfProcessors;
#else
	return sysconf ( _SC_NPROCESSORS_ONLN );
#endif
}


int GetNumLogicalCPUs()
{
	static int iCPUs = GetNumLogicalCPUsImpl();
	return iCPUs;
}


static int GetNumPhysicalCPUsImpl()
{
#if _WIN32
	DWORD uResponseSize = 0;
	if ( GetLogicalProcessorInformationEx ( RelationProcessorCore, nullptr, &uResponseSize ) )
		return -1;

	if ( GetLastError()!=ERROR_INSUFFICIENT_BUFFER )
		return -1;

	CSphFixedVector<uint8_t> dBuffer ( uResponseSize );
	if ( !GetLogicalProcessorInformationEx ( RelationProcessorCore, (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)dBuffer.Begin(), &uResponseSize ) )
		return -1;

	int iNumCPUs = 0;
	uint8_t * pPtr = dBuffer.Begin();
	uint8_t * pMax = pPtr + uResponseSize;
	while ( pPtr < pMax )
	{
		auto pInfo = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *)pPtr;
		pPtr += pInfo->Size;
		iNumCPUs++;
	}

	return iNumCPUs;
#else
	FILE * pFile = fopen ( "/proc/cpuinfo", "r" );
	if ( !pFile )
		return -1;

	using ProcCore_t = std::pair<size_t, size_t>;
	std::set<ProcCore_t> tPCs;

	ProcCore_t tPC;
	char szLine[1024];
	while ( fgets ( szLine, sizeof(szLine), pFile ) )
	{
		const char * pCol = strchr ( szLine, ':' );
		if ( !pCol )
			continue;

		CSphString sKey ( szLine, pCol-szLine );
		CSphString sValue ( pCol+1 );
		sKey.Trim();
		sValue.Trim();
		if ( sKey.Begins ( "physical id" ) )
			tPC.first = atoi ( sValue.cstr() );
		else if ( sKey.Begins ( "core id" ) )
		{
			tPC.second = atoi ( sValue.cstr() );
			tPCs.insert(tPC);
		}
	}

	fclose(pFile);
	return tPCs.empty() ? -1 : tPCs.size();
#endif
}


int GetNumPhysicalCPUs()
{
	static int iCPUs = GetNumPhysicalCPUsImpl();
	return iCPUs;
}


static bool IsSSE42SupportedImpl()
{
#if defined(__x86_64__) || defined(__i386__)
	uint32_t dInfo[4];
	__cpuid ( 1, dInfo[0], dInfo[1], dInfo[2], dInfo[3] );
	return (dInfo[2] & (1 << 20)) != 0;
#else
	return true;	// assumes that it's ARM and simde is used
#endif
}


bool IsSSE42Supported()
{
	static bool bSSE = IsSSE42SupportedImpl();
	return bSSE;
}


static int GetMemPageSizeImpl()
{
#if _WIN32
	SYSTEM_INFO tInfo;
	GetSystemInfo ( &tInfo );
	return tInfo.dwPageSize;
#else
	return getpagesize();
#endif
}


int GetMemPageSize()
{
	static int iMemPageSize = GetMemPageSizeImpl();
	return iMemPageSize;
}
