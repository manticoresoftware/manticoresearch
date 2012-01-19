//
// $Id$
//

//
// Copyright (c) 2001-2011, Andrew Aksyonoff
// Copyright (c) 2008-2011, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"

#include <time.h>
#include <math.h>

#if !USE_WINDOWS
#include <unistd.h>
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////
// TRAITS
//////////////////////////////////////////////////////////////////////////

/// groupby key type
typedef int64_t				SphGroupKey_t;


/// base grouper (class that computes groupby key)
class CSphGrouper
{
public:
	virtual					~CSphGrouper () {}
	virtual SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const = 0;
	virtual SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const = 0;
	virtual void			GetLocator ( CSphAttrLocator & tOut ) const = 0;
	virtual ESphAttr		GetResultType () const = 0;
	virtual void			SetStringPool ( const BYTE * ) {}
};


/// match-sorting priority queue traits
class CSphMatchQueueTraits : public ISphMatchSorter, ISphNoncopyable
{
protected:
	CSphMatch *					m_pData;
	int							m_iUsed;
	int							m_iSize;

	CSphMatchComparatorState	m_tState;
	const bool					m_bUsesAttrs;

public:
	/// ctor
	CSphMatchQueueTraits ( int iSize, bool bUsesAttrs )
		: m_iUsed ( 0 )
		, m_iSize ( iSize )
		, m_bUsesAttrs ( bUsesAttrs )
	{
		assert ( iSize>0 );
		m_pData = new CSphMatch [ iSize ];
		assert ( m_pData );

		m_tState.m_iNow = (DWORD) time ( NULL );
	}

	/// dtor
	~CSphMatchQueueTraits ()
	{
		SafeDeleteArray ( m_pData );
	}

public:
	void		SetState ( const CSphMatchComparatorState & tState )	{ m_tState = tState; m_tState.m_iNow = (DWORD) time ( NULL ); }
	bool		UsesAttrs () const										{ return m_bUsesAttrs; }
	virtual CSphMatch *	Finalize ()												{ return m_pData; }
	virtual int			GetLength () const										{ return m_iUsed; }
};

//////////////////////////////////////////////////////////////////////////
// PLAIN SORTING QUEUE
//////////////////////////////////////////////////////////////////////////

/// normal match-sorting priority queue
template < typename COMP > class CSphMatchQueue : public CSphMatchQueueTraits
{
public:
	/// ctor
	CSphMatchQueue ( int iSize, bool bUsesAttrs )
		: CSphMatchQueueTraits ( iSize, bUsesAttrs )
	{}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
	{
		return false;
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		m_iTotal++;

		if ( m_iUsed==m_iSize )
		{
			// if it's worse that current min, reject it, else pop off current min
			if ( COMP::IsLess ( tEntry, m_pData[0], m_tState ) )
				return true;
			else
				Pop ();
		}

		// do add
		m_pData[m_iUsed].Clone ( tEntry, m_tSchema.GetDynamicSize() );
		int iEntry = m_iUsed++;

		// sift up if needed, so that worst (lesser) ones float to the top
		while ( iEntry )
		{
			int iParent = ( iEntry-1 ) >> 1;
			if ( !COMP::IsLess ( m_pData[iEntry], m_pData[iParent], m_tState ) )
				break;

			// entry is less than parent, should float to the top
			Swap ( m_pData[iEntry], m_pData[iParent] );
			iEntry = iParent;
		}

		return true;
	}

	/// add grouped entry (must not happen)
	virtual bool PushGrouped ( const CSphMatch & )
	{
		assert ( 0 );
		return false;
	}

	/// remove root (ie. top priority) entry
	virtual void Pop ()
	{
		assert ( m_iUsed );
		if ( !(--m_iUsed) ) // empty queue? just return
			return;

		// make the last entry my new root
		Swap ( m_pData[0], m_pData[m_iUsed] );

		// sift down if needed
		int iEntry = 0;
		for ( ;; )
		{
			// select child
			int iChild = (iEntry<<1) + 1;
			if ( iChild>=m_iUsed )
				break;

			// select smallest child
			if ( iChild+1<m_iUsed )
				if ( COMP::IsLess ( m_pData[iChild+1], m_pData[iChild], m_tState ) )
					iChild++;

			// if smallest child is less than entry, do float it to the top
			if ( COMP::IsLess ( m_pData[iChild], m_pData[iEntry], m_tState ) )
			{
				Swap ( m_pData[iChild], m_pData[iEntry] );
				iEntry = iChild;
				continue;
			}

			break;
		}
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	void Flatten ( CSphMatch * pTo, int iTag )
	{
		assert ( m_iUsed>=0 );
		pTo += m_iUsed;
		while ( m_iUsed>0 )
		{
			--pTo;
			pTo[0].Clone ( m_pData[0], m_tSchema.GetDynamicSize() ); // OPTIMIZE? reset dst + swap?
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			Pop ();
		}
		m_iTotal = 0;
	}
};

/// collector for UPDATE statement
class CSphUpdateQueue : public CSphMatchQueueTraits
{
	CSphAttrUpdateEx*	m_pUpdate;
private:
	void DoUpdate()
	{
		if ( !m_iUsed )
			return;

		CSphAttrUpdate tSet;
		tSet.m_dAttrs = m_pUpdate->m_pUpdate->m_dAttrs;
		tSet.m_dPool = m_pUpdate->m_pUpdate->m_dPool;
		tSet.m_dRowOffset.Resize ( m_iUsed );
		if ( !DOCINFO2ID ( STATIC2DOCINFO ( m_pData->m_pStatic ) ) ) // if static attrs were copied, so, they actually dynamic
		{
			tSet.m_dDocids.Resize ( m_iUsed );
			ARRAY_FOREACH ( i, tSet.m_dDocids )
			{
				tSet.m_dDocids[i] = m_pData[i].m_iDocID;
				tSet.m_dRowOffset[i] = 0;
			}
		} else // static attrs points to the active indexes - so, no lookup, 5 times faster update.
		{
			tSet.m_dRows.Resize ( m_iUsed );
			ARRAY_FOREACH ( i, tSet.m_dRows )
			{
				tSet.m_dRows[i] = m_pData[i].m_pStatic - ( sizeof(SphDocID_t) / sizeof(CSphRowitem) );
				tSet.m_dRowOffset[i] = 0;
			}
		}

		m_pUpdate->m_iAffected += m_pUpdate->m_pIndex->UpdateAttributes ( tSet, -1, *m_pUpdate->m_pError );
		m_iUsed = 0;
	}
public:
	/// ctor
	CSphUpdateQueue ( int iSize, CSphAttrUpdateEx* pUpdate )
		: CSphMatchQueueTraits ( iSize, true )
		, m_pUpdate ( pUpdate )
	{}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
	{
		return false;
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		m_iTotal++;

		if ( m_iUsed==m_iSize )
			DoUpdate();

		// do add
		m_pData[m_iUsed++].Clone ( tEntry, m_tSchema.GetDynamicSize() );
		return true;
	}

	/// add grouped entry (must not happen)
	virtual bool PushGrouped ( const CSphMatch & )
	{
		assert ( 0 );
		return false;
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	void Flatten ( CSphMatch *, int )
	{
		assert ( m_iUsed>=0 );
		DoUpdate();
		m_iTotal = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
// SORTING+GROUPING QUEUE
//////////////////////////////////////////////////////////////////////////

static bool IsCount ( const CSphString & s )
{
	return s=="@count" || s=="count(*)";
}

static bool IsGroupby ( const CSphString & s )
{
	return s=="@groupby" || s=="@distinct";
}

static bool IsGroupbyMagic ( const CSphString & s )
{
	return IsGroupby ( s ) || IsCount ( s );
}

/// groupers
#define GROUPER_BEGIN(_name) \
	class _name : public CSphGrouper \
	{ \
	protected: \
		CSphAttrLocator m_tLocator; \
	public: \
		explicit _name ( const CSphAttrLocator & tLoc ) : m_tLocator ( tLoc ) {} \
		virtual void GetLocator ( CSphAttrLocator & tOut ) const { tOut = m_tLocator; } \
		virtual ESphAttr GetResultType () const { return m_tLocator.m_iBitCount>8*(int)sizeof(DWORD) ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER; } \
		virtual SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const { return KeyFromValue ( tMatch.GetAttr ( m_tLocator ) ); } \
		virtual SphGroupKey_t KeyFromValue ( SphAttr_t uValue ) const \
		{
// NOLINT

#define GROUPER_END \
		} \
	};


#define GROUPER_BEGIN_SPLIT(_name) \
	GROUPER_BEGIN(_name) \
	time_t tStamp = (time_t)uValue; \
	struct tm * pSplit = localtime ( &tStamp );


GROUPER_BEGIN ( CSphGrouperAttr )
	return uValue;
GROUPER_END


GROUPER_BEGIN_SPLIT ( CSphGrouperDay )
	return (pSplit->tm_year+1900)*10000 + (1+pSplit->tm_mon)*100 + pSplit->tm_mday;
GROUPER_END


GROUPER_BEGIN_SPLIT ( CSphGrouperWeek )
	int iPrevSunday = (1+pSplit->tm_yday) - pSplit->tm_wday; // prev Sunday day of year, base 1
	int iYear = pSplit->tm_year+1900;
	if ( iPrevSunday<=0 ) // check if we crossed year boundary
	{
		// adjust day and year
		iPrevSunday += 365;
		iYear--;

		// adjust for leap years
		if ( iYear%4==0 && ( iYear%100!=0 || iYear%400==0 ) )
			iPrevSunday++;
	}
	return iYear*1000 + iPrevSunday;
GROUPER_END


GROUPER_BEGIN_SPLIT ( CSphGrouperMonth )
	return (pSplit->tm_year+1900)*100 + (1+pSplit->tm_mon);
GROUPER_END


GROUPER_BEGIN_SPLIT ( CSphGrouperYear )
	return (pSplit->tm_year+1900);
GROUPER_END

template <class PRED>
class CSphGrouperString : public CSphGrouperAttr, public PRED
{
private:
	const BYTE * m_pStringBase;

public:

	explicit CSphGrouperString ( const CSphAttrLocator & tLoc )
		: CSphGrouperAttr ( tLoc )
		, m_pStringBase ( NULL )
	{
	}

	virtual ESphAttr GetResultType () const
	{
		return SPH_ATTR_BIGINT;
	}

	virtual SphGroupKey_t KeyFromValue ( SphAttr_t uValue ) const
	{
		if ( !m_pStringBase || !uValue )
			return 0;

		const BYTE * pStr = NULL;
		int iLen = sphUnpackStr ( m_pStringBase+uValue, &pStr );

		if ( !pStr || !iLen )
			return 0;

		return PRED::Hash ( pStr, iLen );
	}

	virtual void SetStringPool ( const BYTE * pStrings )
	{
		m_pStringBase = pStrings;
	}
};


//////////////////////////////////////////////////////////////////////////

/// simple fixed-size hash
/// doesn't keep the order
template < typename T, typename KEY, typename HASHFUNC >
class CSphFixedHash : ISphNoncopyable
{
protected:
	static const int			HASH_LIST_END	= -1;
	static const int			HASH_DELETED	= -2;

	struct HashEntry_t
	{
		KEY		m_tKey;
		T		m_tValue;
		int		m_iNext;
	};

protected:
	CSphVector<HashEntry_t>		m_dEntries;		///< key-value pairs storage pool
	CSphVector<int>				m_dHash;		///< hash into m_dEntries pool

	int							m_iFree;		///< free pairs count
	CSphVector<int>				m_dFree;		///< free pair indexes

public:
	/// ctor
	explicit CSphFixedHash ( int iLength )
	{
		int iBuckets = ( 2 << sphLog2 ( iLength-1 ) ); // less than 50% bucket usage guaranteed
		assert ( iLength>0 );
		assert ( iLength<=iBuckets );

		m_dEntries.Resize ( iLength );
		m_dHash.Resize ( iBuckets );
		m_dFree.Resize ( iLength );

		Reset ();
	}

	/// cleanup
	void Reset ()
	{
		ARRAY_FOREACH ( i, m_dEntries )
			m_dEntries[i].m_iNext = HASH_DELETED;

		ARRAY_FOREACH ( i, m_dHash )
			m_dHash[i] = HASH_LIST_END;

		m_iFree = m_dFree.GetLength();
		ARRAY_FOREACH ( i, m_dFree )
			m_dFree[i] = i;
	}

	/// add new entry
	/// returns NULL on success
	/// returns pointer to value if already hashed
	T * Add ( const T & tValue, const KEY & tKey )
	{
		assert ( m_iFree>0 && "hash overflow" );

		// check if it's already hashed
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );
		int iPrev = -1, iEntry;

		for ( iEntry=m_dHash[uHash]; iEntry>=0; iPrev=iEntry, iEntry=m_dEntries[iEntry].m_iNext )
			if ( m_dEntries[iEntry].m_tKey==tKey )
				return &m_dEntries[iEntry].m_tValue;
		assert ( iEntry!=HASH_DELETED );

		// if it's not, do add
		int iNew = m_dFree [ --m_iFree ];

		HashEntry_t & tNew = m_dEntries[iNew];
		assert ( tNew.m_iNext==HASH_DELETED );

		tNew.m_tKey = tKey;
		tNew.m_tValue = tValue;
		tNew.m_iNext = HASH_LIST_END;

		if ( iPrev>=0 )
		{
			assert ( m_dEntries[iPrev].m_iNext==HASH_LIST_END );
			m_dEntries[iPrev].m_iNext = iNew;
		} else
		{
			assert ( m_dHash[uHash]==HASH_LIST_END );
			m_dHash[uHash] = iNew;
		}
		return NULL;
	}

	/// remove entry from hash
	void Remove ( const KEY & tKey )
	{
		// check if it's already hashed
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );
		int iPrev = -1, iEntry;

		for ( iEntry=m_dHash[uHash]; iEntry>=0; iPrev=iEntry, iEntry=m_dEntries[iEntry].m_iNext )
			if ( m_dEntries[iEntry].m_tKey==tKey )
		{
			// found, remove it
			assert ( m_dEntries[iEntry].m_iNext!=HASH_DELETED );
			if ( iPrev>=0 )
				m_dEntries[iPrev].m_iNext = m_dEntries[iEntry].m_iNext;
			else
				m_dHash[uHash] = m_dEntries[iEntry].m_iNext;

#ifndef NDEBUG
			m_dEntries[iEntry].m_iNext = HASH_DELETED;
#endif

			m_dFree [ m_iFree++ ] = iEntry;
			return;
		}
		assert ( iEntry!=HASH_DELETED );
	}

	/// get value pointer by key
	T * operator () ( const KEY & tKey ) const
	{
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );
		int iEntry;

		for ( iEntry=m_dHash[uHash]; iEntry>=0; iEntry=m_dEntries[iEntry].m_iNext )
			if ( m_dEntries[iEntry].m_tKey==tKey )
				return (T*)&m_dEntries[iEntry].m_tValue;

		assert ( iEntry!=HASH_DELETED );
		return NULL;
	}
};


/////////////////////////////////////////////////////////////////////////////

/// (group,attrvalue) pair
struct SphGroupedValue_t
{
public:
	SphGroupKey_t	m_uGroup;
	SphAttr_t		m_uValue;

public:
	SphGroupedValue_t ()
	{}

	SphGroupedValue_t ( SphGroupKey_t uGroup, SphAttr_t uValue )
		: m_uGroup ( uGroup )
		, m_uValue ( uValue )
	{}

	inline bool operator < ( const SphGroupedValue_t & rhs ) const
	{
		if ( m_uGroup<rhs.m_uGroup ) return true;
		if ( m_uGroup>rhs.m_uGroup ) return false;
		return m_uValue<rhs.m_uValue;
	}

	inline bool operator == ( const SphGroupedValue_t & rhs ) const
	{
		return m_uGroup==rhs.m_uGroup && m_uValue==rhs.m_uValue;
	}
};


/// unique values counter
/// used for COUNT(DISTINCT xxx) GROUP BY yyy queries
class CSphUniqounter : public CSphVector<SphGroupedValue_t>
{
public:
#ifndef NDEBUG
					CSphUniqounter () : m_iCountPos ( 0 ), m_bSorted ( true ) { Reserve ( 16384 ); }
	void			Add ( const SphGroupedValue_t & tValue )	{ CSphVector<SphGroupedValue_t>::Add ( tValue ); m_bSorted = false; }
	void			Sort ()										{ CSphVector<SphGroupedValue_t>::Sort (); m_bSorted = true; }

#else
					CSphUniqounter () : m_iCountPos ( 0 ) {}
#endif

public:
	int				CountStart ( SphGroupKey_t * pOutGroup );	///< starting counting distinct values, returns count and group key (0 if empty)
	int				CountNext ( SphGroupKey_t * pOutGroup );	///< continues counting distinct values, returns count and group key (0 if done)
	void			Compact ( SphGroupKey_t * pRemoveGroups, int iRemoveGroups );

protected:
	int				m_iCountPos;

#ifndef NDEBUG
	bool			m_bSorted;
#endif
};


int CSphUniqounter::CountStart ( SphGroupKey_t * pOutGroup )
{
	m_iCountPos = 0;
	return CountNext ( pOutGroup );
}


int CSphUniqounter::CountNext ( SphGroupKey_t * pOutGroup )
{
	assert ( m_bSorted );
	if ( m_iCountPos>=m_iLength )
		return 0;

	SphGroupKey_t uGroup = m_pData[m_iCountPos].m_uGroup;
	SphAttr_t uValue = m_pData[m_iCountPos].m_uValue;
	*pOutGroup = uGroup;

	int iCount = 1;
	while ( m_iCountPos<m_iLength && m_pData[m_iCountPos].m_uGroup==uGroup )
	{
		if ( m_pData[m_iCountPos].m_uValue!=uValue )
			iCount++;
		uValue = m_pData[m_iCountPos].m_uValue;
		m_iCountPos++;
	}
	return iCount;
}


void CSphUniqounter::Compact ( SphGroupKey_t * pRemoveGroups, int iRemoveGroups )
{
	assert ( m_bSorted );
	if ( !m_iLength )
		return;

	sphSort ( pRemoveGroups, iRemoveGroups );

	SphGroupedValue_t * pSrc = m_pData;
	SphGroupedValue_t * pDst = m_pData;
	SphGroupedValue_t * pEnd = m_pData + m_iLength;

	// skip remove-groups which are not in my list
	while ( iRemoveGroups && (*pRemoveGroups)<pSrc->m_uGroup )
	{
		pRemoveGroups++;
		iRemoveGroups--;
	}

	for ( ; pSrc<pEnd; pSrc++ )
	{
		// check if this entry needs to be removed
		while ( iRemoveGroups && (*pRemoveGroups)<pSrc->m_uGroup )
		{
			pRemoveGroups++;
			iRemoveGroups--;
		}
		if ( iRemoveGroups && pSrc->m_uGroup==*pRemoveGroups )
			continue;

		// check if it's a dupe
		if ( pDst>m_pData && pDst[-1]==pSrc[0] )
			continue;

		*pDst++ = *pSrc;
	}

	assert ( pDst-m_pData<=m_iLength );
	m_iLength = pDst-m_pData;
}

/////////////////////////////////////////////////////////////////////////////

/// attribute magic
enum
{
	SPH_VATTR_ID			= -1,	///< tells match sorter to use doc id
	SPH_VATTR_RELEVANCE		= -2,	///< tells match sorter to use match weight
	SPH_VATTR_FLOAT			= 10000	///< tells match sorter to compare floats
};


/// match comparator interface from group-by sorter point of view
struct ISphMatchComparator
{
	virtual ~ISphMatchComparator () {}
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState ) const = 0;
};


