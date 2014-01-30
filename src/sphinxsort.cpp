//
// $Id$
//

//
// Copyright (c) 2001-2014, Andrew Aksyonoff
// Copyright (c) 2008-2014, Sphinx Technologies Inc
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

#include <time.h>
#include <math.h>

#if !USE_WINDOWS
#include <unistd.h>
#include <sys/time.h>
#endif

//////////////////////////////////////////////////////////////////////////
// TRAITS
//////////////////////////////////////////////////////////////////////////

void ISphMatchSorter::SetState ( const CSphMatchComparatorState & tState )
{
	m_tState = tState;
	m_tState.m_iNow = (DWORD) time ( NULL );
}


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
	virtual bool			CanMulti () const { return true; }
};


static bool HasString ( const CSphMatchComparatorState * pState )
{
	assert ( pState );

	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( pState->m_eKeypart[i]==SPH_KEYPART_STRING || pState->m_eKeypart[i]==SPH_KEYPART_STRINGPTR || ( pState->m_tSubKeys[i].m_sKey.cstr() ) )
			return true;
	}

	return false;
}



/// match-sorting priority queue traits
class CSphMatchQueueTraits : public ISphMatchSorter, ISphNoncopyable
{
protected:
	CSphMatch *					m_pData;
	int							m_iUsed;
	int							m_iSize;
	const bool					m_bUsesAttrs;

private:
	int							m_iAllocatedSize;
	int							m_iDataLength;

public:
	/// ctor
	CSphMatchQueueTraits ( int iSize, bool bUsesAttrs )
		: m_iUsed ( 0 )
		, m_iSize ( iSize )
		, m_bUsesAttrs ( bUsesAttrs )
		, m_iAllocatedSize ( iSize )
		, m_iDataLength ( iSize )
	{
		assert ( iSize>0 );
		m_pData = new CSphMatch [ m_iAllocatedSize ];
		assert ( m_pData );

		m_tState.m_iNow = (DWORD) time ( NULL );
	}

	/// dtor
	~CSphMatchQueueTraits ()
	{
		for ( int i=0; i<m_iAllocatedSize; ++i )
			m_tSchema.FreeStringPtrs ( m_pData+i );
		SafeDeleteArray ( m_pData );
	}

public:
	bool			UsesAttrs () const										{ return m_bUsesAttrs; }
	virtual int			GetLength () const										{ return m_iUsed; }
	virtual int			GetDataLength () const									{ return m_iDataLength; }

	virtual bool CanMulti () const
	{
		return !HasString ( &m_tState );
	}
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
	CSphMatchQueue ( int iSize, bool bUsesAttrs )
		: CSphMatchQueueTraits ( iSize, bUsesAttrs )
	{
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);
	}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
	{
		return false;
	}

	virtual const CSphMatch * GetWorst() const
	{
		return m_pData;
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		m_iTotal++;

		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = 0;
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
		m_tSchema.CloneMatch ( m_pData+m_iUsed, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tEntry.m_uDocID;

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
	virtual bool PushGrouped ( const CSphMatch &, bool )
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
		m_tSchema.FreeStringPtrs ( &m_pData[m_iUsed] );

		if_const ( NOTIFICATIONS )
		{
			if ( m_dJustPopped.GetLength() )
				m_dJustPopped[0] = m_pData[m_iUsed].m_uDocID;
			else
				m_dJustPopped.Add ( m_pData[m_iUsed].m_uDocID );
		}

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
	int Flatten ( CSphMatch * pTo, int iTag )
	{
		assert ( m_iUsed>=0 );
		pTo += m_iUsed;
		int iCopied = m_iUsed;
		while ( m_iUsed>0 )
		{
			--pTo;
			m_tSchema.FreeStringPtrs ( pTo );
			Swap ( *pTo, *m_pData );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;
			Pop ();
		}
		m_iTotal = 0;
		return iCopied;
	}

	void Finalize ( ISphMatchProcessor & tProcessor, bool bCallProcessInResultSetOrder )
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

	bool IsLess ( const MEDIAN_TYPE a, const MEDIAN_TYPE b )
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
	CSphKbufferMatchQueue ( int iSize, bool bUsesAttrs )
		: CSphMatchQueueTraits ( iSize*COEFF, bUsesAttrs )
		, m_pEnd ( m_pData+iSize*COEFF )
		, m_pWorst ( NULL )
		, m_bFinalized ( false )
	{
		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );

		m_iSize /= COEFF;
	}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
	{
		return false;
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = 0;
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
		m_tSchema.CloneMatch ( m_pEnd-m_iUsed, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tEntry.m_uDocID;

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
					m_dJustPopped.Add ( pMatch->m_uDocID );
			}

			m_iUsed = m_iSize;
			m_pWorst = m_pEnd-m_iSize;
			m_bFinalized = true;
		}
		return true;
	}

	/// add grouped entry (must not happen)
	virtual bool PushGrouped ( const CSphMatch &, bool )
	{
		assert ( 0 );
		return false;
	}

	/// finalize, perform final sort/cut as needed
	virtual void Finalize ( ISphMatchProcessor & tProcessor, bool )
	{
		if ( !GetLength() )
			return;

		if ( !m_bFinalized )
		{
			MatchSort_fn<COMP> tComp ( m_tState );
			sphSort ( m_pEnd-m_iUsed, m_iUsed, tComp, tComp );
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
	virtual int GetLength () const
	{
		return Min ( m_iUsed, m_iSize );
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag )
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
			m_tSchema.FreeStringPtrs ( pTo );
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
		m_tWorkSet.m_dRows.Resize ( m_iUsed );

		ARRAY_FOREACH ( i, m_tWorkSet.m_dDocids )
		{
			m_tWorkSet.m_dRowOffset[i] = 0;
			m_tWorkSet.m_dDocids[i] = 0;
			m_tWorkSet.m_dRows[i] = NULL;
			if ( !DOCINFO2ID ( STATIC2DOCINFO ( m_pData[i].m_pStatic ) ) ) // if static attributes were copied, so, they actually dynamic
			{
				m_tWorkSet.m_dDocids[i] = m_pData[i].m_uDocID;
			} else // static attributes points to the active indexes - so, no lookup, 5 times faster update.
			{
				m_tWorkSet.m_dRows[i] = m_pData[i].m_pStatic - ( sizeof(SphDocID_t) / sizeof(CSphRowitem) );
			}
		}

		*m_pAffected += m_pIndex->UpdateAttributes ( m_tWorkSet, -1, *m_pError, *m_pWarning );
		m_iUsed = 0;
	}
public:
	/// ctor
	CSphUpdateQueue ( int iSize, CSphAttrUpdateEx * pUpdate, bool bIgnoreNonexistent, bool bStrict )
		: CSphMatchQueueTraits ( iSize, true )
	{
		m_tWorkSet.m_dRowOffset.Reserve ( m_iSize );
		m_tWorkSet.m_dDocids.Reserve ( m_iSize );
		m_tWorkSet.m_dRows.Reserve ( m_iSize );

		m_tWorkSet.m_bIgnoreNonexistent = bIgnoreNonexistent;
		m_tWorkSet.m_bStrict = bStrict;
		m_tWorkSet.m_dTypes = pUpdate->m_pUpdate->m_dTypes;
		m_tWorkSet.m_dPool = pUpdate->m_pUpdate->m_dPool;
		m_tWorkSet.m_dAttrs.Resize ( pUpdate->m_pUpdate->m_dAttrs.GetLength() );
		ARRAY_FOREACH ( i, m_tWorkSet.m_dAttrs )
		{
			CSphString sTmp;
			sTmp = pUpdate->m_pUpdate->m_dAttrs[i];
			m_tWorkSet.m_dAttrs[i] = sTmp.Leak();
		}

		m_pIndex = pUpdate->m_pIndex;
		m_pError = pUpdate->m_pError;
		m_pWarning = pUpdate->m_pWarning;
		m_pAffected = &pUpdate->m_iAffected;
	}

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
		m_tSchema.CloneMatch ( &m_pData[m_iUsed++], tEntry );
		return true;
	}

	/// add grouped entry (must not happen)
	virtual bool PushGrouped ( const CSphMatch &, bool )
	{
		assert ( 0 );
		return false;
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch *, int )
	{
		assert ( m_iUsed>=0 );
		DoUpdate();
		m_iTotal = 0;
		return 0;
	}

	virtual void Finalize ( ISphMatchProcessor & tProcessor, bool )
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

/// collector for DELETE statement
class CSphDeleteQueue : public CSphMatchQueueTraits
{
	CSphVector<SphDocID_t>* m_pValues;
private:
	void DoDelete()
	{
		if ( !m_iUsed )
			return;

		if ( !DOCINFO2ID ( STATIC2DOCINFO ( m_pData->m_pStatic ) ) ) // if static attrs were copied, so, they actually dynamic
		{
			for ( int i=0; i<m_iUsed; ++i )
				m_pValues->Add ( m_pData[i].m_uDocID );
		} else // static attrs points to the active indexes - so, no lookup, 5 times faster search.
		{
			for ( int i=0; i<m_iUsed; ++i )
				m_pValues->Add ( DOCINFO2ID ( m_pData[i].m_pStatic - ( sizeof(SphDocID_t) / sizeof(CSphRowitem) )));
		}
		m_iUsed = 0;
	}
public:
	/// ctor
	CSphDeleteQueue ( int iSize, CSphVector<SphDocID_t>* pDeletes )
		: CSphMatchQueueTraits ( iSize, true )
		, m_pValues ( pDeletes )
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
			DoDelete();

		// do add
		m_tSchema.CloneMatch ( &m_pData[m_iUsed++], tEntry );
		return true;
	}

	/// add grouped entry (must not happen)
	virtual bool PushGrouped ( const CSphMatch &, bool )
	{
		assert ( 0 );
		return false;
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch *, int )
	{
		assert ( m_iUsed>=0 );
		DoDelete();
		m_iTotal = 0;
		return 0;
	}

	virtual void Finalize ( ISphMatchProcessor & tProcessor, bool )
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
// SORTING+GROUPING QUEUE
//////////////////////////////////////////////////////////////////////////

static bool IsCount ( const CSphString & s )
{
	return s=="@count" || s=="count(*)";
}

