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

/// @file sphinxutils.h
/// Declarations for the stuff shared by all Sphinx utilities.

#ifndef _sphinxutils_
#define _sphinxutils_

#include <ctype.h>
#include <stdarg.h>

#include "sphinxstd.h"

//////////////////////////////////////////////////////////////////////////

/// my own isalpha (let's build our own theme park!)
inline int sphIsAlpha ( int c )
{
	return ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='-' || c=='_';
}

inline bool sphIsInteger ( char c )
{
	return ( c>='0' && c<='9' ) || c=='+' || c=='-';
}

/// symbols allowed in numbers (like 1.84E-20)
inline bool sphIsDigital ( char c )
{
	return ( c>='0' && c<='9' ) || c=='.' || c=='E' || c=='e' || c=='+' || c=='-';
}

/// pointer to the last number in the buf, touching it's end
inline const char * sphFindLastNumeric ( const char * pBuf, int iLen )
{
	if ( !pBuf || !iLen )
		return pBuf;

	for ( auto pLast = pBuf + iLen - 1; pLast>=pBuf; --pLast )
		if ( !sphIsDigital ( *pLast ) )
			return pLast + 1;

	return pBuf;
}

/// my own isspace
inline bool sphIsSpace ( int iCode )
{
	return memchr ( " \t\n\r", iCode, 4 ) != nullptr;
}


/// check for keyword modifiers
inline bool sphIsModifier ( int iSymbol )
{
	return iSymbol=='^' || iSymbol=='$' || iSymbol=='=' || iSymbol=='*';
}


/// all wildcards
template < typename T >
inline bool sphIsWild ( T c )
{
	return c=='*' || c=='?' || c=='%';
}

namespace sph {

	/* Custom format specifiers for types:

	'int' values:
	%d - decimal int
	%F - fixed-point int (see notes below about fixed-points)
	%i - skip int. Prints nothing, just skips parameter. Useful for conditional format.

	'DWORD' (uint32) values:
	%u - decimal uint32
	%x - hex uint32

	'int64_t' (signed) values:
	%l - decimal int64
	%D - fixed-point int64
	%t - timespan (see notes below)
	%T - timestamp from now

	'uint64_t' (unsigned) values:
	%U - decimal uint64

	z-terminated string:
	%s - print string, or "(null)"

	etc.:
	%p - print pointer (16 hex digits)
	%f - float (fall-back to standard sprintf)

	Fixed-point ints (both 32 and 64 bits) need precise param to set the decimal point at this place
	Example:	( "%.4F", 999005 ) will output '99.9005'.
				( "%.3D", (int64_t) -10000 ) will output '-10.000'

	Timespan prints time expressed in useconds in human-readable format. It output number with suffix.
	Suffixes are 'us', 'ms', 's', 'm', 'h', 'd', 'w' (usecs, msecs, secs, mins, hours, days, weeks).
	Example:	("%t", 1555555) will print "2s" (2 seconds)
				("%t", 3600000000*24*2) will print "2d" (2 days)

	Timespan may be supplied with precision param, and then it will print up to 7 numbers with suffixes.
	Example:	("%.2t", 1555555) will print "1.56s"
				("%.5t", 1555555) will print "1s 555.56ms"
				("%.8t", 71555555) will print "1m 11s 555ms 555us"
	Timespan also reasonable rounds the value when necessary and doesn't output redundant zeros.
	Example:	("%.7t", 89999994) will print "1m 29s 999.99ms"
				("%.7t", 89999995) will print "1m 30s"

	Timestamp from now - just takes provided value, compares it with sphMicroTimer() output
	and print the timespan difference according to the rules:
		- if given value is in the past, print 'TM ago' (where TM is timespan of the difference)
		- if given value is in the future, print 'in TM'.
		- if given value is exactly now, print the word 'now'.
	 */
	int vSprintf ( char * pOutput, const char * sFmt, va_list ap );
	int Sprintf ( char * pOutput, const char * sFmt, ... );
	void vSprintf ( StringBuilder_c &pOutput, const char * sFmt, va_list ap );
	void Sprintf ( StringBuilder_c& pOutput, const char * sFmt, ...);

