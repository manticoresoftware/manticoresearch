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

#include "sphinxfilter.h"

/// attribute-based
struct IFilter_Attr: virtual ISphFilter
{
	CSphAttrLocator m_tLocator;
	
	virtual void SetLocator ( const CSphAttrLocator & tLocator )
	{
		m_tLocator = tLocator;
	}
};

/// values
struct IFilter_Values: virtual ISphFilter
{
	const SphAttr_t *	m_pValues;
	int					m_iValueCount;

	IFilter_Values ()
		: m_pValues		( NULL )
		, m_iValueCount	( 0 )
	{}
	
	virtual void SetValues ( const SphAttr_t * pStorage, int iCount )
	{
		assert ( pStorage );
		assert ( iCount > 0 );
		#ifndef NDEBUG // values must be sorted
		for ( int i = 1; i < iCount; i++ )
			assert ( pStorage[i-1] <= pStorage[i] );
		#endif
			
		m_pValues = pStorage;
		m_iValueCount = iCount;
	}

	inline const SphAttr_t GetValue ( int iIndex ) const
	{
		assert ( iIndex >= 0 && iIndex < m_iValueCount );
		return m_pValues[iIndex];
	}
	
	bool EvalValues ( SphAttr_t uValue ) const;
	bool EvalBlockValues ( SphAttr_t uBlockMin, SphAttr_t uBlockMax ) const;
};


bool IFilter_Values::EvalValues ( SphAttr_t uValue ) const
{
	if ( !m_pValues )
		return true;

	const SphAttr_t * pA = m_pValues;
	const SphAttr_t * pB = m_pValues + m_iValueCount - 1;
	
	if ( uValue==*pA || uValue==*pB ) return true;
	if ( uValue<(*pA) || uValue>(*pB) ) return false;

	while ( pB-pA>1 )
	{
		const SphAttr_t * pM = pA + ((pB-pA)/2);
		if ( uValue==(*pM) )
			return true;
		if ( uValue<(*pM) )
			pB = pM;
		else
			pA = pM;
	}
	return false;
}


// OPTIMIZE: use binary search
bool IFilter_Values::EvalBlockValues ( SphAttr_t uBlockMin, SphAttr_t uBlockMax ) const
{
	// is any of our values inside the block?
	for ( int i = 0; i < m_iValueCount; i++ )
		if ( GetValue(i) >= uBlockMin && GetValue(i) <= uBlockMax )
			return true;
	return false;
}

/// range
struct IFilter_Range: virtual ISphFilter
{
	SphAttr_t m_uMinValue;
	SphAttr_t m_uMaxValue;

	virtual void SetRange ( SphAttr_t tMin, SphAttr_t tMax )
	{
		m_uMinValue = tMin;
		m_uMaxValue = tMax;
	}

	bool EvalRange ( const SphAttr_t uValue ) const
	{
		return uValue>=m_uMinValue && uValue<=m_uMaxValue;
	}
};

/// MVA
struct IFilter_MVA: virtual IFilter_Attr
{
	const DWORD *	m_pMvaStorage;

	IFilter_MVA ()
		: m_pMvaStorage ( NULL )
	{}
	
	virtual void SetMVAStorage ( const DWORD * pMva )
	{
		m_pMvaStorage = pMva;
	}

	inline bool LoadMVA( const CSphMatch & tMatch, const DWORD ** pMva, const DWORD ** pMvaMax ) const
	{
		assert ( m_pMvaStorage );
		
		*pMva = tMatch.GetAttrMVA ( m_tLocator, m_pMvaStorage );
		if ( !*pMva )
			return false;

		*pMvaMax = *pMva + (**pMva) + 1;
		(*pMva)++;
		return true;
	}
};

/// filters

// attr

struct Filter_Values: public IFilter_Attr, IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalValues ( tMatch.GetAttr ( m_tLocator ) );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo, int iSchemaSize ) const
	{
		if ( m_tLocator.m_iBitOffset >= iSchemaSize )
			return true; // ignore computed attributes
		
		SphAttr_t uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS(pMinDocinfo), m_tLocator );
		SphAttr_t uBlockMax = sphGetRowAttr ( DOCINFO2ATTRS(pMaxDocinfo), m_tLocator );

		return EvalBlockValues ( uBlockMin, uBlockMax );
	}
};

struct Filter_Range: public IFilter_Attr, IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
 		return EvalRange ( tMatch.GetAttr ( m_tLocator ) );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo, int iSchemaSize ) const
	{
		if ( m_tLocator.m_iBitOffset >= iSchemaSize )
			return true; // ignore computed attributes
		
		SphAttr_t uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS(pMinDocinfo), m_tLocator );
		return m_uMinValue <= uBlockMin && m_uMaxValue >= uBlockMin;
	}
};

