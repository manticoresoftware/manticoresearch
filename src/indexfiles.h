//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "sphinxstd.h"
#include "sphinxint.h"
#include "indexfilebase.h"

#include <utility>

enum ESphExt : BYTE
{
	SPH_EXT_SPH,
	SPH_EXT_SPA,
	SPH_EXT_SPB,
	SPH_EXT_SPC,
	SPH_EXT_SPI,
	SPH_EXT_SPD,
	SPH_EXT_SPP,
	SPH_EXT_SPK,
	SPH_EXT_SPE,
	SPH_EXT_SPM,
	SPH_EXT_SPT,
	SPH_EXT_SPHI,
	SPH_EXT_SPDS,
	SPH_EXT_SPL,
	SPH_EXT_SETTINGS,
	SPH_EXT_SPIDX,
	SPH_EXT_SPJIDX,
	SPH_EXT_SPKNN,

	SPH_EXT_TOTAL
};

struct IndexFileExt_t
{
	ESphExt			m_eExt;
	const char *	m_szExt;
	DWORD			m_uMinVer;
	bool			m_bOptional;
	bool			m_bCopy;	// file needs to be copied
	const char *	m_szDesc;
};


CSphVector<IndexFileExt_t>	sphGetExts();
const char*					sphGetExt ( ESphExt eExt );

/// encapsulates all common actions over index files in general (copy/rename/delete etc.)
class IndexFiles_c : public IndexFileBase_c
{
	DWORD		m_uVersion = INDEX_FORMAT_VERSION;
	CSphString	m_sIndexName;	// used for information purposes (logs)
	CSphString	m_sLastError;
	bool		m_bFatal = false; // if fatal fail happened (unable to rename during rollback)
	CSphString FullPath ( const char * szExt, const CSphString& sSuffix = "", const CSphString& sBase = "" );
	inline void SetName ( CSphString sIndex ) { m_sIndexName = std::move(sIndex); }

public:
	IndexFiles_c() = default;
	explicit IndexFiles_c ( CSphString sBase, const char* sIndex=nullptr, DWORD uVersion = INDEX_FORMAT_VERSION )
		: IndexFileBase_c { std::move ( sBase ) }
		, m_uVersion ( uVersion )
	{
		if ( sIndex )
			SetName ( sIndex );
	}

	inline const char * ErrorMsg () const { return m_sLastError.cstr(); }
	inline bool IsFatal() const { return m_bFatal; }

	// read .sph and adopt index version from there.
	bool CheckHeader ( const char * sType="" );

	// read the beginning of .spk and parse killlist targets
	bool ReadKlistTargets ( StrVec_t & dTargets, const char * sType="" );

	DWORD GetVersion() const { return m_uVersion; }

	// simple make decorated path, like '.old' -> /path/to/index.old
	CSphString MakePath ( const char * szSuffix = "" );
	static CSphString MakePath ( const char* szSuffix, const CSphString& sBase );

	bool TryRename ( const CSphString& sFrom, const CSphString& sTo );  // move with auto backup and rollback
	bool Rename ( const CSphString& sFrom, const CSphString& sTo ); // oneshot move without backups
	bool TryRenameSuffix ( const CSphString& sFromSuffix, const CSphString& sToSuffix );  // move files in base between two suffixes
	bool TryRenameBase ( const CSphString& sToBase );  // move files to different base
	bool RenameLock ( const CSphString& sTo, int & iLockFD ); // safely move lock file
	bool RelocateToNew ( const CSphString& sNewBase ); // relocate to new base and append '.new' suffix
	bool RenameSuffix ( const CSphString& sFrom, const CSphString& sTo ); // move from backup to active; fail is fatal
	bool HasAllFiles ( const char * szType = "" ); // check that all necessary files are readable
	void Unlink ( const char * szType = "" );
	void UnlinkExisted ();

	// if prev op fails with fatal error - log the message and terminate
	CSphString FatalMsg(const char * szMsg=nullptr);
};
