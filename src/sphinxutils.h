//
// Copyright (c) 2017-2018, Manticore Software LTD (http://manticoresearch.com)
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
#include "sphinx.h"

//////////////////////////////////////////////////////////////////////////

/// my own isalpha (let's build our own theme park!)
inline int sphIsAlpha ( int c )
{
	return ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='-' || c=='_';
}

/// symbols allowed in numbers (like 1.84E-20)
inline bool sphIsDigital ( char c )
{
	return ( c>='0' && c<='9' ) || c=='.' || c=='E' || c=='e' || c=='+' || c=='-';
}

/// pointer to the last number in the buf, touching it's end
inline const char * sphFindLastNumeric ( const char * pBuf, int iLen )
{
	for ( auto pLast = pBuf + iLen - 1; pLast>=pBuf; --pLast )
		if ( !sphIsDigital ( *pLast ) )
			return pLast + 1;

	return pBuf;
}

/// my own isspace
inline bool sphIsSpace ( int iCode )
{
	return iCode==' ' || iCode=='\t' || iCode=='\n' || iCode=='\r';
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
	/// my own converter unsigned to string. Instanciated for DWORD and uint64_t
	template <typename Int> // signed
	int ItoA ( char * pOutput, Int nVal, int iBase = 10, int iWidth = 0, int iPrec = 0, char cFill = ' ' );
	template < typename UInt > // unsigned
	int UItoA ( char * pOutput, UInt nVal, int iBase = 10, int iWidth = 0, int iPrec = 0, char cFill = ' ' );
	template < typename Num > // let compiler deduce whether signed or unsigned...
	int NtoA ( char * pOutput, Num nVal, int iBase = 10, int iWidth = 0, int iPrec = 0, char cFill = ' ' );

	/// my own fixed-point floats. iPrec - num of digits after point. i.e. 100000, 3 -> 100.000
	int IFtoA ( char * pOutput, int nVal, int iPrec = 3 );
	int IFtoA ( char * pOutput, int64_t nVal, int iPrec = 6 );
	int vSprintf ( char * pOutput, const char * sFmt, va_list ap );
	int Sprintf ( char * pOutput, const char * sFmt, ... );
	void vSprintf ( StringBuilder_c &pOutput, const char * sFmt, va_list ap );
	void Sprintf ( StringBuilder_c& pOutput, const char * sFmt, ...);
}

/// string splitter, extracts sequences of alphas (as in sphIsAlpha)
void sphSplit ( StrVec_t & dOut, const char * sIn );

/// string splitter, splits by the given boundaries
void sphSplit ( StrVec_t & dOut, const char * sIn, const char * sBounds );

/// perform sphSplit, but applies a functor instead of add a chunk to the vector
using StrFunctor = std::function<void ( const char*, int )>;
void sphSplitApply ( const char * sIn, int iSize, StrFunctor &&dFunc );

/// string wildcard matching (case-sensitive, supports * and ? patterns)
bool sphWildcardMatch ( const char * sSstring, const char * sPattern, const int * pPattern = NULL );

/// parse size from text (int, or K/M/G/T suffix), or return provided default value.
/// *ppErr, if provided, will point to parsing error, if any
int64_t sphGetSize64 ( const char * sValue, char ** ppErr = nullptr, int64_t iDefault=-1 );
//////////////////////////////////////////////////////////////////////////

/// config section (hash of variant values)
class CSphConfigSection : public SmallStringHash_T < CSphVariant >
{
public:
	CSphConfigSection ()
		: m_iTag ( 0 )
	{}

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
	const char * GetStr ( const char * sKey, const char * sDefault="" ) const
	{
		CSphVariant * pEntry = (*this)( sKey );
		return pEntry ? pEntry->strval().cstr() : sDefault;
	}

	/// get size option (plain int, or with K/M prefix) value by key and default value
	int		GetSize ( const char * sKey, int iDefault ) const;
	int64_t GetSize64 ( const char * sKey, int64_t iDefault ) const;

	int m_iTag;
};

/// config section type (hash of sections)
using CSphConfigType = SmallStringHash_T < CSphConfigSection >;

/// config (hash of section types)
using CSphConfig = SmallStringHash_T < CSphConfigType >;

/// simple config file
class CSphConfigParser
{
public:
	CSphConfig		m_tConf;

public:
	bool			Parse ( const char * sFileName, const char * pBuffer = NULL );

	// fail-save loading new config over existing.
	bool			ReParse ( const char * sFileName, const char * pBuffer = NULL );

protected:
	CSphString		m_sFileName;
	int				m_iLine = -1;
	CSphString		m_sSectionType;
	CSphString		m_sSectionName;
	char			m_sError [ 1024 ];