static bool IsGroupby ( const CSphString & s )
{
	return s=="@groupby" || s=="@distinct" || s=="groupby()";
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

template <class PRED>
class CSphGrouperString : public CSphGrouperAttr, public PRED
{
protected:
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

	virtual bool CanMulti () const { return false; }
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


/// lookup JSON key, group by looked up value
class CSphGrouperJson : public CSphGrouperString<BinaryHash_fn>
{
protected:
	ISphExpr * m_pExpr;

public:
	explicit CSphGrouperJson ( const CSphAttrLocator & tLoc, ISphExpr * pExpr )
		: CSphGrouperString<BinaryHash_fn> ( tLoc )
		, m_pExpr ( pExpr )
	{}

	virtual ~CSphGrouperJson ()
	{
		SafeDelete ( m_pExpr );
	}

	virtual SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const
	{
		if ( !m_pExpr )
			return SphGroupKey_t();

		// inlining Eval() for simple cases like json.key1 does not pay off
		// only yields marginal improvements (eg 42.8 ms vs 44.3 ms on ~457K rows)
		uint64_t uValue = m_pExpr->Int64Eval ( tMatch );
		const BYTE * pValue = m_pStringBase + ( uValue & 0xffffffff );
		ESphJsonType eRes = (ESphJsonType)( uValue >> 32 );
		int iLen;

		char sBuf[32];
		switch ( eRes )
		{
			case JSON_STRING:
			case JSON_STRING_VECTOR:
			case JSON_OBJECT:
			case JSON_MIXED_VECTOR:
				iLen = sphJsonUnpackInt ( &pValue );
				return sphFNV64 ( pValue, iLen );
			case JSON_INT32:
				return sphFNV64 ( FormatInt ( sBuf, (int)sphGetDword(pValue) ) );
			case JSON_INT64:
				return sphFNV64 ( FormatInt ( sBuf, (int)sphJsonLoadBigint ( &pValue ) ) );
			case JSON_DOUBLE:
				snprintf ( sBuf, sizeof(sBuf), "%f", sphQW2D ( sphJsonLoadBigint ( &pValue ) ) );
				return sphFNV64 ( sBuf );
			case JSON_INT32_VECTOR:
				iLen = sphJsonUnpackInt ( &pValue ) * 4;
				return sphFNV64 ( pValue, iLen );
			case JSON_INT64_VECTOR:
			case JSON_DOUBLE_VECTOR:
				iLen = sphJsonUnpackInt ( &pValue ) * 8;
				return sphFNV64 ( pValue, iLen );
			default:
				return 0;
		}
	}

	virtual void SetStringPool ( const BYTE * pStrings )
	{
		m_pStringBase = pStrings;
		if ( m_pExpr )
			m_pExpr->Command ( SPH_EXPR_SET_STRING_POOL, (void*)pStrings );
	}

	virtual SphGroupKey_t KeyFromValue ( SphAttr_t ) const { assert(0); return SphGroupKey_t(); }
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

	virtual ~CSphGrouperMulti ()
	{
		ARRAY_FOREACH ( i, m_dJsonKeys )
			SafeDelete ( m_dJsonKeys[i] );
	}

	virtual SphGroupKey_t KeyFromMatch ( const CSphMatch & tMatch ) const
	{
		SphGroupKey_t tKey = SPH_FNV64_SEED;

		for ( int i=0; i<m_dLocators.GetLength(); i++ )
		{
			SphAttr_t tAttr = tMatch.GetAttr ( m_dLocators[i] );
			if ( m_dAttrTypes[i]==SPH_ATTR_STRING )
			{
				assert ( m_pStringBase );

				const BYTE * pStr = NULL;
				int iLen = sphUnpackStr ( m_pStringBase+tAttr, &pStr );

				if ( !pStr || !iLen )
					continue;

				tKey = PRED::Hash ( pStr, iLen, tKey );

			} else if ( m_dAttrTypes[i]==SPH_ATTR_JSON )
			{
				assert ( m_pStringBase );

				const BYTE * pStr = NULL;
				int iLen = sphUnpackStr ( m_pStringBase+tAttr, &pStr );

				if ( !pStr || !iLen )
					continue;

				uint64_t uValue = m_dJsonKeys[i]->Int64Eval ( tMatch );
				const BYTE * pValue = m_pStringBase + ( uValue & 0xffffffff );
				ESphJsonType eRes = (ESphJsonType)( uValue >> 32 );

				int i32Val;
				int64_t i64Val;
				double fVal;
				switch ( eRes )
				{
				case JSON_STRING:
					iLen = sphJsonUnpackInt ( &pValue );
					tKey = sphFNV64 ( pValue, iLen, tKey );
					break;
				case JSON_INT32:
					i32Val = sphJsonLoadInt ( &pValue );
					tKey = sphFNV64 ( &i32Val, sizeof(i32Val), tKey );
					break;
				case JSON_INT64:
					i64Val = sphJsonLoadBigint ( &pValue );
					tKey = sphFNV64 ( &i64Val, sizeof(i64Val), tKey );
					break;
				case JSON_DOUBLE:
					fVal = sphQW2D ( sphJsonLoadBigint ( &pValue ) );
					tKey = sphFNV64 ( &fVal, sizeof(fVal), tKey );
					break;
				default:
					break;
				}

			} else
				tKey = sphFNV64 ( &tAttr, sizeof(SphAttr_t), tKey );
		}

		return tKey;
	}

	virtual void SetStringPool ( const BYTE * pStrings )
	{
		m_pStringBase = pStrings;

		ARRAY_FOREACH ( i, m_dJsonKeys )
		{
			if ( m_dJsonKeys[i] )
				m_dJsonKeys[i]->Command ( SPH_EXPR_SET_STRING_POOL, (void*)pStrings );
		}
	}

	virtual SphGroupKey_t KeyFromValue ( SphAttr_t ) const { assert(0); return SphGroupKey_t(); }
	virtual void GetLocator ( CSphAttrLocator & ) const { assert(0); }
	virtual ESphAttr GetResultType() const { return SPH_ATTR_BIGINT; }
	virtual bool CanMulti () const { return false; }

private:
	CSphVector<CSphAttrLocator>	m_dLocators;
	CSphVector<ESphAttr>		m_dAttrTypes;
	const BYTE *				m_pStringBase;
	CSphVector<ISphExpr *>		m_dJsonKeys;
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
	/// returns pointer to value if already hashed, or replace it with new one, if insisted.
	T * Add ( const T & tValue, const KEY & tKey, bool bReplace=false )
	{
		assert ( m_iFree>0 && "hash overflow" );

		// check if it's already hashed
		DWORD uHash = DWORD ( HASHFUNC::Hash ( tKey ) ) & ( m_dHash.GetLength()-1 );
		int iPrev = -1, iEntry;

		for ( iEntry=m_dHash[uHash]; iEntry>=0; iPrev=iEntry, iEntry=m_dEntries[iEntry].m_iNext )
			if ( m_dEntries[iEntry].m_tKey==tKey )
			{
				if ( bReplace )
					m_dEntries[iEntry].m_tValue = tValue;
				return &m_dEntries[iEntry].m_tValue;
			}
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
	int				m_iCount;

public:
	SphGroupedValue_t ()
	{}

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
	SphUngroupedValue_t ()
	{}

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
	int iCount = m_pData[m_iCountPos].m_iCount;
	*pOutGroup = uGroup;

	while ( m_iCountPos<m_iLength && m_pData[m_iCountPos].m_uGroup==uGroup )
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
	bool				m_bImplicit;		///< for queries with aggregate functions but without group by clause
	const ISphFilter *	m_pAggrFilterTrait; ///< aggregate filter that got owned by grouper

	CSphGroupSorterSettings ()
		: m_bDistinct ( false )
		, m_bMVA ( false )
		, m_bMva64 ( false )
		, m_pGrouper ( NULL )
		, m_bImplicit ( false )
		, m_pAggrFilterTrait ( NULL )
	{}
};


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


/// GROUP_CONCAT() implementation
class AggrConcat_t : public IAggrFunc
{
protected:
	CSphAttrLocator	m_tLoc;

public:
	explicit AggrConcat_t ( const CSphColumnInfo & tCol )
		: m_tLoc ( tCol.m_tLocator )
	{}

	void Ungroup ( CSphMatch * ) {}
	void Finalize ( CSphMatch * ) {}

	void Update ( CSphMatch * pDst, const CSphMatch * pSrc, bool )
	{
		const char * sDst = (const char*) pDst->GetAttr(m_tLoc);
		const char * sSrc = (const char*) pSrc->GetAttr(m_tLoc);
		assert ( !sDst || *sDst ); // ensure the string is either NULL, or has data
		assert ( !sSrc || *sSrc );

		// empty source? kinda weird, but done!
		if ( !sSrc )
			return;

		// empty destination? just clone the source
		if ( !sDst )
		{
			if ( sSrc )
				pDst->SetAttr ( m_tLoc, (SphAttr_t)strdup(sSrc) );
			return;
		}

		// both source and destination present? append source to destination
		// note that it gotta be manual copying here, as SetSprintf (currently) comes with a 1K limit
		assert ( sDst && sSrc );
		int iSrc = strlen ( sSrc );
		int iDst = strlen ( sDst );
		char * sNew = new char [ iSrc+iDst+2 ]; // OPTIMIZE? careful pre-reserve and/or realloc would be even faster
		memcpy ( sNew, sDst, iDst );
		sNew [ iDst ] = ',';
		memcpy ( sNew+iDst+1, sSrc, iSrc );
		sNew [ iSrc+iDst+1 ] = '\0';
		pDst->SetAttr ( m_tLoc, (SphAttr_t)sNew );
		SafeDeleteArray ( sDst );
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
	CSphFixedVector<CSphRowitem>	m_dRowBuf;
	CSphVector<CSphAttrLocator>		m_dAttrsRaw;
	CSphVector<CSphAttrLocator>		m_dAttrsPtr;
	const CSphRsetSchema *			m_pSchema;

	MatchCloner_t ()
		: m_dRowBuf ( 0 )
		, m_pSchema ( NULL )
	{ }

	void SetSchema ( const CSphRsetSchema * pSchema )
	{
		m_pSchema = pSchema;
		m_dRowBuf.Reset ( m_pSchema->GetDynamicSize() );
	}

	void Combine ( CSphMatch * pDst, const CSphMatch * pSrc, const CSphMatch * pGroup )
	{
		assert ( m_pSchema && pDst && pSrc && pGroup );
		assert ( pDst!=pGroup );
		m_pSchema->CloneMatch ( pDst, *pSrc );

		ARRAY_FOREACH ( i, m_dAttrsRaw )
		{
			pDst->SetAttr ( m_dAttrsRaw[i], pGroup->GetAttr ( m_dAttrsRaw[i] ) );
		}

		ARRAY_FOREACH ( i, m_dAttrsPtr )
		{
			assert ( !pDst->GetAttr ( m_dAttrsPtr[i] ) );
			const char * sSrc = (const char*)pGroup->GetAttr ( m_dAttrsPtr[i] );
			const char * sDst = NULL;
			if ( sSrc && *sSrc )
				sDst = strdup(sSrc);

			pDst->SetAttr ( m_dAttrsPtr[i], (SphAttr_t)sDst );
		}
	}

	void Clone ( CSphMatch * pOld, const CSphMatch * pNew )
	{
		assert ( m_pSchema && pOld && pNew );
		if ( pOld->m_pDynamic==NULL ) // no old match has no data to copy, just a fresh but old match
		{
			m_pSchema->CloneMatch ( pOld, *pNew );
			return;
		}

		memcpy ( m_dRowBuf.Begin(), pOld->m_pDynamic, sizeof(m_dRowBuf[0]) * m_dRowBuf.GetLength() );

		// don't let cloning operation to free old string data
		// as it will be copied back
		ARRAY_FOREACH ( i, m_dAttrsPtr )
			pOld->SetAttr ( m_dAttrsPtr[i], 0 );

		m_pSchema->CloneMatch ( pOld, *pNew );

		ARRAY_FOREACH ( i, m_dAttrsRaw )
			pOld->SetAttr ( m_dAttrsRaw[i], sphGetRowAttr ( m_dRowBuf.Begin(), m_dAttrsRaw[i] ) );
		ARRAY_FOREACH ( i, m_dAttrsPtr )
			pOld->SetAttr ( m_dAttrsPtr[i], sphGetRowAttr ( m_dRowBuf.Begin(), m_dAttrsPtr[i] ) );
	}
};


static void ExtractAggregates ( const CSphRsetSchema & tSchema, const CSphAttrLocator & tLocCount, const ESphSortKeyPart * m_pGroupSorterKeyparts, const CSphAttrLocator * m_pGroupSorterLocator,
								CSphVector<IAggrFunc *> & dAggregates, CSphVector<IAggrFunc *> & dAvgs, MatchCloner_t & tCloner )
{
	for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		bool bMagicAggr = IsGroupbyMagic ( tAttr.m_sName ) || sphIsSortStringInternal ( tAttr.m_sName.cstr() ); // magic legacy aggregates

		if ( tAttr.m_eAggrFunc==SPH_AGGR_NONE || bMagicAggr )
			continue;

		switch ( tAttr.m_eAggrFunc )
		{
		case SPH_AGGR_SUM:
			switch ( tAttr.m_eAttrType )
			{
			case SPH_ATTR_INTEGER:	dAggregates.Add ( new AggrSum_t<DWORD> ( tAttr.m_tLocator ) ); break;
			case SPH_ATTR_BIGINT:	dAggregates.Add ( new AggrSum_t<int64_t> ( tAttr.m_tLocator ) ); break;
			case SPH_ATTR_FLOAT:	dAggregates.Add ( new AggrSum_t<float> ( tAttr.m_tLocator ) ); break;
			default:				assert ( 0 && "internal error: unhandled aggregate type" ); break;
			}
			break;

		case SPH_AGGR_AVG:
			switch ( tAttr.m_eAttrType )
			{
			case SPH_ATTR_INTEGER:	dAggregates.Add ( new AggrAvg_t<DWORD> ( tAttr.m_tLocator, tLocCount ) ); break;
			case SPH_ATTR_BIGINT:	dAggregates.Add ( new AggrAvg_t<int64_t> ( tAttr.m_tLocator, tLocCount ) ); break;
			case SPH_ATTR_FLOAT:	dAggregates.Add ( new AggrAvg_t<float> ( tAttr.m_tLocator, tLocCount ) ); break;
			default:				assert ( 0 && "internal error: unhandled aggregate type" ); break;
			}
			// store avg to calculate these attributes prior to groups sort
			for ( int iState=0; iState<CSphMatchComparatorState::MAX_ATTRS; iState++ )
			{
				ESphSortKeyPart eKeypart = m_pGroupSorterKeyparts[iState];
				CSphAttrLocator tLoc = m_pGroupSorterLocator[iState];
				if ( ( eKeypart==SPH_KEYPART_INT || eKeypart==SPH_KEYPART_FLOAT )
					&& tLoc.m_bDynamic==tAttr.m_tLocator.m_bDynamic && tLoc.m_iBitOffset==tAttr.m_tLocator.m_iBitOffset
					&& tLoc.m_iBitCount==tAttr.m_tLocator.m_iBitCount )
				{
					dAvgs.Add ( dAggregates.Last() );
					break;
				}
			}
			break;

		case SPH_AGGR_MIN:
			switch ( tAttr.m_eAttrType )
			{
			case SPH_ATTR_INTEGER:	dAggregates.Add ( new AggrMin_t<DWORD> ( tAttr.m_tLocator ) ); break;
			case SPH_ATTR_BIGINT:	dAggregates.Add ( new AggrMin_t<int64_t> ( tAttr.m_tLocator ) ); break;
			case SPH_ATTR_FLOAT:	dAggregates.Add ( new AggrMin_t<float> ( tAttr.m_tLocator ) ); break;
			default:				assert ( 0 && "internal error: unhandled aggregate type" ); break;
			}
			break;

		case SPH_AGGR_MAX:
			switch ( tAttr.m_eAttrType )
			{
			case SPH_ATTR_INTEGER:	dAggregates.Add ( new AggrMax_t<DWORD> ( tAttr.m_tLocator ) ); break;
			case SPH_ATTR_BIGINT:	dAggregates.Add ( new AggrMax_t<int64_t> ( tAttr.m_tLocator ) ); break;
			case SPH_ATTR_FLOAT:	dAggregates.Add ( new AggrMax_t<float> ( tAttr.m_tLocator ) ); break;
			default:				assert ( 0 && "internal error: unhandled aggregate type" ); break;
			}
			break;

		case SPH_AGGR_CAT:
			dAggregates.Add ( new AggrConcat_t ( tAttr ) );
			break;

		default:
			assert ( 0 && "internal error: unhandled aggregate function" );
			break;
		}

		if ( tAttr.m_eAggrFunc==SPH_AGGR_CAT )
			tCloner.m_dAttrsPtr.Add ( tAttr.m_tLocator );
		else
			tCloner.m_dAttrsRaw.Add ( tAttr.m_tLocator );
	}
}


/// match sorter with k-buffering and group-by
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferGroupSorter : public CSphMatchQueueTraits, protected CSphGroupSorterSettings
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

	CSphVector<IAggrFunc *>		m_dAggregates;
	CSphVector<IAggrFunc *>		m_dAvgs;
	const ISphFilter *			m_pAggrFilter; ///< aggregate filter for matches on flatten
	MatchCloner_t				m_tPregroup;

	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

public:
	/// ctor
	CSphKBufferGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings ) // FIXME! make k configurable
		: CSphMatchQueueTraits ( pQuery->m_iMaxMatches*GROUPBY_FACTOR, true )
		, CSphGroupSorterSettings ( tSettings )
		, m_eGroupBy ( pQuery->m_eGroupFunc )
		, m_pGrouper ( tSettings.m_pGrouper )
		, m_hGroup2Match ( pQuery->m_iMaxMatches*GROUPBY_FACTOR )
		, m_iLimit ( pQuery->m_iMaxMatches )
		, m_bSortByDistinct ( false )
		, m_pComp ( pComp )
		, m_pAggrFilter ( tSettings.m_pAggrFilterTrait )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( DISTINCT==false || tSettings.m_tDistinctLoc.m_iBitOffset>=0 );

		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );
	}

	/// schema setup
	virtual void SetSchema ( CSphRsetSchema & tSchema )
	{
		m_tSchema = tSchema;
		m_tPregroup.SetSchema ( &m_tSchema );
		m_tPregroup.m_dAttrsRaw.Add ( m_tLocGroupby );
		m_tPregroup.m_dAttrsRaw.Add ( m_tLocCount );
		if_const ( DISTINCT )
		{
			m_tPregroup.m_dAttrsRaw.Add ( m_tLocDistinct );
		}
		ExtractAggregates ( m_tSchema, m_tLocCount, m_tGroupSorter.m_eKeypart, m_tGroupSorter.m_tLocator, m_dAggregates, m_dAvgs, m_tPregroup );
	}

	/// dtor
	~CSphKBufferGroupSorter ()
	{
		SafeDelete ( m_pComp );
		SafeDelete ( m_pGrouper );
		SafeDelete ( m_pAggrFilter );
		ARRAY_FOREACH ( i, m_dAggregates )
			SafeDelete ( m_dAggregates[i] );
	}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
	{
		return true;
	}

	virtual bool CanMulti () const
	{
		if ( m_pGrouper && !m_pGrouper->CanMulti() )
			return false;

		if ( HasString ( &m_tState ) )
			return false;

		if ( HasString ( &m_tGroupSorter ) )
			return false;

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
		return PushEx ( tEntry, uGroupKey, false, false );
	}

	/// add grouped entry to the queue
	virtual bool PushGrouped ( const CSphMatch & tEntry, bool )
	{
		return PushEx ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), true, false );
	}

	/// add entry to the queue
	virtual bool PushEx ( const CSphMatch & tEntry, const SphGroupKey_t uGroupKey, bool bGrouped, bool )
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = 0;
			m_dJustPopped.Resize(0);
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
			ARRAY_FOREACH ( i, m_dAggregates )
				m_dAggregates[i]->Update ( pMatch, &tEntry, bGrouped );

			// if new entry is more relevant, update from it
			if ( m_pComp->VirtualIsLess ( *pMatch, tEntry, m_tState ) )
			{
				if_const ( NOTIFICATIONS )
				{
					m_iJustPushed = tEntry.m_uDocID;
					m_dJustPopped.Add ( pMatch->m_uDocID );
				}

				// clone the low part of the match
				m_tPregroup.Clone ( pMatch, &tEntry );
			}
		}

		// submit actual distinct value in all cases
		if_const ( DISTINCT )
		{
			int iCount = 1;
			if ( bGrouped )
				iCount = (int)tEntry.GetAttr ( m_tLocDistinct );
			m_tUniq.Add ( SphGroupedValue_t ( uGroupKey, tEntry.GetAttr ( m_tDistinctLoc ), iCount ) ); // OPTIMIZE! use simpler locator here?
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
		m_tSchema.CloneMatch ( &tNew, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tNew.m_uDocID;

		if ( !bGrouped )
		{
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				tNew.SetAttr ( m_tLocDistinct, 0 );
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
	int Flatten ( CSphMatch * pTo, int iTag )
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

		// FIXME!!! we should provide up-to max_matches to output buffer
		const CSphMatch * pBegin = pTo;
		int iLen = GetLength ();
		for ( int i=0; i<iLen; ++i )
		{
			CSphMatch & tMatch = m_pData[i];
			ARRAY_FOREACH ( j, dAggrs )
				dAggrs[j]->Finalize ( &tMatch );

			// HAVING filtering
			if ( m_pAggrFilter && !m_pAggrFilter->Eval ( tMatch ) )
				continue;

			m_tSchema.CloneMatch ( pTo, tMatch );
			if ( iTag>=0 )
				pTo->m_iTag = iTag;

			pTo++;
		}

		m_iUsed = 0;
		m_iTotal = 0;

		m_hGroup2Match.Reset ();
		if_const ( DISTINCT )
			m_tUniq.Resize ( 0 );

		return ( pTo-pBegin );
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
				m_dJustPopped.Add ( m_pData[i].m_uDocID );
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

		// cut groups
		m_iUsed = iBound;
	}

	/// sort groups buffer
	void SortGroups ()
	{
		sphSort ( m_pData, m_iUsed, m_tGroupSorter, m_tGroupSorter );
	}

	virtual void Finalize ( ISphMatchProcessor & tProcessor, bool )
	{
		if ( !GetLength() )
			return;

		if ( m_iUsed>m_iLimit )
			CutWorst ( m_iLimit );

		// just evaluate in heap order
		CSphMatch * pCur = m_pData;
		const CSphMatch * pEnd = m_pData + m_iUsed;
		while ( pCur<pEnd )
			tProcessor.Process ( pCur++ );
	}
};


/// match sorter with k-buffering and N-best group-by
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferNGroupSorter : public CSphMatchQueueTraits, protected CSphGroupSorterSettings
{
protected:
	ESphGroupBy		m_eGroupBy;			///< group-by function
	CSphGrouper *	m_pGrouper;

	CSphFixedHash < CSphMatch *, SphGroupKey_t, IdentityHash_fn >	m_hGroup2Match;

protected:
	int				m_iLimit;		///< max matches to be retrieved
	int				m_iGLimit;		///< limit per one group
	CSphVector<int>	m_dGroupByList;	///< chains of equal matches from groups
	CSphVector<int>	m_dGroupsLen;	///< lengths of chains of equal matches from groups
	int				m_iHeads;		///< insertion point for head matches.
	int				m_iTails;		///< where to put tails of the subgroups
	SphGroupKey_t	m_uLastGroupKey;	///< helps to determine in pushEx whether the new subgroup started
#ifndef NDEBUG
	int				m_iruns;		///< helpers for conditional breakpoints on debug
	int				m_ipushed;
#endif
	CSphUniqounter	m_tUniq;
	bool			m_bSortByDistinct;

	GroupSorter_fn<COMPGROUP>	m_tGroupSorter;
	const ISphMatchComparator *	m_pComp;

	CSphVector<IAggrFunc *>		m_dAggregates;
	CSphVector<IAggrFunc *>		m_dAvgs;
	const ISphFilter *			m_pAggrFilter; ///< aggregate filter for matches on flatten
	MatchCloner_t				m_tPregroup;

	static const int			GROUPBY_FACTOR = 4;	///< allocate this times more storage when doing group-by (k, as in k-buffer)

protected:
	inline int AddMatch ( bool bTail=false )
	{
		// if we're full, let's cut off some worst groups
		if ( m_iUsed==m_iSize )
		{
			CutWorst ( m_iLimit * (int)(GROUPBY_FACTOR/2) );
			// don't return true value for tail this case,
			// since the context might be changed by the CutWorst!
			if ( bTail )
				return -1;
		}

		// do add
		assert ( m_iUsed<m_iSize );
		++m_iUsed;
		if ( bTail )
		{
			// trick!
			// m_iTails may be in 1-st of 2-nd subrange of 0..m_iSize..2*m_iSize.
			// 1-st case the next free elem is just the next a row (+m_iSize shift)
			// in 2-nd it is pulled from linked list
			assert ( m_iTails>=0 && m_iTails<2*m_iSize );
			int iRes;
			if ( m_iTails<m_iSize )
				iRes = m_iSize+m_iTails++;
			else
			{
				iRes = m_iTails;
				m_iTails = m_dGroupByList[m_iTails];
			}
			return iRes;
		} else
			return m_iHeads++;
	}

public:
	/// ctor
	CSphKBufferNGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings ) // FIXME! make k configurable
		: CSphMatchQueueTraits ( ((pQuery->m_iGroupbyLimit>1)?2:1)*pQuery->m_iMaxMatches*GROUPBY_FACTOR, true )
		, CSphGroupSorterSettings ( tSettings )
		, m_eGroupBy ( pQuery->m_eGroupFunc )
		, m_pGrouper ( tSettings.m_pGrouper )
		, m_hGroup2Match ( pQuery->m_iMaxMatches*GROUPBY_FACTOR*2 )
		, m_iLimit ( pQuery->m_iMaxMatches )
		, m_iGLimit ( pQuery->m_iGroupbyLimit )
		, m_iHeads ( 0 )
		, m_iTails ( 0 )
		, m_uLastGroupKey ( -1 )
		, m_bSortByDistinct ( false )
		, m_pComp ( pComp )
		, m_pAggrFilter ( tSettings.m_pAggrFilterTrait )
	{
		assert ( GROUPBY_FACTOR>1 );
		assert ( DISTINCT==false || tSettings.m_tDistinctLoc.m_iBitOffset>=0 );
		assert ( m_iGLimit > 1 );

		// trick! This case we allocated 2*m_iSize mem.
		// range 0..m_iSize used for 1-st matches of each subgroup (i.e., for heads)
		// range m_iSize+1..2*m_iSize used for the tails.
		m_dGroupByList.Resize ( m_iSize );
		m_dGroupsLen.Resize ( m_iSize );
		m_iSize >>= 1;

		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve ( m_iSize );

#ifndef NDEBUG
		m_iruns = 0;
		m_ipushed = 0;
#endif
	}

	// only for n-group
	virtual int			GetDataLength () const
	{
		return CSphMatchQueueTraits::GetDataLength()/2;
	}

	/// schema setup
	virtual void SetSchema ( CSphRsetSchema & tSchema )
	{
		m_tSchema = tSchema;

		m_tPregroup.SetSchema ( &m_tSchema );
		m_tPregroup.m_dAttrsRaw.Add ( m_tLocGroupby );
		m_tPregroup.m_dAttrsRaw.Add ( m_tLocCount );
		if_const ( DISTINCT )
		{
			m_tPregroup.m_dAttrsRaw.Add ( m_tLocDistinct );
		}
		ExtractAggregates ( m_tSchema, m_tLocCount, m_tGroupSorter.m_eKeypart, m_tGroupSorter.m_tLocator, m_dAggregates, m_dAvgs, m_tPregroup );
	}

	/// dtor
	~CSphKBufferNGroupSorter ()
	{
		SafeDelete ( m_pComp );
		SafeDelete ( m_pGrouper );
		SafeDelete ( m_pAggrFilter );
		ARRAY_FOREACH ( i, m_dAggregates )
			SafeDelete ( m_dAggregates[i] );
	}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
	{
		return true;
	}

	virtual bool CanMulti () const
	{
		if ( m_pGrouper && !m_pGrouper->CanMulti() )
			return false;

		if ( HasString ( &m_tState ) )
			return false;

		if ( HasString ( &m_tGroupSorter ) )
			return false;

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
		return PushEx ( tEntry, uGroupKey, false, false );
	}

	/// add grouped entry to the queue
	virtual bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet )
	{
		return PushEx ( tEntry, tEntry.GetAttr ( m_tLocGroupby ), true, bNewSet );
	}

	// insert a match into subgroup, or discard it
	// returns 0 if no place now (need to flush)
	// returns 1 if value was discarded or replaced other existing value
	// returns 2 if value was added.
	int InsertMatch ( int iPos, const CSphMatch & tEntry )
	{
		int iHead = iPos;
		int iPrev = -1;
		bool bDoAdd = m_dGroupsLen[iHead] < m_iGLimit;
		while ( iPos>=0 )
		{
			CSphMatch * pMatch = m_pData+iPos;
			if ( m_pComp->VirtualIsLess ( *pMatch, tEntry, m_tState ) ) // the tEntry is better than current *pMatch
			{
				int iPoint = iPos;
				if ( bDoAdd )
				{
					iPoint = AddMatch(true); // add to the tails (2-nd subrange)
					if ( iPoint<0 )
						return 0;
				} else
				{
					int iPreLast = iPrev;
					while ( m_dGroupByList[iPoint]>0 )
					{
						iPreLast = iPoint;
						iPoint = m_dGroupByList[iPoint];
					}
					m_dGroupByList[iPreLast]=-1;
					if ( iPos==iPoint ) // avoid cycle link to itself
						iPos = -1;
				}

				CSphMatch & tNew = m_pData [ iPoint ];
				if ( iPos==iHead ) // this is the first elem, need to copy groupby staff from it
				{
					// trick point! The first elem MUST live in the low half of the pool.
					// So, replacing the first is actually moving existing one to the last half,
					// then overwriting the one in the low half with the new value and link them
					m_tPregroup.Clone ( &tNew, pMatch );
					m_tPregroup.Clone ( pMatch, &tEntry );
					m_dGroupByList[iPoint]=m_dGroupByList[iPos];
					m_dGroupByList[iPos]=iPoint;
				} else // this is elem somewhere in the chain, just shift it.
				{
					m_tPregroup.Clone ( &tNew, &tEntry );
					m_dGroupByList[iPrev] = iPoint;
					m_dGroupByList[iPoint] = iPos;
				}
				break;
			}
			iPrev = iPos;
			iPos = m_dGroupByList[iPos];
		}

		if ( bDoAdd )
			++m_dGroupsLen[iHead];

		if ( iPos<0 && bDoAdd ) // this item is less than everything, but still appropriate
		{
			int iPoint = AddMatch(true); // add to the tails (2-nd subrange)
			if ( iPoint<0 )
				return false;
			CSphMatch & tNew = m_pData [ iPoint ];
			m_tPregroup.Clone ( &tNew, &tEntry );
			m_dGroupByList[iPrev] = iPoint;
			m_dGroupByList[iPoint] = iPos;
		}
		return bDoAdd ? 2 : 1;
	}

