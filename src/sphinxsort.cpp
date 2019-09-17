//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"
#include "sphinxjson.h"
#include "attribute.h"

#include <time.h>
#include <math.h>

#if !USE_WINDOWS
#include <unistd.h>
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////
// TRAITS
//////////////////////////////////////////////////////////////////////////

void sphFixupLocator ( CSphAttrLocator & tLocator, const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	// first time schema setup?
	if ( !pOldSchema )
		return;

	assert ( pNewSchema );
	for ( int i = 0; i < pOldSchema->GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = pOldSchema->GetAttr(i);
		if ( tLocator==tAttr.m_tLocator )
		{
			const CSphColumnInfo * pAttrInNewSchema = pNewSchema->GetAttr ( tAttr.m_sName.cstr() );
			if ( pAttrInNewSchema )
			{
				tLocator = pAttrInNewSchema->m_tLocator;
				return;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void ISphMatchSorter::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	assert ( pSchema );
	m_tState.FixupLocators ( m_pSchema, pSchema, bRemapCmp );

	m_pSchema = pSchema;
}


void ISphMatchSorter::SetState ( const CSphMatchComparatorState & tState )
{
	m_tState = tState;
	for ( ISphExpr *&pExpr :  m_tState.m_tSubExpr ) SafeAddRef ( pExpr );
	m_tState.m_iNow = (DWORD) time ( nullptr );
}


/// groupby key type
typedef int64_t				SphGroupKey_t;

static const char g_sIntAttrPrefix[] = "@int_str2ptr_";
static const char g_sIntJsonPrefix[] = "@groupbystr";


/// base grouper (class that computes groupby key)
class CSphGrouper : public ISphRefcounted
{
protected:
	virtual					~CSphGrouper () {}; // =default causes bunch of errors building on wheezy
public:
	virtual SphGroupKey_t	KeyFromValue ( SphAttr_t uValue ) const = 0;
	virtual SphGroupKey_t	KeyFromMatch ( const CSphMatch & tMatch ) const = 0;
	virtual void			GetLocator ( CSphAttrLocator & tOut ) const = 0;
	virtual ESphAttr		GetResultType () const = 0;
	virtual void			SetBlobPool ( const BYTE * pBlobPool ) { m_pBlobPool = pBlobPool; }
	virtual const BYTE *	GetBlobPool() { return m_pBlobPool; }

protected:
	const BYTE *			m_pBlobPool {nullptr};
};


/// match-sorting priority queue traits
class CSphMatchQueueTraits : public ISphMatchSorter, ISphNoncopyable
{
protected:
	CSphMatch *					m_pData;
	int							m_iUsed = 0;
	int							m_iSize;

private:
	const int					m_iDataLength;

public:
	/// ctor
	explicit CSphMatchQueueTraits ( int iSize )
		: m_iSize ( iSize )
		, m_iDataLength ( iSize )
	{
		assert ( iSize>0 );
		m_pData = new CSphMatch [ m_iDataLength ];
		assert ( m_pData );

		m_tState.m_iNow = (DWORD) time ( nullptr );
		m_iMatchCapacity = m_iDataLength;
	}

	/// dtor
	~CSphMatchQueueTraits () override
	{
		if ( m_pSchema )
		{
			for ( int i=0; i<m_iDataLength; ++i )
				m_pSchema->FreeDataPtrs ( m_pData+i );
		}

		SafeDeleteArray ( m_pData );
	}

public:
	int			GetLength () const override								{ return m_iUsed; }
	int			GetDataLength () const override							{ return m_iDataLength; }
};

//////////////////////////////////////////////////////////////////////////
// SORTING QUEUES
//////////////////////////////////////////////////////////////////////////

template < typename COMP >
struct CompareIndex_fn
{
	const CSphMatch * m_pBase;
	const CSphMatchComparatorState * m_pState;

	CompareIndex_fn ( const CSphMatch * pBase, const CSphMatchComparatorState * pState )
		: m_pBase ( pBase )
		, m_pState ( pState )
	{}

	bool IsLess ( int a, int b ) const
	{
		return COMP::IsLess ( m_pBase[b], m_pBase[a], *m_pState );
	}
};

/// heap sorter
/// plain binary heap based PQ
template < typename COMP, bool NOTIFICATIONS >
class CSphMatchQueue : public CSphMatchQueueTraits
{
public:
	/// ctor
	CSphMatchQueue ( int iSize )
		: CSphMatchQueueTraits ( iSize )
	{
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);
	}

	/// check if this sorter does groupby
	bool IsGroupby () const override
	{
		return false;
	}

	const CSphMatch * GetWorst() const override
	{
		return m_pData;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		m_iTotal++;

		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
			m_dJustPopped.Resize(0);
		}

		if ( m_iUsed==m_iSize )
		{
			// if it's worse that current min, reject it, else pop off current min
			if ( COMP::IsLess ( tEntry, m_pData[0], m_tState ) )
				return true;
			else
				Pop ();
		}

		// do add
		m_pSchema->CloneMatch ( m_pData+m_iUsed, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tEntry.m_tRowID;

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
	bool PushGrouped ( const CSphMatch &, bool ) override
	{
		assert ( false );
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
		m_pSchema->FreeDataPtrs ( &m_pData[m_iUsed] );

		if_const ( NOTIFICATIONS )
		{
			if ( m_dJustPopped.GetLength() )
				m_dJustPopped[0] = m_pData[m_iUsed].m_tRowID;
			else
				m_dJustPopped.Add ( m_pData[m_iUsed].m_tRowID );
		}

		// sift down if needed
		int iEntry = 0;
		while (true)
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
	int Flatten ( CSphMatch * pTo, int iTag ) override
	{
		assert ( m_iUsed>=0 );
		pTo += m_iUsed;
		int iCopied = m_iUsed;
		while ( m_iUsed>0 )
		{
			--pTo;
			m_pSchema->FreeDataPtrs ( pTo );
			Swap ( *pTo, *m_pData );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			Pop ();
		}
		m_iTotal = 0;
		return iCopied;
	}

	void Finalize ( ISphMatchProcessor & tProcessor, bool bCallProcessInResultSetOrder ) override
	{
		if ( !GetLength() )
			return;

		if ( !bCallProcessInResultSetOrder )
		{
			// just evaluate in heap order
			CSphMatch * pCur = m_pData;
			const CSphMatch * pEnd = m_pData + m_iUsed;
			while ( pCur<pEnd )
			{
				tProcessor.Process ( pCur++ );
			}
		} else
		{
			// means final-stage calls will be evaluated
			// a) over the final, pre-limit result set
			// b) in the final result set order
			CSphFixedVector<int> dIndexes ( GetLength() );
			ARRAY_FOREACH ( i, dIndexes )
				dIndexes[i] = i;
			sphSort ( dIndexes.Begin(), dIndexes.GetLength(), CompareIndex_fn<COMP> ( m_pData, &m_tState ) );

			ARRAY_FOREACH ( i, dIndexes )
			{
				tProcessor.Process ( m_pData + dIndexes[i] );
			}
		}
	}
};

//////////////////////////////////////////////////////////////////////////

/// match sorting functor
template < typename COMP >
struct MatchSort_fn : public MatchSortAccessor_t
{
	CSphMatchComparatorState	m_tState;

	explicit MatchSort_fn ( const CSphMatchComparatorState & tState )
		: m_tState ( tState )
	{}

	bool IsLess ( const MEDIAN_TYPE a, const MEDIAN_TYPE b ) const
	{
		return COMP::IsLess ( *a, *b, m_tState );
	}
};


/// K-buffer (generalized double buffer) sorter
/// faster worst-case but slower average-case than the heap sorter
template < typename COMP, bool NOTIFICATIONS >
class CSphKbufferMatchQueue : public CSphMatchQueueTraits
{
protected:
	static const int	COEFF = 4;

	CSphMatch *			m_pEnd;
	CSphMatch *			m_pWorst;
	bool				m_bFinalized;

public:
	/// ctor
	CSphKbufferMatchQueue ( int iSize )
		: CSphMatchQueueTraits ( iSize*COEFF )
		, m_pEnd ( m_pData+iSize*COEFF )
		, m_pWorst ( nullptr )
		, m_bFinalized ( false )
	{
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );

		m_iSize /= COEFF;
	}

	/// check if this sorter does groupby
	bool IsGroupby () const override
	{
		return false;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
			m_dJustPopped.Resize(0);
		}

		// quick early rejection checks
		m_iTotal++;
		if ( m_pWorst && COMP::IsLess ( tEntry, *m_pWorst, m_tState ) )
			return true;

		// quick check passed
		// fill the data, back to front
		m_bFinalized = false;
		m_iUsed++;
		m_pSchema->CloneMatch ( m_pEnd-m_iUsed, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tEntry.m_tRowID;

		// do the initial sort once
		if ( m_iTotal==m_iSize )
		{
			assert ( m_iUsed==m_iSize && !m_pWorst );
			MatchSort_fn<COMP> tComp ( m_tState );
			sphSort ( m_pEnd-m_iSize, m_iSize, tComp, tComp );
			m_pWorst = m_pEnd-m_iSize;
			m_bFinalized = true;
			return true;
		}

		// do the sort/cut when the K-buffer is full
		if ( m_iUsed==m_iSize*COEFF )
		{
			MatchSort_fn<COMP> tComp ( m_tState );
			sphSort ( m_pData, m_iUsed, tComp, tComp );

			if_const ( NOTIFICATIONS )
			{
				for ( CSphMatch * pMatch = m_pData; pMatch < m_pEnd-m_iSize; pMatch++ )
					m_dJustPopped.Add ( pMatch->m_tRowID );
			}

			for ( CSphMatch * pMatch = m_pData; pMatch < m_pEnd-m_iSize; pMatch++ )
				m_pSchema->FreeDataPtrs(pMatch);

			m_iUsed = m_iSize;
			m_pWorst = m_pEnd-m_iSize;
			m_bFinalized = true;
		}
		return true;
	}

	/// add grouped entry (must not happen)
	bool PushGrouped ( const CSphMatch &, bool ) override
	{
		assert ( 0 );
		return false;
	}

	/// finalize, perform final sort/cut as needed
	void Finalize ( ISphMatchProcessor & tProcessor, bool ) override
	{
		if ( !GetLength() )
			return;

		if ( !m_bFinalized )
		{
			MatchSort_fn<COMP> tComp ( m_tState );
			sphSort ( m_pEnd-m_iUsed, m_iUsed, tComp, tComp );
			for ( int i = m_iSize; i < m_iUsed; i++ )
			{
				CSphMatch * pMatch = m_pEnd-i-1;
				m_pSchema->FreeDataPtrs ( pMatch );
				pMatch->ResetDynamic();
			}

			m_iUsed = Min ( m_iUsed, m_iSize );
			m_bFinalized = true;
		}

		// reverse order iteration
		CSphMatch * pCur = m_pEnd - 1;
		const CSphMatch * pEnd = m_pEnd - m_iUsed;
		while ( pCur>=pEnd )
		{
			tProcessor.Process ( pCur-- );
		}
	}

	/// current result set length
	int GetLength () const override
	{
		return Min ( m_iUsed, m_iSize );
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) override
	{
		const CSphMatch * pBegin = pTo;

		// ensure we are sorted
		if ( m_iUsed )
		{
			MatchSort_fn<COMP> tComp ( m_tState );
			sphSort ( m_pEnd-m_iUsed, m_iUsed, tComp, tComp );
		}

		// reverse copy
		for ( int i=1; i<=Min ( m_iUsed, m_iSize ); i++ )
		{
			m_pSchema->FreeDataPtrs ( pTo );
			Swap ( *pTo, m_pEnd[-i] );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			pTo++;
		}

		// clean up for the next work session
		m_iTotal = 0;
		m_iUsed = 0;
		m_iSize = 0;
		m_bFinalized = false;

		return ( pTo-pBegin );
	}
};

//////////////////////////////////////////////////////////////////////////

/// collector for UPDATE statement
class CSphUpdateQueue : public CSphMatchQueueTraits
{
	CSphAttrUpdate		m_tWorkSet;
	CSphIndex *			m_pIndex;
	CSphString *		m_pError;
	CSphString *		m_pWarning;
	int *				m_pAffected;

private:
	void DoUpdate()
	{
		if ( !m_iUsed )
			return;

		m_tWorkSet.m_dRowOffset.Resize ( m_iUsed );
		m_tWorkSet.m_dDocids.Resize ( m_iUsed );

		ARRAY_FOREACH ( i, m_tWorkSet.m_dDocids )
		{
			m_tWorkSet.m_dRowOffset[i] = 0;
			m_tWorkSet.m_dDocids[i] = sphGetDocID ( m_pData[i].m_pStatic ? m_pData[i].m_pStatic : m_pData[i].m_pDynamic );
		}

		bool bCritical = false;
		*m_pAffected += m_pIndex->UpdateAttributes ( m_tWorkSet, -1, bCritical, *m_pError, *m_pWarning );
		assert ( !bCritical ); // fixme! handle this

		m_iUsed = 0;
	}
public:
	/// ctor
	CSphUpdateQueue ( int iSize, CSphAttrUpdateEx * pUpdate, bool bIgnoreNonexistent, bool bStrict )
		: CSphMatchQueueTraits ( iSize )
	{
		m_tWorkSet.m_dRowOffset.Reserve ( m_iSize );
		m_tWorkSet.m_dDocids.Reserve ( m_iSize );

		m_tWorkSet.m_bIgnoreNonexistent = bIgnoreNonexistent;
		m_tWorkSet.m_bStrict = bStrict;
		m_tWorkSet.m_dAttributes = pUpdate->m_pUpdate->m_dAttributes;
		m_tWorkSet.m_dPool = pUpdate->m_pUpdate->m_dPool;
		m_tWorkSet.m_dBlobs = pUpdate->m_pUpdate->m_dBlobs;
		m_pIndex = pUpdate->m_pIndex;
		m_pError = pUpdate->m_pError;
		m_pWarning = pUpdate->m_pWarning;
		m_pAffected = &pUpdate->m_iAffected;
	}

	/// stub
	bool IsGroupby () const final
	{
		return false;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) final
	{
		++m_iTotal;

		if ( m_iUsed==m_iSize )
			DoUpdate();

		// do add
		m_pSchema->CloneMatch ( &m_pData[m_iUsed++], tEntry );
		return true;
	}

	/// stub
	bool PushGrouped ( const CSphMatch &, bool ) final
	{
		assert ( 0 );
		return false;
	}

	/// final update pass
	int Flatten ( CSphMatch *, int ) final
	{
		assert ( m_iUsed>=0 );
		DoUpdate();
		m_iTotal = 0;
		return 0;
	}

	void Finalize ( ISphMatchProcessor & tProcessor, bool ) final
	{
		if ( !GetLength() )
			return;

		// just evaluate in heap order
		CSphMatch * pCur = m_pData;
		const CSphMatch * pEnd = m_pData + m_iUsed;
		while ( pCur<pEnd )
		{
			tProcessor.Process ( pCur++ );
		}
	}
};

//////////////////////////////////////////////////////////////////////////

/// collect list of matched DOCIDs
/// (mainly used to collect docs in `DELETE... WHERE` statement)
class CSphCollectQueue : public CSphMatchQueueTraits
{
public:
	/// ctor
	CSphCollectQueue ( int iSize, CSphVector<DocID_t> * pValues )
		: CSphMatchQueueTraits ( 1 )
		, m_pValues ( pValues )
	{
		m_pValues->Reserve ( iSize );
	}

	/// stub
	bool IsGroupby () const final
	{
		return false;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) final
	{
		++m_iTotal;
		m_pValues->Add ( sphGetDocID( tEntry.m_pStatic ) );
		return true;
	}

	/// stub
	bool PushGrouped ( const CSphMatch &, bool ) final
	{
		assert ( 0 );
		return false;
	}

	/// stub
	int Flatten ( CSphMatch *, int ) final
	{
		m_iTotal = 0;
		return 0;
	}

	// stub
	void Finalize ( ISphMatchProcessor &, bool ) final
	{}

private:
	CSphVector<DocID_t>* m_pValues;
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
	return s=="@groupby"
		|| s=="@distinct"
		|| s=="groupby()"
		|| IsSortJsonInternal(s);
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
	struct tm tSplit; \
	localtime_r ( &tStamp, &tSplit );

GROUPER_BEGIN ( CSphGrouperAttr )
	return uValue;
GROUPER_END


GROUPER_BEGIN_SPLIT ( CSphGrouperDay )
	return (tSplit.tm_year+1900)*10000 + (1+tSplit.tm_mon)*100 + tSplit.tm_mday;
GROUPER_END

GROUPER_BEGIN_SPLIT ( CSphGrouperWeek )
	int iPrevSunday = (1+tSplit.tm_yday) - tSplit.tm_wday; // prev Sunday day of year, base 1
	int iYear = tSplit.tm_year+1900;
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
	return (tSplit.tm_year+1900)*100 + (1+tSplit.tm_mon);
GROUPER_END

GROUPER_BEGIN_SPLIT ( CSphGrouperYear )
	return (tSplit.tm_year+1900);
GROUPER_END

#define GROUPER_BEGIN_SPLIT_UTC( _name ) \
    GROUPER_BEGIN(_name) \
    time_t tStamp = (time_t)uValue; \
    struct tm tSplit; \
    gmtime_r ( &tStamp, &tSplit );

GROUPER_BEGIN_SPLIT_UTC ( CSphGrouperDayUtc )
		return (tSplit.tm_year + 1900) * 10000 + (1 + tSplit.tm_mon) * 100 + tSplit.tm_mday;
GROUPER_END

GROUPER_BEGIN_SPLIT_UTC ( CSphGrouperWeekUtc )
		int iPrevSunday = (1 + tSplit.tm_yday) - tSplit.tm_wday; // prev Sunday day of year, base 1
		int iYear = tSplit.tm_year + 1900;
		if ( iPrevSunday<=0 ) // check if we crossed year boundary
		{
			// adjust day and year
			iPrevSunday += 365;
			iYear--;

			// adjust for leap years
			if ( iYear % 4==0 && (iYear % 100!=0 || iYear % 400==0) )
				iPrevSunday++;
		}
		return iYear * 1000 + iPrevSunday;
GROUPER_END

GROUPER_BEGIN_SPLIT_UTC ( CSphGrouperMonthUtc )
		return (tSplit.tm_year + 1900) * 100 + (1 + tSplit.tm_mon);
GROUPER_END

GROUPER_BEGIN_SPLIT_UTC ( CSphGrouperYearUtc )
		return (tSplit.tm_year + 1900);
GROUPER_END

bool bGroupingInUtc = false;

CSphGrouper * getDayGrouper ( const CSphAttrLocator &tLoc )
{
	return bGroupingInUtc
			? (CSphGrouper *) new CSphGrouperDayUtc ( tLoc )
			: (CSphGrouper *) new CSphGrouperDay ( tLoc );
}

CSphGrouper * getWeekGrouper ( const CSphAttrLocator &tLoc )
{
	return bGroupingInUtc
		   ? (CSphGrouper *) new CSphGrouperWeekUtc ( tLoc )
		   : (CSphGrouper *) new CSphGrouperWeek ( tLoc );
}

CSphGrouper * getMonthGrouper ( const CSphAttrLocator &tLoc )
{
	return bGroupingInUtc
		   ? (CSphGrouper *) new CSphGrouperMonthUtc ( tLoc )
		   : (CSphGrouper *) new CSphGrouperMonth ( tLoc );
}

CSphGrouper * getYearGrouper ( const CSphAttrLocator &tLoc )
{
	return bGroupingInUtc
		   ? (CSphGrouper *) new CSphGrouperYearUtc ( tLoc )
		   : (CSphGrouper *) new CSphGrouperYear ( tLoc );
}

template <class PRED>
class CSphGrouperString : public CSphGrouperAttr, public PRED
{
public:
	explicit CSphGrouperString ( const CSphAttrLocator & tLoc )
		: CSphGrouperAttr ( tLoc )
	{}

	ESphAttr GetResultType () const override
	{
		return SPH_ATTR_BIGINT;
	}

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const override
	{
		int iLen = 0;
		const BYTE * pStr = tMatch.FetchAttrData ( m_tLocator, m_pBlobPool, iLen );
		if ( !pStr || !iLen )
			return 0;

		return PRED::Hash ( pStr, iLen );
	}
};


class BinaryHash_fn
{
public:
	uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED ) const
	{
		assert ( pStr && iLen );
		return sphFNV64 ( pStr, iLen, uPrev );
	}
};


template < typename T >
inline static char * FormatInt ( char sBuf[32], T v )
{
	if_const ( sizeof(T)==4 && v==INT_MIN )
		return strncpy ( sBuf, "-2147483648", 32 );
	if_const ( sizeof(T)==8 && v==LLONG_MIN )
		return strncpy ( sBuf, "-9223372036854775808", 32 );

	bool s = ( v<0 );
	if ( s )
		v = -v;

	char * p = sBuf+31;
	*p = 0;
	do
	{
		*--p = '0' + char ( v % 10 );
		v /= 10;
	} while ( v );
	if ( s )
		*--p = '-';
	return p;
}


/// lookup JSON key, group by looked up value (used in CSphKBufferJsonGroupSorter)
class CSphGrouperJsonField : public CSphGrouper
{
public:
	CSphGrouperJsonField ( const CSphAttrLocator & tLoc, ISphExpr * pExpr )
		: m_tLocator ( tLoc )
		, m_pExpr ( pExpr )
	{
		SafeAddRef ( pExpr );
	}

	void SetBlobPool ( const BYTE * pBlobPool ) override
	{
		CSphGrouper::SetBlobPool ( pBlobPool );
		if ( m_pExpr )
			m_pExpr->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
	}

	void GetLocator ( CSphAttrLocator & tOut ) const override
	{
		tOut = m_tLocator;
	}

	ESphAttr GetResultType () const override
	{
		return SPH_ATTR_BIGINT;
	}

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const override
	{
		if ( !m_pExpr )
			return SphGroupKey_t();
		return m_pExpr->Int64Eval ( tMatch );
	}

	SphGroupKey_t KeyFromValue ( SphAttr_t ) const override { assert(0); return SphGroupKey_t(); }

protected:
	CSphAttrLocator				m_tLocator;
	CSphRefcountedPtr<ISphExpr>	m_pExpr;
	const BYTE *				m_pBlobPool {nullptr};
};


template <class PRED>
class CSphGrouperMulti : public CSphGrouper, public PRED
{
public:
	CSphGrouperMulti ( const CSphVector<CSphAttrLocator> & dLocators, const CSphVector<ESphAttr> & dAttrTypes, const CSphVector<ISphExpr *> & dJsonKeys )
		: m_dLocators ( dLocators )
		, m_dAttrTypes ( dAttrTypes )
		, m_dJsonKeys ( dJsonKeys )
	{
		assert ( m_dLocators.GetLength()>1 );
		assert ( m_dLocators.GetLength()==m_dAttrTypes.GetLength() && m_dLocators.GetLength()==dJsonKeys.GetLength() );
	}

	~CSphGrouperMulti () override
	{
		ARRAY_FOREACH ( i, m_dJsonKeys )
			SafeDelete ( m_dJsonKeys[i] );
	}

	SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const override
	{
		auto tKey = ( SphGroupKey_t ) SPH_FNV64_SEED;

		for ( int i=0; i<m_dLocators.GetLength(); i++ )
		{
			if ( m_dAttrTypes[i]==SPH_ATTR_STRING )
			{
				int iLen = 0;
				const BYTE * pStr = sphGetBlobAttr( tMatch, m_dLocators[i], m_pBlobPool, iLen );
				if ( !pStr || !iLen )
					continue;

				tKey = PRED::Hash ( pStr, iLen, tKey );

			} else if ( m_dAttrTypes[i]==SPH_ATTR_JSON )
			{
				int iLen = 0;
				const BYTE * pStr = sphGetBlobAttr( tMatch, m_dLocators[i], m_pBlobPool, iLen );
				if ( !pStr || !iLen )
					continue;

				uint64_t uPacked = m_dJsonKeys[i]->Int64Eval ( tMatch );

				ESphJsonType eType = sphJsonUnpackType ( uPacked );
				const BYTE * pValue = m_pBlobPool + sphJsonUnpackOffset ( uPacked );

				int i32Val;
				int64_t i64Val;
				double fVal;
				switch ( eType )
				{
				case JSON_STRING:
					iLen = sphJsonUnpackInt ( &pValue );
					tKey = ( SphGroupKey_t ) sphFNV64 ( pValue, iLen, tKey );
					break;
				case JSON_INT32:
					i32Val = sphJsonLoadInt ( &pValue );
					tKey = ( SphGroupKey_t ) sphFNV64 ( &i32Val, sizeof(i32Val), tKey );
					break;
				case JSON_INT64:
					i64Val = sphJsonLoadBigint ( &pValue );
					tKey = ( SphGroupKey_t ) sphFNV64 ( &i64Val, sizeof(i64Val), tKey );
					break;
				case JSON_DOUBLE:
					fVal = sphQW2D ( sphJsonLoadBigint ( &pValue ) );
					tKey = ( SphGroupKey_t ) sphFNV64 ( &fVal, sizeof(fVal), tKey );
					break;
				default:
					break;
				}

			} else
			{
				SphAttr_t tAttr = tMatch.GetAttr ( m_dLocators[i] );
				tKey = ( SphGroupKey_t ) sphFNV64 ( &tAttr, sizeof(SphAttr_t), tKey );
			}
		}

		return tKey;
	}

	void SetBlobPool ( const BYTE * pBlobPool ) override
	{
		CSphGrouper::SetBlobPool ( pBlobPool );
		ARRAY_FOREACH ( i, m_dJsonKeys )
		{
			if ( m_dJsonKeys[i] )
				m_dJsonKeys[i]->Command ( SPH_EXPR_SET_BLOB_POOL, (void*)pBlobPool );
		}
	}

	SphGroupKey_t KeyFromValue ( SphAttr_t ) const override { assert(0); return SphGroupKey_t(); }
	void GetLocator ( CSphAttrLocator & ) const override { assert(0); }
	ESphAttr GetResultType() const override { return SPH_ATTR_BIGINT; }

private:
	CSphVector<CSphAttrLocator>	m_dLocators;
	CSphVector<ESphAttr>		m_dAttrTypes;
	CSphVector<ISphExpr *>		m_dJsonKeys;
};

//////////////////////////////////////////////////////////////////////////

/// simple fixed-size hash
/// doesn't keep the order
template < typename T, typename KEY, typename HASHFUNC >
class CSphFixedHash : ISphNoncopyable
{
protected:
	static const int HASH_LIST_END = -1;

	struct HashEntry_t
	{
		KEY		m_tKey;
		T		m_tValue;
	};

	struct HashTraits_t
	{
		int m_iEntry;
		int m_iNext;
	};

protected:
	CSphVector<HashEntry_t>		m_dEntries;		///< key-value pairs storage pool
	CSphVector<HashTraits_t>	m_dHash;		///< hash into m_dEntries pool

	int							m_iFree = 0;	///< free pairs count
	int							m_iLength = 0;

public:
	/// ctor
	explicit CSphFixedHash ( int iLength )
	{
		int iBuckets = ( 1 << sphLog2 ( iLength-1 ) ); // less than 50% bucket usage guaranteed
		assert ( iLength>0 );
		assert ( iLength<=iBuckets );
		m_iLength = iLength;

		m_dEntries.Resize ( iLength );
		m_dHash.Resize ( iBuckets );

		Reset ();
	}

	/// cleanup
	void Reset ()
	{
		m_iFree = 0;
		int iBytes = sizeof(m_dHash[0]) * m_dHash.GetLength();
		memset ( m_dHash.Begin(), 0xff, iBytes );
	}

	/// add new entry
	/// returns nullptr on success
	/// returns pointer to value if already hashed, or replace it with new one, if insisted.
	T * Add ( const T & tValue, const KEY & tKey )
	{
		assert ( m_iFree<m_iLength && "hash overflow" );

		// check if it's already hashed
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );
		int iPrev = HASH_LIST_END;

		for ( int iEntry=m_dHash[uHash].m_iEntry; iEntry>=0; iPrev=iEntry, iEntry=m_dHash[iEntry].m_iNext )
		{
			if ( m_dEntries[iEntry].m_tKey==tKey )
				return &m_dEntries[iEntry].m_tValue;
		}

		int iNew = m_iFree++;
		HashEntry_t & tNew = m_dEntries[iNew];

		tNew.m_tKey = tKey;
		tNew.m_tValue = tValue;

		if ( iPrev>=0 )
		{
			assert ( m_dHash[iPrev].m_iNext==HASH_LIST_END );
			m_dHash[iPrev].m_iNext = iNew;
			m_dHash[iNew].m_iNext = HASH_LIST_END;
			m_dHash[iNew].m_iEntry = iNew;
		} else
		{
			assert ( m_dHash[uHash].m_iEntry==HASH_LIST_END );
			m_dHash[uHash].m_iEntry = iNew;
			m_dHash[uHash].m_iNext = HASH_LIST_END;
		}
		return nullptr;
	}

	/// get value pointer by key
	T * operator () ( const KEY & tKey ) const
	{
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );

		for ( int iEntry=m_dHash[uHash].m_iEntry; iEntry!=HASH_LIST_END; iEntry=m_dHash[iEntry].m_iNext )
			if ( m_dEntries[iEntry].m_tKey==tKey )
				return (T*)&m_dEntries[iEntry].m_tValue;

		return nullptr;
	}
};


/////////////////////////////////////////////////////////////////////////////

/// (group,attrvalue) pair
struct SphGroupedValue_t
{
public:
	SphGroupKey_t	m_uGroup;
	SphAttr_t		m_uValue;
	int				m_iCount;

public:
	SphGroupedValue_t () = default;

	SphGroupedValue_t ( SphGroupKey_t uGroup, SphAttr_t uValue, int iCount )
		: m_uGroup ( uGroup )
		, m_uValue ( uValue )
		, m_iCount ( iCount )
	{}

	inline bool operator < ( const SphGroupedValue_t & rhs ) const
	{
		if ( m_uGroup!=rhs.m_uGroup ) return m_uGroup<rhs.m_uGroup;
		if ( m_uValue!=rhs.m_uValue ) return m_uValue<rhs.m_uValue;
		return m_iCount>rhs.m_iCount;
	}

	inline bool operator == ( const SphGroupedValue_t & rhs ) const
	{
		return m_uGroup==rhs.m_uGroup && m_uValue==rhs.m_uValue;
	}
};

/// same as SphGroupedValue_t but without group
struct SphUngroupedValue_t
{
public:
	SphAttr_t		m_uValue;
	int				m_iCount;

public:
	SphUngroupedValue_t () = default;

	SphUngroupedValue_t ( SphAttr_t uValue, int iCount )
		: m_uValue ( uValue )
		, m_iCount ( iCount )
	{}

	inline bool operator < ( const SphUngroupedValue_t & rhs ) const
	{
		if ( m_uValue!=rhs.m_uValue ) return m_uValue<rhs.m_uValue;
		return m_iCount>rhs.m_iCount;
	}

	inline bool operator == ( const SphUngroupedValue_t & rhs ) const
	{
		return m_uValue==rhs.m_uValue;
	}
};


/// unique values counter
/// used for COUNT(DISTINCT xxx) GROUP BY yyy queries
class CSphUniqounter : public CSphVector<SphGroupedValue_t>
{
public:
#ifndef NDEBUG
					CSphUniqounter () { Reserve ( 16384 ); }
	void			Add ( const SphGroupedValue_t & tValue )	{ CSphVector<SphGroupedValue_t>::Add ( tValue ); m_bSorted = false; }
	void			Sort ()										{ CSphVector<SphGroupedValue_t>::Sort (); m_bSorted = true; }
#endif

public:
	int				CountStart ( SphGroupKey_t * pOutGroup );	///< starting counting distinct values, returns count and group key (0 if empty)
	int				CountNext ( SphGroupKey_t * pOutGroup );	///< continues counting distinct values, returns count and group key (0 if done)
	void			Compact ( SphGroupKey_t * pRemoveGroups, int iRemoveGroups );

protected:
	int				m_iCountPos = 0;

#ifndef NDEBUG
	bool			m_bSorted = true;
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
	if ( m_iCountPos>=m_iCount )
		return 0;

	SphGroupKey_t uGroup = m_pData[m_iCountPos].m_uGroup;
	SphAttr_t uValue = m_pData[m_iCountPos].m_uValue;
	int iCount = m_pData[m_iCountPos].m_iCount;
	*pOutGroup = uGroup;

	while ( m_iCountPos<m_iCount && m_pData[m_iCountPos].m_uGroup==uGroup )
	{
		if ( m_pData[m_iCountPos].m_uValue!=uValue )
			iCount += m_pData[m_iCountPos].m_iCount;
		uValue = m_pData[m_iCountPos].m_uValue;
		m_iCountPos++;
	}
	return iCount;
}


void CSphUniqounter::Compact ( SphGroupKey_t * pRemoveGroups, int iRemoveGroups )
{
	assert ( m_bSorted );
	if ( !m_iCount )
		return;

	sphSort ( pRemoveGroups, iRemoveGroups );

	SphGroupedValue_t * pSrc = m_pData;
	SphGroupedValue_t * pDst = m_pData;
	SphGroupedValue_t * pEnd = m_pData + m_iCount;

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

	assert ( pDst-m_pData<=m_iCount );
	m_iCount = pDst-m_pData;
}

/////////////////////////////////////////////////////////////////////////////

/// match comparator interface from group-by sorter point of view
struct ISphMatchComparator : public ISphRefcounted // non-mt refcounted is enough right now
{
	virtual bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState ) const = 0;
protected:
	virtual            ~ISphMatchComparator () = default;
};


