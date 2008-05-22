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

#include "sphinx.h"

#include <time.h>
#include <math.h>

#if !USE_WINDOWS
#include <unistd.h>
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////
// TRAITS
//////////////////////////////////////////////////////////////////////////

/// helper to get i-th sorter attr from match
template<bool BITS>
inline SphAttr_t sphGetCompAttr ( const CSphMatchComparatorState & t, const CSphMatch & m, int i );

template<>
inline SphAttr_t sphGetCompAttr<false> ( const CSphMatchComparatorState & t, const CSphMatch & m, int i )
{
	return m.GetAttr ( t.m_tLocator[i] ); // FIXME! OPTIMIZE!!! we can go the short route here
}

template<>
inline SphAttr_t sphGetCompAttr<true> ( const CSphMatchComparatorState & t, const CSphMatch & m, int i )
{
	return m.GetAttr ( t.m_tLocator[i] );
}

//////////////////////////////////////////////////////////////////////////

/// groupby key type
typedef uint64_t	SphGroupKey_t;


/// base grouper (class that computes groupby key)
class CSphGrouper
{
public:
	virtual					~CSphGrouper () {}
	virtual SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const = 0;
	virtual SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const = 0;
	virtual void			GetLocator ( CSphAttrLocator & tOut ) const = 0;
	virtual DWORD			GetResultType () const = 0;
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
	int			GetLength () const										{ return m_iUsed; }
	void		SetState ( const CSphMatchComparatorState & tState )	{ m_tState = tState; m_tState.m_iNow = (DWORD) time ( NULL ); }
	bool		UsesAttrs ()											{ return m_bUsesAttrs; }
	CSphMatch *	First ()												{ return m_pData; }
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
		m_pData [ m_iUsed ] = tEntry;
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

	/// remove root (ie. top priority) entry
	virtual void Pop ()
	{
		assert ( m_iUsed );
		if ( !(--m_iUsed) ) // empty queue? just return
			return;

		// make the last entry my new root
		m_pData[0] = m_pData[m_iUsed];

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
			pTo[0] = m_pData[0]; // OPTIMIZE? reset dst + swap?
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			Pop ();
		}
		m_iTotal = 0;
	}
};

//////////////////////////////////////////////////////////////////////////
// SORTING+GROUPING QUEUE
//////////////////////////////////////////////////////////////////////////