#ifndef NDEBUG
	void CheckIntegrity()
	{
#if PARANOID
		int iTotalLen = 0;
		for ( int i=0; i<m_iHeads; ++i )
		{
			int iLen = 0;
			int iCur = i;
			while ( iCur>=0 )
			{
				iCur = m_dGroupByList[iCur];
				++iLen;
			}
			assert ( m_dGroupsLen[i]==iLen );
			iTotalLen += iLen;
		}
		assert ( iTotalLen==m_iUsed );
#endif
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
			m_iJustPushed = 0;
			m_dJustPopped.Resize(0);
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
				ARRAY_FOREACH ( i, m_dAggregates )
					m_dAggregates[i]->Update ( pMatch, &tEntry, bGrouped );

			// if new entry is more relevant, update from it
			int iAdded = InsertMatch ( pMatch-m_pData, tEntry );
			if ( !iAdded )
				// was no insertion because cache cleaning. Recall myself
				PushEx ( tEntry, uGroupKey, bGrouped, bNewSet );
			else if ( iAdded>1 )
			{
				if ( bGrouped )
					return true;
				++m_iTotal;
			}
		}

		// submit actual distinct value in all cases
		if_const ( DISTINCT )
		{
			int iCount = 1;
			if ( bGrouped )
				iCount = (int)tEntry.GetAttr ( m_tLocDistinct );
			m_tUniq.Add ( SphGroupedValue_t ( uGroupKey, tEntry.GetAttr ( m_tDistinctLoc ), iCount ) ); // OPTIMIZE! use simpler locator here?
		}
		CHECKINTEGRITY();
		// it's a dupe anyway, so we shouldn't update total matches count
		if ( ppMatch )
			return false;

		// do add
		int iNew = AddMatch();
		CSphMatch & tNew = m_pData [ iNew ];
		m_tSchema.CloneMatch ( &tNew, tEntry );

		m_dGroupByList [ iNew ] = -1;
		m_dGroupsLen [ iNew ] = 1;

		if_const ( NOTIFICATIONS )
			m_iJustPushed = tNew.m_uDocID;

		if ( !bGrouped )
		{
			tNew.SetAttr ( m_tLocGroupby, uGroupKey );
			tNew.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				tNew.SetAttr ( m_tLocDistinct, 0 );
		} else
		{
			m_uLastGroupKey = uGroupKey;
			ARRAY_FOREACH ( i, m_dAggregates )
				m_dAggregates[i]->Ungroup ( &tNew );
		}

		m_hGroup2Match.Add ( &tNew, uGroupKey );
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
			ARRAY_FOREACH ( j, m_dAvgs )
			{
				if ( bGroup )
					m_dAvgs[j]->Finalize ( pMatch );
				else
					m_dAvgs[j]->Ungroup ( pMatch );
			}
			iMatch = m_dGroupByList [ iMatch ];
			if ( iMatch<0 )
				iMatch = ++iHeadMatch;
		}
	}

	// rebuild m_hGroup2Match to point to the subgroups (2-nd elem and further)
	// returns true if any such subroup found
	inline bool Hash2nd()
	{
		// let the hash points to the chains from 2-nd elem
		m_hGroup2Match.Reset();
		bool bHaveTails = false;
		int iHeads = m_iUsed;
		for ( int i=0; i<iHeads; i++ )
		{
			if ( m_dGroupByList[i]>0 )
			{
				m_hGroup2Match.Add ( m_pData+m_dGroupByList[i], m_pData[i].GetAttr ( m_tLocGroupby ) );
				bHaveTails = true;
				iHeads-=m_dGroupsLen[i]-1;
				m_dGroupsLen[m_dGroupByList[i]] = m_dGroupsLen[i];
			}
		}
		return bHaveTails;
	}


	/// store all entries into specified location in sorted order, and remove them from queue
	int Flatten ( CSphMatch * pTo, int iTag )
	{
		CountDistinct ();
		CalcAvg ( true );

		Hash2nd();

		SortGroups ();
		CSphVector<IAggrFunc *> dAggrs;
		if ( m_dAggregates.GetLength()!=m_dAvgs.GetLength() )
		{
			dAggrs = m_dAggregates;
			ARRAY_FOREACH ( i, m_dAvgs )
				dAggrs.RemoveValue ( m_dAvgs[i] );
		}

		const CSphMatch * pBegin = pTo;
		int iTotal = GetLength ();
		int iTopGroupMatch = 0;
		int iEntry = 0;

		while ( iEntry<iTotal )
		{
			CSphMatch * pMatch = m_pData + iTopGroupMatch;
			ARRAY_FOREACH ( j, dAggrs )
				dAggrs[j]->Finalize ( pMatch );

			bool bTopPassed = ( !m_pAggrFilter || m_pAggrFilter->Eval ( *pMatch ) );

			// copy top group match
			if ( bTopPassed )
			{
				m_tSchema.CloneMatch ( pTo, *pMatch );
				if ( iTag>=0 )
					pTo->m_iTag = iTag;
				pTo++;
			}
			iEntry++;
			iTopGroupMatch++;

			// now look for the next match.
			// In this specific case (2-nd, just after the head)
			// we have to look it in the hash, not in the linked list!
			CSphMatch ** ppMatch = m_hGroup2Match ( pMatch->GetAttr ( m_tLocGroupby ) );
			int iNext = ( ppMatch ? *ppMatch-m_pData : -1 );
			while ( iNext>=0 )
			{
				// copy rest group matches
				if ( bTopPassed )
				{
					m_tPregroup.Combine ( pTo, m_pData+iNext, pMatch );
					if ( iTag>=0 )
						pTo->m_iTag = iTag;
					pTo++;
				}
				iEntry++;
				iNext = m_dGroupByList[iNext];
			}
		}

		m_iHeads = m_iUsed = 0;
		m_iTotal = 0;

		memset ( m_pData+m_iSize, 0, m_iSize*sizeof(CSphMatch) );
		m_iTails = 0;

		m_hGroup2Match.Reset ();
		if_const ( DISTINCT )
			m_tUniq.Resize ( 0 );

		return ( pTo-pBegin );
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

	void CutWorstSubgroups ( int iBound )
	{
#ifndef NDEBUG
		++m_iruns;
#endif
		CHECKINTEGRITY();
		CalcAvg ( true );
		SortGroups ();
		CalcAvg ( false );

		CSphVector<SphGroupKey_t> dRemove;
		if_const ( DISTINCT )
			dRemove.Reserve ( m_iUsed-iBound );

		int iHeadMatch = 1;
		int iMatch = 0;
		int iHeadBound = -1;
		int iLastSize = 0;
		for ( int i=0; i<m_iUsed; ++i )
		{
			CSphMatch * pMatch = m_pData + iMatch;
			if ( i>=iBound )
			{
				if ( iHeadBound<0 )
					iHeadBound = ( iMatch+1==iHeadMatch ) ? iMatch : iHeadMatch;

				// do the staff with matches to cut
				if_const ( NOTIFICATIONS )
					m_dJustPopped.Add ( pMatch->m_uDocID );

				if_const ( DISTINCT )
					dRemove.Add ( pMatch->GetAttr ( m_tLocGroupby ) );

				if ( iMatch>=m_iSize )
				{
					Swap ( m_dGroupByList[iMatch], m_iTails );
					Swap ( m_iTails, iMatch );
					if ( iMatch<0 )
						iMatch = iHeadMatch++;
					continue;
				}
			}
			++iLastSize;
			if ( iMatch < m_iSize )
			{
				// next match have to be looked in the hash
				CSphMatch ** ppMatch = m_hGroup2Match ( pMatch->GetAttr ( m_tLocGroupby ) );
				if ( ppMatch )
				{
					int iChainLen = 1;
					if ( i==iBound-1 )
					{
						// edge case. Next match will be deleted.
						m_dGroupByList[iMatch] = -1;
					} else
					{
						m_dGroupByList[iMatch] = *ppMatch-m_pData;
						iChainLen = m_dGroupsLen[*ppMatch-m_pData];
					}
					m_dGroupsLen[iMatch] = iChainLen;
					if ( i+iChainLen<iBound ) // optimize: may jump over the chain
					{
						i+=iChainLen-1;
						iMatch = iHeadMatch++;
						iLastSize = 0;
					} else
						iMatch = *ppMatch-m_pData;
				} else
				{
					m_dGroupByList[iMatch] = -1;
					m_dGroupsLen[iMatch] = 1;
					iMatch = iHeadMatch++;
					iLastSize = 0;
				}
			} else
			{
				if ( i==iBound-1 )
				{
					int ifoo = m_dGroupByList[iMatch];
					// edge case. Next node will be deleted, so we need to terminate the pointer to it.
					m_dGroupByList[iMatch]=-1;
					m_dGroupsLen[iHeadMatch-1] = iLastSize;
					iMatch = ifoo;
				} else
					iMatch = m_dGroupByList[iMatch];
				if ( iMatch<0 )
				{
					iMatch = iHeadMatch++;
					iLastSize=0;
				}
			}
		}

		if_const ( DISTINCT )
		{
			if ( !m_bSortByDistinct )
				m_tUniq.Sort ();
			m_tUniq.Compact ( &dRemove[0], iBound );
		}

		// rehash
		m_hGroup2Match.Reset ();
		for ( int i=0; i<iHeadBound; i++ )
			m_hGroup2Match.Add ( m_pData+i, m_pData[i].GetAttr ( m_tLocGroupby ) );


#ifndef NDEBUG
		{
			int imanaged = 0;
			int i;
			for ( i=m_iTails; i>=m_iSize; i=m_dGroupByList[i] )
			{
				assert ( i!=m_dGroupByList[i] ); // cycle link to myself
				++imanaged;
				assert ( imanaged<=m_iSize ); // cycle links
			}

			imanaged +=iBound - iHeadBound;
			assert ( imanaged==i || imanaged+1==i ); // leaks
		}
#endif

		// cut groups
		m_iUsed = iBound;
		m_iHeads = iHeadBound;
		CHECKINTEGRITY();
	}

	/// cut worst N groups off the buffer tail
	void CutWorst ( int iBound )
	{
		// sort groups
		if ( m_bSortByDistinct )
			CountDistinct ();

		CHECKINTEGRITY();

		if ( Hash2nd() )
		{
			CutWorstSubgroups ( iBound );
			return;
		}

		CalcAvg ( true );
		SortGroups ();
		CalcAvg ( false );

		if_const ( NOTIFICATIONS )
		{
			for ( int i = iBound; i < m_iUsed; ++i )
				m_dJustPopped.Add ( m_pData[i].m_uDocID );
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

		// cut groups
		m_iHeads = m_iUsed = iBound;
	}

	/// sort groups buffer
	void SortGroups ()
	{
		sphSort ( m_pData, m_iHeads, m_tGroupSorter, m_tGroupSorter );
	}

	virtual void Finalize ( ISphMatchProcessor & tProcessor, bool )
	{
		if ( !GetLength() )
			return;

		if ( m_iUsed>m_iLimit )
			CutWorst ( m_iLimit );

		int iMatch = 0;
		int iNextHead = 0;
		for ( int i=0; i<m_iUsed; ++i )
		{
			tProcessor.Process ( m_pData + iMatch );
			if ( iMatch < m_iSize ) // this is head match
				iNextHead = iMatch + 1;

			iMatch = m_dGroupByList[iMatch];
			if ( iMatch<0 )
				iMatch = iNextHead;
		}
	}
};


/// match sorter with k-buffering and group-by for MVAs
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphKBufferMVAGroupSorter : public CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS >
{
protected:
	const DWORD *		m_pMva;		///< pointer to MVA pool for incoming matches
	CSphAttrLocator		m_tMvaLocator;
	bool				m_bMva64;

public:
	/// ctor
	CSphKBufferMVAGroupSorter ( const ISphMatchComparator * pComp, const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings )
		: CSphKBufferGroupSorter < COMPGROUP, DISTINCT, NOTIFICATIONS > ( pComp, pQuery, tSettings )
		, m_pMva ( NULL )
		, m_bMva64 ( tSettings.m_bMva64 )
	{
		this->m_pGrouper->GetLocator ( m_tMvaLocator );
	}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
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
				int64_t iMva = MVA_UPSIZE ( pValues );
				SphGroupKey_t uGroupkey = this->m_pGrouper->KeyFromValue ( iMva );
				bRes |= this->PushEx ( tEntry, uGroupkey, false, false );
			}

		} else
		{
			while ( iValues-- )
			{
				SphGroupKey_t uGroupkey = this->m_pGrouper->KeyFromValue ( *pValues++ );
				bRes |= this->PushEx ( tEntry, uGroupkey, false, false );
			}
		}
		return bRes;
	}

	/// add pre-grouped entry to the queue
	virtual bool PushGrouped ( const CSphMatch & tEntry, bool bNewSet )
	{
		// re-group it based on the group key
		// (first 'this' is for icc; second 'this' is for gcc)
		return this->PushEx ( tEntry, tEntry.GetAttr ( this->m_tLocGroupby ), true, bNewSet );
	}
};