	/// output fVal with arbitrary 6 or 8 digits
	/// ensure that sBuffer has enough space to fit fVal!
	int PrintVarFloat ( char* sBuffer, float fVal );
	int PrintVarDouble ( char* sBuffer, double fVal );
}

/// string splitter, extracts sequences of alphas (as in sphIsAlpha)
void sphSplit ( StrVec_t & dOut, const char * sIn );

/// string splitter, splits by the given boundaries
void sphSplit ( StrVec_t & dOut, const char * sIn, const char * sBounds );
void sphSplit ( StrVec_t & dOut, const char * sIn, int iLen, const char * sBounds );
StrVec_t sphSplit ( const char * sIn, const char * sBounds );
StrVec_t sphSplit ( const char * sIn, int iLen, const char * sBounds );

/// perform sphSplit by whitespaces, but applies a functor instead of add a chunk to the vector
using StrFunctor = std::function<void ( const char*, int )>;
void sphSplitApply ( const char * sIn, int iSize, StrFunctor &&dFunc );

/// string wildcard matching (case-sensitive, supports * and ? patterns)
bool sphWildcardMatch ( const char * sSstring, const char * sPattern, const int * pPattern = NULL );

/// parse size from text (int, or K/M/G/T suffix), or return provided default value.
/// *ppErr, if provided, will point to parsing error, if any
int64_t sphGetSize64 ( const char * sValue, char ** ppErr = nullptr, int64_t iDefault=-1 );

/// parse time from text (seconds, or us/ms/s/m/h/d/w suffix), or return provided default value.
/// \return result in uSeconds, i.e. may be directly compared with sphMicroTimer(), etc.
/// *ppErr, if provided, will point to parsing error, if any. By default scale is 's', seconds.
int64_t sphGetTime64 ( const char* sValue, char** ppErr = nullptr, int64_t iDefault = -1 );
//////////////////////////////////////////////////////////////////////////

namespace sph
{
	template<typename FnFilter>
	void Split ( const char * sIn, int iLen, const char * sBounds, FnFilter fnFilter )
	{
		if ( !sIn )
			return;

		const char * p = sIn;
		if ( iLen<0 ) iLen = (int)strlen (sIn);
		const char * pEnd = p + iLen;
		while (p<pEnd)
		{
			// skip until the first non-boundary character
			const char * sNext = p;
			while (p<pEnd && !strchr ( sBounds, *p ))
				++p;

			// add the token, skip the char
			fnFilter ( sNext, int(p-sNext) );

			// skip all boundaries
			while (p<pEnd && strchr ( sBounds, *p ))
				++p;
		}
	}

	// common parser for 'foo=1;bar=2;baz=str, me=here' constructions.
	SmallStringHash_T<CSphString> ParseKeyValueStrings ( const char * sBuf );
	SmallStringHash_T<CSphVariant> ParseKeyValueVars ( const char * sBuf );

	template<typename FnFilter>
	void ParseKeyValues ( const char * sBuf, FnFilter && fnFilter )
	{
		if ( sBuf && ( *sBuf ))
		{
			sph::Split ( sBuf, -1, ",; \t\n\r", [&] (const char* sToken, int iLen)
			{
				auto dOption = sphSplit ( sToken, iLen, "=" );
				assert ( dOption.GetLength ()==2 ); // as 'key' = 'value'
				dOption.Apply ( [] ( CSphString & sVal ) { sVal.Trim (); } );
				fnFilter ( std::move ( dOption[0] ), std::move ( dOption[1] ));
			});
		}
	}

	/// zero-copy split by the given boundaries, result valid until sIn lives.
	void Split ( StrtVec_t& dOut, const char* sIn, const char* sBounds );
	void Split ( StrtVec_t& dOut, const char* sIn, int iLen, const char* sBounds );
	StrtVec_t Split ( const char* sIn, const char* sBounds );
	StrtVec_t Split ( const char* sIn, int iLen, const char* sBounds );

	/// zero-copy trim leading and trailing spaces; result valid until tIn lives
	Str_t Trim ( Str_t tIn );
	Str_t Trim ( Str_t tIn, char cGarbage );

} // namespace sph

/// config section (hash of variant values)
class CSphConfigSection : public SmallStringHash_T < CSphVariant >
{
public:
	CSphConfigSection ()
		: m_iTag ( 0 )
	{}