// float

struct Filter_FloatRange: public IFilter_Attr
{
	float m_fMinValue;
	float m_fMaxValue;
	
	virtual void SetRangeFloat ( float fMin, float fMax )
	{
		m_fMinValue = fMin;
		m_fMaxValue = fMax;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
 		const float & fValue = tMatch.GetAttrFloat ( m_tLocator );
 		return fValue >= m_fMinValue && fValue <= m_fMaxValue;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo, int iSchemaSize ) const
	{
		if ( m_tLocator.m_iBitOffset >= iSchemaSize )
			return true; // ignore computed attributes

		SphAttr_t uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS(pMinDocinfo), m_tLocator );
		float fBlockMin = sphDW2F ( (DWORD)uBlockMin );
		return m_fMinValue <= fBlockMin && m_fMaxValue >= fBlockMin;
	}
};

// id

struct Filter_IdValues: public IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalValues ( tMatch.m_iDocID );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo, int ) const
	{
		const SphAttr_t uBlockMin = DOCINFO2ID(pMinDocinfo);
		const SphAttr_t uBlockMax = DOCINFO2ID(pMaxDocinfo);

		return EvalBlockValues ( uBlockMin, uBlockMax );
	}
};

struct Filter_IdRange: public IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const SphDocID_t uID = tMatch.m_iDocID;
		return uID>=(SphDocID_t)m_uMinValue && uID<=(SphDocID_t)m_uMaxValue;
	}
};

// weight

struct Filter_WeightValues: public IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalValues ( tMatch.m_iWeight );
	}
};

struct Filter_WeightRange: public IFilter_Range
{
	virtual bool IsEarly () { return false; }
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange ( tMatch.m_iWeight );
	}
};

// MVA

struct Filter_MVAValues: public IFilter_MVA, IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const;
};

bool Filter_MVAValues::Eval ( const CSphMatch & tMatch ) const
{
	const DWORD * pMva, * pMvaMax;
	if ( !LoadMVA ( tMatch, &pMva, &pMvaMax ) )
		return false;

	const SphAttr_t * pFilter = m_pValues;
	const SphAttr_t * pFilterMax = pFilter + m_iValueCount;
	
	const DWORD * L = pMva;
	const DWORD * R = pMvaMax - 1;
	for ( ; pFilter < pFilterMax; pFilter++ )
	{
		while ( L <= R )
		{
			const DWORD * m = L + (R - L) / 2;
			if ( *pFilter > *m )
				L = m + 1;
			else if ( *pFilter < *m )
				R = m - 1;
			else
				return true;
		}
		R = pMvaMax - 1;
	}
	return false;
}

struct Filter_MVARange: public IFilter_MVA, IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const;
};

bool Filter_MVARange::Eval ( const CSphMatch & tMatch ) const
{
	const DWORD * pMva, * pMvaMax;
	if ( !LoadMVA ( tMatch, &pMva, &pMvaMax ) )
		return false;
	
	const DWORD * L = pMva;
	const DWORD * R = pMvaMax - 1;

	while ( L <= R )
	{
		const DWORD * m = L + (R - L) / 2;
		if ( m_uMinValue > *m )
			L = m + 1;
		else if ( m_uMinValue < *m )
			R = m - 1;
		else
			return true;
	}
	if ( L == pMvaMax )
		return false;
	return *L <= m_uMaxValue;
}

// and

struct Filter_And: public ISphFilter
{
	CSphVector<ISphFilter *> m_dFilters;

	~Filter_And ()
	{
		ARRAY_FOREACH ( i, m_dFilters )
			SafeDelete ( m_dFilters[i] );
	}

	void Add ( ISphFilter * pFilter )
	{
		m_dFilters.Add ( pFilter );
	}
	
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		ARRAY_FOREACH ( i, m_dFilters )
			if ( !m_dFilters[i]->Eval ( tMatch ) )
				return false;
		return true;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo, int iSchemaSize ) const
	{
		ARRAY_FOREACH ( i, m_dFilters )
			if ( !m_dFilters[i]->EvalBlock ( pMinDocinfo, pMaxDocinfo, iSchemaSize ) )
				return false;
		return true;
	}

	virtual ISphFilter * Join ( ISphFilter * pFilter )
	{
		Add ( pFilter );
		return this;
	}
};

// not

struct Filter_Not: public ISphFilter
{
	ISphFilter * m_pFilter;
	
	explicit Filter_Not ( ISphFilter * pFilter )
		: m_pFilter(pFilter)
	{
		assert ( pFilter );
	}

	~Filter_Not ()
	{
		SafeDelete ( m_pFilter );
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return !m_pFilter->Eval ( tMatch );
	}

