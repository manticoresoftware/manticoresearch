//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
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

//////////////////////////////////////////////////////////////////////////

/// my own isalpha (let's build our own theme park!)
inline int sphIsAlpha ( int c )
{
	return ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='-' || c=='_';
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


/// string splitter, extracts sequences of alphas (as in sphIsAlpha)
void sphSplit ( CSphVector<CSphString> & dOut, const char * sIn );

/// string wildcard matching (case-sensitive, supports * and ? patterns)
bool sphWildcardMatch ( const char * sSstring, const char * sPattern );

//////////////////////////////////////////////////////////////////////////

/// config section (hash of variant values)
class CSphConfigSection : public SmallStringHash_T < CSphVariant >
{
public:
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
		return pEntry ? pEntry->cstr() : sDefault;
	}

	/// get size option (plain int, or with K/M prefix) value by key and default value
	int		GetSize ( const char * sKey, int iDefault ) const;
	int64_t GetSize64 ( const char * sKey, int64_t iDefault ) const;
};

/// config section type (hash of sections)
typedef SmallStringHash_T < CSphConfigSection >	CSphConfigType;

/// config (hash of section types)
typedef SmallStringHash_T < CSphConfigType >	CSphConfig;

/// simple config file
class CSphConfigParser
{
public:
	CSphConfig		m_tConf;

public:
					CSphConfigParser ();
	bool			Parse ( const char * sFileName, const char * pBuffer = NULL );

	// fail-save loading new config over existing.
	bool			ReParse ( const char * sFileName, const char * pBuffer = NULL );

protected:
	CSphString		m_sFileName;
	int				m_iLine;
	CSphString		m_sSectionType;
	CSphString		m_sSectionName;
	char			m_sError [ 1024 ];

	int					m_iWarnings;
	static const int	WARNS_THRESH	= 5;

protected:
	bool			IsPlainSection ( const char * sKey );
	bool			IsNamedSection ( const char * sKey );
	bool			AddSection ( const char * sType, const char * sSection );
	void			AddKey ( const char * sKey, char * sValue );
	bool			ValidateKey ( const char * sKey );

#if !USE_WINDOWS
	bool			TryToExec ( char * pBuffer, char * pEnd, const char * szFilename, CSphVector<char> & dResult );
#endif
	char *			GetBufferString ( char * szDest, int iMax, const char * & szSource );
};

/////////////////////////////////////////////////////////////////////////////

enum
{
	TOKENIZER_SBCS		= 1,
	TOKENIZER_UTF8		= 2,
	TOKENIZER_NGRAM	= 3
};

/// load config file
const char *	sphLoadConfig ( const char * sOptConfig, bool bQuiet, CSphConfigParser & cp );

/// configure tokenizer from index definition section
bool			sphConfTokenizer ( const CSphConfigSection & hIndex, CSphTokenizerSettings & tSettings, CSphString & sError );

/// configure dictionary from index definition section
void			sphConfDictionary ( const CSphConfigSection & hIndex, CSphDictSettings & tSettings );

/// configure field filter from index definition section
bool			sphConfFieldFilter ( const CSphConfigSection & hIndex, CSphFieldFilterSettings & tSettings, CSphString & sError );

/// configure index from index definition section
bool			sphConfIndex ( const CSphConfigSection & hIndex, CSphIndexSettings & tSettings, CSphString & sError );

/// try to set dictionary, tokenizer and misc settings for an index (if not already set)
bool			sphFixupIndexSettings ( CSphIndex * pIndex, const CSphConfigSection & hIndex, CSphString & sError );

enum ESphLogLevel
{
	SPH_LOG_FATAL	= 0,
	SPH_LOG_WARNING	= 1,
	SPH_LOG_INFO	= 2,
	SPH_LOG_DEBUG	= 3,
	SPH_LOG_VERBOSE_DEBUG = 4,
	SPH_LOG_VERY_VERBOSE_DEBUG = 5
};

typedef void ( *SphLogger_fn )( ESphLogLevel, const char *, va_list );

void sphWarning ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphInfo ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogFatal ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebug ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebugv ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphLogDebugvv ( const char * sFmt, ... ) __attribute__((format(printf,1,2))); //NOLINT
void sphSetLogger ( SphLogger_fn fnLog );

//////////////////////////////////////////////////////////////////////////

/// how do we properly exit from the crash handler?
#if !USE_WINDOWS
	#ifndef NDEBUG
		// UNIX debug build, die and dump core
		#define CRASH_EXIT { signal ( sig, SIG_DFL ); kill ( getpid(), sig ); }
	#else
		// UNIX release build, just die
		#define CRASH_EXIT exit ( 2 )
	#endif
#else
	#ifndef NDEBUG
		// Windows debug build, show prompt to attach debugger
		#define CRASH_EXIT return EXCEPTION_CONTINUE_SEARCH
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

void sphBacktraceSetBinaryName ( const char * sName );

/// plain backtrace - returns static buffer with the text of the call stack
const char * DoBacktrace ( int iDepth=0, int iSkip=0 );

#endif // _sphinxutils_

//
// $Id$
//
