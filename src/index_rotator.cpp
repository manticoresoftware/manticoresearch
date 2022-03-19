// Copyright (c) 2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "index_rotator.h"
#include "searchdaemon.h"
#include "indexfiles.h"

RotateFrom_e CheckIndexHeaderRotate ( const CSphString& sServedPath )
{
	// check order:
	// current_path/idx.new.sph		- rotation of current index
	// current_path/idx.sph			- enable back current index

	if ( IndexFiles_c ( sServedPath ).CheckHeader ( ".new" ) )
		return RotateFrom_e::NEW;

	if ( IndexFiles_c ( sServedPath ).CheckHeader()	)
		return RotateFrom_e::REENABLE;

	return RotateFrom_e::NONE;
}

RotateFrom_e CheckIndexHeaderRotate ( const ServedDesc_t& tServed )
{
	return CheckIndexHeaderRotate ( tServed.m_sIndexPath );
}

class IndexRotator_c::Impl_c
{
	// since it is impl, everything is private and accessible by friendship
	friend class IndexRotator_c;

	CSphString m_sServedPath;
	const char* m_szIndex;
	RotateFrom_e m_eRotateFrom;

	CSphString m_sPathTo;
	CSphString m_sBase;

	// rotation stuff;
	CSphIndex* m_pOldIdx = nullptr;
	bool m_bHasOld = false;
	std::function<void(CSphString)> m_fnClean;


	Impl_c ( const CSphString& tServedPath, const char* szIndex )
		: m_sServedPath { tServedPath }
		, m_szIndex { szIndex }
		, m_eRotateFrom { CheckIndexHeaderRotate ( tServedPath ) }
	{}

	bool ConfigureIfNeed() noexcept
	{
		switch ( m_eRotateFrom )
		{
		case RotateFrom_e::NEW: // move from current_path/idx.new.sph -> current_path/idx.sph
			m_sPathTo = m_sServedPath;
			m_sBase = IndexFiles_c::MakePath ( ".new", m_sPathTo );
			return true;
		case RotateFrom_e::REENABLE: // load from current_path/idx.sph
			m_sBase = m_sServedPath;
			return true;
		case RotateFrom_e::NONE:
			return false;
		default:
			assert ( false );
			return false;
		}
	}

	CSphString GetNewBase() const noexcept
	{
		return m_sBase;
	}

	bool NeedMoveFiles() const noexcept
	{
		return !m_sPathTo.IsEmpty();
	}

	//////////////////
	/// stuff with moving index (in seamless)
	//////////////////

	void BackupIfNeed ( CSphIndex* pIdx, std::function<void ( CSphString )> fnClean )
	{
		assert ( NeedMoveFiles() );

		if ( !pIdx )
			return;

		CSphString sExistingBase = pIdx->GetFilename();
		if ( sExistingBase != m_sPathTo )
			return; // nothing to backup, paths are unrelated.

		CSphString sError;
		// backup to .old path
		switch ( pIdx->RenameEx ( IndexFiles_c::MakePath ( ".old", m_sPathTo ) ) )
		{
		case CSphIndex::RE_FATAL: // not just failed, but also rollback wasn't success. Source index is damaged!
			sError.SetSprintf ( "rotating index '%s': cur to old rename failed: %s; rollback also failed, INDEX UNUSABLE", m_szIndex, pIdx->GetLastError().cstr() );
			throw RotationError_c ( std::move ( sError ), true );
		case CSphIndex::RE_FAIL:
			sError.SetSprintf ( "rotating index '%s': cur to old rename failed: %s", m_szIndex, pIdx->GetLastError().cstr() );
			throw RotationError_c ( std::move ( sError ) );
		default:
			break;
		}

		m_pOldIdx = pIdx;
		m_fnClean = std::move ( fnClean );
	}

	void MoveIndex ( CSphIndex* pNewIndex )
	{
		assert ( pNewIndex );
		assert ( NeedMoveFiles() );
		if ( pNewIndex->Rename ( m_sPathTo ) )
			return;

		CSphString sError;
		sError.SetSprintf ( "rotating index '%s': new to cur rename failed: %s", m_szIndex, pNewIndex->GetLastError().cstr() );
		if ( m_pOldIdx && !m_pOldIdx->Rename ( m_sPathTo ) )
		{
			sError.SetSprintf ( "rotating index '%s': old to cur rename failed: %s; INDEX UNUSABLE", m_szIndex, m_pOldIdx->GetLastError().cstr() );
			m_fnClean = nullptr;
			throw RotationError_c ( std::move ( sError ), true );
		}
		throw RotationError_c ( std::move ( sError ) );

	}