/// additional group-by sorter settings
struct CSphGroupSorterSettings
{
	CSphAttrLocator		m_tLocGroupby;		///< locator for @groupby
	CSphAttrLocator		m_tLocCount;		///< locator for @count
	CSphAttrLocator		m_tLocDistinct;		///< locator for @distinct
	CSphAttrLocator		m_tDistinctLoc;		///< locator for attribute to compute count(distinct) for
	CSphAttrLocator		m_tLocGroupbyStr;	///< locator for @groupbystr

	ESphAttr			m_eDistinctAttr = SPH_ATTR_NONE;	///< type of attribute to compute count(distinct) for
	bool				m_bDistinct = false;///< whether we need distinct
	bool				m_bMVA = false;		///< whether we're grouping by MVA attribute
	bool				m_bMva64 = false;
	CSphRefcountedPtr<CSphGrouper>		m_pGrouper;///< group key calculator
	bool				m_bImplicit = false;///< for queries with aggregate functions but without group by clause
	const ISphFilter *	m_pAggrFilterTrait = nullptr; ///< aggregate filter that got owned by grouper
	bool				m_bJson = false;	///< whether we're grouping by Json attribute

	void FixupLocators ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
	{
		sphFixupLocator ( m_tLocGroupby, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocCount, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocDistinct, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tDistinctLoc, pOldSchema, pNewSchema );
		sphFixupLocator ( m_tLocGroupbyStr, pOldSchema, pNewSchema );
	}
};


/// aggregate function interface
class IAggrFunc
{
public:
	virtual			~IAggrFunc() = default;
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
	T				GetValue ( const CSphMatch * pRow );
	void			SetValue ( CSphMatch * pRow, T val );

protected:
	CSphAttrLocator	m_tLocator;
};

template<>
inline DWORD IAggrFuncTraits<DWORD>::GetValue ( const CSphMatch * pRow )
{
	return (DWORD)pRow->GetAttr ( m_tLocator );
}

template<>
inline void IAggrFuncTraits<DWORD>::SetValue ( CSphMatch * pRow, DWORD val )
{
	pRow->SetAttr ( m_tLocator, val );
}

template<>
inline int64_t IAggrFuncTraits<int64_t>::GetValue ( const CSphMatch * pRow )
{
	return pRow->GetAttr ( m_tLocator );
}

template<>
inline void IAggrFuncTraits<int64_t>::SetValue ( CSphMatch * pRow, int64_t val )
{
	pRow->SetAttr ( m_tLocator, val );
}

template<>
inline float IAggrFuncTraits<float>::GetValue ( const CSphMatch * pRow )
{
	return pRow->GetAttrFloat ( m_tLocator );
}

template<>
inline void IAggrFuncTraits<float>::SetValue ( CSphMatch * pRow, float val )
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

	void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool ) override
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

	void Ungroup ( CSphMatch * pDst ) override
	{
		this->SetValue ( pDst, T ( this->GetValue ( pDst ) * pDst->GetAttr ( m_tCountLoc ) ) );
	}

	void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool bGrouped ) override
	{
		if ( bGrouped )
			this->SetValue ( pDst, T ( this->GetValue ( pDst ) + this->GetValue ( pSrc ) * pSrc->GetAttr ( m_tCountLoc ) ) );
		else
			this->SetValue ( pDst, this->GetValue ( pDst ) + this->GetValue ( pSrc ) );
	}

	void Finalize ( CSphMatch * pDst ) override
	{
		auto uAttr = pDst->GetAttr ( m_tCountLoc );
		if ( uAttr )
			this->SetValue ( pDst, T ( this->GetValue ( pDst ) / uAttr ) );
	}
};


/// MAX() implementation
template < typename T >
class AggrMax_t : public IAggrFuncTraits<T>
{
public:
	explicit AggrMax_t ( const CSphAttrLocator & tLoc ) : IAggrFuncTraits<T> ( tLoc )
	{}

	void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool ) override
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

	void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool ) override
	{
		this->SetValue ( pDst, Min ( this->GetValue(pDst), this->GetValue(pSrc) ) );
	}
};


/// GROUP_CONCAT() implementation
class AggrConcat_t : public IAggrFunc
{
protected:
	CSphAttrLocator	m_tLoc;

public:
	explicit AggrConcat_t ( const CSphColumnInfo & tCol )
		: m_tLoc ( tCol.m_tLocator )
	{}

	void Ungroup ( CSphMatch * ) override {}
	void Finalize ( CSphMatch * ) override {}

	void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool ) override
	{
		auto sSrc = (const BYTE*)pSrc->GetAttr(m_tLoc);
		auto sDst = (const BYTE*)pDst->GetAttr(m_tLoc);
		auto pOldDst = const_cast<BYTE*> ( sDst );

		int iSrc = sphUnpackPtrAttr ( sSrc, &sSrc );
		int iDst = sphUnpackPtrAttr ( sDst, &sDst );

		assert ( !sDst || *sDst ); // ensure the string is either nullptr, or has data
		assert ( !sSrc || *sSrc );

		// empty source? kinda weird, but done!
		if ( !sSrc )
			return;

		// empty destination? just clone the source
		if ( !sDst )
		{
			if ( sSrc )
				pDst->SetAttr ( m_tLoc, (SphAttr_t)sphPackPtrAttr ( sSrc, iSrc ) );
			return;
		}

		// both source and destination present? append source to destination
		// note that it gotta be manual copying here, as SetSprintf (currently) comes with a 1K limit
		assert ( sDst && sSrc );
		CSphFixedVector<BYTE> dNew ( iSrc+iDst+1 ); // OPTIMIZE? careful pre-reserve and/or realloc would be even faster
		memcpy ( dNew.Begin(), sDst, iDst );
		dNew[iDst] = ',';
		memcpy ( dNew.Begin()+iDst+1, sSrc, iSrc );

		// clear dst to avoid leaks
		// SafeDeleteArray ( pOldDst );
		sphDeallocatePacked ( pOldDst );
		pDst->SetAttr ( m_tLoc, (SphAttr_t)sphPackPtrAttr ( dNew.Begin(), iSrc+iDst+1 ) );
	}
};


/// group sorting functor
template < typename COMPGROUP >
struct GroupSorter_fn : public CSphMatchComparatorState, public MatchSortAccessor_t
{
	bool IsLess ( const MEDIAN_TYPE a, const MEDIAN_TYPE b ) const
	{
		return COMPGROUP::IsLess ( *b, *a, *this );
	}
};


struct MatchCloner_t
{
private:
	CSphFixedVector<CSphRowitem>	m_dRowBuf { 0 };
	CSphVector<CSphAttrLocator>		m_dAttrsGrp; // locators for grouping attrs (@groupby, @count, etc.)
	CSphVector<CSphAttrLocator>		m_dAttrsPtr; // locators for group_concat attrs
	CSphVector<int>					m_dMyPtrRows; // rowids matching m_dAttrsPtr
	CSphVector<int>					m_dOtherPtrRows; // rest rowids NOT matching m_dAttrsPtr
	const CSphSchemaHelper *		m_pSchema = nullptr;
	bool							m_bPtrRowsCommited = false;

public:
	void SetSchema ( const ISphSchema * pSchema )
	{
		m_pSchema = ( CSphSchemaHelper * ) pSchema; /// lazy hack
		m_dRowBuf.Reset ( m_pSchema->GetDynamicSize() );
	}

	// clone src to dst, then apply attrs from pGroup according stored locators.
	void Combine ( CSphMatch * pDst, const CSphMatch * pSrc, const CSphMatch * pGroup )
	{
		assert ( m_pSchema && pDst && pSrc && pGroup );
		assert ( pDst!=pGroup );
		assert ( m_bPtrRowsCommited );

		// clone
		m_pSchema->CloneMatchSpecial ( pDst, *pSrc, m_dOtherPtrRows );

		for ( auto &AttrsRaw : m_dAttrsGrp )
			pDst->SetAttr ( AttrsRaw, pGroup->GetAttr ( AttrsRaw ) );

//		CSphSchemaHelper::FreeDataSpecial ( pDst, m_dMyPtrRows );
		CSphSchemaHelper::CopyPtrsSpecial ( pDst, pGroup->m_pDynamic, m_dMyPtrRows );
	}

	void Clone ( CSphMatch * pDst, const CSphMatch * pSrc )
	{
		assert ( m_pSchema && pDst && pSrc );
		assert ( m_bPtrRowsCommited );
		if ( !pDst->m_pDynamic ) // no old match has no data to copy, just a fresh but old match
		{
			m_pSchema->CloneMatch ( pDst, *pSrc );
			return;
		}

		memcpy ( m_dRowBuf.Begin(), pDst->m_pDynamic, m_dRowBuf.GetLengthBytes() );
		m_pSchema->CloneMatchSpecial ( pDst, *pSrc, m_dOtherPtrRows );

		for ( auto &AttrsRaw : m_dAttrsGrp )
			pDst->SetAttr ( AttrsRaw, sphGetRowAttr ( m_dRowBuf.Begin(), AttrsRaw ) );

		for ( auto &AttrsRaw : m_dAttrsPtr )
			pDst->SetAttr ( AttrsRaw, sphGetRowAttr ( m_dRowBuf.Begin (), AttrsRaw ) );
	}

	inline void AddRaw ( const CSphAttrLocator& tLoc )
	{
		m_dAttrsGrp.Add ( tLoc );
	}

	inline void AddPtr ( const CSphAttrLocator &tLoc )
	{
		m_dAttrsPtr.Add ( tLoc );
	}
	inline void ResetAttrs()
	{
		m_dAttrsGrp.Resize ( 0 );
		m_dAttrsPtr.Resize ( 0 );
	}

	inline void CommitPtrs ()
	{
		assert ( m_pSchema );
		static const int SIZE_OF_ROW = 8 * sizeof ( CSphRowitem );

		if ( m_bPtrRowsCommited )
			m_dMyPtrRows.Resize(0);

		for ( const CSphAttrLocator &tLoc : m_dAttrsPtr )
			m_dMyPtrRows.Add ( tLoc.m_iBitOffset / SIZE_OF_ROW );

		m_dOtherPtrRows = m_pSchema->SubsetPtrs ( m_dMyPtrRows );

#ifndef NDEBUG
		// sanitize check
		m_dMyPtrRows = m_pSchema->SubsetPtrs ( m_dOtherPtrRows );
		assert ( m_dMyPtrRows.GetLength ()==m_dAttrsPtr.GetLength () );
#endif
		m_bPtrRowsCommited = true;
	}
};





template <typename MVA, typename T>
static void AddGroupedMVA ( const BYTE * pMVA, int iLengthBytes, const SphGroupKey_t & tGroupKey, int iCount, T & tUniq )
{
	int nValues = iLengthBytes / sizeof(MVA);
	const MVA * pValues = (const MVA*)pMVA;
	for ( int i = 0; i < nValues; i++ )
	{
		tUniq.Add ( tGroupKey, sphUnalignedRead ( *pValues ), iCount );
		pValues++;
	}
}


template <typename T>
static void AddDistinctKeys ( const CSphMatch & tEntry, const SphGroupKey_t & tGroupKey, int iCount, T & tUniq, CSphAttrLocator & tDistinctLoc, ESphAttr eDistinctAttr, const BYTE * pBlobPool )
{
	switch ( eDistinctAttr )
	{
	case SPH_ATTR_STRING:
	case SPH_ATTR_STRINGPTR:
		{
			int iLen = 0;
			const BYTE * pStr = nullptr;

			if ( sphIsBlobAttr ( eDistinctAttr ) )
				pStr = sphGetBlobAttr ( tEntry, tDistinctLoc, pBlobPool, iLen );
			else
				iLen = sphUnpackPtrAttr ( (const BYTE *)tEntry.GetAttr ( tDistinctLoc ), &pStr );

			SphAttr_t tAttr = (SphAttr_t)sphFNV64 ( pStr, iLen );
			tUniq.Add ( tGroupKey, tAttr, iCount );
		}
		break;

	case SPH_ATTR_UINT32SET:
	case SPH_ATTR_INT64SET:
		{
			int iLen = 0;
			const BYTE * pMVA = nullptr;

			if ( sphIsBlobAttr ( eDistinctAttr ) )
				pMVA = sphGetBlobAttr ( tEntry, tDistinctLoc, pBlobPool, iLen );
			else
				iLen = sphUnpackPtrAttr ( (const BYTE *)tEntry.GetAttr ( tDistinctLoc ), &pMVA );

			if ( eDistinctAttr==SPH_ATTR_UINT32SET || eDistinctAttr==SPH_ATTR_UINT32SET_PTR )
				AddGroupedMVA<DWORD, T> ( pMVA, iLen, tGroupKey, iCount, tUniq );
			else
				AddGroupedMVA<int64_t, T> ( pMVA, iLen, tGroupKey, iCount, tUniq );
		}
		break;

	default:
		tUniq.Add ( tGroupKey, tEntry.GetAttr ( tDistinctLoc ), iCount );
		break;
	}
}


template <typename COMPGROUP>
static void FixupSorterLocators ( CSphGroupSorterSettings & tSettings, ISphSchema * pOldSchema, const ISphSchema * pNewSchema, GroupSorter_fn<COMPGROUP> * pGroupSorter,
	CSphVector<IAggrFunc *> & dAggregates, MatchCloner_t & tPregroup, bool bRemapCmp )
{
	// this is the case when we are moving to a new schema with non-pooled attrs
	if ( !pOldSchema )
		return;

	tSettings.FixupLocators ( pOldSchema, pNewSchema );
	if ( pGroupSorter )
		pGroupSorter->FixupLocators ( pOldSchema, pNewSchema, bRemapCmp );

	for ( auto i : dAggregates )
		SafeDelete(i);

	dAggregates.Resize(0);
	tPregroup.ResetAttrs ();
}

class BaseGroupSorter_c : protected CSphGroupSorterSettings
{
protected:
	MatchCloner_t			m_tPregroup;
	CSphVector<IAggrFunc *>	m_dAggregates;
	const ISphFilter*		m_pAggrFilter; ///< aggregate filter for matches on flatten

public:
	explicit BaseGroupSorter_c ( const CSphGroupSorterSettings &tSettings )
		: CSphGroupSorterSettings ( tSettings )
		, m_pAggrFilter ( tSettings.m_pAggrFilterTrait )
	{}

	virtual ~BaseGroupSorter_c()
	{
		for ( auto &pAggregate : m_dAggregates )
			SafeDelete ( pAggregate );
		SafeDelete ( m_pAggrFilter );
	}

