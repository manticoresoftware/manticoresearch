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

#include "crash_logger.h"
#include "logger.h"

#include "std/fatal.h"
#include "std/thread_annotations.h"
#include "sphinxutils.h"
#include "fileutils.h"
#include "sphinxint.h"
#include "threadutils.h"
#include "client_task_info.h"

#include <sys/types.h>
#include <ostream>
#include <boost/stacktrace.hpp>

constexpr int SPH_TIME_PID_MAX_SIZE = 256;

static bool g_bCoreDump = false;
static bool g_bSafeTrace = false;
static BYTE g_dCrashQueryBuff[16 * 1024];
static char g_sCrashInfo[SPH_TIME_PID_MAX_SIZE] = "[][]\n";
static int g_iCrashInfoLen = 0;
const char g_sCrashedBannerAPI[] = "\n--- crashed SphinxAPI request dump ---\n";

const char g_sCrashedBannerMySQL[] = "\n--- crashed SphinxQL request dump ---\n";
const char g_sCrashedBannerHTTP[] = "\n--- crashed HTTP request dump ---\n";
const char g_sCrashedBannerBad[] = "\n--- crashed invalid query ---\n";
const char g_sCrashedBannerTail[] = "\n--- request dump end ---\n";
const char g_sCrashedIndex[] = "--- local index:";
const char g_sEndLine[] = "\n";
#if _WIN32
const char		g_sMinidumpBanner[] = "minidump located at: ";
static char		g_sMinidump[SPH_TIME_PID_MAX_SIZE] = "";
#endif


void CrashLogger::SetCoredump ( bool bVal )
{
	g_bCoreDump = bVal;
}

void CrashLogger::SetSafeTrace ( bool bVal )
{
	g_bSafeTrace = bVal;
}


struct QueryCopyState_t
{
	BYTE * m_pDst;
	BYTE * m_pDstEnd;
	const BYTE * m_pSrc;
	const BYTE * m_pSrcEnd;
};

using CopyQuery_fn = bool ( * ) ( QueryCopyState_t & tState );

// crash query handler
constexpr int g_iQueryLineLen = 80;
constexpr char g_dEncodeBase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static bool sphCopyEncodedBase64 ( QueryCopyState_t & tEnc )
{
	BYTE * pDst = tEnc.m_pDst;
	const BYTE * pDstBase = tEnc.m_pDst;
	const BYTE * pSrc = tEnc.m_pSrc;
	const BYTE * pDstEnd = tEnc.m_pDstEnd - 5;
	const BYTE * pSrcEnd = tEnc.m_pSrcEnd - 3;

	while ( pDst <= pDstEnd && pSrc <= pSrcEnd )
	{
		// put line delimiter at max line length
		if ( ((pDst - pDstBase) % g_iQueryLineLen) > ((pDst - pDstBase + 4) % g_iQueryLineLen) )
			*pDst++ = '\n';

		// Convert to big endian
		DWORD uSrc = (pSrc[0] << 16) | (pSrc[1] << 8) | (pSrc[2]);
		pSrc += 3;

		*pDst++ = g_dEncodeBase64[(uSrc & 0x00FC0000) >> 18];
		*pDst++ = g_dEncodeBase64[(uSrc & 0x0003F000) >> 12];
		*pDst++ = g_dEncodeBase64[(uSrc & 0x00000FC0) >> 6];
		*pDst++ = g_dEncodeBase64[(uSrc & 0x0000003F)];
	}

	// there is a tail in source data and a room for it at destination buffer
	if ( pSrc < tEnc.m_pSrcEnd && (tEnc.m_pSrcEnd - pSrc < 3) && (pDst <= pDstEnd - 4) )
	{
		int iLeft = (tEnc.m_pSrcEnd - pSrc) % 3;
		if ( iLeft == 1 )
		{
			DWORD uSrc = pSrc[0] << 16;
			pSrc += 1;
			*pDst++ = g_dEncodeBase64[(uSrc & 0x00FC0000) >> 18];
			*pDst++ = g_dEncodeBase64[(uSrc & 0x0003F000) >> 12];
			*pDst++ = '=';
			*pDst++ = '=';
		} else if ( iLeft == 2 )
		{
			DWORD uSrc = (pSrc[0] << 16) | (pSrc[1] << 8);
			pSrc += 2;
			*pDst++ = g_dEncodeBase64[(uSrc & 0x00FC0000) >> 18];
			*pDst++ = g_dEncodeBase64[(uSrc & 0x0003F000) >> 12];
			*pDst++ = g_dEncodeBase64[(uSrc & 0x00000FC0) >> 6];
			*pDst++ = '=';
		}
	}

	tEnc.m_pDst = pDst;
	tEnc.m_pSrc = pSrc;

	return (tEnc.m_pSrc < tEnc.m_pSrcEnd);
}