	void AddEntry ( const char * szKey, const char * szValue );

	/// get integer option value by key and default value
	int GetInt ( const char * sKey, int iDefault=0 ) const
	{
		CSphVariant * pEntry = (*this)( sKey );
		return pEntry ? pEntry->intval() : iDefault;
	}

	/// get float option value by key and default value
	float GetFloat ( const char * sKey, float fDefault=0.0f ) const
	{
		CSphVariant * pEntry = (*this)( sKey );
		return pEntry ? pEntry->floatval() : fDefault;
	}

	/// get string option value by key and default value
	CSphString GetStr ( const char * sKey, const char * sDefault="" ) const
	{
		CSphVariant * pEntry = (*this)( sKey );
		return pEntry ? pEntry->strval() : sDefault;
	}

	/// get bool option value by key and default value
	bool GetBool ( const char * sKey, bool bDefault = true ) const
	{
		CSphVariant * pEntry = ( *this ) ( sKey );
		return pEntry ? (pEntry->intval ()!=0) : bDefault;
	}

	/// get size option (plain int, or with K/M suffix) value by key and default value
	int		GetSize ( const char * sKey, int iDefault ) const;
	int64_t GetSize64 ( const char * sKey, int64_t iDefault ) const;

	/// get time option in useconds (1000*1000 useconds = 1 second)
	/// (plain integer, or with us/ms/s/m/h/d/w suffix) value by key and default value
	/// if no key specified, GetTime64 considers num in seconds, GetTime64ms in milliseconds.
	int64_t GetUsTime64S ( const char* sKey, int64_t iDefault ) const;
	int64_t GetUsTime64Ms ( const char* sKey, int64_t iDefault ) const;

	// same as GetUsTime, but returns value expressed in int milliseconds and seconds
	int GetSTimeS ( const char* sKey, int iDefault=0 ) const; // default seconds
	int GetMsTimeMs ( const char* sKey, int iDefault=0 ) const; // default milliseconds

	void Swap ( CSphConfigSection& other ) noexcept
	{
		::Swap(m_iTag, other.m_iTag);
		SmallStringHash_T<CSphVariant>::Swap(other);
	}

	/// copying
	CSphConfigSection& operator= ( CSphConfigSection rhs )
	{
		Swap ( rhs );
		return *this;
	}

	int m_iTag;
};

/// config section type (hash of sections)
using CSphConfigType = SmallStringHash_T < CSphConfigSection >;

/// config (hash of section types)
using CSphConfig = SmallStringHash_T < CSphConfigType >;

bool TryToExec ( char * pBuffer, const char * szFilename, CSphVector<char> & dResult, const char * sArgs=nullptr );

/////////////////////////////////////////////////////////////////////////////

/// Provided or default config file
const char *	sphGetConfigFile ( const char * sHint = nullptr );

/// load config file (will die inside if an error happened)
CSphConfig		sphLoadConfig ( const char * sOptConfig, bool bQuiet, bool bIgnoreIndexes, const char ** ppActualConfig=nullptr );

/// load config file into provided hConfig (on error hConfig is unchanged)
bool			ParseConfig ( CSphConfig* pConfig, const char* sFileName, const char* pBuffer = nullptr );

enum ESphLogLevel : BYTE
{
	SPH_LOG_FATAL	= 0,
	SPH_LOG_WARNING	= 1,
	SPH_LOG_INFO	= 2,
	SPH_LOG_DEBUG	= 3,
	SPH_LOG_RPL_DEBUG	= 4,
	SPH_LOG_VERBOSE_DEBUG = 5,
	SPH_LOG_VERY_VERBOSE_DEBUG = 6,
	SPH_LOG_MAX = SPH_LOG_VERY_VERBOSE_DEBUG
};

extern ESphLogLevel g_eLogLevel;		// current log level, can be changed on the fly

typedef void ( *SphLogger_fn )( ESphLogLevel, const char *, va_list );
volatile SphLogger_fn& g_pLogger();