	/// schema, aggregates setup
	template <bool DISTINCT>
	inline void SetupBaseGrouper ( ISphSchema * pSchema, const ESphSortKeyPart * pSortKeyPart = nullptr,
		const CSphAttrLocator * pAttrLocator=nullptr, CSphVector<IAggrFunc *>* pAvgs = nullptr )
	{
		m_tPregroup.SetSchema ( pSchema );
		m_tPregroup.AddRaw ( m_tLocGroupby );
		m_tPregroup.AddRaw ( m_tLocCount );
		if_const ( DISTINCT )
			m_tPregroup.AddRaw ( m_tLocDistinct );

		// extract aggregates
		for ( int i = 0; i<pSchema->GetAttrsCount (); i++ )
		{
			const CSphColumnInfo &tAttr = pSchema->GetAttr ( i );
			bool bMagicAggr = IsGroupbyMagic ( tAttr.m_sName )
				|| IsSortStringInternal ( tAttr.m_sName.cstr () ); // magic legacy aggregates

			if ( tAttr.m_eAggrFunc==SPH_AGGR_NONE || bMagicAggr )
				continue;

			switch ( tAttr.m_eAggrFunc )
			{
			case SPH_AGGR_SUM:
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_INTEGER: m_dAggregates.Add ( new AggrSum_t<DWORD> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_BIGINT: m_dAggregates.Add ( new AggrSum_t<int64_t> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_FLOAT: m_dAggregates.Add ( new AggrSum_t<float> ( tAttr.m_tLocator ) ); break;
				default: assert ( 0 && "internal error: unhandled aggregate type" );
					break;
				}
				break;

			case SPH_AGGR_AVG:
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_INTEGER: m_dAggregates.Add ( new AggrAvg_t<DWORD> ( tAttr.m_tLocator, m_tLocCount ) ); break;
				case SPH_ATTR_BIGINT: m_dAggregates.Add ( new AggrAvg_t<int64_t> ( tAttr.m_tLocator, m_tLocCount ) );break;
				case SPH_ATTR_FLOAT: m_dAggregates.Add ( new AggrAvg_t<float> ( tAttr.m_tLocator, m_tLocCount ) ); break;
				default: assert ( 0 && "internal error: unhandled aggregate type" );
					break;
				}
				// store avg to calculate these attributes prior to groups sort
				if ( pAvgs && pSortKeyPart && pAttrLocator )
					for ( int iState = 0; iState<CSphMatchComparatorState::MAX_ATTRS; ++iState )
					{
						auto eKeypart = pSortKeyPart[iState];
						const auto & tLoc = pAttrLocator[iState];
						if ( ( eKeypart==SPH_KEYPART_INT || eKeypart==SPH_KEYPART_FLOAT )
							&& tLoc.m_bDynamic==tAttr.m_tLocator.m_bDynamic
							&& tLoc.m_iBitOffset==tAttr.m_tLocator.m_iBitOffset
							&& tLoc.m_iBitCount==tAttr.m_tLocator.m_iBitCount )
						{
							pAvgs->Add ( m_dAggregates.Last () );
							break;
						}
					}
				break;

			case SPH_AGGR_MIN:
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_INTEGER: m_dAggregates.Add ( new AggrMin_t<DWORD> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_BIGINT: m_dAggregates.Add ( new AggrMin_t<int64_t> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_FLOAT: m_dAggregates.Add ( new AggrMin_t<float> ( tAttr.m_tLocator ) ); break;
				default: assert ( 0 && "internal error: unhandled aggregate type" );
					break;
				}
				break;

			case SPH_AGGR_MAX:
				switch ( tAttr.m_eAttrType )
				{
				case SPH_ATTR_INTEGER: m_dAggregates.Add ( new AggrMax_t<DWORD> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_BIGINT: m_dAggregates.Add ( new AggrMax_t<int64_t> ( tAttr.m_tLocator ) ); break;
				case SPH_ATTR_FLOAT: m_dAggregates.Add ( new AggrMax_t<float> ( tAttr.m_tLocator ) ); break;
				default: assert ( 0 && "internal error: unhandled aggregate type" );
					break;
				}
				break;

			case SPH_AGGR_CAT:
				m_dAggregates.Add ( new AggrConcat_t ( tAttr ) );
				m_tPregroup.AddPtr ( tAttr.m_tLocator );
				break;

			default: assert ( 0 && "internal error: unhandled aggregate function" );
				break;
			}

			if ( tAttr.m_eAggrFunc!=SPH_AGGR_CAT )
				m_tPregroup.AddRaw ( tAttr.m_tLocator );
		}
		m_tPregroup.CommitPtrs();
	}
};
class GroupedUniq_c : public ISphNoncopyable
{
public:
	GroupedUniq_c ( CSphUniqounter & tUniq )
		: m_tUniq ( tUniq )
	{}

	void Add ( const SphGroupKey_t & tGroupKey, SphAttr_t tAttr, int iCount )
	{
		m_tUniq.Add ( SphGroupedValue_t ( tGroupKey, tAttr, iCount ) );
	}

private:
	CSphUniqounter & m_tUniq;
};


class UngroupedUniq_c : public ISphNoncopyable
{
public:
	UngroupedUniq_c ( CSphVector<SphUngroupedValue_t> & dUniq )
		: m_dUniq ( dUniq )
	{}

	void Add ( const SphGroupKey_t & /*tKey*/, SphAttr_t tAttr, int iCount )
	{
		m_dUniq.Add ( SphUngroupedValue_t ( tAttr, iCount ) );
	}

private:
	CSphVector<SphUngroupedValue_t>	& m_dUniq;
};


/// match sorter with k-buffering and group-by
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferGroupSorter : public CSphMatchQueueTraits, protected BaseGroupSorter_c
{
protected:
	ESphGroupBy		m_eGroupBy;			///< group-by function
	CSphRefcountedPtr<CSphGrouper>	m_pGrouper;
	CSphFixedHash < CSphMatch *, SphGroupKey_t, IdentityHash_fn >	m_hGroup2Match;

protected:
	int				m_iLimit;		///< max matches to be retrieved
	int				m_iMaxUsed = 0;

	CSphUniqounter	m_tUniq;
	bool			m_bSortByDistinct = false;

	GroupSorter_fn<COMPGROUP>	m_tGroupSorter;
	const ISphMatchComparator *	m_pComp;

	CSphVector<IAggrFunc *>		m_dAvgs;
	const BYTE *				m_pBlobPool = nullptr;

	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

public:
	/// ctor
	CSphKBufferGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery,
			const CSphGroupSorterSettings & tSettings )
		: CSphMatchQueueTraits ( pQuery->m_iMaxMatches*GROUPBY_FACTOR )
		, BaseGroupSorter_c ( tSettings )
		, m_eGroupBy ( pQuery->m_eGroupFunc )
		, m_pGrouper ( tSettings.m_pGrouper )
		, m_hGroup2Match ( pQuery->m_iMaxMatches*GROUPBY_FACTOR )
		, m_iLimit ( pQuery->m_iMaxMatches )
		, m_pComp ( pComp )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( DISTINCT==false || tSettings.m_tDistinctLoc.m_iBitOffset>=0 );
		SafeAddRef ( pComp );
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );
	}

	/// schema setup
	void SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override
	{
		FixupSorterLocators ( *this, m_pSchema, pSchema, &m_tGroupSorter, m_dAggregates, m_tPregroup, bRemapCmp );
		ISphMatchSorter::SetSchema ( pSchema, bRemapCmp );
		m_dAvgs.Resize ( 0 );
		SetupBaseGrouper<DISTINCT> ( pSchema, m_tGroupSorter.m_eKeypart, m_tGroupSorter.m_tLocator, &m_dAvgs );
	}

	/// dtor
	~CSphKBufferGroupSorter () override
	{
		SafeRelease ( m_pComp );
	}

	/// check if this sorter does groupby
	bool IsGroupby () const override
	{
		return true;
	}

	/// set blob pool pointer (for string+groupby sorters)
	void SetBlobPool ( const BYTE * pBlobPool ) override
	{
		m_pBlobPool = pBlobPool;
		m_pGrouper->SetBlobPool ( pBlobPool );
	}

	const BYTE * GetBlobPool() const
	{
		return m_pBlobPool;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		SphGroupKey_t uGroupKey = m_pGrouper->KeyFromMatch ( tEntry );
		return PushEx ( tEntry, uGroupKey, false, false );
	}

	/// add grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool ) override
	{
		return PushEx ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), true, false );
	}

	/// add entry to the queue
	bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bGrouped, bool, SphAttr_t * pAttr=nullptr )
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
			m_dJustPopped.Resize ( 0 );
		}

		// if this group is already hashed, we only need to update the corresponding match
		CSphMatch ** ppMatch = m_hGroup2Match ( uGroupKey );
		if ( ppMatch )
		{
			CSphMatch * pMatch = (*ppMatch);
			assert ( pMatch );
			assert ( pMatch->GetAttr ( m_tLocGroupby )==uGroupKey );
			assert ( pMatch->m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

			if ( bGrouped )
			{
				// it's already grouped match
				// sum grouped matches count
				pMatch->SetAttr ( m_tLocCount, pMatch->GetAttr ( m_tLocCount ) + tEntry.GetAttr ( m_tLocCount ) ); // OPTIMIZE! AddAttr()?
			} else
			{
				// it's a simple match
				// increase grouped matches count
				pMatch->SetAttr ( m_tLocCount, 1 + pMatch->GetAttr ( m_tLocCount ) ); // OPTIMIZE! IncAttr()?
			}

			// update aggregates
			for ( auto * pAggregate : m_dAggregates )
				pAggregate->Update ( pMatch, &tEntry, bGrouped );

			// if new entry is more relevant, update from it
			if ( m_pComp->VirtualIsLess ( *pMatch, tEntry, m_tState ) )
			{
				if_const ( NOTIFICATIONS )
				{
					m_iJustPushed = tEntry.m_tRowID;
					m_dJustPopped.Add ( pMatch->m_tRowID );
				}

				// clone the low part of the match
				m_tPregroup.Clone ( pMatch, &tEntry );

				// update @groupbystr value, if available
				if ( pAttr && m_tLocGroupbyStr.m_bDynamic )
					pMatch->SetAttr ( m_tLocGroupbyStr, *pAttr );
			}
		}

		// submit actual distinct value in all cases
		if_const ( DISTINCT )
		{
			int iCount = 1;
			if ( bGrouped )
				iCount = (int)tEntry.GetAttr ( m_tLocDistinct );

			GroupedUniq_c tUniq ( m_tUniq );
			AddDistinctKeys ( tEntry, uGroupKey, iCount, tUniq, m_tDistinctLoc, m_eDistinctAttr, m_pBlobPool );
		}

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( ppMatch )
			return false;

		// if we're full, let's cut off some worst groups
		if ( m_iUsed==m_iSize )
			CutWorst ( m_iLimit * (int)(GROUPBY_FACTOR/2) );

		// do add
		assert ( m_iUsed<m_iSize );
		CSphMatch & tNew = m_pData [ m_iUsed++ ];
		m_pSchema->CloneMatch ( &tNew, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tNew.m_tRowID;

		if ( !bGrouped )
		{
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				tNew.SetAttr ( m_tLocDistinct, 0 );

			// set @groupbystr value if available
			if ( pAttr && m_tLocGroupbyStr.m_bDynamic )
				tNew.SetAttr ( m_tLocGroupbyStr, *pAttr );
		} else
		{
			for ( auto * pAggregate : m_dAggregates )
				pAggregate->Ungroup ( &tNew );
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
			for ( auto * pAvg : m_dAvgs )
			{
				if ( bGroup )
					pAvg->Finalize ( pMatch );
				else
					pAvg->Ungroup ( pMatch );
			}
			++pMatch;
		}
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) override
	{
		CountDistinct ();

		CalcAvg ( true );
		SortGroups ();

		CSphVector<IAggrFunc *> dAggrs;
		if ( m_dAggregates.GetLength()!=m_dAvgs.GetLength() )
		{
			dAggrs = m_dAggregates;
			for ( auto * pAvg : m_dAvgs )
				dAggrs.RemoveValue ( pAvg );
		}

		// FIXME!!! we should provide up-to max_matches to output buffer
		const CSphMatch * pBegin = pTo;
		int iLen = GetLength ();
		for ( int i=0; i<iLen; ++i )
		{
			CSphMatch & tMatch = m_pData[i];
			for ( auto * pAggr : dAggrs )
				pAggr->Finalize ( &tMatch );

			// HAVING filtering
			if ( m_pAggrFilter && !m_pAggrFilter->Eval ( tMatch ) )
				continue;

			m_pSchema->CloneMatch ( pTo, tMatch );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;

			pTo++;
		}

		m_iUsed = 0;
		m_iTotal = 0;
		m_iMaxUsed = 0;

		m_hGroup2Match.Reset ();
		if_const ( DISTINCT )
			m_tUniq.Resize ( 0 );

		return ( pTo-pBegin );
	}

	/// get entries count
	int GetLength () const override
	{
		return Min ( m_iUsed, m_iLimit );
	}

	/// set group comparator state
	void SetGroupState ( const CSphMatchComparatorState & tState ) override
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
		if_const ( DISTINCT && m_tDistinctLoc.m_iBitOffset>=0 )
			for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
				if ( m_tGroupSorter.m_tLocator[i].m_iBitOffset==m_tDistinctLoc.m_iBitOffset )
			{
				m_bSortByDistinct = true;
				break;
			}
	}

protected:
	/// count distinct values if necessary
	void CountDistinct ()
	{
		if_const ( DISTINCT )
		{
			m_tUniq.Sort ();
			SphGroupKey_t uGroup;
			for ( int iCount = m_tUniq.CountStart ( &uGroup ); iCount; iCount = m_tUniq.CountNext ( &uGroup ) )
			{
				CSphMatch ** ppMatch = m_hGroup2Match(uGroup);
				if ( ppMatch )
					(*ppMatch)->SetAttr ( m_tLocDistinct, iCount );
			}
		}
	}

	/// cut worst N groups off the buffer tail
	void CutWorst ( int iBound )
	{
		// sort groups
		if ( m_bSortByDistinct )
			CountDistinct ();

		CalcAvg ( true );
		SortGroups ();
		CalcAvg ( false );

		if_const ( NOTIFICATIONS )
		{
			for ( int i = iBound; i < m_iUsed; ++i )
				m_dJustPopped.Add ( m_pData[i].m_tRowID );
		}

		// cleanup unused distinct stuff
		if_const ( DISTINCT )
		{
			// build kill-list
			CSphVector<SphGroupKey_t> dRemove;
			dRemove.Resize ( m_iUsed-iBound );
			ARRAY_FOREACH ( i, dRemove )
				dRemove[i] = m_pData[iBound+i].GetAttr ( m_tLocGroupby );

			// sort and compact
			if ( !m_bSortByDistinct )
				m_tUniq.Sort ();
			m_tUniq.Compact ( &dRemove[0], m_iUsed-iBound );
		}

		// rehash
		m_hGroup2Match.Reset ();
		for ( int i=0; i<iBound; i++ )
			m_hGroup2Match.Add ( m_pData+i, m_pData[i].GetAttr ( m_tLocGroupby ) );

		for ( int i = iBound; i < m_iUsed; i++ )
			m_pSchema->FreeDataPtrs ( m_pData+i );

		m_iMaxUsed = Max ( m_iMaxUsed, m_iUsed );

		// cut groups
		m_iUsed = iBound;
	}

	/// sort groups buffer
	void SortGroups ()
	{
		sphSort ( m_pData, m_iUsed, m_tGroupSorter, m_tGroupSorter );
	}

	void Finalize ( ISphMatchProcessor & tProcessor, bool ) override
	{
		if ( !GetLength() )
			return;

		if ( m_iUsed>m_iLimit )
			CutWorst ( m_iLimit );

		for ( int i = m_iUsed; i < m_iMaxUsed; i++ )
			m_pData[i].ResetDynamic();
		m_iMaxUsed = m_iUsed;

		// just evaluate in heap order
		CSphMatch * pCur = m_pData;
		const CSphMatch * pEnd = m_pData + m_iUsed;
		while ( pCur<pEnd )
			tProcessor.Process ( pCur++ );
	}
};


/// match sorter with k-buffering and N-best group-by
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferNGroupSorter : public CSphMatchQueueTraits, protected BaseGroupSorter_c
{
protected:
	ESphGroupBy		m_eGroupBy;			///< group-by function
	CSphRefcountedPtr<CSphGrouper>	m_pGrouper;

	// can not use CSphFixedHash as it replaces stored value on add
	// that breaks group by list on Hash collisions
	OpenHash_T < CSphMatch *, DocID_t >	m_hGroup2Match;

protected:
	int				m_iLimit;		///< max matches to be retrieved
	int				m_iGLimit;		///< limit per one group
	CSphFixedVector<int>	m_dGroupByList {0};	///< chains of equal matches from groups
	CSphFixedVector<int>	m_dGroupsLen {0};	///< lengths of chains of equal matches from groups
	int				m_iFreeHeads = 0;		///< insertion point for head matches.
	CSphFreeList	m_dFreeTails;			///< where to put tails of the subgroups
	SphGroupKey_t	m_uLastGroupKey = -1;	///< helps to determine in pushEx whether the new subgroup started
#ifndef NDEBUG
	int				m_iruns = 0;		///< helpers for conditional breakpoints on debug
	int				m_ipushed = 0;
#endif
	CSphUniqounter	m_tUniq;
	bool			m_bSortByDistinct = false;

	GroupSorter_fn<COMPGROUP>	m_tGroupSorter;
	const ISphMatchComparator *	m_pComp;

	CSphVector<IAggrFunc *>		m_dAvgs;

	const BYTE *				m_pBlobPool = nullptr;

	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

protected:
	inline int GetFreePos ( bool bTailPos )
	{
		// if we're full, let's cut off some worst groups
		if ( m_iUsed==m_iSize )
		{
			CutWorst ( m_iLimit * (int)(GROUPBY_FACTOR/2) );
			// don't return true value for tail this case,
			// since the context might be changed by the CutWorst!
			if ( bTailPos )
				return -1;
		}

		// do add
		assert ( m_iUsed<m_iSize );
		++m_iUsed;
		if ( bTailPos )
		{
			int iRes = m_dFreeTails.Get() + m_iSize;
			assert ( iRes<CSphMatchQueueTraits::GetDataLength() );
			return iRes;
		} else
		{
			assert ( m_iFreeHeads<m_iSize );
			return m_iFreeHeads++;
		}
	}

public:
	/// ctor
	CSphKBufferNGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery,
			const CSphGroupSorterSettings & tSettings ) // FIXME! make k configurable
		: CSphMatchQueueTraits ( ( pQuery->m_iGroupbyLimit>1 ? 2 : 1 ) * pQuery->m_iMaxMatches * GROUPBY_FACTOR )
		, BaseGroupSorter_c ( tSettings )
		, m_eGroupBy ( pQuery->m_eGroupFunc )
		, m_pGrouper ( tSettings.m_pGrouper )
		, m_hGroup2Match ( pQuery->m_iMaxMatches*GROUPBY_FACTOR*2 )
		, m_iLimit ( pQuery->m_iMaxMatches )
		, m_iGLimit ( pQuery->m_iGroupbyLimit )
		, m_pComp ( pComp )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( DISTINCT==false || tSettings.m_tDistinctLoc.m_iBitOffset>=0 );
		assert ( m_iGLimit > 1 );
		SafeAddRef ( pComp );

		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );

		// trick! This case we allocated 2*m_iSize mem.
		// range [0..m_iSize) used for 1-st matches of each subgroup (i.e., for heads)
		// range [m_iSize..2*m_iSize) used for the tails.
		m_dGroupByList.Reset ( m_iSize );
		m_dGroupsLen.Reset ( m_iSize );
		m_iSize >>= 1;
		ARRAY_FOREACH ( i, m_dGroupByList )
		{
			m_dGroupByList[i] = -1;
			m_dGroupsLen[i] = 0;
		}
		m_dFreeTails.Reset ( m_iSize );
	}

	// only for n-group
	int GetDataLength () const override
	{
		return CSphMatchQueueTraits::GetDataLength()/2;
	}

	/// schema setup
	void SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override
	{
		FixupSorterLocators ( *this, m_pSchema, pSchema, &m_tGroupSorter, m_dAggregates, m_tPregroup, bRemapCmp );
		ISphMatchSorter::SetSchema ( pSchema, bRemapCmp );
		m_dAvgs.Resize ( 0 );
		SetupBaseGrouper<DISTINCT> ( pSchema, m_tGroupSorter.m_eKeypart, m_tGroupSorter.m_tLocator, &m_dAvgs );
	}

	/// dtor
	~CSphKBufferNGroupSorter () override
	{
		SafeRelease ( m_pComp );
	}

	/// check if this sorter does groupby
	bool IsGroupby () const override
	{
		return true;
	}

	/// set blob pool pointer (for string+groupby sorters)
	void SetBlobPool ( const BYTE * pBlobPool ) override
	{
		m_pBlobPool = pBlobPool;
		m_pGrouper->SetBlobPool ( pBlobPool );
	}

	const BYTE * GetBlobPool() const
	{
		return m_pBlobPool;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		SphGroupKey_t uGroupKey = m_pGrouper->KeyFromMatch ( tEntry );
		return PushEx ( tEntry, uGroupKey, false, false );
	}

	/// add grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		return PushEx ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), true, bNewSet );
	}

	void FreeMatchChain ( int iPos )
	{
		while ( iPos>=0 )
		{
			if_const ( NOTIFICATIONS )
				m_dJustPopped.Add ( m_pData[iPos].m_tRowID );

			int iLastPos = iPos;
			m_pSchema->FreeDataPtrs ( m_pData + iPos );
			m_pData[iPos].ResetDynamic();
			iPos = m_dGroupByList[iPos];
			m_dGroupByList[iLastPos] = -1;
			if ( iLastPos>=m_iSize )
				m_dFreeTails.Free ( iLastPos - m_iSize );
		}
	}

	// insert a match into subgroup, or discard it
	// returns:
	// 	NOPLACE if no place now (need to flush)
	// 	REPLACED if value was discarded or replaced other existing value
	// 	ADDED if value was added.
	enum class InsertRes_e
	{
		NOPLACE,
		REPLACED,
		ADDED
	};

	InsertRes_e InsertMatch ( int iPos, const CSphMatch & tEntry, const SphGroupKey_t uGroupKey)
	{
		const int iHead = iPos;
		int iPrev = 0;
		const bool bDoAdd = ( m_dGroupsLen[iHead]<m_iGLimit );
		while ( iPos>=0 )
		{
			CSphMatch * pMatch = m_pData+iPos;
			if ( m_pComp->VirtualIsLess ( *pMatch, tEntry, m_tState ) ) // the tEntry is better than current *pMatch
			{
				int iNew;
				if ( bDoAdd )
				{
					iNew = GetFreePos ( true ); // add to the tails (2-nd subrange)
					if ( iNew<0 )
						return InsertRes_e::NOPLACE;
				} else
				{
					iNew = iPos;
					int iPreLast = iPrev;
					while ( m_dGroupByList[iNew]>=0 )
					{
						iPreLast = iNew;
						iNew = m_dGroupByList[iNew];
					}

					m_pSchema->FreeDataPtrs ( m_pData + iNew );
					m_pData[iNew].ResetDynamic();

					if ( iPreLast>=0 )
						m_dGroupByList[iPreLast] = -1;

					if ( iPos==iNew ) // avoid cycle link to itself
						iPos = -1;
				}

				CSphMatch & tNew = m_pData [ iNew ];

				if_const ( NOTIFICATIONS )
				{
					m_iJustPushed = tEntry.m_tRowID;
					if ( tNew.m_tRowID!=INVALID_ROWID )
						m_dJustPopped.Add ( tNew.m_tRowID );
				}
				if ( bDoAdd )
					++m_dGroupsLen[iHead];

				if ( iPos==iHead ) // this is the first elem, need to copy groupby staff from it
				{
					// trick point! The first elem MUST live in the low half of the pool.
					// So, replacing the first is actually moving existing one to the last half,
					// then overwriting the one in the low half with the new value and link them
					m_tPregroup.Clone ( &tNew, pMatch );
					m_tPregroup.Clone ( pMatch, &tEntry );
					m_dGroupByList[iNew] = m_dGroupByList[iPos];
					m_dGroupByList[iPos] = iNew;
				} else // this is elem somewhere in the chain, just shift it.
				{
					m_tPregroup.Clone ( &tNew, &tEntry );
					m_dGroupByList[iPrev] = iNew;
					m_dGroupByList[iNew] = iPos;
				}
				// need to keep group by for whole chain
				tNew.SetAttr ( m_tLocGroupby, uGroupKey );
				break;
			}
			iPrev = iPos;
			iPos = m_dGroupByList[iPos];
		}

		if ( iPos<0 && bDoAdd ) // this item is less than everything, but still appropriate
		{
			int iNew = GetFreePos ( true ); // add to the tails (2-nd subrange)
			if ( iNew<0 )
				return InsertRes_e::NOPLACE;

			CSphMatch & tNew = m_pData [ iNew ];
			assert ( !tNew.m_pDynamic );
			m_tPregroup.Clone ( &tNew, &tEntry );
			m_dGroupByList[iPrev] = iNew;
			m_dGroupByList[iNew] = iPos;
			// need to keep group key for whole chain
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );

			if_const ( NOTIFICATIONS )
				m_iJustPushed = tEntry.m_tRowID;
			if ( bDoAdd )
				++m_dGroupsLen[iHead];
		}

		return bDoAdd ? InsertRes_e::ADDED : InsertRes_e::REPLACED;
	}

