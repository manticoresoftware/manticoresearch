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


template<bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT = false, bool OPEN_RIGHT = false, typename T = SphAttr_t>
bool EvalRange ( T tValue, T tMin, T tMax )
{
	if_const ( OPEN_LEFT )
	{
		if_const ( HAS_EQUAL_MAX )
			return tValue<=tMax;
		else
			return tValue<tMax;
	}
	
	if_const ( OPEN_RIGHT )
	{
		if_const ( HAS_EQUAL_MIN )
			return tValue>=tMin;
		else
			return tValue>tMin;
	}

	bool bMinOk;
	if_const ( HAS_EQUAL_MIN )
		bMinOk = tValue>=tMin;
	else
		bMinOk = tValue>tMin;

	bool bMaxOk;
	if_const ( HAS_EQUAL_MAX )
		bMaxOk = tValue<=tMax;
	else
		bMaxOk = tValue<tMax;

	return bMinOk && bMaxOk;
}


template<bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT = false, bool OPEN_RIGHT = false, typename T = SphAttr_t>
bool EvalBlockRangeAny ( T tMin1, T tMax1, T tMin2, T tMax2 )
{
	if_const ( OPEN_LEFT )
	{
		if_const ( HAS_EQUAL_MAX )
			return tMin1<=tMax2;
		else
			return tMin1<tMax2;
	}

	if_const ( OPEN_RIGHT )
	{
		if_const ( HAS_EQUAL_MIN )
			return tMax1>=tMin2;
		else
			return tMax1>tMin2;
	}

	bool bMinOk;
	if_const ( HAS_EQUAL_MIN )
		bMinOk = tMin1<=tMax2;
	else
		bMinOk = tMin1<tMax2;

	bool bMaxOk;
	if_const ( HAS_EQUAL_MAX )
		bMaxOk = tMax1>=tMin2;
	else
		bMaxOk = tMax1>tMin2;

	return bMinOk && bMaxOk;
}


template<bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, typename T = SphAttr_t>
bool EvalBlockRangeAll ( T tMin1, T tMax1, T tMin2, T tMax2 )
{
	bool bMinOk;
	if_const ( HAS_EQUAL_MIN )
		bMinOk = tMin1>=tMin2;
	else
		bMinOk = tMin1>tMin2;

	bool bMaxOk;
	if_const ( HAS_EQUAL_MAX )
		bMaxOk = tMax1<=tMax2;
	else
		bMaxOk = tMax1<tMax2;

	return bMinOk && bMaxOk;
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


template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX, bool OPEN_LEFT, bool OPEN_RIGHT >
struct Filter_Range: public IFilter_Attr, IFilter_Range
{
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT> ( tMatch.GetAttr ( m_tLocator ), m_iMinValue, m_iMaxValue );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		SphAttr_t uBlockMin = sphGetRowAttr ( DOCINFO2ATTRS ( pMinDocinfo ), m_tLocator );
		SphAttr_t uBlockMax = sphGetRowAttr ( DOCINFO2ATTRS ( pMaxDocinfo ), m_tLocator );

		// not-reject
		return EvalBlockRangeAny<HAS_EQUAL_MIN,HAS_EQUAL_MAX,OPEN_LEFT,OPEN_RIGHT> ( uBlockMin, uBlockMax, m_iMinValue, m_iMaxValue );
	}
};

// float

template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
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
		return EvalRange<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( tMatch.GetAttrFloat ( m_tLocator ), m_fMinValue, m_fMaxValue );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		if ( m_tLocator.m_bDynamic )
			return true; // ignore computed attributes

		float fBlockMin = sphDW2F ( (DWORD)sphGetRowAttr ( DOCINFO2ATTRS ( pMinDocinfo ), m_tLocator ) );
		float fBlockMax = sphDW2F ( (DWORD)sphGetRowAttr ( DOCINFO2ATTRS ( pMaxDocinfo ), m_tLocator ) );

		// not-reject
		return EvalBlockRangeAny<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( fBlockMin, fBlockMax, m_fMinValue, m_fMaxValue );
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