/// groupers
#define GROUPER_BEGIN(_name) \
	class _name : public CSphGrouper \
	{ \
	protected: \
		CSphAttrLocator m_tLocator; \
	public: \
		_name ( const CSphAttrLocator & tLoc ) : m_tLocator ( tLoc ) {} \
		virtual void GetLocator ( CSphAttrLocator & tOut ) const { tOut = m_tLocator; } \
		virtual DWORD GetResultType () const { return m_tLocator.m_iBitCount>8*(int)sizeof(DWORD) ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER; } \
		virtual SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const { return KeyFromValue ( tMatch.GetAttr ( m_tLocator ) ); } \
		virtual SphGroupKey_t KeyFromValue ( SphAttr_t uValue ) const \
		{

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


/// HACK HACK HACK
class CSphGrouperAttrdiv : public CSphGrouper
{
protected:
	CSphAttrLocator m_tLocator;
	int m_iDiv;
public:
	CSphGrouperAttrdiv ( const CSphAttrLocator & tLoc, int iDiv ) : m_tLocator ( tLoc ), m_iDiv ( iDiv ) {}
	virtual void GetLocator ( CSphAttrLocator & tOut ) const { tOut = m_tLocator; }
	virtual SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const { return KeyFromValue ( tMatch.GetAttr ( m_tLocator ) ); }
	virtual SphGroupKey_t KeyFromValue ( SphAttr_t uValue ) const { return uValue/m_iDiv; }
	virtual DWORD GetResultType () const { return m_tLocator.m_iBitCount>8*(int)sizeof(DWORD) ? SPH_ATTR_BIGINT : SPH_ATTR_INTEGER; }
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
	CSphFixedHash ( int iLength )
	{
		int iBuckets = 2<<sphLog2(iLength-1); // less than 50% bucket usage guaranteed
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


struct IdentityHash_fn
{
	static inline uint64_t	Hash ( uint64_t iValue )	{ return iValue; }
	static inline DWORD		Hash ( DWORD iValue )		{ return iValue; }
	static inline int		Hash ( int iValue )			{ return iValue; }
};

/////////////////////////////////////////////////////////////////////////////

typedef uint64_t	SphGroupKey_t;

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
	int					m_iPresortRowitems;	///< how much raw (non-groupby) rowitems should the sorter expect
	int					m_iAddRowitems;		///< how much rowitems to add for non-grouped matches
	bool				m_bDistinct;		///< whether we need distinct
	bool				m_bMVA;				///< whether we're grouping by MVA attribute
	CSphGrouper *		m_pGrouper;			///< group key calculator

	CSphGroupSorterSettings ()
		: m_iPresortRowitems ( 0 )
		, m_iAddRowitems ( 0 )
		, m_bDistinct ( false )
		, m_bMVA ( false )
	{}
};


#if USE_WINDOWS
#pragma warning(disable:4127)
#endif


/// match sorter with k-buffering and group-by
template < typename COMPGROUP, bool DISTINCT >
class CSphKBufferGroupSorter : public CSphMatchQueueTraits
{
protected:
	const int		m_iRowitems;		///< normal match rowitems count (to distinguish already grouped matches)

	ESphGroupBy		m_eGroupBy;			///< group-by function
	CSphGrouper *	m_pGrouper;

	CSphFixedHash < CSphMatch *, SphGroupKey_t, IdentityHash_fn >	m_hGroup2Match;

protected:
	int				m_iLimit;		///< max matches to be retrieved

	CSphUniqounter	m_tUniq;
	CSphAttrLocator	m_tDistinctLoc;
	bool			m_bSortByDistinct;

	CSphMatchComparatorState	m_tStateGroup;
	const ISphMatchComparator *	m_pComp;

	CSphGroupSorterSettings		m_tSettings;

	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

public:
	/// ctor
	CSphKBufferGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings ) // FIXME! make k configurable
		: CSphMatchQueueTraits ( pQuery->m_iMaxMatches*GROUPBY_FACTOR, true )
		, m_iRowitems		( tSettings.m_iPresortRowitems )
		, m_eGroupBy		( pQuery->m_eGroupFunc )
		, m_pGrouper		( tSettings.m_pGrouper )
		, m_hGroup2Match	( pQuery->m_iMaxMatches*GROUPBY_FACTOR )
		, m_iLimit			( pQuery->m_iMaxMatches )
		, m_tDistinctLoc	( pQuery->m_tDistinctLoc )
		, m_bSortByDistinct	( false )
		, m_pComp			( pComp )
		, m_tSettings		( tSettings )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( DISTINCT==false || m_tDistinctLoc.m_iBitOffset>=0 );
	}

	/// dtor
	~CSphKBufferGroupSorter ()
	{
		SafeDelete ( m_pComp );
		SafeDelete ( m_pGrouper );
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		SphGroupKey_t uGroupKey = m_pGrouper->KeyFromMatch ( tEntry );
		return PushEx ( tEntry, uGroupKey );
	}

	/// add entry to the queue
	virtual bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey )
	{
		// check whether incoming match is already grouped
		assert ( tEntry.m_iRowitems==m_iRowitems || tEntry.m_iRowitems==m_iRowitems+m_tSettings.m_iAddRowitems );

		bool bGrouped = ( tEntry.m_iRowitems!=m_iRowitems );

		// if this group is already hashed, we only need to update the corresponding match
		CSphMatch ** ppMatch = m_hGroup2Match ( uGroupKey );
		if ( ppMatch )
		{
			CSphMatch * pMatch = (*ppMatch);
			assert ( pMatch );
			assert ( pMatch->GetAttr(m_tSettings.m_tLocGroupby)==uGroupKey );

			if ( bGrouped )
			{
				// it's already grouped match
				// sum grouped matches count
				assert ( pMatch->m_iRowitems==tEntry.m_iRowitems );
				pMatch->SetAttr ( m_tSettings.m_tLocCount, pMatch->GetAttr(m_tSettings.m_tLocCount) + tEntry.GetAttr(m_tSettings.m_tLocCount) ); // OPTIMIZE! AddAttr()?
				if ( DISTINCT )
					pMatch->SetAttr ( m_tSettings.m_tLocDistinct, pMatch->GetAttr(m_tSettings.m_tLocDistinct) + tEntry.GetAttr(m_tSettings.m_tLocDistinct) );

			} else
			{
				// it's a simple match
				// increase grouped matches count
				assert ( pMatch->m_iRowitems==tEntry.m_iRowitems+m_tSettings.m_iAddRowitems );
				pMatch->SetAttr ( m_tSettings.m_tLocCount, 1+pMatch->GetAttr(m_tSettings.m_tLocCount) ); // OPTIMIZE! IncAttr()?
			}

			// if new entry is more relevant, update from it
			if ( m_pComp->VirtualIsLess ( *pMatch, tEntry, m_tState ) )
			{
				pMatch->m_iDocID = tEntry.m_iDocID;
				pMatch->m_iWeight = tEntry.m_iWeight;
				pMatch->m_iTag = tEntry.m_iTag;

				for ( int i=0; i<pMatch->m_iRowitems-m_tSettings.m_iAddRowitems; i++ )
					pMatch->m_pRowitems[i] = tEntry.m_pRowitems[i];
			}
		}

		// submit actual distinct value in all cases
		if ( DISTINCT && !bGrouped )
			m_tUniq.Add ( SphGroupedValue_t ( uGroupKey, tEntry.GetAttr(m_tDistinctLoc) ) ); // OPTIMIZE! use simpler locator here?

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( ppMatch )
			return false;

		// if we're full, let's cut off some worst groups
		if ( m_iUsed==m_iSize )
			CutWorst ();

		// do add
		assert ( m_iUsed<m_iSize );
		CSphMatch & tNew = m_pData [ m_iUsed++ ];

		int iNewSize = tEntry.m_iRowitems + ( bGrouped ? 0 : m_tSettings.m_iAddRowitems );
		assert ( tNew.m_iRowitems==0 || tNew.m_iRowitems==iNewSize );

		if ( !tNew.m_iRowitems )
			tNew.Reset ( iNewSize );

		tNew.m_iDocID = tEntry.m_iDocID;
		tNew.m_iWeight = tEntry.m_iWeight;
		tNew.m_iTag = tEntry.m_iTag;

		memcpy ( tNew.m_pRowitems, tEntry.m_pRowitems, tEntry.m_iRowitems*sizeof(CSphRowitem) );

		if ( !bGrouped )
		{
			tNew.SetAttr ( m_tSettings.m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( m_tSettings.m_tLocCount, 1 );
			if ( DISTINCT )
				tNew.SetAttr ( m_tSettings.m_tLocDistinct, 0 );
		}

		m_hGroup2Match.Add ( &tNew, uGroupKey );
		m_iTotal++;
		return true;
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	void Flatten ( CSphMatch * pTo, int iTag )
	{
		CountDistinct ();
		SortGroups ();

		int iLen = GetLength ();
		for ( int i=0; i<iLen; i++, pTo++ )
		{
			pTo[0] = m_pData[i];
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
		m_tStateGroup = tState;

		if ( DISTINCT && m_tDistinctLoc.m_iBitOffset>=0 )
			for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
				if ( m_tStateGroup.m_tLocator[i].m_iBitOffset==m_tDistinctLoc.m_iBitOffset )
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
			for ( int iCount = m_tUniq.CountStart(&uGroup); iCount; iCount = m_tUniq.CountNext(&uGroup) )
			{
				CSphMatch ** ppMatch = m_hGroup2Match(uGroup);
				if ( ppMatch )
					(*ppMatch)->SetAttr ( m_tSettings.m_tLocDistinct, iCount );
			}
		}
	}

	/// cut worst N groups off the buffer tail
	void CutWorst ()
	{
		// sort groups
		if ( m_bSortByDistinct )
			CountDistinct ();
		SortGroups ();

		// cut groups
		int iCut = m_iLimit * (int)(GROUPBY_FACTOR/2);
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
		CSphMatch * pData = m_pData;
		int iCount = m_iUsed;

		int st0[32], st1[32], a, b, k, i, j;
		CSphMatch x;

		k = 1;
		st0[0] = 0;
		st1[0] = iCount-1;
		while ( k )
		{
			k--;
			i = a = st0[k];
			j = b = st1[k];
			x = pData [ (a+b)/2 ]; // FIXME! add better median at least
			while ( a<b )
			{
				while ( i<=j )
				{

					while ( COMPGROUP::IsLess ( x, pData[i], m_tStateGroup ) ) i++;
					while ( COMPGROUP::IsLess ( pData[j], x, m_tStateGroup ) ) j--;
					if (i <= j) { Swap ( pData[i], pData[j] ); i++; j--; }
				}

				if ( j-a>=b-i )
				{
					if ( a<j ) { st0[k] = a; st1[k] = j; k++; }
					a = i;
				} else
				{
					if ( i<b ) { st0[k] = i; st1[k] = b; k++; }
					b = j;
				}
			}
		}
	}
};


/// match sorter with k-buffering and group-by for MVAs
template < typename COMPGROUP, bool DISTINCT >
class CSphKBufferMVAGroupSorter : public CSphKBufferGroupSorter < COMPGROUP, DISTINCT >
{
protected:
	const DWORD *		m_pMva;		///< pointer to MVA pool for incoming matches
	CSphAttrLocator		m_tMvaLocator;

public:
	/// ctor
	CSphKBufferMVAGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: CSphKBufferGroupSorter < COMPGROUP, DISTINCT > ( pComp, pQuery, tSettings )
		, m_pMva ( NULL )
	{
		this->m_pGrouper->GetLocator ( m_tMvaLocator );
	}

	/// set MVA pool for subsequent matches
	void SetMVAPool ( const DWORD * pMva )
	{
		m_pMva = pMva;
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		// check whether incoming match is already grouped
		if ( tEntry.m_iRowitems!=this->m_iRowitems )
		{
			// it must be pre-grouped; well, just re-group it based on the group key
			// (this pointer is for gcc; it doesn't work otherwise)
			return PushEx ( tEntry, tEntry.GetAttr(this->m_tSettings.m_tLocGroupby) );
		}

		// ungrouped match
		if ( !m_pMva )
			return false;

		// (this pointer is for gcc; it doesn't work otherwise)
		SphAttr_t iMvaIndex = tEntry.GetAttr ( this->m_tMvaLocator ); // FIXME! OPTIMIZE! use simpler locator than full bits/count here
		if ( !iMvaIndex )
			return false;

		const DWORD * pValues = m_pMva + iMvaIndex; // FIXME! hardcoded MVA type
		DWORD iValues = *pValues++;

		bool bRes = false;
		while ( iValues-- )
		{
			SphGroupKey_t uGroupkey = this->m_pGrouper->KeyFromValue ( *pValues++ );
			bRes |= this->PushEx ( tEntry, uGroupkey );
		}
		return bRes;
	}
};


#if USE_WINDOWS
#pragma warning(default:4127)
#endif

//////////////////////////////////////////////////////////////////////////
// PLAIN SORTING FUNCTORS 
//////////////////////////////////////////////////////////////////////////

/// match sorter
template < bool BITS >
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
template < bool BITS >
struct MatchAttrLt_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphAttr_t aa = sphGetCompAttr<BITS> ( t, a, 0 );
		SphAttr_t bb = sphGetCompAttr<BITS> ( t, b, 0 );
		if ( aa!=bb )
			return aa<bb;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
template < bool BITS >
struct MatchAttrGt_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphAttr_t aa = sphGetCompAttr<BITS> ( t, a, 0 );
		SphAttr_t bb = sphGetCompAttr<BITS> ( t, b, 0 );
		if ( aa!=bb )
			return aa>bb;

		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return a.m_iDocID > b.m_iDocID;
	};
};