/// implicit group-by sorter
template < typename COMPGROUP, bool DISTINCT, bool NOTIFICATIONS >
class CSphImplicitGroupSorter : public ISphMatchSorter, ISphNoncopyable, protected CSphGroupSorterSettings
{
protected:
	CSphMatch		m_tData;
	bool			m_bDataInitialized;

	CSphVector<SphUngroupedValue_t>	m_dUniq;

	CSphVector<IAggrFunc *>		m_dAggregates;
	const ISphFilter *			m_pAggrFilter;				///< aggregate filter for matches on flatten
	MatchCloner_t				m_tPregroup;

public:
	/// ctor
	CSphImplicitGroupSorter ( const ISphMatchComparator * DEBUGARG(pComp), const CSphQuery *, const CSphGroupSorterSettings & tSettings )
		: CSphGroupSorterSettings ( tSettings )
		, m_bDataInitialized ( false )
		, m_pAggrFilter ( tSettings.m_pAggrFilterTrait )
	{
		assert ( DISTINCT==false || tSettings.m_tDistinctLoc.m_iBitOffset>=0 );
		assert ( !pComp );

		if_const ( NOTIFICATIONS )
			m_dJustPopped.Reserve(1);

		m_dUniq.Reserve ( 16384 );
	}

	/// dtor
	~CSphImplicitGroupSorter ()
	{
		SafeDelete ( m_pAggrFilter );
		ARRAY_FOREACH ( i, m_dAggregates )
			SafeDelete ( m_dAggregates[i] );
	}

