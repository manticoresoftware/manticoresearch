//
// $Id$
//

//
// Copyright (c) 2001-2012, Andrew Aksyonoff
// Copyright (c) 2008-2012, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxfilter.h"
#include "sphinxint.h"

#if USE_WINDOWS
#pragma warning(disable:4250) // inheritance via dominance is our intent
#endif

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
			assert ( pStorage[i-1]<=pStorage[i] );
		#endif

		m_pValues = pStorage;
		m_iValueCount = iCount;
	}

	inline const SphAttr_t GetValue ( int iIndex ) const
	{
		assert ( iIndex>=0 && iIndex<m_iValueCount );
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
		if ( GetValue(i)>=uBlockMin && GetValue(i)<=uBlockMax )
			return true;
	return false;
}

/// range
struct IFilter_Range: virtual ISphFilter
{
	SphAttr_t m_iMinValue;
	SphAttr_t m_iMaxValue;

	virtual void SetRange ( SphAttr_t tMin, SphAttr_t tMax )
	{
		m_iMinValue = tMin;
		m_iMaxValue = tMax;
	}

	bool EvalRange ( const SphAttr_t uValue ) const
	{
		return uValue>=m_iMinValue && uValue<=m_iMaxValue;
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

	inline bool LoadMVA ( const CSphMatch & tMatch, const DWORD ** pMva, const DWORD ** pMvaMax ) const
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

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		SphAttr_t uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS ( pMinDocinfo ), m_tLocator );
		SphAttr_t uBlockMax = sphGetRowAttr ( DOCINFO2ATTRS ( pMaxDocinfo ), m_tLocator );

		return EvalBlockValues ( uBlockMin, uBlockMax );
	}
};


struct Filter_SingleValue : public IFilter_Attr
{
	SphAttr_t m_RefValue;

#ifndef NDEBUG
	virtual void SetValues ( const SphAttr_t * pStorage, int iCount )
#else
	virtual void SetValues ( const SphAttr_t * pStorage, int )
#endif
	{
		assert ( pStorage );
		assert ( iCount==1 );
		m_RefValue = (*pStorage);
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return tMatch.GetAttr ( m_tLocator )==m_RefValue;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		SphAttr_t uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS ( pMinDocinfo ), m_tLocator );
		SphAttr_t uBlockMax = sphGetRowAttr ( DOCINFO2ATTRS ( pMaxDocinfo ), m_tLocator );
		return ( uBlockMin<=m_RefValue && m_RefValue<=uBlockMax );
	}
};


struct Filter_SingleValueStatic32 : public Filter_SingleValue
{
	int m_iIndex;

	virtual void SetLocator ( const CSphAttrLocator & tLoc )
	{
		assert ( tLoc.m_iBitCount==32 );
		assert ( ( tLoc.m_iBitOffset % 32 )==0 );
		assert ( !tLoc.m_bDynamic );
		m_tLocator = tLoc;
		m_iIndex = tLoc.m_iBitOffset / 32;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return tMatch.m_pStatic [ m_iIndex ]==m_RefValue;
	}
};


struct Filter_Range: public IFilter_Attr, IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange ( tMatch.GetAttr ( m_tLocator ) );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		SphAttr_t uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS ( pMinDocinfo ), m_tLocator );
		SphAttr_t uBlockMax = sphGetRowAttr ( DOCINFO2ATTRS ( pMaxDocinfo ), m_tLocator );
		return (!( m_iMaxValue<uBlockMin || m_iMinValue>uBlockMax )); // not-reject
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
		return fValue>=m_fMinValue && fValue<=m_fMaxValue;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		float fBlockMin = sphDW2F ( (DWORD)sphGetRowAttr ( DOCINFO2ATTRS ( pMinDocinfo ), m_tLocator ) );
		float fBlockMax = sphDW2F ( (DWORD)sphGetRowAttr ( DOCINFO2ATTRS ( pMaxDocinfo ), m_tLocator ) );
		return (!( m_fMaxValue<fBlockMin || m_fMinValue>fBlockMax )); // not-reject
	}
};

// id

struct Filter_IdValues: public IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalValues ( tMatch.m_iDocID );
	}

	bool EvalBlockValues ( SphAttr_t uBlockMin, SphAttr_t uBlockMax ) const
	{
		// is any of our values inside the block?
		for ( int i = 0; i < m_iValueCount; i++ )
			if ( (SphDocID_t)GetValue(i)>=(SphDocID_t)uBlockMin && (SphDocID_t)GetValue(i)<=(SphDocID_t)uBlockMax )
				return true;
		return false;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		const SphAttr_t uBlockMin = DOCINFO2ID ( pMinDocinfo );
		const SphAttr_t uBlockMax = DOCINFO2ID ( pMaxDocinfo );

		return EvalBlockValues ( uBlockMin, uBlockMax );
	}

	Filter_IdValues ()
	{
		m_bUsesAttrs = false;
	}
};