void sphLogVa ( const char * sFmt, va_list ap, ESphLogLevel eLevel = SPH_LOG_WARNING );
void sphWarning_impl ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogf ( ESphLogLevel eLevel, const char* sFmt, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) ); // NOLINT;
void sphInfo_impl ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogFatal ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebug_impl ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebugv_impl ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebugvv_impl ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebugRpl_impl ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT

#define sphWarning( ... ) do if ( g_eLogLevel>=SPH_LOG_WARNING ) sphWarning_impl (__VA_ARGS__); while(0)
#define sphInfo( ... ) do if ( g_eLogLevel>=SPH_LOG_INFO ) sphInfo_impl (__VA_ARGS__); while(0)
#define sphLogDebug( ... ) do if ( g_eLogLevel>=SPH_LOG_DEBUG ) sphLogDebug_impl (__VA_ARGS__); while(0)
#define sphLogDebugv( ... ) do if ( g_eLogLevel>=SPH_LOG_VERBOSE_DEBUG ) sphLogDebugv_impl (__VA_ARGS__); while(0)
#define sphLogDebugvv( ... ) do if ( g_eLogLevel>=SPH_LOG_VERY_VERBOSE_DEBUG ) sphLogDebugvv_impl (__VA_ARGS__); while(0)
#define sphLogDebugRpl( ... ) do if ( g_eLogLevel>=SPH_LOG_RPL_DEBUG ) sphLogDebugRpl_impl (__VA_ARGS__); while(0)

// set the prefix to supress the log
void sphLogSupress ( const char * sPrefix, ESphLogLevel eLevel = SPH_LOG_WARNING );
void sphLogSupressRemove ( const char * sPrefix, ESphLogLevel eLevel = SPH_LOG_WARNING );

// TimePrefixed logging - output "prefix [ms] ...'; 'ms' is reset to 0 by TimeStart().
namespace TimePrefixed {
	void TimeStart ();
	int64_t TimeStamp();
	void LogDebugv ( const char* sPrefix, const char* sFmt, ... );
}

namespace CustomLog {
	void Warning_impl ( const char* sFmt, ... );
	void Info_impl ( const char* sFmt, ... );
}

#define LogWarning( ... ) do if ( g_eLogLevel >= SPH_LOG_WARNING ) CustomLog::Warning_impl ( __VA_ARGS__ ); while ( 0 )
#define LogInfo( ... ) do if ( g_eLogLevel >= SPH_LOG_INFO ) CustomLog::Info_impl ( __VA_ARGS__ ); while ( 0 )

//////////////////////////////////////////////////////////////////////////

/// how do we properly exit from the crash handler?
#if !_WIN32
	#define CRASH_EXIT_CORE { signal ( sig, SIG_DFL ); kill ( getpid(), sig ); }
	#ifndef NDEBUG
		// UNIX debug build, die and dump core
		#define CRASH_EXIT CRASH_EXIT_CORE
	#else
		// UNIX release build, just die
		#define CRASH_EXIT { sphQuickExit ( 2 ); }
	#endif
#else
	#define CRASH_EXIT_CORE return EXCEPTION_CONTINUE_SEARCH
	#ifndef NDEBUG
		// Windows debug build, show prompt to attach debugger
		#define CRASH_EXIT CRASH_EXIT_CORE
	#else
		// Windows release build, just die
		#define CRASH_EXIT return EXCEPTION_EXECUTE_HANDLER
	#endif
#endif

/// async safe, BUT NOT THREAD SAFE, fprintf
void sphSafeInfo ( int iFD, const char * sFmt, ... );

#if !_WIN32
/// UNIX backtrace gets printed out to a stream
void sphBacktrace ( int iFD, bool bSafe=false );
#else
/// Windows minidump gets saved to a file
void sphBacktrace ( EXCEPTION_POINTERS * pExc, const char * sFile );
#endif

/// dummy call of backtrace to alloc internal structures and prevent deadlock at malloc on crash
void sphBacktraceInit();

/// calls actual (libc or jemalloc) malloc stats
void sphMallocStats ( const char* szParams = nullptr );

/// actualy dump of process sName with pid sPid and sink output to iFD
/// (warning, that function uses fork!)
bool sphDumpGdb ( int iFD, const char* sName, const char* sPid );

void sphBacktraceSetBinaryName ( const char * sName );

