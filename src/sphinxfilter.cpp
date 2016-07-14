//
// $Id$
//

//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinxfilter.h"
#include "sphinxint.h"
#include "sphinxjson.h"

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
struct IFilter_Values : virtual ISphFilter
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

	inline SphAttr_t GetValue ( int iIndex ) const
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


template<bool HAS_EQUAL>
bool EvalRange ( SphAttr_t uValue, SphAttr_t iMin, SphAttr_t iMax )
{
	if_const ( HAS_EQUAL )
		return uValue>=iMin && uValue<=iMax;
	else
		return uValue>iMin && uValue<iMax;
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


template < bool HAS_EQUAL >
struct Filter_Range: public IFilter_Attr, IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange<HAS_EQUAL> ( tMatch.GetAttr ( m_tLocator ), m_iMinValue, m_iMaxValue );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		SphAttr_t uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS ( pMinDocinfo ), m_tLocator );
		SphAttr_t uBlockMax = sphGetRowAttr ( DOCINFO2ATTRS ( pMaxDocinfo ), m_tLocator );
		// not-reject
		if_const ( HAS_EQUAL )
			return ( m_iMaxValue>=uBlockMin && m_iMinValue<=uBlockMax );
		else
			return ( m_iMaxValue>uBlockMin && m_iMinValue<uBlockMax );
	}
};

// float

template < bool HAS_EQUAL >
struct Filter_FloatRange : public IFilter_Attr
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
		if_const ( HAS_EQUAL )
			return fValue>=m_fMinValue && fValue<=m_fMaxValue;
		else
			return fValue>m_fMinValue && fValue<m_fMaxValue;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		float fBlockMin = sphDW2F ( (DWORD)sphGetRowAttr ( DOCINFO2ATTRS ( pMinDocinfo ), m_tLocator ) );
		float fBlockMax = sphDW2F ( (DWORD)sphGetRowAttr ( DOCINFO2ATTRS ( pMaxDocinfo ), m_tLocator ) );
		// not-reject
		if_const ( HAS_EQUAL )
			return ( m_fMaxValue>=fBlockMin && m_fMinValue<=fBlockMax );
		else
			return ( m_fMaxValue>fBlockMin && m_fMinValue<fBlockMax );
	}
};

// id

struct Filter_IdValues: public IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalValues ( tMatch.m_uDocID );
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

template < bool HAS_EQUAL >
struct Filter_IdRange: public IFilter_Range
{
	virtual void SetRange ( SphAttr_t tMin, SphAttr_t tMax )
	{
		m_iMinValue = (SphDocID_t)Max ( (SphDocID_t)tMin, 0u );
		m_iMaxValue = tMax;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const SphDocID_t uID = tMatch.m_uDocID;
		if_const ( HAS_EQUAL )
			return uID>=(SphDocID_t)m_iMinValue && uID<=(SphDocID_t)m_iMaxValue;
		else
			return uID>(SphDocID_t)m_iMinValue && uID<(SphDocID_t)m_iMaxValue;
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		const SphDocID_t uBlockMin = DOCINFO2ID ( pMinDocinfo );
		const SphDocID_t uBlockMax = DOCINFO2ID ( pMaxDocinfo );
		// not-reject
		if_const ( HAS_EQUAL )
			return ( (SphDocID_t)m_iMaxValue>=uBlockMin && (SphDocID_t)m_iMinValue<=uBlockMax );
		else
			return ( (SphDocID_t)m_iMaxValue>uBlockMin && (SphDocID_t)m_iMinValue<uBlockMax );
	}

	Filter_IdRange ()
	{
		m_bUsesAttrs = false;
	}
};

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

template < bool HAS_EQUAL >
struct Filter_WeightRange: public IFilter_Range
{
	virtual bool IsEarly () { return false; }
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange<HAS_EQUAL> ( tMatch.m_iWeight, m_iMinValue, m_iMaxValue );
	}

	Filter_WeightRange ()
	{
		m_bUsesAttrs = false;
	}
};

//////////////////////////////////////////////////////////////////////////
// MVA
//////////////////////////////////////////////////////////////////////////

struct IFilter_MVA: virtual IFilter_Attr
{
	const DWORD *	m_pMvaStorage;
	bool			m_bArenaProhibit;

	IFilter_MVA ()
		: m_pMvaStorage ( NULL )
		, m_bArenaProhibit ( false )
	{}

	virtual void SetMVAStorage ( const DWORD * pMva, bool bArenaProhibit )
	{
		m_pMvaStorage = pMva;
		m_bArenaProhibit = bArenaProhibit;
	}

