//
// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#pragma once

#include "postingwindow.h"
#include "sortbound.h"
#include "secondaryindex.h"


// Worker-local competitive rows for a direct rowwise integer sort. SI and min/max
// are alternative sources; the exact WHERE filter remains in the ranker.
class SortBoundFilter_c final : public PostingCandidateFilter_i
{
public:
				SortBoundFilter_c ( const SortBound_c & tBound, const CSphAttrLocator & tLocator, const DWORD * pRows, const DWORD * pMinMax, RowID_t tRows, int iStride, const SIContainer_c * pSI );

	std::unique_ptr<PostingCandidateFilter_i> CloneCursor() final;

	RowID_t		AdvanceTo ( RowID_t tMin ) final;
	int			SelectRows ( RowID_t * pRows, int iCount ) final;
	bool		HasPruned() const		{ return m_pSource->m_bPruned; }
	const char * GetName() const		{ return m_pSource->m_bSI ? "SortBoundSI" : "SortBoundMinMax"; }

private:
	const SortBound_c &	m_tBound;
	CSphAttrLocator		m_tLocator;
	const DWORD *		m_pRows;
	const DWORD *		m_pMinMax;
	RowID_t				m_tRows;
	int					m_iStride;
	const SIContainer_c * m_pSI;
	std::unique_ptr<RowidIterator_i> m_pIterator;
	RowIdBlock_t		m_dRows;
	int					m_iRead = 0;
	SortBoundFilter_c *	m_pSource = this;
	SphAttr_t			m_tSourceBound = 0;
	bool				m_bCursorReady = false;
	bool				m_bSIReady = false;
	bool				m_bSelected = false;
	bool				m_bSI = false;
	bool				m_bPruned = false;

	bool		Accepts ( SphAttr_t tValue ) const;
	void		SelectSource();
	RowID_t		AdvanceSI ( RowID_t tMin );
};
