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

#include "indexfiles.h"

#include "sphinxstd.h"
#include "fileio.h"
#include "fileutils.h"
#include "sphinxint.h"
#include "sphinxjson.h"
#include "tokenizer/tokenizer.h"

static IndexFileExt_t g_dIndexFilesExts[SPH_EXT_TOTAL] =
{
	{ SPH_EXT_SPH,	".sph",		1,	false,	true,	"header file" },
	{ SPH_EXT_SPA,	".spa",		1,	true,	true,	"attribute values" },
	{ SPH_EXT_SPB,	".spb",		50,	true,	true,	"var-length attrs: strings, jsons, mva" },
	{ SPH_EXT_SPC,	".spc",		61,	true,	true,	"columnar storage" },
	{ SPH_EXT_SPI,	".spi",		1,	false,	true,	"dictionary (aka wordlist)" },
	{ SPH_EXT_SPD,	".spd",		1,	false,	true,	"document lists (aka doclists)"},
	{ SPH_EXT_SPP,	".spp",		3,	false,	true,	"keyword positions lists (aka hitlists)" },
	{ SPH_EXT_SPK,	".spk",		10, true,	true,	"kill list (aka klist)" },
	{ SPH_EXT_SPE,	".spe",		31,	false,	true,	"skip-lists to speed up doc-list filtering" },
	{ SPH_EXT_SPM,	".spm",		4,	false,	true,	"dead row map" },
	{ SPH_EXT_SPT,	".spt",		53,	true,	true,	"docid lookup table" },
	{ SPH_EXT_SPHI,	".sphi",	53,	true,	true,	"secondary index histograms" },
	{ SPH_EXT_SPDS, ".spds",	57, true,	true,	"document storage" },
	{ SPH_EXT_SPL,	".spl",		1,	true,	false,	"file lock for the table" },
	{ SPH_EXT_SETTINGS,	".settings", 1,	true,	false,	"table runtime settings" },
	{ SPH_EXT_SPIDX,	".spidx",	62,	true,	true,	"secondary index" },
	{ SPH_EXT_SPJIDX,	".spjidx",	66,	true,	true,	"secondary index for json attributes" },
	{ SPH_EXT_SPKNN,	".spknn",	65,	true,	true,	"knn index" }
};


const char* sphGetExt ( ESphExt eExt )
{
	if ( eExt>=SPH_EXT_TOTAL )
		return "";

	return g_dIndexFilesExts[eExt].m_szExt;
}


CSphVector<IndexFileExt_t> sphGetExts()
{
	// we may add support for older index versions in the future
	CSphVector<IndexFileExt_t> dResult;
	for (const auto & dIndexFilesExt : g_dIndexFilesExts)
		dResult.Add ( dIndexFilesExt );

	return dResult;
}

//////////////////////////////////////////////////////////////////////////
CSphString IndexFiles_c::FatalMsg ( const char * sMsg )
{
	CSphString sFatalMsg;
	if ( sMsg )
	{
		if ( m_sIndexName.IsEmpty () )
			sFatalMsg.SetSprintf ( "%s: %s", sMsg, ErrorMsg () );
		else
			sFatalMsg.SetSprintf ( "%s table '%s': %s", sMsg, m_sIndexName.cstr(), ErrorMsg() );
	} else
	{
		if ( m_sIndexName.IsEmpty () )
			sFatalMsg.SetSprintf ( "%s", ErrorMsg () );
		else
			sFatalMsg.SetSprintf ( "table '%s': %s", m_sIndexName.cstr (), ErrorMsg () );
	}
	return sFatalMsg;
}

CSphString IndexFiles_c::FullPath ( const char * szExt, const CSphString& sSuffix, const CSphString& sBase )
{
	StringBuilder_c sResult;
	sResult << (sBase.IsEmpty() ? GetFilebase() : sBase) << sSuffix << szExt;
	return (CSphString)sResult;
}

CSphString IndexFiles_c::MakePath ( const char* szSuffix, const CSphString& sBase )
{
	StringBuilder_c sResult;
	sResult << sBase << szSuffix;
	return (CSphString)sResult;
}

CSphString IndexFiles_c::MakePath ( const char * szSuffix )
{
	return MakePath ( szSuffix, GetFilebase() );
}

bool IndexFiles_c::HasAllFiles ( const char * sType )
{
	for ( const auto & dExt : g_dIndexFilesExts )
	{
		if ( m_uVersion<dExt.m_uMinVer || dExt.m_bOptional )
			continue;

		if ( !sphIsReadable ( FullPath ( dExt.m_szExt, sType ) ) )
			return false;
	}
	return true;
}

void IndexFiles_c::Unlink ( const char * szType )
{
	for ( const auto &dExt : g_dIndexFilesExts )
	{
		auto sFile = FullPath ( dExt.m_szExt, szType );
		if ( ::unlink ( sFile.cstr() ) && !dExt.m_bOptional )
			sphWarning ( "unlink failed (file '%s', error '%s'", sFile.cstr (), strerrorm ( errno ) );
	}
}