	/// schema setup
	virtual void SetSchema ( CSphRsetSchema & tSchema )
	{
		m_tSchema = tSchema;
		m_tPregroup.SetSchema ( &m_tSchema );
		m_tPregroup.m_dAttrsRaw.Add ( m_tLocGroupby );
		m_tPregroup.m_dAttrsRaw.Add ( m_tLocCount );
		if_const ( DISTINCT )
		{
			m_tPregroup.m_dAttrsRaw.Add ( m_tLocDistinct );
		}

		CSphVector<IAggrFunc *> dTmp;
		ESphSortKeyPart dTmpKeypart[CSphMatchComparatorState::MAX_ATTRS];
		CSphAttrLocator dTmpLocator[CSphMatchComparatorState::MAX_ATTRS];
		ExtractAggregates ( m_tSchema, m_tLocCount, dTmpKeypart, dTmpLocator, m_dAggregates, dTmp, m_tPregroup );
		assert ( !dTmp.GetLength() );
	}

	int GetDataLength () const
	{
		return 1;
	}

	bool UsesAttrs () const
	{
		return true;
	}

	/// check if this sorter does groupby
	virtual bool IsGroupby () const
	{
		return true;
	}

	virtual bool CanMulti () const
	{
		return true;
	}

	/// set string pool pointer (for string+groupby sorters)
	void SetStringPool ( const BYTE * )
	{
	}

	/// add entry to the queue
	virtual bool Push ( const CSphMatch & tEntry )
	{
		return PushEx ( tEntry, false );
	}

	/// add grouped entry to the queue. bNewSet indicates the beginning of resultset returned by an agent.
	virtual bool PushGrouped ( const CSphMatch & tEntry, bool )
	{
		return PushEx ( tEntry, true );
	}

	/// store all entries into specified location in sorted order, and remove them from queue
	virtual int Flatten ( CSphMatch * pTo, int iTag )
	{
		assert ( m_bDataInitialized );

		CountDistinct ();

		ARRAY_FOREACH ( j, m_dAggregates )
			m_dAggregates[j]->Finalize ( &m_tData );

		int iCopied = 0;
		if ( !m_pAggrFilter || m_pAggrFilter->Eval ( m_tData ) )
		{
			iCopied = 1;
			m_tSchema.CloneMatch ( pTo, m_tData );
			m_tSchema.FreeStringPtrs ( &m_tData );
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
	void Finalize ( ISphMatchProcessor & tProcessor, bool )
	{
		if ( !GetLength() )
			return;

		tProcessor.Process ( &m_tData );
	}

	/// get entries count
	int GetLength () const
	{
		return m_bDataInitialized ? 1 : 0;
	}

protected:
	/// add entry to the queue
	bool PushEx ( const CSphMatch & tEntry, bool bGrouped )
	{
		if_const ( NOTIFICATIONS )
		{
			m_iJustPushed = 0;
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
			ARRAY_FOREACH ( i, m_dAggregates )
				m_dAggregates[i]->Update ( &m_tData, &tEntry, bGrouped );

			// if new entry is more relevant, update from it
			if ( tEntry.m_uDocID<m_tData.m_uDocID )
			{
				if_const ( NOTIFICATIONS )
				{
					m_iJustPushed = tEntry.m_uDocID;
					m_dJustPopped.Add ( m_tData.m_uDocID );
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
			m_dUniq.Add ( SphUngroupedValue_t ( tEntry.GetAttr ( m_tDistinctLoc ), iCount ) ); // OPTIMIZE! use simpler locator here?
		}

		// it's a dupe anyway, so we shouldn't update total matches count
		if ( m_bDataInitialized )
			return false;

		// add first
		m_tSchema.CloneMatch ( &m_tData, tEntry );

		if_const ( NOTIFICATIONS )
			m_iJustPushed = m_tData.m_uDocID;

		if ( !bGrouped )
		{
			m_tData.SetAttr ( m_tLocGroupby, 1 ); // fake group number
			m_tData.SetAttr ( m_tLocCount, 1 );
			if_const ( DISTINCT )
				m_tData.SetAttr ( m_tLocDistinct, 0 );
		} else
		{
			ARRAY_FOREACH ( i, m_dAggregates )
				m_dAggregates[i]->Ungroup ( &m_tData );
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

		return a.m_uDocID > b.m_uDocID;
	}
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

		return a.m_uDocID > b.m_uDocID;
	}
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

		return a.m_uDocID > b.m_uDocID;
	}
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

		return a.m_uDocID > b.m_uDocID;
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
		return a.m_uDocID>b.m_uDocID;
	}
};

/////////////////////////////////////////////////////////////////////////////

#define SPH_TEST_PAIR(_aa,_bb,_idx ) \
	if ( (_aa)!=(_bb) ) \
		return ( (t.m_uAttrDesc >> (_idx)) & 1 ) ^ ( (_aa) > (_bb) );


#define SPH_TEST_KEYPART(_idx) \
	switch ( t.m_eKeypart[_idx] ) \
	{ \
		case SPH_KEYPART_ID:		SPH_TEST_PAIR ( a.m_uDocID, b.m_uDocID, _idx ); break; \
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
		return a.m_uDocID>b.m_uDocID;
	}
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
		return a.m_uDocID>b.m_uDocID;
	}
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
		return a.m_uDocID>b.m_uDocID;
	}
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
		return a.m_uDocID>b.m_uDocID;
	}
};

//////////////////////////////////////////////////////////////////////////
// SORT CLAUSE PARSER
//////////////////////////////////////////////////////////////////////////

static const int MAX_SORT_FIELDS = 5; // MUST be in sync with CSphMatchComparatorState::m_iAttr


class SortClauseTokenizer_t
{
protected:
	char * m_pCur;
	char * m_pMax;
	char * m_pBuf;

protected:
	char ToLower ( char c )
	{
		// 0..9, A..Z->a..z, _, a..z, @, .
		if ( ( c>='0' && c<='9' ) || ( c>='a' && c<='z' ) || c=='_' || c=='@' || c=='.' || c=='[' || c==']' || c=='\'' || c=='\"' || c=='(' || c==')' )
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
		case SPH_ATTR_JSON:
		case SPH_ATTR_JSON_FIELD:
		case SPH_ATTR_STRINGPTR: return SPH_KEYPART_STRINGPTR;
		default:				return SPH_KEYPART_INT;
	}
}


static const char g_sIntAttrPrefix[] = "@int_str2ptr_";