/// match sorter
template < bool BITS >
struct MatchTimeSegments_fn : public ISphMatchComparator
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SphAttr_t aa = sphGetCompAttr<BITS> ( t, a, 0 );
		SphAttr_t bb = sphGetCompAttr<BITS> ( t, b, 0 );
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
		return ( (t.m_uAttrDesc>>(_idx))&1 ) ^ ( (_aa)>(_bb) );


#define SPH_TEST_KEYPART(_idx) \
	switch ( t.m_iAttr[_idx] ) \
	{ \
		case SPH_VATTR_ID:			SPH_TEST_PAIR ( a.m_iDocID, b.m_iDocID, _idx ); break; \
		case SPH_VATTR_RELEVANCE:	SPH_TEST_PAIR ( a.m_iWeight, b.m_iWeight, _idx ); break; \
		case SPH_VATTR_FLOAT: \
		{ \
			register float aa = a.GetAttrFloat ( t.m_tLocator[_idx] ); \
			register float bb = b.GetAttrFloat ( t.m_tLocator[_idx] ); \
			SPH_TEST_PAIR ( aa, bb, _idx ) \
			break; \
		} \
		default: \
		{ \
			register SphAttr_t aa = sphGetCompAttr<BITS> ( t, a, _idx ); \
			register SphAttr_t bb = sphGetCompAttr<BITS> ( t, b, _idx ); \
			SPH_TEST_PAIR ( aa, bb, _idx ); \
			break; \
		} \
	}


