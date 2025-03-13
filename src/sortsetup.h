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

#ifndef _sortsetup_
#define _sortsetup_

#include "match.h"
#include "sphinx.h"

/// match comparator state
struct CSphMatchComparatorState
{
	static const int	MAX_ATTRS = 5;

	ESphSortKeyPart		m_eKeypart[MAX_ATTRS];		///< sort-by key part type
	CSphAttrLocator		m_tLocator[MAX_ATTRS];		///< sort-by attr locator
	int					m_dAttrs[MAX_ATTRS];		///< sort-by attr index

	DWORD				m_uAttrDesc = 0;			///< sort order mask (if i-th bit is set, i-th attr order is DESC)
	DWORD				m_iNow = 0;					///< timestamp (for timesegments sorting mode)
	SphStringCmp_fn		m_fnStrCmp = nullptr;		///< string comparator
	CSphBitvec			m_dRemapped { CSphMatchComparatorState::MAX_ATTRS };

						CSphMatchComparatorState();

	/// check if any of my attrs are bitfields
	bool				UsesBitfields() const;
	void				FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema, bool bRemapKeyparts );

	FORCE_INLINE int CmpStrings ( const CSphMatch & a, const CSphMatch & b, int iAttr ) const
	{
		assert ( iAttr>=0 && iAttr<MAX_ATTRS );
		assert ( m_eKeypart[iAttr]==SPH_KEYPART_STRING || m_eKeypart[iAttr]==SPH_KEYPART_STRINGPTR );
		assert ( m_fnStrCmp );

		const BYTE * aa = (const BYTE*) a.GetAttr ( m_tLocator[iAttr] );
		const BYTE * bb = (const BYTE*) b.GetAttr ( m_tLocator[iAttr] );
		if ( aa==nullptr || bb==nullptr )
		{
			if ( aa==bb )
				return 0;
			if ( aa==nullptr )
				return -1;
			return 1;
		}

		return m_fnStrCmp ( {aa, 0}, {bb, 0}, m_eKeypart[iAttr]==SPH_KEYPART_STRINGPTR );
	}
};


struct ExtraSortExpr_t
{
	CSphRefcountedPtr<ISphExpr> m_pExpr;
	JsonKey_t					m_tKey;
	ESphAttr					m_eType = SPH_ATTR_NONE;
};

/// parses sort clause, using a given schema
/// fills eFunc and tState and optionally sError, returns result code
ESortClauseParseResult sphParseSortClause ( const CSphQuery & tQuery, const char * sClause, const ISphSchema & tSchema, ESphSortFunc & eFunc, CSphMatchComparatorState & tState, CSphVector<ExtraSortExpr_t> & dExtraExprs, const JoinArgs_t * pJoinArgs, CSphString & sError );

#endif // _sortsetup_
