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

#include "sphinxstd.h"
#include "sphinxdefs.h"

class CSphSchema;
struct CSphAttrLocator;

// FIXME!!! for over INT_MAX attributes
/// attr min-max builder
class AttrIndexBuilder_c : ISphNoncopyable
{
public:
	explicit	AttrIndexBuilder_c ( const CSphSchema & tSchema );
				AttrIndexBuilder_c() = default;

	void		Init ( const CSphSchema & tSchema );
	void		Collect ( const CSphRowitem * pRow );
	void		FinishCollect();
	const CSphTightVector<CSphRowitem> & GetCollected() const;

private:
	CSphVector<CSphAttrLocator>	m_dIntAttrs;
	CSphVector<CSphAttrLocator>	m_dFloatAttrs;

	CSphVector<SphAttr_t>		m_dIntMin;
	CSphVector<SphAttr_t>		m_dIntMax;
	CSphVector<float>			m_dFloatMin;
	CSphVector<float>			m_dFloatMax;

	CSphVector<SphAttr_t>		m_dIntIndexMin;
	CSphVector<SphAttr_t>		m_dIntIndexMax;
	CSphVector<float>			m_dFloatIndexMin;
	CSphVector<float>			m_dFloatIndexMax;

	DWORD						m_uStride {0};
	int							m_nLocalCollected {0};

	CSphTightVector<CSphRowitem> m_dMinMaxRows;

	void						ResetLocal();
	void						FlushComputed();
};
