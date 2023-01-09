// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
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

CheckIndexRotate_c::CheckIndexRotate_c ( const ServedDesc_t& tServed )
{
	// check order:
	// current_path/idx.new.sph		- rotation of current index
	// current_path/idx.sph			- enable back current index

	if ( IndexFiles_c ( tServed.m_sIndexPath ).CheckHeader ( ".new" ) )
		m_eRotateFrom = RotateFrom_e::NEW;

	else if ( IndexFiles_c ( tServed.m_sIndexPath ).CheckHeader() )
		m_eRotateFrom = RotateFrom_e::REENABLE;

	else
		m_eRotateFrom = RotateFrom_e::NONE;
};

bool CheckIndexRotate_c::RotateFromNew() const noexcept
{
	return m_eRotateFrom == RotateFrom_e::NEW;
}

bool CheckIndexRotate_c::NothingToRotate() const noexcept
{
	return m_eRotateFrom == RotateFrom_e::NONE;
}

class StepActionEx_c : public StepAction_c
{
	CSphString m_sError;
	bool m_bFatal = false;

protected:
	inline bool ThrowError ( CSphString sError, bool bFatal=false )
	{
		m_sError = std::move ( sError );
		m_bFatal = bFatal;
		return false;
	}

public:
	virtual bool Rollback() { return true; };
	std::pair<CSphString,bool> GetError() const
	{
		return { m_sError, m_bFatal };
	}
};

//////////////////
/// stuff with moving index (in seamless)
//////////////////

class RenameIdx_c : public StepActionEx_c
{
	CSphIndex* m_pIdx = nullptr;
	CSphString m_sOrigPath;
	CSphString m_sTargetPath;
	const char* m_szTo;

private:
	bool DoRename ( const char* szExt, const CSphString& sTo )
	{
		switch ( m_pIdx->RenameEx ( sTo ) )
		{
		case CSphIndex::RE_FATAL: // not just failed, but also rollback wasn't success. Source index is damaged!
			return ThrowError ( SphSprintf ( "rename to '%s' failed: %s; rollback also failed, TABLE UNUSABLE", szExt, m_pIdx->GetLastError().cstr() ), true );
		case CSphIndex::RE_FAIL:
			return ThrowError ( SphSprintf ( "rename to '%s' failed: %s", szExt, m_pIdx->GetLastError().cstr() ) );
		default:
			return true;
		}
	}

public:
	void SetIdx ( CSphIndex* pIdx )
	{
		m_pIdx = pIdx;
		assert ( m_pIdx );
		m_sOrigPath = m_pIdx->GetFilebase();
		m_sTargetPath = IndexFiles_c::MakePath ( m_szTo, m_sOrigPath );
	}

	void SetTargetPath ( const CSphString& sTo )
	{
		m_sTargetPath = sTo;
	}

public:
	explicit RenameIdx_c ( const char* szTo, CSphIndex* pIdx=nullptr )
		: m_szTo ( szTo )
	{
		if ( pIdx )
			SetIdx ( pIdx );
	}

	bool Action() final
	{
		return DoRename ( m_szTo, m_sTargetPath );
	}

	bool Rollback() final
	{
		return DoRename ( "cur", m_sOrigPath );
	}
};

StepActionPtr_c RenameIdx ( CSphIndex* pIdx, const CSphString& sTo )
{
	auto pAction = std::make_unique<RenameIdx_c> ( ".new", pIdx );
	pAction->SetTargetPath ( sTo );
	return pAction;
}

class RenameServedIdx_c: public RenameIdx_c
{
	RWIdx_c m_pIdx;
public:
	RenameServedIdx_c ( const char* szTo, const cServedIndexRefPtr_c& pIdx )
		: RenameIdx_c ( szTo )
		, m_pIdx ( pIdx )
	{
		SetIdx ( m_pIdx );
	}
};

StepActionPtr_c RenameIdxSuffix ( const cServedIndexRefPtr_c& pIdx, const char* szToExt )
{
	sphInfo ( "RW-idx for rename to %s, acquiring...", szToExt);
	auto pResult = std::make_unique<RenameServedIdx_c> ( szToExt, pIdx );
	sphInfo ( "RW-idx for rename to %s, acquired...", szToExt );
	return pResult;
}

//////////////////
/// stuff with moving files (in greedy)
//////////////////

class RenameFiles_c: public StepActionEx_c
{
	IndexFiles_c& m_tFiles;
	const char* m_szFromExt;
	const char* m_szToExt;

private:
	bool DoRename ( const char* szFromExt, const char* szToExt )
	{
		if ( m_tFiles.TryRenameSuffix ( szFromExt, szToExt ) )
			return true;

		// backup failed.
		if ( m_tFiles.IsFatal() )
			return ThrowError ( SphSprintf ( "'%s' to '%s' rename failed: %s; rollback also failed, TABLE UNUSABLE", szFromExt, szToExt, m_tFiles.FatalMsg ( "rotating" ).cstr() ), true );
		return ThrowError ( SphSprintf ( "'%s' to '%s' rename failed: %s", szFromExt, szToExt, m_tFiles.ErrorMsg() ) );
	}

public:
	explicit RenameFiles_c ( const char* szFromExt, const char* szToExt, IndexFiles_c& tFiles )
		: m_tFiles { tFiles }
		, m_szFromExt { szFromExt }
		, m_szToExt { szToExt }
	{}

	bool Action() final
	{
		return DoRename ( m_szFromExt, m_szToExt );
	}

	bool Rollback() final
	{
		return DoRename ( m_szToExt, m_szFromExt );
	}
};

StepActionPtr_c RenameFiles ( IndexFiles_c& tFiles, const char* szFromExt, const char* szToExt )
{
	return std::make_unique<RenameFiles_c> ( szFromExt, szToExt, tFiles );
}

void ActionSequence_c::Defer ( StepActionPtr_c&& pAction )
{
	m_dActions.Add ( std::move ( pAction ) );
}

bool ActionSequence_c::UnRunDefers()
{
	for ( auto i=m_iRun-1; i>=0; --i )
	{
		auto pStep = (StepActionEx_c*)m_dActions[i].get();
		if ( !pStep->Rollback() )
		{
			m_tError = pStep->GetError();
			return false;
		}
	}
	return true;
}

bool ActionSequence_c::RunDefers ()
{
	m_iRun = 0;
	for ( int i = 0, iEnd = m_dActions.GetLength(); i < iEnd; ++i )
	{
		auto pStep = (StepActionEx_c*)m_dActions[i].get();
		if ( !pStep->Action() )
		{
			m_tError = pStep->GetError();
			if ( !UnRunDefers() )
			{
				auto tError = pStep->GetError();
				m_tError.second = m_tError.second || tError.second;
				m_tError.first = SphSprintf ( "%s; than %s", tError.first.cstr(), m_tError.first.cstr() );
			}
			return false;
		} else
			++m_iRun;
	}
	return true;
}

std::pair<CSphString, bool> ActionSequence_c::GetError() const
{
	return m_tError;
}
