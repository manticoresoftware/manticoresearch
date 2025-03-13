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

#include "sphinx.h"


struct DiskChunkSearcherCtx_t
{
	using Sorters_t = VecTraits_T<ISphMatchSorter *>;
	Sorters_t &				m_dSorters;
	CSphQueryResultMeta &	m_tMeta;


			DiskChunkSearcherCtx_t ( Sorters_t & dSorters, CSphQueryResultMeta & tMeta );

	// called from finalize
	void	MergeChild ( DiskChunkSearcherCtx_t tChild ) const;
	bool	IsClonable () const;
};


struct DiskChunkSearcherCloneCtx_t
{
	CSphVector<ISphMatchSorter *>	m_dSorters;
	CSphQueryResultMeta				m_tMeta;

	explicit DiskChunkSearcherCloneCtx_t ( const DiskChunkSearcherCtx_t & dParent );
	~DiskChunkSearcherCloneCtx_t();

	explicit operator DiskChunkSearcherCtx_t();
};