template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
struct Filter_IdRange: public IFilter_Range
{
	virtual void SetRange ( SphAttr_t tMin, SphAttr_t tMax )
	{
		m_iMinValue = (SphDocID_t)Max ( (SphDocID_t)tMin, 0u );
		m_iMaxValue = tMax;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange<HAS_EQUAL_MIN, HAS_EQUAL_MAX>( tMatch.m_uDocID, (SphDocID_t)m_iMinValue, (SphDocID_t)m_iMaxValue );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		const SphDocID_t uBlockMin = DOCINFO2ID ( pMinDocinfo );
		const SphDocID_t uBlockMax = DOCINFO2ID ( pMaxDocinfo );

		// not-reject
		return EvalBlockRangeAny<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( uBlockMin, uBlockMax, (SphDocID_t)m_iMinValue, (SphDocID_t)m_iMaxValue );
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

template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
struct Filter_WeightRange: public IFilter_Range
{
	virtual bool IsEarly () { return false; }
	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange<HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( (SphAttr_t)tMatch.m_iWeight, m_iMinValue, m_iMaxValue );
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

template < typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
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
				return ( HAS_EQUAL_MIN || pVal+1<pEnd );
		}
		if ( L==pEnd )
			return false;

		T iMvaL = GetMvaValue ( L );
		if_const ( HAS_EQUAL_MAX )
			return iMvaL<=m_iMaxValue;
		else
			return iMvaL<m_iMaxValue;
	}
};


template < typename T, bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
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

		return EvalBlockRangeAll<HAS_EQUAL_MIN,HAS_EQUAL_MAX> ( *L, *R, (T)m_iMinValue, (T)m_iMaxValue );
	}
};


SphStringCmp_fn CmpFn ( ESphCollation eCollation )
{
	switch ( eCollation )
	{
		case SPH_COLLATION_LIBC_CI:			return sphCollateLibcCI;
		case SPH_COLLATION_LIBC_CS:			return sphCollateLibcCS;
		case SPH_COLLATION_UTF8_GENERAL_CI: return sphCollateUtf8GeneralCI;
		default:							return sphCollateBinary;
	}
}


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
		, m_fnStrCmp ( CmpFn ( eCollation ) )
		, m_pStringBase ( NULL )
		, m_bPacked ( eType==SPH_ATTR_STRING )
	{}

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


struct Filter_Or: public ISphFilter
{
	ISphFilter * m_pLeft;
	ISphFilter * m_pRight;

	Filter_Or ( ISphFilter * pLeft, ISphFilter * pRight )
		: m_pLeft ( pLeft )
		, m_pRight ( pRight )
	{
		m_bUsesAttrs |= pLeft->UsesAttrs();
		m_bUsesAttrs |= pRight->UsesAttrs();
	}

	~Filter_Or ()
	{
		SafeDelete ( m_pLeft );
		SafeDelete ( m_pRight );
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return ( m_pLeft->Eval ( tMatch ) || m_pRight->Eval ( tMatch ) );
	}

	virtual bool EvalBlock ( const DWORD * pMinDocinfo, const DWORD * pMaxDocinfo ) const
	{
		return ( m_pLeft->EvalBlock ( pMinDocinfo, pMaxDocinfo ) || m_pRight->EvalBlock ( pMinDocinfo, pMaxDocinfo ) );
	}

	virtual void SetMVAStorage ( const DWORD * pMva, bool bArenaProhibit )
	{
		m_pLeft->SetMVAStorage ( pMva, bArenaProhibit );
		m_pRight->SetMVAStorage ( pMva, bArenaProhibit );
	}

	virtual void SetStringStorage ( const BYTE * pStrings )
	{
		m_pLeft->SetStringStorage ( pStrings );
		m_pRight->SetStringStorage ( pStrings );
	}