volatile int& getParentPID ();	/// set by watchdog init and allows children to signal parent
volatile bool& getSafeGDB (); /// if we detected that jemalloc is available

/// plain backtrace - returns static buffer with the text of the call stack
const char * DoBacktrace ( int iDepth=0, int iSkip=0 );

void sphCheckDuplicatePaths ( const CSphConfig & hConf );

/// set globals from the common config section
void sphConfigureCommon ( const CSphConfig & hConf );

/// my own is chinese
bool sphIsChineseCode ( int iCode );

/// detect chinese chars in a buffer
bool sphDetectChinese ( const BYTE * szBuffer, int iLength );

class CSphDynamicLibrary : public ISphNoncopyable
{
	bool		m_bReady; // whether the lib is valid or not
	void *		m_pLibrary; // internal handle

public:
	explicit CSphDynamicLibrary ( const char* sPath );

	// We are suppose, that library is loaded once when necessary, and will alive whole lifetime of utility.
	// So, no need to explicitly desctruct it, this is intended leak.
	~CSphDynamicLibrary ();

	bool		LoadSymbols ( const char** sNames, void*** pppFuncs, int iNum );
};

/// collect warnings/errors from any suitable context.
/// on multiple calls appends new message, separating it with '; ' from previous.
class Warner_c : public ISphNoncopyable
{
	StringBuilder_c m_sWarnings;
	StringBuilder_c m_sErrors;

	const char * m_sDel = nullptr;
	const char * m_sPref = nullptr;
	const char * m_sTerm = nullptr;

public:
	Warner_c ( const char * sDel = ", ", const char * sPref = nullptr, const char * sTerm = nullptr );
	Warner_c ( Warner_c&& rhs ) noexcept;
	Warner_c& operator= ( Warner_c &&rhs ) noexcept;

	// append message as error.
	// always return false (in order to simplify pattern {error='foo'; return false;})
	void Err ( const char * sFmt, ... );
	void Err ( const CSphString &sMsg );
	StringBuilder_c& Err() { return m_sErrors; }
	void Warn ( const char * sFmt, ... );
	void Warn ( const CSphString &sMsg );
	StringBuilder_c& Warn() { return m_sWarnings; }

	void Clear ();

	const char * sError () const;
	const char * sWarning () const;

	bool ErrEmpty () const { return m_sErrors.IsEmpty (); }
	bool WarnEmpty () const { return m_sWarnings.IsEmpty (); }

	void AddStringsFrom ( const Warner_c &sSrc );
	void MoveErrorsTo ( CSphString &sTarget );
	void MoveWarningsTo ( CSphString &sTarget );
	void MoveAllTo ( CSphString &sTarget );
};

namespace TlsMsg
{
	// format error, then return false
	bool Err( const char* sFmt, ... );

	// put error from string, then return false
	bool Err( const CSphString& sMsg );

	// clear current state and return builder for user manipulations
	StringBuilder_c& Err();

	// return last error
	const char* szError();

	// move error to given string, or leave it intact if no error
	void MoveError( CSphString& sError );

	// true if some error was reported.
	bool HasErr();

	// RAII keep previous msg; restore it on destroy
	class KeepError_c : public ISphNoncopyable, public ISphNonmovable
	{
		CSphString m_sPrevError;
	public:
		KeepError_c() { MoveError ( m_sPrevError ); }
		~KeepError_c() { Err ( m_sPrevError); }

		// to use as legacy error collector
		operator CSphString&() { return m_sPrevError;}
	};
}

// extract basename from path
const char * GetBaseName ( const CSphString & sFullPath );
bool HasMvaUpdated ( const CSphString & sIndexPath );

// uuid short generator - static across daemon
//	bytes	value
//	1		server_id & 0x7f
//	4		startup time of server in seconds
//	3		increment base part
int64_t	UidShort();

// server - is server id used as iServer & 0x7f
// started - is a server start time \ Unix timestamp in seconds
void		UidShortSetup ( int iServer, int iStarted );

BYTE Pearson8 ( const BYTE * pBuf, int iLen );

#if _WIN32
void		CheckWinInstall();
CSphString	GetWinInstallDir();
CSphString	AppendWinInstallDir ( const CSphString & sDir );
#endif

#endif // _sphinxutils_