/// additional group-by sorter settings
struct CSphGroupSorterSettings
{
	CSphAttrLocator		m_tLocGroupby;		///< locator for @groupby
	CSphAttrLocator		m_tLocCount;		///< locator for @count
	CSphAttrLocator		m_tLocDistinct;		///< locator for @distinct
	CSphAttrLocator		m_tDistinctLoc;		///< locator for attribute to compute count(distinct) for
	bool				m_bDistinct;		///< whether we need distinct
	bool				m_bMVA;				///< whether we're grouping by MVA attribute
	bool				m_bMva64;
	CSphGrouper *		m_pGrouper;			///< group key calculator

	CSphGroupSorterSettings ()
		: m_bDistinct ( false )
		, m_bMVA ( false )
		, m_bMva64 ( false )
		, m_pGrouper ( NULL )
	{}
};


#if USE_WINDOWS
#pragma warning(disable:4127)
#endif


/// aggregate function interface
class IAggrFunc
{
public:
	virtual			~IAggrFunc() {}
	virtual void	Ungroup ( CSphMatch * ) {}
	virtual void	Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool bGrouped ) = 0;
	virtual void	Finalize ( CSphMatch * ) {}
};


/// aggregate traits for different attribute types
template < typename T >
class IAggrFuncTraits : public IAggrFunc
{
public:
	explicit		IAggrFuncTraits ( const CSphAttrLocator & tLocator ) : m_tLocator ( tLocator ) {}
	inline T		GetValue ( const CSphMatch * pRow );
	inline void		SetValue ( CSphMatch * pRow, T val );

protected:
	CSphAttrLocator	m_tLocator;
};

template<>
DWORD IAggrFuncTraits<DWORD>::GetValue ( const CSphMatch * pRow )
{
	return (DWORD)pRow->GetAttr ( m_tLocator );
}

template<>
void IAggrFuncTraits<DWORD>::SetValue ( CSphMatch * pRow, DWORD val )
{
	pRow->SetAttr ( m_tLocator, val );
}

template<>
int64_t IAggrFuncTraits<int64_t>::GetValue ( const CSphMatch * pRow )
{
	return pRow->GetAttr ( m_tLocator );
}

template<>
void IAggrFuncTraits<int64_t>::SetValue ( CSphMatch * pRow, int64_t val )
{
	pRow->SetAttr ( m_tLocator, val );
}

template<>
float IAggrFuncTraits<float>::GetValue ( const CSphMatch * pRow )
{
	return pRow->GetAttrFloat ( m_tLocator );
}

template<>
void IAggrFuncTraits<float>::SetValue ( CSphMatch * pRow, float val )
{
	pRow->SetAttrFloat ( m_tLocator, val );
}



/// SUM() implementation
template < typename T >
class AggrSum_t : public IAggrFuncTraits<T>
{
public:
	explicit AggrSum_t ( const CSphAttrLocator & tLoc ) : IAggrFuncTraits<T> ( tLoc )
	{}

	virtual void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool )
	{
		this->SetValue ( pDst, this->GetValue(pDst)+this->GetValue(pSrc) );
	}
};


/// AVG() implementation
template < typename T >
class AggrAvg_t : public IAggrFuncTraits<T>
{
protected:
	CSphAttrLocator m_tCountLoc;
public:
	AggrAvg_t ( const CSphAttrLocator & tLoc, const CSphAttrLocator & tCountLoc ) : IAggrFuncTraits<T> ( tLoc ), m_tCountLoc ( tCountLoc )
	{}

	virtual void Ungroup ( CSphMatch * pDst )
	{
		this->SetValue ( pDst, T ( this->GetValue ( pDst ) * pDst->GetAttr ( m_tCountLoc ) ) );
	}

	virtual void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool bGrouped )
	{
		if ( bGrouped )
			this->SetValue ( pDst, T ( this->GetValue ( pDst ) + this->GetValue ( pSrc ) * pSrc->GetAttr ( m_tCountLoc ) ) );
		else
			this->SetValue ( pDst, this->GetValue ( pDst ) + this->GetValue ( pSrc ) );
	}

	virtual void Finalize ( CSphMatch * pDst )
	{
		this->SetValue ( pDst, T ( this->GetValue ( pDst ) / pDst->GetAttr ( m_tCountLoc ) ) );
	}
};


/// MAX() implementation
template < typename T >
class AggrMax_t : public IAggrFuncTraits<T>
{
public:
	explicit AggrMax_t ( const CSphAttrLocator & tLoc ) : IAggrFuncTraits<T> ( tLoc )
	{}

	virtual void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool )
	{
		this->SetValue ( pDst, Max ( this->GetValue(pDst), this->GetValue(pSrc) ) );
	}
};


/// MIN() implementation
template < typename T >
class AggrMin_t : public IAggrFuncTraits<T>
{
public:
	explicit AggrMin_t ( const CSphAttrLocator & tLoc ) : IAggrFuncTraits<T> ( tLoc )
	{}

	virtual void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool )
	{
		this->SetValue ( pDst, Min ( this->GetValue(pDst), this->GetValue(pSrc) ) );
	}
};


/// group sorting functor
template < typename COMPGROUP >
struct GroupSorter_fn : public CSphMatchComparatorState, public SphAccessor_T<CSphMatch>
{
	typedef CSphMatch MEDIAN_TYPE;

	int m_iDynamic;

	GroupSorter_fn ()
	{
		m_iDynamic = 0;
	}

	void CopyKey ( MEDIAN_TYPE * pMed, CSphMatch * pVal ) const
	{
		pMed->Clone ( *pVal, m_iDynamic );
	}

	bool IsLess ( const CSphMatch & a, const CSphMatch & b ) const
	{
		return COMPGROUP::IsLess ( b, a, *this );
	}

	// inherited swap does not work on gcc
	void Swap ( CSphMatch * a, CSphMatch * b ) const
	{
		::Swap ( *a, *b );
	}
};

/// match sorter with k-buffering and group-by
template < typename COMPGROUP, bool DISTINCT >
class CSphKBufferGroupSorter : public CSphMatchQueueTraits
{
protected:
	ESphGroupBy		m_eGroupBy;			///< group-by function
	CSphGrouper *	m_pGrouper;

	CSphFixedHash < CSphMatch *, SphGroupKey_t, IdentityHash_fn >	m_hGroup2Match;

protected:
	int				m_iLimit;		///< max matches to be retrieved

	CSphUniqounter	m_tUniq;
	bool			m_bSortByDistinct;

	GroupSorter_fn<COMPGROUP>	m_tGroupSorter;
	const ISphMatchComparator *	m_pComp;