#ifndef NDEBUG
	void CheckIntegrity()
	{
		int iTotalLen = 0;
		for ( int i=0; i<m_iFreeHeads; ++i )
		{
			int iLen = 0;
			int iCur = i;
			while ( iCur>=0 )
			{
				int iNext = m_dGroupByList[iCur];
				assert ( iNext==-1 || m_pData[iCur].GetAttr ( m_tLocGroupby )==0 || m_pData[iNext].GetAttr ( m_tLocGroupby )==0
						|| m_pData[iCur].GetAttr ( m_tLocGroupby )==m_pData[iNext].GetAttr ( m_tLocGroupby ) );
				++iLen;
				iCur = iNext;
			}
			assert ( m_dGroupsLen[i]==iLen );
			iTotalLen += iLen;
		}
		assert ( iTotalLen==m_iUsed );
	}
#define CHECKINTEGRITY() CheckIntegrity()
#else
#define CHECKINTEGRITY()
#endif

	/// add entry to the queue
	virtual bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bGrouped, bool bNewSet )
	{
#ifndef NDEBUG
		++m_ipushed;
#endif
		CHECKINTEGRITY();
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
			m_dJustPopped.Resize(0);
		}

		// if this group is already hashed, we only need to update the corresponding match
		CSphMatch ** ppMatch = m_hGroup2Match.Find ( uGroupKey );
		if ( ppMatch )
		{
			CSphMatch * pMatch = (*ppMatch);
			assert ( pMatch );
			assert ( pMatch->GetAttr ( m_tLocGroupby )==uGroupKey );
			assert ( pMatch->m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

			if ( bGrouped )
			{
				// it's already grouped match
				// sum grouped matches count
				if ( bNewSet || uGroupKey!=m_uLastGroupKey )
				{
					pMatch->SetAttr ( m_tLocCount, pMatch->GetAttr ( m_tLocCount ) + tEntry.GetAttr ( m_tLocCount ) ); // OPTIMIZE! AddAttr()?
					m_uLastGroupKey = uGroupKey;
					bNewSet = true;
				}
			} else
			{
				// it's a simple match
				// increase grouped matches count
				pMatch->SetAttr ( m_tLocCount, 1 + pMatch->GetAttr ( m_tLocCount ) ); // OPTIMIZE! IncAttr()?
			}

			bNewSet |= !bGrouped;

			// update aggregates
			if ( bNewSet )
				for ( auto* pAggregate : m_dAggregates )
					pAggregate->Update ( pMatch, &tEntry, bGrouped );

			// if new entry is more relevant, update from it
			switch ( InsertMatch ( pMatch-m_pData, tEntry, uGroupKey ) )
			{
				case InsertRes_e::NOPLACE:
				{
					// need to keep all popped values
					CSphTightVector<RowID_t> dJustPopped;
					dJustPopped.SwapData ( m_dJustPopped );

					// was no insertion because cache cleaning. Recall myself
					PushEx ( tEntry, uGroupKey, bGrouped, bNewSet );

					// post Push work
					ARRAY_FOREACH ( i, dJustPopped )
						m_dJustPopped.Add ( dJustPopped[i] );
					CSphMatch ** ppDec = m_hGroup2Match.Find ( uGroupKey );
					assert ( ppDec );
					(*ppDec)->SetAttr ( m_tLocCount, (*ppDec)->GetAttr ( m_tLocCount )-1 );
					break;
				}
				case InsertRes_e::ADDED:
				{
					if ( bGrouped )
						return true;
					++m_iTotal;
				}
				default: break;
			}
		}

		// submit actual distinct value in all cases
		if_const ( DISTINCT )
		{
			int iCount = 1;
			if ( bGrouped )
				iCount = (int)tEntry.GetAttr ( m_tLocDistinct );

			GroupedUniq_c tUniq ( m_tUniq );
			AddDistinctKeys ( tEntry, uGroupKey, iCount, tUniq, m_tDistinctLoc, m_eDistinctAttr, m_pBlobPool );
		}
		CHECKINTEGRITY();
		// it's a dupe anyway, so we shouldn't update total matches count
		if ( ppMatch )
			return false;

		// do add
		int iNew = GetFreePos ( false );
		assert ( iNew>=0 && iNew<m_iSize );

		CSphMatch & tNew = m_pData [ iNew ];
		assert ( !tNew.m_pDynamic );
		m_pSchema->CloneMatch ( &tNew, tEntry );

		m_dGroupByList [ iNew ] = -1;
		m_dGroupsLen [ iNew ] = 1;

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tNew.m_tRowID;

		if ( !bGrouped )
		{
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				tNew.SetAttr ( m_tLocDistinct, 0 );
		} else
		{
			m_uLastGroupKey = uGroupKey;
			for ( auto * pAggregate : m_dAggregates )
				pAggregate->Ungroup ( &tNew );
		}

		Verify ( m_hGroup2Match.Add ( uGroupKey, &tNew ) );
		m_iTotal++;
		CHECKINTEGRITY();
		return true;
	}

	void CalcAvg ( bool bGroup )
	{
		if ( !m_dAvgs.GetLength() )
			return;

		int iHeadMatch;
		int iMatch = iHeadMatch = 0;
		for ( int i=0; i<m_iUsed; ++i )
		{
			CSphMatch * pMatch = m_pData + iMatch;
			for ( auto * pAvg : m_dAvgs )
			{
				if ( bGroup )
					pAvg->Finalize ( pMatch );
				else
					pAvg->Ungroup ( pMatch );
			}
			iMatch = m_dGroupByList [ iMatch ];
			if ( iMatch<0 )
				iMatch = ++iHeadMatch;
		}
	}

	// rebuild m_hGroup2Match to point to the subgroups (2-nd elem and further)
	// returns true if any such subgroup found
	void Hash2nd()
	{
		// let the hash points to the chains from 2-nd elem
		m_hGroup2Match.Clear();
		int iHeads = m_iUsed;
		for ( int i=0; i<iHeads; i++ )
		{
			if ( m_dGroupsLen[i]>1 )
			{
				int iCount = m_dGroupsLen[i];
				int iTail = m_dGroupByList[i];

				assert ( iTail>=0 );
				assert ( m_pData[iTail].GetAttr ( m_tLocGroupby )==0 ||
						m_pData[i].GetAttr ( m_tLocGroupby )==m_pData[iTail].GetAttr ( m_tLocGroupby ) );

				Verify ( m_hGroup2Match.Add ( m_pData[i].GetAttr ( m_tLocGroupby ), m_pData + iTail ) );
				iHeads -= iCount - 1;
				m_dGroupsLen[iTail] = iCount;
			}
		}
	}


	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) override
	{
		CountDistinct ();
		CalcAvg ( true );

		Hash2nd();

		SortGroups ();
		CSphVector<IAggrFunc *> dAggrs;
		if ( m_dAggregates.GetLength()!=m_dAvgs.GetLength() )
		{
			dAggrs = m_dAggregates;
			for ( auto * pAvg : m_dAvgs )
				dAggrs.RemoveValue ( pAvg );
		}

		const CSphMatch * pBegin = pTo;
		int iTotal = GetLength ();
		int iTopGroupMatch = 0;

		for ( int iEntry=0; iEntry<iTotal; ++iEntry )
		{
			CSphMatch * pMatch = m_pData + iTopGroupMatch;
			for ( auto * pAggr : dAggrs )
				pAggr->Finalize ( pMatch );
			++iTopGroupMatch;

			// check if match (and whole group) is filtered out; skip them if so.
			if ( m_pAggrFilter && !m_pAggrFilter->Eval ( *pMatch ) )
			{
				auto** ppMatch = m_hGroup2Match.Find ( pMatch->GetAttr ( m_tLocGroupby ));
				for ( int iNext = ( ppMatch ? *ppMatch - m_pData : -1 ); iNext>=0; iNext = m_dGroupByList[iNext] )
					++iEntry;
				continue;
			}

			// copy top group match
			m_pSchema->CloneMatch ( pTo, *pMatch );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			++pTo;

			// now look for the next match.
			// In this specific case (2-nd, just after the head)
			// we have to look it in the hash, not in the linked list!
			CSphMatch ** ppMatch = m_hGroup2Match.Find ( pMatch->GetAttr ( m_tLocGroupby ) );
			int iNext = ( ppMatch ? *ppMatch-m_pData : -1 );
			while ( iNext>=0 )
			{
				// copy rest group matches
				m_tPregroup.Combine ( pTo, m_pData+iNext, pMatch );
				if ( iTag>=0 )
					pTo->m_iTag = iTag;
				++pTo;
				++iEntry;
				iNext = m_dGroupByList[iNext];
			}
		}

		m_iFreeHeads = m_iUsed = 0;
		m_iTotal = 0;
		ARRAY_FOREACH ( i, m_dGroupByList )
		{
			m_dGroupByList[i] = -1;
			m_dGroupsLen[i] = 0;
		}

		m_hGroup2Match.Clear();
		if_const ( DISTINCT )
			m_tUniq.Resize ( 0 );

		return ( pTo-pBegin );
	}

	/// get entries count
	int GetLength () const override
	{
		return Min ( m_iUsed, m_iLimit );
	}

	/// set group comparator state
	void SetGroupState ( const CSphMatchComparatorState & tState ) override
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
		if_const ( DISTINCT && m_tDistinctLoc.m_iBitOffset>=0 )
			for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
				if ( m_tGroupSorter.m_tLocator[i].m_iBitOffset==m_tDistinctLoc.m_iBitOffset )
				{
					m_bSortByDistinct = true;
					break;
				}
	}

protected:
	/// count distinct values if necessary
	void CountDistinct ()
	{
		if_const ( DISTINCT )
		{
			m_tUniq.Sort ();
			SphGroupKey_t uGroup;
			for ( int iCount = m_tUniq.CountStart ( &uGroup ); iCount; iCount = m_tUniq.CountNext ( &uGroup ) )
			{
				CSphMatch ** ppMatch = m_hGroup2Match.Find ( uGroup );
				if ( ppMatch )
					(*ppMatch)->SetAttr ( m_tLocDistinct, iCount );
			}
		}
	}

	/// cut worst N groups off the buffer tail
	void CutWorst ( int iBound )
	{
#ifndef NDEBUG
		++m_iruns;
#endif
		CHECKINTEGRITY ();

		// sort groups
		if ( m_bSortByDistinct )
			CountDistinct ();

		Hash2nd();

		CalcAvg ( true );
		SortGroups ();
		CalcAvg ( false );

		int iHead = 0;
		for ( int iLimit=0; iLimit<iBound; iHead++ )
		{
			const CSphMatch * pMatch = m_pData + iHead;
			CSphMatch ** ppTailMatch = m_hGroup2Match.Find ( pMatch->GetAttr ( m_tLocGroupby ) );
			int iCount = 1;
			int iTail = -1;
			if ( ppTailMatch )
			{
				assert ( (*ppTailMatch)->GetAttr ( m_tLocGroupby )==0 ||
						pMatch->GetAttr ( m_tLocGroupby )==(*ppTailMatch)->GetAttr ( m_tLocGroupby ) );
				iTail = (*ppTailMatch) - m_pData;
				iCount = m_dGroupsLen[iTail];
				assert ( iCount>0 );
			}

			// whole chain fits limit
			if ( ( iLimit + iCount )<=iBound )
			{
				m_dGroupByList[iHead] = iTail;
				m_dGroupsLen[iHead] = iCount;
				iLimit += iCount;
				continue;
			}

			// only head match fits limit but not tail match(es)
			if ( ( iLimit + 1 )==iBound )
			{
				m_dGroupByList[iHead] = -1;
				m_dGroupsLen[iHead] = 1;

				FreeMatchChain ( iTail );
				iHead++;
				break;
			}

			// part of tail fits limit - it our last chain
			// fix-up chain to fits limit
			assert ( iBound-iLimit<=iCount );
			iCount = iBound - iLimit;
			m_dGroupByList[iHead] = iTail;
			m_dGroupsLen[iHead] = iCount;

			iCount--;
			int iPrev = iTail;
			while ( iCount>0 )
			{
				iPrev = iTail;
				iTail = m_dGroupByList[iTail];
				iCount--;
				assert ( !iCount || iTail>=0 );
			}
			m_dGroupByList[iPrev] = -1;

			iHead++;
			FreeMatchChain ( iTail );
			break;
		}

		if_const ( DISTINCT )
		{
			int iCount = m_iUsed - iHead;
			CSphFixedVector<SphGroupKey_t> dRemove ( iCount );
			for ( int i=0; i<iCount; i++ )
				dRemove[i] = m_pData [ i + iHead ].GetAttr ( m_tLocGroupby );

			if ( !m_bSortByDistinct )
				m_tUniq.Sort ();

			m_tUniq.Compact ( &dRemove[0], iCount );
		}

		// cleanup chains after limit
		for ( int i=iHead; i<m_iFreeHeads; ++i )
		{
			CSphMatch * pMatch = m_pData + i;
			CSphMatch ** ppTailMatch = m_hGroup2Match.Find ( pMatch->GetAttr ( m_tLocGroupby ) );
			if ( ppTailMatch )
			{
				assert ( ( *ppTailMatch )->GetAttr ( m_tLocGroupby )==0 || pMatch->GetAttr ( m_tLocGroupby )==( *ppTailMatch )->GetAttr ( m_tLocGroupby ) );
				int iTail = ( *ppTailMatch ) - m_pData;
				assert ( m_dGroupsLen[iTail]>0 );
				FreeMatchChain ( iTail );
			}

			if_const ( NOTIFICATIONS )
				m_dJustPopped.Add ( pMatch->m_tRowID );
			m_pSchema->FreeDataPtrs ( pMatch );
			pMatch->ResetDynamic();

			m_dGroupByList[i] = -1;
			m_dGroupsLen[i] = 0;
		}

		// cleanup chain lengths after hash2nd
		for ( int i=m_iSize; i<m_dGroupsLen.GetLength(); i++ )
			m_dGroupsLen[i] = 0;

		// rehash
		m_hGroup2Match.Clear();
		for ( int i=0; i<iHead; i++ )
			Verify ( m_hGroup2Match.Add ( m_pData[i].GetAttr ( m_tLocGroupby ), m_pData + i ) );

		// cut groups
		m_iUsed = iBound;
		m_iFreeHeads = iHead;
		CHECKINTEGRITY();
	}

	/// sort groups buffer
	void SortGroups ()
	{
		sphSort ( m_pData, m_iFreeHeads, m_tGroupSorter, m_tGroupSorter );
	}

	void Finalize ( ISphMatchProcessor & tProcessor, bool ) override
	{
		if ( !GetLength() )
			return;

		if ( m_iUsed>m_iLimit )
			CutWorst ( m_iLimit );

		CSphBitvec dProcessedMatches ( CSphMatchQueueTraits::GetDataLength() );


		for ( int iMatch=0; iMatch<m_iFreeHeads; iMatch++ )
		{
			// this is head match
			dProcessedMatches.BitSet ( iMatch );

			int iNext = m_dGroupByList[iMatch];
			int iCount = m_dGroupsLen[iMatch] - 1;
			while ( iCount>0 )
			{
				dProcessedMatches.BitSet ( iNext );
				iNext = m_dGroupByList[iNext];
				iCount--;
			}
		}

		for ( int i = 0; i < dProcessedMatches.GetBits(); i++ )
		{
			CSphMatch * pMatch = m_pData+i;

			if ( dProcessedMatches.BitGet(i) )
				tProcessor.Process ( pMatch );
			else
			{
				m_pSchema->FreeDataPtrs ( pMatch );
				pMatch->ResetDynamic();
			}
		}
	}
};


/// match sorter with k-buffering and group-by for MVAs
template < typename T, typename MVA >
class MVAGroupSorter_T : public T
{
public:
	/// ctor
	MVAGroupSorter_T ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: T ( pComp, pQuery, tSettings )
	{
		this->m_pGrouper->GetLocator ( m_tMvaLocator );
	}

	/// check if this sorter does groupby
	bool IsGroupby () const override
	{
		return true;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		if ( !T::GetBlobPool() )
			return false;

		int iLengthBytes = 0;
		const BYTE * pMva = sphGetBlobAttr ( tEntry, m_tMvaLocator, T::GetBlobPool(), iLengthBytes );
		int nValues = iLengthBytes / sizeof(MVA);
		const MVA * pValues = (const MVA*)pMva;

		bool bRes = false;
		for ( int i = 0; i<nValues; i++ )
		{
			SphGroupKey_t uGroupkey = this->m_pGrouper->KeyFromValue ( pValues[i] );
			bRes |= this->PushEx ( tEntry, uGroupkey, false, false );
		}

		return bRes;
	}

	/// add pre-grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		// re-group it based on the group key
		// (first 'this' is for icc; second 'this' is for gcc)
		return this->PushEx ( tEntry, tEntry.GetAttr ( this->m_tLocGroupby ), true, bNewSet );
	}

protected:
	CSphAttrLocator		m_tMvaLocator;
};

template < typename COMPGROUP, typename MVA, bool DISTINCT, bool NOTIFICATIONS >
struct MvaGroupSorter_c : public MVAGroupSorter_T < CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >, MVA >
{
	typedef MVAGroupSorter_T < CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >, MVA > BASE;
		/// ctor
	MvaGroupSorter_c ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: BASE ( pComp, pQuery, tSettings )
	{
	}
};


template < typename COMPGROUP, typename MVA, bool DISTINCT, bool NOTIFICATIONS >
struct MvaNGroupSorter_c : public MVAGroupSorter_T < CSphKBufferNGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >, MVA >
{
	typedef MVAGroupSorter_T < CSphKBufferNGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >, MVA > BASE;

	/// ctor
	MvaNGroupSorter_c ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: BASE ( pComp, pQuery, tSettings )
	{
	}
};


/// match sorter with k-buffering and group-by for JSON arrays
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferJsonGroupSorter : public CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >
{
public:
	/// ctor
	CSphKBufferJsonGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS > ( pComp, pQuery, tSettings )
	{}

	/// check if this sorter does groupby
	bool IsGroupby () const override
	{
		return true;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tMatch ) override
	{
		bool bRes = false;

		int iLen;
		char sBuf[32];

		SphGroupKey_t uGroupkey = this->m_pGrouper->KeyFromMatch ( tMatch );

		auto iValue = (int64_t)uGroupkey;
		CSphGrouper* pGrouper = this->m_pGrouper;
		const BYTE * pBlobPool = ((CSphGrouperJsonField*)pGrouper)->GetBlobPool();

		ESphJsonType eJson = sphJsonUnpackType ( iValue );
		const BYTE * pValue = pBlobPool + sphJsonUnpackOffset ( iValue );

		switch ( eJson )
		{
		case JSON_ROOT:
			{
				iLen = sphJsonNodeSize ( JSON_ROOT, pValue );
				bool bEmpty = iLen==5; // mask and JSON_EOF
				uGroupkey = bEmpty ? 0 : sphFNV64 ( pValue, iLen );
				return this->PushEx ( tMatch, uGroupkey, false, false, bEmpty ? nullptr : &iValue );
			}
		case JSON_STRING:
		case JSON_OBJECT:
		case JSON_MIXED_VECTOR:
			iLen = sphJsonUnpackInt ( &pValue );
			uGroupkey = ( iLen==1 && eJson!=JSON_STRING ) ? 0 : sphFNV64 ( pValue, iLen );
			return this->PushEx ( tMatch, uGroupkey, false, false, ( iLen==1 && eJson!=JSON_STRING ) ? 0: &iValue );

		case JSON_STRING_VECTOR:
			{
				sphJsonUnpackInt ( &pValue );
				iLen = sphJsonUnpackInt ( &pValue );
				for ( int i=0;i<iLen;i++ )
				{
					int64_t iNewValue = sphJsonPackTypeOffset ( JSON_STRING, pValue-pBlobPool );

					int iStrLen = sphJsonUnpackInt ( &pValue );
					uGroupkey = sphFNV64 ( pValue, iStrLen );
					bRes |= this->PushEx ( tMatch, uGroupkey, false, false, &iNewValue );
					pValue += iStrLen;
				}
				return bRes;
			}
		case JSON_INT32:
			uGroupkey = sphFNV64 ( (BYTE*)FormatInt ( sBuf, (int)sphGetDword(pValue) ) );
			break;
		case JSON_INT64:
			uGroupkey = sphFNV64 ( (BYTE*)FormatInt ( sBuf, (int)sphJsonLoadBigint ( &pValue ) ) );
			break;
		case JSON_DOUBLE:
			snprintf ( sBuf, sizeof(sBuf), "%f", sphQW2D ( sphJsonLoadBigint ( &pValue ) ) );
			uGroupkey = sphFNV64 ( (const BYTE*)sBuf );
			break;
		case JSON_INT32_VECTOR:
			{
				iLen = sphJsonUnpackInt ( &pValue );
				auto p = (int*)pValue;
				for ( int i=0;i<iLen;i++ )
				{
					int64_t iPacked = sphJsonPackTypeOffset ( JSON_INT32, (BYTE*)p-pBlobPool );
					uGroupkey = *p++;
					bRes |= this->PushEx ( tMatch, uGroupkey, false, false, &iPacked );
				}
				return bRes;
			}
			break;
		case JSON_INT64_VECTOR:
		case JSON_DOUBLE_VECTOR:
			{
				iLen = sphJsonUnpackInt ( &pValue );
				auto p = (int64_t*)pValue;
				ESphJsonType eType = eJson==JSON_INT64_VECTOR ? JSON_INT64 : JSON_DOUBLE;
				for ( int i=0;i<iLen;i++ )
				{
					int64_t iPacked = sphJsonPackTypeOffset ( eType, (BYTE*)p-pBlobPool );
					uGroupkey = *p++;
					bRes |= this->PushEx ( tMatch, uGroupkey, false, false, &iPacked );
				}
				return bRes;
			}
			break;
		default:
			uGroupkey = 0;
			iValue = 0;
			break;
		}

		bRes |= this->PushEx ( tMatch, uGroupkey, false, false, &iValue );
		return bRes;
	}

	/// add pre-grouped entry to the queue
	bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override
	{
		// re-group it based on the group key
		// (first 'this' is for icc; second 'this' is for gcc)
		return this->PushEx ( tEntry, tEntry.GetAttr ( this->m_tLocGroupby ), true, bNewSet );
	}
};