	virtual ISphFilter * Optimize()
	{
		m_pLeft->Optimize();
		m_pRight->Optimize();
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


#define CREATE_RANGE_FILTER(FILTER,SETTINGS) \
{ \
	if ( SETTINGS.m_bHasEqualMin ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<true,true>; \
		else \
			return new FILTER<true,false>; \
	} else \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<false,true>; \
		else \
			return new FILTER<false,false>; \
	} \
}


#define CREATE_RANGE_FILTER_WITH_OPEN(FILTER,SETTINGS) \
{ \
	if ( SETTINGS.m_bOpenLeft ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<true,true,true,false>; \
		else \
			return new FILTER<true,false,true,false>; \
	} else if ( SETTINGS.m_bOpenRight ) \
	{ \
		if ( SETTINGS.m_bHasEqualMin ) \
			return new FILTER<true,true,false,true>; \
		else \
			return new FILTER<false,true,false,true>; \
	} \
	assert ( !SETTINGS.m_bOpenLeft && !SETTINGS.m_bOpenRight ); \
	if ( SETTINGS.m_bHasEqualMin ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<true,true,false,false>; \
		else \
			return new FILTER<true,false,false,false>; \
	} else \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<false,true,false,false>; \
		else \
			return new FILTER<false,false,false,false>; \
	} \
}


#define CREATE_MVA_RANGE_FILTER(FILTER,T,SETTINGS) \
{ \
	if ( SETTINGS.m_bHasEqualMin ) \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<T,true,true>; \
		else \
			return new FILTER<T,true,false>; \
	} else \
	{ \
		if ( SETTINGS.m_bHasEqualMax ) \
			return new FILTER<T,false,true>; \
		else \
			return new FILTER<T,false,false>; \
	} \
}


#define CREATE_EXPR_RANGE_FILTER(FILTER,EXPR,bHasEqualMin,bHasEqualMax) \
{ \
	if ( bHasEqualMin ) \
	{ \
		if ( bHasEqualMax ) \
			return new FILTER<true,true>(EXPR); \
		else \
			return new FILTER<true,false>(EXPR); \
	} else \
	{ \
		if ( bHasEqualMax ) \
			return new FILTER<false,true>(EXPR); \
		else \
			return new FILTER<false,false>(EXPR); \
	} \
}


static ISphFilter * CreateSpecialFilter ( const CSphString & sName, const CSphFilterSettings & tSettings, CSphString & sError )
{
	if ( sName=="@id" )
	{
		switch ( tSettings.m_eType )
		{
		case SPH_FILTER_VALUES:	return new Filter_IdValues;
		case SPH_FILTER_RANGE:	CREATE_RANGE_FILTER(Filter_IdRange,tSettings);
		default:
			return ReportError ( sError, "unsupported filter type '%s' on @id", tSettings.m_eType );
		}
	} else if ( sName=="@weight" )
	{
		switch ( tSettings.m_eType )
		{
		case SPH_FILTER_VALUES:	return new Filter_WeightValues;
		case SPH_FILTER_RANGE:	CREATE_RANGE_FILTER ( Filter_WeightRange, tSettings );
		default:
			return ReportError ( sError, "unsupported filter type '%s' on @weight", tSettings.m_eType );
		}
	}

	return NULL;
}