	inline bool LoadMVA ( const CSphMatch & tMatch, const DWORD ** pMva, const DWORD ** pMvaMax ) const
	{
		assert ( m_pMvaStorage );

		*pMva = tMatch.GetAttrMVA ( m_tLocator, m_pMvaStorage, m_bArenaProhibit );
		if ( !*pMva )
			return false;

		*pMvaMax = *pMva + (**pMva) + 1;
		(*pMva)++;
		return true;
	}
};


template < typename T >
struct Filter_MVAValues_Any : public IFilter_MVA, IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const DWORD * pMva, * pMvaMax;
		if ( !LoadMVA ( tMatch, &pMva, &pMvaMax ) )
			return false;

		return MvaEval ( pMva, pMvaMax );
	}

	bool MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const
	{
		const SphAttr_t * pFilter = m_pValues;
		const SphAttr_t * pFilterMax = pFilter + m_iValueCount;

		const T * L = (const T *)pMva;
		const T * R = (const T *)pMvaMax;
		R--;
		for ( ; pFilter < pFilterMax; pFilter++ )
		{
			while ( L<=R )
			{
				const T * pVal = L + (R - L) / 2;
				if ( *pFilter > *pVal )
					L = pVal + 1;
				else if ( *pFilter < *pVal )
					R = pVal - 1;
				else
					return true;
			}
			R = (const T *)pMvaMax;
			R--;
		}
		return false;
	}
};


template < typename T >
struct Filter_MVAValues_All : public IFilter_MVA, IFilter_Values
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const DWORD * pMva, * pMvaMax;
		if ( !LoadMVA ( tMatch, &pMva, &pMvaMax ) )
			return false;

		return MvaEval ( pMva, pMvaMax );
	}

	bool MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const
	{
		const T * L = (const T *)pMva;
		const T * R = (const T *)pMvaMax;

		for ( const T * pVal=L; pVal<R; pVal++ )
		{
			const SphAttr_t iCheck = *pVal;
			if ( !sphBinarySearch ( m_pValues, m_pValues + m_iValueCount - 1, iCheck ) )
				return false;
		}
		return true;
	}
};


static DWORD GetMvaValue ( const DWORD * pVal )
{
	return *pVal;
}

static int64_t GetMvaValue ( const int64_t * pVal )
{
	return MVA_UPSIZE ( (const DWORD *)pVal );
}

template < typename T, bool HAS_EQUAL >
struct Filter_MVARange_Any : public IFilter_MVA, IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const DWORD * pMva, * pMvaMax;
		if ( !LoadMVA ( tMatch, &pMva, &pMvaMax ) )
			return false;

		return MvaEval ( pMva, pMvaMax );
	}

	bool MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const
	{
		const T * pEnd = (const T *)pMvaMax;
		const T * L = (const T *)pMva;
		const T * R = pEnd - 1;

		while ( L<=R )
		{
			const T * pVal = L + (R - L) / 2;
			T iMva = GetMvaValue ( pVal );

			if ( m_iMinValue>iMva )
				L = pVal + 1;
			else if ( m_iMinValue<iMva )
				R = pVal - 1;
			else
				return ( HAS_EQUAL || pVal+1<pEnd );
		}
		if ( L==pEnd )
			return false;

		T iMvaL = GetMvaValue ( L );
		if_const ( HAS_EQUAL )
			return iMvaL<=m_iMaxValue;
		else
			return iMvaL<m_iMaxValue;
	}
};


template < typename T, bool HAS_EQUAL >
struct Filter_MVARange_All : public IFilter_MVA, IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const DWORD * pMva, * pMvaMax;
		if ( !LoadMVA ( tMatch, &pMva, &pMvaMax ) )
			return false;

		return MvaEval ( pMva, pMvaMax );
	}

	bool MvaEval ( const DWORD * pMva, const DWORD * pMvaMax ) const
	{
		const T * L = (const T *)pMva;
		const T * pEnd = (const T *)pMvaMax;
		const T * R = pEnd - 1;

		if_const ( HAS_EQUAL )
			return ( m_iMinValue<=*L && *R<=m_iMaxValue );
		else
			return ( m_iMinValue<*L && *R<m_iMaxValue );
	}
};


class FilterString_c : public IFilter_Attr
{
private:
	CSphFixedVector<BYTE>	m_dVal;
	bool					m_bEq;

protected:
	SphStringCmp_fn			m_fnStrCmp;
	const BYTE *			m_pStringBase;
	bool					m_bPacked;

public:
	FilterString_c ( ESphCollation eCollation, ESphAttr eType, bool bEq )
		: m_dVal ( 0 )
		, m_bEq ( bEq )
		, m_pStringBase ( NULL )
	{
		assert ( eType==SPH_ATTR_STRING || eType==SPH_ATTR_STRINGPTR );
		m_bPacked = ( eType==SPH_ATTR_STRING );

		switch ( eCollation )
		{
		case SPH_COLLATION_LIBC_CI:
			m_fnStrCmp = sphCollateLibcCI;
			break;
		case SPH_COLLATION_LIBC_CS:
			m_fnStrCmp = sphCollateLibcCS;
			break;
		case SPH_COLLATION_UTF8_GENERAL_CI:
			m_fnStrCmp = sphCollateUtf8GeneralCI;
			break;
		case SPH_COLLATION_BINARY:
			m_fnStrCmp = sphCollateBinary;
			break;
		}
	}

