//
// Copyright (c) 2017-2021, Manticore Software LTD (https://manticoresearch.com)
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
	{ SPH_EXT_SPL,	".spl",		1,	true,	false,	"file lock for the index" },
	{ SPH_EXT_SETTINGS,	".settings", 1,	true,	false,	"index runtime settings" }
};


const char* sphGetExt ( ESphExt eExt )
{
	if ( eExt<SPH_EXT_SPH || eExt>=SPH_EXT_TOTAL )
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
			sFatalMsg.SetSprintf ( "%s index '%s': %s", sMsg, m_sIndexName.cstr(), ErrorMsg() );
	} else
	{
		if ( m_sIndexName.IsEmpty () )
			sFatalMsg.SetSprintf ( "%s", ErrorMsg () );
		else
			sFatalMsg.SetSprintf ( "index '%s': %s", m_sIndexName.cstr (), ErrorMsg () );
	}
	return sFatalMsg;
}

CSphString IndexFiles_c::FullPath ( const char * sExt, const char* sSuffix, const char * sBase )
{
	CSphString sResult;
	sResult.SetSprintf ( "%s%s%s", sBase ? sBase : m_sFilename.cstr (), sSuffix, sExt );
	return sResult;
}

CSphString IndexFiles_c::MakePath ( const char * sSuffix, const char * sBase )
{
	CSphString sResult;
	sResult.SetSprintf ( "%s%s", sBase ? sBase : m_sFilename.cstr (), sSuffix );
	return sResult;
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

void IndexFiles_c::Unlink ( const char * sType )
{
	for ( const auto &dExt : g_dIndexFilesExts )
	{
		auto sFile = FullPath ( dExt.m_szExt, sType );
		if ( ::unlink ( sFile.cstr() ) && !dExt.m_bOptional )
			sphWarning ( "unlink failed (file '%s', error '%s'", sFile.cstr (), strerrorm ( errno ) );
	}
}

bool IndexFiles_c::Rename ( const char * sFromSz, const char * sToSz )  // move files between different bases
{
	m_bFatal = false;
	bool bRenamed[SPH_EXT_TOTAL] = { false };
	bool bAllOk = true;
	for ( int i = 0; i<SPH_EXT_TOTAL; i++ )
	{
		const auto & dExt = g_dIndexFilesExts[i];
		if ( m_uVersion<dExt.m_uMinVer || !dExt.m_bCopy )
			continue;

		auto sFrom = FullPath ( dExt.m_szExt, "", sFromSz );
		auto sTo = FullPath ( dExt.m_szExt, "", sToSz );

#if _WIN32
		::unlink ( sTo.cstr() );
		sphLogDebug ( "%s unlinked", sTo.cstr() );
#endif

		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			// this is no reason to fail if not necessary files missed.
			if ( dExt.m_bOptional )
				continue;

			m_sLastError.SetSprintf ( "rename %s to %s failed: %s", sFrom.cstr (), sTo.cstr (), strerrorm ( errno ) );
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
		auto sFrom = FullPath ( dExt.m_szExt, "", sToSz );
		auto sTo = FullPath ( dExt.m_szExt, "", sFromSz );
		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			sphLogDebug ( "rollback failure when renaming %s to %s", sFrom.cstr (), sTo.cstr () );
			m_bFatal = true;
		}
	}

	return false;
}

bool IndexFiles_c::RenameLock ( const char * sToSz, int &iLockFD )
{
	if ( iLockFD<0 ) // no lock, no renaming need
		return true;

	m_bFatal = false;
	auto sFrom = FullPath ( sphGetExt(SPH_EXT_SPL) );
	auto sTo = FullPath ( sphGetExt(SPH_EXT_SPL), "", sToSz );

#if !_WIN32
	if ( !sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		return true;

	m_sLastError.SetSprintf ("failed to rename lock %s to %s, fd=%d, error %s (%d); ", sFrom.cstr(),
				   sTo.cstr(), iLockFD, strerrorm ( errno ), errno );

	// that is renaming of only 1 file failed; no need to rollback.
	m_bFatal = true;
	return false;

#else
	// on Windows - no direct rename. Lock new instead, release previous.
	int iNewLock=-1;
	if ( !RawFileLock ( sTo, iNewLock, m_sLastError ) )
		return false;
	auto iOldLock = iLockFD;
	iLockFD = iNewLock;
	RawFileUnLock ( sFrom, iOldLock );
	return true;
#endif
}

// move from backup to path using full (long) paths; fail is fatal
bool IndexFiles_c::Rollback ( const char * sBackup, const char * sPath )
{
	m_bFatal = false;
	for ( const auto &dExt : g_dIndexFilesExts )
	{
		auto sFrom = FullPath ( dExt.m_szExt, "", sBackup );
		auto sTo = FullPath ( dExt.m_szExt, "", sPath );

		if ( !sphIsReadable ( sFrom ) )
		{
			::unlink ( sTo.cstr () );
			continue;
		}
#if _WIN32
		::unlink ( sTo.cstr() );
		sphLogDebug ( "%s unlinked", sTo.cstr() );
#endif
		if ( sph::rename ( sFrom.cstr (), sTo.cstr () ) )
		{
			sphLogDebug ( "rollback rename %s to %s failed: %s", sFrom.cstr (), sTo.cstr (), strerrorm (	errno ) );
			m_bFatal = true;
			return false;
		}
	}
	return true;
}

// move everything except not intended for copying.
bool IndexFiles_c::RenameSuffix ( const char * sFromSuffix, const char * sToSuffix )
{
	return Rename ( FullPath ( "", sFromSuffix ).cstr (), FullPath ( "", sToSuffix ).cstr () );
}

bool IndexFiles_c::RenameBase ( const char * sToBase )  // move files to different base
{
	return Rename ( FullPath ( "" ).cstr (), sToBase );
}

bool IndexFiles_c::RelocateToNew ( const char * sNewBase )
{
	return Rollback( FullPath ( "", "", sNewBase ).cstr(), FullPath ( "", ".new" ).cstr() );
}

bool IndexFiles_c::RollbackSuff ( const char * sBackupSuffix, const char * sActiveSuffix )
{
	return Rollback ( FullPath ( "", sBackupSuffix ).cstr (), FullPath ( "", sActiveSuffix ).cstr () );
}

bool IndexFiles_c::CheckHeader ( const char * sType )
{
	auto sPath = FullPath ( sphGetExt(SPH_EXT_SPH), sType );
	BYTE dBuffer[8];

	CSphAutoreader rdHeader ( dBuffer, sizeof ( dBuffer ) );
	if ( !rdHeader.Open ( sPath, m_sLastError ) )
		return false;

	// check magic header
	auto uMagic = rdHeader.GetDword();
	if ( dBuffer[0] == '{' ) // that is new style json header, no need to check further...
		return true;

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