struct Filter_IdRange: public IFilter_Range
{
	virtual void SetRange ( SphAttr_t tMin, SphAttr_t tMax )
	{
		m_iMinValue = Max ( 0, tMin );
		m_iMaxValue = tMax;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const SphDocID_t uID = tMatch.m_iDocID;
		return uID>=(SphDocID_t)m_iMinValue && uID<=(SphDocID_t)m_iMaxValue;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		const SphDocID_t uBlockMin = DOCINFO2ID ( pMinDocinfo );
		const SphDocID_t uBlockMax = DOCINFO2ID ( pMaxDocinfo );

		return (!( (SphDocID_t)m_iMaxValue<uBlockMin || (SphDocID_t)m_iMinValue>uBlockMax ));
	}

	Filter_IdRange ()
	{
		m_bUsesAttrs = false;
	}
};

// weight

struct Filter_WeightValues: public IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalValues ( tMatch.m_iWeight );
	}

	Filter_WeightValues ()
	{
		m_bUsesAttrs = false;
	}
};

struct Filter_WeightRange: public IFilter_Range
{
	virtual bool IsEarly () { return false; }
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange ( tMatch.m_iWeight );
	}

	Filter_WeightRange ()
	{
		m_bUsesAttrs = false;
	}
};

// MVA
template < bool IS_MVA64 >
struct Filter_MVAValues: public IFilter_MVA, IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const DWORD * pMva, * pMvaMax;
		if ( !LoadMVA ( tMatch, &pMva, &pMvaMax ) )
			return false;

		return MvaEval ( pMva, pMvaMax );
	}

	bool MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const;
};


template<>
bool Filter_MVAValues<false>::MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const
{
	const SphAttr_t * pFilter = m_pValues;
	const SphAttr_t * pFilterMax = pFilter + m_iValueCount;

	const DWORD * L = pMva;
	const DWORD * R = pMvaMax - 1;
	for ( ; pFilter < pFilterMax; pFilter++ )
	{
		while ( L<=R )
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


template<>
bool Filter_MVAValues<true>::MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const
{
	const SphAttr_t * pFilter = m_pValues;
	const SphAttr_t * pFilterMax = pFilter + m_iValueCount;

	const int64_t * L = (const int64_t *)pMva;
	const int64_t * R = (const int64_t *)( pMvaMax - 2 );
	for ( ; pFilter < pFilterMax; pFilter++ )
	{
		int64_t uFilter = *pFilter;
		while ( L<=R )
		{
			const int64_t * pVal = L + (R - L) / 2;
			int64_t iMva = MVA_UPSIZE ( (const DWORD *)pVal );

			if ( uFilter > iMva )
				L = pVal + 1;
			else if ( uFilter < iMva )
				R = pVal - 1;
			else
				return true;
		}
		R = (const int64_t *)( pMvaMax - 2 );
	}
	return false;
}


template < bool IS_MVA64 >
struct Filter_MVARange: public IFilter_MVA, IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const DWORD * pMva, * pMvaMax;
		if ( !LoadMVA ( tMatch, &pMva, &pMvaMax ) )
			return false;

		return MvaEval ( pMva, pMvaMax );
	}

	bool MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const;
};


template<>
bool Filter_MVARange<false>::MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const
{
	const DWORD * L = pMva;
	const DWORD * R = pMvaMax - 1;

	while ( L<=R )
	{
		const DWORD * m = L + (R - L) / 2;
		if ( m_iMinValue > *m )
			L = m + 1;
		else if ( m_iMinValue < *m )
			R = m - 1;
		else
			return true;
	}
	if ( L==pMvaMax )
		return false;
	return *L<=m_iMaxValue;
}


template<>
bool Filter_MVARange<true>::MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const
{
	const int64_t * L = (const int64_t *)pMva;
	const int64_t * R = (const int64_t *)( pMvaMax - 2 );

	while ( L<=R )
	{
		const int64_t * pVal = L + (R - L) / 2;
		int64_t iMva = MVA_UPSIZE ( (const DWORD *)pVal );

		if ( m_iMinValue>iMva )
			L = pVal + 1;
		else if ( m_iMinValue < iMva )
			R = pVal - 1;
		else
			return true;
	}
	if ( L==(const int64_t *)pMvaMax )
		return false;

	int64_t iMvaL = MVA_UPSIZE ( (const DWORD *)L );
	return iMvaL<=m_iMaxValue;
}


// and

struct Filter_And2 : public ISphFilter
{
	ISphFilter * m_pArg1;
	ISphFilter * m_pArg2;