ESortClauseParseResult sphParseSortClause ( const CSphQuery * pQuery, const char * sClause, const ISphSchema & tSchema,
	ESphSortFunc & eFunc, CSphMatchComparatorState & tState, CSphString & sError )
{
	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
		tState.m_dAttrs[i] = -1;

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

		} else if ( !strcasecmp ( pTok, "@id" ) || !strcasecmp ( pTok, "id" ) )
		{
			tState.m_eKeypart[iField] = SPH_KEYPART_ID;

		} else
		{
			if ( !strcasecmp ( pTok, "@group" ) )
				pTok = "@groupby";

			// try to lookup plain attr in sorter schema
			int iAttr = tSchema.GetAttrIndex ( pTok );

			// try JSON attribute and use JSON attribute instead of JSON field
			if ( iAttr<0 || ( iAttr>=0 && tSchema.GetAttr ( iAttr ).m_eAttrType==SPH_ATTR_JSON_FIELD ) )
			{
				if ( iAttr>=0 )
				{
					// aliased SPH_ATTR_JSON_FIELD, reuse existing expression
					const CSphColumnInfo * pAttr = &tSchema.GetAttr(iAttr);
					if ( pAttr->m_pExpr.Ptr() )
						pAttr->m_pExpr->AddRef(); // SetupSortRemap uses refcounted pointer, but does not AddRef() itself, so help it
					tState.m_tSubExpr[iField] = pAttr->m_pExpr.Ptr();
					tState.m_tSubKeys[iField] = JsonKey_t ( pTok, strlen ( pTok ) );

				} else
				{
					CSphString sJsonCol, sJsonKey;
					if ( sphJsonNameSplit ( pTok, &sJsonCol, &sJsonKey ) )
					{
						iAttr = tSchema.GetAttrIndex ( sJsonCol.cstr() );
						if ( iAttr>=0 )
						{
							tState.m_tSubExpr[iField] = sphExprParse ( pTok, tSchema, NULL, NULL, sError, NULL );
							tState.m_tSubKeys[iField] = JsonKey_t ( pTok, strlen ( pTok ) );
						}
					}
				}
			}

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
					else if ( tItem.m_sExpr=="count(*)" )
						iAttr = tSchema.GetAttrIndex ( "@count" );
					break; // break in any case; because we did match the alias
				}
			}

			// try json conversion functions
			bool bIsFunc = false;
			ESphAttr eAttrType = SPH_ATTR_JSON_FIELD;
			if ( iAttr<0 )
			{
				ISphExpr * pExpr = sphExprParse ( pTok, tSchema, &eAttrType, NULL, sError, NULL );
				if ( pExpr )
				{
					tState.m_tSubExpr[iField] = pExpr;
					tState.m_tSubKeys[iField] = JsonKey_t ( pTok, strlen(pTok) );
					tState.m_tSubKeys[iField].m_uMask = 0;
					tState.m_tSubType[iField] = eAttrType;
					iAttr = 0; // will be remapped in SetupSortRemap
					bIsFunc = true;
				}
			}

			// try internal attributes received from agents
			if ( iAttr<0 )
			{
				CSphString sName;
				sName.SetSprintf( "%s%s", g_sIntAttrPrefix, pTok );
				iAttr = tSchema.GetAttrIndex ( sName.cstr() );
			}

			// epic fail
			if ( iAttr<0 )
			{
				sError.SetSprintf ( "sort-by attribute '%s' not found", pTok );
				return SORT_CLAUSE_ERROR;
			}

			const CSphColumnInfo & tCol = tSchema.GetAttr(iAttr);
			tState.m_eKeypart[iField] = Attr2Keypart ( bIsFunc ? eAttrType : tCol.m_eAttrType );
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
static ISphMatchSorter * sphCreateSorter3rd ( const ISphMatchComparator * pComp, const CSphQuery * pQuery,
	const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	BYTE uSelector = (bHasPackedFactors?1:0)
		+(tSettings.m_bDistinct?2:0)
		+(tSettings.m_bMVA?4:0)
		+(tSettings.m_bImplicit?8:0)
		+((pQuery->m_iGroupbyLimit>1)?16:0);
	switch ( uSelector )
	{
	case 0:
		return new CSphKBufferGroupSorter < COMPGROUP, false, false > ( pComp, pQuery, tSettings );
	case 1:
		return new CSphKBufferGroupSorter < COMPGROUP, false, true > ( pComp, pQuery, tSettings );
	case 2:
		return new CSphKBufferGroupSorter < COMPGROUP, true, false > ( pComp, pQuery, tSettings );
	case 3:
		return new CSphKBufferGroupSorter < COMPGROUP, true, true > ( pComp, pQuery, tSettings );
	case 4:
		return new CSphKBufferMVAGroupSorter < COMPGROUP, false, false > ( pComp, pQuery, tSettings );
	case 5:
		return new CSphKBufferMVAGroupSorter < COMPGROUP, false, true > ( pComp, pQuery, tSettings );
	case 6:
		return new CSphKBufferMVAGroupSorter < COMPGROUP, true, false > ( pComp, pQuery, tSettings);
	case 7:
		return new CSphKBufferMVAGroupSorter < COMPGROUP, true, true > ( pComp, pQuery, tSettings);
	case 8:
		return new CSphImplicitGroupSorter < COMPGROUP, false, false > ( pComp, pQuery, tSettings );
	case 9:
		return new CSphImplicitGroupSorter < COMPGROUP, false, true > ( pComp, pQuery, tSettings );
	case 10:
		return new CSphImplicitGroupSorter < COMPGROUP, true, false > ( pComp, pQuery, tSettings );
	case 11:
		return new CSphImplicitGroupSorter < COMPGROUP, true, true > ( pComp, pQuery, tSettings );
	case 16:
		return new CSphKBufferNGroupSorter < COMPGROUP, false, false > ( pComp, pQuery, tSettings );
	case 17:
		return new CSphKBufferNGroupSorter < COMPGROUP, false, true > ( pComp, pQuery, tSettings );
	case 18:
		return new CSphKBufferNGroupSorter < COMPGROUP, true, false > ( pComp, pQuery, tSettings );
	case 19:
		return new CSphKBufferNGroupSorter < COMPGROUP, true, true > ( pComp, pQuery, tSettings );
	}
	assert(0);
	return NULL;
}


static ISphMatchSorter * sphCreateSorter2nd ( ESphSortFunc eGroupFunc, const ISphMatchComparator * pComp,
	const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	switch ( eGroupFunc )
	{
		case FUNC_GENERIC2:		return sphCreateSorter3rd<MatchGeneric2_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC3:		return sphCreateSorter3rd<MatchGeneric3_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC4:		return sphCreateSorter3rd<MatchGeneric4_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_GENERIC5:		return sphCreateSorter3rd<MatchGeneric5_fn>	( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		case FUNC_EXPR:			return sphCreateSorter3rd<MatchExpr_fn>		( pComp, pQuery, tSettings, bHasPackedFactors ); break;
		default:				return NULL;
	}
}


static ISphMatchSorter * sphCreateSorter1st ( ESphSortFunc eMatchFunc, ESphSortFunc eGroupFunc,
	const CSphQuery * pQuery, const CSphGroupSorterSettings & tSettings, bool bHasPackedFactors )
{
	if ( tSettings.m_bImplicit )
		return sphCreateSorter2nd ( eGroupFunc, NULL, pQuery, tSettings, bHasPackedFactors );

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
		case FUNC_EXPR:			pComp = new MatchExpr_fn(); break; // only for non-bitfields, obviously
	}

	assert ( pComp );
	return sphCreateSorter2nd ( eGroupFunc, pComp, pQuery, tSettings, bHasPackedFactors );
}

//////////////////////////////////////////////////////////////////////////
// GEODIST
//////////////////////////////////////////////////////////////////////////

struct ExprGeodist_t : public ISphExpr
{
public:
						ExprGeodist_t () {}
	bool				Setup ( const CSphQuery * pQuery, const ISphSchema & tSchema, CSphString & sError );
	virtual float		Eval ( const CSphMatch & tMatch ) const;
	virtual void		Command ( ESphExprCommand eCmd, void * pArg );

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

void ExprGeodist_t::Command ( ESphExprCommand eCmd, void * pArg )
{
	if ( eCmd==SPH_EXPR_GET_DEPENDENT_COLS )
	{
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iLat );
		static_cast < CSphVector<int>* >(pArg)->Add ( m_iLon );
	}
}

//////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS (FACTORY AND FLATTENING)
//////////////////////////////////////////////////////////////////////////

static CSphGrouper * sphCreateGrouperString ( const CSphAttrLocator & tLoc, ESphCollation eCollation );
static CSphGrouper * sphCreateGrouperMulti ( const CSphVector<CSphAttrLocator> & dLocators, const CSphVector<ESphAttr> & dAttrTypes,
											const CSphVector<ISphExpr *> & dJsonKeys, ESphCollation eCollation );

static bool SetupGroupbySettings ( const CSphQuery * pQuery, const ISphSchema & tSchema,
								CSphGroupSorterSettings & tSettings, CSphVector<int> & dGroupColumns, CSphString & sError, bool bImplicit )
{
	tSettings.m_tDistinctLoc.m_iBitOffset = -1;

	if ( pQuery->m_sGroupBy.IsEmpty() && !bImplicit )
		return true;

	if ( pQuery->m_eGroupFunc==SPH_GROUPBY_ATTRPAIR )
	{
		sError.SetSprintf ( "SPH_GROUPBY_ATTRPAIR is not supported any more (just group on 'bigint' attribute)" );
		return false;
	}

	CSphString sJsonColumn;
	CSphString sJsonKey;
	if ( pQuery->m_eGroupFunc==SPH_GROUPBY_MULTIPLE )
	{
		CSphVector<CSphAttrLocator> dLocators;
		CSphVector<ESphAttr> dAttrTypes;
		CSphVector<ISphExpr *> dJsonKeys;

		CSphVector<CSphString> dGroupBy;
		const char * a = pQuery->m_sGroupBy.cstr();
		const char * b = a;
		while ( *a )
		{
			while ( *b && *b!=',' )
				b++;
			CSphString & sNew = dGroupBy.Add();
			sNew.SetBinary ( a, b-a );
			if ( *b==',' )
				b += 2;
			a = b;
		}
		dGroupBy.Uniq();

		ARRAY_FOREACH ( i, dGroupBy )
		{
			CSphString sJsonExpr;
			dJsonKeys.Add ( NULL );
			if ( sphJsonNameSplit ( dGroupBy[i].cstr(), &sJsonColumn, &sJsonKey ) )
			{
				sJsonExpr = dGroupBy[i];
				dGroupBy[i] = sJsonColumn;
			}

			const int iAttr = tSchema.GetAttrIndex ( dGroupBy[i].cstr() );
			if ( iAttr<0 )
			{
				sError.SetSprintf ( "group-by attribute '%s' not found", dGroupBy[i].cstr() );
				return false;
			}

			ESphAttr eType = tSchema.GetAttr ( iAttr ).m_eAttrType;
			if ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET )
			{
				sError.SetSprintf ( "MVA values can't be used in multiple group-by" );
				return false;
			} else if ( sJsonExpr.IsEmpty() && eType==SPH_ATTR_JSON )
			{
				sError.SetSprintf ( "JSON blob can't be used in multiple group-by" );
				return false;
			}

			dLocators.Add ( tSchema.GetAttr ( iAttr ).m_tLocator );
			dAttrTypes.Add ( eType );
			dGroupColumns.Add ( iAttr );

			if ( !sJsonExpr.IsEmpty() )
				dJsonKeys.Last() = sphExprParse ( sJsonExpr.cstr(), tSchema, NULL, NULL, sError, NULL );
		}

		tSettings.m_pGrouper = sphCreateGrouperMulti ( dLocators, dAttrTypes, dJsonKeys, pQuery->m_eCollation );

	} else if ( sphJsonNameSplit ( pQuery->m_sGroupBy.cstr(), &sJsonColumn, &sJsonKey ) )
	{
		const int iAttr = tSchema.GetAttrIndex ( sJsonColumn.cstr() );
		if ( iAttr<0 )
		{
			sError.SetSprintf ( "groupby: no such attribute '%s'", sJsonColumn.cstr() );
			return false;
		}

		if ( tSchema.GetAttr(iAttr).m_eAttrType!=SPH_ATTR_JSON )
		{
			sError.SetSprintf ( "groupby: attribute '%s' does not have subfields (must be sql_attr_json)", sJsonColumn.cstr() );
			return false;
		}

		if ( pQuery->m_eGroupFunc!=SPH_GROUPBY_ATTR )
		{
			sError.SetSprintf ( "groupby: legacy groupby modes are not supported on JSON attributes" );
			return false;
		}

		dGroupColumns.Add ( iAttr );

		// FIXME! handle collations here?
		ISphExpr * pExpr = sphExprParse ( pQuery->m_sGroupBy.cstr(), tSchema, NULL, NULL, sError, NULL );
		tSettings.m_pGrouper = new CSphGrouperJson ( tSchema.GetAttr(iAttr).m_tLocator, pExpr );

	} else if ( bImplicit )
	{
		tSettings.m_bImplicit = true;

	} else
	{
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
				if ( eType==SPH_ATTR_JSON )
				{
					sError.SetSprintf ( "invalid group-by value - JSON blob" );
					return false;
				} else if ( eType==SPH_ATTR_STRING )
					tSettings.m_pGrouper = sphCreateGrouperString ( tLoc, pQuery->m_eCollation );
				else
					tSettings.m_pGrouper = new CSphGrouperAttr ( tLoc );
				break;
			default:
				sError.SetSprintf ( "invalid group-by mode (mode=%d)", pQuery->m_eGroupFunc );
				return false;
		}

		tSettings.m_bMVA = ( eType==SPH_ATTR_UINT32SET || eType==SPH_ATTR_INT64SET );
		tSettings.m_bMva64 = ( eType==SPH_ATTR_INT64SET );
		dGroupColumns.Add ( iGroupBy );
	}

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

// move expressions used in ORDER BY or WITHIN GROUP ORDER BY to presort phase
static bool FixupDependency ( ISphSchema & tSchema, const int * pAttrs, int iAttrCount )
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
			tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
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

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd==SPH_EXPR_SET_STRING_POOL )
			m_pStrings = (const BYTE*)pArg;
	}
};


// expression that transform string pool base + offset -> ptr
struct ExprSortJson2StringPtr_c : public ISphExpr
{
	const BYTE *			m_pStrings; ///< string pool; base for offset of string attributes
	const CSphAttrLocator	m_tJsonCol; ///< JSON attribute to fix
	CSphRefcountedPtr<ISphExpr>	m_pExpr;

	ExprSortJson2StringPtr_c ( const CSphAttrLocator & tLocator, ISphExpr * pExpr )
		: m_pStrings ( NULL )
		, m_tJsonCol ( tLocator )
		, m_pExpr ( pExpr )
	{}

	virtual bool IsStringPtr () const { return true; }

	virtual float Eval ( const CSphMatch & ) const { assert ( 0 ); return 0.0f; }