static ISphFilter * CreateFilter ( const CSphFilterSettings & tSettings, ESphFilter eFilterType, ESphAttr eAttrType, const CSphAttrLocator & tLoc, ESphCollation eCollation, CSphString & sError, CSphString & sWarning )
{
	// MVA
	if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET )
	{
		if (!( eFilterType==SPH_FILTER_VALUES || eFilterType==SPH_FILTER_RANGE ))
			return ReportError ( sError, "unsupported filter type '%s' on MVA column", eFilterType );

		if ( tSettings.m_eMvaFunc==SPH_MVAFUNC_NONE )
			sWarning.SetSprintf ( "suggest an explicit ANY()/ALL() around a filter on MVA column" );

		bool bWide = ( eAttrType==SPH_ATTR_INT64SET );
		bool bRange = ( eFilterType==SPH_FILTER_RANGE );
		bool bAll = ( tSettings.m_eMvaFunc==SPH_MVAFUNC_ALL );
		int iIndex = bWide*4 + bRange*2 + bAll;

		switch ( iIndex )
		{
			case 0:	return new Filter_MVAValues_Any<DWORD>();
			case 1:	return new Filter_MVAValues_All<DWORD>();

			case 2:	CREATE_MVA_RANGE_FILTER ( Filter_MVARange_Any, DWORD, tSettings );
			case 3:	CREATE_MVA_RANGE_FILTER ( Filter_MVARange_All, DWORD, tSettings );

			case 4:	return new Filter_MVAValues_Any<int64_t>();
			case 5:	return new Filter_MVAValues_All<int64_t>();

			case 6:	CREATE_MVA_RANGE_FILTER ( Filter_MVARange_Any, int64_t, tSettings );
			case 7:	CREATE_MVA_RANGE_FILTER ( Filter_MVARange_All, int64_t, tSettings );
		}
	}

	// float
	if ( eAttrType==SPH_ATTR_FLOAT )
	{
		if ( eFilterType==SPH_FILTER_FLOATRANGE || eFilterType==SPH_FILTER_RANGE )
			CREATE_RANGE_FILTER ( Filter_FloatRange, tSettings );

		return ReportError ( sError, "unsupported filter type '%s' on float column", eFilterType );
	}

	if ( eAttrType==SPH_ATTR_STRING || eAttrType==SPH_ATTR_STRINGPTR )
	{
		if ( eFilterType==SPH_FILTER_STRING_LIST )
			return new Filter_StringValues_c ( eCollation, eAttrType );
		else
			return new FilterString_c ( eCollation, eAttrType, tSettings.m_bHasEqualMin || tSettings.m_bHasEqualMax );
	}

	// non-float, non-MVA
	switch ( eFilterType )
	{
		case SPH_FILTER_VALUES:
			if ( tSettings.GetNumValues()==1 && ( eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_BIGINT || eAttrType==SPH_ATTR_TOKENCOUNT ) )
			{
				if ( ( eAttrType==SPH_ATTR_INTEGER || eAttrType==SPH_ATTR_TOKENCOUNT ) && !tLoc.m_bDynamic && tLoc.m_iBitCount==32 && ( tLoc.m_iBitOffset % 32 )==0 )
					return new Filter_SingleValueStatic32();
				else
					return new Filter_SingleValue();
			} else
				return new Filter_Values();

		case SPH_FILTER_RANGE:	CREATE_RANGE_FILTER_WITH_OPEN ( Filter_Range, tSettings );
		default:				return ReportError ( sError, "unsupported filter type '%s' on int column", eFilterType );
	}
}

//////////////////////////////////////////////////////////////////////////
// EXPRESSION STUFF
//////////////////////////////////////////////////////////////////////////

template<typename BASE>
class ExprFilter_c : public BASE
{
protected:
	const BYTE *				m_pStrings;
	CSphRefcountedPtr<ISphExpr>	m_pExpr;

public:
	explicit ExprFilter_c ( ISphExpr * pExpr )
		: m_pStrings ( NULL )
		, m_pExpr ( pExpr )
	{}

	virtual void SetStringStorage ( const BYTE * pStrings )
	{
		m_pStrings = pStrings;
		if ( m_pExpr.Ptr() )
			m_pExpr->Command ( SPH_EXPR_SET_STRING_POOL, (void*)pStrings );
	}
};


template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
class ExprFilterFloatRange_c : public ExprFilter_c<IFilter_Range>
{
public:
	explicit ExprFilterFloatRange_c ( ISphExpr * pExpr )
		: ExprFilter_c<IFilter_Range> ( pExpr )
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
		return EvalRange<HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( m_pExpr->Eval ( tMatch ), m_fMinValue, m_fMaxValue );
	}
};


