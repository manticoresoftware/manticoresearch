//
// $Id$
//

//
// Copyright (c) 2001-2008, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _sphinxfilter_
#define _sphinxfilter_

#include "sphinxstd.h"
#include "sphinx.h"

struct ISphFilter
{
	virtual void SetLocator ( const CSphAttrLocator & ) {}
	virtual void SetRange ( SphAttr_t, SphAttr_t ) {}
	virtual void SetRangeFloat ( float, float ) {}
	virtual void SetValues ( const SphAttr_t *, int ) {}
	virtual void SetMVAStorage ( const DWORD * ) {}
	
	virtual ~ISphFilter () {}
	
	/// evaluate filter for a given match
	/// returns true if match satisfies the filter critertia (i.e. in range, found in values list etc)
	virtual bool Eval ( const CSphMatch & tMatch ) const = 0;

	/// evaluate filter for a given block
	/// returns false if no document in block can possibly pass through the filter
	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo, int iSchemaSize ) const
	{
		// if filter does not implement block-level evaluation we assume the block will pass
		return true;
	}

	virtual ISphFilter * Join ( ISphFilter * pFilter );
};

ISphFilter * sphCreateFilter ( CSphFilterSettings &, const CSphSchema &, const DWORD * pMva );
ISphFilter * sphCreateFilters ( CSphVector<CSphFilterSettings> &, const CSphSchema &, const DWORD * pMva );
ISphFilter * sphJoinFilters ( ISphFilter *, ISphFilter * );

#endif // _sphinxfilter_

//
// $Id$
//
