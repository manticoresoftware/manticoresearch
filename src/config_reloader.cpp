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

#include "config_reloader.h"
#include "searchdaemon.h"
#include "searchdha.h"
#include "index_rotator.h"

static auto& g_bSeamlessRotate = sphGetSeamlessRotate();

// Reloading called always from same thread (so, for now not need to be th-safe for itself)
// ServiceMain() -> TickHead() -> CheckRotate() -> ReloadConfigAndRotateIndexes().
class ConfigReloader_c::Impl_c
{
	ReadOnlyServedHash_c::Snapshot_t m_hLocalSnapshot;
	ReadOnlyDistrHash_c::Snapshot_t m_hDistrSnapshot;
	WriteableServedHash_c m_hNewLocalIndexes { *g_pLocalIndexes };
	WriteableDistrHash_c m_hNewDistrIndexes { *g_pDistIndexes };
	HashOfServed_c& m_hDeferred;
	sph::StringSet m_hProcessed;
	sph::StringSet m_hLocals;

private:
	bool ExistsLocal ( const CSphString& sIndex ) const
	{
		assert ( m_hLocalSnapshot.first );
		return m_hLocalSnapshot.first->Exists ( sIndex );
	}

	bool ExistsDistr ( const CSphString& sIndex ) const
	{
		assert ( m_hDistrSnapshot.first );
		return m_hDistrSnapshot.first->Exists ( sIndex );
	}

	inline cServedIndexRefPtr_c GetSnapServed ( const CSphString& sName ) const
	{
		assert ( m_hLocalSnapshot.first );
		auto* pEntry = ( *m_hLocalSnapshot.first ) ( sName );
		assert ( pEntry );
		return *pEntry;
	}

	bool CopyExistingLocal ( const CSphString& sIndex )
	{
		assert ( m_hLocalSnapshot.first );
		auto* pExisting = ( *m_hLocalSnapshot.first ) ( sIndex );
		if ( !pExisting )
			return false;
		sphLogDebug ( "Keep existing before local table %s", sIndex.cstr() );
		return m_hNewLocalIndexes.Add ( *pExisting, sIndex );
	}

	bool CopyExistingDistr ( const CSphString& sIndex )
	{
		assert ( m_hDistrSnapshot.first );
		auto* pExisting = ( *m_hDistrSnapshot.first ) ( sIndex );
		if ( !pExisting )
			return false;
		sphLogDebug ( "Keep existing before distr table %s", sIndex.cstr() );
		return m_hNewDistrIndexes.Add ( *pExisting, sIndex );
	}

	void KeepExisting ( const CSphString& sIndex )
	{
		sphLogDebug ( "keep existing table  %s", sIndex.cstr() );
		CopyExistingLocal ( sIndex );
		m_hLocals.Add ( sIndex );
	}

	void AddDeferred ( const CSphString& sIndex, ServedIndexRefPtr_c&& pServed )
	{
		sphLogDebug ( "add deferred table %s", sIndex.cstr() );
		KeepExisting ( sIndex );
		CopyExistingDistr ( sIndex ); // keep for now; will be removed when deferred processed.
		m_hDeferred.Add ( pServed, sIndex );
	}

	void LoadDistrFromConfig ( const CSphString& sIndex, const CSphConfigSection& hIndex )
	{
		DistributedIndexRefPtr_t pNewDistr ( new DistributedIndex_t );
		ConfigureDistributedIndex ( [this] ( const auto& sIdx ) { return m_hLocals[sIdx]; }, *pNewDistr, sIndex.cstr(), hIndex );
		if ( pNewDistr->IsEmpty() )
		{
			if ( CopyExistingDistr ( sIndex ) || CopyExistingLocal ( sIndex ) )
				sphWarning ( "table '%s': no valid local/remote tables in distributed table; using last valid definition", sIndex.cstr() );
		} else
			m_hNewDistrIndexes.Add ( pNewDistr, sIndex );
	}