template < bool BITS >
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


template < bool BITS >
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


template < bool BITS >
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


template < bool BITS >
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

template < bool BITS >
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
		tState.m_iAttr[iIdx] = iAttr;
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
;

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
#define MATCH_DECLARE_ATTR(_name)	;
#define MATCH_WEIGHT				float(MATCH_VAR.m_iWeight)
#define MATCH_NOW					float(t.m_iNow)
#define MATCH_ATTR(_idx)			float(sphGetCompAttr<BITS>(t,MATCH_VAR,_idx))

		float aa, bb;

#undef MATCH_FUNCTION
#undef MATCH_VAR
#define MATCH_FUNCTION aa
#define MATCH_VAR a
#include "sphinxcustomsort.inl"
;

#undef MATCH_FUNCTION
#undef MATCH_VAR
#define MATCH_FUNCTION bb
#define MATCH_VAR b
#include "sphinxcustomsort.inl"
;

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


enum
{
	FIXUP_GEODIST	= -1000,
	FIXUP_COUNT		= -1001,
	FIXUP_GROUP		= -1002,
	FIXUP_DISTINCT	= -1003,
	FIXUP_EXPR		= -1004
};


static ESortClauseParseResult sphParseSortClause ( const char * sClause, const CSphQuery * pQuery,
	const CSphSchema & tSchema, bool bGroupClause,
	ESphSortFunc & eFunc, CSphMatchComparatorState & tState, CSphString & sError )
{
	// mini parser
	CSphScopedPtr<ISphTokenizer> pTokenizer ( sphCreateSBCSTokenizer () );
	if ( !pTokenizer.Ptr() )
	{
		sError.SetSprintf ( "INTERNAL ERROR: failed to create tokenizer when parsing sort clause" );
		return SORT_CLAUSE_ERROR;
	}

	CSphString sTmp;
	pTokenizer->SetCaseFolding ( "0..9, A..Z->a..z, _, a..z, @", sTmp );

	CSphString sSortClause ( sClause );
	pTokenizer->SetBuffer ( (BYTE*)sSortClause.cstr(), strlen(sSortClause.cstr())  );

	bool bField = false; // whether i'm expecting field name or sort order
	int iField = 0;

	for ( const char * pTok=(char*)pTokenizer->GetToken(); pTok; pTok=(char*)pTokenizer->GetToken() )
	{
		bField = !bField;

		// special case, sort by random
		if ( iField==0 && bField && strcmp ( pTok, "@random" )==0 )
			return SORT_CLAUSE_RANDOM;

		// special case, sort by custom
		if ( iField==0 && bField && strcmp ( pTok, "@custom" )==0 )
		{
			eFunc = FUNC_CUSTOM;
			return MatchCustom_fn<false>::Setup ( tSchema, tState, sError ) ? SORT_CLAUSE_OK : SORT_CLAUSE_ERROR;
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

		// handle field name
		if ( iField==MAX_SORT_FIELDS )
		{
			sError.SetSprintf ( "too much sort-by fields; maximum count is %d", MAX_SORT_FIELDS );
			return SORT_CLAUSE_ERROR;
		}

		if ( !strcasecmp ( pTok, "@relevance" )
			|| !strcasecmp ( pTok, "@rank" )
			|| !strcasecmp ( pTok, "@weight" ) )
		{
			tState.m_iAttr[iField] = SPH_VATTR_RELEVANCE;

		} else if ( !strcasecmp ( pTok, "@id" ) )
		{
			tState.m_iAttr[iField] = SPH_VATTR_ID;

		} else if ( !strcasecmp ( pTok, "@geodist" ) )
		{
			tState.m_iAttr[iField] = FIXUP_GEODIST;

		} else if ( !strcasecmp ( pTok, "@count" ) && bGroupClause )
		{
			if ( !pQuery->IsGroupby() )
			{
				sError.SetSprintf ( "no group-by attribute; can not sort by @count" );
				return SORT_CLAUSE_ERROR;
			}
			tState.m_iAttr[iField] = FIXUP_COUNT;

		} else if ( ( !strcasecmp ( pTok, "@group" ) || !strcasecmp ( pTok, "@groupby" ) ) && bGroupClause )
		{
			if ( !pQuery->IsGroupby() )
			{
				sError.SetSprintf ( "no group-by attribute; can not sort by @groupby" );
				return SORT_CLAUSE_ERROR;
			}
			tState.m_iAttr[iField] = FIXUP_GROUP;

		} else if ( !strcasecmp ( pTok, "@distinct" ) && bGroupClause )
		{
			if ( pQuery->m_tDistinctLoc.m_iBitOffset<0 )
			{
				sError.SetSprintf ( "no count-distinct attribute; can not sort by @distinct" );
				return SORT_CLAUSE_ERROR;
			}
			tState.m_iAttr[iField] = FIXUP_DISTINCT;

		} else
		{
			int iAttr = tSchema.GetAttrIndex ( pTok );
			if ( iAttr<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pTok );
				return SORT_CLAUSE_ERROR;
			}
			tState.m_iAttr[iField] = ( tSchema.GetAttr(iAttr).m_eAttrType==SPH_ATTR_FLOAT ) ? SPH_VATTR_FLOAT : iAttr;
			tState.m_tLocator[iField] = tSchema.GetAttr(iAttr).m_tLocator;
		}
	}

	if ( iField==0 )
	{
		sError.SetSprintf ( "no sort order defined" );
		return SORT_CLAUSE_ERROR;
	}

	if ( iField==1 )
		tState.m_iAttr[iField++] = SPH_VATTR_ID; // add "id ASC"

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
	if ( tSettings.m_bMVA==true )
	{
		if ( tSettings.m_bDistinct==true )
			return new CSphKBufferMVAGroupSorter<COMPGROUP,true> ( pComp, pQuery, tSettings);
		else
			return new CSphKBufferMVAGroupSorter<COMPGROUP,false> ( pComp, pQuery, tSettings );
	} else
	{
		if ( tSettings.m_bDistinct==true )
			return new CSphKBufferGroupSorter<COMPGROUP,true> ( pComp, pQuery, tSettings );
		else
			return new CSphKBufferGroupSorter<COMPGROUP,false> ( pComp, pQuery, tSettings );
	}
}