	CSphGroupSorterSettings		m_tSettings;
	CSphVector<IAggrFunc *>		m_dAggregates;
	CSphVector<IAggrFunc *>		m_dAvgs;
	int							m_iPregroupDynamic;	///< how much dynamic attributes are computed by the index (before groupby sorter)

	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

public:
	/// ctor
	CSphKBufferGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings ) // FIXME! make k configurable
		: CSphMatchQueueTraits ( pQuery->m_iMaxMatches*GROUPBY_FACTOR, true )
		, m_eGroupBy ( pQuery->m_eGroupFunc )
		, m_pGrouper ( tSettings.m_pGrouper )
		, m_hGroup2Match ( pQuery->m_iMaxMatches*GROUPBY_FACTOR )
		, m_iLimit ( pQuery->m_iMaxMatches )
		, m_bSortByDistinct ( false )
		, m_pComp ( pComp )
		, m_tSettings ( tSettings )
		, m_iPregroupDynamic ( 0 )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( DISTINCT==false || tSettings.m_tDistinctLoc.m_iBitOffset>=0 );
	}

	/// schema setup
	virtual void SetSchema ( const CSphSchema & tSchema )
	{
		m_tSchema = tSchema;
		m_tGroupSorter.m_iDynamic = m_tSchema.GetDynamicSize();

		bool bAggrStarted = false;
		for ( int i=0; i<m_tSchema.GetAttrsCount(); i++ )
		{
			const CSphColumnInfo & tAttr = m_tSchema.GetAttr(i);
			bool bMagicAggr = IsGroupbyMagic ( tAttr.m_sName ) || sphIsSortStringInternal ( tAttr.m_sName.cstr() ); // magic legacy aggregates

			if ( tAttr.m_eAggrFunc==SPH_AGGR_NONE && !bMagicAggr )
			{
				if ( !bAggrStarted )
					continue;

				// !COMMIT
				assert ( 0 && "internal error: aggregates must not be followed by non-aggregates" );
			}

			if ( !bAggrStarted )
			{
				assert ( ( tAttr.m_tLocator.m_iBitOffset % ROWITEM_BITS )==0 );
				m_iPregroupDynamic = tAttr.m_tLocator.m_iBitOffset / ROWITEM_BITS;
			}

			bAggrStarted = true;
			if ( bMagicAggr )
				continue;

			switch ( tAttr.m_eAggrFunc )
			{
				case SPH_AGGR_SUM:
					switch ( tAttr.m_eAttrType )
					{
						case SPH_ATTR_INTEGER:	m_dAggregates.Add ( new AggrSum_t<DWORD> ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_BIGINT:	m_dAggregates.Add ( new AggrSum_t<int64_t> ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_FLOAT:	m_dAggregates.Add ( new AggrSum_t<float> ( tAttr.m_tLocator ) ); break;
						default:				assert ( 0 && "internal error: unhandled aggregate type" ); break;
					}
					break;

				case SPH_AGGR_AVG:
					switch ( tAttr.m_eAttrType )
					{
						case SPH_ATTR_INTEGER:	m_dAggregates.Add ( new AggrAvg_t<DWORD> ( tAttr.m_tLocator, m_tSettings.m_tLocCount ) ); break;
						case SPH_ATTR_BIGINT:	m_dAggregates.Add ( new AggrAvg_t<int64_t> ( tAttr.m_tLocator, m_tSettings.m_tLocCount ) ); break;
						case SPH_ATTR_FLOAT:	m_dAggregates.Add ( new AggrAvg_t<float> ( tAttr.m_tLocator, m_tSettings.m_tLocCount ) ); break;
						default:				assert ( 0 && "internal error: unhandled aggregate type" ); break;
					}
					// store avg to calculate these attributes prior to groups sort
					for ( int iState=0; iState<CSphMatchComparatorState::MAX_ATTRS; iState++ )
					{
						ESphSortKeyPart eKeypart = m_tGroupSorter.m_eKeypart[iState];
						CSphAttrLocator tLoc = m_tGroupSorter.m_tLocator[iState];
						if ( ( eKeypart==SPH_KEYPART_INT || eKeypart==SPH_KEYPART_FLOAT )
							&& tLoc.m_bDynamic==tAttr.m_tLocator.m_bDynamic && tLoc.m_iBitOffset==tAttr.m_tLocator.m_iBitOffset
							&& tLoc.m_iBitCount==tAttr.m_tLocator.m_iBitCount )
						{
								m_dAvgs.Add ( m_dAggregates.Last() );
								break;
						}
					}
					break;

				case SPH_AGGR_MIN:
					switch ( tAttr.m_eAttrType )
					{
						case SPH_ATTR_INTEGER:	m_dAggregates.Add ( new AggrMin_t<DWORD> ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_BIGINT:	m_dAggregates.Add ( new AggrMin_t<int64_t> ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_FLOAT:	m_dAggregates.Add ( new AggrMin_t<float> ( tAttr.m_tLocator ) ); break;
						default:				assert ( 0 && "internal error: unhandled aggregate type" ); break;
					}
					break;

				case SPH_AGGR_MAX:
					switch ( tAttr.m_eAttrType )
					{
						case SPH_ATTR_INTEGER:	m_dAggregates.Add ( new AggrMax_t<DWORD> ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_BIGINT:	m_dAggregates.Add ( new AggrMax_t<int64_t> ( tAttr.m_tLocator ) ); break;
						case SPH_ATTR_FLOAT:	m_dAggregates.Add ( new AggrMax_t<float> ( tAttr.m_tLocator ) ); break;
						default:				assert ( 0 && "internal error: unhandled aggregate type" ); break;
					}
					break;

				default:
					assert ( 0 && "internal error: unhandled aggregate function" );
					break;
			}
		}
	}

	/// dtor
	~CSphKBufferGroupSorter ()
	{
		SafeDelete ( m_pComp );
		SafeDelete ( m_pGrouper );
	}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
	{
		return true;
	}

	/// set string pool pointer (for string+groupby sorters)
	void SetStringPool ( const BYTE * pStrings )
	{
		m_pGrouper->SetStringPool ( pStrings );
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		SphGroupKey_t uGroupKey = m_pGrouper->KeyFromMatch ( tEntry );
		return PushEx ( tEntry, uGroupKey, false );
	}

	/// add grouped entry to the queue
	virtual bool PushGrouped ( const CSphMatch & tEntry )
	{
		return PushEx ( tEntry, tEntry.GetAttr ( m_tSettings.m_tLocGroupby ), true );
	}

	/// add entry to the queue
	virtual bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bGrouped )
	{
		// if this group is already hashed, we only need to update the corresponding match
		CSphMatch ** ppMatch = m_hGroup2Match ( uGroupKey );
		if ( ppMatch )
		{
			CSphMatch * pMatch = (*ppMatch);
			assert ( pMatch );
			assert ( pMatch->GetAttr ( m_tSettings.m_tLocGroupby )==uGroupKey );
			assert ( pMatch->m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

			if ( bGrouped )
			{
				// it's already grouped match
				// sum grouped matches count
				pMatch->SetAttr ( m_tSettings.m_tLocCount, pMatch->GetAttr ( m_tSettings.m_tLocCount ) + tEntry.GetAttr ( m_tSettings.m_tLocCount ) ); // OPTIMIZE! AddAttr()?
				if ( DISTINCT )
					pMatch->SetAttr ( m_tSettings.m_tLocDistinct, pMatch->GetAttr ( m_tSettings.m_tLocDistinct ) + tEntry.GetAttr ( m_tSettings.m_tLocDistinct ) );
			} else
			{
				// it's a simple match
				// increase grouped matches count
				pMatch->SetAttr ( m_tSettings.m_tLocCount, 1 + pMatch->GetAttr ( m_tSettings.m_tLocCount ) ); // OPTIMIZE! IncAttr()?
			}

			// update aggregates
			ARRAY_FOREACH ( i, m_dAggregates )
				m_dAggregates[i]->Update ( pMatch, &tEntry, bGrouped );

			// if new entry is more relevant, update from it
			if ( m_pComp->VirtualIsLess ( *pMatch, tEntry, m_tState ) )
			{
				// can't use Clone() here; must keep current aggregate values
				pMatch->m_iDocID = tEntry.m_iDocID;
				pMatch->m_iWeight = tEntry.m_iWeight;
				pMatch->m_pStatic = tEntry.m_pStatic;
				pMatch->m_iTag = tEntry.m_iTag;

				if ( m_iPregroupDynamic )
				{
					assert ( pMatch->m_pDynamic );
					assert ( tEntry.m_pDynamic );
					assert ( pMatch->m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

					for ( int i=0; i<m_iPregroupDynamic; i++ )
						pMatch->m_pDynamic[i] = tEntry.m_pDynamic[i];
				}
			}
		}

		// submit actual distinct value in all cases
		if ( DISTINCT && !bGrouped )
			m_tUniq.Add ( SphGroupedValue_t ( uGroupKey, tEntry.GetAttr ( m_tSettings.m_tDistinctLoc ) ) ); // OPTIMIZE! use simpler locator here?

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( ppMatch )
			return false;

		// if we're full, let's cut off some worst groups
		if ( m_iUsed==m_iSize )
			CutWorst ( m_iLimit * (int)(GROUPBY_FACTOR/2) );

		// do add
		assert ( m_iUsed<m_iSize );
		CSphMatch & tNew = m_pData [ m_iUsed++ ];
		tNew.Clone ( tEntry, m_tSchema.GetDynamicSize() );

		if ( !bGrouped )
		{
			tNew.SetAttr ( m_tSettings.m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( m_tSettings.m_tLocCount, 1 );
			if ( DISTINCT )
				tNew.SetAttr ( m_tSettings.m_tLocDistinct, 0 );
		} else
		{
			ARRAY_FOREACH ( i, m_dAggregates )
				m_dAggregates[i]->Ungroup ( &tNew );
		}

		m_hGroup2Match.Add ( &tNew, uGroupKey );
		m_iTotal++;
		return true;
	}

	void CalcAvg ( bool bGroup )
	{
		if ( !m_dAvgs.GetLength() )
			return;

		CSphMatch * pMatch = m_pData;
		CSphMatch * pEnd = pMatch + m_iUsed;
		while ( pMatch<pEnd )
		{
			ARRAY_FOREACH ( j, m_dAvgs )
			{
				if ( bGroup )
					m_dAvgs[j]->Finalize ( pMatch );
				else
					m_dAvgs[j]->Ungroup ( pMatch );
			}
			++pMatch;
		}
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	void Flatten ( CSphMatch * pTo, int iTag )
	{
		CountDistinct ();

		CalcAvg ( true );
		SortGroups ();

		CSphVector<IAggrFunc *> dAggrs;
		if ( m_dAggregates.GetLength()!=m_dAvgs.GetLength() )
		{
			dAggrs = m_dAggregates;
			ARRAY_FOREACH ( i, m_dAvgs )
				dAggrs.RemoveValue ( m_dAvgs[i] );
		}

		int iLen = GetLength ();
		for ( int i=0; i<iLen; i++, pTo++ )
		{
			ARRAY_FOREACH ( j, dAggrs )
				dAggrs[j]->Finalize ( &m_pData[i] );

			pTo->Clone ( m_pData[i], m_tSchema.GetDynamicSize() );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
		}

		m_iUsed = 0;
		m_iTotal = 0;

		m_hGroup2Match.Reset ();
		if ( DISTINCT )
			m_tUniq.Resize ( 0 );
	}

	/// get entries count
	int GetLength () const
	{
		return Min ( m_iUsed, m_iLimit );
	}

	/// set group comparator state
	void SetGroupState ( const CSphMatchComparatorState & tState )
	{
		m_tGroupSorter.m_fnStrCmp = tState.m_fnStrCmp;

		// FIXME! manual bitwise copying.. yuck
		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
		{
			m_tGroupSorter.m_eKeypart[i] = tState.m_eKeypart[i];
			m_tGroupSorter.m_tLocator[i] = tState.m_tLocator[i];
		}
		m_tGroupSorter.m_uAttrDesc = tState.m_uAttrDesc;
		m_tGroupSorter.m_iNow = tState.m_iNow;

		// check whether we sort by distinct
		if ( DISTINCT && m_tSettings.m_tDistinctLoc.m_iBitOffset>=0 )
			for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
				if ( m_tGroupSorter.m_tLocator[i].m_iBitOffset==m_tSettings.m_tDistinctLoc.m_iBitOffset )
			{
				m_bSortByDistinct = true;
				break;
			}
	}

protected:
	/// count distinct values if necessary
	void CountDistinct ()
	{
		if ( DISTINCT )
		{
			m_tUniq.Sort ();
			SphGroupKey_t uGroup;
			for ( int iCount = m_tUniq.CountStart ( &uGroup ); iCount; iCount = m_tUniq.CountNext ( &uGroup ) )
			{
				CSphMatch ** ppMatch = m_hGroup2Match(uGroup);
				if ( ppMatch )
					(*ppMatch)->SetAttr ( m_tSettings.m_tLocDistinct, iCount );
			}
		}
	}

	/// cut worst N groups off the buffer tail
	void CutWorst ( int iCut )
	{
		// sort groups
		if ( m_bSortByDistinct )
			CountDistinct ();

		CalcAvg ( true );
		SortGroups ();
		CalcAvg ( false );

		// cut groups
		m_iUsed -= iCut;

		// cleanup unused distinct stuff
		if ( DISTINCT )
		{
			// build kill-list
			CSphVector<SphGroupKey_t> dRemove;
			dRemove.Resize ( iCut );
			for ( int i=0; i<iCut; i++ )
				dRemove[i] = m_pData[m_iUsed+i].GetAttr ( m_tSettings.m_tLocGroupby );

			// sort and compact
			if ( !m_bSortByDistinct )
				m_tUniq.Sort ();
			m_tUniq.Compact ( &dRemove[0], iCut );
		}

		// rehash
		m_hGroup2Match.Reset ();
		for ( int i=0; i<m_iUsed; i++ )
			m_hGroup2Match.Add ( m_pData+i, m_pData[i].GetAttr ( m_tSettings.m_tLocGroupby ) );
	}

	/// sort groups buffer
	void SortGroups ()
	{
		sphSort ( m_pData, m_iUsed, m_tGroupSorter, m_tGroupSorter );
	}

	virtual CSphMatch * Finalize()
	{
		if ( m_iUsed>m_iLimit )
			CutWorst ( m_iUsed - m_iLimit );

		return m_pData;
	}
};


/// match sorter with k-buffering and group-by for MVAs
template < typename COMPGROUP, bool DISTINCT >
class CSphKBufferMVAGroupSorter : public CSphKBufferGroupSorter < COMPGROUP, DISTINCT >
{
protected:
	const DWORD *		m_pMva;		///< pointer to MVA pool for incoming matches
	CSphAttrLocator		m_tMvaLocator;
	bool				m_bMva64;

public:
	/// ctor
	CSphKBufferMVAGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: CSphKBufferGroupSorter < COMPGROUP, DISTINCT > ( pComp, pQuery, tSettings )
		, m_pMva ( NULL )
		, m_bMva64 ( tSettings.m_bMva64 )
	{
		this->m_pGrouper->GetLocator ( m_tMvaLocator );
	}

	/// check if this sorter does groupby
	virtual bool IsGroupby ()
	{
		return true;
	}

	/// set MVA pool for subsequent matches
	void SetMVAPool ( const DWORD * pMva )
	{
		m_pMva = pMva;
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		assert ( m_pMva );
		if ( !m_pMva )
			return false;

		// get that list
		// FIXME! OPTIMIZE! use simpler locator than full bits/count here
		// FIXME! hardcoded MVA type, so here's MVA_DOWNSIZE marker for searching
		const DWORD * pValues = tEntry.GetAttrMVA ( this->m_tMvaLocator, m_pMva ); // (this pointer is for gcc; it doesn't work otherwise)
		if ( !pValues )
			return false;

		DWORD iValues = *pValues++;

		bool bRes = false;
		if ( m_bMva64 )
		{
			assert ( ( iValues%2 )==0 );
			for ( ;iValues>0; iValues-=2, pValues+=2 )
			{
				uint64_t uMva = MVA_UPSIZE ( pValues );
				SphGroupKey_t uGroupkey = this->m_pGrouper->KeyFromValue ( uMva );
				bRes |= this->PushEx ( tEntry, uGroupkey, false );
			}

		} else
		{
			while ( iValues-- )
			{
				SphGroupKey_t uGroupkey = this->m_pGrouper->KeyFromValue ( *pValues++ );
				bRes |= this->PushEx ( tEntry, uGroupkey, false );
			}
		}
		return bRes;
	}

	/// add pre-grouped entry to the queue
	virtual bool PushGrouped ( const CSphMatch & tEntry )
	{
		// re-group it based on the group key
		// (first 'this' is for icc; second 'this' is for gcc)
		return this->PushEx ( tEntry, tEntry.GetAttr ( this->m_tSettings.m_tLocGroupby ), true );
	}
};


#if USE_WINDOWS
#pragma warning(default:4127)
#endif

//////////////////////////////////////////////////////////////////////////
// PLAIN SORTING FUNCTORS
//////////////////////////////////////////////////////////////////////////

/// match sorter
struct MatchRelevanceLt_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & )
	{
		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
struct MatchAttrLt_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		if ( t.m_eKeypart[0]!=SPH_KEYPART_STRING )
		{
			SphAttr_t aa = a.GetAttr ( t.m_tLocator[0] );
			SphAttr_t bb = b.GetAttr ( t.m_tLocator[0] );
			if ( aa!=bb )
				return aa<bb;
		} else
		{
			int iCmp = t.CmpStrings ( a, b, 0 );
			if ( iCmp!=0 )
				return iCmp<0;
		}

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
struct MatchAttrGt_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		if ( t.m_eKeypart[0]!=SPH_KEYPART_STRING )
		{
			SphAttr_t aa = a.GetAttr ( t.m_tLocator[0] );
			SphAttr_t bb = b.GetAttr ( t.m_tLocator[0] );
			if ( aa!=bb )
				return aa>bb;
		} else
		{
			int iCmp = t.CmpStrings ( a, b, 0 );
			if ( iCmp!=0 )
				return iCmp>0;
		}

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
struct MatchTimeSegments_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphAttr_t aa = a.GetAttr ( t.m_tLocator[0] );
		SphAttr_t bb = b.GetAttr ( t.m_tLocator[0] );
		int iA = GetSegment ( aa, t.m_iNow );
		int iB = GetSegment ( bb, t.m_iNow );
		if ( iA!=iB )
			return iA > iB;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		if ( aa!=bb )
			return aa<bb;

		return a.m_iDocID > b.m_iDocID;
	};

protected:
	static inline int GetSegment ( SphAttr_t iStamp, SphAttr_t iNow )
	{
		if ( iStamp>=iNow-3600 ) return 0; // last hour
		if ( iStamp>=iNow-24*3600 ) return 1; // last day
		if ( iStamp>=iNow-7*24*3600 ) return 2; // last week
		if ( iStamp>=iNow-30*24*3600 ) return 3; // last month
		if ( iStamp>=iNow-90*24*3600 ) return 4; // last 3 months
		return 5; // everything else
	}
};


/// match sorter
struct MatchExpr_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		float aa = a.GetAttrFloat ( t.m_tLocator[0] ); // FIXME! OPTIMIZE!!! simplified (dword-granular) getter could be used here
		float bb = b.GetAttrFloat ( t.m_tLocator[0] );
		if ( aa!=bb )
			return aa<bb;
		return a.m_iDocID>b.m_iDocID;
	}
};

/////////////////////////////////////////////////////////////////////////////

#define SPH_TEST_PAIR(_aa,_bb,_idx ) \
	if ( (_aa)!=(_bb) ) \
		return ( (t.m_uAttrDesc >> (_idx)) & 1 ) ^ ( (_aa) > (_bb) );


#define SPH_TEST_KEYPART(_idx) \
	switch ( t.m_eKeypart[_idx] ) \
	{ \
		case SPH_KEYPART_ID:		SPH_TEST_PAIR ( a.m_iDocID, b.m_iDocID, _idx ); break; \
		case SPH_KEYPART_WEIGHT:	SPH_TEST_PAIR ( a.m_iWeight, b.m_iWeight, _idx ); break; \
		case SPH_KEYPART_INT: \
		{ \
			register SphAttr_t aa = a.GetAttr ( t.m_tLocator[_idx] ); \
			register SphAttr_t bb = b.GetAttr ( t.m_tLocator[_idx] ); \
			SPH_TEST_PAIR ( aa, bb, _idx ); \
			break; \
		} \
		case SPH_KEYPART_FLOAT: \
		{ \
			register float aa = a.GetAttrFloat ( t.m_tLocator[_idx] ); \
			register float bb = b.GetAttrFloat ( t.m_tLocator[_idx] ); \
			SPH_TEST_PAIR ( aa, bb, _idx ) \
			break; \
		} \
		case SPH_KEYPART_STRING: \
		{ \
			int iCmp = t.CmpStrings ( a, b, _idx ); \
			if ( iCmp!=0 ) \
				return ( ( t.m_uAttrDesc >> (_idx) ) & 1 ) ^ ( iCmp>0 ); \
			break; \
		} \
	}


struct MatchGeneric2_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		return false;
	};
};


struct MatchGeneric3_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		return false;
	};
};