template < bool HAS_EQUAL_MIN, bool HAS_EQUAL_MAX >
class ExprFilterRange_c : public ExprFilter_c<IFilter_Range>
{
public:
	explicit ExprFilterRange_c ( ISphExpr * pExpr )
		: ExprFilter_c<IFilter_Range> ( pExpr )
	{}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		return EvalRange<HAS_EQUAL_MIN, HAS_EQUAL_MAX> ( m_pExpr->Int64Eval(tMatch), m_iMinValue, m_iMaxValue );
	}
};


class ExprFilterValues_c : public ExprFilter_c<IFilter_Values>
{
public:
	explicit ExprFilterValues_c ( ISphExpr * pExpr )
		: ExprFilter_c<IFilter_Values> ( pExpr )
	{}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		assert ( this->m_pExpr.Ptr()!=NULL );
		return EvalValues ( m_pExpr->Int64Eval ( tMatch ) );
	}
};


class ExprFilterString_c : public ExprFilter_c<ISphFilter>
{
protected:
	CSphFixedVector<BYTE>	m_dVal;
	SphStringCmp_fn			m_fnStrCmp;
	bool					m_bEq;

public:
	explicit ExprFilterString_c ( ISphExpr * pExpr, ESphCollation eCollation, bool bEq )
		: ExprFilter_c<ISphFilter> ( pExpr )
		, m_dVal ( 0 )
		, m_fnStrCmp ( CmpFn ( eCollation ) )
		, m_bEq ( bEq )
	{}

	virtual void SetRefString ( const CSphString * pRef, int iCount )
	{
		assert ( iCount<2 );
		const char * sVal = pRef ? pRef->cstr() : NULL;
		int iLen = pRef ? pRef->Length() : 0;
		m_dVal.Reset ( iLen+4 );
		int iPacked = sphPackStrlen ( m_dVal.Begin(), iLen );
		memcpy ( m_dVal.Begin()+iPacked, sVal, iLen );
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		const BYTE * pVal;
		int iLen = m_pExpr->StringEval ( tMatch, &pVal );
		int iPacked = iLen==0 ? 0 : iLen < 0x80 ? 1 : iLen < 0x4000 ? 2 : 3;
		pVal = iLen ? pVal : (const BYTE*)"\0";
		bool bEq = ( m_fnStrCmp ( pVal-iPacked, m_dVal.Begin(), true )==0 );
		return ( m_bEq==bEq );
	}
};


class ExprFilterNull_c : public ExprFilter_c<ISphFilter>
{
protected:
	CSphAttrLocator m_tLoc;
	const bool m_bEquals;
	const bool m_bCheckOnlyKey;

public:
	ExprFilterNull_c ( ISphExpr * pExpr, bool bEquals, bool bCheckOnlyKey )
		: ExprFilter_c<ISphFilter> ( pExpr )
		, m_bEquals ( bEquals )
		, m_bCheckOnlyKey ( bCheckOnlyKey )
	{}

	virtual void SetLocator ( const CSphAttrLocator & tLocator )
	{
		m_tLoc = tLocator;
	}

	ESphJsonType GetKey ( const CSphMatch & tMatch ) const
	{
		if ( !m_pStrings || !m_pExpr )
			return JSON_EOF;
		uint64_t uValue = m_pExpr->Int64Eval ( tMatch );
		if ( uValue==0 ) // either no data or invalid path
			return JSON_EOF;

		ESphJsonType eRes = (ESphJsonType)( uValue >> 32 );
		return eRes;
	}