void IndexFiles_c::UnlinkExisted()
{
	for ( const auto & tExt : g_dIndexFilesExts )
	{
		auto sFile = FullPath ( tExt.m_szExt );
		if ( sphIsReadable ( sFile.cstr() ) && ::unlink ( sFile.cstr() ) && !tExt.m_bOptional )
			sphWarning ( "unlink failed (file '%s', error '%s'", sFile.cstr (), strerrorm ( errno ) );
	}
}

bool IndexFiles_c::TryRename ( const CSphString& sFrom, const CSphString& sTo )  // move files between different bases
{
	m_bFatal = false;
	bool bRenamed[SPH_EXT_TOTAL] = { false };
	bool bAllOk = true;
	for ( int i = 0; i<SPH_EXT_TOTAL; i++ )
	{
		const auto & dExt = g_dIndexFilesExts[i];
		if ( m_uVersion<dExt.m_uMinVer || !dExt.m_bCopy )
			continue;

		auto sFullFrom = FullPath ( dExt.m_szExt, "", sFrom );
		auto sFullTo = FullPath ( dExt.m_szExt, "", sTo );

#if _WIN32
		::unlink ( sFullTo.cstr() );
		sphLogDebug ( "%s unlinked", sTo.cstr() );
#endif

		if ( sph::rename ( sFullFrom.cstr (), sFullTo.cstr () ) )
		{
			// this is no reason to fail if not necessary files missed.
			if ( dExt.m_bOptional )
				continue;

			m_sLastError.SetSprintf ( "rename %s to %s failed: %s", sFullFrom.cstr (), sFullTo.cstr (), strerrorm ( errno ) );
			bAllOk = false;
			break;
		}
		bRenamed[i] = true;
	}

	if ( bAllOk )
		return true;

	for ( int i = 0; i<SPH_EXT_TOTAL; ++i )
	{
		if ( !bRenamed[i] )
			continue;

		const auto & dExt = g_dIndexFilesExts[i];
		auto sFullFrom = FullPath ( dExt.m_szExt, "", sTo );
		auto sFullTo = FullPath ( dExt.m_szExt, "", sFrom );
		if ( sph::rename ( sFullFrom.cstr (), sFullTo.cstr () ) )
		{
			sphLogDebug ( "rollback failure when renaming %s to %s", sFullFrom.cstr (), sFullTo.cstr () );
			m_bFatal = true;
		}
	}

	return false;
}

bool IndexFiles_c::RenameLock ( const CSphString& sTo, int &iLockFD )
{
	if ( iLockFD<0 ) // no lock, no renaming need
		return true;

	m_bFatal = false;
	auto sFullFrom = FullPath ( sphGetExt(SPH_EXT_SPL) );
	auto sFullTo = FullPath ( sphGetExt(SPH_EXT_SPL), "", sTo );

#if !_WIN32
	if ( !sph::rename ( sFullFrom.cstr (), sFullTo.cstr () ) )
		return true;

	m_sLastError.SetSprintf ("failed to rename lock %s to %s, fd=%d, error %s (%d); ", sFullFrom.cstr(), sFullTo.cstr(), iLockFD, strerrorm ( errno ), errno );

	// that is renaming of only 1 file failed; no need to rollback.
	m_bFatal = true;
	return false;

#else
	// on Windows - no direct rename. Lock new instead, release previous.
	int iNewLock=-1;
	if ( !RawFileLock ( sFullTo, iNewLock, m_sLastError ) )
		return false;
	auto iOldLock = iLockFD;
	iLockFD = iNewLock;
	RawFileUnLock ( sFullFrom, iOldLock );
	return true;
#endif
}

// move from backup to path using full (long) paths; fail is fatal
bool IndexFiles_c::Rename ( const CSphString& sFrom, const CSphString& sTo )
{
	for ( const auto &dExt : g_dIndexFilesExts )
	{
		auto sFullFrom = FullPath ( dExt.m_szExt, "", sFrom );
		auto sFullTo = FullPath ( dExt.m_szExt, "", sTo );

		if ( !sphIsReadable ( sFullFrom ) )
		{
			::unlink ( sFullTo.cstr () );
			continue;
		}
#if _WIN32
		::unlink ( sFullTo.cstr() );
		sphLogDebug ( "%s unlinked", sFullTo.cstr() );
#endif
		if ( sph::rename ( sFullFrom.cstr (), sFullTo.cstr () ) )
		{
			sphLogDebug ( "rename %s to %s failed: %s", sFullFrom.cstr (), sFullTo.cstr (), strerrorm ( errno ) );
			return false;
		}
	}
	return true;
}