struct MatchGeneric4_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_KEYPART(3);
		return false;
	};
};


struct MatchGeneric5_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_KEYPART(3);
		SPH_TEST_KEYPART(4);
		return false;
	};
};

//////////////////////////////////////////////////////////////////////////

struct MatchCustom_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	// setup sorting state
	static bool SetupAttr ( const CSphSchema & tSchema, CSphMatchComparatorState & tState, CSphString & sError, int iIdx, const char * sAttr )
	{
		if ( iIdx>=CSphMatchComparatorState::MAX_ATTRS )
		{
			sError.SetSprintf ( "custom sort: too many attributes declared" );
			return false;
		}

		int iAttr = tSchema.GetAttrIndex(sAttr);
		if ( iAttr<0 )
		{
			sError.SetSprintf ( "custom sort: attr '%s' not found in schema", sAttr );
			return false;
		}

		const CSphColumnInfo & tAttr = tSchema.GetAttr(iAttr);
		tState.m_eKeypart[iIdx] = tAttr.m_eAttrType==SPH_ATTR_FLOAT ? SPH_KEYPART_FLOAT : SPH_KEYPART_INT;
		tState.m_tLocator[iIdx] = tAttr.m_tLocator;
		return true;
	}

	// setup sorting state
	static bool Setup ( const CSphSchema & tSchema, CSphMatchComparatorState & tState, CSphString & sError )
	{
		float fTmp;
		int iAttr = 0;

#define MATCH_FUNCTION				fTmp
#define MATCH_WEIGHT				1.0f
#define MATCH_NOW					1.0f
#define MATCH_ATTR(_idx)			1.0f
#define MATCH_DECLARE_ATTR(_name)	if ( !SetupAttr ( tSchema, tState, sError, iAttr++, _name ) ) return false;
#include "sphinxcustomsort.inl"
; // NOLINT

		return true;
	}

	// calc function and compare matches
	// OPTIMIZE? could calc once per match on submit
	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
#undef MATCH_DECLARE_ATTR
#undef MATCH_WEIGHT
#undef MATCH_NOW
#undef MATCH_ATTR
#define MATCH_DECLARE_ATTR(_name)	; // NOLINT
#define MATCH_WEIGHT				float(MATCH_VAR.m_iWeight)
#define MATCH_NOW					float(t.m_iNow)
#define MATCH_ATTR(_idx)			float(MATCH_VAR.GetAttr(t.m_tLocator[_idx]))

		float aa, bb;

#undef MATCH_FUNCTION
#undef MATCH_VAR
#define MATCH_FUNCTION aa
#define MATCH_VAR a
#include "sphinxcustomsort.inl" // NOLINT
; // NOLINT

#undef MATCH_FUNCTION
#undef MATCH_VAR
#define MATCH_FUNCTION bb
#define MATCH_VAR b
#include "sphinxcustomsort.inl" // NOLINT
; // NOLINT

		return aa<bb;
	}
};

//////////////////////////////////////////////////////////////////////////
// SORT CLAUSE PARSER
//////////////////////////////////////////////////////////////////////////

static const int MAX_SORT_FIELDS = 5; // MUST be in sync with CSphMatchComparatorState::m_iAttr


enum ESphSortFunc
{
	FUNC_REL_DESC,
	FUNC_ATTR_DESC,
	FUNC_ATTR_ASC,
	FUNC_TIMESEGS,
	FUNC_GENERIC2,
	FUNC_GENERIC3,
	FUNC_GENERIC4,
	FUNC_GENERIC5,
	FUNC_CUSTOM,
	FUNC_EXPR
};


enum ESortClauseParseResult
{
	SORT_CLAUSE_OK,
	SORT_CLAUSE_ERROR,
	SORT_CLAUSE_RANDOM
};


class SortClauseTokenizer_t
{
protected:
	char * m_pCur;
	char * m_pMax;
	char * m_pBuf;

protected:
	char ToLower ( char c )
	{
		// 0..9, A..Z->a..z, _, a..z, @
		if ( ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || c=='_' || c=='@' )
			return c;
		if ( c>='A' && c<='Z' )
			return c-'A'+'a';
		return 0;
	}

public:
	explicit SortClauseTokenizer_t ( const char * sBuffer )
	{
		int iLen = strlen(sBuffer);
		m_pBuf = new char [ iLen+1 ];
		m_pMax = m_pBuf+iLen;
		m_pCur = m_pBuf;

		for ( int i=0; i<=iLen; i++ )
			m_pBuf[i] = ToLower ( sBuffer[i] );
	}

	~SortClauseTokenizer_t ()
	{
		SafeDeleteArray ( m_pBuf );
	}

	const char * GetToken ()
	{
		// skip spaces
		while ( m_pCur<m_pMax && !*m_pCur )
			m_pCur++;
		if ( m_pCur>=m_pMax )
			return NULL;

		// memorize token start, and move pointer forward
		const char * sRes = m_pCur;
		while ( *m_pCur )
			m_pCur++;
		return sRes;
	}
};


static inline ESphSortKeyPart Attr2Keypart ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_FLOAT:	return SPH_KEYPART_FLOAT;
		case SPH_ATTR_STRING:	return SPH_KEYPART_STRING;
		default:				return SPH_KEYPART_INT;
	}
}


static ESortClauseParseResult sphParseSortClause ( const CSphQuery * pQuery, const char * sClause, const CSphSchema & tSchema,
	ESphSortFunc & eFunc, CSphMatchComparatorState & tState, int * dAttrs, CSphString & sError, CSphSchema * pExtra = NULL )
{
	assert ( dAttrs );
	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
		dAttrs[i] = -1;

	// mini parser
	SortClauseTokenizer_t tTok ( sClause );

	bool bField = false; // whether i'm expecting field name or sort order
	int iField = 0;

	for ( const char * pTok=tTok.GetToken(); pTok; pTok=tTok.GetToken() )
	{
		bField = !bField;

		// special case, sort by random
		if ( iField==0 && bField && strcmp ( pTok, "@random" )==0 )
			return SORT_CLAUSE_RANDOM;

		// special case, sort by custom
		if ( iField==0 && bField && strcmp ( pTok, "@custom" )==0 )
		{
			eFunc = FUNC_CUSTOM;
			return MatchCustom_fn::Setup ( tSchema, tState, sError ) ? SORT_CLAUSE_OK : SORT_CLAUSE_ERROR;
		}

		// handle sort order
		if ( !bField )
		{
			// check
			if ( strcmp ( pTok, "desc" ) && strcmp ( pTok, "asc" ) )
			{
				sError.SetSprintf ( "invalid sorting order '%s'", pTok );
				return SORT_CLAUSE_ERROR;
			}

			// set
			if ( !strcmp ( pTok, "desc" ) )
				tState.m_uAttrDesc |= ( 1<<iField );

			iField++;
			continue;
		}

		// handle attribute name
		if ( iField==MAX_SORT_FIELDS )
		{
			sError.SetSprintf ( "too many sort-by attributes; maximum count is %d", MAX_SORT_FIELDS );
			return SORT_CLAUSE_ERROR;
		}

		if ( !strcasecmp ( pTok, "@relevance" )
			|| !strcasecmp ( pTok, "@rank" )
			|| !strcasecmp ( pTok, "@weight" ) )
		{
			tState.m_eKeypart[iField] = SPH_KEYPART_WEIGHT;

		} else if ( !strcasecmp ( pTok, "@id" ) || !strcasecmp ( pTok, "id" ) )
		{
			tState.m_eKeypart[iField] = SPH_KEYPART_ID;

		} else
		{
			if ( !strcasecmp ( pTok, "@group" ) )
				pTok = "@groupby";

			// try to lookup plain attr in sorter schema
			int iAttr = tSchema.GetAttrIndex ( pTok );

			// try to lookup aliased count(*) in select items
			if ( iAttr<0 )
			{
				ARRAY_FOREACH ( i, pQuery->m_dItems )
				{
					const CSphQueryItem & tItem = pQuery->m_dItems[i];
					if ( !tItem.m_sAlias.cstr() || strcasecmp ( tItem.m_sAlias.cstr(), pTok ) )
						continue;
					if ( tItem.m_sExpr.Begins("@") )
						iAttr = tSchema.GetAttrIndex ( tItem.m_sExpr.cstr() );
					break; // break in any case; because we did match the alias
				}
			}

			// epic fail
			if ( iAttr<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pTok );
				return SORT_CLAUSE_ERROR;
			}

			const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
			if ( pExtra )
				pExtra->AddAttr ( tCol, true );
			tState.m_eKeypart[iField] = Attr2Keypart ( tCol.m_eAttrType );
			tState.m_tLocator[iField] = tSchema.GetAttr(iAttr).m_tLocator;
			dAttrs[iField] = iAttr;
		}
	}

	if ( iField==0 )
	{
		sError.SetSprintf ( "no sort order defined" );
		return SORT_CLAUSE_ERROR;
	}

	if ( iField==1 )
		tState.m_eKeypart[iField++] = SPH_KEYPART_ID; // add "id ASC"

	switch ( iField )
	{
		case 2:		eFunc = FUNC_GENERIC2; break;
		case 3:		eFunc = FUNC_GENERIC3; break;
		case 4:		eFunc = FUNC_GENERIC4; break;
		case 5:		eFunc = FUNC_GENERIC5; break;
		default:	sError.SetSprintf ( "INTERNAL ERROR: %d fields in sphParseSortClause()", iField ); return SORT_CLAUSE_ERROR;
	}
	return SORT_CLAUSE_OK;
}