/// implicit group-by sorter
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphImplicitGroupSorter : public ISphMatchSorter, ISphNoncopyable, protected BaseGroupSorter_c
{
protected:
	CSphMatch		m_tData;
	bool			m_bDataInitialized = false;

	CSphVector<SphUngroupedValue_t>	m_dUniq;
	const BYTE *				m_pBlobPool {nullptr};

public:
	/// ctor
	CSphImplicitGroupSorter ( const ISphMatchComparator * DEBUGARG(pComp),
		const CSphQuery *, const CSphGroupSorterSettings & tSettings )
		: BaseGroupSorter_c ( tSettings )
	{
		assert ( DISTINCT==false || tSettings.m_tDistinctLoc.m_iBitOffset>=0 );
		assert ( !pComp );

		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);

		m_dUniq.Reserve ( 16384 );
		m_iMatchCapacity = 1;
	}

	/// schema setup
	void SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override
	{
		FixupSorterLocators<void> ( *this, m_pSchema, pSchema, nullptr, m_dAggregates, m_tPregroup, bRemapCmp );
		ISphMatchSorter::SetSchema ( pSchema, bRemapCmp );
		SetupBaseGrouper<DISTINCT> ( pSchema );
	}

	int GetDataLength () const override
	{
		return 1;
	}

	/// check if this sorter does groupby
	bool IsGroupby () const override
	{
		return true;
	}

	/// set blob pool pointer (for string+groupby sorters)
	void SetBlobPool ( const BYTE * pBlobPool ) override
	{
		m_pBlobPool = pBlobPool;
	}

	/// add entry to the queue
	bool Push ( const CSphMatch & tEntry ) override
	{
		return PushEx ( tEntry, false );
	}

	/// add grouped entry to the queue. bNewSet indicates the beginning of resultset returned by an agent.
	bool PushGrouped ( const CSphMatch & tEntry, bool ) override
	{
		return PushEx ( tEntry, true );
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag ) override
	{
		assert ( m_bDataInitialized );

		CountDistinct ();

		for ( auto * pAggregate : m_dAggregates )
			pAggregate->Finalize ( &m_tData );

		int iCopied = 0;
		if ( !m_pAggrFilter || m_pAggrFilter->Eval ( m_tData ) )
		{
			iCopied = 1;
			m_pSchema->CloneMatch ( pTo, m_tData );
			m_pSchema->FreeDataPtrs ( &m_tData );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
		}

		m_iTotal = 0;
		m_bDataInitialized = false;

		if_const ( DISTINCT )
			m_dUniq.Resize(0);

		return iCopied;
	}

	/// finalize, perform final sort/cut as needed
	void Finalize ( ISphMatchProcessor & tProcessor, bool ) override
	{
		if ( !GetLength() )
			return;

		tProcessor.Process ( &m_tData );
	}

	/// get entries count
	int GetLength () const override
	{
		return m_bDataInitialized ? 1 : 0;
	}

protected:
	/// add entry to the queue
	bool PushEx ( const CSphMatch & tEntry, bool bGrouped )
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = INVALID_ROWID;
			m_dJustPopped.Resize(0);
		}

		if ( m_bDataInitialized )
		{
			assert ( m_tData.m_pDynamic[-1]==tEntry.m_pDynamic[-1] );

			if ( bGrouped )
			{
				// it's already grouped match
				// sum grouped matches count
				m_tData.SetAttr ( m_tLocCount, m_tData.GetAttr ( m_tLocCount ) + tEntry.GetAttr ( m_tLocCount ) ); // OPTIMIZE! AddAttr()?
			} else
			{
				// it's a simple match
				// increase grouped matches count
				m_tData.SetAttr ( m_tLocCount, 1 + m_tData.GetAttr ( m_tLocCount ) ); // OPTIMIZE! IncAttr()?
			}

			// update aggregates
			for ( auto * pAggregate : m_dAggregates )
				pAggregate->Update ( &m_tData, &tEntry, bGrouped );

			// if new entry is more relevant, update from it
			if ( tEntry.m_tRowID<m_tData.m_tRowID )
			{
				if_const ( NOTIFICATIONS )
				{
					m_iJustPushed = tEntry.m_tRowID;
					m_dJustPopped.Add ( m_tData.m_tRowID );
				}

				m_tPregroup.Clone ( &m_tData, &tEntry );
			}
		}

		// submit actual distinct value in all cases
		if_const ( DISTINCT )
		{
			int iCount = 1;
			if ( bGrouped )
				iCount = (int)tEntry.GetAttr ( m_tLocDistinct );

			UngroupedUniq_c tUniq ( m_dUniq );
			AddDistinctKeys ( tEntry, 0, iCount, tUniq, m_tDistinctLoc, m_eDistinctAttr, m_pBlobPool );
		}

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( m_bDataInitialized )
			return false;

		// add first
		m_pSchema->CloneMatch ( &m_tData, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = m_tData.m_tRowID;

		if ( !bGrouped )
		{
			m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
			m_tData.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				m_tData.SetAttr ( m_tLocDistinct, 0 );
		} else
		{
			for ( auto * pAggregate : m_dAggregates )
				pAggregate->Ungroup ( &m_tData );
		}

		m_bDataInitialized = true;
		m_iTotal++;
		return true;
	}

	/// count distinct values if necessary
	void CountDistinct ()
	{
		if_const ( DISTINCT )
		{
			assert ( m_bDataInitialized );

			m_dUniq.Sort ();
			int iCount = 0;
			ARRAY_FOREACH ( i, m_dUniq )
			{
				if ( i>0 && m_dUniq[i-1]==m_dUniq[i] )
					continue;
				iCount += m_dUniq[i].m_iCount;
			}

			m_tData.SetAttr ( m_tLocDistinct, iCount );
		}
	}
};

//////////////////////////////////////////////////////////////////////////
// PLAIN SORTING FUNCTORS
//////////////////////////////////////////////////////////////////////////

static bool IsDocidLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState )
{
	DocID_t tDocidA, tDocidB;
	
	switch ( tState.m_eKeypart[1] )
	{
		case SPH_KEYPART_DOCID_S:
			tDocidA = sphGetDocID ( a.m_pStatic );
			tDocidB = sphGetDocID ( b.m_pStatic );
			break;

		case SPH_KEYPART_DOCID_D:
			tDocidA = sphGetDocID ( a.m_pDynamic );
			tDocidB = sphGetDocID ( b.m_pDynamic );
			break;

		default:
			return a.m_tRowID > b.m_tRowID;
	}

	if ( tDocidA!=tDocidB )
		return tDocidA > tDocidB;

	return a.m_tRowID > b.m_tRowID;
}

/// match sorter
struct MatchRelevanceLt_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & tState )
	{
		if ( a.m_iWeight!=b.m_iWeight )
			return a.m_iWeight < b.m_iWeight;

		return IsDocidLess ( a, b, tState );
	}
};


/// match sorter
struct MatchAttrLt_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
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

		return IsDocidLess ( a, b, t );
	}
};


/// match sorter
struct MatchAttrGt_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
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

		return IsDocidLess ( a, b, t );
	}
};


/// match sorter
struct MatchTimeSegments_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
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

		return IsDocidLess ( a, b, t );
	}

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
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		float aa = a.GetAttrFloat ( t.m_tLocator[0] ); // FIXME! OPTIMIZE!!! simplified (dword-granular) getter could be used here
		float bb = b.GetAttrFloat ( t.m_tLocator[0] );
		if ( aa!=bb )
			return aa<bb;

		return IsDocidLess ( a, b, t );
	}
};

/////////////////////////////////////////////////////////////////////////////

#define SPH_TEST_PAIR(_aa,_bb,_idx ) \
	if ( (_aa)!=(_bb) ) \
		return ( (t.m_uAttrDesc >> (_idx)) & 1 ) ^ ( (_aa) > (_bb) );


#define SPH_TEST_KEYPART(_idx) \
	switch ( t.m_eKeypart[_idx] ) \
	{ \
		case SPH_KEYPART_ROWID:		SPH_TEST_PAIR ( a.m_tRowID, b.m_tRowID, _idx ); break; \
		case SPH_KEYPART_DOCID_S:	return sphGetDocID(a.m_pStatic) > sphGetDocID(b.m_pStatic); break; \
		case SPH_KEYPART_DOCID_D:	return sphGetDocID(a.m_pDynamic) > sphGetDocID(b.m_pDynamic); break; \
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
		case SPH_KEYPART_STRINGPTR: \
		case SPH_KEYPART_STRING: \
		{ \
			int iCmp = t.CmpStrings ( a, b, _idx ); \
			if ( iCmp!=0 ) \
				return ( ( t.m_uAttrDesc >> (_idx) ) & 1 ) ^ ( iCmp>0 ); \
			break; \
		} \
	}


struct MatchGeneric1_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		return a.m_tRowID>b.m_tRowID;
	}
};


struct MatchGeneric2_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		return a.m_tRowID>b.m_tRowID;
	}
};


struct MatchGeneric3_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		return a.m_tRowID>b.m_tRowID;
	}
};


struct MatchGeneric4_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
	{
		return IsLess ( a, b, t );
	}

	static inline bool IsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t )
	{
		SPH_TEST_KEYPART(0);
		SPH_TEST_KEYPART(1);
		SPH_TEST_KEYPART(2);
		SPH_TEST_KEYPART(3);
		return a.m_tRowID>b.m_tRowID;
	}
};


struct MatchGeneric5_fn : public ISphMatchComparator
{
	bool VirtualIsLess ( const CSphMatch & a, const CSphMatch & b, const CSphMatchComparatorState & t ) const override
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
		return a.m_tRowID>b.m_tRowID;
	}
};

//////////////////////////////////////////////////////////////////////////
// SORT CLAUSE PARSER
//////////////////////////////////////////////////////////////////////////

static const int MAX_SORT_FIELDS = 5; // MUST be in sync with CSphMatchComparatorState::m_iAttr


class SortClauseTokenizer_t
{
protected:
	const char * m_pCur;
	const char * m_pMax;
	char * m_pBuf;

protected:
	char ToLower ( char c )
	{
		// 0..9, A..Z->a..z, _, a..z, @, .
		if ( ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || c=='_' || c=='@' || c=='.' || c=='[' || c==']' || c=='\'' || c=='\"' || c=='(' || c==')' || c=='*' )
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

		// make string lowercase but keep case of JSON.field
		bool bJson = false;
		for ( int i=0; i<=iLen; i++ )
		{
			char cSrc = sBuffer[i];
			char cDst = ToLower ( cSrc );
			bJson = ( cSrc=='.' || cSrc=='[' || ( bJson && cDst>0 ) ); // keep case of valid char sequence after '.' and '[' symbols
			m_pBuf[i] = bJson ? cSrc : cDst;
		}
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
			return nullptr;

		// memorize token start, and move pointer forward
		const char * sRes = m_pCur;
		while ( *m_pCur )
			m_pCur++;
		return sRes;
	}

	bool IsSparseCount ( const char * sTok )
	{
		const char * sSeq = "(*)";
		for ( ; sTok<m_pMax && *sSeq; sTok++ )
		{
			bool bGotSeq = ( *sSeq==*sTok );
			if ( bGotSeq )
				sSeq++;

			// stop checking on any non space char outside sequence or sequence end
			if ( ( !bGotSeq && !sphIsSpace ( *sTok ) && *sTok!='\0' ) || !*sSeq )
				break;
		}

		if ( !*sSeq && sTok+1<m_pMax && !sTok[1] )
		{
			// advance token iterator after composite count(*) token
			m_pCur = sTok+1;
			return true;
		} else
		{
			return false;
		}
	}
};


static inline ESphSortKeyPart Attr2Keypart ( ESphAttr eType )
{
	switch ( eType )
	{
		case SPH_ATTR_FLOAT:
			return SPH_KEYPART_FLOAT;

		case SPH_ATTR_STRING:
			return SPH_KEYPART_STRING;

		case SPH_ATTR_JSON:
		case SPH_ATTR_JSON_PTR:
		case SPH_ATTR_JSON_FIELD:
		case SPH_ATTR_JSON_FIELD_PTR:
		case SPH_ATTR_STRINGPTR:
			return SPH_KEYPART_STRINGPTR;

		default:
			return SPH_KEYPART_INT;
	}
}


static bool SetupSortByDocID ( const ISphSchema & tSchema, CSphMatchComparatorState & tState, const CSphQuery * pQuery, bool bInitialSearch )
{
	const CSphColumnInfo * pDocID = tSchema.GetAttr ( sphGetDocidName() );
	assert ( pDocID && pDocID->m_eAttrType==SPH_ATTR_BIGINT );

	if ( pQuery && pQuery->m_eRanker==SPH_RANK_NONE && tState.m_eKeypart[0]==SPH_KEYPART_WEIGHT && bInitialSearch )
	{
		// if it's the first local search and there's no ranker, use sort by rowid
		// if we're merging results, sort by weight/docid
		tState.m_eKeypart[0] = SPH_KEYPART_ROWID;
		tState.m_uAttrDesc = 0;
		return false;
	}
	else
	{
		if ( pDocID->m_tLocator.m_iBitOffset==0 )
			tState.m_eKeypart[1] = pDocID->m_tLocator.m_bDynamic ? SPH_KEYPART_DOCID_D : SPH_KEYPART_DOCID_S;	// fastpath, static or dynamic docid at offset 0
		else
			tState.m_eKeypart[1] = SPH_KEYPART_INT;	// generic path, fetch docid as a plain attribute
		
		tState.m_tLocator[1] = pDocID->m_tLocator;
		tState.m_dAttrs[1] = tSchema.GetAttrIndex ( sphGetDocidName() );
		return true;
	}
}


ESortClauseParseResult sphParseSortClause ( const CSphQuery * pQuery, const char * sClause,
		const ISphSchema & tSchema, ESphSortFunc & eFunc, CSphMatchComparatorState & tState,
		bool bComputeItems, CSphString & sError )
{
	for ( auto &dAttr : tState.m_dAttrs )
		dAttr = -1;

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
			|| !strcasecmp ( pTok, "@weight" )
			|| !strcasecmp ( pTok, "weight()" ) )
		{
			tState.m_eKeypart[iField] = SPH_KEYPART_WEIGHT;

		} else
		{
			if ( !strcasecmp ( pTok, "@group" ) )
				pTok = "@groupby";
			else if ( !strcasecmp ( pTok, "count(*)" ) )
				pTok = "@count";
			else if ( !strcasecmp ( pTok, "facet()" ) )
				pTok = "@groupby"; // facet() is essentially a @groupby alias
			else if ( strcasecmp ( pTok, "count" )>=0 && tTok.IsSparseCount ( pTok + sizeof ( "count" ) - 1 ) ) // epression count(*) with various spaces
				pTok = "@count";


			// try to lookup plain attr in sorter schema
			int iAttr = tSchema.GetAttrIndex ( pTok );

			// do not order by mva (the result is undefined)			
			if ( iAttr>=0 )
			{
				ESphAttr eAttrType = tSchema.GetAttr(iAttr).m_eAttrType;
				if ( eAttrType==SPH_ATTR_UINT32SET || eAttrType==SPH_ATTR_INT64SET || eAttrType==SPH_ATTR_UINT32SET_PTR || eAttrType==SPH_ATTR_INT64SET_PTR )
				{
					sError.SetSprintf ( "order by MVA is undefined" );
					return SORT_CLAUSE_ERROR;
				}
			}

			// try to lookup aliased count(*) and aliased groupby() in select items
			if ( iAttr<0 )
			{
				ARRAY_FOREACH ( i, pQuery->m_dItems )
				{
					const CSphQueryItem & tItem = pQuery->m_dItems[i];
					if ( !tItem.m_sAlias.cstr() || strcasecmp ( tItem.m_sAlias.cstr(), pTok ) )
						continue;
					if ( tItem.m_sExpr.Begins("@") )
						iAttr = tSchema.GetAttrIndex ( tItem.m_sExpr.cstr() );
					else if ( tItem.m_sExpr=="count(*)" )
						iAttr = tSchema.GetAttrIndex ( "@count" );
					else if ( tItem.m_sExpr=="groupby()" )
					{
						CSphString sGroupJson = SortJsonInternalSet ( pQuery->m_sGroupBy );
						iAttr = tSchema.GetAttrIndex ( sGroupJson.cstr() );
						// try numeric group by
						if ( iAttr<0 )
							iAttr = tSchema.GetAttrIndex ( "@groupby" );
					}
					break; // break in any case; because we did match the alias
				}
			}

			// try JSON attribute and use JSON attribute instead of JSON field
			bool bJsonAttr = false;

			if ( iAttr>=0 )
			{
				ESphAttr eAttrType = tSchema.GetAttr(iAttr).m_eAttrType;
				if ( eAttrType==SPH_ATTR_JSON_FIELD || eAttrType==SPH_ATTR_JSON_FIELD_PTR || eAttrType==SPH_ATTR_JSON || eAttrType==SPH_ATTR_JSON_PTR )
					bJsonAttr = true;
			}

			if ( iAttr<0 || bJsonAttr )
			{
				if ( iAttr>=0 )
				{
					// aliased SPH_ATTR_JSON_FIELD, reuse existing expression
					const CSphColumnInfo * pAttr = &tSchema.GetAttr(iAttr);
					if ( pAttr->m_pExpr )
						pAttr->m_pExpr->AddRef(); // SetupSortRemap uses refcounted pointer, but does not AddRef() itself, so help it
					tState.m_tSubExpr[iField] = pAttr->m_pExpr;
					tState.m_tSubKeys[iField] = JsonKey_t ( pTok, strlen ( pTok ) );

				} else
				{
					CSphString sJsonCol, sJsonKey;
					if ( sphJsonNameSplit ( pTok, &sJsonCol, &sJsonKey ) )
					{
						iAttr = tSchema.GetAttrIndex ( sJsonCol.cstr() );
						if ( iAttr>=0 )
						{
							ExprParseArgs_t tExprArgs;
							tState.m_tSubExpr[iField] = sphExprParse ( pTok, tSchema, sError, tExprArgs );
							tState.m_tSubKeys[iField] = JsonKey_t ( pTok, strlen ( pTok ) );
						}
					}
				}
			}

			// try json conversion functions (integer()/double()/bigint() in the order by clause)
			ESphAttr eAttrType = SPH_ATTR_NONE;
			if ( iAttr<0 )
			{
				ExprParseArgs_t tExprArgs;
				tExprArgs.m_pAttrType = &eAttrType;
				ISphExpr * pExpr = sphExprParse ( pTok, tSchema, sError, tExprArgs );
				if ( pExpr )
				{
					tState.m_tSubExpr[iField] = pExpr;
					tState.m_tSubKeys[iField] = JsonKey_t ( pTok, strlen(pTok) );
					tState.m_tSubKeys[iField].m_uMask = 0;
					tState.m_tSubType[iField] = eAttrType;
					iAttr = 0; // will be remapped in SetupSortRemap
				}
			}

			// try precalculated json fields received from agents (prefixed with @int_*)
			if ( iAttr<0 )
			{
				CSphString sName;
				sName.SetSprintf ( "%s%s", g_sIntAttrPrefix, pTok );
				iAttr = tSchema.GetAttrIndex ( sName.cstr() );
			}

			// epic fail
			if ( iAttr<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pTok );
				return SORT_CLAUSE_ERROR;
			}

			const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
			tState.m_eKeypart[iField] = Attr2Keypart ( eAttrType!=SPH_ATTR_NONE ? eAttrType : tCol.m_eAttrType );
			tState.m_tLocator[iField] = tCol.m_tLocator;
			tState.m_dAttrs[iField] = iAttr;
		}
	}

	if ( iField==0 )
	{
		sError.SetSprintf ( "no sort order defined" );
		return SORT_CLAUSE_ERROR;
	}

	if ( iField==1 )
	{
		if ( SetupSortByDocID ( tSchema, tState, pQuery, bComputeItems ) )
			iField++;
	}

	switch ( iField )
	{
		case 1:		eFunc = FUNC_GENERIC1; break;
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

#define CREATE_SORTER_4TH(SORTER,COMPGROUP,COMP,QUERY,SETTINGS,HAS_PACKEDFACTORS) \
{ \
	if ( SETTINGS.m_bDistinct ) \
	{ \
		if ( HAS_PACKEDFACTORS ) \
			return new SORTER<COMPGROUP,true,true>(COMP,QUERY,SETTINGS); \
		else \
			return new SORTER<COMPGROUP,true,false>(COMP,QUERY,SETTINGS); \
	} else \
	{ \
		if ( HAS_PACKEDFACTORS ) \
			return new SORTER<COMPGROUP,false,true>(COMP,QUERY,SETTINGS); \
		else \
			return new SORTER<COMPGROUP,false,false>(COMP,QUERY,SETTINGS); \
	} \
}

#define CREATE_SORTER_4TH_MVA(SORTER,COMPGROUP,COMP,QUERY,SETTINGS,HAS_PACKEDFACTORS) \
{ \
	BYTE uSelector = (tSettings.m_bDistinct?1:0) + (bHasPackedFactors?2:0) + (tSettings.m_bMva64?4:0); \
	switch ( uSelector ) \
	{ \
	case 0:	return new SORTER<COMPGROUP,DWORD,false,false> ( pComp, pQuery, tSettings ); \
	case 1:	return new SORTER<COMPGROUP,DWORD,true,false> ( pComp, pQuery, tSettings ); \
	case 2:	return new SORTER<COMPGROUP,DWORD,false,true> ( pComp, pQuery, tSettings ); \
	case 3:	return new SORTER<COMPGROUP,DWORD,true,true> ( pComp, pQuery, tSettings ); \
	case 4:	return new SORTER<COMPGROUP,int64_t,false,false> ( pComp, pQuery, tSettings ); \
	case 5:	return new SORTER<COMPGROUP,int64_t,true,false> ( pComp, pQuery, tSettings ); \
	case 6:	return new SORTER<COMPGROUP,int64_t,false,true> ( pComp, pQuery, tSettings ); \
	case 7:	return new SORTER<COMPGROUP,int64_t,true,true> ( pComp, pQuery, tSettings ); \
	default: assert(0); return nullptr; \
	} \
}


template < typename COMPGROUP >
static ISphMatchSorter * sphCreateSorter3rd ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	BYTE uSelector = (tSettings.m_bMVA?1:0)
		+(tSettings.m_bImplicit?2:0)
		+((pQuery->m_iGroupbyLimit>1)?4:0)
		+(tSettings.m_bJson?8:0);

	switch ( uSelector )
	{
	case 0:	CREATE_SORTER_4TH		( CSphKBufferGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 1: CREATE_SORTER_4TH_MVA	( MvaGroupSorter_c,			COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 2: CREATE_SORTER_4TH		( CSphImplicitGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 4:	CREATE_SORTER_4TH		( CSphKBufferNGroupSorter,	COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 5: CREATE_SORTER_4TH_MVA	( MvaNGroupSorter_c,		COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	case 8: CREATE_SORTER_4TH		( CSphKBufferJsonGroupSorter, COMPGROUP, pComp, pQuery, tSettings, bHasPackedFactors );
	default: assert(0); return nullptr;
	}
}


static ISphMatchSorter * sphCreateSorter2nd ( ESphSortFunc eGroupFunc, const ISphMatchComparator * pComp,
	const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	switch ( eGroupFunc )
	{
		case FUNC_GENERIC1:		return sphCreateSorter3rd<MatchGeneric1_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC2:		return sphCreateSorter3rd<MatchGeneric2_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC3:		return sphCreateSorter3rd<MatchGeneric3_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC4:		return sphCreateSorter3rd<MatchGeneric4_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC5:		return sphCreateSorter3rd<MatchGeneric5_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_EXPR:			return sphCreateSorter3rd<MatchExpr_fn>		( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		default:				return nullptr;
	}
}


static ISphMatchSorter * sphCreateSorter1st ( ESphSortFunc eMatchFunc, ESphSortFunc eGroupFunc,
	const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	CSphRefcountedPtr<ISphMatchComparator> pComp;
	if ( !tSettings.m_bImplicit )
		switch ( eMatchFunc )
		{
			case FUNC_REL_DESC:		pComp = new MatchRelevanceLt_fn(); break;
			case FUNC_ATTR_DESC:	pComp = new MatchAttrLt_fn(); break;
			case FUNC_ATTR_ASC:		pComp = new MatchAttrGt_fn(); break;
			case FUNC_TIMESEGS:		pComp = new MatchTimeSegments_fn(); break;
			case FUNC_GENERIC1:		pComp = new MatchGeneric1_fn(); break;
			case FUNC_GENERIC2:		pComp = new MatchGeneric2_fn(); break;
			case FUNC_GENERIC3:		pComp = new MatchGeneric3_fn(); break;
			case FUNC_GENERIC4:		pComp = new MatchGeneric4_fn(); break;
			case FUNC_GENERIC5:		pComp = new MatchGeneric5_fn(); break;
			case FUNC_EXPR:			pComp = new MatchExpr_fn(); break; // only for non-bitfields, obviously
		}

	return sphCreateSorter2nd ( eGroupFunc, pComp, pQuery, tSettings, bHasPackedFactors );
}

//////////////////////////////////////////////////////////////////////////
// GEODIST
//////////////////////////////////////////////////////////////////////////

struct ExprGeodist_t : public ISphExpr
{
public:
						ExprGeodist_t () = default;

	bool				Setup ( const CSphQuery * pQuery, const ISphSchema & tSchema, CSphString & sError );
	float				Eval ( const CSphMatch & tMatch ) const override;
	void				FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override;
	void				Command ( ESphExprCommand eCmd, void * pArg ) override;
	uint64_t			GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable ) override;

protected:
	CSphAttrLocator		m_tGeoLatLoc;
	CSphAttrLocator		m_tGeoLongLoc;
	float				m_fGeoAnchorLat;
	float				m_fGeoAnchorLong;
	int					m_iLat;
	int					m_iLon;
};


bool ExprGeodist_t::Setup ( const CSphQuery * pQuery, const ISphSchema & tSchema, CSphString & sError )
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
	double c = 2*asin ( Min ( 1.0, sqrt(a) ) );
	return (float)(R*c);
}


void ExprGeodist_t::FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema )
{
	sphFixupLocator ( m_tGeoLatLoc, pOldSchema, pNewSchema );
	sphFixupLocator ( m_tGeoLongLoc, pOldSchema, pNewSchema );
}


void ExprGeodist_t::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS )
	{
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iLat );
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iLon );
	}
}