	virtual void SetRefString ( const CSphString * pRef, int iCount )
	{
		assert ( iCount<2 );
		const char * sVal = pRef ? pRef->cstr() : NULL;
		int iLen = pRef ? pRef->Length() : 0;
		if ( m_bPacked )
		{
			m_dVal.Reset ( iLen+4 );
			int iPacked = sphPackStrlen ( m_dVal.Begin(), iLen );
			memcpy ( m_dVal.Begin()+iPacked, sVal, iLen );
		} else
		{
			m_dVal.Reset ( iLen+1 );
			memcpy ( m_dVal.Begin(), sVal, iLen );
			m_dVal[iLen] = '\0';
		}
	}

	virtual void SetStringStorage ( const BYTE * pStrings )
	{
		m_pStringBase = pStrings;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		if ( m_bPacked && !m_pStringBase )
			return !m_bEq;

		SphAttr_t uVal = tMatch.GetAttr ( m_tLocator );

		const BYTE * pStr;
		if ( !uVal )
			pStr = (const BYTE*)"\0"; // 2 bytes, for packed strings
		else if ( m_bPacked )
			pStr = m_pStringBase + uVal;
		else
			pStr = (const BYTE *)uVal;

		bool bEq = ( m_fnStrCmp ( pStr, m_dVal.Begin(), m_bPacked )==0 );
		return ( m_bEq==bEq );
	}
};


struct Filter_StringValues_c: FilterString_c
{
	CSphVector<BYTE>		m_dVal;
	CSphVector<int>			m_dOfs;

	Filter_StringValues_c ( ESphCollation eCollation, ESphAttr eType )
		: FilterString_c ( eCollation, eType, false )
	{}