//////////////////////////////////////////////////////////////////////////
// SORTING+GROUPING INSTANTIATION
//////////////////////////////////////////////////////////////////////////

template < typename COMPGROUP >
static ISphMatchSorter * sphCreateSorter3rd ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
{
	if ( tSettings.m_bMVA )
	{
		if ( tSettings.m_bDistinct==true )
			return new CSphKBufferMVAGroupSorter < COMPGROUP, true > ( pComp, pQuery, tSettings);
		else
			return new CSphKBufferMVAGroupSorter < COMPGROUP, false > ( pComp, pQuery, tSettings );
	} else
	{
		if ( tSettings.m_bDistinct==true )
			return new CSphKBufferGroupSorter < COMPGROUP, true > ( pComp, pQuery, tSettings );
		else
			return new CSphKBufferGroupSorter < COMPGROUP, false > ( pComp, pQuery, tSettings );
	}
}


static ISphMatchSorter * sphCreateSorter2nd ( ESphSortFunc eGroupFunc, const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
{
	switch ( eGroupFunc )
	{
		case FUNC_GENERIC2:		return sphCreateSorter3rd<MatchGeneric2_fn>	( pComp, pQuery, tSettings ); break;
		case FUNC_GENERIC3:		return sphCreateSorter3rd<MatchGeneric3_fn>	( pComp, pQuery, tSettings ); break;
		case FUNC_GENERIC4:		return sphCreateSorter3rd<MatchGeneric4_fn>	( pComp, pQuery, tSettings ); break;
		case FUNC_GENERIC5:		return sphCreateSorter3rd<MatchGeneric5_fn>	( pComp, pQuery, tSettings ); break;
		case FUNC_CUSTOM:		return sphCreateSorter3rd<MatchCustom_fn>	( pComp, pQuery, tSettings ); break;
		case FUNC_EXPR:			return sphCreateSorter3rd<MatchExpr_fn>		( pComp, pQuery, tSettings ); break;
		default:				return NULL;
	}
}


static ISphMatchSorter * sphCreateSorter1st ( ESphSortFunc eMatchFunc, ESphSortFunc eGroupFunc, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
{
	ISphMatchComparator * pComp = NULL;
	switch ( eMatchFunc )
	{
		case FUNC_REL_DESC:		pComp = new MatchRelevanceLt_fn(); break;
		case FUNC_ATTR_DESC:	pComp = new MatchAttrLt_fn(); break;
		case FUNC_ATTR_ASC:		pComp = new MatchAttrGt_fn(); break;
		case FUNC_TIMESEGS:		pComp = new MatchTimeSegments_fn(); break;
		case FUNC_GENERIC2:		pComp = new MatchGeneric2_fn(); break;
		case FUNC_GENERIC3:		pComp = new MatchGeneric3_fn(); break;
		case FUNC_GENERIC4:		pComp = new MatchGeneric4_fn(); break;
		case FUNC_GENERIC5:		pComp = new MatchGeneric5_fn(); break;
		case FUNC_CUSTOM:		pComp = new MatchCustom_fn(); break;
		case FUNC_EXPR:			pComp = new MatchExpr_fn(); break; // only for non-bitfields, obviously
	}

	assert ( pComp );
	return sphCreateSorter2nd ( eGroupFunc, pComp, pQuery, tSettings );
}

//////////////////////////////////////////////////////////////////////////
// GEODIST
//////////////////////////////////////////////////////////////////////////

struct ExprGeodist_t : public ISphExpr
{
public:
						ExprGeodist_t () {}
	bool				Setup ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sError );
	virtual float		Eval ( const CSphMatch & tMatch ) const;
	virtual void		SetMVAPool ( const DWORD * ) {}
	virtual void		GetDependencyColumns ( CSphVector<int> & dColumns ) const;

protected:
	CSphAttrLocator		m_tGeoLatLoc;
	CSphAttrLocator		m_tGeoLongLoc;
	float				m_fGeoAnchorLat;
	float				m_fGeoAnchorLong;
	int					m_iLat;
	int					m_iLon;
};


bool ExprGeodist_t::Setup ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sError )
{
	if ( !pQuery->m_bGeoAnchor )
	{
		sError.SetSprintf ( "INTERNAL ERROR: no geoanchor, can not create geodist evaluator" );
		return false;
	}

	int iLat = tSchema.GetAttrIndex ( pQuery->m_sGeoLatAttr.cstr() );
	if ( iLat<0 )
	{
		sError.SetSprintf ( "unknown latitude attribute '%s'", pQuery->m_sGeoLatAttr.cstr() );
		return false;
	}

	int iLong = tSchema.GetAttrIndex ( pQuery->m_sGeoLongAttr.cstr() );
	if ( iLong<0 )
	{
		sError.SetSprintf ( "unknown latitude attribute '%s'", pQuery->m_sGeoLongAttr.cstr() );
		return false;
	}

	m_tGeoLatLoc = tSchema.GetAttr(iLat).m_tLocator;
	m_tGeoLongLoc = tSchema.GetAttr(iLong).m_tLocator;
	m_fGeoAnchorLat = pQuery->m_fGeoLatitude;
	m_fGeoAnchorLong = pQuery->m_fGeoLongitude;
	m_iLat = iLat;
	m_iLon = iLong;
	return true;
}


static inline double sphSqr ( double v )
{
	return v*v;
}


float ExprGeodist_t::Eval ( const CSphMatch & tMatch ) const
{
	const double R = 6384000;
	float plat = tMatch.GetAttrFloat ( m_tGeoLatLoc );
	float plon = tMatch.GetAttrFloat ( m_tGeoLongLoc );
	double dlat = plat - m_fGeoAnchorLat;
	double dlon = plon - m_fGeoAnchorLong;
	double a = sphSqr ( sin ( dlat/2 ) ) + cos(plat)*cos(m_fGeoAnchorLat)*sphSqr(sin(dlon/2));
	double c = 2*asin ( Min ( 1, sqrt(a) ) );
	return (float)(R*c);
}

void ExprGeodist_t::GetDependencyColumns ( CSphVector<int> & dColumns ) const
{
	dColumns.Add ( m_iLat );
	dColumns.Add ( m_iLon );
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS (FACTORY AND FLATTENING)
//////////////////////////////////////////////////////////////////////////

static CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation );

static bool SetupGroupbySettings ( const CSphQuery * pQuery, const CSphSchema & tSchema, CSphGroupSorterSettings & tSettings, CSphString & sError )
{
	tSettings.m_tDistinctLoc.m_iBitOffset = -1;

	if ( pQuery->m_sGroupBy.IsEmpty() )
		return true;

	if ( pQuery->m_eGroupFunc==SPH_GROUPBY_ATTRPAIR )
	{
		sError.SetSprintf ( "SPH_GROUPBY_ATTRPAIR is not supported any more (just group on 'bigint' attribute)" );
		return false;
	}

	// setup groupby attr
	int iGroupBy = tSchema.GetAttrIndex ( pQuery->m_sGroupBy.cstr() );
	if ( iGroupBy<0 )
	{
		sError.SetSprintf ( "group-by attribute '%s' not found", pQuery->m_sGroupBy.cstr() );
		return false;
	}

	ESphAttr eType = tSchema.GetAttr ( iGroupBy ).m_eAttrType;
	CSphAttrLocator tLoc = tSchema.GetAttr ( iGroupBy ).m_tLocator;
	switch ( pQuery->m_eGroupFunc )
	{
		case SPH_GROUPBY_DAY:		tSettings.m_pGrouper = new CSphGrouperDay ( tLoc ); break;
		case SPH_GROUPBY_WEEK:		tSettings.m_pGrouper = new CSphGrouperWeek ( tLoc ); break;
		case SPH_GROUPBY_MONTH:		tSettings.m_pGrouper = new CSphGrouperMonth ( tLoc ); break;
		case SPH_GROUPBY_YEAR:		tSettings.m_pGrouper = new CSphGrouperYear ( tLoc ); break;
		case SPH_GROUPBY_ATTR:
		{
			if ( eType!=SPH_ATTR_STRING )
				tSettings.m_pGrouper = new CSphGrouperAttr ( tLoc );
			else
				tSettings.m_pGrouper = sphCreateGrouperString ( tLoc, pQuery->m_eCollation );
		}
		break;
		default:
			sError.SetSprintf ( "invalid group-by mode (mode=%d)", pQuery->m_eGroupFunc );
			return false;
	}

	tSettings.m_bMVA = ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_UINT64SET );
	tSettings.m_bMva64 = ( eType==SPH_ATTR_UINT64SET );

	// setup distinct attr
	if ( !pQuery->m_sGroupDistinct.IsEmpty() )
	{
		int iDistinct = tSchema.GetAttrIndex ( pQuery->m_sGroupDistinct.cstr() );
		if ( iDistinct<0 )
		{
			sError.SetSprintf ( "group-count-distinct attribute '%s' not found", pQuery->m_sGroupDistinct.cstr() );
			return false;
		}

		tSettings.m_tDistinctLoc = tSchema.GetAttr ( iDistinct ).m_tLocator;
	}

	return true;
}


static bool FixupDependency ( CSphSchema & tSchema, const int * pAttrs, int iAttrCount )
{
	assert ( pAttrs );

	CSphVector<int> dCur;

	// add valid attributes to processing list
	for ( int i=0; i<iAttrCount; i++ )
		if ( pAttrs[i]>=0 )
			dCur.Add ( pAttrs[i] );

	int iInitialAttrs = dCur.GetLength();

	// collect columns which affect current expressions
	for ( int i=0; i<dCur.GetLength(); i++ )
	{
		const CSphColumnInfo & tCol = tSchema.GetAttr ( dCur[i] );
		if ( tCol.m_eStage>SPH_EVAL_PRESORT && tCol.m_pExpr.Ptr()!=NULL )
			tCol.m_pExpr->GetDependencyColumns ( dCur );
	}

	// get rid of dupes
	dCur.Uniq();

	// fix up of attributes stages
	ARRAY_FOREACH ( i, dCur )
	{
		int iAttr = dCur[i];
		if ( iAttr<0 )
			continue;

		CSphColumnInfo & tCol = const_cast < CSphColumnInfo & > ( tSchema.GetAttr ( iAttr ) );
		if ( tCol.m_eStage==SPH_EVAL_FINAL )
			tCol.m_eStage = SPH_EVAL_PRESORT;
	}

	// it uses attributes if it has dependencies from other attributes
	return ( iInitialAttrs>dCur.GetLength() );
}


// expression that transform string pool base + offset -> ptr
struct ExprSortStringAttrFixup_c : public ISphExpr
{
	const BYTE *			m_pStrings; ///< string pool; base for offset of string attributes
	const CSphAttrLocator	m_tLocator; ///< string attribute to fix

	explicit ExprSortStringAttrFixup_c ( const CSphAttrLocator & tLocator )
		: m_pStrings ( NULL )
		, m_tLocator ( tLocator )
	{
	}

	virtual float Eval ( const CSphMatch & ) const { assert ( 0 ); return 0.0f; }

	virtual int64_t Int64Eval ( const CSphMatch & tMatch ) const
	{
		SphAttr_t uOff = tMatch.GetAttr ( m_tLocator );
		return (int64_t)( m_pStrings && uOff ? m_pStrings + uOff : NULL );
	}

	virtual void SetStringPool ( const BYTE * pStrings ) { m_pStrings = pStrings; }
};


static const char g_sIntAttrPrefix[] = "@int_str2ptr_";


bool sphIsSortStringInternal ( const char * sColumnName )
{
	assert ( sColumnName );
	return ( strncmp ( sColumnName, g_sIntAttrPrefix, sizeof(g_sIntAttrPrefix)-1 )==0 );
}


static bool SetupSortStringRemap ( CSphSchema & tSorterSchema, CSphMatchComparatorState & tState, const int * dAttr )
{
#ifndef NDEBUG
	int iColWasCount = tSorterSchema.GetAttrsCount();
#endif
	bool bUsesAtrrs = false;
	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_eKeypart[i]!=SPH_KEYPART_STRING )
			continue;

		assert ( dAttr[i]>=0 && dAttr[i]<iColWasCount );

		CSphString sRemapCol;
		sRemapCol.SetSprintf ( "%s%s", g_sIntAttrPrefix, tSorterSchema.GetAttr ( dAttr[i] ).m_sName.cstr() );

		int iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );
		if ( iRemap==-1 )
		{
			CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_BIGINT );
			tRemapCol.m_eStage = SPH_EVAL_PRESORT;

			iRemap = tSorterSchema.GetAttrsCount();
			tSorterSchema.AddAttr ( tRemapCol, true );
		}
		tState.m_tLocator[i] = tSorterSchema.GetAttr ( iRemap ).m_tLocator;
	}

	return bUsesAtrrs;
}