static bool sphCopySphinxQL ( QueryCopyState_t & tState )
{
	BYTE * pDst = tState.m_pDst;
	const BYTE * pSrc = tState.m_pSrc;
	BYTE * pNextLine = pDst + g_iQueryLineLen;

	while ( pDst < tState.m_pDstEnd && pSrc < tState.m_pSrcEnd )
	{
		if ( pDst > pNextLine && pDst + 1 < tState.m_pDstEnd && (sphIsSpace ( *pSrc ) || *pSrc == ',') )
		{
			*pDst++ = *pSrc++;
			*pDst++ = '\n';
			pNextLine = pDst + g_iQueryLineLen;
		} else
		{
			*pDst++ = *pSrc++;
		}
	}

	tState.m_pDst = pDst;
	tState.m_pSrc = pSrc;

	return (tState.m_pSrc < tState.m_pSrcEnd);
}


static bool sphCopySphinxHttp ( QueryCopyState_t & tState )
{
	BYTE * pDst = tState.m_pDst;
	const BYTE * pSrc = tState.m_pSrc;

	while ( pDst < tState.m_pDstEnd && pSrc < tState.m_pSrcEnd )
	{
		*pDst++ = *pSrc++;
	}

	tState.m_pDst = pDst;
	tState.m_pSrc = pSrc;

	return (tState.m_pSrc < tState.m_pSrcEnd);
}

class FixedStreamBuf : public std::streambuf
{
public:
    FixedStreamBuf ( char * pBuf, int iSize )
	{
		char * pEnd = pBuf + iSize;
        setp ( pBuf, pEnd );
    }

	int GetPos() const { return (int)( pptr()-pbase() ); }

protected:
    int overflow ( int c ) override
	{
        if ( c!=EOF && pptr()<epptr() )
		{
            *pptr() = c;
            pbump ( 1 );
            return c;
        }
        return EOF;
    }

    std::streamsize xsputn ( const char * sBuf, std::streamsize iBufSize ) override
	{
        std::streamsize iCopied = std::min ( iBufSize, static_cast<std::streamsize> ( epptr() - pptr() ) );
        std::copy ( sBuf, sBuf+iCopied, pptr() );
        pbump ( iCopied );
        return iCopied;
    }
};

extern CSphString g_sBannerVersion;
extern CSphString g_sPidFile;