	// load fresh local index
	void LoadNewLocalFromConfig ( const CSphString& sIndex, const CSphConfigSection& hIndex )
	{
		CSphString sError;
		auto [ eAdd, pFreshLocal ] = AddIndex ( sIndex.cstr(), hIndex, false, false, nullptr, sError );
		assert ( eAdd != ADD_DISTR && "internal error: distr table should not be here!" );

		switch ( eAdd )
		{
		case ADD_ERROR:
			sphWarning ( "table '%s': failed to load with error %s", sIndex.cstr(), sError.cstr() );
			break;
		case ADD_NEEDLOAD:
			assert ( ServedDesc_t::IsLocal ( pFreshLocal ) ); // that is: PLAIN, RT, or PERCOLATE
			if ( pFreshLocal->m_eType == IndexType_e::PLAIN && !PreloadKlistTarget ( *pFreshLocal, CheckIndexRotate_c ( *pFreshLocal ), pFreshLocal->m_dKilllistTargets ) )
				pFreshLocal->m_dKilllistTargets.Reset();
			AddDeferred ( sIndex, std::move ( pFreshLocal ) );
			break;
		case ADD_SERVED:
			sphLogDebug ( "add template table %s", sIndex.cstr() );
			assert ( pFreshLocal->m_eType == IndexType_e::TEMPLATE ); // only templates are immediately ready
			m_hLocals.Add ( sIndex );
			m_hNewLocalIndexes.Add ( pFreshLocal, sIndex );
		default:
			break;
		}
	}

	// special pass for 'simple' rotation (i.e. *.new to current)
	void PreparePlainRotationIfNeed ( const CSphString& sIndex, const cServedIndexRefPtr_c& pAlreadyServed )
	{
		assert ( pAlreadyServed->m_eType == IndexType_e::PLAIN );
		if ( !CheckIndexRotate_c ( *pAlreadyServed ).RotateFromNew() )
			return KeepExisting ( sIndex ); // no .new, no need to rotate, just keep existing

		ServedIndexRefPtr_c pIndex = MakeCloneForRotation ( pAlreadyServed, sIndex );

		// reinit klist targets for rotating index
		pIndex->m_dKilllistTargets.Reset();
		if ( !PreloadKlistTarget ( *pIndex, RotateFrom_e::NEW, pIndex->m_dKilllistTargets ) )
			pIndex->m_dKilllistTargets.Reset();

		AddDeferred ( sIndex, std::move ( pIndex ) );
	}

	void ReconfigureIndex ( const CSphString& sIndex, const CSphConfigSection& hIndex )
	{
		cServedIndexRefPtr_c pAlreadyServed = GetSnapServed ( sIndex );
		assert ( pAlreadyServed );

		ServedIndexRefPtr_c pClone = MakeFullClone ( pAlreadyServed );
		ConfigureLocalIndex ( pClone, hIndex, false, nullptr );
		m_hNewLocalIndexes.Add ( pClone, sIndex );

		// need w-lock of already served (exposed) idx, since we change settings.
		// fixme! what about another probably changed settings?
		WIdx_c pIdx { pAlreadyServed };
		pIdx->SetMutableSettings ( pClone->m_tSettings );
		pIdx->SetGlobalIDFPath ( pClone->m_sGlobalIDFPath );
	}