	virtual int StringEval ( const CSphMatch & tMatch, const BYTE ** ppStr ) const
	{
		if ( !m_pStrings || !m_pExpr )
		{
			*ppStr = NULL;
			return 0;
		}

		uint64_t uValue = m_pExpr->Int64Eval ( tMatch );
		const BYTE * pVal = m_pStrings + ( uValue & 0xffffffff );
		ESphJsonType eJson = (ESphJsonType)( uValue >> 32 );
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

	virtual void Command ( ESphExprCommand eCmd, void * pArg )
	{
		if ( eCmd==SPH_EXPR_SET_STRING_POOL )
		{
			m_pStrings = (const BYTE*)pArg;
			if ( m_pExpr.Ptr() )
				m_pExpr->Command ( eCmd, pArg );
		}
	}
};



bool sphIsSortStringInternal ( const char * sColumnName )
{
	assert ( sColumnName );
	return ( strncmp ( sColumnName, g_sIntAttrPrefix, sizeof(g_sIntAttrPrefix)-1 )==0 );
}


// only STRING ( static packed ) and JSON fields mush be remapped
static void SetupSortRemap ( CSphRsetSchema & tSorterSchema, CSphMatchComparatorState & tState )
{
#ifndef NDEBUG
	int iColWasCount = tSorterSchema.GetAttrsCount();
#endif
	for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
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
		if ( iRemap==-1 )
		{
			CSphColumnInfo tRemapCol ( sRemapCol.cstr(), bIsJson ? SPH_ATTR_STRINGPTR : SPH_ATTR_BIGINT );
			tRemapCol.m_eStage = SPH_EVAL_PRESORT;
			if ( bIsJson )
				tRemapCol.m_pExpr = bIsFunc ? tState.m_tSubExpr[i] : new ExprSortJson2StringPtr_c ( tState.m_tLocator[i], tState.m_tSubExpr[i] );
			else
				tRemapCol.m_pExpr = new ExprSortStringAttrFixup_c ( tState.m_tLocator[i] );

			if ( bIsFunc )
			{
				tRemapCol.m_eAttrType = tState.m_tSubType[i];
				tState.m_eKeypart[i] = Attr2Keypart ( tState.m_tSubType[i] );
			}

			iRemap = tSorterSchema.GetAttrsCount();
			tSorterSchema.AddDynamicAttr ( tRemapCol );
		}
		tState.m_tLocator[i] = tSorterSchema.GetAttr ( iRemap ).m_tLocator;
	}
}


bool sphSortGetStringRemap ( const ISphSchema & tSorterSchema, const ISphSchema & tIndexSchema,
	CSphVector<SphStringSorterRemap_t> & dAttrs )
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

int CollateBinary ( const BYTE * pStr1, const BYTE * pStr2, bool bPacked )
{
	if ( bPacked )
	{
		int iLen1 = sphUnpackStr ( pStr1, &pStr1 );
		int iLen2 = sphUnpackStr ( pStr2, &pStr2 );
		int iRes = memcmp ( (const char *)pStr1, (const char *)pStr2, Min ( iLen1, iLen2 ) );
		return iRes ? iRes : ( iLen1-iLen2 );
	} else
	{
		return strcmp ( (const char *)pStr1, (const char *)pStr2 );
	}
}

///////////////////////////////
// LIBC_CI, LIBC_CS COLLATIONS
///////////////////////////////

/// libc_ci, wrapper for strcasecmp
int CollateLibcCI ( const BYTE * pStr1, const BYTE * pStr2, bool bPacked )
{
	if ( bPacked )
	{
		int iLen1 = sphUnpackStr ( pStr1, &pStr1 );
		int iLen2 = sphUnpackStr ( pStr2, &pStr2 );
		int iRes = strncasecmp ( (const char *)pStr1, (const char *)pStr2, Min ( iLen1, iLen2 ) );
		return iRes ? iRes : ( iLen1-iLen2 );
	} else
	{
		return strcasecmp ( (const char *)pStr1, (const char *)pStr2 );
	}
}


/// libc_cs, wrapper for strcoll
int CollateLibcCS ( const BYTE * pStr1, const BYTE * pStr2, bool bPacked )
{
	#define COLLATE_STACK_BUFFER 1024

	if ( bPacked )
	{
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
	} else
	{
		return strcoll ( (const char *)pStr1, (const char *)pStr2 );
	}
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
int CollateUtf8GeneralCI ( const BYTE * pArg1, const BYTE * pArg2, bool bPacked )
{
	const BYTE * pStr1 = pArg1;
	const BYTE * pStr2 = pArg2;
	const BYTE * pMax1 = NULL;
	const BYTE * pMax2 = NULL;
	if ( bPacked )
	{
		int iLen1 = sphUnpackStr ( pStr1, (const BYTE**)&pStr1 );
		int iLen2 = sphUnpackStr ( pStr2, (const BYTE**)&pStr2 );
		pMax1 = pStr1 + iLen1;
		pMax2 = pStr2 + iLen2;
	}

	while ( ( bPacked && pStr1<pMax1 && pStr2<pMax2 ) || ( !bPacked && *pStr1 && *pStr2 ) )
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

	if ( bPacked )
	{
		if ( pStr1>=pMax1 && pStr2>=pMax2 )
			return 0;
		return ( pStr1<pMax1 ) ? 1 : -1;
	} else
	{
		if ( !*pStr1 && !*pStr2 )
			return 0;
		return ( *pStr1 ? 1 : -1 );
	}
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

	uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED ) const
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

		uint64_t uAcc = sphFNV64 ( pDst, iDstLen, uPrev );

		return uAcc;
	}
};


class LibcCIHash_fn
{
public:
	uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED ) const
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
	uint64_t Hash ( const BYTE * pStr, int iLen, uint64_t uPrev=SPH_FNV64_SEED ) const
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
	if ( eCollation==SPH_COLLATION_UTF8_GENERAL_CI )
		return new CSphGrouperString<Utf8CIHash_fn> ( tLoc );
	else if ( eCollation==SPH_COLLATION_LIBC_CI )
		return new CSphGrouperString<LibcCIHash_fn> ( tLoc );
	else if ( eCollation==SPH_COLLATION_LIBC_CS )
		return new CSphGrouperString<LibcCSHash_fn> ( tLoc );
	else
		return new CSphGrouperString<BinaryHash_fn> ( tLoc );
}

CSphGrouper * sphCreateGrouperMulti ( const CSphVector<CSphAttrLocator> & dLocators, const CSphVector<ESphAttr> & dAttrTypes,
									const CSphVector<ISphExpr *> & dJsonKeys, ESphCollation eCollation )
{
	if ( eCollation==SPH_COLLATION_UTF8_GENERAL_CI )
		return new CSphGrouperMulti<Utf8CIHash_fn> ( dLocators, dAttrTypes, dJsonKeys );
	else if ( eCollation==SPH_COLLATION_LIBC_CI )
		return new CSphGrouperMulti<LibcCIHash_fn> ( dLocators, dAttrTypes, dJsonKeys );
	else if ( eCollation==SPH_COLLATION_LIBC_CS )
		return new CSphGrouperMulti<LibcCSHash_fn> ( dLocators, dAttrTypes, dJsonKeys );
	else
		return new CSphGrouperMulti<BinaryHash_fn> ( dLocators, dAttrTypes, dJsonKeys );
}


/////////////////////////
// SORTING QUEUE FACTORY
/////////////////////////

template < typename COMP >
static ISphMatchSorter * CreatePlainSorter ( bool bKbuffer, int iMaxMatches, bool bUsesAttrs, bool bFactors )
{
	if ( bKbuffer )
	{
		if ( bFactors )
			return new CSphKbufferMatchQueue<COMP, true> ( iMaxMatches, bUsesAttrs );
		else
			return new CSphKbufferMatchQueue<COMP, false> ( iMaxMatches, bUsesAttrs );
	} else
	{
		if ( bFactors )
			return new CSphMatchQueue<COMP, true> ( iMaxMatches, bUsesAttrs );
		else
			return new CSphMatchQueue<COMP, false> ( iMaxMatches, bUsesAttrs );
	}
}


