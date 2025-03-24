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

#include "schema/locator.h"

/// search query match (document info plus weight/tag)
class CSphMatch : public ISphNoncopyable
{
	friend class ISphSchema;
	friend class CSphSchema;
	friend class CSphSchemaHelper;
	friend class CSphRsetSchema;

public:
	RowID_t					m_tRowID {INVALID_ROWID};	///< document ID
	const CSphRowitem *		m_pStatic {nullptr};		///< static part (stored in and owned by the index)
	CSphRowitem *			m_pDynamic {nullptr};		///< dynamic part (computed per query; owned by the match)
	int						m_iWeight {0};				///< my computed weight
	int						m_iTag {0};					///< my index tag

public:
	CSphMatch () = default;
	CSphMatch ( RowID_t tRowID, const CSphRowitem* pStatic )
		: m_tRowID { tRowID }
		, m_pStatic { pStatic }
	{}

	/// dtor. frees everything
	~CSphMatch ();

	/// reset
	void Reset ( int iDynamic );

	// if mem region reinterpreted as match - ensure d-tr will not do sad things as delete garbaged m_pDynamic
	void CleanGarbage();

	void ResetDynamic();

	/// assignment
	void Combine ( const CSphMatch & rhs, int iDynamic );

	/// integer getter
	FORCE_INLINE SphAttr_t GetAttr ( const CSphAttrLocator & tLoc ) const;

	float GetAttrFloat ( const CSphAttrLocator & tLoc ) const;
	double GetAttrDouble ( const CSphAttrLocator & tLoc ) const;

	/// integer setter
	FORCE_INLINE void SetAttr ( const CSphAttrLocator & tLoc, SphAttr_t uValue ) const;

	/// add scalar value to attribute
	void AddCounterScalar ( const CSphAttrLocator & tLoc, SphAttr_t uValue ) const;

	/// add same-located value from another match
	void AddCounterAttr ( const CSphAttrLocator & tLoc, const CSphMatch& tValue ) const;

	void SetAttrFloat ( const CSphAttrLocator & tLoc, float fValue ) const;
	void SetAttrDouble ( const CSphAttrLocator & tLoc, double fValue ) const;

	/// fetches blobs from both data ptr attrs and pooled blob attrs
	ByteBlob_t FetchAttrData ( const CSphAttrLocator & tLoc, const BYTE * pPool ) const;
};

/// specialized swapper
void Swap ( CSphMatch & a, CSphMatch & b );

FORCE_INLINE SphAttr_t ExchangeAttr ( const CSphMatch& tMatch, const CSphAttrLocator& tLoc, SphAttr_t uNewValue );

#include "match_impl.h"