	int					m_iWarnings = 0;
	static const int	WARNS_THRESH	= 5;

protected:
	bool			IsPlainSection ( const char * sKey );
	bool			IsNamedSection ( const char * sKey );
	bool			AddSection ( const char * sType, const char * sSection );
	void			AddKey ( const char * sKey, char * sValue );
	bool			ValidateKey ( const char * sKey );
	char *			GetBufferString ( char * szDest, int iMax, const char * & szSource );
};

bool TryToExec ( char * pBuffer, const char * szFilename, CSphVector<char> & dResult, char * sError, int iErrorLen, const char * sArgs=nullptr );

/////////////////////////////////////////////////////////////////////////////

/// load config file
const char *	sphLoadConfig ( const char * sOptConfig, bool bQuiet, CSphConfigParser & cp );

/// configure tokenizer from index definition section
void			sphConfTokenizer ( const CSphConfigSection & hIndex, CSphTokenizerSettings & tSettings );

/// configure dictionary from index definition section
void			sphConfDictionary ( const CSphConfigSection & hIndex, CSphDictSettings & tSettings );

/// configure field filter from index definition section
bool			sphConfFieldFilter ( const CSphConfigSection & hIndex, CSphFieldFilterSettings & tSettings, CSphString & sError );

/// configure index from index definition section
bool			sphConfIndex ( const CSphConfigSection & hIndex, CSphIndexSettings & tSettings, CSphString & sError );

/// try to set dictionary, tokenizer and misc settings for an index (if not already set)
bool			sphFixupIndexSettings ( CSphIndex * pIndex, const CSphConfigSection & hIndex, CSphString & sError, bool bStripFile=false );

bool			sphInitCharsetAliasTable ( CSphString & sError );

const char * sphBigramName ( ESphBigram eType );

enum ESphLogLevel
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
void sphSetLogger ( SphLogger_fn fnLog );

void sphLogVa ( const char * sFmt, va_list ap, ESphLogLevel eLevel = SPH_LOG_WARNING );
void sphWarning ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphInfo ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogFatal ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebug ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebugv ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebugvv ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebugRpl ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT

// set the prefix to supress the log
void sphLogSupress ( const char * sPrefix, ESphLogLevel eLevel = SPH_LOG_WARNING );
void sphLogSupressRemove ( const char * sPrefix, ESphLogLevel eLevel = SPH_LOG_WARNING );

//////////////////////////////////////////////////////////////////////////

/// how do we properly exit from the crash handler?
#if !USE_WINDOWS
	#define CRASH_EXIT_CORE { signal ( sig, SIG_DFL ); kill ( getpid(), sig ); }
	#ifndef NDEBUG
		// UNIX debug build, die and dump core
		#define CRASH_EXIT CRASH_EXIT_CORE
	#else
		// UNIX release build, just die
		#define CRASH_EXIT { exit ( 2 ); }
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

/// simple write wrapper
/// simplifies partial write checks, and also supresses "fortified" glibc warnings
bool sphWrite ( int iFD, const void * pBuf, size_t iSize );

/// async safe, BUT NOT THREAD SAFE, fprintf
void sphSafeInfo ( int iFD, const char * sFmt, ... );

#if !USE_WINDOWS
/// UNIX backtrace gets printed out to a stream
void sphBacktrace ( int iFD, bool bSafe=false );
#else
/// Windows minidump gets saved to a file
void sphBacktrace ( EXCEPTION_POINTERS * pExc, const char * sFile );
#endif

/// dummy call of backtrace to alloc internal structures and prevent deadlock at malloc on crash
void sphBacktraceInit();

void sphBacktraceSetBinaryName ( const char * sName );

/// plain backtrace - returns static buffer with the text of the call stack
const char * DoBacktrace ( int iDepth=0, int iSkip=0 );

void sphCheckDuplicatePaths ( const CSphConfig & hConf );

/// set globals from the common config section
void sphConfigureCommon ( const CSphConfig & hConf );

/// my own is chinese
bool sphIsChineseCode ( int iCode );

/// detect chinese chars in a buffer
bool sphDetectChinese ( const BYTE * szBuffer, int iLength );

/// returns ranker name as string
const char * sphGetRankerName ( ESphRankMode eRanker );

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
	bool Err ( const char * sFmt, ... );
	bool Err ( const CSphString &sMsg );
	StringBuilder_c& Err() { return m_sErrors; }
	void Warn ( const char * sFmt, ... );
	void Warn ( const CSphString &sMsg );
	StringBuilder_c& Warn() { return m_sWarnings; }

	void Clear ();

	const char * sError () const;
	const char * sWarning () const;

	bool ErrEmpty () const { return m_sErrors.IsEmpty (); }
	bool WarnEmpty () const { return m_sWarnings.IsEmpty (); };

	void AddStringsFrom ( const Warner_c &sSrc );
	void MoveErrorsTo ( CSphString &sTarget );
	void MoveWarningsTo ( CSphString &sTarget );
	void MoveAllTo ( CSphString &sTarget );
};


#endif // _sphinxutils_