	void LoadLocalFromConfig ( const CSphString& sIndex, IndexType_e eType, const CSphConfigSection& hIndex )
	{
		if ( !ExistsLocal ( sIndex ) )
			return LoadNewLocalFromConfig ( sIndex, hIndex );

		cServedIndexRefPtr_c pAlreadyServed = GetSnapServed ( sIndex );
		assert ( pAlreadyServed );
		if ( eType != pAlreadyServed->m_eType )
		{
			if ( !g_bSeamlessRotate && ServedDesc_t::IsLocal ( pAlreadyServed ) && ( eType == IndexType_e::RT || eType == IndexType_e::PERCOLATE || eType == IndexType_e::PLAIN ) )
			{
				sphWarning ( "table '%s': changing table role plain<>rt<>percolate is possible only with seamless rotation - skip", sIndex.cstr() );
				return;
			}
			return LoadNewLocalFromConfig ( sIndex, hIndex );
		}

		// reload/reconfigure already existing local index of the same type
		bool bPathChanged =
			pAlreadyServed->m_eType != IndexType_e::TEMPLATE
			&& hIndex.Exists ( "path" )
			&& hIndex["path"].strval() != pAlreadyServed->m_sIndexPath;

		if ( bPathChanged )
			return LoadNewLocalFromConfig ( sIndex, hIndex );

		ServedDesc_t tNewDesc;
		ConfigureLocalIndex ( &tNewDesc, hIndex, false, nullptr );
		bool bReconfigured =
			tNewDesc.m_tSettings.m_iExpandKeywords != pAlreadyServed->m_tSettings.m_iExpandKeywords
			|| tNewDesc.m_tSettings.m_tFileAccess != pAlreadyServed->m_tSettings.m_tFileAccess
			|| tNewDesc.m_tSettings.m_bPreopen != pAlreadyServed->m_tSettings.m_bPreopen
			|| tNewDesc.m_sGlobalIDFPath != pAlreadyServed->m_sGlobalIDFPath;

		if ( bReconfigured )
			return ReconfigureIndex ( sIndex, hIndex );

		if ( pAlreadyServed->m_eType == IndexType_e::PLAIN )
			PreparePlainRotationIfNeed ( sIndex, pAlreadyServed );
		else
			KeepExisting ( sIndex );
	}

public:
	Impl_c ( HashOfServed_c& hDeferred )
		: m_hLocalSnapshot { g_pLocalIndexes->GetSnapshot() }
		, m_hDistrSnapshot { g_pDistIndexes->GetSnapshot() }
		, m_hNewLocalIndexes { *g_pLocalIndexes }
		, m_hNewDistrIndexes { *g_pDistIndexes }
		, m_hDeferred ( hDeferred )
	{
		m_hNewLocalIndexes.InitEmptyHash();
		m_hNewDistrIndexes.InitEmptyHash();
	}

	void LoadIndexFromConfig ( const CSphString& sIndex, IndexType_e eType, const CSphConfigSection& hIndex )
	{
		sphLogDebug ( "Load from config table %s with type %s", sIndex.cstr(), szIndexType ( eType ) );
		assert ( eType != IndexType_e::ERROR_ );
		if ( m_hProcessed[sIndex] )
		{
			sphWarning ( "table '%s': duplicate name - NOT SERVING", sIndex.cstr() );
			return;
		}

		if ( eType == IndexType_e::DISTR )
			LoadDistrFromConfig ( sIndex, hIndex );
		else
			LoadLocalFromConfig ( sIndex, eType, hIndex );
		m_hProcessed.Add ( sIndex );
	}

	void IssuePlainOldRotation()
	{
		m_hNewLocalIndexes.CopyOwnerHash();
		m_hNewDistrIndexes.CopyOwnerHash();

		for ( auto& tServed : *m_hLocalSnapshot.first )
		{
			cServedIndexRefPtr_c pAlreadyServed = tServed.second;
			assert ( pAlreadyServed );
			if ( pAlreadyServed->m_eType == IndexType_e::PLAIN )
				PreparePlainRotationIfNeed ( tServed.first, pAlreadyServed );
		}
	}
};

/// public iface
ConfigReloader_c::ConfigReloader_c ( HashOfServed_c& hDeferred )
	: m_pImpl { new Impl_c ( hDeferred ) }
{
	assert ( m_pImpl );
}

ConfigReloader_c::~ConfigReloader_c()
{
	SafeDelete ( m_pImpl );
}


void ConfigReloader_c::LoadIndexFromConfig ( const CSphString& sIndex, IndexType_e eType, const CSphConfigSection& hIndex )
{
	m_pImpl->LoadIndexFromConfig ( sIndex, eType, hIndex );
}

void ConfigReloader_c::IssuePlainOldRotation()
{
	m_pImpl->IssuePlainOldRotation();
}