	virtual bool EvalBlock ( const DWORD *, const DWORD *, int ) const
	{
		// if block passes through the filter we can't just negate the
		// result since it's imprecise at this point
		return true;
	}
};

/// impl

ISphFilter * ISphFilter::Join ( ISphFilter * pFilter )
{
	Filter_And * pAnd = new Filter_And();

	pAnd->Add ( this );
	pAnd->Add ( pFilter );

	return pAnd;
}

/// helper functions

static ISphFilter * CreateSpecialFilter ( const CSphString & sName, ESphFilter eFilterType )
{
	if ( sName=="@id" )
	{
		switch ( eFilterType )
		{
			case SPH_FILTER_VALUES:	return new Filter_IdValues;
			case SPH_FILTER_RANGE:	return new Filter_IdRange;
			default:
				assert ( 0 && "invalid filter on @id" );
				return NULL;
		}
	}
	else if ( sName=="@weight" )
	{
		switch ( eFilterType )
		{
			case SPH_FILTER_VALUES:	return new Filter_WeightValues;
			case SPH_FILTER_RANGE:	return new Filter_WeightRange;
			default:
				assert ( 0 && "invalid filter on @weight" );
				return NULL;
		}
	}

	return NULL;
}

static ISphFilter * CreateFilter ( DWORD eAttrType, ESphFilter eFilterType )
{
	// MVA
	if ( eAttrType & SPH_ATTR_MULTI )
	{
		switch ( eFilterType )
		{
			case SPH_FILTER_VALUES:	return new Filter_MVAValues;
			case SPH_FILTER_RANGE:	return new Filter_MVARange;
			default:
				assert ( 0 && "invalid MVA filter" );
				return NULL;
		}
	}

	// non-MVA
	if ( eAttrType == SPH_ATTR_FLOAT )
	{
		if ( eFilterType == SPH_FILTER_FLOATRANGE )
			return new Filter_FloatRange;
		else
			assert ( 0 && "invalid float filter" );
	}
	else
	{
		switch ( eFilterType )
		{
			case SPH_FILTER_VALUES:	return new Filter_Values;
			case SPH_FILTER_RANGE:	return new Filter_Range;
			default:
				assert ( 0 && "invalid attribute filter" );
		}
	}

	return NULL;
}	

ISphFilter * sphCreateFilter ( CSphFilterSettings & tSettings, const CSphSchema & tSchema, const DWORD * pMvaPool )
{
	ISphFilter * pFilter = 0;

	// try to create filter on special attribute
	const CSphString & sAttrName = tSettings.m_sAttrName;
	if ( sAttrName.Begins("@") )
		pFilter = CreateSpecialFilter ( sAttrName, tSettings.m_eType );
	
	// fetch column info
	const CSphColumnInfo * pAttr = NULL;
	const int iAttr = tSchema.GetAttrIndex ( sAttrName.cstr() );
	if ( iAttr < 0 )
	{
		if ( !pFilter )
			return NULL; // no such attribute
	}
	else
	{
		assert ( !pFilter );
		
		pAttr = &tSchema.GetAttr(iAttr);
		pFilter = CreateFilter ( pAttr->m_eAttrType, tSettings.m_eType );
	}

	// fill filter's properties
	if ( pFilter )
	{
		if ( pAttr )
			pFilter->SetLocator ( pAttr->m_tLocator );
		
		pFilter->SetRange ( tSettings.m_uMinValue, tSettings.m_uMaxValue );
		pFilter->SetRangeFloat ( tSettings.m_fMinValue, tSettings.m_fMaxValue );
		pFilter->SetMVAStorage ( pMvaPool );

		if ( tSettings.GetNumValues() > 0 )
		{
			tSettings.SortValues();
			pFilter->SetValues ( tSettings.GetValueArray(), tSettings.GetNumValues() );
		}

		if ( tSettings.m_bExclude )
			pFilter = new Filter_Not ( pFilter );
	}

	return pFilter;
}

ISphFilter * sphCreateFilters ( CSphVector<CSphFilterSettings> & dSettings, const CSphSchema & tSchema, const DWORD * pMvaPool )
{
	ISphFilter *pResult = NULL;
	
	ARRAY_FOREACH ( i, dSettings )
	{
		ISphFilter * pFilter = sphCreateFilter ( dSettings[i], tSchema, pMvaPool );
		if ( !pFilter )
			continue;

		pResult = sphJoinFilters ( pResult, pFilter );
	}

	return pResult;
}

ISphFilter * sphJoinFilters ( ISphFilter * pA, ISphFilter * pB )
{
	if ( pA )
		return pB ? pA->Join(pB) : pA;
	return pB;
}

//
// $Id$
//