ISphExpr * sphSortSetupExpr ( const CSphString & sName, const CSphSchema & tIndexSchema )
{
	if ( !sName.Begins ( g_sIntAttrPrefix ) )
		return NULL;

	const CSphColumnInfo * pCol = tIndexSchema.GetAttr ( sName.cstr()+sizeof(g_sIntAttrPrefix)-1 );
	if ( !pCol )
		return NULL;

	return new ExprSortStringAttrFixup_c ( pCol->m_tLocator );
}


bool sphSortGetStringRemap ( const CSphSchema & tSorterSchema, const CSphSchema & tIndexSchema, CSphVector<SphStringSorterRemap_t> & dAttrs )
{
	dAttrs.Resize ( 0 );
	for ( int i=0; i<tSorterSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tDst = tSorterSchema.GetAttr(i);
		if ( !tDst.m_sName.Begins ( g_sIntAttrPrefix ) )
			continue;

		const CSphColumnInfo * pSrcCol = tIndexSchema.GetAttr ( tDst.m_sName.cstr()+sizeof(g_sIntAttrPrefix)-1 );
		assert ( pSrcCol );

		SphStringSorterRemap_t & tRemap = dAttrs.Add();
		tRemap.m_tSrc = pSrcCol->m_tLocator;
		tRemap.m_tDst = tDst.m_tLocator;
	}

	return ( dAttrs.GetLength()>0 );
}

////////////////////
// BINARY COLLATION
////////////////////

int CollateBinary ( const BYTE * pStr1, const BYTE * pStr2 )
{
	int iLen1 = sphUnpackStr ( pStr1, &pStr1 );
	int iLen2 = sphUnpackStr ( pStr2, &pStr2 );
	int iRes = memcmp ( (const char *)pStr1, (const char *)pStr2, Min ( iLen1, iLen2 ) );
	return iRes ? iRes : ( iLen1-iLen2 );
}

///////////////////////////////
// LIBC_CI, LIBC_CS COLLATIONS
///////////////////////////////

/// libc_ci, wrapper for strcasecmp
int CollateLibcCI ( const BYTE * pStr1, const BYTE * pStr2 )
{
	int iLen1 = sphUnpackStr ( pStr1, &pStr1 );
	int iLen2 = sphUnpackStr ( pStr2, &pStr2 );
	int iRes = strncasecmp ( (const char *)pStr1, (const char *)pStr2, Min ( iLen1, iLen2 ) );
	return iRes ? iRes : ( iLen1-iLen2 );
}


/// libc_cs, wrapper for strcoll
int CollateLibcCS ( const BYTE * pStr1, const BYTE * pStr2 )
{
	#define COLLATE_STACK_BUFFER 1024

	int iLen1 = sphUnpackStr ( pStr1, &pStr1 );
	int iLen2 = sphUnpackStr ( pStr2, &pStr2 );

	// strcoll wants asciiz strings, so we would have to copy them over
	// lets use stack buffer for smaller ones, and allocate from heap for bigger ones
	int iRes = 0;
	int iLen = Min ( iLen1, iLen2 );
	if ( iLen<COLLATE_STACK_BUFFER )
	{
		// small strings on stack
		BYTE sBuf1[COLLATE_STACK_BUFFER];
		BYTE sBuf2[COLLATE_STACK_BUFFER];

		memcpy ( sBuf1, pStr1, iLen );
		memcpy ( sBuf2, pStr2, iLen );
		sBuf1[iLen] = sBuf2[iLen] = '\0';
		iRes = strcoll ( (const char*)sBuf1, (const char*)sBuf2 );
	} else
	{
		// big strings on heap
		char * pBuf1 = new char [ iLen ];
		char * pBuf2 = new char [ iLen ];

		memcpy ( pBuf1, pStr1, iLen );
		memcpy ( pBuf2, pStr2, iLen );
		pBuf1[iLen] = pBuf2[iLen] = '\0';
		iRes = strcoll ( (const char*)pBuf1, (const char*)pBuf2 );

		SafeDeleteArray ( pBuf2 );
		SafeDeleteArray ( pBuf1 );
	}

	return iRes ? iRes : ( iLen1-iLen2 );
}

/////////////////////////////
// UTF8_GENERAL_CI COLLATION
/////////////////////////////

/// 1st level LUT
static unsigned short * g_dCollPlanes_UTF8CI[0x100];

/// 2nd level LUT, non-trivial collation data
static unsigned short g_dCollWeights_UTF8CI[0xb00] =
{
	// weights for 0x0 to 0x5ff
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 924, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	65, 65, 65, 65, 65, 65, 198, 67, 69, 69, 69, 69, 73, 73, 73, 73,
	208, 78, 79, 79, 79, 79, 79, 215, 216, 85, 85, 85, 85, 89, 222, 83,
	65, 65, 65, 65, 65, 65, 198, 67, 69, 69, 69, 69, 73, 73, 73, 73,
	208, 78, 79, 79, 79, 79, 79, 247, 216, 85, 85, 85, 85, 89, 222, 89,
	65, 65, 65, 65, 65, 65, 67, 67, 67, 67, 67, 67, 67, 67, 68, 68,
	272, 272, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 71, 71, 71, 71,
	71, 71, 71, 71, 72, 72, 294, 294, 73, 73, 73, 73, 73, 73, 73, 73,
	73, 73, 306, 306, 74, 74, 75, 75, 312, 76, 76, 76, 76, 76, 76, 319,
	319, 321, 321, 78, 78, 78, 78, 78, 78, 329, 330, 330, 79, 79, 79, 79,
	79, 79, 338, 338, 82, 82, 82, 82, 82, 82, 83, 83, 83, 83, 83, 83,
	83, 83, 84, 84, 84, 84, 358, 358, 85, 85, 85, 85, 85, 85, 85, 85,
	85, 85, 85, 85, 87, 87, 89, 89, 89, 90, 90, 90, 90, 90, 90, 83,
	384, 385, 386, 386, 388, 388, 390, 391, 391, 393, 394, 395, 395, 397, 398, 399,
	400, 401, 401, 403, 404, 502, 406, 407, 408, 408, 410, 411, 412, 413, 414, 415,
	79, 79, 418, 418, 420, 420, 422, 423, 423, 425, 426, 427, 428, 428, 430, 85,
	85, 433, 434, 435, 435, 437, 437, 439, 440, 440, 442, 443, 444, 444, 446, 503,
	448, 449, 450, 451, 452, 452, 452, 455, 455, 455, 458, 458, 458, 65, 65, 73,
	73, 79, 79, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 398, 65, 65,
	65, 65, 198, 198, 484, 484, 71, 71, 75, 75, 79, 79, 79, 79, 439, 439,
	74, 497, 497, 497, 71, 71, 502, 503, 78, 78, 65, 65, 198, 198, 216, 216,
	65, 65, 65, 65, 69, 69, 69, 69, 73, 73, 73, 73, 79, 79, 79, 79,
	82, 82, 82, 82, 85, 85, 85, 85, 83, 83, 84, 84, 540, 540, 72, 72,
	544, 545, 546, 546, 548, 548, 65, 65, 69, 69, 79, 79, 79, 79, 79, 79,
	79, 79, 89, 89, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575,
	576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591,
	592, 593, 594, 385, 390, 597, 393, 394, 600, 399, 602, 400, 604, 605, 606, 607,
	403, 609, 610, 404, 612, 613, 614, 615, 407, 406, 618, 619, 620, 621, 622, 412,
	624, 625, 413, 627, 628, 415, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639,
	422, 641, 642, 425, 644, 645, 646, 647, 430, 649, 433, 434, 652, 653, 654, 655,
	656, 657, 439, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671,
	672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687,
	688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703,
	704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719,
	720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735,
	736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751,
	752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767,
	768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783,
	784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799,
	800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815,
	816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831,
	832, 833, 834, 835, 836, 921, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847,
	848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863,
	864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879,
	880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895,
	896, 897, 898, 899, 900, 901, 913, 903, 917, 919, 921, 907, 927, 909, 933, 937,
	921, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927,
	928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 921, 933, 913, 917, 919, 921,
	933, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927,
	928, 929, 931, 931, 932, 933, 934, 935, 936, 937, 921, 933, 927, 933, 937, 975,
	914, 920, 978, 978, 978, 934, 928, 983, 984, 985, 986, 986, 988, 988, 990, 990,
	992, 992, 994, 994, 996, 996, 998, 998, 1000, 1000, 1002, 1002, 1004, 1004, 1006, 1006,
	922, 929, 931, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019, 1020, 1021, 1022, 1023,
	1045, 1045, 1026, 1043, 1028, 1029, 1030, 1030, 1032, 1033, 1034, 1035, 1050, 1048, 1059, 1039,
	1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055,
	1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071,
	1040, 1041, 1042, 1043, 1044, 1045, 1046, 1047, 1048, 1049, 1050, 1051, 1052, 1053, 1054, 1055,
	1056, 1057, 1058, 1059, 1060, 1061, 1062, 1063, 1064, 1065, 1066, 1067, 1068, 1069, 1070, 1071,
	1045, 1045, 1026, 1043, 1028, 1029, 1030, 1030, 1032, 1033, 1034, 1035, 1050, 1048, 1059, 1039,
	1120, 1120, 1122, 1122, 1124, 1124, 1126, 1126, 1128, 1128, 1130, 1130, 1132, 1132, 1134, 1134,
	1136, 1136, 1138, 1138, 1140, 1140, 1140, 1140, 1144, 1144, 1146, 1146, 1148, 1148, 1150, 1150,
	1152, 1152, 1154, 1155, 1156, 1157, 1158, 1159, 1160, 1161, 1162, 1163, 1164, 1164, 1166, 1166,
	1168, 1168, 1170, 1170, 1172, 1172, 1174, 1174, 1176, 1176, 1178, 1178, 1180, 1180, 1182, 1182,
	1184, 1184, 1186, 1186, 1188, 1188, 1190, 1190, 1192, 1192, 1194, 1194, 1196, 1196, 1198, 1198,
	1200, 1200, 1202, 1202, 1204, 1204, 1206, 1206, 1208, 1208, 1210, 1210, 1212, 1212, 1214, 1214,
	1216, 1046, 1046, 1219, 1219, 1221, 1222, 1223, 1223, 1225, 1226, 1227, 1227, 1229, 1230, 1231,
	1040, 1040, 1040, 1040, 1236, 1236, 1045, 1045, 1240, 1240, 1240, 1240, 1046, 1046, 1047, 1047,
	1248, 1248, 1048, 1048, 1048, 1048, 1054, 1054, 1256, 1256, 1256, 1256, 1069, 1069, 1059, 1059,
	1059, 1059, 1059, 1059, 1063, 1063, 1270, 1271, 1067, 1067, 1274, 1275, 1276, 1277, 1278, 1279,
	1280, 1281, 1282, 1283, 1284, 1285, 1286, 1287, 1288, 1289, 1290, 1291, 1292, 1293, 1294, 1295,
	1296, 1297, 1298, 1299, 1300, 1301, 1302, 1303, 1304, 1305, 1306, 1307, 1308, 1309, 1310, 1311,
	1312, 1313, 1314, 1315, 1316, 1317, 1318, 1319, 1320, 1321, 1322, 1323, 1324, 1325, 1326, 1327,
	1328, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1343,
	1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359,
	1360, 1361, 1362, 1363, 1364, 1365, 1366, 1367, 1368, 1369, 1370, 1371, 1372, 1373, 1374, 1375,
	1376, 1329, 1330, 1331, 1332, 1333, 1334, 1335, 1336, 1337, 1338, 1339, 1340, 1341, 1342, 1343,
	1344, 1345, 1346, 1347, 1348, 1349, 1350, 1351, 1352, 1353, 1354, 1355, 1356, 1357, 1358, 1359,
	1360, 1361, 1362, 1363, 1364, 1365, 1366, 1415, 1416, 1417, 1418, 1419, 1420, 1421, 1422, 1423,
	1424, 1425, 1426, 1427, 1428, 1429, 1430, 1431, 1432, 1433, 1434, 1435, 1436, 1437, 1438, 1439,
	1440, 1441, 1442, 1443, 1444, 1445, 1446, 1447, 1448, 1449, 1450, 1451, 1452, 1453, 1454, 1455,
	1456, 1457, 1458, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 1466, 1467, 1468, 1469, 1470, 1471,
	1472, 1473, 1474, 1475, 1476, 1477, 1478, 1479, 1480, 1481, 1482, 1483, 1484, 1485, 1486, 1487,
	1488, 1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, 1499, 1500, 1501, 1502, 1503,
	1504, 1505, 1506, 1507, 1508, 1509, 1510, 1511, 1512, 1513, 1514, 1515, 1516, 1517, 1518, 1519,
	1520, 1521, 1522, 1523, 1524, 1525, 1526, 1527, 1528, 1529, 1530, 1531, 1532, 1533, 1534, 1535,

	// weights for codepoints 0x1e00 to 0x1fff
	65, 65, 66, 66, 66, 66, 66, 66, 67, 67, 68, 68, 68, 68, 68, 68,
	68, 68, 68, 68, 69, 69, 69, 69, 69, 69, 69, 69, 69, 69, 70, 70,
	71, 71, 72, 72, 72, 72, 72, 72, 72, 72, 72, 72, 73, 73, 73, 73,
	75, 75, 75, 75, 75, 75, 76, 76, 76, 76, 76, 76, 76, 76, 77, 77,
	77, 77, 77, 77, 78, 78, 78, 78, 78, 78, 78, 78, 79, 79, 79, 79,
	79, 79, 79, 79, 80, 80, 80, 80, 82, 82, 82, 82, 82, 82, 82, 82,
	83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 84, 84, 84, 84, 84, 84,
	84, 84, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 86, 86, 86, 86,
	87, 87, 87, 87, 87, 87, 87, 87, 87, 87, 88, 88, 88, 88, 89, 89,
	90, 90, 90, 90, 90, 90, 72, 84, 87, 89, 7834, 83, 7836, 7837, 7838, 7839,
	65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
	65, 65, 65, 65, 65, 65, 65, 65, 69, 69, 69, 69, 69, 69, 69, 69,
	69, 69, 69, 69, 69, 69, 69, 69, 73, 73, 73, 73, 79, 79, 79, 79,
	79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
	79, 79, 79, 79, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
	85, 85, 89, 89, 89, 89, 89, 89, 89, 89, 7930, 7931, 7932, 7933, 7934, 7935,
	913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913,
	917, 917, 917, 917, 917, 917, 7958, 7959, 917, 917, 917, 917, 917, 917, 7966, 7967,
	919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919,
	921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921, 921,
	927, 927, 927, 927, 927, 927, 8006, 8007, 927, 927, 927, 927, 927, 927, 8014, 8015,
	933, 933, 933, 933, 933, 933, 933, 933, 8024, 933, 8026, 933, 8028, 933, 8030, 933,
	937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937,
	913, 8123, 917, 8137, 919, 8139, 921, 8155, 927, 8185, 933, 8171, 937, 8187, 8062, 8063,
	913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913, 913,
	919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919, 919,
	937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937, 937,
	913, 913, 913, 913, 913, 8117, 913, 913, 913, 913, 913, 8123, 913, 8125, 921, 8127,
	8128, 8129, 919, 919, 919, 8133, 919, 919, 917, 8137, 919, 8139, 919, 8141, 8142, 8143,
	921, 921, 921, 8147, 8148, 8149, 921, 921, 921, 921, 921, 8155, 8156, 8157, 8158, 8159,
	933, 933, 933, 8163, 929, 929, 933, 933, 933, 933, 933, 8171, 929, 8173, 8174, 8175,
	8176, 8177, 937, 937, 937, 8181, 937, 937, 927, 8185, 937, 8187, 937, 8189, 8190, 8191

	// space for codepoints 0x21xx, 0x24xx, 0xffxx (generated)
};


