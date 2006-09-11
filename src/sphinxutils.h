//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
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

/// known keys for different config sections
extern const char * g_dSphKeysCommon[];
extern const char * g_dSphKeysIndexer[];
extern const char * g_dSphKeysSearchd[];
extern const char * g_dSphKeysSearch[];

/////////////////////////////////////////////////////////////////////////////

/// let's build our own theme park!
inline int sphIsAlpha ( int c )
{
	return isalpha(c) || isdigit(c) || c=='_';
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
class SmallStringHash_T : public CSphFixedOrderedHash < T, CSphString, CSphStrHashFunc, 128, 13 > {};

/////////////////////////////////////////////////////////////////////////////

/// config section (hash of variant values)
typedef SmallStringHash_T < CSphVariant >		CSphConfigSection;

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
	bool			Parse ( const char * sFile );

protected:
	CSphString		m_sFileName;
	int				m_iLine;
	CSphString		m_sSectionType;
	CSphString		m_sSectionName;
	char			m_sError [ 1024 ];

protected:
	bool			ValidateKey ( const char * sKey, const char ** dKnownKeys );

	bool			IsPlainSection ( const char * sKey );
	bool			IsNamedSection ( const char * sKey );
	bool			AddSection ( const char * sType, const char * sSection );
	bool			AddKey ( const char * sKey, char * sValue );
};

/////////////////////////////////////////////////////////////////////////////

/// parse morphology option string to bit mask
DWORD	sphParseMorphology ( const CSphString & sOption, bool bUseUTF8 );

#endif // _sphinxutils_

//
// $Id$
//