	void BackupFilesIfNeed ()
	{
		assert ( NeedMoveFiles() );
		if ( m_sServedPath != m_sPathTo )
			return; // nothing to backup, paths are unrelated.

		IndexFiles_c dServedFiles ( m_sServedPath, m_szIndex );
		if ( !dServedFiles.HasAllFiles() )
			return;

		CSphString sError;

		// backup to .old path
		if ( dServedFiles.TryRenameSuffix ( "", ".old" ) )
		{
			sphLogDebug ( "Rotation: Current index renamed to .old" );
			m_bHasOld = true;
			return;
		}

		// backup failed.
		if ( dServedFiles.IsFatal() )
		{
			sError.SetSprintf ( "rotating index '%s': cur to old rename failed: %s; rollback also failed, INDEX UNUSABLE", m_szIndex, dServedFiles.FatalMsg ( "rotating" ).cstr() );
			throw RotationError_c ( std::move ( sError ), true );
		}

		sError.SetSprintf ( "rotating index '%s': cur to old rename failed: %s", m_szIndex, dServedFiles.ErrorMsg() );
		throw RotationError_c ( std::move ( sError ) );
	}

	void RollBackOld()
	{
		if ( !m_bHasOld )
			return;

		m_bHasOld = false;

		IndexFiles_c dOldServedFiles ( m_sServedPath, m_szIndex );
		if ( !dOldServedFiles.RenameSuffix ( ".old", "" ) )
		{
			CSphString sError;
			sError.SetSprintf ( "%s", dOldServedFiles.FatalMsg ( "rotating" ).cstr() );
			throw RotationError_c ( std::move ( sError ), true );
		}
	}

	void MoveFiles ()
	{
		assert ( NeedMoveFiles() );
		assert ( m_sBase != m_sPathTo );

		IndexFiles_c dNewFiles ( m_sBase, m_szIndex );
		if ( dNewFiles.TryRename ( m_sBase, m_sPathTo ) )
		{
			sphLogDebug ( "RotateIndexFilesGreedy: New renamed to current" );
			return;
		}

		// moving failed.
		CSphString sError;
		if ( dNewFiles.IsFatal() )
		{
			sError.SetSprintf ( "%s", dNewFiles.FatalMsg ( "rotating" ).cstr() );
			throw RotationError_c ( std::move ( sError ), true );
		}

		// rollback, if need
		RollBackOld();

		sError.SetSprintf ( "rotating index '%s': %s; using old index", m_szIndex, dNewFiles.ErrorMsg() );
		throw RotationError_c ( std::move ( sError ) );
	}

	bool RollbackMovingFiles()
	{
		if ( !NeedMoveFiles() )
			return false;

		CSphString sError;

		// try to recover: rollback cur to .new, .old to cur.
		IndexFiles_c dRollbackFiles ( m_sPathTo, m_szIndex );
		if ( !dRollbackFiles.Rename ( m_sPathTo, m_sBase ) )
		{
			sError.SetSprintf ( "%s", dRollbackFiles.FatalMsg ( "rotating" ).cstr() );
			throw RotationError_c ( std::move ( sError ), true );
		}
		RollBackOld();
		return true;
	}


	//////////////////
	/// stuff for both, moving idx/files
	//////////////////
	void CleanBackup() const noexcept
	{
		if ( m_fnClean )
			m_fnClean ( IndexFiles_c::MakePath ( ".old", m_sPathTo ) );

		if ( !m_bHasOld )
			return;

		IndexFiles_c dFiles ( m_sServedPath, m_szIndex );
		dFiles.Unlink ( ".old" );
		sphLogDebug ( "PreallocIndexGreedy: the old index unlinked" );
	}
};


/// public iface
IndexRotator_c::IndexRotator_c ( const CSphString& sServedPath, const char* szIndex )
	: m_pImpl { new Impl_c ( sServedPath, szIndex ) }
{
	assert ( m_pImpl );
}

IndexRotator_c::~IndexRotator_c ()
{
	SafeDelete ( m_pImpl );
}


bool IndexRotator_c::ConfigureIfNeed() noexcept
{
	return m_pImpl->ConfigureIfNeed();
}

CSphString IndexRotator_c::GetNewBase() const noexcept
{
	return m_pImpl->GetNewBase();
}

bool IndexRotator_c::NeedMoveFiles() const noexcept
{
	return m_pImpl->NeedMoveFiles();
}

void IndexRotator_c::BackupIfNeed ( CSphIndex* pIdx, std::function<void ( CSphString )> fnClean )
{
	m_pImpl->BackupIfNeed ( pIdx, std::move (fnClean) );
}

void IndexRotator_c::MoveIndex ( CSphIndex* pIdx )
{
	m_pImpl->MoveIndex ( pIdx );
}

void IndexRotator_c::BackupFilesIfNeed()
{
	m_pImpl->BackupFilesIfNeed();
}

void IndexRotator_c::MoveFiles ()
{
	m_pImpl->MoveFiles ();
}

bool IndexRotator_c::RollbackMovingFiles()
{
	return m_pImpl->RollbackMovingFiles();
}

void IndexRotator_c::CleanBackup () const noexcept
{
	m_pImpl->CleanBackup ();
}