static ISphMatchSorter * sphCreateSorter2nd ( ESphSortFunc eGroupFunc, bool bGroupBits, const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
{
	if ( bGroupBits )
	{
		switch ( eGroupFunc )
		{
			case FUNC_GENERIC2:		return sphCreateSorter3rd<MatchGeneric2_fn<true> >	( pComp, pQuery, tSettings ); break;
			case FUNC_GENERIC3:		return sphCreateSorter3rd<MatchGeneric3_fn<true> >	( pComp, pQuery, tSettings ); break;
			case FUNC_GENERIC4:		return sphCreateSorter3rd<MatchGeneric4_fn<true> >	( pComp, pQuery, tSettings ); break;
			case FUNC_GENERIC5:		return sphCreateSorter3rd<MatchGeneric5_fn<true> >	( pComp, pQuery, tSettings ); break;
			case FUNC_CUSTOM:		return sphCreateSorter3rd<MatchCustom_fn<true>   >	( pComp, pQuery, tSettings ); break;
			default:				return NULL;
		}
	} else
	{
		switch ( eGroupFunc )
		{
			case FUNC_GENERIC2:		return sphCreateSorter3rd<MatchGeneric2_fn<false> >	( pComp, pQuery, tSettings ); break;
			case FUNC_GENERIC3:		return sphCreateSorter3rd<MatchGeneric3_fn<false> >	( pComp, pQuery, tSettings ); break;
			case FUNC_GENERIC4:		return sphCreateSorter3rd<MatchGeneric4_fn<false> >	( pComp, pQuery, tSettings ); break;
			case FUNC_GENERIC5:		return sphCreateSorter3rd<MatchGeneric5_fn<false> >	( pComp, pQuery, tSettings ); break;
			case FUNC_CUSTOM:		return sphCreateSorter3rd<MatchCustom_fn<false>   >	( pComp, pQuery, tSettings ); break;
			case FUNC_EXPR:			return sphCreateSorter3rd<MatchExpr_fn>				( pComp, pQuery, tSettings ); break;
			default:				return NULL;
		}
	}
}


static ISphMatchSorter * sphCreateSorter1st ( ESphSortFunc eMatchFunc, bool bMatchBits, ESphSortFunc eGroupFunc, bool bGroupBits, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
{
	ISphMatchComparator * pComp = NULL;
	if ( bMatchBits )
	{
		switch ( eMatchFunc )
		{
			case FUNC_REL_DESC:		pComp = new MatchRelevanceLt_fn<true>(); break;
			case FUNC_ATTR_DESC:	pComp = new MatchAttrLt_fn<true>(); break;
			case FUNC_ATTR_ASC:		pComp = new MatchAttrGt_fn<true>(); break;
			case FUNC_TIMESEGS:		pComp = new MatchTimeSegments_fn<true>(); break;
			case FUNC_GENERIC2:		pComp = new MatchGeneric2_fn<true>(); break;
			case FUNC_GENERIC3:		pComp = new MatchGeneric3_fn<true>(); break;
			case FUNC_GENERIC4:		pComp = new MatchGeneric4_fn<true>(); break;
			case FUNC_GENERIC5:		pComp = new MatchGeneric5_fn<true>(); break;
			case FUNC_CUSTOM:		pComp = new MatchCustom_fn<true>(); break;
			case FUNC_EXPR:			pComp = NULL; break; // expr can't be bitfield
		}
	} else
	{
		switch ( eMatchFunc )
		{
			case FUNC_REL_DESC:		pComp = new MatchRelevanceLt_fn<false>(); break;
			case FUNC_ATTR_DESC:	pComp = new MatchAttrLt_fn<false>(); break;
			case FUNC_ATTR_ASC:		pComp = new MatchAttrGt_fn<false>(); break;
			case FUNC_TIMESEGS:		pComp = new MatchTimeSegments_fn<false>(); break;
			case FUNC_GENERIC2:		pComp = new MatchGeneric2_fn<false>(); break;
			case FUNC_GENERIC3:		pComp = new MatchGeneric3_fn<false>(); break;
			case FUNC_GENERIC4:		pComp = new MatchGeneric4_fn<false>(); break;
			case FUNC_GENERIC5:		pComp = new MatchGeneric5_fn<false>(); break;
			case FUNC_CUSTOM:		pComp = new MatchCustom_fn<false>(); break;
			case FUNC_EXPR:			pComp = new MatchExpr_fn(); break; // only for non-bitfields, obviously
		}
	}

	return pComp
		? sphCreateSorter2nd ( eGroupFunc, bGroupBits, pComp, pQuery, tSettings )
		: NULL;
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS (FACTORY AND FLATTENING)
//////////////////////////////////////////////////////////////////////////

static bool UpdateQueryForSchema ( CSphQuery * pQuery, const CSphSchema & tSchema, CSphGroupSorterSettings & tSettings, CSphString & sError )
{
	pQuery->m_tDistinctLoc.m_iBitOffset = -1;

	if ( pQuery->m_sGroupBy.IsEmpty() )
		return true;

	if ( pQuery->m_eGroupFunc==SPH_GROUPBY_ATTRPAIR )
	{
		sError.SetSprintf ( "SPH_GROUPBY_ATTRPAIR is not supported any more (just group on 'bigint' attribute)" );
		return false;
	}

	// setup groupby attr
	int iGroupBy = -1;
	if ( pQuery->m_eGroupFunc==SPH_GROUPBY_EXTENDED )
	{
		// groupby clause
		// HACK HACK HACK, implement full blown expr support here
		CSphString sBuf = pQuery->m_sGroupBy;

		char * p = strchr ( (char*)sBuf.cstr(), '/' );
		if ( p )
			*p++ = '\0';

		iGroupBy = tSchema.GetAttrIndex ( sBuf.cstr() );
		if ( iGroupBy<0 )
		{
			sError.SetSprintf ( "group-by attribute '%s' not found", sBuf.cstr() );
			return false;
		}

		if ( p )
		{
			int iDiv = strtoul ( p, NULL, 10 );
			if ( !iDiv )
			{
				sError.SetSprintf ( "group-by clause: division by zero" );
				return false;
			}
			tSettings.m_pGrouper = new CSphGrouperAttrdiv ( tSchema.GetAttr(iGroupBy).m_tLocator, iDiv );
		} else
		{
			tSettings.m_pGrouper = new CSphGrouperAttr ( tSchema.GetAttr(iGroupBy).m_tLocator );
		}

	} else
	{
		// plain old name
		iGroupBy = tSchema.GetAttrIndex ( pQuery->m_sGroupBy.cstr() );
		if ( iGroupBy<0 )
		{
			sError.SetSprintf ( "group-by attribute '%s' not found", pQuery->m_sGroupBy.cstr() );
			return false;
		}

		CSphAttrLocator tLoc = tSchema.GetAttr(iGroupBy).m_tLocator;
		switch ( pQuery->m_eGroupFunc )
		{
			case SPH_GROUPBY_DAY:		tSettings.m_pGrouper = new CSphGrouperDay ( tLoc ); break;
			case SPH_GROUPBY_WEEK:		tSettings.m_pGrouper = new CSphGrouperWeek ( tLoc ); break;
			case SPH_GROUPBY_MONTH:		tSettings.m_pGrouper = new CSphGrouperMonth ( tLoc ); break;
			case SPH_GROUPBY_YEAR:		tSettings.m_pGrouper = new CSphGrouperYear ( tLoc ); break;
			case SPH_GROUPBY_ATTR:		tSettings.m_pGrouper = new CSphGrouperAttr ( tLoc ); break;
			default:
				sError.SetSprintf ( "invalid group-by mode (mode=%d)", pQuery->m_eGroupFunc );
				return false;
		}
	}

	tSettings.m_bMVA = ( tSchema.GetAttr(iGroupBy).m_eAttrType & SPH_ATTR_MULTI )!=0;

	// setup distinct attr
	if ( !pQuery->m_sGroupDistinct.IsEmpty() )
	{
		int iDistinct = tSchema.GetAttrIndex ( pQuery->m_sGroupDistinct.cstr() );
		if ( iDistinct<0 )
		{
			sError.SetSprintf ( "group-count-distinct attribute '%s' not found", pQuery->m_sGroupDistinct.cstr() );
			return false;
		}

		pQuery->m_tDistinctLoc = tSchema.GetAttr(iDistinct).m_tLocator;
	}

	return true;
}


ISphMatchSorter * sphCreateQueue ( CSphQuery * pQuery, const CSphSchema & tSchema, CSphString & sError )
{
	CSphGroupSorterSettings tSettings;

	// lookup proper attribute index to group by, if any
	if ( !UpdateQueryForSchema ( pQuery, tSchema, tSettings, sError ) )
		return NULL;
	assert ( pQuery->m_sGroupBy.IsEmpty() || pQuery->IsGroupby() );

	// prepare
	ISphMatchSorter * pTop = NULL;
	CSphMatchComparatorState tStateMatch, tStateGroup;

	sError = "";

	////////////////////////////////////
	// choose and setup sorting functor
	////////////////////////////////////

	ESphSortFunc eMatchFunc = FUNC_REL_DESC;
	ESphSortFunc eGroupFunc = FUNC_REL_DESC;
	bool bUsesAttrs = false;
	bool bRandomize = false;
	pQuery->m_bCalcGeodist = false;

	// matches sorting function
	if ( pQuery->m_eSort==SPH_SORT_EXTENDED )
	{
		ESortClauseParseResult eRes = sphParseSortClause ( pQuery->m_sSortBy.cstr(), pQuery, tSchema,
			false, eMatchFunc, tStateMatch, sError );

		if ( eRes==SORT_CLAUSE_ERROR )
			return NULL;

		if ( eRes==SORT_CLAUSE_RANDOM )
			bRandomize = true;

		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
		{
			if ( tStateMatch.m_iAttr[i]>=0 )
				bUsesAttrs = true;
			if ( tStateMatch.m_iAttr[i]==FIXUP_GEODIST )
			{
				bUsesAttrs = true;
				pQuery->m_bCalcGeodist = true;
			}
		}

	} else if ( pQuery->m_eSort==SPH_SORT_EXPR )
	{
		SafeDelete ( pQuery->m_pExpr );
		pQuery->m_pExpr = sphExprParse ( pQuery->m_sSortBy.cstr(), tSchema, sError );
		if ( !pQuery->m_pExpr )
			return NULL;

		tStateMatch.m_iAttr[0] = FIXUP_EXPR;
		tStateMatch.m_iAttr[1] = SPH_VATTR_ID;
		tStateMatch.m_uAttrDesc = 1;
		eMatchFunc = FUNC_EXPR;
		bUsesAttrs = true;

	} else
	{
		// check sort-by attribute
		if ( pQuery->m_eSort!=SPH_SORT_RELEVANCE )
		{
			tStateMatch.m_iAttr[0] = tSchema.GetAttrIndex ( pQuery->m_sSortBy.cstr() );
			if ( tStateMatch.m_iAttr[0]<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pQuery->m_sSortBy.cstr() );
				return NULL;
			}

			const CSphColumnInfo & tAttr = tSchema.GetAttr ( tStateMatch.m_iAttr[0] );
			tStateMatch.m_tLocator[0] = tAttr.m_tLocator;
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
	if ( pQuery->IsGroupby() )
	{
		ESortClauseParseResult eRes = sphParseSortClause ( pQuery->m_sGroupSortBy.cstr(), pQuery, tSchema,
			true, eGroupFunc, tStateGroup, sError );

		if ( eRes==SORT_CLAUSE_ERROR || eRes==SORT_CLAUSE_RANDOM )
		{
			if ( eRes==SORT_CLAUSE_RANDOM )
				sError.SetSprintf ( "groups can not be sorted by @random" );
			return NULL;
		}
	}

	// update for calculated stuff
	ARRAY_FOREACH ( i, pQuery->m_dFilters )
		if ( pQuery->m_dFilters[i].m_sAttrName=="@geodist" )
	{
		pQuery->m_bCalcGeodist = true;
		break;
	}

	// create the new schema
	// (note that in theory incoming schema should contain either no virtual columns at all, or both calculated and groupby virtual columns already)
	// WARNING, THIS MUST BE IN SYNC WITH WHAT INDEX ACTUALLY DOES
	CSphSchema tNewSchema = tSchema;

	if ( pQuery->m_bCalcGeodist && tNewSchema.GetAttrIndex ( "@geodist" )<0 )
	{
		CSphColumnInfo tCol ( "@geodist", SPH_ATTR_FLOAT );
		tNewSchema.AddAttr ( tCol );
	}

	if ( pQuery->m_eSort==SPH_SORT_EXPR && tNewSchema.GetAttrIndex ( "@expr" )<0 )
	{
		CSphColumnInfo tCol ( "@expr", SPH_ATTR_FLOAT );
		tNewSchema.AddAttr ( tCol );
	}

	if ( pQuery->IsGroupby() && tNewSchema.GetAttrIndex ( "@groupby" )<0 )
	{
		CSphColumnInfo tGroupby ( "@groupby", tSettings.m_pGrouper->GetResultType() );
		CSphColumnInfo tCount ( "@count", SPH_ATTR_INTEGER );
		CSphColumnInfo tDistinct ( "@distinct", SPH_ATTR_INTEGER );

		tNewSchema.AddAttr ( tGroupby );
		tNewSchema.AddAttr ( tCount );
		if ( pQuery->HasDistinct() )
			tNewSchema.AddAttr ( tDistinct );
	}

	int iGroupby = tNewSchema.GetAttrIndex ( "@groupby" );
	if ( iGroupby>=0 )
	{
		tSettings.m_iPresortRowitems = tNewSchema.GetAttr ( iGroupby ).m_tLocator.CalcRowitem(); // how much raw (non-groupby) rowitems should the sorter expect
		tSettings.m_iAddRowitems = tNewSchema.GetRowSize() - tSettings.m_iPresortRowitems; // how much groupby rowitems the sorter should add for non-grouped matches
		tSettings.m_bDistinct = ( pQuery->m_tDistinctLoc.m_iBitOffset>=0 );

		tSettings.m_tLocGroupby = tNewSchema.GetAttr ( iGroupby ).m_tLocator;

		int iCount = tNewSchema.GetAttrIndex ( "@count" );
		if ( iCount>=0 )
			tSettings.m_tLocCount = tNewSchema.GetAttr ( iCount ).m_tLocator;

		int iDistinct = tNewSchema.GetAttrIndex ( "@distinct" );
		if ( iDistinct>=0 )
			tSettings.m_tLocDistinct = tNewSchema.GetAttr ( iDistinct ).m_tLocator;
	}

	// perform fixup
	for ( int iState=0; iState<2; iState++ )
	{
		CSphMatchComparatorState & tState = iState ? tStateMatch : tStateGroup;
		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
		{
			const char * sName = NULL;
			switch ( tState.m_iAttr[i] )
			{
				case FIXUP_GEODIST:		sName = "@geodist"; break;
				case FIXUP_EXPR:		sName = "@expr"; break;
				case FIXUP_GROUP:		sName = "@groupby"; break;
				case FIXUP_COUNT:		sName = "@count"; break;
				case FIXUP_DISTINCT:	sName = "@distinct"; break;
				default:				continue;
			}

			assert ( sName );
			int iAttr = tNewSchema.GetAttrIndex(sName);

			if ( iAttr<0 || iAttr>=tNewSchema.GetAttrsCount() )
			{
				sError.SetSprintf ( "INTERNAL ERROR: fixup failed for '%s' virtual attribute", sName );
				return NULL;
			}

			const CSphColumnInfo & tCol = tNewSchema.GetAttr(iAttr);
			tState.m_iAttr[i] = iAttr;
			tState.m_tLocator[i] = tCol.m_tLocator;
		}
	}

	///////////////////
	// spawn the queue
	///////////////////

	bool bMatchBits = tStateMatch.UsesBitfields ();
	bool bGroupBits = tStateGroup.UsesBitfields ();

	if ( !pQuery->IsGroupby() )
	{
		if ( bMatchBits )
		{
			switch ( eMatchFunc )
			{
				case FUNC_REL_DESC:	pTop = new CSphMatchQueue<MatchRelevanceLt_fn<true> >	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_DESC:pTop = new CSphMatchQueue<MatchAttrLt_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_ASC:	pTop = new CSphMatchQueue<MatchAttrGt_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_TIMESEGS:	pTop = new CSphMatchQueue<MatchTimeSegments_fn<true> >	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC2:	pTop = new CSphMatchQueue<MatchGeneric2_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC3:	pTop = new CSphMatchQueue<MatchGeneric3_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC4:	pTop = new CSphMatchQueue<MatchGeneric4_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC5:	pTop = new CSphMatchQueue<MatchGeneric5_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_CUSTOM:	pTop = new CSphMatchQueue<MatchCustom_fn<true> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				default:			pTop = NULL;
			}
		} else
		{
			switch ( eMatchFunc )
			{
				case FUNC_REL_DESC:	pTop = new CSphMatchQueue<MatchRelevanceLt_fn<false> >	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_DESC:pTop = new CSphMatchQueue<MatchAttrLt_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_ATTR_ASC:	pTop = new CSphMatchQueue<MatchAttrGt_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_TIMESEGS:	pTop = new CSphMatchQueue<MatchTimeSegments_fn<false> >	( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC2:	pTop = new CSphMatchQueue<MatchGeneric2_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC3:	pTop = new CSphMatchQueue<MatchGeneric3_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC4:	pTop = new CSphMatchQueue<MatchGeneric4_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_GENERIC5:	pTop = new CSphMatchQueue<MatchGeneric5_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_CUSTOM:	pTop = new CSphMatchQueue<MatchCustom_fn<false> >		( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				case FUNC_EXPR:		pTop = new CSphMatchQueue<MatchExpr_fn>					( pQuery->m_iMaxMatches, bUsesAttrs ); break;
				default:			pTop = NULL;
			}
		}

	} else
	{
		pTop = sphCreateSorter1st ( eMatchFunc, bMatchBits, eGroupFunc, bGroupBits, pQuery, tSettings );
	}

	if ( !pTop )
	{
		sError.SetSprintf ( "internal error: unhandled sorting mode (match-sort=%d, group=%d, group-sort=%d)",
			eMatchFunc, pQuery->IsGroupby(), eGroupFunc );
		return NULL;
	}

	assert ( pTop );
	pTop->SetState ( tStateMatch );
	pTop->SetGroupState ( tStateGroup );
	pTop->m_bRandomize = bRandomize;
	pTop->m_tSchema = tNewSchema;
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

//
// $Id$
//
