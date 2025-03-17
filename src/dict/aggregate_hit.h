//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "sphinxdefs.h"
#include "sphinx.h"

/// aggregated hit info
struct AggregateHit_t
{
	RowID_t m_tRowID { INVALID_ROWID }; ///< document ID
	SphWordID_t m_uWordID { 0 };		///< word ID in current dictionary
	const BYTE* m_szKeyword { nullptr }; ///< word itself (in keywords dictionary case only)
	Hitpos_t m_iWordPos { 0 };			///< word position in current document, or hit count in case of aggregate hit
	FieldMask_t m_dFieldMask;			///< mask of fields containing this word, 0 for regular hits, non-0 for aggregate hits

	int GetAggrCount() const
	{
		assert ( !m_dFieldMask.TestAll ( false ) );
		return m_iWordPos;
	}

	void SetAggrCount ( int iVal )
	{
		m_iWordPos = iVal;
	}
};