	virtual bool Eval ( const CSphMatch & tMatch ) const
	{
		assert ( m_pStrings );

		if ( !m_pExpr ) // regular attribute? return true if blob size is null
		{
			const BYTE * pStr = NULL;
			DWORD uOffset = (DWORD) tMatch.GetAttr ( m_tLoc );
			if ( uOffset )
				sphUnpackStr ( m_pStrings+uOffset, &pStr );
			return m_bEquals ^ ( pStr!=NULL );
		}

		// possibly json
		ESphJsonType eRes = GetKey ( tMatch );
		if ( m_bCheckOnlyKey )
			return ( eRes!=JSON_EOF );

		return m_bEquals ^ ( eRes!=JSON_EOF && eRes!=JSON_NULL );
	}
};


static ISphFilter * CreateFilterExpr ( ISphExpr * pExpr, const CSphFilterSettings & tSettings, CSphString & sError, ESphCollation eCollation, ESphAttr eAttrType )
{
	// autoconvert all json types except SPH_FILTER_NULL, it needs more info
	bool bAutoConvert = ( eAttrType==SPH_ATTR_JSON || eAttrType==SPH_ATTR_JSON_FIELD )
		&& tSettings.m_eType!=SPH_FILTER_NULL;
	if ( bAutoConvert )
		pExpr = sphJsonFieldConv ( pExpr );

	switch ( tSettings.m_eType )
	{
		case SPH_FILTER_VALUES:			return new ExprFilterValues_c ( pExpr );
		case SPH_FILTER_FLOATRANGE:		CREATE_EXPR_RANGE_FILTER ( ExprFilterFloatRange_c, pExpr, tSettings.m_bHasEqualMin, tSettings.m_bHasEqualMax );
		case SPH_FILTER_RANGE:			CREATE_EXPR_RANGE_FILTER ( ExprFilterRange_c, pExpr, tSettings.m_bHasEqualMin, tSettings.m_bHasEqualMax );
		case SPH_FILTER_STRING:			return new ExprFilterString_c ( pExpr, eCollation, tSettings.m_bHasEqualMin || tSettings.m_bHasEqualMax );
		case SPH_FILTER_NULL:			return new ExprFilterNull_c ( pExpr, tSettings.m_bIsNull, false );
		default:
			sError = "this filter type on expressions is not implemented yet";
			if ( bAutoConvert )
				SafeDelete ( pExpr );
			return nullptr;
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
		pFilter = CreateSpecialFilter ( sAttrName, tSettings, sError );
		if ( !pFilter && !sError.IsEmpty() )
			return NULL;
	}

	// try to lookup a regular attribute
	ESphFilter eType = tSettings.m_eType;
	float fMin = tSettings.m_fMinValue;
	float fMax = tSettings.m_fMaxValue;

	ESphAttr eAttrType = SPH_ATTR_NONE;
	ISphExpr * pExpr = NULL;

	if ( !pFilter )
	{
		const int iAttr = tSchema.GetAttrIndex ( sAttrName.cstr() );
		if ( iAttr<0 )
		{
			// try expression
			pExpr = sphExprParse ( sAttrName.cstr(), tSchema, &eAttrType, NULL, sError, NULL, eCollation );
			if ( pExpr )
			{
				pFilter = CreateFilterExpr ( pExpr, tSettings, sError, eCollation, eAttrType );

			} else
			{
				sError.SetSprintf ( "no such filter attribute '%s'", sAttrName.cstr() );
				return NULL;
			}

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
					pAttr->m_pExpr->AddRef(); // CreateFilterExpr() uses a refcounted pointer, but does not AddRef() itself, so help it

				pExpr = pAttr->m_pExpr.Ptr();
				eAttrType = pAttr->m_eAttrType;
				pFilter = CreateFilterExpr ( pAttr->m_pExpr.Ptr(), tSettings, sError, eCollation, pAttr->m_eAttrType );

			} else
			{
				// fixup "fltcol=intval" conditions
				if ( pAttr->m_eAttrType==SPH_ATTR_FLOAT && tSettings.m_eType==SPH_FILTER_VALUES && tSettings.GetNumValues()==1 )
				{
					eType = SPH_FILTER_FLOATRANGE;
					fMin = fMax = (float)tSettings.GetValue(0);
				}

				pFilter = CreateFilter ( tSettings, eType, pAttr->m_eAttrType, pAttr->m_tLocator, eCollation, sError, sWarning );
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


		// filter for json || json.field needs to check that key exists
		if ( ( eAttrType==SPH_ATTR_JSON || eAttrType==SPH_ATTR_JSON_FIELD ) && tSettings.m_eType!=SPH_FILTER_NULL && pFilter && pExpr )
		{
			pExpr->AddRef();
			ISphFilter * pNotNull = new ExprFilterNull_c ( pExpr, false, true );
			pNotNull->SetStringStorage ( pStrings );

			pFilter = new Filter_And2 ( pFilter, pNotNull, true );
		}
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

static bool IsWeightColumn ( const CSphString & sAttr, const ISphSchema & tSchema )
{
	if ( sAttr=="@weight" )
		return true;

	const CSphColumnInfo * pCol = tSchema.GetAttr ( sAttr.cstr() );
	return ( pCol && pCol->m_bWeight );
}

CreateFilterContext_t::CreateFilterContext_t()
	: m_pFilters ( NULL )
	, m_pFilterTree ( NULL )
	, m_pKillList ( NULL )
	, m_pFilter ( NULL )
	, m_pWeightFilter ( NULL )
{
}

CreateFilterContext_t::~CreateFilterContext_t()
{
	SafeDelete ( m_pFilter );
	SafeDelete ( m_pWeightFilter );

	ARRAY_FOREACH ( i, m_dUserVals )
		m_dUserVals[i]->Release();
}


static ISphFilter * CreateFilterNode ( CreateFilterContext_t & tCtx, int iNode, CSphString & sError, CSphString & sWarning, bool & bHasWeight )
{
	const FilterTreeItem_t * pCur = tCtx.m_pFilterTree->Begin() + iNode;
	if ( pCur->m_iFilterItem!=-1 )
	{
		const CSphFilterSettings * pFilterSettings = tCtx.m_pFilters->Begin() + pCur->m_iFilterItem;
		if ( pFilterSettings->m_sAttrName.IsEmpty() )
		{
			sError = "filter with empty name";
			return NULL;
		}

		bHasWeight |= IsWeightColumn ( pFilterSettings->m_sAttrName, *tCtx.m_pSchema );

		// bind user variable local to that daemon
		CSphFilterSettings tUservar;
		if ( pFilterSettings->m_eType==SPH_FILTER_USERVAR )
		{
			const CSphString * sVar = pFilterSettings->m_dStrings.GetLength()==1 ? pFilterSettings->m_dStrings.Begin() : NULL;
			if ( !g_pUservarsHook || !sVar )
			{
				sError = "no global variables found";
				return NULL;
			}

			const UservarIntSet_c * pUservar = g_pUservarsHook ( *sVar );
			if ( !pUservar )
			{
				sError.SetSprintf ( "undefined global variable '%s'", sVar->cstr() );
				return NULL;
			}

			tCtx.m_dUserVals.Add ( pUservar );
			tUservar = *pFilterSettings;
			tUservar.m_eType = SPH_FILTER_VALUES;
			tUservar.SetExternalValues ( pUservar->Begin(), pUservar->GetLength() );
			pFilterSettings = &tUservar;
		}

		ISphFilter * pFilter = sphCreateFilter ( *pFilterSettings, *tCtx.m_pSchema, tCtx.m_pMvaPool, tCtx.m_pStrings, sError, sWarning, tCtx.m_eCollation, tCtx.m_bArenaProhibit );
		if ( !pFilter )
			return NULL;

		return pFilter;
	}

	assert ( pCur->m_iLeft!=-1 && pCur->m_iRight!=-1 );
	ISphFilter * pLeft = CreateFilterNode ( tCtx, pCur->m_iLeft, sError, sWarning, bHasWeight );
	ISphFilter * pRight = CreateFilterNode ( tCtx, pCur->m_iRight, sError, sWarning, bHasWeight );

	if ( !pLeft || !pRight )
	{
		SafeDelete ( pLeft );
		SafeDelete ( pRight );
		return NULL;
	}

	if ( pCur->m_bOr )
		return new Filter_Or ( pLeft, pRight );
	else
		return new Filter_And2 ( pLeft, pRight, pLeft->UsesAttrs() || pRight->UsesAttrs() );
}


bool sphCreateFilters ( CreateFilterContext_t & tCtx, CSphString & sError, CSphString & sWarning )
{
	bool bGotFilters = ( tCtx.m_pFilters && tCtx.m_pFilters->GetLength() );
	bool bGotKlist = ( tCtx.m_pKillList && tCtx.m_pKillList->GetLength() );

	if ( !bGotFilters && !bGotKlist )
		return true;

	bool bGotTree = ( tCtx.m_pFilterTree && tCtx.m_pFilterTree->GetLength() );
	assert ( tCtx.m_pSchema );

	if ( bGotFilters )
	{
		if ( !bGotTree )
		{
			bool bScan = tCtx.m_bScan;
			ARRAY_FOREACH ( i, (*tCtx.m_pFilters) )
			{
				const CSphFilterSettings * pFilterSettings = tCtx.m_pFilters->Begin() + i;
				if ( pFilterSettings->m_sAttrName.IsEmpty() )
					continue;

				bool bWeight = IsWeightColumn ( pFilterSettings->m_sAttrName, *tCtx.m_pSchema );
				if ( bScan && bWeight )
					continue; // @weight is not available in fullscan mode

				// bind user variable local to that daemon
				CSphFilterSettings tUservar;
				if ( pFilterSettings->m_eType==SPH_FILTER_USERVAR )
				{
					const CSphString * sVar = pFilterSettings->m_dStrings.GetLength()==1 ? pFilterSettings->m_dStrings.Begin() : NULL;
					if ( !g_pUservarsHook || !sVar )
					{
						sError = "no global variables found";
						return false;
					}

					const UservarIntSet_c * pUservar = g_pUservarsHook ( *sVar );
					if ( !pUservar )
					{
						sError.SetSprintf ( "undefined global variable '%s'", sVar->cstr() );
						return false;
					}

					tCtx.m_dUserVals.Add ( pUservar );
					tUservar = *pFilterSettings;
					tUservar.m_eType = SPH_FILTER_VALUES;
					tUservar.SetExternalValues ( pUservar->Begin(), pUservar->GetLength() );
					pFilterSettings = &tUservar;
				}

				ISphFilter * pFilter = sphCreateFilter ( *pFilterSettings, *tCtx.m_pSchema, tCtx.m_pMvaPool, tCtx.m_pStrings, sError, sWarning, tCtx.m_eCollation, tCtx.m_bArenaProhibit );
				if ( !pFilter )
					return false;

				ISphFilter ** pGroup = bWeight ? &tCtx.m_pWeightFilter : &tCtx.m_pFilter;
				*pGroup = sphJoinFilters ( *pGroup, pFilter );
			}
		} else
		{
			bool bWeight = false;
			ISphFilter * pFilter = CreateFilterNode ( tCtx, tCtx.m_pFilterTree->GetLength() - 1, sError, sWarning, bWeight );
			if ( !pFilter )
				return false;

			// weight filter phase only on match path
			if ( bWeight && tCtx.m_bScan )
				tCtx.m_pWeightFilter = pFilter;
			else
				tCtx.m_pFilter = pFilter;
		}
	}

	if ( bGotKlist )
	{
		ISphFilter * pFilter = sphCreateFilter ( *tCtx.m_pKillList );
		if ( !pFilter )
			return false;

		tCtx.m_pFilter = sphJoinFilters ( tCtx.m_pFilter, pFilter );
	}

	return true;
}

//
// $Id$
//