/// initialize collation LUTs
void sphCollationInit()
{
	const int dWeightPlane[0x0b] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x1e, 0x1f, 0x21, 0x24, 0xff };

	// generate missing weights
	for ( int i=0; i<0x100; i++ )
	{
		g_dCollWeights_UTF8CI[i+0x800] = (unsigned short)( 0x2100 + i - ( i>=0x70 && i<=0x7f )*16 ); // 2170..217f, -16
		g_dCollWeights_UTF8CI[i+0x900] = (unsigned short)( 0x2400 + i - ( i>=0xd0 && i<=0xe9 )*26 ); // 24d0..24e9, -26
		g_dCollWeights_UTF8CI[i+0xa00] = (unsigned short)( 0xff00 + i - ( i>=0x41 && i<=0x5a )*32 ); // ff41..ff5a, -32
	}

	// generate planes table
	for ( int i=0; i<0x100; i++ )
		g_dCollPlanes_UTF8CI[i] = NULL;

	for ( int i=0; i<0x0b; i++ )
		g_dCollPlanes_UTF8CI [ dWeightPlane[i] ] = g_dCollWeights_UTF8CI + 0x100*i;
}


/// collate a single codepoint
static inline int CollateUTF8CI ( int iCode )
{
	return ( ( iCode>>16 ) || !g_dCollPlanes_UTF8CI [ iCode>>8 ] )
		? iCode
		: g_dCollPlanes_UTF8CI [ iCode>>8 ][ iCode&0xff ];
}


/// utf8_general_ci
int CollateUtf8GeneralCI ( const BYTE * pArg1, const BYTE * pArg2 )
{
	// some const breakage and mess
	// we MUST NOT actually modify the data
	// but sphUTF8Decode() calls currently need non-const pointers
	BYTE * pStr1 = (BYTE*) pArg1;
	BYTE * pStr2 = (BYTE*) pArg2;
	int iLen1 = sphUnpackStr ( pStr1, (const BYTE**)&pStr1 );
	int iLen2 = sphUnpackStr ( pStr2, (const BYTE**)&pStr2 );

	const BYTE * pMax1 = pStr1 + iLen1;
	const BYTE * pMax2 = pStr2 + iLen2;
	while ( pStr1<pMax1 && pStr2<pMax2 )
	{
		// FIXME! on broken data, decode might go beyond buffer bounds
		int iCode1 = sphUTF8Decode ( pStr1 );
		int iCode2 = sphUTF8Decode ( pStr2 );
		if ( !iCode1 && !iCode2 )
			return 0;
		if ( !iCode1 || !iCode2 )
			return !iCode1 ? -1 : 1;

		if ( iCode1==iCode2 )
			continue;
		iCode1 = CollateUTF8CI ( iCode1 );
		iCode2 = CollateUTF8CI ( iCode2 );
		if ( iCode1!=iCode2 )
			return iCode1-iCode2;
	}

	if ( pStr1>=pMax1 && pStr2>=pMax2 )
		return 0;
	return ( pStr1==pMax1 ) ? 1 : -1;
}


/////////////////////////////
// hashing functions
/////////////////////////////


class LibcCSHash_fn
{
public:
	mutable CSphTightVector<BYTE> m_dBuf;
	static const int LOCALE_SAFE_GAP = 16;

	LibcCSHash_fn()
	{
		m_dBuf.Resize ( COLLATE_STACK_BUFFER );
	}

	uint64_t Hash ( const BYTE * pStr, int iLen ) const
	{
		assert ( pStr && iLen );

		int iCompositeLen = iLen + 1 + (int)( 3.0f * iLen ) + LOCALE_SAFE_GAP;
		if ( m_dBuf.GetLength()<iCompositeLen )
			m_dBuf.Resize ( iCompositeLen );

		memcpy ( m_dBuf.Begin(), pStr, iLen );
		m_dBuf[iLen] = '\0';

		BYTE * pDst = m_dBuf.Begin()+iLen+1;
		int iDstAvailable = m_dBuf.GetLength() - iLen - LOCALE_SAFE_GAP;

		int iDstLen = strxfrm ( (char *)pDst, (const char *)m_dBuf.Begin(), iDstAvailable );
		assert ( iDstLen<iDstAvailable+LOCALE_SAFE_GAP );

		uint64_t uAcc = sphFNV64 ( pDst, iDstLen );

		return uAcc;
	}
};


class LibcCIHash_fn
{
public:
	uint64_t Hash ( const BYTE * pStr, int iLen ) const
	{
		assert ( pStr && iLen );

		uint64_t uAcc = SPH_FNV64_SEED;
		while ( iLen-- )
		{
			int iChar = tolower ( *pStr++ );
			uAcc = sphFNV64 ( (const BYTE *)&iChar, 4, uAcc );
		}

		return uAcc;
	}
};


class Utf8CIHash_fn
{
public:
	uint64_t Hash ( const BYTE * pStr, int iLen ) const
	{
		assert ( pStr && iLen );

		uint64_t uAcc = SPH_FNV64_SEED;
		while ( iLen-- )
		{
			BYTE * pCur = (BYTE *)pStr++;
			int iCode = sphUTF8Decode ( pCur );
			iCode = CollateUTF8CI ( iCode );
			uAcc = sphFNV64 ( (const BYTE *)&iCode, 4, uAcc );
		}

		return uAcc;
	}
};


class BinaryHash_fn
{
public:
	uint64_t Hash ( const BYTE * pStr, int iLen ) const
	{
		assert ( pStr && iLen );
		return sphFNV64 ( pStr, iLen );
	}
};


CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation )
{
	if ( eCollation==SPH_COLLATION_UTF8_GENERAL_CI )
		return new CSphGrouperString<Utf8CIHash_fn> ( tLoc );
	else if ( eCollation==SPH_COLLATION_LIBC_CI )
		return new CSphGrouperString<LibcCIHash_fn> ( tLoc );
	else if ( eCollation==SPH_COLLATION_LIBC_CS )
		return new CSphGrouperString<LibcCSHash_fn> ( tLoc );
	else
		return new CSphGrouperString<BinaryHash_fn> ( tLoc );
}


/////////////////////////
// SORTING QUEUE FACTORY
/////////////////////////