	explicit Filter_And2 ( ISphFilter * pArg1, ISphFilter * pArg2, bool bUsesAttrs )
		: m_pArg1 ( pArg1 )
		, m_pArg2 ( pArg2 )
	{
		m_bUsesAttrs = bUsesAttrs;
	}

	~Filter_And2 ()
	{
		SafeDelete ( m_pArg1 );
		SafeDelete ( m_pArg2 );
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return m_pArg1->Eval ( tMatch ) && m_pArg2->Eval ( tMatch );
	}

	virtual bool EvalBlock ( const DWORD * pMin, const DWORD * pMax ) const
	{
		return m_pArg1->EvalBlock ( pMin, pMax ) && m_pArg2->EvalBlock ( pMin, pMax );
	}

	virtual ISphFilter * Join ( ISphFilter * pFilter )
	{
		ISphFilter * pJoined = new Filter_And2 ( m_pArg2, pFilter, m_bUsesAttrs );
		m_pArg2 = pJoined;
		return this;
	}

	virtual void SetMVAStorage ( const DWORD * pMva )
	{
		m_pArg1->SetMVAStorage ( pMva );
		m_pArg2->SetMVAStorage ( pMva );
	}
};


struct Filter_And3 : public ISphFilter
{
	ISphFilter * m_pArg1;
	ISphFilter * m_pArg2;
	ISphFilter * m_pArg3;

	explicit Filter_And3 ( ISphFilter * pArg1, ISphFilter * pArg2, ISphFilter * pArg3, bool bUsesAttrs )
		: m_pArg1 ( pArg1 )
		, m_pArg2 ( pArg2 )
		, m_pArg3 ( pArg3 )
	{
		m_bUsesAttrs = bUsesAttrs;
	}

	~Filter_And3 ()
	{
		SafeDelete ( m_pArg1 );
		SafeDelete ( m_pArg2 );
		SafeDelete ( m_pArg3 );
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return m_pArg1->Eval ( tMatch ) && m_pArg2->Eval ( tMatch ) && m_pArg3->Eval ( tMatch );
	}

	virtual bool EvalBlock ( const DWORD * pMin, const DWORD * pMax ) const
	{
		return m_pArg1->EvalBlock ( pMin, pMax ) && m_pArg2->EvalBlock ( pMin, pMax ) && m_pArg3->EvalBlock ( pMin, pMax );
	}

	virtual ISphFilter * Join ( ISphFilter * pFilter )
	{
		ISphFilter * pJoined = new Filter_And2 ( m_pArg3, pFilter, m_bUsesAttrs );
		m_pArg3 = pJoined;
		return this;
	}

	virtual void SetMVAStorage ( const DWORD * pMva )
	{
		m_pArg1->SetMVAStorage ( pMva );
		m_pArg2->SetMVAStorage ( pMva );
		m_pArg3->SetMVAStorage ( pMva );
	}
};


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
		m_bUsesAttrs |= pFilter->UsesAttrs();
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		ARRAY_FOREACH ( i, m_dFilters )
			if ( !m_dFilters[i]->Eval ( tMatch ) )
				return false;
		return true;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		ARRAY_FOREACH ( i, m_dFilters )
			if ( !m_dFilters[i]->EvalBlock ( pMinDocinfo, pMaxDocinfo ) )
				return false;
		return true;
	}

	virtual ISphFilter * Join ( ISphFilter * pFilter )
	{
		Add ( pFilter );
		return this;
	}

	Filter_And ()
	{
		m_bUsesAttrs = false;
	}


	virtual void SetMVAStorage ( const DWORD * pMva )
	{
		ARRAY_FOREACH ( i, m_dFilters )
			m_dFilters[i]->SetMVAStorage ( pMva );
	}

	virtual ISphFilter * Optimize()
	{
		if ( m_dFilters.GetLength()==2 )
		{
			ISphFilter * pOpt = new Filter_And2 ( m_dFilters[0], m_dFilters[1], m_bUsesAttrs );
			m_dFilters.Reset();
			delete this;
			return pOpt;
		}
		if ( m_dFilters.GetLength()==3 )
		{
			ISphFilter * pOpt = new Filter_And3 ( m_dFilters[0], m_dFilters[1], m_dFilters[2], m_bUsesAttrs );
			m_dFilters.Reset();
			delete this;
			return pOpt;
		}
		return this;
	}
};

// not

struct Filter_Not: public ISphFilter
{
	ISphFilter * m_pFilter;

	explicit Filter_Not ( ISphFilter * pFilter )
		: m_pFilter ( pFilter )
	{
		assert ( pFilter );
		m_bUsesAttrs = pFilter->UsesAttrs();
	}

	~Filter_Not ()
	{
		SafeDelete ( m_pFilter );
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return !m_pFilter->Eval ( tMatch );
	}

	virtual bool EvalBlock ( const DWORD *, const DWORD * ) const
	{
		// if block passes through the filter we can't just negate the
		// result since it's imprecise at this point
		return true;
	}