static ISphMatchSorter * CreatePlainSorter ( ESphSortFunc eMatchFunc, bool bKbuffer, int iMaxMatches, bool bUsesAttrs, bool bFactors )
{
	switch ( eMatchFunc )
	{
		case FUNC_REL_DESC:		return CreatePlainSorter<MatchRelevanceLt_fn>	( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		case FUNC_ATTR_DESC:	return CreatePlainSorter<MatchAttrLt_fn>		( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		case FUNC_ATTR_ASC:		return CreatePlainSorter<MatchAttrGt_fn>		( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		case FUNC_TIMESEGS:		return CreatePlainSorter<MatchTimeSegments_fn>	( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		case FUNC_GENERIC2:		return CreatePlainSorter<MatchGeneric2_fn>		( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		case FUNC_GENERIC3:		return CreatePlainSorter<MatchGeneric3_fn>		( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		case FUNC_GENERIC4:		return CreatePlainSorter<MatchGeneric4_fn>		( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		case FUNC_GENERIC5:		return CreatePlainSorter<MatchGeneric5_fn>		( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		case FUNC_EXPR:			return CreatePlainSorter<MatchExpr_fn>			( bKbuffer, iMaxMatches, bUsesAttrs, bFactors ); break;
		default:				return NULL;
	}
}


static void ExtraAddSortkeys ( CSphSchema * pExtra, const ISphSchema & tSorterSchema, const int * dAttrs )
{
	if ( pExtra )
		for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
			if ( dAttrs[i]>=0 )
				pExtra->AddAttr ( tSorterSchema.GetAttr ( dAttrs[i] ), true );
}


ISphMatchSorter * sphCreateQueue ( SphQueueSettings_t & tQueue )
{
	// prepare for descent
	ISphMatchSorter * pTop = NULL;
	CSphMatchComparatorState tStateMatch, tStateGroup;

	// short-cuts
	const CSphQuery * pQuery = &tQueue.m_tQuery;
	const ISphSchema & tSchema = tQueue.m_tSchema;
	CSphString & sError = tQueue.m_sError;
	CSphQueryProfile * pProfiler = tQueue.m_pProfiler;
	CSphSchema * pExtra = tQueue.m_pExtra;

	sError = "";
	bool bHasZonespanlist = false;
	bool bNeedZonespanlist = false;
	bool bNeedPackedFactors = false;

	///////////////////////////////////////
	// build incoming and outgoing schemas
	///////////////////////////////////////

	// sorter schema
	// adds computed expressions and groupby stuff on top of the original index schema
	CSphRsetSchema tSorterSchema;
	tSorterSchema = tSchema;

	CSphVector<uint64_t> dQueryAttrs;

	// we need this to perform a sanity check
	bool bHasGroupByExpr = false;

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

		CSphColumnInfo tCol;
		tCol = tSorterSchema.GetAttr ( iIndex );
		tCol.m_eStage = SPH_EVAL_OVERRIDE;
		tSorterSchema.AddDynamicAttr ( tCol );
		if ( pExtra )
			pExtra->AddAttr ( tCol, true );
		tSorterSchema.RemoveStaticAttr ( iIndex );

		dQueryAttrs.Add ( sphFNV64 ( tCol.m_sName.cstr() ) );
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
		tSorterSchema.AddDynamicAttr ( tCol );
		if ( pExtra )
			pExtra->AddAttr ( tCol, true );

		dQueryAttrs.Add ( sphFNV64 ( tCol.m_sName.cstr() ) );
	}

	// setup @expr
	if ( pQuery->m_eSort==SPH_SORT_EXPR && tSorterSchema.GetAttrIndex ( "@expr" )<0 )
	{
		CSphColumnInfo tCol ( "@expr", SPH_ATTR_FLOAT ); // enforce float type for backwards compatibility (ie. too lazy to fix those tests right now)
		tCol.m_pExpr = sphExprParse ( pQuery->m_sSortBy.cstr(), tSorterSchema, NULL, NULL, sError, pProfiler, NULL, &bHasZonespanlist );
		bNeedZonespanlist |= bHasZonespanlist;
		if ( !tCol.m_pExpr )
			return NULL;
		tCol.m_eStage = SPH_EVAL_PRESORT;
		tSorterSchema.AddDynamicAttr ( tCol );

		dQueryAttrs.Add ( sphFNV64 ( tCol.m_sName.cstr() ) );
	}

	// expressions from select items
	bool bHasCount = false;

	if ( tQueue.m_bComputeItems )
		ARRAY_FOREACH ( iItem, pQuery->m_dItems )
	{
		const CSphQueryItem & tItem = pQuery->m_dItems[iItem];
		const CSphString & sExpr = tItem.m_sExpr;
		bool bIsCount = IsCount(sExpr);
		bHasCount |= bIsCount;

		if ( sExpr=="*" )
		{
			for ( int i=0; i<tSchema.GetAttrsCount(); i++ )
				dQueryAttrs.Add ( sphFNV64 ( tSchema.GetAttr(i).m_sName.cstr() ) );
		}

		// for now, just always pass "plain" attrs from index to sorter; they will be filtered on searchd level
		int iAttrIdx = tSchema.GetAttrIndex ( sExpr.cstr() );
		bool bPlainAttr = ( ( sExpr=="*" || ( iAttrIdx>=0 && tItem.m_eAggrFunc==SPH_AGGR_NONE ) ) &&
							( tItem.m_sAlias.IsEmpty() || tItem.m_sAlias==tItem.m_sExpr ) );
		if ( iAttrIdx>=0 )
		{
			ESphAttr eAttr = tSchema.GetAttr ( iAttrIdx ).m_eAttrType;
			if ( eAttr==SPH_ATTR_STRING || eAttr==SPH_ATTR_UINT32SET || eAttr==SPH_ATTR_INT64SET )
			{
				if ( tItem.m_eAggrFunc!=SPH_AGGR_NONE )
				{
					sError.SetSprintf ( "can not aggregate non-scalar attribute '%s'", tItem.m_sExpr.cstr() );
					return NULL;
				}

				if ( !bPlainAttr )
				{
					bPlainAttr = true;
					for ( int i=0; i<iItem && bPlainAttr; i++ )
						if ( sExpr==pQuery->m_dItems[i].m_sAlias )
							bPlainAttr = false;
				}
			}
		}

		if ( bPlainAttr || IsGroupby ( sExpr ) || bIsCount )
		{
			bHasGroupByExpr = IsGroupby ( sExpr );
			continue;
		}

		// not an attribute? must be an expression, and must be aliased by query parser
		assert ( !tItem.m_sAlias.IsEmpty() );

		// tricky part
		// we might be fed with precomputed matches, but it's all or nothing
		// the incoming match either does not have anything computed, or it has everything
		int iSorterAttr = tSorterSchema.GetAttrIndex ( tItem.m_sAlias.cstr() );
		if ( iSorterAttr>=0 )
		{
			if ( dQueryAttrs.Contains ( sphFNV64 ( tItem.m_sAlias.cstr() ) ) )
			{
				sError.SetSprintf ( "alias '%s' must be unique (conflicts with another alias)", tItem.m_sAlias.cstr() );
				return NULL;
			} else
			{
				tSorterSchema.RemoveStaticAttr ( iSorterAttr );
			}
		}

		// a new and shiny expression, lets parse
		CSphColumnInfo tExprCol ( tItem.m_sAlias.cstr(), SPH_ATTR_NONE );
		bool bHasPackedFactors = false;

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
			tExprCol.m_pExpr = sphExprParse ( sExpr2.cstr(), tSorterSchema, &tExprCol.m_eAttrType,
				&tExprCol.m_bWeight, sError, pProfiler, tQueue.m_pHook, &bHasZonespanlist, &bHasPackedFactors, &tExprCol.m_eStage );
		} else
		{
			tExprCol.m_pExpr = sphExprParse ( sExpr.cstr(), tSorterSchema, &tExprCol.m_eAttrType,
				&tExprCol.m_bWeight, sError, pProfiler, tQueue.m_pHook, &bHasZonespanlist, &bHasPackedFactors, &tExprCol.m_eStage );
		}

		bNeedPackedFactors |= bHasPackedFactors;
		bNeedZonespanlist |= bHasZonespanlist;
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

		// force explicit type conversion for JSON attributes
		if ( tExprCol.m_eAggrFunc!=SPH_AGGR_NONE && tExprCol.m_eAttrType==SPH_ATTR_JSON_FIELD )
		{
			sError.SetSprintf ( "ambiguous attribute type '%s', use INTEGER(), BIGINT() or DOUBLE() conversion functions", tItem.m_sExpr.cstr() );
			return NULL;
		}

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
			ARRAY_FOREACH ( i, pQuery->m_dFilters )
				if ( pQuery->m_dFilters[i].m_sAttrName==tExprCol.m_sName )
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
					const CSphColumnInfo & tCol = tSorterSchema.GetAttr ( dCur[j] );
					if ( tCol.m_bWeight )
					{
						tExprCol.m_eStage = SPH_EVAL_PRESORT;
						tExprCol.m_bWeight = true;
					}
					// handle chains of dependencies (e.g. SELECT 1+attr f1, f1-1 f2 ... WHERE f2>5)
					if ( tCol.m_pExpr.Ptr() )
					{
						tCol.m_pExpr->Command ( SPH_EXPR_GET_DEPENDENT_COLS, &dCur );
					}
				}
				dCur.Uniq();

				ARRAY_FOREACH ( j, dCur )
				{
					CSphColumnInfo & tDep = const_cast < CSphColumnInfo & > ( tSorterSchema.GetAttr ( dCur[j] ) );
					if ( tDep.m_eStage>tExprCol.m_eStage )
						tDep.m_eStage = tExprCol.m_eStage;
				}
				break;
			}

			// add it!
			// NOTE, "final" stage might need to be fixed up later
			// we'll do that when parsing sorting clause
			tSorterSchema.AddDynamicAttr ( tExprCol );
		} else // some aggregate
		{
			tExprCol.m_eStage = SPH_EVAL_PRESORT; // sorter expects computed expression
			tSorterSchema.AddDynamicAttr ( tExprCol );
			if ( pExtra )
				pExtra->AddAttr ( tExprCol, true );
		}

		dQueryAttrs.Add ( sphFNV64 ( (const BYTE *)tExprCol.m_sName.cstr() ) );
	}

	////////////////////////////////////////////
	// setup groupby settings, create shortcuts
	////////////////////////////////////////////

	CSphVector<int> dGroupColumns;
	CSphGroupSorterSettings tSettings;
	bool bImplicit = false;

	if ( pQuery->m_sGroupBy.IsEmpty() )
		ARRAY_FOREACH_COND ( i, pQuery->m_dItems, !bImplicit )
		{
			const CSphQueryItem & t = pQuery->m_dItems[i];
			bImplicit = ( t.m_eAggrFunc!=SPH_AGGR_NONE ) || t.m_sExpr=="count(*)" || t.m_sExpr=="@distinct";
		}

	if ( !SetupGroupbySettings ( pQuery, tSorterSchema, tSettings, dGroupColumns, sError, bImplicit ) )
		return NULL;

	const bool bGotGroupby = !pQuery->m_sGroupBy.IsEmpty() || tSettings.m_bImplicit; // or else, check in SetupGroupbySettings() would already fail
	const bool bGotDistinct = ( tSettings.m_tDistinctLoc.m_iBitOffset>=0 );

	if ( bHasGroupByExpr && !bGotGroupby )
	{
		sError = "GROUPBY() is allowed only in GROUP BY queries";
		return NULL;
	}

	// check for HAVING constrains
	if ( tQueue.m_pAggrFilter && !tQueue.m_pAggrFilter->m_sAttrName.IsEmpty() && !bGotGroupby )
	{
		sError.SetSprintf ( "can not use HAVING without group by" );
		return NULL;
	}

	// now lets add @groupby etc if needed
	if ( bGotGroupby && tSorterSchema.GetAttrIndex ( "@groupby" )<0 )
	{
		ESphAttr eGroupByResult = ( !tSettings.m_bImplicit ) ? tSettings.m_pGrouper->GetResultType() : SPH_ATTR_INTEGER; // implicit do not have grouper
		if ( tSettings.m_bMva64 )
			eGroupByResult = SPH_ATTR_BIGINT;
		CSphColumnInfo tGroupby ( "@groupby", eGroupByResult );
		CSphColumnInfo tCount ( "@count", SPH_ATTR_INTEGER );
		CSphColumnInfo tDistinct ( "@distinct", SPH_ATTR_INTEGER );

		tGroupby.m_eStage = SPH_EVAL_SORTER;
		tCount.m_eStage = SPH_EVAL_SORTER;
		tDistinct.m_eStage = SPH_EVAL_SORTER;

		tSorterSchema.AddDynamicAttr ( tGroupby );
		tSorterSchema.AddDynamicAttr ( tCount );
		if ( pExtra )
		{
			pExtra->AddAttr ( tGroupby, true );
			pExtra->AddAttr ( tCount, true );
		}
		if ( bGotDistinct )
		{
			tSorterSchema.AddDynamicAttr ( tDistinct );
			if ( pExtra )
				pExtra->AddAttr ( tDistinct, true );
		}
	}

#define LOC_CHECK(_cond,_msg) if (!(_cond)) { sError = "invalid schema: " _msg; return NULL; }

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
		ESortClauseParseResult eRes = sphParseSortClause ( pQuery, pQuery->m_sSortBy.cstr(),
			tSorterSchema, eMatchFunc, tStateMatch, sError );

		if ( eRes==SORT_CLAUSE_ERROR )
			return NULL;

		if ( eRes==SORT_CLAUSE_RANDOM )
			bRandomize = true;

		ExtraAddSortkeys ( pExtra, tSorterSchema, tStateMatch.m_dAttrs );

		bUsesAttrs = FixupDependency ( tSorterSchema, tStateMatch.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );
		if ( !bUsesAttrs )
		{
			for ( int i=0; i<CSphMatchComparatorState::MAX_ATTRS; i++ )
			{
				ESphSortKeyPart ePart = tStateMatch.m_eKeypart[i];
				if ( ePart==SPH_KEYPART_INT || ePart==SPH_KEYPART_FLOAT || ePart==SPH_KEYPART_STRING || ePart==SPH_KEYPART_STRINGPTR )
					bUsesAttrs = true;
			}
		}
		SetupSortRemap ( tSorterSchema, tStateMatch );

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
			tStateMatch.m_dAttrs[0] = iSortAttr;
			SetupSortRemap ( tSorterSchema, tStateMatch );
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
		ESortClauseParseResult eRes = sphParseSortClause ( pQuery, pQuery->m_sGroupSortBy.cstr(),
			tSorterSchema, eGroupFunc, tStateGroup, sError );

		if ( eRes==SORT_CLAUSE_ERROR || eRes==SORT_CLAUSE_RANDOM )
		{
			if ( eRes==SORT_CLAUSE_RANDOM )
				sError.SetSprintf ( "groups can not be sorted by @random" );
			return NULL;
		}

		ExtraAddSortkeys ( pExtra, tSorterSchema, tStateGroup.m_dAttrs );

		assert ( dGroupColumns.GetLength() || tSettings.m_bImplicit );
		if ( pExtra && !tSettings.m_bImplicit )
		{
			ARRAY_FOREACH ( i, dGroupColumns )
				pExtra->AddAttr ( tSorterSchema.GetAttr ( dGroupColumns[i] ), true );
		}

		if ( bGotDistinct )
		{
			dGroupColumns.Add ( tSorterSchema.GetAttrIndex ( pQuery->m_sGroupDistinct.cstr() ) );
			assert ( dGroupColumns.Last()>=0 );
			if ( pExtra )
				pExtra->AddAttr ( tSorterSchema.GetAttr ( dGroupColumns.Last() ), true );
		}

		if ( dGroupColumns.GetLength() ) // implicit case
		{
			FixupDependency ( tSorterSchema, dGroupColumns.Begin(), dGroupColumns.GetLength() );
		}
		FixupDependency ( tSorterSchema, tStateGroup.m_dAttrs, CSphMatchComparatorState::MAX_ATTRS );

		// GroupSortBy str attributes setup
		SetupSortRemap ( tSorterSchema, tStateGroup );
	}

	// set up aggregate filter for grouper
	if ( bGotGroupby && tQueue.m_pAggrFilter && !tQueue.m_pAggrFilter->m_sAttrName.IsEmpty() )
	{
		if ( tSorterSchema.GetAttr ( tQueue.m_pAggrFilter->m_sAttrName.cstr() ) )
		{
			tSettings.m_pAggrFilterTrait = sphCreateAggrFilter ( tQueue.m_pAggrFilter, tQueue.m_pAggrFilter->m_sAttrName, tSorterSchema, sError );
		} else
		{
			// having might reference aliased attributes but @* attributes got stored without alias in sorter schema
			CSphString sHaving;
			ARRAY_FOREACH ( i, pQuery->m_dItems )
			{
				const CSphQueryItem & tItem = pQuery->m_dItems[i];
				if ( tItem.m_sAlias==tQueue.m_pAggrFilter->m_sAttrName )
				{
					sHaving = tItem.m_sExpr;
					break;
				}
			}

			if ( sHaving=="groupby()" )
				sHaving = "@groupby";
			else if ( sHaving=="count(*)" )
				sHaving = "@count";

			tSettings.m_pAggrFilterTrait = sphCreateAggrFilter ( tQueue.m_pAggrFilter, sHaving, tSorterSchema, sError );
		}

		if ( !tSettings.m_pAggrFilterTrait )
			return NULL;
	}

	///////////////////
	// spawn the queue
	///////////////////

	if ( !bGotGroupby )
	{
		if ( tQueue.m_pUpdate )
			pTop = new CSphUpdateQueue ( pQuery->m_iMaxMatches, tQueue.m_pUpdate, pQuery->m_bIgnoreNonexistent, pQuery->m_bStrict );
		else if ( tQueue.m_pDeletes )
			pTop = new CSphDeleteQueue ( pQuery->m_iMaxMatches, tQueue.m_pDeletes );
		else
			pTop = CreatePlainSorter ( eMatchFunc, pQuery->m_bSortKbuffer, pQuery->m_iMaxMatches, bUsesAttrs, bNeedPackedFactors );
	} else
	{
		pTop = sphCreateSorter1st ( eMatchFunc, eGroupFunc, pQuery, tSettings, bNeedPackedFactors );
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

	tQueue.m_bZonespanlist = bNeedZonespanlist;
	tQueue.m_bPackedFactors = bNeedPackedFactors;

	return pTop;
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
	ARRAY_FOREACH ( i, tQuery.m_dItems )
	{
		const CSphQueryItem & tItem = tQuery.m_dItems[i];
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


//
// $Id$
//