#if !_WIN32
void HandleCrash ( int sig ) NO_THREAD_SAFETY_ANALYSIS
#else
LONG WINAPI HandleCrash ( EXCEPTION_POINTERS * pExc )
#endif // !_WIN32
{
	sphSetDied();
	auto iLogFile = GetDaemonLogFD();
	if ( iLogFile < 0 )
	{
		if ( g_bCoreDump )
		{
			CRASH_EXIT_CORE;
		} else
		{
			CRASH_EXIT;
		}
	}

	sphSafeInfoStdOut ( ForceLogStdout() );

	// log [time][pid]
	sphSeek ( iLogFile, 0, SEEK_END );
	sphSafeInfoWrite ( iLogFile, g_sCrashInfo, g_iCrashInfoLen );

	// log query
	auto & tQuery = GlobalCrashQueryGetRef();

	bool bValidQuery = IsFilled ( tQuery.m_dQuery );
#if !_WIN32
	if ( bValidQuery )
	{
		size_t iPageSize = GetMemPageSize();

		// FIXME! That is too complex way, remove all of this and just move query dump to the bottom
		// remove also mincore_test.cmake, it's invokation from CMakeLists.txt and HAVE_UNSIGNED_MINCORE
		// declatarion from config_cmake.h.in
#if HAVE_UNSIGNED_MINCORE
		BYTE dPages = 0;
#else
		char dPages = 0;
#endif

		auto pPageStart = (uintptr_t) tQuery.m_dQuery.first;
		pPageStart &= ~(iPageSize - 1);
		bValidQuery &= (mincore ( (void *) pPageStart, 1, &dPages ) == 0);

		auto pPageEnd = (uintptr_t) (tQuery.m_dQuery.first + tQuery.m_dQuery.second - 1);
		pPageEnd &= ~(iPageSize - 1);
		bValidQuery &= (mincore ( (void *) pPageEnd, 1, &dPages ) == 0);
	}
#endif

	// request dump banner
	Str_t dBanner = { g_sCrashedBannerAPI, sizeof (g_sCrashedBannerAPI) - 1 };
	if ( tQuery.m_eType == QUERY_SQL )
		dBanner = { g_sCrashedBannerMySQL, sizeof (g_sCrashedBannerMySQL) - 1 };
	else if ( tQuery.m_eType == QUERY_JSON )
		dBanner = { g_sCrashedBannerHTTP, sizeof (g_sCrashedBannerHTTP) - 1 };

	if ( !bValidQuery )
		dBanner = { g_sCrashedBannerBad, sizeof (g_sCrashedBannerBad) - 1 };

	sphSafeInfoWrite ( iLogFile, dBanner.first, dBanner.second );

	// query
	if ( bValidQuery )
	{
		QueryCopyState_t tCopyState;
		tCopyState.m_pDst = g_dCrashQueryBuff;
		tCopyState.m_pDstEnd = g_dCrashQueryBuff + sizeof(g_dCrashQueryBuff);
		tCopyState.m_pSrc = tQuery.m_dQuery.first;
		tCopyState.m_pSrcEnd = tQuery.m_dQuery.first + tQuery.m_dQuery.second;

		CopyQuery_fn pfnCopy = nullptr;
		if ( tQuery.m_eType == QUERY_API )
		{
			pfnCopy = &sphCopyEncodedBase64;

			// should be power of 3 to seamlessly convert to BASE64
			BYTE dHeader[] = {
				(BYTE) ((tQuery.m_uCMD >> 8) & 0xff),
				(BYTE) (tQuery.m_uCMD & 0xff),
				(BYTE) ((tQuery.m_uVer >> 8) & 0xff),
				(BYTE) (tQuery.m_uVer & 0xff),
				(BYTE) ((tQuery.m_dQuery.second >> 24) & 0xff),
				(BYTE) ((tQuery.m_dQuery.second >> 16) & 0xff),
				(BYTE) ((tQuery.m_dQuery.second >> 8) & 0xff),
				(BYTE) (tQuery.m_dQuery.second & 0xff),
				*tQuery.m_dQuery.first
			};

			QueryCopyState_t tHeaderState;
			tHeaderState.m_pDst = g_dCrashQueryBuff;
			tHeaderState.m_pDstEnd = g_dCrashQueryBuff + sizeof(g_dCrashQueryBuff);
			tHeaderState.m_pSrc = dHeader;
			tHeaderState.m_pSrcEnd = dHeader + sizeof(dHeader);
			pfnCopy ( tHeaderState );
			assert ( tHeaderState.m_pSrc==tHeaderState.m_pSrcEnd );
			tCopyState.m_pDst = tHeaderState.m_pDst;
			tCopyState.m_pSrc++;
		} else if ( tQuery.m_eType == QUERY_JSON )
		{
			pfnCopy = &sphCopySphinxHttp;
		} else
		{
			pfnCopy = &sphCopySphinxQL;
		}

		while ( pfnCopy ( tCopyState ) )
		{
			sphSafeInfoWrite ( iLogFile, g_dCrashQueryBuff, tCopyState.m_pDst - g_dCrashQueryBuff );
			tCopyState.m_pDst = g_dCrashQueryBuff; // reset the destination buffer
		}
		assert ( tCopyState.m_pSrc==tCopyState.m_pSrcEnd );

		int iLeft = int ( tCopyState.m_pDst - g_dCrashQueryBuff );
		if ( iLeft > 0 )
			sphSafeInfoWrite ( iLogFile, g_dCrashQueryBuff, iLeft );
	}

	// tail
	sphSafeInfoWrite ( iLogFile, g_sCrashedBannerTail, sizeof(g_sCrashedBannerTail) - 1 );

	// index name
	sphSafeInfoWrite ( iLogFile, g_sCrashedIndex, sizeof (g_sCrashedIndex) - 1 );
	if ( IsFilled ( tQuery.m_dIndex ) )
		sphSafeInfoWrite ( iLogFile, tQuery.m_dIndex.first, tQuery.m_dIndex.second );
	sphSafeInfoWrite ( iLogFile, g_sEndLine, sizeof (g_sEndLine) - 1 );

	sphSafeInfo ( iLogFile, g_sBannerVersion.cstr() );

#if _WIN32
	// mini-dump reference
	int iMiniDumpLen = snprintf ( (char *)g_dCrashQueryBuff, sizeof(g_dCrashQueryBuff),
		"%s %s.%p.mdmp\n", g_sMinidumpBanner, g_sMinidump, tQuery.m_dQuery.first );
	sphSafeInfoWrite ( iLogFile, g_dCrashQueryBuff, iMiniDumpLen );
	snprintf ( (char *)g_dCrashQueryBuff, sizeof(g_dCrashQueryBuff), "%s.%p.mdmp",
		g_sMinidump, tQuery.m_dQuery.first );
	sphBacktrace ( pExc, (char *)g_dCrashQueryBuff );

	// backtrace dump with boost - similar to we got on linux
	FixedStreamBuf tStreamBuf ( (char *)g_dCrashQueryBuff, sizeof ( g_dCrashQueryBuff ) );
	std::ostream tOStream ( &tStreamBuf );
	tOStream << boost::stacktrace::stacktrace();
	int iStacktraceLen = tStreamBuf.GetPos();
	sphSafeInfoWrite ( iLogFile, g_dCrashQueryBuff, iStacktraceLen );
#else

	// log trace
	sphSafeInfo ( iLogFile, "Handling signal %d", sig );
	sphBacktrace ( iLogFile, g_bSafeTrace );
#endif

	// threads table
	sphSafeInfo ( iLogFile, "--- active threads ---" );
	int iThd = 0;
	int iAllThd = 0;
	Threads::IterateActive ( [&iThd,&iAllThd,iLogFile] ( Threads::LowThreadDesc_t * pThread ) {
		if ( pThread )
		{
			auto pSrc = (ClientTaskInfo_t *) pThread->m_pTaskInfo.load ( std::memory_order_relaxed );
			if ( pSrc ) ++iAllThd;
			for ( ; pSrc; pSrc = (ClientTaskInfo_t *) pSrc->m_pPrev.load ( std::memory_order_relaxed ) )
				if ( pSrc->m_eType == ClientTaskInfo_t::Task() )
				{
					sphSafeInfo ( iLogFile, "thd %d (%s), proto %s, state %s, command %s", iThd,
					              pThread->m_sThreadName.cstr(),
					              ProtoName ( pSrc->GetProto() ), TaskStateName ( pSrc->GetTaskState() ),
					              pSrc->m_szCommand ? pSrc->m_szCommand : "-" );
					++iThd;
					break;
				}
		}
	} );

	sphSafeInfo ( iLogFile, "--- Totally %d threads, and %d client-working threads ---", iAllThd, iThd );

	// memory info
#if SPH_ALLOCS_PROFILER
	sphSafeInfoWrite ( iLogFile, g_sMemoryStatBanner, sizeof ( g_sMemoryStatBanner )-1 );
	sphMemStatDump ( iLogFile );
#endif

	sphSafeInfo ( iLogFile, "------- CRASH DUMP END -------" );

	if ( g_bCoreDump )
	{
		CRASH_EXIT_CORE;
	} else
	{
		CRASH_EXIT;
	}
}


