//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxfilter_
#define _sphinxfilter_

#include "sphinx.h"

struct ISphFilter
{
	virtual void SetLocator ( const CSphAttrLocator & ) {}
	virtual void SetRange ( SphAttr_t, SphAttr_t ) {}
	virtual void SetRangeFloat ( float, float ) {}
	virtual void SetValues ( const SphAttr_t *, int ) {}
	virtual void SetBlobStorage ( const BYTE * ) {}
	virtual void SetRefString ( const CSphString * , int ) {}

	virtual ~ISphFilter () {}

	virtual ISphFilter * Optimize() { return this; }

	/// evaluate filter for a given match
	/// returns true if match satisfies the filter critertia (i.e. in range, found in values list etc)
	virtual bool Eval ( const CSphMatch & tMatch ) const = 0;

	/// evaluate filter for a given block
	/// args are pMinDocinfo and pMaxDocinfo
	/// returns false if no document in block can possibly pass through the filter
	virtual bool EvalBlock ( const DWORD *, const DWORD * ) const
	{
		// if filter does not implement block-level evaluation we assume the block will pass
		return true;
	}

	virtual ISphFilter * Join ( ISphFilter * pFilter );

	ISphFilter() {}
};

// fwd
using UservarIntSetValues_c = CSphVector<SphAttr_t>;
using UservarIntSet_c = SharedPtr_t<UservarIntSetValues_c *>;
class HistogramContainer_c;

struct CreateFilterContext_t
{
	const CSphVector<CSphFilterSettings> * m_pFilters = nullptr;
	const CSphVector<FilterTreeItem_t> * m_pFilterTree = nullptr;

	const ISphSchema * m_pSchema = nullptr;
	const BYTE * m_pBlobPool { nullptr };
	ESphCollation m_eCollation { SPH_COLLATION_DEFAULT };
	bool m_bScan = false;

	ISphFilter * m_pFilter = nullptr;
	ISphFilter * m_pWeightFilter = nullptr;
	CSphVector<UservarIntSet_c> m_dUserVals;

	CreateFilterContext_t ( const ISphSchema * pSchema=nullptr )
		: m_pSchema ( pSchema ) {};

	const HistogramContainer_c * m_pHistograms = nullptr;

	~CreateFilterContext_t();
};

ISphFilter * sphCreateFilter ( const CSphFilterSettings &tSettings, const CreateFilterContext_t &tCtx, CSphString &sError, CSphString &sWarning);
ISphFilter * sphCreateAggrFilter ( const CSphFilterSettings * pSettings, const CSphString & sAttrName, const ISphSchema & tSchema, CSphString & sError );
ISphFilter * sphJoinFilters ( ISphFilter *, ISphFilter * );



bool sphCreateFilters ( CreateFilterContext_t & tCtx, CSphString & sError, CSphString & sWarning );

void FormatFilterQL ( const CSphFilterSettings & tFilter, StringBuilder_c & tBuf, int iCompactIN );
void FormatFiltersQL ( const CSphVector<CSphFilterSettings> & dFilters, const CSphVector<FilterTreeItem_t> & dFilterTree, StringBuilder_c & tBuf, int iCompactIN=5 );


// fwd
class PercolateFilter_i
{
public:
	PercolateFilter_i () {};
	virtual ~PercolateFilter_i() {};

	virtual bool Eval ( SphAttr_t uUID ) = 0;
	virtual void SetRange ( SphAttr_t tMin, SphAttr_t tMax ) {}
};

PercolateFilter_i * CreatePercolateFilter ( const CSphFilterSettings * pUID );

#endif // _sphinxfilter_