	virtual void SetRefString ( const CSphString * pRef, int iCount )
	{
		assert ( pRef );
		assert ( iCount>0 );

		int iOfs = 0;
		for ( int i=0; i<iCount; i++ )
		{
			const char * sRef = ( pRef + i )->cstr();
			int iLen = ( pRef + i )->Length();

			if ( m_bPacked )
			{
				m_dVal.Resize ( iOfs+iLen+4 );
				int iPacked = sphPackStrlen ( m_dVal.Begin() + iOfs, iLen );
				memcpy ( m_dVal.Begin() + iOfs + iPacked, sRef, iLen );
			} else
			{
				m_dVal.Resize ( iOfs+iLen+1 );
				memcpy ( m_dVal.Begin() + iOfs, sRef, iLen );
				m_dVal[iOfs+iLen] = '\0';
			}

			m_dOfs.Add ( iOfs );
			iOfs = m_dVal.GetLength();
		}
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		SphAttr_t uVal = tMatch.GetAttr ( m_tLocator );

		const BYTE * pStr;
		if ( !uVal )
			pStr = (const BYTE*)"\0";
		else if ( m_bPacked )
			pStr = m_pStringBase + uVal;
		else
			pStr = (const BYTE *)uVal;

		ARRAY_FOREACH ( i, m_dOfs )
			if ( m_fnStrCmp ( pStr, m_dVal.Begin() + m_dOfs[i], m_bPacked )==0 )
				return true;

		return false;
	}
};


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

	virtual void SetMVAStorage ( const DWORD * pMva, bool bArenaProhibit )
	{
		m_pArg1->SetMVAStorage ( pMva, bArenaProhibit );
		m_pArg2->SetMVAStorage ( pMva, bArenaProhibit );
	}

	virtual void SetStringStorage ( const BYTE * pStrings )
	{
		m_pArg1->SetStringStorage ( pStrings );
		m_pArg2->SetStringStorage ( pStrings );
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

	virtual void SetMVAStorage ( const DWORD * pMva, bool bArenaProhibit )
	{
		m_pArg1->SetMVAStorage ( pMva, bArenaProhibit );
		m_pArg2->SetMVAStorage ( pMva, bArenaProhibit );
		m_pArg3->SetMVAStorage ( pMva, bArenaProhibit );
	}

	virtual void SetStringStorage ( const BYTE * pStrings )
	{
		m_pArg1->SetStringStorage ( pStrings );
		m_pArg2->SetStringStorage ( pStrings );
		m_pArg3->SetStringStorage ( pStrings );
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


	virtual void SetMVAStorage ( const DWORD * pMva, bool bArenaProhibit )
	{
		ARRAY_FOREACH ( i, m_dFilters )
			m_dFilters[i]->SetMVAStorage ( pMva, bArenaProhibit );
	}

	virtual void SetStringStorage ( const BYTE * pStrings )
	{
		ARRAY_FOREACH ( i, m_dFilters )
			m_dFilters[i]->SetStringStorage ( pStrings );
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

	virtual void SetMVAStorage ( const DWORD * pMva, bool bArenaProhibit )
	{
		m_pFilter->SetMVAStorage ( pMva, bArenaProhibit );
	}

	virtual void SetStringStorage ( const BYTE * pStrings )
	{
		m_pFilter->SetStringStorage ( pStrings );
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

static inline ISphFilter * ReportError ( CSphString & sError, const char * sMessage, ESphFilter eFilterType )
{
	CSphString sFilterName;
	switch ( eFilterType )
	{
		case SPH_FILTER_VALUES:			sFilterName = "intvalues"; break;
		case SPH_FILTER_RANGE:			sFilterName = "intrange"; break;
		case SPH_FILTER_FLOATRANGE:		sFilterName = "floatrange"; break;
		case SPH_FILTER_STRING:			sFilterName = "string"; break;
		case SPH_FILTER_NULL:			sFilterName = "null"; break;
		default:						sFilterName.SetSprintf ( "(filter-type-%d)", eFilterType ); break;
	}

	sError.SetSprintf ( sMessage, sFilterName.cstr() );
	return NULL;
}


static ISphFilter * CreateSpecialFilter ( const CSphString & sName, ESphFilter eFilterType, bool bHasEqual, CSphString & sError )
{
	if ( sName=="@id" )
	{
		switch ( eFilterType )
		{
		case SPH_FILTER_VALUES:	return new Filter_IdValues;
		case SPH_FILTER_RANGE:
			if ( bHasEqual )
				return new Filter_IdRange<true>;
			else
				return new Filter_IdRange<false>;
		default:
			return ReportError ( sError, "unsupported filter type '%s' on @id", eFilterType );
		}
	} else if ( sName=="@weight" )
	{
		switch ( eFilterType )
		{
		case SPH_FILTER_VALUES:	return new Filter_WeightValues;
		case SPH_FILTER_RANGE:
			if ( bHasEqual )
				return new Filter_WeightRange<true>;
			else
				return new Filter_WeightRange<false>;
		default:
			return ReportError ( sError, "unsupported filter type '%s' on @weight", eFilterType );
		}
	}

	return NULL;
}


static ISphFilter * CreateFilter ( ESphAttr eAttrType, ESphFilter eFilterType, ESphMvaFunc eMvaFunc, int iNumValues,
	const CSphAttrLocator & tLoc, CSphString & sError, CSphString & sWarning, bool bHasEqual, ESphCollation eCollation )
{
	// MVA
	if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET )
	{
		if (!( eFilterType==SPH_FILTER_VALUES || eFilterType==SPH_FILTER_RANGE ))
			return ReportError ( sError, "unsupported filter type '%s' on MVA column", eFilterType );

		if ( eMvaFunc==SPH_MVAFUNC_NONE )
			sWarning.SetSprintf ( "suggest an explicit ANY()/ALL() around a filter on MVA column" );

		bool bWide = ( eAttrType==SPH_ATTR_INT64SET );
		bool bRange = ( eFilterType==SPH_FILTER_RANGE );
		bool bAll = ( eMvaFunc==SPH_MVAFUNC_ALL );
		int iIndex = bWide*8 + bRange*4 + bAll*2 + bHasEqual;

		switch ( iIndex )
		{
			case 0:		return new Filter_MVAValues_Any<DWORD>();
			case 1:		return new Filter_MVAValues_Any<DWORD>();
			case 2:		return new Filter_MVAValues_All<DWORD>();
			case 3:		return new Filter_MVAValues_All<DWORD>();

			case 4:		return new Filter_MVARange_Any<DWORD, false>();
			case 5:		return new Filter_MVARange_Any<DWORD, true>();
			case 6:		return new Filter_MVARange_All<DWORD, false>();
			case 7:		return new Filter_MVARange_All<DWORD, true>();

			case 8:		return new Filter_MVAValues_Any<int64_t>();
			case 9:		return new Filter_MVAValues_Any<int64_t>();
			case 10:	return new Filter_MVAValues_All<int64_t>();
			case 11:	return new Filter_MVAValues_All<int64_t>();

			case 12:	return new Filter_MVARange_Any<int64_t, false>();
			case 13:	return new Filter_MVARange_Any<int64_t, true>();
			case 14:	return new Filter_MVARange_All<int64_t, false>();
			case 15:	return new Filter_MVARange_All<int64_t, true>();
		}
	}

	// float
	if ( eAttrType==SPH_ATTR_FLOAT )
	{
		if ( eFilterType==SPH_FILTER_FLOATRANGE || eFilterType==SPH_FILTER_RANGE )
		{
			if ( bHasEqual )
				return new Filter_FloatRange<true>;
			else
				return new Filter_FloatRange<false>;
		}

		return ReportError ( sError, "unsupported filter type '%s' on float column", eFilterType );
	}

	if ( eAttrType==SPH_ATTR_STRING || eAttrType==SPH_ATTR_STRINGPTR )
	{
		if ( eFilterType==SPH_FILTER_STRING_LIST )
			return new Filter_StringValues_c ( eCollation, eAttrType );
		else
			return new FilterString_c ( eCollation, eAttrType, bHasEqual );
	}

	// non-float, non-MVA
	switch ( eFilterType )
	{
		case SPH_FILTER_VALUES:
			if ( iNumValues==1 && ( eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_BIGINT || eAttrType==SPH_ATTR_TOKENCOUNT ) )
			{
				if ( ( eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_TOKENCOUNT ) && !tLoc.m_bDynamic && tLoc.m_iBitCount==32 && ( tLoc.m_iBitOffset % 32 )==0 )
					return new Filter_SingleValueStatic32();
				else
					return new Filter_SingleValue();
			} else
			{
				return new Filter_Values();
			}
		case SPH_FILTER_RANGE:
			if ( bHasEqual )
				return new Filter_Range<true>();
			else
				return new Filter_Range<false>();
		default:				return ReportError ( sError, "unsupported filter type '%s' on int column", eFilterType );
	}
}

//////////////////////////////////////////////////////////////////////////
// JSON STUFF
//////////////////////////////////////////////////////////////////////////

/// i can not seem to handle multiple inheritance well
template<typename BASE>
class JsonFilter_c : public BASE
{
protected:
	const BYTE *				m_pStrings;
	CSphRefcountedPtr<ISphExpr>	m_pExpr;	// OPTIMIZE? make a quicker version for simple static accesses like json.key?

public:
	explicit JsonFilter_c ( ISphExpr * pExpr )
		: m_pStrings ( NULL )
		, m_pExpr ( pExpr )
	{}

	virtual void SetStringStorage ( const BYTE * pStrings )
	{
		m_pStrings = pStrings;
		if ( m_pExpr.Ptr() )
			m_pExpr->Command ( SPH_EXPR_SET_STRING_POOL, (void*)pStrings );
	}

	ESphJsonType GetKey ( const BYTE ** ppKey, const CSphMatch & tMatch ) const
	{
		assert ( ppKey );
		if ( !m_pExpr )
			return JSON_EOF;
		uint64_t uValue = m_pExpr->Int64Eval ( tMatch );
		if ( uValue==0 ) // either no data or invalid path
			return JSON_NULL;
		const BYTE * pValue = m_pStrings + ( uValue & 0xffffffff );
		ESphJsonType eRes = (ESphJsonType)( uValue >> 32 );
		*ppKey = pValue;
		return eRes;
	}
};


class JsonFilterValues_c : public JsonFilter_c<IFilter_Values>
{
public:
	explicit JsonFilterValues_c ( ISphExpr * pExpr )
		: JsonFilter_c<IFilter_Values> ( pExpr )
	{}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		assert ( this->m_pExpr.Ptr()!=NULL );

		uint64_t uValue = m_pExpr->Int64Eval ( tMatch );
		const BYTE * pValue = m_pStrings + ( uValue & 0xffffffff );
		switch ( uValue>>32 ) // so that 0 from Eval() gets handled as eof
		{
			case JSON_INT32:	return EvalValues ( sphGetDword ( pValue ) );
			case JSON_INT64:	return EvalValues ( sphJsonLoadBigint ( &pValue ) );
			case JSON_DOUBLE:	return EvalValues ( (SphAttr_t)sphQW2D ( sphJsonLoadBigint ( &pValue ) ) );
			case JSON_TRUE:		return !EvalValues (0);
			case JSON_FALSE:
			case JSON_NULL:		return EvalValues (0);
			default:			return false;
		}
	}
};

template < bool HAS_EQUALS >
class JsonFilterRange_c : public JsonFilter_c<IFilter_Range>
{
public:
	explicit JsonFilterRange_c ( ISphExpr * pExpr )
		: JsonFilter_c<IFilter_Range> ( pExpr )
	{}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const BYTE * pValue;
		ESphJsonType eRes = GetKey ( &pValue, tMatch );
		switch ( eRes )
		{
			case JSON_INT32:
				return EvalRange<HAS_EQUALS> ( sphJsonLoadInt ( &pValue ), m_iMinValue, m_iMaxValue );
			case JSON_INT64:
				return EvalRange<HAS_EQUALS> ( sphJsonLoadBigint ( &pValue ), m_iMinValue, m_iMaxValue );
			case JSON_DOUBLE:
			{
				double fValue = sphQW2D ( sphJsonLoadBigint ( &pValue ) );
				if_const ( HAS_EQUALS )
					return fValue>=m_iMinValue && fValue<=m_iMaxValue;
				else
					return fValue>m_iMinValue && fValue<m_iMaxValue;
			}
			case JSON_TRUE:		return !EvalRange<HAS_EQUALS>(0, m_iMinValue, m_iMaxValue);
			case JSON_FALSE:
			case JSON_NULL:		return EvalRange<HAS_EQUALS>(0, m_iMinValue, m_iMaxValue);
			default:
				return false;
		}
	}
};


template < bool HAS_EQUALS >
class JsonFilterFloatRange_c : public JsonFilter_c<IFilter_Range>
{
public:
	explicit JsonFilterFloatRange_c ( ISphExpr * pExpr )
		: JsonFilter_c<IFilter_Range> ( pExpr )
	{}

	float m_fMinValue;
	float m_fMaxValue;

	virtual void SetRangeFloat ( float fMin, float fMax )
	{
		m_fMinValue = fMin;
		m_fMaxValue = fMax;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const BYTE * pValue;
		float fValue;
		ESphJsonType eRes = GetKey ( &pValue, tMatch );
		switch ( eRes )
		{
			case JSON_INT32:
				fValue = (float)sphJsonLoadInt ( &pValue );
				break;
			case JSON_INT64:
				fValue = (float)sphJsonLoadBigint ( &pValue );
				break;
			case JSON_DOUBLE:
				fValue = (float)sphQW2D ( sphJsonLoadBigint ( &pValue ) );
				break;
			default:
				return false;
		}
		if_const ( HAS_EQUALS )
			return fValue>=m_fMinValue && fValue<=m_fMaxValue;
		else
			return fValue>m_fMinValue && fValue<m_fMaxValue;
	}
};


class JsonFilterString_c : public JsonFilter_c<ISphFilter>
{
protected:
	CSphFixedVector<BYTE>	m_dVal;
	SphStringCmp_fn			m_fnStrCmp;
	bool					m_bEq;

public:
	explicit JsonFilterString_c ( ISphExpr * pExpr, ESphCollation eCollation, bool bEq )
		: JsonFilter_c<ISphFilter> ( pExpr )
		, m_dVal ( 0 )
		, m_bEq ( bEq )
	{
		switch ( eCollation )
		{
		case SPH_COLLATION_LIBC_CI:
			m_fnStrCmp = sphCollateLibcCI;
			break;
		case SPH_COLLATION_LIBC_CS:
			m_fnStrCmp = sphCollateLibcCS;
			break;
		case SPH_COLLATION_UTF8_GENERAL_CI:
			m_fnStrCmp = sphCollateUtf8GeneralCI;
			break;
		case SPH_COLLATION_BINARY:
			m_fnStrCmp = sphCollateBinary;
			break;
		}
	}

	virtual void SetRefString ( const CSphString * pRef, int iCount )
	{
		assert ( pRef && iCount==1 );
		int iLen = pRef->Length();
		m_dVal.Reset ( iLen+4 );
		int iPacked = sphPackStrlen ( m_dVal.Begin(), iLen );
		memcpy ( m_dVal.Begin()+iPacked, pRef->cstr(), iLen );
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const BYTE * pValue;
		ESphJsonType eRes = GetKey ( &pValue, tMatch );
		if ( eRes!=JSON_STRING )
			return !m_bEq;

		bool bEq = ( m_fnStrCmp ( pValue, m_dVal.Begin(), true )==0 );
		return ( m_bEq==bEq );
	}
};


class JsonFilterNull_c : public JsonFilter_c<ISphFilter>
{
protected:
	bool m_bEquals;
	CSphAttrLocator m_tLoc;

public:
	JsonFilterNull_c ( ISphExpr * pExpr, bool bEquals )
		: JsonFilter_c<ISphFilter> ( pExpr )
		, m_bEquals ( bEquals )
	{}

	virtual void SetLocator ( const CSphAttrLocator & tLocator )
	{
		m_tLoc = tLocator;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		assert ( m_pStrings );

		if ( !m_pExpr ) // regular attribute? check blob size
		{
			const BYTE * pStr = NULL;
			DWORD uOffset = (DWORD) tMatch.GetAttr ( m_tLoc );
			if ( uOffset )
				sphUnpackStr ( m_pStrings+uOffset, &pStr );
			return m_bEquals ^ ( pStr!=NULL );
		}

		const BYTE * pValue;
		ESphJsonType eRes = GetKey ( &pValue, tMatch );
		return m_bEquals ^ ( eRes!=JSON_NULL );
	}
};


static ISphFilter * CreateFilterJson ( const CSphColumnInfo * DEBUGARG(pAttr), ISphExpr * pExpr,
	ESphFilter eType, bool bRangeEq, CSphString & sError, ESphCollation eCollation )
{
	assert ( pAttr );

	switch ( eType )
	{
		case SPH_FILTER_VALUES:
			return new JsonFilterValues_c ( pExpr );
		case SPH_FILTER_FLOATRANGE:
			if ( bRangeEq )
				return new JsonFilterFloatRange_c<true> ( pExpr );
			else
				return new JsonFilterFloatRange_c<false> ( pExpr );
		case SPH_FILTER_RANGE:
			if ( bRangeEq )
				return new JsonFilterRange_c<true> ( pExpr );
			else
				return new JsonFilterRange_c<false> ( pExpr );
		case SPH_FILTER_STRING:
			return new JsonFilterString_c ( pExpr, eCollation, bRangeEq );
		case SPH_FILTER_NULL:
			return new JsonFilterNull_c ( pExpr, bRangeEq );
		default:
			sError = "this filter type on JSON subfields is not implemented yet";
			return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// KillList STUFF
//////////////////////////////////////////////////////////////////////////

static int g_iKillMerge = 128;

struct Filter_KillList : public ISphFilter
{
	KillListVector			m_dExt;
	CSphVector<SphDocID_t>	m_dMerged;

	explicit Filter_KillList ( const KillListVector & dKillList )
	{
		m_bUsesAttrs = false;
		m_dExt = dKillList;

		int iMerged = 0;
		if ( m_dExt.GetLength()>1 )
		{
			ARRAY_FOREACH ( i, m_dExt )
			{
				iMerged += ( m_dExt[i].m_iLen<=g_iKillMerge ? m_dExt[i].m_iLen : 0 );
			}
			if ( iMerged>0 )
			{
				m_dMerged.Reserve ( iMerged );
				ARRAY_FOREACH ( i, m_dExt )
				{
					if ( m_dExt[i].m_iLen>g_iKillMerge )
						continue;

					int iOff = m_dMerged.GetLength();
					m_dMerged.Resize ( iOff + m_dExt[i].m_iLen );
					memcpy ( m_dMerged.Begin()+iOff, m_dExt[i].m_pBegin, sizeof ( m_dMerged[0] ) * m_dExt[i].m_iLen );
					m_dExt.RemoveFast ( i );
					i--;
				}
			}
			m_dMerged.Uniq();
		}
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		if ( !m_dMerged.GetLength() && !m_dExt.GetLength() )
			return true;

		if ( m_dMerged.BinarySearch ( tMatch.m_uDocID )!=NULL )
			return false;

		ARRAY_FOREACH ( i, m_dExt )
		{
			if ( sphBinarySearch ( m_dExt[i].m_pBegin, m_dExt[i].m_pBegin + m_dExt[i].m_iLen - 1, tMatch.m_uDocID )!=NULL )
				return false;
		}

		return true;
	}
};


//////////////////////////////////////////////////////////////////////////
// PUBLIC FACING INTERFACE
//////////////////////////////////////////////////////////////////////////

static ISphFilter * CreateFilter ( const CSphFilterSettings & tSettings, const CSphString & sAttrName, const ISphSchema & tSchema, const DWORD * pMvaPool, const BYTE * pStrings,
	CSphString & sError, CSphString & sWarning, bool bHaving, ESphCollation eCollation, bool bArenaProhibit )
{
	ISphFilter * pFilter = NULL;
	const CSphColumnInfo * pAttr = NULL;

	// try to create a filter on a special attribute
	if ( sAttrName.Begins("@") && !bHaving && ( sAttrName=="@groupby" || sAttrName=="@count" || sAttrName=="@distinct" ) )
	{
		sError.SetSprintf ( "unsupported filter column '%s'", sAttrName.cstr() );
		return NULL;
	}

	if ( sAttrName.Begins("@") )
	{
		pFilter = CreateSpecialFilter ( sAttrName, tSettings.m_eType, tSettings.m_bHasEqual, sError );
		if ( !pFilter && !sError.IsEmpty() )
			return NULL;
	}

	// try to create a filter on a JSON attribute
	CSphString sJsonCol, sJsonKey;
	if ( !pFilter && sphJsonNameSplit ( sAttrName.cstr(), &sJsonCol, &sJsonKey ) )
	{
		pAttr = tSchema.GetAttr ( sJsonCol.cstr() );
		if ( !pAttr )
		{
			sError.SetSprintf ( "no such attribute '%s'", sJsonCol.cstr() );
			return NULL;
		}

		if ( pAttr->m_eAttrType!=SPH_ATTR_JSON )
		{
			sError.SetSprintf ( "attribute '%s' does not have subfields (must be sql_attr_json)", sJsonCol.cstr() );
			return NULL;
		}

		// fastpath for simple cases like j.key1 is handled in the expression
		// combined access/filter nodes are only marginally faster (eg 17.4 msec vs 18.5 msec on 457K rows)
		ISphExpr * pExpr = sphExprParse ( sAttrName.cstr(), tSchema, NULL, NULL, sError, NULL, eCollation );
		pFilter = CreateFilterJson ( pAttr, pExpr, tSettings.m_eType, tSettings.m_bHasEqual, sError, eCollation );

		if ( !pFilter )
			return NULL;
	}

	// try to lookup a regular attribute
	ESphFilter eType = tSettings.m_eType;
	float fMin = tSettings.m_fMinValue;
	float fMax = tSettings.m_fMaxValue;

	if ( !pFilter )
	{
		const int iAttr = tSchema.GetAttrIndex ( sAttrName.cstr() );
		if ( iAttr<0 )
		{
			sError.SetSprintf ( "no such filter attribute '%s'", sAttrName.cstr() );
			return NULL; // no such attribute
		} else
		{
			pAttr = &tSchema.GetAttr(iAttr);
			if ( !bHaving && pAttr->m_eAggrFunc!=SPH_AGGR_NONE )
			{
				sError.SetSprintf ( "unsupported filter '%s' on aggregate column", sAttrName.cstr() );
				return NULL;
			}

			if ( pAttr->m_eAttrType==SPH_ATTR_JSON || pAttr->m_eAttrType==SPH_ATTR_JSON_FIELD )
			{
				if ( pAttr->m_pExpr.Ptr() )
					pAttr->m_pExpr->AddRef(); // CreateFilterJson() uses a refcounted pointer, but does not AddRef() itself, so help it
				pFilter = CreateFilterJson ( pAttr, pAttr->m_pExpr.Ptr(), tSettings.m_eType, tSettings.m_bHasEqual, sError, eCollation );
			} else
			{
				// fixup "fltcol=intval" conditions
				if ( pAttr->m_eAttrType==SPH_ATTR_FLOAT && tSettings.m_eType==SPH_FILTER_VALUES && tSettings.GetNumValues()==1 )
				{
					eType = SPH_FILTER_FLOATRANGE;
					fMin = fMax = (float)tSettings.GetValue(0);
				}

				pFilter = CreateFilter ( pAttr->m_eAttrType, eType, tSettings.m_eMvaFunc, tSettings.GetNumValues(), pAttr->m_tLocator, sError, sWarning, tSettings.m_bHasEqual, eCollation );
			}
		}
	}

	// fill filter's properties
	if ( pFilter )
	{
		if ( pAttr )
			pFilter->SetLocator ( pAttr->m_tLocator );

		pFilter->SetMVAStorage ( pMvaPool, bArenaProhibit );
		pFilter->SetStringStorage ( pStrings );

		pFilter->SetRange ( tSettings.m_iMinValue, tSettings.m_iMaxValue );
		if ( eType==SPH_FILTER_FLOATRANGE )
			pFilter->SetRangeFloat ( fMin, fMax );
		else
			pFilter->SetRangeFloat ( (float)tSettings.m_iMinValue, (float)tSettings.m_iMaxValue );

		pFilter->SetRefString ( tSettings.m_dStrings.Begin(), tSettings.m_dStrings.GetLength() );
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


ISphFilter * sphCreateFilter ( const CSphFilterSettings & tSettings, const ISphSchema & tSchema, const DWORD * pMvaPool, const BYTE * pStrings, CSphString & sError, CSphString & sWarning, ESphCollation eCollation, bool bArenaProhibit )
{
	return CreateFilter ( tSettings, tSettings.m_sAttrName, tSchema, pMvaPool, pStrings, sError, sWarning, false, eCollation, bArenaProhibit );
}


ISphFilter * sphCreateAggrFilter ( const CSphFilterSettings * pSettings, const CSphString & sAttrName, const ISphSchema & tSchema, CSphString & sError )
{
	assert ( pSettings );
	CSphString sWarning;
	ISphFilter * pRes = CreateFilter ( *pSettings, sAttrName, tSchema, NULL, NULL, sError, sWarning, true, SPH_COLLATION_DEFAULT, false );
	assert ( sWarning.IsEmpty() );
	return pRes;
}

ISphFilter * sphCreateFilter ( const KillListVector & dKillList )
{
	return new Filter_KillList ( dKillList );
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
