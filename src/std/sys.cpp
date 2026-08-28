//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
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

#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
	#define SPH_CPU_X86 1
#else
	#define SPH_CPU_X86 0
#endif

#if SPH_CPU_X86 && !defined(_MSC_VER)
	#include <cpuid.h>
#endif

#if SPH_CPU_X86 && defined(_MSC_VER)
	#include <intrin.h>
#endif

#if _WIN32
	#include <immintrin.h>
#else
	#include <unistd.h>
#endif


#if SPH_CPU_X86
static void GetCPUID ( uint32_t uLeaf, uint32_t dInfo[4] )
{
#if defined(_MSC_VER)
	int dCPUInfo[4];
	__cpuidex ( dCPUInfo, (int)uLeaf, 0 );
	dInfo[0] = (uint32_t)dCPUInfo[0];
	dInfo[1] = (uint32_t)dCPUInfo[1];
	dInfo[2] = (uint32_t)dCPUInfo[2];
	dInfo[3] = (uint32_t)dCPUInfo[3];
#else
	__cpuid ( uLeaf, dInfo[0], dInfo[1], dInfo[2], dInfo[3] );
#endif
}


static void GetCPUID ( uint32_t uLeaf, uint32_t uSubLeaf, uint32_t dInfo[4] )
{
#if defined(_MSC_VER)
	int dCPUInfo[4];
	__cpuidex ( dCPUInfo, (int)uLeaf, (int)uSubLeaf );
	dInfo[0] = (uint32_t)dCPUInfo[0];
	dInfo[1] = (uint32_t)dCPUInfo[1];
	dInfo[2] = (uint32_t)dCPUInfo[2];
	dInfo[3] = (uint32_t)dCPUInfo[3];
#else
	__cpuid_count ( uLeaf, uSubLeaf, dInfo[0], dInfo[1], dInfo[2], dInfo[3] );
#endif
}
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
#if SPH_CPU_X86
	uint32_t dInfo[4];
	GetCPUID ( 1, dInfo );
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


static bool IsAVX512SupportedImpl()
{
#if SPH_CPU_X86
	uint32_t dInfo[4];
	GetCPUID ( 1, dInfo );
	bool bHasAVX = (dInfo[2] & (1 << 28)) != 0;
	bool bHasOSXSAVE = (dInfo[2] & (1 << 27)) != 0;
	if ( !bHasAVX || !bHasOSXSAVE )
		return false;

#if defined(_MSC_VER)
	uint64_t uXcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
#else
	uint32_t eax, edx;
	__asm__ volatile ( "xgetbv" : "=a"(eax), "=d"(edx) : "c"(0) );
	uint64_t uXcrFeatureMask = ((uint64_t)edx << 32) | eax;
#endif
	if ( ( uXcrFeatureMask & 0xe6 ) != 0xe6 )
		return false;

	GetCPUID ( 7, 0, dInfo );
	bool bHasAVX512F = ( dInfo[1] & ( 1u << 16 ) ) != 0;
	bool bHasAVX512VPOPCNTDQ = ( dInfo[2] & ( 1u << 14 ) ) != 0;
#if _WIN32
	bool bHasAVX512DQ = ( dInfo[1] & ( 1u << 17 ) ) != 0;
	bool bHasAVX512CD = ( dInfo[1] & ( 1u << 28 ) ) != 0;
	bool bHasAVX512BW = ( dInfo[1] & ( 1u << 30 ) ) != 0;
	bool bHasAVX512VL = ( dInfo[1] & ( 1u << 31 ) ) != 0;
	return bHasAVX512F
		&& bHasAVX512DQ
		&& bHasAVX512CD
		&& bHasAVX512BW
		&& bHasAVX512VL
		&& bHasAVX512VPOPCNTDQ;
#else
	return bHasAVX512F && bHasAVX512VPOPCNTDQ;
#endif
#else
	return true;	// assumes that it's ARM and simde is used
#endif
}


bool IsAVX512Supported()
{
	static bool bAVX512 = IsAVX512SupportedImpl();
	return bAVX512;
}


static bool IsAVX2SupportedImpl()
{
#if SPH_CPU_X86
    uint32_t dInfo[4];
	GetCPUID ( 1, dInfo );
    bool bHasAVX = (dInfo[2] & (1 << 28)) != 0;
    bool bHasOSXSAVE = (dInfo[2] & (1 << 27)) != 0;
    if ( !bHasAVX || !bHasOSXSAVE )
        return false;

#if defined(_MSC_VER)
    uint64_t uXcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
#else
    uint32_t eax, edx;
    __asm__ volatile ( "xgetbv" : "=a"(eax), "=d"(edx) : "c"(0) );
    uint64_t uXcrFeatureMask = ((uint64_t)edx << 32) | eax;
#endif
    if ( ( uXcrFeatureMask & 0x6 ) != 0x6 )
        return false;

	GetCPUID ( 7, 0, dInfo );
    return (dInfo[1] & (1 << 5)) != 0;
#else
    return true;	// assumes that it's ARM and simde is used
#endif
}


bool IsAVX2Supported()
{
	static bool bAVX2 = IsAVX2SupportedImpl();
	return bAVX2;
}


static bool IsPOPCNTSupportedImpl()
{
#if SPH_CPU_X86
	uint32_t dInfo[4];
	GetCPUID ( 1, dInfo );
	return (dInfo[2] & (1 << 23)) != 0;
#else
	return true;	// assumes that it's ARM and simde is used
#endif
}


bool IsPOPCNTSupported()
{
	static bool bPopcnt = IsPOPCNTSupportedImpl();
	return bPopcnt;
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
