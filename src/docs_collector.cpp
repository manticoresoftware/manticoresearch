//
// Copyright (c) 2008-2022, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "docs_collector.h"
#include "searchdaemon.h"
#include "indexfiles.h"

class DocsCollector_c::Impl_c
{
	VecTraits_T<DocID_t>	m_dFastSlice;
	CSphVector<DocID_t>		m_dValues;
	CSphVector<BYTE>		m_dCompressedDocids;
	std::unique_ptr<MemoryReader_c>	m_pCompressedReader;
	int						m_iFastIdx = 0;
	DocID_t					m_iLastId = -1;
	bool					m_bFastPath = false;

	// check the short path - if we have clauses 'id=smth' or 'id in (xx,yy)' or 'id in @uservar' - we know
	// all the values list immediatelly and don't have to run the heavy query here.
	bool ProcessFast( const CSphQuery& tQuery )
	{
		if ( !tQuery.m_sQuery.IsEmpty() || !tQuery.m_dFilterTree.IsEmpty() || tQuery.m_dFilters.GetLength() != 1 )
			return false;

		const CSphFilterSettings* pFilter = tQuery.m_dFilters.Begin();
		if ( ( pFilter->m_bHasEqualMin || pFilter->m_bHasEqualMax ) && pFilter->m_eType == SPH_FILTER_VALUES
				&& ( pFilter->m_sAttrName == "@id" || pFilter->m_sAttrName == "id" ) && !pFilter->m_bExclude )
		{
			m_dFastSlice = pFilter->GetValues();
			m_iFastIdx = 0;
			m_bFastPath = true;
		}
		return m_bFastPath;
	}

	void ProcessFull ( const CSphQuery& tQuery, bool bJson, const CSphString& sIndex, const cServedIndexRefPtr_c& pDesc, CSphString* pError )
	{
		PubSearchHandler_c tHandler ( 1, CreateQueryParser ( bJson ), tQuery.m_eQueryType, false );
		tHandler.PushIndex ( sIndex, pDesc );
		tHandler.RunCollect ( tQuery, sIndex, pError, &m_dCompressedDocids );

		if ( m_dCompressedDocids.IsEmpty() )
			return;

		m_pCompressedReader = std::make_unique<MemoryReader_c> ( m_dCompressedDocids );
		m_iLastId = 0;
	}

	bool GetValuesChunkFast ( CSphVector<DocID_t>& dValues, int iValues=-1 )
	{
		assert ( m_bFastPath );
		if ( iValues < 0 )
			iValues = m_dFastSlice.GetLength() - m_iFastIdx;
		else
			iValues = Min ( iValues, m_dFastSlice.GetLength() - m_iFastIdx );

		if ( !iValues )
			return false;

		for ( const auto& i : m_dFastSlice.Slice ( std::exchange ( m_iFastIdx, m_iFastIdx + iValues ), iValues ) )
			dValues.Add ( i );
		return true;
	}

	bool GetValuesChunkFull ( CSphVector<DocID_t>& dValues, int iValues )
	{
		assert ( !m_bFastPath );

		if ( !m_pCompressedReader || !m_pCompressedReader->HasData() )
			return false;

		while ( m_pCompressedReader->HasData() && iValues-- )
		{
			m_iLastId += m_pCompressedReader->UnzipOffset();
			dValues.Add ( m_iLastId );
		}

		return true;
	}

public:
	Impl_c ( const CSphQuery& tQuery, bool bJson, const CSphString& sIndex, const cServedIndexRefPtr_c& pDesc, CSphString* pError )
	{
		if ( !ProcessFast ( tQuery ) )
			ProcessFull ( tQuery, bJson, sIndex, pDesc, pError );
	}

	bool GetValuesChunk ( CSphVector<DocID_t>& dValues, int iValues )
	{
		dValues.Resize(0);
		return m_bFastPath ? GetValuesChunkFast ( dValues, iValues ) : GetValuesChunkFull ( dValues, iValues );
	}

	// beware, that slice lives together with this class, and will became undefined once it destroyed.
	VecTraits_T<DocID_t> GetValuesSlice ()
	{
		if ( m_bFastPath )
			return m_dFastSlice;

		if ( !m_pCompressedReader && !m_dCompressedDocids.IsEmpty() )
		{
			m_pCompressedReader = std::make_unique<MemoryReader_c> ( m_dCompressedDocids );
			m_iLastId = 0;
			m_dValues.Reset();
		}

		if ( m_pCompressedReader )
			while ( m_pCompressedReader->HasData() )
			{
				m_iLastId += m_pCompressedReader->UnzipOffset();
				m_dValues.Add ( m_iLastId );
			}
		return m_dValues;
	}
};

/// public iface
DocsCollector_c::DocsCollector_c ( const CSphQuery& tQuery, bool bJson, const CSphString& sIndex, const cServedIndexRefPtr_c& pDesc, CSphString* pError )
	: m_pImpl { std::make_unique<Impl_c> ( tQuery, bJson, sIndex, pDesc, pError ) }
{}

DocsCollector_c::~DocsCollector_c() = default;

DocsCollector_c::DocsCollector_c ( DocsCollector_c&& rhs ) noexcept
	: m_pImpl ( std::exchange ( rhs.m_pImpl, nullptr ) )
{}

bool DocsCollector_c::GetValuesChunk ( CSphVector<DocID_t>& dValues, int iValues )
{
	return m_pImpl->GetValuesChunk( dValues, iValues );
}

VecTraits_T<DocID_t> DocsCollector_c::GetValuesSlice()
{
	return m_pImpl->GetValuesSlice();
}