// move everything except not intended for copying.
bool IndexFiles_c::TryRenameSuffix ( const CSphString& sFromSuffix, const CSphString& sToSuffix )
{
	return TryRename ( FullPath ( "", sFromSuffix ), FullPath ( "", sToSuffix ) );
}

bool IndexFiles_c::TryRenameBase ( const CSphString& sToBase )  // move files to different base
{
	return TryRename ( FullPath ( "" ), sToBase );
}

bool IndexFiles_c::RelocateToNew ( const CSphString& sNewBase )
{
	return Rename ( FullPath ( "", "", sNewBase ), FullPath ( "", ".new" ) );
}

bool IndexFiles_c::RenameSuffix ( const CSphString& sFrom, const CSphString& sTo )
{
	return Rename ( FullPath ( "", sFrom ), FullPath ( "", sTo ) );
}

bool IndexFiles_c::CheckHeader ( const char * sType )
{
	auto sPath = FullPath ( sphGetExt(SPH_EXT_SPH), sType );
	BYTE dBuffer[8];

	CSphAutoreader rdHeader ( dBuffer, sizeof ( dBuffer ) );
	if ( !rdHeader.Open ( sPath, m_sLastError ) )
		return false;

	// Check the legacy binary magic first. JSON headers do not have that magic;
	// they are identified by their opening brace below.
	auto uMagic = rdHeader.GetDword();
	if ( dBuffer[0] == '{' ) // new style JSON header
	{
		// Do not merely report that a JSON header exists. CheckHeader() also owns
		// the invariant that GetVersion() returns the version read from disk.
		// In particular, indextool --apply-killlists passes that value to the
		// lookup reader. If m_uVersion is left at INDEX_FORMAT_VERSION, an older
		// lookup can be decoded with the current layout. Version 71 added an
		// SphOffset_t to the .spt preamble, so making that mistake shifts every
		// checkpoint and eventually produces invalid row IDs and memory offsets.
		CSphVector<BYTE> dData;
		if ( !sphJsonParse ( dData, sPath, m_sLastError ) )
			return false;

		bson::Bson_c tBson ( dData );
		if ( tBson.IsEmpty() || !tBson.IsAssoc() )
		{
			m_sLastError.SetSprintf ( "invalid JSON index header %s", sPath.cstr() );
			return false;
		}

		// Keep the field name and layout-version rules in sync with the full header
		// readers in sphinx.cpp and indexcheck.cpp. A missing, invalid, or future
		// version must fail instead of falling back to the running binary's version.
		// This is only the broad format check; callers may impose a newer minimum.
		m_uVersion = (DWORD)bson::Int ( tBson.ChildByName ( "index_format_version" ) );
		if ( m_uVersion<=1 || m_uVersion>INDEX_FORMAT_VERSION )
		{
			m_sLastError.SetSprintf ( "%s is v.%u, binary is v.%u", sPath.cstr(), m_uVersion, INDEX_FORMAT_VERSION );
			return false;
		}

		// JSON detection currently assumes that '{' is the first byte. If headers
		// ever permit a BOM or leading whitespace, update this probe together with
		// the initial read above; otherwise a valid JSON header will be handled as
		// a legacy binary header.
		return true;
	}

	const char* sMsg = CheckFmtMagic ( uMagic );
	if ( sMsg )
	{
		m_sLastError.SetSprintf ( sMsg, sPath.cstr() );
		return false;
	}

	// get version
	DWORD uVersion = rdHeader.GetDword ();
	if ( uVersion==0 || uVersion>INDEX_FORMAT_VERSION )
	{
		m_sLastError.SetSprintf ( "%s is v.%u, binary is v.%u", sPath.cstr(), uVersion, INDEX_FORMAT_VERSION );
		return false;
	}
	m_uVersion = uVersion;
	return true;
}


bool IndexFiles_c::ReadKlistTargets ( StrVec_t & dTargets, const char * szType )
{
	CSphString sPath = FullPath ( sphGetExt(SPH_EXT_SPK), szType );
	if ( !sphIsReadable(sPath) )
		return true;

	CSphString sError;
	CSphAutoreader tReader;
	if ( !tReader.Open ( sPath, sError ) )
		return false;

	DWORD nIndexes = tReader.GetDword();
	dTargets.Resize ( nIndexes );
	for ( auto & i : dTargets )
	{
		i = tReader.GetString();
		tReader.GetDword();	// skip flags
	}

	return true;
}

// remove all old files these are not in the list of current index files
void RemoveOutdatedFiles ( const StrVec_t & dNewFiles, StrVec_t & dOldFiles )
{
	dOldFiles.Uniq();
	sph::StringSet hNewFiles ( dNewFiles );

	for ( const CSphString & tOldName : dOldFiles )
	{
		if ( hNewFiles[tOldName] )
			continue;

		if ( sphIsReadable ( tOldName ) )
			::unlink ( tOldName.cstr() );
	}
}