uint64_t ExprGeodist_t::GetHash ( const ISphSchema & tSorterSchema, uint64_t uPrevHash, bool & bDisable )
{
	uint64_t uHash = sphCalcExprDepHash ( this, tSorterSchema, uPrevHash, bDisable );

	static const char * EXPR_TAG = "ExprGeodist_t";
	uHash = sphFNV64 ( EXPR_TAG, strlen(EXPR_TAG), uHash );
	uHash = sphFNV64 ( &m_fGeoAnchorLat, sizeof(m_fGeoAnchorLat), uHash );
	uHash = sphFNV64 ( &m_fGeoAnchorLong, sizeof(m_fGeoAnchorLong), uHash );

	return uHash;
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS (FACTORY AND FLATTENING)
//////////////////////////////////////////////////////////////////////////

static CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation );
static CSphGrouper * sphCreateGrouperMulti ( const CSphVector<CSphAttrLocator> & dLocators,
		const CSphVector<ESphAttr> & dAttrTypes, VecRefPtrs_t<ISphExpr *> dJsonKeys,
		ESphCollation eCollation );

class QueueCreator_c
{
	const SphQueueSettings_t& m_tSettings;
	const CSphQuery & m_tQuery;
	CSphString & m_sError;
	StrVec_t * m_pExtra = nullptr;

	bool m_bHasCount = false;
	bool m_bHasGroupByExpr = false;
	sph::StringSet m_hQueryAttrs;
	CSphScopedPtr<CSphRsetSchema> m_pSorterSchema;

	bool m_bGotGroupby;
	bool m_bRandomize;
	ESphSortFunc m_eMatchFunc = FUNC_REL_DESC;
	ESphSortFunc m_eGroupFunc = FUNC_REL_DESC;
	CSphMatchComparatorState m_tStateMatch;
	CSphMatchComparatorState m_tStateGroup;
	CSphGroupSorterSettings m_tGroupSorterSettings;
	CSphVector<int> m_dGroupColumns;
	bool m_bHeadWOGroup;
	bool m_bGotDistinct;


private:
	// helpers
	bool ParseQueryItem ( const CSphQueryItem & tItem );
	bool MaybeAddGeodistColumn ();
	bool MaybeAddExprColumn ();
	bool MaybeAddExpressionsFromSelectList ();
	bool MaybeAddGroupbyMagic ( bool bGotDistinct );
	bool CheckHavingConstraints () const;
	bool SetupGroupbySettings ( bool bHasImplicitGrouping );
	void AssignOrderByToPresortStage ( const int * pAttrs, int iAttrCount );
	void RemapStaticStringsAndJsons ( CSphMatchComparatorState & tState );
	bool SetupMatchesSortingFunc ();
	bool SetupGroupSortingFunc ( bool bGotDistinct );
	bool AddGroupbyStuff();
	bool SetGroupSorting ();
	void ExtraAddSortkeys ( const int * dAttrs );

	ISphMatchSorter * SpawnQueue ();
	ISphFilter * CreateAggrFilter () const;
	void SetupCollation ();
	bool Err ( const char * sFmt, ... ) const;
	ISphMatchSorter * Errz ( const char * sFmt, ... ) const;

public:
	QueueCreator_c ( const SphQueueSettings_t& tSettings, const CSphQuery & tQuery,
			CSphString& sError, StrVec_t * pExtra )
		: m_tSettings ( tSettings )
		, m_tQuery ( tQuery )
		, m_sError ( sError )
		, m_pExtra ( pExtra )
		, m_pSorterSchema { new CSphRsetSchema }
	{
		// short-cuts
		m_sError = "";
		*m_pSorterSchema.Ptr () = m_tSettings.m_tSchema;
	}

	bool SetupComputeQueue ();
	bool SetupGroupQueue ();
	bool SetupQueue ();

	CSphRsetSchema& SorterSchema() const { return *m_pSorterSchema.Ptr(); }
	bool HasJson () const { return m_tGroupSorterSettings.m_bJson; }
	/// creates proper queue for given query
	/// may return NULL on error; in this case, error message is placed in sError
	/// if the pUpdate is given, creates the updater's queue and perform the index update
	/// instead of searching
	ISphMatchSorter * CreateQueue ();

	bool m_bMulti = false;
	bool m_bCreate = true;
	bool m_bZonespanlist = false;
	DWORD m_uPackedFactorFlags = SPH_FACTOR_DISABLE;

};

bool QueueCreator_c::SetupGroupbySettings ( bool bHasImplicitGrouping )
{
	if ( m_tQuery.m_sGroupBy.IsEmpty() && !bHasImplicitGrouping )
		return true;

	if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_ATTRPAIR )
		return Err ( "SPH_GROUPBY_ATTRPAIR is not supported any more (just group on 'bigint' attribute)" );

	assert ( m_pSorterSchema );
	auto & tSchema = *m_pSorterSchema.Ptr();

	// setup distinct attr
	m_tGroupSorterSettings.m_tDistinctLoc.m_iBitOffset = -1;
	if ( !m_tQuery.m_sGroupDistinct.IsEmpty () )
	{
		int iDistinct = tSchema.GetAttrIndex ( m_tQuery.m_sGroupDistinct.cstr () );
		if ( iDistinct<0 )
			return Err ( "group-count-distinct attribute '%s' not found", m_tQuery.m_sGroupDistinct.cstr () );

		const auto & tDistinctAttr = tSchema.GetAttr ( iDistinct );
		m_tGroupSorterSettings.m_tDistinctLoc = tDistinctAttr.m_tLocator;
		m_tGroupSorterSettings.m_eDistinctAttr = tDistinctAttr.m_eAttrType;
	}


	CSphString sJsonColumn;
	CSphString sJsonKey;
	if ( m_tQuery.m_eGroupFunc==SPH_GROUPBY_MULTIPLE )
	{
		CSphVector<CSphAttrLocator> dLocators;
		CSphVector<ESphAttr> dAttrTypes;
		VecRefPtrs_t<ISphExpr *> dJsonKeys;

		StrVec_t dGroupBy;
		sph::Split ( m_tQuery.m_sGroupBy.cstr (), -1, ",", [&] ( const char * sToken, int iLen )
		{
			CSphString sGroupBy ( sToken, iLen );
			sGroupBy.Trim ();
			dGroupBy.Add ( std::move ( sGroupBy ));
		} );
		dGroupBy.Uniq();

		for ( auto& sGroupBy : dGroupBy )
		{
			CSphString sJsonExpr;
			if ( sphJsonNameSplit ( sGroupBy.cstr(), &sJsonColumn, &sJsonKey ) )
			{
				sJsonExpr = sGroupBy;
				sGroupBy = sJsonColumn;
			}

			const int iAttr = tSchema.GetAttrIndex ( sGroupBy.cstr() );
			if ( iAttr<0 )
				return Err( "group-by attribute '%s' not found", sGroupBy.cstr() );

			auto tAttr = tSchema.GetAttr ( iAttr );
			ESphAttr eType = tAttr.m_eAttrType;
			if ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET )
				return Err ( "MVA values can't be used in multiple group-by" );

			if ( eType==SPH_ATTR_JSON && sJsonExpr.IsEmpty() )
				return Err ( "JSON blob can't be used in multiple group-by" );

			dLocators.Add ( tAttr.m_tLocator );
			dAttrTypes.Add ( eType );
			m_dGroupColumns.Add ( iAttr );

			if ( !sJsonExpr.IsEmpty() )
			{
				ExprParseArgs_t tExprArgs;
				dJsonKeys.Add ( sphExprParse ( sJsonExpr.cstr(), tSchema, m_sError, tExprArgs ) );
			}
			else
				dJsonKeys.Add ( nullptr );
		}

		m_tGroupSorterSettings.m_pGrouper = sphCreateGrouperMulti ( dLocators, dAttrTypes,
				std::move(dJsonKeys), m_tQuery.m_eCollation );
		return true;
	}

	if ( sphJsonNameSplit ( m_tQuery.m_sGroupBy.cstr(), &sJsonColumn, &sJsonKey ) )
	{
		const int iAttr = tSchema.GetAttrIndex ( sJsonColumn.cstr() );
		if ( iAttr<0 )
			return Err ( "groupby: no such attribute '%s'", sJsonColumn.cstr ());

		if ( tSchema.GetAttr(iAttr).m_eAttrType!=SPH_ATTR_JSON
			&& tSchema.GetAttr(iAttr).m_eAttrType!=SPH_ATTR_JSON_PTR )
			return Err ( "groupby: attribute '%s' does not have subfields (must be sql_attr_json)",
					sJsonColumn.cstr() );

		if ( m_tQuery.m_eGroupFunc!=SPH_GROUPBY_ATTR )
			return Err ( "groupby: legacy groupby modes are not supported on JSON attributes" );

		m_dGroupColumns.Add ( iAttr );

		ExprParseArgs_t tExprArgs;
		tExprArgs.m_eCollation = m_tQuery.m_eCollation;

		CSphRefcountedPtr<ISphExpr> pExpr { sphExprParse ( m_tQuery.m_sGroupBy.cstr(),
				tSchema, m_sError, tExprArgs ) };
		m_tGroupSorterSettings.m_pGrouper = new CSphGrouperJsonField ( tSchema.GetAttr(iAttr).m_tLocator, pExpr );
		m_tGroupSorterSettings.m_bJson = true;
		return true;

	}

	if ( bHasImplicitGrouping )
	{
		m_tGroupSorterSettings.m_bImplicit = true;
		return true;
	}

	// setup groupby attr
	int iGroupBy = tSchema.GetAttrIndex ( m_tQuery.m_sGroupBy.cstr() );

	if ( iGroupBy<0 )
	{
		// try aliased groupby attr (facets)
		ARRAY_FOREACH ( i, m_tQuery.m_dItems )
			if ( m_tQuery.m_sGroupBy==m_tQuery.m_dItems[i].m_sExpr )
			{
				iGroupBy = tSchema.GetAttrIndex ( m_tQuery.m_dItems[i].m_sAlias.cstr() );
				break;

			} else if ( m_tQuery.m_sGroupBy==m_tQuery.m_dItems[i].m_sAlias )
			{
				iGroupBy = tSchema.GetAttrIndex ( m_tQuery.m_dItems[i].m_sExpr.cstr() );
				break;
			}
	}

	if ( iGroupBy<0 )
		return Err ( "group-by attribute '%s' not found", m_tQuery.m_sGroupBy.cstr() );

	ESphAttr eType = tSchema.GetAttr ( iGroupBy ).m_eAttrType;
	CSphAttrLocator tLoc = tSchema.GetAttr ( iGroupBy ).m_tLocator;
	switch (m_tQuery.m_eGroupFunc )
	{
		case SPH_GROUPBY_DAY:
			m_tGroupSorterSettings.m_pGrouper = getDayGrouper ( tLoc ); break;
		case SPH_GROUPBY_WEEK:
			m_tGroupSorterSettings.m_pGrouper = getWeekGrouper ( tLoc ); break;
		case SPH_GROUPBY_MONTH:
			m_tGroupSorterSettings.m_pGrouper = getMonthGrouper ( tLoc ); break;
		case SPH_GROUPBY_YEAR:
			m_tGroupSorterSettings.m_pGrouper = getYearGrouper ( tLoc ); break;
		case SPH_GROUPBY_ATTR:
			if ( eType==SPH_ATTR_JSON || eType==SPH_ATTR_JSON_FIELD )
			{
				ExprParseArgs_t tExprArgs;
				tExprArgs.m_eCollation = m_tQuery.m_eCollation;

				CSphRefcountedPtr<ISphExpr> pExpr { sphExprParse ( m_tQuery.m_sGroupBy.cstr(),
						tSchema, m_sError, tExprArgs ) };
				m_tGroupSorterSettings.m_pGrouper = new CSphGrouperJsonField ( tLoc, pExpr );
				m_tGroupSorterSettings.m_bJson = true;

			} else if ( eType==SPH_ATTR_STRING || eType==SPH_ATTR_STRINGPTR ) // percolate select list push matches with string_ptr
				m_tGroupSorterSettings.m_pGrouper = sphCreateGrouperString ( tLoc, m_tQuery.m_eCollation );
			else
				m_tGroupSorterSettings.m_pGrouper = new CSphGrouperAttr ( tLoc );
			break;
		default:
			return Err ( "invalid group-by mode (mode=%d)", m_tQuery.m_eGroupFunc );
	}

	m_tGroupSorterSettings.m_bMVA = ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET );
	m_tGroupSorterSettings.m_bMva64 = ( eType==SPH_ATTR_INT64SET );
	m_dGroupColumns.Add ( iGroupBy );
	return true;
}

// move expressions used in ORDER BY or WITHIN GROUP ORDER BY to presort phase
void QueueCreator_c::AssignOrderByToPresortStage ( const int * pAttrs, int iAttrCount )
{
	assert ( pAttrs );
	assert ( m_pSorterSchema );

	CSphVector<int> dCur;

	// add valid attributes to processing list
	for ( int i=0; i<iAttrCount; ++i )
		if ( pAttrs[i]>=0 )
			dCur.Add ( pAttrs[i] );

	// collect columns which affect current expressions
	for ( int i=0; i<dCur.GetLength(); ++i )
	{
		const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr ( dCur[i] );
		if ( tCol.m_eStage>SPH_EVAL_PRESORT && tCol.m_pExpr )
			tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
	}

	// get rid of dupes
	dCur.Uniq();

	// fix up of attributes stages
	for ( int iAttr : dCur )
	{
		if ( iAttr<0 )
			continue;

		auto & tCol = const_cast < CSphColumnInfo & > ( m_pSorterSchema->GetAttr ( iAttr ) );
		if ( tCol.m_eStage==SPH_EVAL_FINAL )
			tCol.m_eStage = SPH_EVAL_PRESORT;
	}
}


// expression that transform string pool base + offset -> ptr
class ExprSortStringAttrFixup_c : public ISphExpr
{
public:
	explicit ExprSortStringAttrFixup_c ( const CSphAttrLocator & tLocator )
		: m_tLocator ( tLocator )
	{}

	float Eval ( const CSphMatch & ) const override { assert ( 0 ); return 0.0f; }

	const BYTE * StringEvalPacked ( const CSphMatch & tMatch ) const override
	{
		// our blob strings are not null-terminated!
		// we can either store nulls in .SPB or add them here
		int iLengthBytes = 0;
		const BYTE * pData = sphGetBlobAttr ( tMatch, m_tLocator, m_pBlobPool, iLengthBytes );
		return sphPackPtrAttr ( pData, iLengthBytes );
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		sphFixupLocator ( m_tLocator, pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
			m_pBlobPool = (const BYTE*)pArg;
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & ) override
	{
		assert ( 0 && "remap expressions in filters" );
		return 0;
	}

public:
	const BYTE *		m_pBlobPool {nullptr};	///< string pool; base for offset of string attributes
	CSphAttrLocator		m_tLocator;				///< string attribute to fix
};


// expression that transform string pool base + offset -> ptr
class ExprSortJson2StringPtr_c : public ISphExpr
{
public:
	ExprSortJson2StringPtr_c ( const CSphAttrLocator & tLocator, ISphExpr * pExpr )
		: m_tJsonCol ( tLocator )
		, m_pExpr ( pExpr )
	{
		if ( pExpr ) // adopt the expression
			pExpr->AddRef();
	}

	bool IsDataPtrAttr () const final { return true; }

	float Eval ( const CSphMatch & ) const override { assert ( 0 ); return 0.0f; }

	int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const override
	{
		if ( !m_pBlobPool || !m_pExpr )
		{
			*ppStr = nullptr;
			return 0;
		}

		uint64_t uPacked = m_pExpr->Int64Eval ( tMatch );

		const BYTE * pVal = m_pBlobPool + sphJsonUnpackOffset ( uPacked );
		ESphJsonType eJson = sphJsonUnpackType ( uPacked );

		CSphString sVal;

		// FIXME!!! make string length configurable for STRING and STRING_VECTOR to compare and allocate only Min(String.Length, CMP_LENGTH)
		switch ( eJson )
		{
		case JSON_INT32:
			sVal.SetSprintf ( "%d", sphJsonLoadInt ( &pVal ) );
			break;
		case JSON_INT64:
			sVal.SetSprintf ( INT64_FMT, sphJsonLoadBigint ( &pVal ) );
			break;
		case JSON_DOUBLE:
			sVal.SetSprintf ( "%f", sphQW2D ( sphJsonLoadBigint ( &pVal ) ) );
			break;
		case JSON_STRING:
		{
			int iLen = sphJsonUnpackInt ( &pVal );
			sVal.SetBinary ( (const char *)pVal, iLen );
			break;
		}
		case JSON_STRING_VECTOR:
		{
			int iTotalLen = sphJsonUnpackInt ( &pVal );
			int iCount = sphJsonUnpackInt ( &pVal );

			CSphFixedVector<BYTE> dBuf ( iTotalLen + 4 ); // data and tail GAP
			BYTE * pDst = dBuf.Begin();

			// head element
			if ( iCount )
			{
				int iElemLen = sphJsonUnpackInt ( &pVal );
				memcpy ( pDst, pVal, iElemLen );
				pDst += iElemLen;
			}

			// tail elements separated by space
			for ( int i=1; i<iCount; i++ )
			{
				*pDst++ = ' ';
				int iElemLen = sphJsonUnpackInt ( &pVal );
				memcpy ( pDst, pVal, iElemLen );
				pDst += iElemLen;
			}

			int iStrLen = pDst-dBuf.Begin();
			// filling junk space
			while ( pDst<dBuf.Begin()+dBuf.GetLength() )
				*pDst++ = '\0';

			*ppStr = dBuf.LeakData();
			return iStrLen;
		}
		default:
			break;
		}

		int iStriLen = sVal.Length();
		*ppStr = (const BYTE *)sVal.Leak();
		return iStriLen;
	}

	void FixupLocator ( const ISphSchema * pOldSchema, const ISphSchema * pNewSchema ) override
	{
		sphFixupLocator ( m_tJsonCol, pOldSchema, pNewSchema );
		if ( m_pExpr )
			m_pExpr->FixupLocator ( pOldSchema, pNewSchema );
	}

	void Command ( ESphExprCommand eCmd, void * pArg ) override
	{
		if ( eCmd==SPH_EXPR_SET_BLOB_POOL )
		{
			m_pBlobPool = (const BYTE*)pArg;
			if ( m_pExpr )
				m_pExpr->Command ( eCmd, pArg );
		}
	}

	uint64_t GetHash ( const ISphSchema &, uint64_t, bool & ) override
	{
		assert ( 0 && "remap expression in filters" );
		return 0;
	}

private:
	const BYTE *		m_pBlobPool {nullptr};	///< string pool; base for offset of string attributes
	CSphAttrLocator		m_tJsonCol;				///< JSON attribute to fix
	CSphRefcountedPtr<ISphExpr>	m_pExpr;
};


bool IsSortStringInternal ( const CSphString & sColumnName )
{
	assert ( sColumnName.cstr ());
	return ( strncmp ( sColumnName.cstr (), g_sIntAttrPrefix, sizeof ( g_sIntAttrPrefix )-1 )==0 );
}

bool IsSortJsonInternal ( const CSphString& sColumnName  )
{
	assert ( sColumnName.cstr ());
	return ( strncmp ( sColumnName.cstr (), g_sIntJsonPrefix, sizeof ( g_sIntJsonPrefix )-1 )==0 );
}

CSphString SortJsonInternalSet ( const CSphString& sColumnName )
{
	CSphString sName;
	if ( !sColumnName.IsEmpty() )
		( StringBuilder_c () << g_sIntJsonPrefix << "_" << sColumnName ).MoveTo ( sName );
	return sName;
}


bool sphSortGetStringRemap ( const ISphSchema & tSorterSchema, const ISphSchema & tIndexSchema,	CSphVector<SphStringSorterRemap_t> & dAttrs )
{
	dAttrs.Resize ( 0 );
	for ( int i=0; i<tSorterSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tDst = tSorterSchema.GetAttr(i);
		// remap only static strings
		if ( !tDst.m_sName.Begins ( g_sIntAttrPrefix ) || tDst.m_eAttrType==SPH_ATTR_STRINGPTR )
			continue;

		const CSphColumnInfo * pSrcCol = tIndexSchema.GetAttr ( tDst.m_sName.cstr()+sizeof(g_sIntAttrPrefix)-1 );
		if ( !pSrcCol ) // skip internal attributes received from agents
			continue;

		SphStringSorterRemap_t & tRemap = dAttrs.Add();
		tRemap.m_tSrc = pSrcCol->m_tLocator;
		tRemap.m_tDst = tDst.m_tLocator;
	}
	return ( dAttrs.GetLength()>0 );
}

////////////////////
// BINARY COLLATION
////////////////////

static const char * EMPTY_STR = "";

inline static void UnpackStrings ( const BYTE * & pStr1, const BYTE * & pStr2, bool bDataPtr, int & iLen1, int & iLen2 )
{
	// strings that are stored in index don't need to be unpacked
	if ( bDataPtr )
	{
		iLen1 = sphUnpackPtrAttr ( pStr1, &pStr1 );
		iLen2 = sphUnpackPtrAttr ( pStr2, &pStr2 );
	}

	if ( !pStr1 ) pStr1 = (const BYTE*)EMPTY_STR;
	if ( !pStr2 ) pStr2 = (const BYTE*)EMPTY_STR;
}


int sphCollateBinary ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 )
{
	UnpackStrings ( pStr1, pStr2, bDataPtr, iLen1, iLen2 );

	int iRes = memcmp ( (const char *)pStr1, (const char *)pStr2, Min ( iLen1, iLen2 ) );
	return iRes ? iRes : ( iLen1-iLen2 );
}

///////////////////////////////
// LIBC_CI, LIBC_CS COLLATIONS
///////////////////////////////

/// libc_ci, wrapper for strcasecmp
int sphCollateLibcCI ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 )
{
	UnpackStrings ( pStr1, pStr2, bDataPtr, iLen1, iLen2 );

	int iRes = strncasecmp ( (const char *)pStr1, (const char *)pStr2, Min ( iLen1, iLen2 ) );
	return iRes ? iRes : ( iLen1-iLen2 );
}