ISphMatchSorter * sphCreateQueue ( const CSphQuery * pQuery, const CSphSchema & tSchema,
								CSphString & sError, bool bComputeItems, CSphSchema * pExtra, CSphAttrUpdateEx* pUpdate )
{
	// prepare for descent
	ISphMatchSorter * pTop = NULL;
	CSphMatchComparatorState tStateMatch, tStateGroup;

	sError = "";

	///////////////////////////////////////
	// build incoming and outgoing schemas
	///////////////////////////////////////

	// sorter schema
	// adds computed expressions and groupby stuff on top of the original index schema
	CSphSchema tSorterSchema = tSchema;

	// setup overrides, detach them into dynamic part
	ARRAY_FOREACH ( i, pQuery->m_dOverrides )
	{
		const char * sAttr = pQuery->m_dOverrides[i].m_sAttr.cstr();

		int iIndex = tSorterSchema.GetAttrIndex ( sAttr );
		if ( iIndex<0 )
		{
			sError.SetSprintf ( "override attribute '%s' not found", sAttr );
			return NULL;
		}

		CSphColumnInfo tCol = tSorterSchema.GetAttr ( iIndex );
		tCol.m_eStage = SPH_EVAL_OVERRIDE;
		tSorterSchema.AddAttr ( tCol, true );
		if ( pExtra )
			pExtra->AddAttr ( tCol, true );
		tSorterSchema.RemoveAttr ( iIndex );
	}

	// setup @geodist
	if ( pQuery->m_bGeoAnchor && tSorterSchema.GetAttrIndex ( "@geodist" )<0 )
	{
		ExprGeodist_t * pExpr = new ExprGeodist_t ();
		if ( !pExpr->Setup ( pQuery, tSorterSchema, sError ) )
		{
			pExpr->Release ();
			return NULL;
		}
		CSphColumnInfo tCol ( "@geodist", SPH_ATTR_FLOAT );
		tCol.m_pExpr = pExpr; // takes ownership, no need to for explicit pExpr release
		tCol.m_eStage = SPH_EVAL_PREFILTER; // OPTIMIZE? actual stage depends on usage
		tSorterSchema.AddAttr ( tCol, true );
		if ( pExtra )
			pExtra->AddAttr ( tCol, true );
	}

	// setup @expr
	if ( pQuery->m_eSort==SPH_SORT_EXPR && tSorterSchema.GetAttrIndex ( "@expr" )<0 )
	{
		CSphColumnInfo tCol ( "@expr", SPH_ATTR_FLOAT ); // enforce float type for backwards compatibility (ie. too lazy to fix those tests right now)
		tCol.m_pExpr = sphExprParse ( pQuery->m_sSortBy.cstr(), tSorterSchema, NULL, NULL, sError, pExtra );
		if ( !tCol.m_pExpr )
			return NULL;
		tCol.m_eStage = SPH_EVAL_PRESORT;
		tSorterSchema.AddAttr ( tCol, true );
	}

	// expressions from select items
	CSphVector<CSphColumnInfo> dAggregates;

	bool bHasCount = false;

	if ( bComputeItems )
		ARRAY_FOREACH ( iItem, pQuery->m_dItems )
	{
		const CSphQueryItem & tItem = pQuery->m_dItems[iItem];
		const CSphString & sExpr = tItem.m_sExpr;
		bool bIsCount = IsCount(sExpr);
		bHasCount |= bIsCount;

		if ( bIsCount && sExpr.cstr()[0]!='@' )
		{
			CSphString & sExprW = const_cast < CSphString & > ( sExpr );
			sExprW = "@count";
		}

		// for now, just always pass "plain" attrs from index to sorter; they will be filtered on searchd level
		if ( sExpr=="*"
			|| ( tSchema.GetAttrIndex ( sExpr.cstr() )>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE )
			|| IsGroupby(sExpr) || bIsCount )
		{
			continue;
		}

		// not an attribute? must be an expression, and must be aliased
		if ( tItem.m_sAlias.IsEmpty() )
		{
			sError.SetSprintf ( "expression '%s' must be aliased (use 'expr AS alias' syntax)", tItem.m_sExpr.cstr() );
			return NULL;
		}

		// tricky part
		// we might be fed with precomputed matches, but it's all or nothing
		// the incoming match either does not have anything computed, or it has everything
		if ( tSchema.GetAttrsCount()==tSorterSchema.GetAttrsCount() )
		{
			// so far we had everything, so we might be precomputed, and the alias just might already exist
			int iSuspect = tSchema.GetAttrIndex ( tItem.m_sAlias.cstr() );
			if ( iSuspect>=0 )
			{
				// however, let's ensure that it was an expression
				if ( tSchema.GetAttr ( iSuspect ).m_pExpr.Ptr()!=NULL )
					continue;

				// otherwise we're not precomputed, *and* have a duplicate name
				sError.SetSprintf ( "alias '%s' must be unique (conflicts with an index attribute)", tItem.m_sAlias.cstr() );
				return NULL;
			}
		} else
		{
			// we are adding stuff, must not be precomputed, check for both kinds of dupes
			if ( tSchema.GetAttrIndex ( tItem.m_sAlias.cstr() )>=0 )
			{
				sError.SetSprintf ( "alias '%s' must be unique (conflicts with an index attribute)", tItem.m_sAlias.cstr() );
				return NULL;
			}
			if ( tSorterSchema.GetAttrIndex ( tItem.m_sAlias.cstr() )>=0 )
			{
				sError.SetSprintf ( "alias '%s' must be unique (conflicts with another alias)", tItem.m_sAlias.cstr() );
				return NULL;
			}
		}

		// a new and shiny expression, lets parse
		bool bUsesWeight;
		CSphColumnInfo tExprCol ( tItem.m_sAlias.cstr(), SPH_ATTR_NONE );
		tExprCol.m_pExpr = sphExprParse ( sExpr.cstr(), tSorterSchema, &tExprCol.m_eAttrType, &bUsesWeight, sError, pExtra );
		tExprCol.m_eAggrFunc = tItem.m_eAggrFunc;
		if ( !tExprCol.m_pExpr )
		{
			sError.SetSprintf ( "parse error: %s", sError.cstr() );
			return NULL;
		}

		// force AVG() to be computed in floats
		if ( tExprCol.m_eAggrFunc==SPH_AGGR_AVG )
		{
			tExprCol.m_eAttrType = SPH_ATTR_FLOAT;
			tExprCol.m_tLocator.m_iBitCount = 32;
		}

		// postpone aggregates, add non-aggregates
		if ( tExprCol.m_eAggrFunc==SPH_AGGR_NONE )
		{
			// by default, lets be lazy and compute expressions as late as possible
			tExprCol.m_eStage = SPH_EVAL_FINAL;

			// is this expression used in filter?
			// OPTIMIZE? hash filters and do hash lookups?
			ARRAY_FOREACH ( i, pQuery->m_dFilters )
				if ( pQuery->m_dFilters[i].m_sAttrName==tExprCol.m_sName )
			{
				if ( bUsesWeight )
				{
					tExprCol.m_eStage = SPH_EVAL_PRESORT; // special, weight filter
					break;
				}

				// usual filter
				tExprCol.m_eStage = SPH_EVAL_PREFILTER;

				// so we are about to add a filter condition
				// but it might depend on some preceding columns
				// lets detect those and move them to prefilter phase too
				CSphVector<int> dCur;
				tExprCol.m_pExpr->GetDependencyColumns ( dCur );

				ARRAY_FOREACH ( i, dCur )
				{
					CSphColumnInfo & tDep = const_cast < CSphColumnInfo & > ( tSorterSchema.GetAttr ( dCur[i] ) );
					if ( tDep.m_eStage>SPH_EVAL_PREFILTER )
						tDep.m_eStage = SPH_EVAL_PREFILTER;
				}
				break;
			}

			// add it!
			// NOTE, "final" stage might need to be fixed up later
			// we'll do that when parsing sorting clause
			tSorterSchema.AddAttr ( tExprCol, true );
		} else
		{
			tExprCol.m_eStage = SPH_EVAL_PRESORT; // sorter expects computed expression
			dAggregates.Add ( tExprCol );
		}
	}

	// expressions wrapped in aggregates must be at the very end of pre-groupby match
	ARRAY_FOREACH ( i, dAggregates )
	{
		tSorterSchema.AddAttr ( dAggregates[i], true );
		if ( pExtra )
			pExtra->AddAttr ( dAggregates[i], true );
	}

	////////////////////////////////////////////
	// setup groupby settings, create shortcuts
	////////////////////////////////////////////

	CSphGroupSorterSettings tSettings;
	if ( !SetupGroupbySettings ( pQuery, tSorterSchema, tSettings, sError ) )
		return NULL;

	const bool bGotGroupby = !pQuery->m_sGroupBy.IsEmpty(); // or else, check in SetupGroupbySettings() would already fail
	const bool bGotDistinct = ( tSettings.m_tDistinctLoc.m_iBitOffset>=0 );

	// now lets add @groupby etc if needed
	if ( bGotGroupby && tSorterSchema.GetAttrIndex ( "@groupby" )<0 )
	{
		CSphColumnInfo tGroupby ( "@groupby", tSettings.m_pGrouper->GetResultType() );
		CSphColumnInfo tCount ( "@count", SPH_ATTR_INTEGER );
		CSphColumnInfo tDistinct ( "@distinct", SPH_ATTR_INTEGER );

		tGroupby.m_eStage = SPH_EVAL_SORTER;
		tCount.m_eStage = SPH_EVAL_SORTER;
		tDistinct.m_eStage = SPH_EVAL_SORTER;


		tSorterSchema.AddAttr ( tGroupby, true );
		if ( pExtra )
			pExtra->AddAttr ( tGroupby, true );
		tSorterSchema.AddAttr ( tCount, true );
		if ( pExtra )
			pExtra->AddAttr ( tCount, true );

		if ( bGotDistinct )
		{
			tSorterSchema.AddAttr ( tDistinct, true );
			if ( pExtra )
				pExtra->AddAttr ( tDistinct, true );
		}
	}

#define LOC_CHECK(_cond,_msg) if (!(_cond)) { sError = "invalid schema: " _msg; return false; }

	int iGroupby = tSorterSchema.GetAttrIndex ( "@groupby" );
	if ( iGroupby>=0 )
	{
		tSettings.m_bDistinct = bGotDistinct;
		tSettings.m_tLocGroupby = tSorterSchema.GetAttr ( iGroupby ).m_tLocator;
		LOC_CHECK ( tSettings.m_tLocGroupby.m_bDynamic, "@groupby must be dynamic" );

		int iCount = tSorterSchema.GetAttrIndex ( "@count" );
		LOC_CHECK ( iCount>=0, "missing @count" );

		tSettings.m_tLocCount = tSorterSchema.GetAttr ( iCount ).m_tLocator;
		LOC_CHECK ( tSettings.m_tLocCount.m_bDynamic, "@count must be dynamic" );

		int iDistinct = tSorterSchema.GetAttrIndex ( "@distinct" );
		if ( bGotDistinct )
		{
			LOC_CHECK ( iDistinct>=0, "missing @distinct" );
			tSettings.m_tLocDistinct = tSorterSchema.GetAttr ( iDistinct ).m_tLocator;
			LOC_CHECK ( tSettings.m_tLocDistinct.m_bDynamic, "@distinct must be dynamic" );
		} else
		{
			LOC_CHECK ( iDistinct<=0, "unexpected @distinct" );
		}
	}

	if ( bHasCount )
	{
		LOC_CHECK ( tSorterSchema.GetAttrIndex ( "@count" )>=0, "Count(*) or @count is queried, but not available in the schema" );
	}

#undef LOC_CHECK

	////////////////////////////////////
	// choose and setup sorting functor
	////////////////////////////////////

	ESphSortFunc eMatchFunc = FUNC_REL_DESC;
	ESphSortFunc eGroupFunc = FUNC_REL_DESC;
	bool bUsesAttrs = false;
	bool bRandomize = false;

	// matches sorting function
	if ( pQuery->m_eSort==SPH_SORT_EXTENDED )
	{
		int dAttrs [ CSphMatchComparatorState::MAX_ATTRS ];
		ESortClauseParseResult eRes = sphParseSortClause ( pQuery, pQuery->m_sSortBy.cstr(), tSorterSchema, eMatchFunc, tStateMatch, dAttrs, sError, pExtra );
		if ( eRes==SORT_CLAUSE_ERROR )
			return NULL;

		if ( eRes==SORT_CLAUSE_RANDOM )
			bRandomize = true;

		bUsesAttrs = FixupDependency ( tSorterSchema, dAttrs, CSphMatchComparatorState::MAX_ATTRS );

		if ( !bUsesAttrs )
		{
			for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
			{
				ESphSortKeyPart ePart = tStateMatch.m_eKeypart[i];
				if ( ePart==SPH_KEYPART_INT || ePart==SPH_KEYPART_FLOAT || ePart==SPH_KEYPART_STRING )
					bUsesAttrs = true;
			}
		}
		bUsesAttrs |= SetupSortStringRemap ( tSorterSchema, tStateMatch, dAttrs );

	} else if ( pQuery->m_eSort==SPH_SORT_EXPR )
	{
		tStateMatch.m_eKeypart[0] = SPH_KEYPART_INT;
		tStateMatch.m_tLocator[0] = tSorterSchema.GetAttr ( tSorterSchema.GetAttrIndex ( "@expr" ) ).m_tLocator;
		tStateMatch.m_eKeypart[1] = SPH_KEYPART_ID;
		tStateMatch.m_uAttrDesc = 1;
		eMatchFunc = FUNC_EXPR;
		bUsesAttrs = true;

	} else
	{
		// check sort-by attribute
		if ( pQuery->m_eSort!=SPH_SORT_RELEVANCE )
		{
			int iSortAttr = tSorterSchema.GetAttrIndex ( pQuery->m_sSortBy.cstr() );
			if ( iSortAttr<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pQuery->m_sSortBy.cstr() );
				return NULL;
			}
			const CSphColumnInfo & tAttr = tSorterSchema.GetAttr ( iSortAttr );
			tStateMatch.m_eKeypart[0] = Attr2Keypart ( tAttr.m_eAttrType );
			tStateMatch.m_tLocator[0] = tAttr.m_tLocator;

			int dAttrs [ CSphMatchComparatorState::MAX_ATTRS ];
			dAttrs[0] = iSortAttr;
			bUsesAttrs |= SetupSortStringRemap ( tSorterSchema, tStateMatch, dAttrs );
		}

		// find out what function to use and whether it needs attributes
		bUsesAttrs = true;
		switch ( pQuery->m_eSort )
		{
			case SPH_SORT_ATTR_DESC:		eMatchFunc = FUNC_ATTR_DESC; break;
			case SPH_SORT_ATTR_ASC:			eMatchFunc = FUNC_ATTR_ASC; break;
			case SPH_SORT_TIME_SEGMENTS:	eMatchFunc = FUNC_TIMESEGS; break;
			case SPH_SORT_RELEVANCE:		eMatchFunc = FUNC_REL_DESC; bUsesAttrs = false; break;
			default:
				sError.SetSprintf ( "unknown sorting mode %d", pQuery->m_eSort );
				return NULL;
		}
	}

	// groups sorting function
	if ( bGotGroupby )
	{
		int dAttrs [ CSphMatchComparatorState::MAX_ATTRS ];
		ESortClauseParseResult eRes = sphParseSortClause ( pQuery, pQuery->m_sGroupSortBy.cstr(), tSorterSchema, eGroupFunc, tStateGroup, dAttrs, sError, pExtra );

		if ( eRes==SORT_CLAUSE_ERROR || eRes==SORT_CLAUSE_RANDOM )
		{
			if ( eRes==SORT_CLAUSE_RANDOM )
				sError.SetSprintf ( "groups can not be sorted by @random" );
			return NULL;
		}
		int idx = tSorterSchema.GetAttrIndex ( pQuery->m_sGroupBy.cstr() );
		if ( pExtra )
			pExtra->AddAttr ( tSorterSchema.GetAttr ( idx ), true );

		FixupDependency ( tSorterSchema, &idx, 1 );
		FixupDependency ( tSorterSchema, dAttrs, CSphMatchComparatorState::MAX_ATTRS );

		// GroupSortBy str attributes setup
		bUsesAttrs |= SetupSortStringRemap ( tSorterSchema, tStateGroup, dAttrs );
	}

	///////////////////
	// spawn the queue
	///////////////////

	if ( !bGotGroupby )
	{
		if ( pUpdate )
			pTop = new CSphUpdateQueue ( pQuery->m_iMaxMatches, pUpdate );
		else
			switch ( eMatchFunc )
			{
				case FUNC_REL_DESC:	pTop = new CSphMatchQueue<MatchRelevanceLt_fn>	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_DESC:pTop = new CSphMatchQueue<MatchAttrLt_fn>		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_ASC:	pTop = new CSphMatchQueue<MatchAttrGt_fn>		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_TIMESEGS:	pTop = new CSphMatchQueue<MatchTimeSegments_fn>	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC2:	pTop = new CSphMatchQueue<MatchGeneric2_fn>		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC3:	pTop = new CSphMatchQueue<MatchGeneric3_fn>		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC4:	pTop = new CSphMatchQueue<MatchGeneric4_fn>		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC5:	pTop = new CSphMatchQueue<MatchGeneric5_fn>		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_CUSTOM:	pTop = new CSphMatchQueue<MatchCustom_fn>		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_EXPR:		pTop = new CSphMatchQueue<MatchExpr_fn>			( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				default:			pTop = NULL;
			}

	} else
	{
		pTop = sphCreateSorter1st ( eMatchFunc, eGroupFunc, pQuery, tSettings );
	}

	if ( !pTop )
	{
		sError.SetSprintf ( "internal error: unhandled sorting mode (match-sort=%d, group=%d, group-sort=%d)",
			eMatchFunc, bGotGroupby, eGroupFunc );
		return NULL;
	}

	switch ( pQuery->m_eCollation )
	{
		case SPH_COLLATION_LIBC_CI:
			tStateMatch.m_fnStrCmp = CollateLibcCI;
			tStateGroup.m_fnStrCmp = CollateLibcCI;
			break;
		case SPH_COLLATION_LIBC_CS:
			tStateMatch.m_fnStrCmp = CollateLibcCS;
			tStateGroup.m_fnStrCmp = CollateLibcCS;
			break;
		case SPH_COLLATION_UTF8_GENERAL_CI:
			tStateMatch.m_fnStrCmp = CollateUtf8GeneralCI;
			tStateGroup.m_fnStrCmp = CollateUtf8GeneralCI;
			break;
		case SPH_COLLATION_BINARY:
			tStateMatch.m_fnStrCmp = CollateBinary;
			tStateGroup.m_fnStrCmp = CollateBinary;
			break;
	}

	assert ( pTop );
	pTop->SetState ( tStateMatch );
	pTop->SetGroupState ( tStateGroup );
	pTop->SetSchema ( tSorterSchema );
	pTop->m_bRandomize = bRandomize;

	if ( bRandomize )
		sphAutoSrand ();

	return pTop;
}


void sphFlattenQueue ( ISphMatchSorter * pQueue, CSphQueryResult * pResult, int iTag )
{
	if ( pQueue && pQueue->GetLength() )
	{
		int iOffset = pResult->m_dMatches.GetLength ();
		pResult->m_dMatches.Resize ( iOffset + pQueue->GetLength() );
		pQueue->Flatten ( &pResult->m_dMatches[iOffset], iTag );
	}
}


bool sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema )
{
	ARRAY_FOREACH ( i, tQuery.m_dItems )
	{
		const CSphString & sExpr = tQuery.m_dItems[i].m_sExpr;

		if ( !( sExpr=="*"
			|| ( tSchema.GetAttrIndex ( sExpr.cstr() )>=0 && tQuery.m_dItems[i].m_eAggrFunc==SPH_AGGR_NONE && tQuery.m_dItems[i].m_sAlias.IsEmpty() )
			|| IsGroupbyMagic(sExpr) ) )
			return true;
	}

	return false;
}


//
// $Id$
//