	virtual void SetMVAStorage ( const DWORD * pMva )
	{
		m_pFilter->SetMVAStorage ( pMva );
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
	} else if ( sName=="@weight" )
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


static inline ISphFilter * ReportError ( CSphString & sError, const char * sMessage, ESphFilter eFilterType )
{
	CSphString sFilterName;
	switch ( eFilterType )
	{
		case SPH_FILTER_VALUES:			sFilterName = "intvalues"; break;
		case SPH_FILTER_RANGE:			sFilterName = "intrange"; break;
		case SPH_FILTER_FLOATRANGE:		sFilterName = "floatrange"; break;
		default:						sFilterName.SetSprintf ( "(filter-type-%d)", eFilterType ); break;
	}

	sError.SetSprintf ( sMessage, sFilterName.cstr() );
	return NULL;
}


static ISphFilter * CreateFilter ( ESphAttr eAttrType, ESphFilter eFilterType, int iNumValues,
	const CSphAttrLocator & tLoc, CSphString & sError )
{
	// MVA
	if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET )
	{
		switch ( eFilterType )
		{
		case SPH_FILTER_VALUES:
			if ( eAttrType==SPH_ATTR_INT64SET )
				return new Filter_MVAValues<true>();
			else
				return new Filter_MVAValues<false>();

		case SPH_FILTER_RANGE:
			if ( eAttrType==SPH_ATTR_INT64SET )
				return new Filter_MVARange<true>();
			else
				return new Filter_MVARange<false>();
		default:				return ReportError ( sError, "unsupported filter type '%s' on MVA column", eFilterType );
		}
	}

	// float
	if ( eAttrType==SPH_ATTR_FLOAT )
	{
		if ( eFilterType==SPH_FILTER_FLOATRANGE )
			return new Filter_FloatRange;

		return ReportError ( sError, "unsupported filter type '%s' on float column", eFilterType );
	}

	// non-float, non-MVA
	switch ( eFilterType )
	{
		case SPH_FILTER_VALUES:
			if ( iNumValues==1 && ( eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_BIGINT ) )
			{
				if ( eAttrType==SPH_ATTR_INTEGER && !tLoc.m_bDynamic && tLoc.m_iBitCount==32 && ( tLoc.m_iBitOffset % 32 )==0 )
					return new Filter_SingleValueStatic32();
				else
					return new Filter_SingleValue();
			} else
			{
				return new Filter_Values();
			}
		case SPH_FILTER_RANGE:	return new Filter_Range();
		default:				return ReportError ( sError, "unsupported filter type '%s' on int column", eFilterType );
	}
}

ISphFilter * sphCreateFilter ( const CSphFilterSettings & tSettings, const CSphSchema & tSchema,
	const DWORD * pMvaPool, CSphString & sError )
{
	ISphFilter * pFilter = 0;

	// try to create filter on special attribute
	const CSphString & sAttrName = tSettings.m_sAttrName;
	if ( sAttrName.Begins("@") )
		pFilter = CreateSpecialFilter ( sAttrName, tSettings.m_eType );

	// fetch column info
	const CSphColumnInfo * pAttr = NULL;
	const int iAttr = tSchema.GetAttrIndex ( sAttrName.cstr() );
	if ( iAttr<0 )
	{
		if ( !pFilter ) // might be special
		{
			sError.SetSprintf ( "no such filter attribute '%s'", sAttrName.cstr() );
			return NULL; // no such attribute
		}
	} else
	{
		assert ( !pFilter );

		pAttr = &tSchema.GetAttr(iAttr);
		pFilter = CreateFilter ( pAttr->m_eAttrType, tSettings.m_eType, tSettings.GetNumValues(), pAttr->m_tLocator, sError );
	}

	// fill filter's properties
	if ( pFilter )
	{
		if ( pAttr )
			pFilter->SetLocator ( pAttr->m_tLocator );

		pFilter->SetRange ( tSettings.m_iMinValue, tSettings.m_iMaxValue );
		pFilter->SetRangeFloat ( tSettings.m_fMinValue, tSettings.m_fMaxValue );
		pFilter->SetMVAStorage ( pMvaPool );

		if ( tSettings.GetNumValues() > 0 )
		{
			pFilter->SetValues ( tSettings.GetValueArray(), tSettings.GetNumValues() );

#ifndef NDEBUG
			// check that the values are actually sorted
			const SphAttr_t * pValues = tSettings.GetValueArray();
			int iValues = tSettings.GetNumValues ();

			for ( int i=1; i<iValues; i++ )
				assert ( pValues[i]>=pValues[i-1] );
#endif
		}

		if ( tSettings.m_bExclude )
			pFilter = new Filter_Not ( pFilter );
	}

	return pFilter;
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