/// libc_cs, wrapper for strcoll
int sphCollateLibcCS ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 )
{
	#define COLLATE_STACK_BUFFER 1024

	UnpackStrings ( pStr1, pStr2, bDataPtr, iLen1, iLen2 );
	
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
		char * pBuf1 = new char[iLen + 1];
		char * pBuf2 = new char[iLen + 1];

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
	for ( auto& dCollPlanes : g_dCollPlanes_UTF8CI )
		dCollPlanes = nullptr;

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
int sphCollateUtf8GeneralCI ( const BYTE * pStr1, const BYTE * pStr2, bool bDataPtr, int iLen1, int iLen2 )
{
	UnpackStrings ( pStr1, pStr2, bDataPtr, iLen1, iLen2 );

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

	return ( pStr1<pMax1 ) ? 1 : -1;
}


/////////////////////////////
// hashing functions
/////////////////////////////


class LibcCSHash_fn
{
	static const int LOCALE_SAFE_GAP = 16;

public:
	static uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
	{
		assert ( pStr && iLen );

		int iCompositeLen = iLen + 1 + (int)( 3.0f * iLen ) + LOCALE_SAFE_GAP;
		CSphFixedVector<BYTE> dBuf { iCompositeLen };

		memcpy ( dBuf.Begin(), pStr, iLen );
		dBuf[iLen] = '\0';

		BYTE * pDst = dBuf.Begin()+iLen+1;
		int iDstAvailable = dBuf.GetLength() - iLen - LOCALE_SAFE_GAP;

		int iDstLen = strxfrm ( (char *)pDst, (const char *) dBuf.Begin(), iDstAvailable );
		assert ( iDstLen<iDstAvailable+LOCALE_SAFE_GAP );

		uint64_t uAcc = sphFNV64 ( pDst, iDstLen, uPrev );

		return uAcc;
	}
};


class LibcCIHash_fn
{
public:
	static uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
	{
		assert ( pStr && iLen );

		uint64_t uAcc = uPrev;
		while ( iLen-- )
		{
			int iChar = tolower ( *pStr++ );
			uAcc = sphFNV64 ( &iChar, 4, uAcc );
		}

		return uAcc;
	}
};


class Utf8CIHash_fn
{
public:
	static uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED )
	{
		assert ( pStr && iLen );

		uint64_t uAcc = uPrev;
		while ( iLen-- )
		{
			const BYTE * pCur = pStr++;
			int iCode = sphUTF8Decode ( pCur );
			iCode = CollateUTF8CI ( iCode );
			uAcc = sphFNV64 ( &iCode, 4, uAcc );
		}

		return uAcc;
	}
};


CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI : return new CSphGrouperString<Utf8CIHash_fn> ( tLoc );
	case SPH_COLLATION_LIBC_CI : return new CSphGrouperString<LibcCIHash_fn> ( tLoc );
	case SPH_COLLATION_LIBC_CS : return new CSphGrouperString<LibcCSHash_fn> ( tLoc );
	default: return new CSphGrouperString<BinaryHash_fn> ( tLoc );
	}
}

CSphGrouper * sphCreateGrouperMulti ( const CSphVector<CSphAttrLocator> & dLocators,
		const CSphVector<ESphAttr> & dAttrTypes, VecRefPtrs_t<ISphExpr *> dJsonKeys,
		ESphCollation eCollation )
{
	switch ( eCollation )
	{
	case SPH_COLLATION_UTF8_GENERAL_CI :
		return new CSphGrouperMulti<Utf8CIHash_fn> ( dLocators, dAttrTypes, std::move ( dJsonKeys ) );
	case SPH_COLLATION_LIBC_CI :
		return new CSphGrouperMulti<LibcCIHash_fn> ( dLocators, dAttrTypes, std::move ( dJsonKeys ) );
	case SPH_COLLATION_LIBC_CS :
		return new CSphGrouperMulti<LibcCSHash_fn> ( dLocators, dAttrTypes, std::move ( dJsonKeys ) );
	default:
		return new CSphGrouperMulti<BinaryHash_fn> ( dLocators, dAttrTypes, std::move ( dJsonKeys ) );
	}
}


/////////////////////////
// SORTING QUEUE FACTORY
/////////////////////////

template < typename COMP >
static ISphMatchSorter * CreatePlainSorter ( bool bKbuffer, int iMaxMatches, bool bFactors )
{
	if ( bKbuffer )
	{
		if ( bFactors )
			return new CSphKbufferMatchQueue<COMP, true> ( iMaxMatches );
		return new CSphKbufferMatchQueue<COMP, false> ( iMaxMatches );
	}
	if ( bFactors )
		return new CSphMatchQueue<COMP, true> ( iMaxMatches );
	return new CSphMatchQueue<COMP, false> ( iMaxMatches );
}


static ISphMatchSorter * CreatePlainSorter ( ESphSortFunc eMatchFunc, bool bKbuffer, int iMaxMatches, bool bFactors )
{
	switch ( eMatchFunc )
	{
		case FUNC_REL_DESC:		return CreatePlainSorter<MatchRelevanceLt_fn>	( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_ATTR_DESC:	return CreatePlainSorter<MatchAttrLt_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_ATTR_ASC:		return CreatePlainSorter<MatchAttrGt_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_TIMESEGS:		return CreatePlainSorter<MatchTimeSegments_fn>	( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC1:		return CreatePlainSorter<MatchGeneric1_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC2:		return CreatePlainSorter<MatchGeneric2_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC3:		return CreatePlainSorter<MatchGeneric3_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC4:		return CreatePlainSorter<MatchGeneric4_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_GENERIC5:		return CreatePlainSorter<MatchGeneric5_fn>		( bKbuffer, iMaxMatches, bFactors ); break;
		case FUNC_EXPR:			return CreatePlainSorter<MatchExpr_fn>			( bKbuffer, iMaxMatches, bFactors ); break;
		default:				return nullptr;
	}
}


void QueueCreator_c::ExtraAddSortkeys ( const int * dAttrs )
{
	if ( m_pExtra )
		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; ++i )
			if ( dAttrs[i]>=0 )
				m_pExtra->Add ( m_pSorterSchema->GetAttr ( dAttrs[i] ).m_sName );
}

bool QueueCreator_c::Err ( const char * sFmt, ... ) const
{
	va_list ap;
	va_start ( ap, sFmt );
	m_sError.SetSprintfVa ( sFmt, ap );
	va_end ( ap );
	return false;
}

ISphMatchSorter * QueueCreator_c::Errz ( const char * sFmt, ... ) const
{
	va_list ap;
	va_start ( ap, sFmt );
	m_sError.SetSprintfVa ( sFmt, ap );
	va_end ( ap );
	return nullptr;
}

bool QueueCreator_c::ParseQueryItem ( const CSphQueryItem & tItem )
{
	assert ( m_pSorterSchema );
	const CSphString & sExpr = tItem.m_sExpr;
	bool bIsCount = IsCount(sExpr);
	m_bHasCount |= bIsCount;

	if ( sExpr=="*" )
	{
		for ( int i=0; i<m_tSettings.m_tSchema.GetAttrsCount(); i++ )
			m_hQueryAttrs.Add ( m_tSettings.m_tSchema.GetAttr(i).m_sName );
	}

	// for now, just always pass "plain" attrs from index to sorter; they will be filtered on searchd level
	int iAttrIdx = m_tSettings.m_tSchema.GetAttrIndex ( sExpr.cstr() );
	bool bPlainAttr = ( ( sExpr=="*" || ( iAttrIdx>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE ) ) &&
		( tItem.m_sAlias.IsEmpty() || tItem.m_sAlias==tItem.m_sExpr ) );
	if ( iAttrIdx>=0 )
	{
		ESphAttr eAttr = m_tSettings.m_tSchema.GetAttr ( iAttrIdx ).m_eAttrType;
		if ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_STRINGPTR
			|| eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET )
		{
			if ( tItem.m_eAggrFunc!=SPH_AGGR_NONE )
				return Err ( "can not aggregate non-scalar attribute '%s'",
						tItem.m_sExpr.cstr() );

			if ( !bPlainAttr && ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_STRINGPTR ) )
			{
				bPlainAttr = true;
				for ( const auto & i : m_tQuery.m_dItems )
					if ( sExpr==i.m_sAlias )
						bPlainAttr = false;
			}
		}
	}

	if ( bPlainAttr || IsGroupby ( sExpr ) || bIsCount )
	{
		if ( sExpr!="*" && !tItem.m_sAlias.IsEmpty() )
			m_hQueryAttrs.Add ( tItem.m_sAlias );
		m_bHasGroupByExpr = IsGroupby ( sExpr );
		return true;
	}

	// not an attribute? must be an expression, and must be aliased by query parser
	assert ( !tItem.m_sAlias.IsEmpty() );

	// tricky part
	// we might be fed with precomputed matches, but it's all or nothing
	// the incoming match either does not have anything computed, or it has everything
	int iSorterAttr = m_pSorterSchema->GetAttrIndex ( tItem.m_sAlias.cstr() );
	if ( iSorterAttr>=0 )
	{
		if ( m_hQueryAttrs[tItem.m_sAlias] )
			return Err ( "alias '%s' must be unique (conflicts with another alias)",
					tItem.m_sAlias.cstr() );
		m_pSorterSchema->RemoveStaticAttr ( iSorterAttr );
	}

	// a new and shiny expression, lets parse
	CSphColumnInfo tExprCol ( tItem.m_sAlias.cstr(), SPH_ATTR_NONE );
	DWORD uQueryPackedFactorFlags = SPH_FACTOR_DISABLE;
	bool bHasZonespanlist = false;

	ExprParseArgs_t tExprParseArgs;
	tExprParseArgs.m_pAttrType = &tExprCol.m_eAttrType;
	tExprParseArgs.m_pUsesWeight = &tExprCol.m_bWeight;
	tExprParseArgs.m_pProfiler = m_tSettings.m_pProfiler;
	tExprParseArgs.m_eCollation = m_tQuery.m_eCollation;
	tExprParseArgs.m_pHook = m_tSettings.m_pHook;
	tExprParseArgs.m_pZonespanlist = &bHasZonespanlist;
	tExprParseArgs.m_pPackedFactorsFlags = &uQueryPackedFactorFlags;
	tExprParseArgs.m_pEvalStage = &tExprCol.m_eStage;
	tExprParseArgs.m_pStoredField = &tExprCol.m_uFieldFlags;

	// tricky bit
	// GROUP_CONCAT() adds an implicit TO_STRING() conversion on top of its argument
	// and then the aggregate operation simply concatenates strings as matches arrive
	// ideally, we would instead pass ownership of the expression to G_C() implementation
	// and also the original expression type, and let the string conversion happen in G_C() itself
	// but that ideal route seems somewhat more complicated in the current architecture
	if ( tItem.m_eAggrFunc==SPH_AGGR_CAT )
	{
		CSphString sExpr2;
		sExpr2.SetSprintf ( "TO_STRING(%s)", sExpr.cstr() );
		tExprCol.m_pExpr = sphExprParse ( sExpr2.cstr(), *m_pSorterSchema.Ptr (), m_sError, tExprParseArgs );
	} else
	{
		tExprCol.m_pExpr = sphExprParse ( sExpr.cstr(), *m_pSorterSchema.Ptr (), m_sError, tExprParseArgs );
	}

	m_uPackedFactorFlags |= uQueryPackedFactorFlags;
	m_bZonespanlist |= bHasZonespanlist;
	tExprCol.m_eAggrFunc = tItem.m_eAggrFunc;
	if ( !tExprCol.m_pExpr )
		return Err ( "parse error: %s", m_sError.cstr() );

	// force AVG() to be computed in floats
	if ( tExprCol.m_eAggrFunc==SPH_AGGR_AVG )
	{
		tExprCol.m_eAttrType = SPH_ATTR_FLOAT;
		tExprCol.m_tLocator.m_iBitCount = 32;
	}

	// force explicit type conversion for JSON attributes
	if ( tExprCol.m_eAggrFunc!=SPH_AGGR_NONE && tExprCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
		return Err ( "ambiguous attribute type '%s', use INTEGER(), BIGINT() or "
			   "DOUBLE() conversion functions", tItem.m_sExpr.cstr() );

	if ( uQueryPackedFactorFlags & SPH_FACTOR_JSON_OUT )
		tExprCol.m_eAttrType = SPH_ATTR_FACTORS_JSON;

	// force GROUP_CONCAT() to be computed as strings
	if ( tExprCol.m_eAggrFunc==SPH_AGGR_CAT )
	{
		tExprCol.m_eAttrType = SPH_ATTR_STRINGPTR;
		tExprCol.m_tLocator.m_iBitCount = ROWITEMPTR_BITS;
	}

	// postpone aggregates, add non-aggregates
	if ( tExprCol.m_eAggrFunc==SPH_AGGR_NONE )
	{
		// is this expression used in filter?
		// OPTIMIZE? hash filters and do hash lookups?
		if ( tExprCol.m_eAttrType!=SPH_ATTR_JSON_FIELD )
			ARRAY_FOREACH ( i, m_tQuery.m_dFilters )
			if ( m_tQuery.m_dFilters[i].m_sAttrName==tExprCol.m_sName )
			{
				// is this a hack?
				// m_bWeight is computed after EarlyReject() get called
				// that means we can't evaluate expressions with WEIGHT() in prefilter phase
				if ( tExprCol.m_bWeight )
				{
					tExprCol.m_eStage = SPH_EVAL_PRESORT; // special, weight filter ( short cut )
					break;
				}

				// so we are about to add a filter condition
				// but it might depend on some preceding columns (e.g. SELECT 1+attr f1 ... WHERE f1>5)
				// lets detect those and move them to prefilter \ presort phase too
				CSphVector<int> dCur;
				tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );

				// usual filter
				tExprCol.m_eStage = SPH_EVAL_PREFILTER;
				ARRAY_FOREACH ( j, dCur )
				{
					const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr ( dCur[j] );
					if ( tCol.m_bWeight )
					{
						tExprCol.m_eStage = SPH_EVAL_PRESORT;
						tExprCol.m_bWeight = true;
					}
					// handle chains of dependencies (e.g. SELECT 1+attr f1, f1-1 f2 ... WHERE f2>5)
					if ( tCol.m_pExpr )
						tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
				}
				dCur.Uniq();

				ARRAY_FOREACH ( j, dCur )
				{
					auto & tDep = const_cast < CSphColumnInfo & > ( m_pSorterSchema->GetAttr ( dCur[j] ) );
					if ( tDep.m_eStage>tExprCol.m_eStage )
						tDep.m_eStage = tExprCol.m_eStage;
				}
				break;
			}

		// add it!
		// NOTE, "final" stage might need to be fixed up later
		// we'll do that when parsing sorting clause
		m_pSorterSchema->AddAttr ( tExprCol, true );
	} else // some aggregate
	{
		tExprCol.m_eStage = SPH_EVAL_PRESORT; // sorter expects computed expression
		m_pSorterSchema->AddAttr ( tExprCol, true );
		if ( m_pExtra )
			m_pExtra->Add ( tExprCol.m_sName );

		/// update aggregate dependencies (e.g. SELECT 1+attr f1, min(f1), ...)
		CSphVector<int> dCur;
		tExprCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );

		ARRAY_FOREACH ( j, dCur )
		{
			const CSphColumnInfo & tCol = m_pSorterSchema->GetAttr ( dCur[j] );
			if ( tCol.m_pExpr )
				tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
		}
		dCur.Uniq();

		ARRAY_FOREACH ( j, dCur )
		{
			auto & tDep = const_cast < CSphColumnInfo & > ( m_pSorterSchema->GetAttr ( dCur[j] ) );
			if ( tDep.m_eStage>tExprCol.m_eStage )
				tDep.m_eStage = tExprCol.m_eStage;
		}
	}

	m_hQueryAttrs.Add ( tExprCol.m_sName );
	return true;
}

// Test for @geodist and setup, if any
bool QueueCreator_c::MaybeAddGeodistColumn ()
{

	if ( m_tQuery.m_bGeoAnchor && m_pSorterSchema->GetAttrIndex ( "@geodist" )<0 )
	{
		auto pExpr = new ExprGeodist_t ();
		if ( !pExpr->Setup ( &m_tQuery, *m_pSorterSchema.Ptr(), m_sError ))
		{
			pExpr->Release ();
			return false;
		}
		CSphColumnInfo tCol ( "@geodist", SPH_ATTR_FLOAT );
		tCol.m_pExpr = pExpr; // takes ownership, no need to for explicit pExpr release
		tCol.m_eStage = SPH_EVAL_PREFILTER; // OPTIMIZE? actual stage depends on usage
		m_pSorterSchema->AddAttr ( tCol, true );
		if ( m_pExtra )
			m_pExtra->Add ( tCol.m_sName );
		m_hQueryAttrs.Add ( tCol.m_sName );
	}
	return true;
}

// Test for @expr and setup, if any
bool QueueCreator_c::MaybeAddExprColumn ()
{

	if ( m_tQuery.m_eSort==SPH_SORT_EXPR && m_pSorterSchema->GetAttrIndex ( "@expr" )<0 )
	{
		CSphColumnInfo tCol ( "@expr", SPH_ATTR_FLOAT ); // enforce float type for backwards compatibility
		// (ie. too lazy to fix those tests right now)
		bool bHasZonespanlist;
		tCol.m_pExpr = sphExprParse ( m_tQuery.m_sSortBy.cstr (), *m_pSorterSchema.Ptr(),
				nullptr, nullptr, m_sError, m_tSettings.m_pProfiler,
				m_tQuery.m_eCollation, nullptr, &bHasZonespanlist );
		m_bZonespanlist |= bHasZonespanlist;
		ExprParseArgs_t tExprArgs;
//		tExprArgs.m_pProfiler = tQueue.m_pProfiler;
//		tExprArgs.m_eCollation = tQuery.m_eCollation;
//		tExprArgs.m_pZonespanlist = &tArgs.m_bHasZonespanlist;
//
//		CSphColumnInfo tCol ( "@expr", SPH_ATTR_FLOAT ); // enforce float type for backwards compatibility (ie. too lazy to fix those tests right now)
//		tCol.m_pExpr = sphExprParse ( tQuery.m_sSortBy.cstr(), tSorterSchema, sError, tExprArgs );
//		tArgs.m_bNeedZonespanlist |= tArgs.m_bHasZonespanlist;
		if ( !tCol.m_pExpr )
			return false;
		tCol.m_eStage = SPH_EVAL_PRESORT;
		m_pSorterSchema->AddAttr ( tCol, true );
		m_hQueryAttrs.Add ( tCol.m_sName );
	}
	return true;
}

// Add computed items
bool QueueCreator_c::MaybeAddExpressionsFromSelectList ()
{
	// expressions from select items
	if ( m_tSettings.m_bComputeItems )
	{
		if ( !m_tQuery.m_dItems.TestAll (
				[&] ( const CSphQueryItem & v ) { return ParseQueryItem ( v ); } ))
			return false;

		// add expressions for stored fields
		if ( m_tQuery.m_dItems.FindFirst ( [] ( const CSphQueryItem & v )
			{ return v.m_sExpr=="*"; } ))
			for ( int i = 0; i<m_tSettings.m_tSchema.GetFieldsCount (); ++i )
			{
				const CSphColumnInfo & tField = m_tSettings.m_tSchema.GetField ( i );
				if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
				{
					CSphQueryItem tItem;
					tItem.m_sExpr = tItem.m_sAlias = tField.m_sName;
					if ( !ParseQueryItem ( tItem ))
						return false;
				}
			}
	}
	return true;
}

bool QueueCreator_c::MaybeAddGroupbyMagic ( bool bGotDistinct )
{
	CSphString sJsonGroupBy;
	// now lets add @groupby etc if needed
	if ( m_bGotGroupby && m_pSorterSchema->GetAttrIndex ( "@groupby" )<0 )
	{
		ESphAttr eGroupByResult = ( !m_tGroupSorterSettings.m_bImplicit )
				? m_tGroupSorterSettings.m_pGrouper->GetResultType ()
				: SPH_ATTR_INTEGER; // implicit do not have grouper
		// all FACET group by should be widest possible type
		if ( m_tGroupSorterSettings.m_bMva64
			|| m_tQuery.m_bFacet
			|| m_tQuery.m_bFacetHead
			|| m_bMulti )
			eGroupByResult = SPH_ATTR_BIGINT;

		CSphColumnInfo tGroupby ( "@groupby", eGroupByResult );
		CSphColumnInfo tCount ( "@count", SPH_ATTR_BIGINT );

		tGroupby.m_eStage = SPH_EVAL_SORTER;
		tCount.m_eStage = SPH_EVAL_SORTER;

		m_pSorterSchema->AddAttr ( tGroupby, true );
		m_pSorterSchema->AddAttr ( tCount, true );
		if ( bGotDistinct )
		{
			CSphColumnInfo tDistinct ( "@distinct", SPH_ATTR_INTEGER );
			tDistinct.m_eStage = SPH_EVAL_SORTER;
			m_pSorterSchema->AddAttr ( tDistinct, true );
		}

		// add @groupbystr last in case we need to skip it on sending (like @int_str2ptr_*)
		if ( m_tGroupSorterSettings.m_bJson )
		{
			sJsonGroupBy = SortJsonInternalSet ( m_tQuery.m_sGroupBy );
			if ( !m_pSorterSchema->GetAttr ( sJsonGroupBy.cstr() ) )
			{
				CSphColumnInfo tGroupbyStr ( sJsonGroupBy.cstr(), SPH_ATTR_JSON_FIELD );
				tGroupbyStr.m_eStage = SPH_EVAL_SORTER;
				m_pSorterSchema->AddAttr ( tGroupbyStr, true );
			}
		}
	}

	#define LOC_CHECK( _cond, _msg ) if (!(_cond)) { m_sError = "invalid schema: " _msg; return false; }

	int iGroupby = m_pSorterSchema->GetAttrIndex ( "@groupby" );
	if ( iGroupby>=0 ) {
		m_tGroupSorterSettings.m_bDistinct = bGotDistinct;
		m_tGroupSorterSettings.m_tLocGroupby = m_pSorterSchema->GetAttr ( iGroupby ).m_tLocator;
		LOC_CHECK ( m_tGroupSorterSettings.m_tLocGroupby.m_bDynamic, "@groupby must be dynamic" );

		int iCount = m_pSorterSchema->GetAttrIndex ( "@count" );
		LOC_CHECK ( iCount>=0, "missing @count" );

		m_tGroupSorterSettings.m_tLocCount = m_pSorterSchema->GetAttr ( iCount ).m_tLocator;
		LOC_CHECK ( m_tGroupSorterSettings.m_tLocCount.m_bDynamic, "@count must be dynamic" );

		int iDistinct = m_pSorterSchema->GetAttrIndex ( "@distinct" );
		if ( bGotDistinct ) {
			LOC_CHECK ( iDistinct>=0, "missing @distinct" );
			m_tGroupSorterSettings.m_tLocDistinct = m_pSorterSchema->GetAttr ( iDistinct ).m_tLocator;
			LOC_CHECK ( m_tGroupSorterSettings.m_tLocDistinct.m_bDynamic, "@distinct must be dynamic" );
		} else LOC_CHECK ( iDistinct<=0, "unexpected @distinct" );

		int iGroupbyStr = m_pSorterSchema->GetAttrIndex ( sJsonGroupBy.cstr() );
		if ( iGroupbyStr>=0 )
			m_tGroupSorterSettings.m_tLocGroupbyStr = m_pSorterSchema->GetAttr ( iGroupbyStr ).m_tLocator;
	}

	if ( m_bHasCount )
		LOC_CHECK ( m_pSorterSchema->GetAttrIndex ( "@count" )>=0
					, "Count(*) or @count is queried, but not available in the schema" );

	#undef LOC_CHECK
	return true;
}