void CrashLogger::SetupTimePID ()
{
	char sTimeBuf[SPH_TIME_PID_MAX_SIZE];
	sphFormatCurrentTime ( sTimeBuf, sizeof(sTimeBuf) );

	g_iCrashInfoLen = snprintf ( g_sCrashInfo, SPH_TIME_PID_MAX_SIZE - 1,
	                             "------- FATAL: CRASH DUMP -------\n[%s] [%5d]\n", sTimeBuf, (int) getpid() );
}
#if _WIN32
bool CrashLogger::SetCrashHandler()
{
	CSphString sPrefix = "searchd";
	if ( !g_sPidFile.IsEmpty() )
		sPrefix = g_sPidFile;

	snprintf ( g_sMinidump, SPH_TIME_PID_MAX_SIZE-1, "%s.%d", sPrefix.scstr(), (int)getpid() );
	AddVectoredExceptionHandler ( TRUE, HandleCrash );
	return true;
}
#else
bool CrashLogger::SetCrashHandler ( ) REQUIRES ( MainThread )
{
	sphBacktraceInit();
	struct sigaction sa;
	sigfillset ( &sa.sa_mask );
	sa.sa_flags = SA_NOCLDSTOP;
	sa.sa_flags |= SA_RESETHAND;

	static CSphVector<BYTE> exception_handler_stack ( Max ( SIGSTKSZ, 65536 ) );
	stack_t ss;
	ss.ss_sp = exception_handler_stack.begin();
	ss.ss_flags = 0;
	ss.ss_size = exception_handler_stack.GetLength();
	sigaltstack ( &ss, 0 );
	sa.sa_flags |= SA_ONSTACK;

	sa.sa_handler = HandleCrash;
	if ( sigaction ( SIGSEGV, &sa, NULL ) != 0 ) return false;
	sa.sa_handler = HandleCrash;
	if ( sigaction ( SIGBUS, &sa, NULL ) != 0 ) return false;
	sa.sa_handler = HandleCrash;
	if ( sigaction ( SIGABRT, &sa, NULL ) != 0 ) return false;
	sa.sa_handler = HandleCrash;
	if ( sigaction ( SIGILL, &sa, NULL ) != 0 ) return false;
	sa.sa_handler = HandleCrash;
	if ( sigaction ( SIGFPE, &sa, NULL ) != 0 ) return false;
	return true;
}
#endif
