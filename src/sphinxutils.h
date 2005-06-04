//
// $Id$
//

//
// Copyright (c) 2001-2005, Andrew Aksyonoff. All rights reserved.
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

/////////////////////////////////////////////////////////////////////////////

/// known keys for different config sections
extern const char * g_dSphKeysCommon[];
extern const char * g_dSphKeysIndexer[];
extern const char * g_dSphKeysSearchd[];
extern const char * g_dSphKeysSearch[];

/////////////////////////////////////////////////////////////////////////////

/// simple "hash"
class CSphHash
{
public:
					CSphHash ();
					~CSphHash ();

	void			Add ( const char * sKey, const char * sValue );
	const char *	Get ( const char * sKey );

private:
	int				m_iCount;
	char **			m_ppKeys;
	char **			m_ppValues;
	int				m_iMax;
};


/// simple config file
class CSphConfig
{
public:
					CSphConfig ();
					~CSphConfig ();
	int				Open ( const char * sFile );
	CSphHash *		LoadSection ( const char * sSection, const char ** dKnownKeys=NULL );

protected:
	FILE *			m_pFP;
	char *			m_sFileName;
	const char *	m_sSection;
	int				m_iLine;

protected:
	bool			ValidateKey ( const char * sKey, const char ** dKnownKeys );
};

#endif // _sphinxutils_

//
// $Id$
//