bool QueueCreator_c::CheckHavingConstraints () const
{
	if ( m_tSettings.m_pAggrFilter && !m_tSettings.m_pAggrFilter->m_sAttrName.IsEmpty () )
	{
		if ( !m_bGotGroupby )
			return Err ( "can not use HAVING without GROUP BY" );

		// should be column named at group by or it's alias or aggregate
		const CSphString & sHaving = m_tSettings.m_pAggrFilter->m_sAttrName;
		if ( !IsGroupbyMagic ( sHaving ) )
		{
			bool bValidHaving = false;
			for ( const CSphQueryItem & tItem : m_tQuery.m_dItems )
			{
				if ( tItem.m_sAlias!=sHaving )
					continue;

				bValidHaving = ( IsGroupbyMagic ( tItem.m_sExpr ) || tItem.m_eAggrFunc!=SPH_AGGR_NONE );
				break;
			}

			if ( !bValidHaving )
				return Err ( "can not use HAVING with attribute not related to GROUP BY" );
		}
	}
	return true;
}


// only STRING ( static packed ) and JSON fields must be remapped
void QueueCreator_c::RemapStaticStringsAndJsons ( CSphMatchComparatorState & tState )
{
	assert ( m_pSorterSchema );
	auto & tSorterSchema = *m_pSorterSchema.Ptr();

	#ifndef NDEBUG
	int iColWasCount = tSorterSchema.GetAttrsCount();
#endif
	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; ++i )
	{
		if ( !( tState.m_eKeypart[i]==SPH_KEYPART_STRING || tState.m_tSubKeys[i].m_sKey.cstr() ) )
			continue;

		assert ( tState.m_dAttrs[i]>=0 && tState.m_dAttrs[i]<iColWasCount );

		bool bIsJson = !tState.m_tSubKeys[i].m_sKey.IsEmpty();
		bool bIsFunc = bIsJson && tState.m_tSubKeys[i].m_uMask==0;

		CSphString sRemapCol;
		sRemapCol.SetSprintf ( "%s%s", g_sIntAttrPrefix, bIsJson
			? tState.m_tSubKeys[i].m_sKey.cstr()
			: tSorterSchema.GetAttr ( tState.m_dAttrs[i] ).m_sName.cstr() );

		int iRemap = tSorterSchema.GetAttrIndex ( sRemapCol.cstr() );
		if ( iRemap==-1 && bIsJson )
		{
			CSphString sRemapLowercase = sRemapCol;
			sRemapLowercase.ToLower();
			iRemap = tSorterSchema.GetAttrIndex ( sRemapLowercase.cstr() );
		}

		if ( iRemap==-1 )
		{
			CSphColumnInfo tRemapCol ( sRemapCol.cstr(), SPH_ATTR_STRINGPTR );
			tRemapCol.m_eStage = SPH_EVAL_PRESORT;
			if ( bIsJson )
				if ( bIsFunc )
				{
					tRemapCol.m_pExpr = tState.m_tSubExpr[i];
					tRemapCol.m_pExpr->AddRef();
				} else
					tRemapCol.m_pExpr = new ExprSortJson2StringPtr_c ( tState.m_tLocator[i], tState.m_tSubExpr[i] );
			else
			{
				tRemapCol.m_pExpr = new ExprSortStringAttrFixup_c ( tState.m_tLocator[i] );
				tState.m_eKeypart[i] = SPH_KEYPART_STRINGPTR;
			}

			if ( bIsFunc )
			{
				tRemapCol.m_eAttrType = tState.m_tSubType[i];
				tState.m_eKeypart[i] = Attr2Keypart ( tState.m_tSubType[i] );
			}

			iRemap = tSorterSchema.GetAttrsCount();
			tSorterSchema.AddAttr ( tRemapCol, true );
		}
		tState.m_tLocator[i] = tSorterSchema.GetAttr ( iRemap ).m_tLocator;
		tState.m_dAttrs[i] = iRemap;
	}
}

// matches sorting function
bool QueueCreator_c::SetupMatchesSortingFunc ()
{
	m_bRandomize = false;
	if ( m_tQuery.m_eSort==SPH_SORT_EXTENDED )
	{
		ESortClauseParseResult eRes = sphParseSortClause ( &m_tQuery, m_tQuery.m_sSortBy.cstr(),
				*m_pSorterSchema.Ptr(), m_eMatchFunc, m_tStateMatch, m_tSettings.m_bComputeItems,
				m_sError );

		if ( eRes==SORT_CLAUSE_ERROR )
			return false;

		if ( eRes==SORT_CLAUSE_RANDOM )
			m_bRandomize = true;

		ExtraAddSortkeys ( m_tStateMatch.m_dAttrs );

		AssignOrderByToPresortStage ( m_tStateMatch.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );
		RemapStaticStringsAndJsons ( m_tStateMatch );
		return true;
	}

	if ( m_tQuery.m_eSort==SPH_SORT_EXPR )
	{
		m_tStateMatch.m_eKeypart[0] = SPH_KEYPART_INT;
		m_tStateMatch.m_tLocator[0] = m_pSorterSchema->GetAttr ( m_pSorterSchema->GetAttrIndex ( "@expr" ) ).m_tLocator;
		m_tStateMatch.m_eKeypart[1] = SPH_KEYPART_ROWID;
		m_tStateMatch.m_uAttrDesc = 1;
		m_eMatchFunc = FUNC_EXPR;
		return true;
	}

	// check sort-by attribute
	if ( m_tQuery.m_eSort!=SPH_SORT_RELEVANCE )
	{
		int iSortAttr = m_pSorterSchema->GetAttrIndex ( m_tQuery.m_sSortBy.cstr() );
		if ( iSortAttr<0 )
			return Errz ( "sort-by attribute '%s' not found", m_tQuery.m_sSortBy.cstr() );

		const CSphColumnInfo & tAttr = m_pSorterSchema->GetAttr ( iSortAttr );
		m_tStateMatch.m_eKeypart[0] = Attr2Keypart ( tAttr.m_eAttrType );
		m_tStateMatch.m_tLocator[0] = tAttr.m_tLocator;
		m_tStateMatch.m_dAttrs[0] = iSortAttr;
		RemapStaticStringsAndJsons ( m_tStateMatch );
	}

	SetupSortByDocID ( *m_pSorterSchema.Ptr(), m_tStateMatch, nullptr, m_tSettings.m_bComputeItems );

	// find out what function to use and whether it needs attributes
	switch (m_tQuery.m_eSort )
	{
		case SPH_SORT_ATTR_DESC:	m_eMatchFunc = FUNC_ATTR_DESC; break;
		case SPH_SORT_ATTR_ASC:		m_eMatchFunc = FUNC_ATTR_ASC; break;
		case SPH_SORT_TIME_SEGMENTS:	m_eMatchFunc = FUNC_TIMESEGS; break;
		case SPH_SORT_RELEVANCE:	m_eMatchFunc = FUNC_REL_DESC; break;
		default:
			return Errz ( "unknown sorting mode %d", m_tQuery.m_eSort );
	}
	return true;
}

bool QueueCreator_c::SetupGroupSortingFunc ( bool bGotDistinct )
{
	assert ( m_bGotGroupby );
	ESortClauseParseResult eRes = sphParseSortClause ( &m_tQuery,
			m_tQuery.m_sGroupSortBy.cstr(), *m_pSorterSchema.Ptr(), m_eGroupFunc,
			m_tStateGroup, m_tSettings.m_bComputeItems, m_sError );

	if ( eRes==SORT_CLAUSE_ERROR || eRes==SORT_CLAUSE_RANDOM )
	{
		if ( eRes==SORT_CLAUSE_RANDOM )
			m_sError = "groups can not be sorted by @random";
		return false;
	}

	ExtraAddSortkeys ( m_tStateGroup.m_dAttrs );

	assert ( m_dGroupColumns.GetLength() || m_tGroupSorterSettings.m_bImplicit );
	if ( m_pExtra && !m_tGroupSorterSettings.m_bImplicit )
	{
		for ( const auto& dGroupColumn : m_dGroupColumns )
			m_pExtra->Add ( m_pSorterSchema->GetAttr ( dGroupColumn ).m_sName );
	}

	if ( bGotDistinct )
	{
		m_dGroupColumns.Add ( m_pSorterSchema->GetAttrIndex ( m_tQuery.m_sGroupDistinct.cstr() ) );
		assert ( m_dGroupColumns.Last()>=0 );
		if ( m_pExtra )
			m_pExtra->Add ( m_pSorterSchema->GetAttr ( m_dGroupColumns.Last() ).m_sName );
	}

	if ( !m_dGroupColumns.IsEmpty() ) // implicit case
		AssignOrderByToPresortStage ( m_dGroupColumns.Begin (), m_dGroupColumns.GetLength ());

	AssignOrderByToPresortStage ( m_tStateGroup.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );

	// GroupSortBy str attributes setup
	RemapStaticStringsAndJsons ( m_tStateGroup );
	return true;
}

// set up aggregate filter for grouper
ISphFilter * QueueCreator_c::CreateAggrFilter () const
{
	assert ( m_bGotGroupby );
	if ( m_pSorterSchema->GetAttr ( m_tSettings.m_pAggrFilter->m_sAttrName.cstr() ) )
		return sphCreateAggrFilter ( m_tSettings.m_pAggrFilter, m_tSettings.m_pAggrFilter->m_sAttrName,
				*m_pSorterSchema.Ptr(), m_sError );

	// having might reference aliased attributes but @* attributes got stored without alias in sorter schema
	CSphString sHaving;
	for ( const auto& tItem : m_tQuery.m_dItems )
		if ( tItem.m_sAlias==m_tSettings.m_pAggrFilter->m_sAttrName )
		{
			sHaving = tItem.m_sExpr;
			break;
		}

	if ( sHaving=="groupby()" )
		sHaving = "@groupby";
	else if ( sHaving=="count(*)" )
		sHaving = "@count";

	return sphCreateAggrFilter ( m_tSettings.m_pAggrFilter, sHaving, *m_pSorterSchema.Ptr(), m_sError );
}

void QueueCreator_c::SetupCollation ()
{
	switch (m_tQuery.m_eCollation)
	{
	case SPH_COLLATION_LIBC_CI:
		m_tStateMatch.m_fnStrCmp = sphCollateLibcCI;
		m_tStateGroup.m_fnStrCmp = sphCollateLibcCI;
		break;
	case SPH_COLLATION_LIBC_CS:
		m_tStateMatch.m_fnStrCmp = sphCollateLibcCS;
		m_tStateGroup.m_fnStrCmp = sphCollateLibcCS;
		break;
	case SPH_COLLATION_UTF8_GENERAL_CI:
		m_tStateMatch.m_fnStrCmp = sphCollateUtf8GeneralCI;
		m_tStateGroup.m_fnStrCmp = sphCollateUtf8GeneralCI;
		break;
	case SPH_COLLATION_BINARY:
		m_tStateMatch.m_fnStrCmp = sphCollateBinary;
		m_tStateGroup.m_fnStrCmp = sphCollateBinary;
		break;
	}
}

bool QueueCreator_c::AddGroupbyStuff ()
{
	////////////////////////////////////////////
	// setup groupby settings, create shortcuts
	////////////////////////////////////////////

	// need schema with group related columns however not need grouper
	m_bHeadWOGroup = ( m_tQuery.m_sGroupBy.IsEmpty () && m_tQuery.m_bFacetHead );
	auto fnIsImplicit = [] ( const CSphQueryItem & t ) {
		return ( t.m_eAggrFunc!=SPH_AGGR_NONE ) || t.m_sExpr=="count(*)" || t.m_sExpr=="@distinct";
	};

	bool bHasImplicitGrouping = m_tQuery.m_sGroupBy.IsEmpty ()
			? m_tQuery.m_dItems.FindFirst ( fnIsImplicit ) : false;

	// count(*) and distinct wo group by at main query should keep implicit flag
	if ( bHasImplicitGrouping && m_bHeadWOGroup )
		m_bHeadWOGroup = !m_tQuery.m_dRefItems.FindFirst ( fnIsImplicit );

	if ( !SetupGroupbySettings ( bHasImplicitGrouping ))
		return false;

	// or else, check in SetupGroupbySettings() would already fail
	m_bGotGroupby = !m_tQuery.m_sGroupBy.IsEmpty () || m_tGroupSorterSettings.m_bImplicit;
	m_bGotDistinct = ( m_tGroupSorterSettings.m_tDistinctLoc.m_iBitOffset>=0 );

	if ( m_bHasGroupByExpr && !m_bGotGroupby )
		return Err ( "GROUPBY() is allowed only in GROUP BY queries" );

	// check for HAVING constrains
	if ( !CheckHavingConstraints ())
		return false;

	// now lets add @groupby stuff, if necessary
	return MaybeAddGroupbyMagic ( m_bGotDistinct );
}

bool QueueCreator_c::SetGroupSorting ()
{
	if ( m_bGotGroupby ) {
		if ( !SetupGroupSortingFunc ( m_bGotDistinct ))
			return false;

		if ( m_tSettings.m_pAggrFilter && !m_tSettings.m_pAggrFilter->m_sAttrName.IsEmpty ()) {
			auto pFilter = CreateAggrFilter ();
			if ( !pFilter )
				return false;
			m_tGroupSorterSettings.m_pAggrFilterTrait = pFilter;
		}
	}
	return true;
}

ISphMatchSorter * QueueCreator_c::SpawnQueue()
{
	if ( !m_bGotGroupby )
	{
		if ( m_tSettings.m_pUpdate )
			return new CSphUpdateQueue ( m_tQuery.m_iMaxMatches, m_tSettings.m_pUpdate,
					m_tQuery.m_bIgnoreNonexistent, m_tQuery.m_bStrict );

		if ( m_tSettings.m_pCollection )
			return new CSphCollectQueue ( m_tQuery.m_iMaxMatches, m_tSettings.m_pCollection );

		return CreatePlainSorter ( m_eMatchFunc, m_tQuery.m_bSortKbuffer, m_tQuery.m_iMaxMatches,
			m_uPackedFactorFlags & SPH_FACTOR_ENABLE );
	}
	return sphCreateSorter1st ( m_eMatchFunc, m_eGroupFunc, &m_tQuery, m_tGroupSorterSettings,
		m_uPackedFactorFlags & SPH_FACTOR_ENABLE );
}

bool QueueCreator_c::SetupComputeQueue ()
{
	return MaybeAddGeodistColumn ()
		&& MaybeAddExprColumn ()
		&& MaybeAddExpressionsFromSelectList ();
}

bool QueueCreator_c::SetupGroupQueue ()
{
	return AddGroupbyStuff ()
		&& SetupMatchesSortingFunc ()
		&& SetGroupSorting ();
}

bool QueueCreator_c::SetupQueue ()
{
	return SetupComputeQueue () && SetupGroupQueue ();
}

ISphMatchSorter * QueueCreator_c::CreateQueue ()
{

	SetupCollation ();

	if ( m_bHeadWOGroup && m_tGroupSorterSettings.m_bImplicit ) {
		m_tGroupSorterSettings.m_bImplicit = false;
		m_bGotGroupby = false;
	}

	///////////////////
	// spawn the queue
	///////////////////

	ISphMatchSorter * pTop = SpawnQueue ();
	if ( !pTop )
	return Errz ( "internal error: unhandled sorting mode (match-sort=%d, group=%d, group-sort=%d)",
			m_eMatchFunc, m_bGotGroupby, m_eGroupFunc );

	assert ( pTop );
	pTop->SetState ( m_tStateMatch );
	pTop->SetGroupState ( m_tStateGroup );
	pTop->SetSchema ( m_pSorterSchema.LeakPtr(), false );
	pTop->m_bRandomize = m_bRandomize;

	if ( m_bRandomize )
	{
		if ( m_tQuery.m_iRandSeed>=0 )
			sphSrand ( (DWORD) m_tQuery.m_iRandSeed );
		else
			sphAutoSrand ();
	}

	return pTop;
}

static ISphMatchSorter * CreateQueue ( QueueCreator_c & tCreator, SphQueueRes_t & tRes )
{
	ISphMatchSorter * pSorter = tCreator.CreateQueue ();
	tRes.m_bZonespanlist = tCreator.m_bZonespanlist;
	tRes.m_uPackedFactorFlags = tCreator.m_uPackedFactorFlags;
	return pSorter;
}


int sphFlattenQueue ( ISphMatchSorter * pQueue, CSphQueryResult * pResult, int iTag )
{
	if ( !pQueue || !pQueue->GetLength() )
		return 0;

	int iOffset = pResult->m_dMatches.GetLength ();
	pResult->m_dMatches.Resize ( iOffset + pQueue->GetLength() );
	int iCopied = pQueue->Flatten ( pResult->m_dMatches.Begin() + iOffset, iTag );
	pResult->m_dMatches.Resize ( iOffset + iCopied );
	return iCopied;
}


bool sphHasExpressions ( const CSphQuery & tQuery, const CSphSchema & tSchema )
{
	for ( const CSphQueryItem &tItem : tQuery.m_dItems )
	{
		const CSphString & sExpr = tItem.m_sExpr;

		// all expressions that come from parser are automatically aliased
		assert ( !tItem.m_sAlias.IsEmpty() );

		if ( !( sExpr=="*"
			|| ( tSchema.GetAttrIndex ( sExpr.cstr() )>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE && tItem.m_sAlias==sExpr )
			|| IsGroupbyMagic ( sExpr ) ) )
			return true;
	}
	return false;
}

static void CreateSorters ( const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries,
	const VecTraits_T<ISphMatchSorter*> & dSorters, const VecTraits_T<QueueCreator_c> & dCreators,
	const VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes )
{
	ARRAY_FOREACH ( i, dCreators )
	{
		if ( !dCreators[i].m_bCreate )
			continue;

		dSorters[i] = CreateQueue ( dCreators[i], tRes );
		assert ( dSorters[i]!=nullptr );
	}

	if ( tRes.m_bAlowMulti )
	{
		ISphMatchSorter * pSorter0 = nullptr;
		for ( int iCheck=0; iCheck<dSorters.GetLength(); ++iCheck )
		{
			if ( !dCreators[iCheck].m_bCreate )
				continue;

			assert ( dSorters[iCheck] );
			if ( !pSorter0 )
			{
				pSorter0 = dSorters[iCheck];
				continue;
			}

			assert ( dSorters[iCheck]->GetSchema()->GetAttrsCount()==pSorter0->GetSchema()->GetAttrsCount() );
		}
	}
}

ISphMatchSorter * sphCreateQueue ( const SphQueueSettings_t & tQueue, const CSphQuery & tQuery, CSphString & sError, SphQueueRes_t & tRes, StrVec_t * pExtra )
{
	QueueCreator_c tCreator ( tQueue, tQuery, sError, pExtra );
	if ( !tCreator.SetupQueue () )
		return nullptr;

	return CreateQueue ( tCreator, tRes );
}

template<typename T>
using RawVector_T = sph::Vector_T<T, sph::SwapCopy_T<T>, sph::DefaultRelimit, sph::RawStorage_T<T>>;

void CreateMultiQueue ( RawVector_T<QueueCreator_c>& dCreators, const SphQueueSettings_t & tQueue,
		const VecTraits_T<CSphQuery> & dQueries, VecTraits_T<ISphMatchSorter*> & dSorters,
		VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes, VecTraits_T<StrVec_t> & dExtras )
{
	assert ( dSorters.GetLength()>1 );
	assert ( dSorters.GetLength()==dQueries.GetLength() );
	assert ( dExtras.IsEmpty() || dSorters.GetLength()==dExtras.GetLength() );
	assert ( dSorters.GetLength()==dErrors.GetLength() );

	dCreators.Reserve_static ( dSorters.GetLength () );
	dCreators.Emplace_back( tQueue, dQueries[0], dErrors[0], dExtras.IsEmpty () ? nullptr : dExtras.begin () );
	dCreators[0].m_bMulti = true;

	// same as SetupQueue
	dCreators[0].SetupComputeQueue ();
	// copy schema WO group by and internals
	CSphRsetSchema tRefSchema = dCreators[0].SorterSchema();
	bool bHasJson = dCreators[0].HasJson();
	bool bJsonMixed = false;

	dCreators[0].SetupGroupQueue ();

	// create rest of schemas
	for ( int i=1; i<dSorters.GetLength(); ++i )
	{
		// fill extra only for initial pass
		dCreators.Emplace_back ( tQueue, dQueries[i], dErrors[i], dExtras.IsEmpty () ? nullptr : &dExtras[i] );
		dCreators[i].m_bMulti = true;
		if ( !dCreators[i].SetupQueue () )
		{
			dCreators[i].m_bCreate = false;
			continue;
		}

		bJsonMixed |= ( bHasJson!=dCreators[i].HasJson () );
		bHasJson |= dCreators[i].HasJson();
	}

	// FIXME!!! check attributes and expressions matches
	bool bSame = !bJsonMixed;
	const auto& tSchema0 = dCreators[0].SorterSchema();
	for ( int i=1; i<dCreators.GetLength() && bSame; ++i )
	{
		const auto & tCur = dCreators[i].SorterSchema();
		bSame &= ( tSchema0.GetDynamicSize()==tCur.GetDynamicSize() && tSchema0.GetAttrsCount()==tCur.GetAttrsCount() );
	}

	// same schemes
	if ( bSame )
		return;

	CSphRsetSchema tMultiSchema = tRefSchema;

	int iMinGroups = INT_MAX;
	int iMaxGroups = 0;
	bool bHasMulti = false;
	ARRAY_FOREACH ( iSchema, dCreators )
	{
		if ( !dCreators[iSchema].m_bCreate )
			continue;

		int iGroups = 0;
		const CSphRsetSchema & tSchema = dCreators[iSchema].SorterSchema();
		for ( int iCol=0; iCol<tSchema.GetAttrsCount(); ++iCol )
		{
			const CSphColumnInfo & tCol = tSchema.GetAttr ( iCol );
			if ( !tCol.m_tLocator.m_bDynamic )
				continue;

			if ( IsGroupbyMagic ( tCol.m_sName ) )
			{
				++iGroups;
				if ( !IsSortJsonInternal ( tCol.m_sName ))
					continue;
			}

			const CSphColumnInfo * pMultiCol = tMultiSchema.GetAttr ( tCol.m_sName.cstr() );
			if ( pMultiCol )
			{
				bool bDisable1 = false;
				bool bDisable2 = false;
				// no need to add attributes that already exists
				if ( pMultiCol->m_eAttrType==tCol.m_eAttrType &&
					pMultiCol->m_pExpr->GetHash ( tMultiSchema, SPH_FNV64_SEED, bDisable1 )==tCol.m_pExpr->GetHash ( tSchema, SPH_FNV64_SEED, bDisable2 ) )
					continue;

				// if attr or expr differs need to create regular sorters and issue search WO multi-query
				tRes.m_bAlowMulti = false;
				return;
			}

			bHasMulti = true;
			tMultiSchema.AddAttr ( tCol, true );
			if ( tCol.m_pExpr )
				tCol.m_pExpr->FixupLocator ( &tSchema, &tMultiSchema );

		}

		iMinGroups = Min ( iMinGroups, iGroups );
		iMaxGroups = Max ( iMaxGroups, iGroups );
	}

	// usual multi query should all have similar group by
	if ( iMinGroups!=iMaxGroups && !dQueries[0].m_bFacetHead && !dQueries[0].m_bFacet )
	{
		tRes.m_bAlowMulti = false;
		return;
	}

	// only group attributes differs - create regular sorters
	if ( !bHasMulti && !bJsonMixed )
		return;

	// setup common schemas
	for ( auto& dCreator : dCreators )
	{
		if ( !dCreator.m_bCreate )
			continue;

		dCreator.SorterSchema() = tMultiSchema;
		if ( !dCreator.SetupGroupQueue () )
			dCreator.m_bCreate = false;
	}
}

void sphCreateMultiQueue ( const SphQueueSettings_t & tQueue, const VecTraits_T<CSphQuery> & dQueries
		, VecTraits_T<ISphMatchSorter *> & dSorters, VecTraits_T<CSphString> & dErrors, SphQueueRes_t & tRes
		, VecTraits_T<StrVec_t> & dExtras )
{
	RawVector_T<QueueCreator_c> dCreators;
	CreateMultiQueue ( dCreators, tQueue, dQueries, dSorters, dErrors, tRes, dExtras );
	CreateSorters ( tQueue, dQueries, dSorters, dCreators, dErrors, tRes );
}