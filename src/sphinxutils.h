//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
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

/////////////////////////////////////////////////////////////////////////////

/// let's build our own theme park!
inline int sphIsAlpha ( int c )
{
	return ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='-' || c=='_';
}

inline bool sphIsSpace ( int iCode )
{
	return iCode==' ' || iCode=='\t' || iCode=='\n' || iCode=='\r';
}

/////////////////////////////////////////////////////////////////////////////

/// string hash function
struct CSphStrHashFunc
{
	static inline int Hash ( const CSphString & sKey )
	{
		return sphCRC32 ( (const BYTE *)sKey.cstr() );
	}
};

/// small hash with string keys
template < typename T >
class SmallStringHash_T : public CSphOrderedHash < T, CSphString, CSphStrHashFunc, 256, 13 > {};

/////////////////////////////////////////////////////////////////////////////

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

	/// get string option value by key and default value
	const char * GetStr ( const char * sKey, const char * sDefault="" ) const
	{
		CSphVariant * pEntry = (*this)( sKey );
		return pEntry ? pEntry->cstr() : sDefault;
	}

	/// get size option (plain int, or with K/M prefix) value by key and default value
	int GetSize ( const char * sKey, int iDefault ) const;
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
	 TOKENIZER_SBCS		= 1
	,TOKENIZER_UTF8		= 2
	,TOKENIZER_NGRAM	= 3
};

/// configure tokenizer from index definition section
bool	sphConfTokenizer ( const CSphConfigSection & hIndex, CSphTokenizerSettings & tSettings, CSphString & sError );

/// configure dictionary from index definition section
void	sphConfDictionary ( const CSphConfigSection & hIndex, CSphDictSettings & tSettings );

/// configure index from index definition section
void	sphConfIndex ( const CSphConfigSection & hIndex, CSphIndexSettings & tSettings );

/// try to set dictionary, tokenizer and misc settings for an index (if not already set)
bool	sphFixupIndexSettings ( CSphIndex * pIndex, const CSphConfigSection & hIndex, CSphString & sError );

#endif // _sphinxutils_

//
// $Id$
//
