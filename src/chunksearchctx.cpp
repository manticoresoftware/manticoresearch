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

#include "chunksearchctx.h"

#include "sphinxsort.h"
#include "queryprofile.h"


DiskChunkSearcherCtx_t::DiskChunkSearcherCtx_t ( Sorters_t & dSorters, CSphQueryResultMeta & tMeta )
	: m_dSorters ( dSorters )
	, m_tMeta ( tMeta )
{}


void DiskChunkSearcherCtx_t::MergeChild ( DiskChunkSearcherCtx_t tChild ) const
{
	// sorting results
	ARRAY_CONSTFOREACH ( i, m_dSorters )
		if ( tChild.m_dSorters[i] )
			tChild.m_dSorters[i]->MoveTo ( m_dSorters[i], false );

	auto & tChildRes = tChild.m_tMeta;

	// word statistics
	m_tMeta.MergeWordStats ( tChildRes );

	// other data (warnings, errors, etc.)
	// errors
	if ( !tChildRes.m_sError.IsEmpty ())
		m_tMeta.m_sError = tChildRes.m_sError;

	// warnings
	if ( !tChildRes.m_sWarning.IsEmpty ())
		m_tMeta.m_sWarning = tChildRes.m_sWarning;

	// prediction counters
	if ( m_tMeta.m_bHasPrediction )
		m_tMeta.m_tStats.Add ( tChildRes.m_tStats );

	// profiling
	if ( tChildRes.m_pProfile )
		m_tMeta.m_pProfile->AddMetric ( *tChildRes.m_pProfile );

	m_tMeta.m_bTotalMatchesApprox |= tChildRes.m_bTotalMatchesApprox;
	m_tMeta.m_tIteratorStats.Merge ( tChildRes.m_tIteratorStats );
}


bool DiskChunkSearcherCtx_t::IsClonable () const
{
	return m_dSorters.all_of ( [] ( const ISphMatchSorter * p ) { return p->CanBeCloned (); } );
}

//////////////////////////////////////////////////////////////////////////

DiskChunkSearcherCloneCtx_t::DiskChunkSearcherCloneCtx_t ( const DiskChunkSearcherCtx_t & dParent )
{
	m_dSorters.Resize ( dParent.m_dSorters.GetLength() );
	ARRAY_FOREACH ( i, m_dSorters )
		m_dSorters[i] = dParent.m_dSorters[i]->Clone();

	m_tMeta.m_bHasPrediction = dParent.m_tMeta.m_bHasPrediction;
	if ( dParent.m_tMeta.m_pProfile )
		m_tMeta.m_pProfile = new QueryProfile_c;
}


DiskChunkSearcherCloneCtx_t::~DiskChunkSearcherCloneCtx_t()
{
	m_dSorters.Apply ( [] ( ISphMatchSorter *& pSorter ) { SafeDelete ( pSorter ); } );
	SafeDelete ( m_tMeta.m_pProfile );
}

DiskChunkSearcherCloneCtx_t::operator DiskChunkSearcherCtx_t()
{
	return { m_dSorters, m_tMeta };
}
