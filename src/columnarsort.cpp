//
// Copyright (c) 2021-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "columnarsort.h"
#include "attribute.h"
#include "sortcomp.h"
#include "conversion.h"


class CompareSingleError_fn
{
public:
	FORCE_INLINE static bool IsLess ( RowID_t tRowIdA, SphAttr_t a, RowID_t tRowIdB, SphAttr_t b )
	{
		assert ( 0 && "Internal error" );
		return false;
	}
};

template <bool INV>
class CompareSingleInt_fn
{
public:
	FORCE_INLINE static bool IsLess ( RowID_t tRowIdA, SphAttr_t a, RowID_t tRowIdB, SphAttr_t b )
	{
		if ( a!=b ) return ( a>b ) ^ INV;
		return tRowIdA>tRowIdB;
	}
};

template <bool INV>
class CompareSingleFloat_fn
{
public:
	FORCE_INLINE static bool IsLess ( RowID_t tRowIdA, SphAttr_t a, RowID_t tRowIdB, SphAttr_t b )
	{
		float fA = sphDW2F(DWORD(a));
		float fB = sphDW2F(DWORD(b));
		if ( fA!=fB ) return ( fA>fB ) ^ INV;
		return tRowIdA>tRowIdB;
	}
};

/////////////////////////////////////////////////////////////////////

class CompareError_fn
{
public:
	FORCE_INLINE static bool IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
	{
		assert ( 0 && "Internal error" );
		return false;
	}
};

template <int ARGS>
class CompareInt_fn
{
public:
	FORCE_INLINE static bool IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
	{
		assert ( 0 && "Internal error" );
		return false;
	}
};

template <>
FORCE_INLINE bool CompareInt_fn<2>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	DWORD uAttrDesc = tState.m_uAttrDesc;
	if ( dA[0]!=dB[0] ) return ( dA[0]>dB[0] ) ^ ( ( uAttrDesc >> 0 ) & 1 );
	if ( dA[1]!=dB[1] ) return ( dA[1]>dB[1] ) ^ ( ( uAttrDesc >> 1 ) & 1 );
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool CompareInt_fn<3>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	DWORD uAttrDesc = tState.m_uAttrDesc;
	if ( dA[0]!=dB[0] ) return ( dA[0]>dB[0] ) ^ ( ( uAttrDesc >> 0 ) & 1 );
	if ( dA[1]!=dB[1] ) return ( dA[1]>dB[1] ) ^ ( ( uAttrDesc >> 1 ) & 1 );
	if ( dA[2]!=dB[2] ) return ( dA[2]>dB[2] ) ^ ( ( uAttrDesc >> 2 ) & 1 );
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool CompareInt_fn<4>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	DWORD uAttrDesc = tState.m_uAttrDesc;
	if ( dA[0]!=dB[0] ) return ( dA[0]>dB[0] ) ^ ( ( uAttrDesc >> 0 ) & 1 );
	if ( dA[1]!=dB[1] ) return ( dA[1]>dB[1] ) ^ ( ( uAttrDesc >> 1 ) & 1 );
	if ( dA[2]!=dB[2] ) return ( dA[2]>dB[2] ) ^ ( ( uAttrDesc >> 2 ) & 1 );
	if ( dA[3]!=dB[3] ) return ( dA[3]>dB[3] ) ^ ( ( uAttrDesc >> 3 ) & 1 );
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool CompareInt_fn<5>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	DWORD uAttrDesc = tState.m_uAttrDesc;
	if ( dA[0]!=dB[0] ) return ( dA[0]>dB[0] ) ^ ( ( uAttrDesc >> 0 ) & 1 );
	if ( dA[1]!=dB[1] ) return ( dA[1]>dB[1] ) ^ ( ( uAttrDesc >> 1 ) & 1 );
	if ( dA[2]!=dB[2] ) return ( dA[2]>dB[2] ) ^ ( ( uAttrDesc >> 2 ) & 1 );
	if ( dA[3]!=dB[3] ) return ( dA[3]>dB[3] ) ^ ( ( uAttrDesc >> 3 ) & 1 );
	if ( dA[4]!=dB[4] ) return ( dA[4]>dB[4] ) ^ ( ( uAttrDesc >> 4 ) & 1 );
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

/////////////////////////////////////////////////////////////////////

#define COMPARE_FLOATS(A,B,INV) \
{ \
	float fA = sphDW2F(DWORD(A)); \
	float fB = sphDW2F(DWORD(B)); \
	if ( fA!=fB ) return ( fA>fB ) ^ INV; \
}

template <int ARGS>
class CompareFloat_fn
{
public:
	FORCE_INLINE static bool IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
	{
		assert ( 0 && "Internal error" );
		return false;
	}
};

template <>
FORCE_INLINE bool CompareFloat_fn<2>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	DWORD uAttrDesc = tState.m_uAttrDesc;
	COMPARE_FLOATS ( dA[0], dB[0], ( ( uAttrDesc >> 0 ) & 1 ) );
	COMPARE_FLOATS ( dA[1], dB[1], ( ( uAttrDesc >> 1 ) & 1 ) );
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool CompareFloat_fn<3>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	DWORD uAttrDesc = tState.m_uAttrDesc;
	COMPARE_FLOATS ( dA[0], dB[0], ( ( uAttrDesc >> 0 ) & 1 ) );
	COMPARE_FLOATS ( dA[1], dB[1], ( ( uAttrDesc >> 1 ) & 1 ) );
	COMPARE_FLOATS ( dA[2], dB[2], ( ( uAttrDesc >> 2 ) & 1 ) );
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool CompareFloat_fn<4>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	DWORD uAttrDesc = tState.m_uAttrDesc;
	COMPARE_FLOATS ( dA[0], dB[0], ( ( uAttrDesc >> 0 ) & 1 ) );
	COMPARE_FLOATS ( dA[1], dB[1], ( ( uAttrDesc >> 1 ) & 1 ) );
	COMPARE_FLOATS ( dA[2], dB[2], ( ( uAttrDesc >> 2 ) & 1 ) );
	COMPARE_FLOATS ( dA[3], dB[3], ( ( uAttrDesc >> 3 ) & 1 ) );
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool CompareFloat_fn<5>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	DWORD uAttrDesc = tState.m_uAttrDesc;
	COMPARE_FLOATS ( dA[0], dB[0], ( ( uAttrDesc >> 0 ) & 1 ) );
	COMPARE_FLOATS ( dA[1], dB[1], ( ( uAttrDesc >> 1 ) & 1 ) );
	COMPARE_FLOATS ( dA[2], dB[2], ( ( uAttrDesc >> 2 ) & 1 ) );
	COMPARE_FLOATS ( dA[3], dB[3], ( ( uAttrDesc >> 3 ) & 1 ) );
	COMPARE_FLOATS ( dA[4], dB[4], ( ( uAttrDesc >> 4 ) & 1 ) );
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

/////////////////////////////////////////////////////////////////////

#define TEST_PAIR(_aa,_bb,_idx ) \
	if ( (_aa)!=(_bb) ) \
		return ( (tState.m_uAttrDesc >> (_idx)) & 1 ) ^ ( (_aa) > (_bb) );

#define TEST_KEYPART(_idx) \
	switch ( tState.m_eKeypart[_idx] ) \
	{ \
		case SPH_KEYPART_ROWID:		TEST_PAIR ( tMatchA.m_tRowID, tMatchB.m_tRowID, _idx ); break; \
		case SPH_KEYPART_WEIGHT:	TEST_PAIR ( tMatchA.m_iWeight, tMatchB.m_iWeight, _idx ); break; \
		case SPH_KEYPART_INT: \
		{ \
			SphAttr_t aa = dA[tState.m_dAttrs[_idx]]; \
			SphAttr_t bb = dB[tState.m_dAttrs[_idx]]; \
			TEST_PAIR ( aa, bb, _idx ); \
			break; \
		} \
		case SPH_KEYPART_FLOAT: \
		{ \
			float aa = sphDW2F ( dA[tState.m_dAttrs[_idx]] ); \
			float bb = sphDW2F ( dB[tState.m_dAttrs[_idx]] ); \
			TEST_PAIR ( aa, bb, _idx ) \
			break; \
		} \
		case SPH_KEYPART_DOUBLE: \
		{ \
			double aa = sphQW2D ( dA[tState.m_dAttrs[_idx]] ); \
			double bb = sphQW2D ( dB[tState.m_dAttrs[_idx]] ); \
			TEST_PAIR ( aa, bb, _idx ) \
			break; \
		} \
		default: \
			assert ( 0 && "Internal error" ); \
			break; \
	}


template <int ARGS>
class Compare_fn
{
public:
	FORCE_INLINE static bool IsLess ( RowID_t tRowIdA, SphAttr_t a, RowID_t tRowIdB, SphAttr_t b, bool bInv )
	{
		assert ( 0 && "Internal error" );
		return false;
	}

	FORCE_INLINE static bool IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
	{
		assert ( 0 && "Internal error" );
		return false;
	}
};

template <>
FORCE_INLINE bool Compare_fn<1>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	TEST_KEYPART(0);
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool Compare_fn<2>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	TEST_KEYPART(0);
	TEST_KEYPART(1);
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool Compare_fn<3>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	TEST_KEYPART(0);
	TEST_KEYPART(1);
	TEST_KEYPART(2);
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool Compare_fn<4>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	TEST_KEYPART(0);
	TEST_KEYPART(1);
	TEST_KEYPART(2);
	TEST_KEYPART(3);
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

template <>
FORCE_INLINE bool Compare_fn<5>::IsLess ( const CSphMatch & tMatchA, SphAttr_t * dA, const CSphMatch & tMatchB, SphAttr_t * dB, const CSphMatchComparatorState & tState )
{
	TEST_KEYPART(0);
	TEST_KEYPART(1);
	TEST_KEYPART(2);
	TEST_KEYPART(3);
	TEST_KEYPART(4);
	return tMatchA.m_tRowID>tMatchB.m_tRowID;
}

/////////////////////////////////////////////////////////////////////

template <typename GENERIC, typename COMP, typename SINGLE>
class ColumnarProxySorter_T : public ISphMatchSorter 
{
public:
			ColumnarProxySorter_T ( ISphMatchSorter * pSorter, int iSize, int iFastPathAttrs );
			~ColumnarProxySorter_T() override;

	bool	Push ( const CSphMatch & tEntry ) final							{ return PushMatch(tEntry); }
	void	Push ( const VecTraits_T<const CSphMatch> & dMatches ) override	{ assert ( 0 && "No batch push to proxy sorter" ); }

	bool	IsGroupby() const override										{ return m_pSorter->IsGroupby(); }
	bool	PushGrouped ( const CSphMatch & tEntry, bool bNewSet ) override	{ return m_pSorter->PushGrouped ( tEntry, bNewSet ); }
	int		GetLength () override;
	void	Finalize ( MatchProcessor_i & tProcessor, bool bCallProcessInResultSetOrder, bool bFinalizeMatches ) override;
	int		Flatten ( CSphMatch * pTo ) override;
	void	MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta ) override;

	ISphMatchSorter * Clone() const override								{ return new ColumnarProxySorter_T<GENERIC,COMP,SINGLE> ( m_pSorter->Clone(), m_iSize, m_iFastPathAttrs ); }
	void	CloneTo ( ISphMatchSorter * pTrg ) const override;

	void	SetState ( const CSphMatchComparatorState & tState ) override;
	const CSphMatchComparatorState & GetState() const override				{ return m_pSorter->GetState(); }

	void	SetSchema ( ISphSchema * pSchema, bool bRemapCmp ) override;
	const ISphSchema * GetSchema() const override							{ return m_pSchema; }

	void	SetColumnar ( columnar::Columnar_i * pColumnar ) override;
	int64_t	GetTotalCount() const override									{ return m_pSorter->GetTotalCount(); }

	void	SetFilteredAttrs ( const sph::StringSet & hAttrs, bool bAddDocid ) override { m_pSorter->SetFilteredAttrs ( hAttrs, bAddDocid ); }
	void	TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters ) override;

	bool	IsRandom() const override 										{ return m_pSorter->IsRandom(); }
	void	SetRandom ( bool bRandom ) override								{ m_pSorter->SetRandom(bRandom); }

	int		GetMatchCapacity() const override								{ return m_pSorter->GetMatchCapacity(); }

	RowTagged_t					GetJustPushed() const override						{ assert (0 && "Not supported" ); return RowTagged_t(); }
	VecTraits_T<RowTagged_t>	GetJustPopped() const override						{ assert (0 && "Not supported" ); return {}; }

	void		SetMerge ( bool bMerge ) override							{}

private:
	struct IteratorWithLocator_t
	{
		std::unique_ptr<columnar::Iterator_i>	m_pIterator;
		CSphAttrLocator							m_tLocator;
	};

	static const int MATCH_BUFFER_SIZE = 1024;

	CSphFixedVector<CSphMatch>	m_dData{MATCH_BUFFER_SIZE};
	CSphVector<CSphRowitem>		m_dDynamic;
	CSphVector<IteratorWithLocator_t> m_dIterators;

	CSphFixedVector<uint32_t>	m_dRowIDs{MATCH_BUFFER_SIZE};
	CSphFixedVector<int64_t>	m_dValues{0};

	columnar::Columnar_i *		m_pColumnar = nullptr;
	std::unique_ptr<ISphMatchSorter> m_pSorter;
	const ISphSchema *			m_pSchema = nullptr;
	CSphMatch *					m_pCurMatch = nullptr;
	CSphMatch *					m_pEndMatch = nullptr;
	uint32_t *					m_pCurRowID = nullptr;

	CSphMatchComparatorState	m_tState;			// a copy of underlying sorter's state, but may have modified m_dAttrs

	int							m_iDynamicSize = 0;
	int							m_iSize = 0;
	int64_t						m_iPushed = 0;
	int							m_iFastPathAttrs = 0;

	FORCE_INLINE bool PushMatch ( const CSphMatch & tEntry );
	void	PushCollectedToSorter();
	void	FetchColumnarValues ( VecTraits_T<CSphMatch> & dMatches );
	void	SpawnIterators();
	void	DoSetSchema ( const ISphSchema * pSchema );
	void	FetchValuesIteratorGeneric ( VecTraits_T<CSphMatch> & dMatches );
	void	FetchValuesIterator1Single ( VecTraits_T<CSphMatch> & dMatches );
	void	FetchValuesMultiIterator ( VecTraits_T<CSphMatch> & dMatches );

	// these are unrolled versions of FetchValuesMultiIterator
	void	FetchValuesIterator1 ( VecTraits_T<CSphMatch> & dMatches );
	void	FetchValuesIterator2 ( VecTraits_T<CSphMatch> & dMatches );
	void	FetchValuesIterator3 ( VecTraits_T<CSphMatch> & dMatches );
	void	FetchValuesIterator4 ( VecTraits_T<CSphMatch> & dMatches );
	void	FetchValuesIterator5 ( VecTraits_T<CSphMatch> & dMatches );
};

template <typename GENERIC, typename COMP, typename SINGLE>
ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::ColumnarProxySorter_T ( ISphMatchSorter * pSorter, int iSize, int iFastPathAttrs )
	: m_pSorter ( pSorter )
	, m_iSize ( iSize )
	, m_iFastPathAttrs ( iFastPathAttrs )
{
	assert(pSorter);
	m_pCurMatch = m_dData.Begin();
	m_pEndMatch = m_pCurMatch + m_dData.GetLength();
	m_pCurRowID = m_dRowIDs.Begin();

	m_dValues.Reset ( ( iFastPathAttrs ? iFastPathAttrs : 1 ) * MATCH_BUFFER_SIZE );
	DoSetSchema ( pSorter->GetSchema() );
	m_tState = pSorter->GetState();
}

template <typename GENERIC, typename COMP, typename SINGLE>
ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::~ColumnarProxySorter_T()
{
	for ( auto & i : m_dData )
		i.m_pDynamic = nullptr;
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::SetSchema ( ISphSchema * pSchema, bool bRemapCmp )
{
	m_pSorter->SetSchema ( pSchema, bRemapCmp );
	DoSetSchema(pSchema);
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::DoSetSchema ( const ISphSchema * pSchema )
{
	m_pSchema = pSchema;
	if ( !m_pSchema )
		return;

	m_iDynamicSize = m_pSchema->GetDynamicSize();
#if NDEBUG
	int iStride = m_iDynamicSize;
#else
	int iStride = m_iDynamicSize+1;
#endif
	m_dDynamic.Resize ( iStride*m_dData.GetLength() );
	CSphRowitem * pDynamic = m_dDynamic.Begin();

	CSphMatch tMatch;

	// set the dynamic part to point to out buffer
	for ( auto & i : m_dData )
	{
#if NDEBUG
		i.m_pDynamic = pDynamic;
#else
		*pDynamic = m_iDynamicSize;
		i.m_pDynamic = pDynamic+1;
#endif

		pDynamic += iStride;
	}
}


template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::CloneTo ( ISphMatchSorter * pTrg ) const
{
	pTrg->SetRandom ( IsRandom() );
	pTrg->SetState  ( GetState() );
	pTrg->SetSchema ( m_pSchema->CloneMe(), false );
}

template <typename GENERIC, typename COMP, typename SINGLE>
int ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::GetLength()
{
	assert(m_pSorter);
	PushCollectedToSorter();
	return m_pSorter->GetLength();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::SetColumnar ( columnar::Columnar_i * pColumnar )
{
	// we might be working with an RT index
	// in that case SetColumnar call tells us that we need to calculate all values and push them to sorter
	if ( m_pColumnar )
		PushCollectedToSorter();

	m_pColumnar = pColumnar;
	m_pSorter->SetColumnar(pColumnar);

	SpawnIterators();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::TransformPooled2StandalonePtrs ( GetBlobPoolFromMatch_fn fnBlobPoolFromMatch, GetColumnarFromMatch_fn fnGetColumnarFromMatch, bool bFinalizeSorters )
{
	assert(m_pSorter);
	PushCollectedToSorter();
	m_pSorter->TransformPooled2StandalonePtrs ( fnBlobPoolFromMatch, fnGetColumnarFromMatch, bFinalizeSorters );
	m_pSchema = m_pSorter->GetSchema();
}

template <typename GENERIC, typename COMP, typename SINGLE>
bool ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::PushMatch ( const CSphMatch & tEntry )
{
	// this is a simplified (faster) version of match cloning
	CSphMatch & tMatch = *m_pCurMatch++;
	tMatch.m_tRowID		= tEntry.m_tRowID;
	tMatch.m_iWeight	= tEntry.m_iWeight;
	tMatch.m_pStatic	= tEntry.m_pStatic;
	tMatch.m_iTag		= tEntry.m_iTag;
	memcpy ( tMatch.m_pDynamic, tEntry.m_pDynamic, m_iDynamicSize*sizeof(CSphRowitem) );

	*m_pCurRowID++ = tEntry.m_tRowID;

	if ( m_pCurMatch==m_pEndMatch )
		PushCollectedToSorter();

	return true;
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::Finalize ( MatchProcessor_i & tProcessor, bool bCallProcessInResultSetOrder, bool bFinalizeMatches )
{
	assert(m_pSorter);
	PushCollectedToSorter();
	m_pSorter->Finalize ( tProcessor, bCallProcessInResultSetOrder, bFinalizeMatches );
}

template <typename GENERIC, typename COMP, typename SINGLE>
int ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::Flatten ( CSphMatch * pTo )
{
	assert(m_pSorter);
	PushCollectedToSorter();
	return m_pSorter->Flatten(pTo);
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::MoveTo ( ISphMatchSorter * pRhs, bool bCopyMeta )
{
	// we assume that the rhs sorter is of the same type, i.e. proxy
	auto pRhsProxy = (ColumnarProxySorter_T<GENERIC,COMP,SINGLE>*)pRhs;

	PushCollectedToSorter();
	pRhsProxy->PushCollectedToSorter();

	m_pSorter->MoveTo ( pRhsProxy->m_pSorter.get(), bCopyMeta );
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::SetState ( const CSphMatchComparatorState & tState )
{
	assert(m_pSorter);
	m_pSorter->SetState(tState);
	m_tState = m_pSorter->GetState();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::PushCollectedToSorter()
{
	assert(m_pSorter);
	CSphMatch * pData = m_dData.Begin();
	int iNumMatches = m_pCurMatch-pData;
	if ( !iNumMatches )
		return;

	VecTraits_T<CSphMatch> dMatches = { pData, iNumMatches };
	FetchColumnarValues(dMatches);
	m_pSorter->Push(dMatches);

	m_pCurMatch = m_dData.Begin();
	m_pCurRowID = m_dRowIDs.Begin();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::SpawnIterators()
{
	assert ( m_pSchema && m_pSorter && m_pColumnar );

	const CSphMatchComparatorState & tOriginalState = m_pSorter->GetState();

	m_dIterators.Reset();
	for ( int i = 0; i < CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tOriginalState.m_dAttrs[i]==-1 )
			continue;

		const CSphColumnInfo & tAttr = m_pSchema->GetAttr ( tOriginalState.m_dAttrs[i] );
		if ( tAttr.IsColumnarExpr() )
		{
			CSphString sAliasedCol;
			tAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sAliasedCol );

			std::string sError;

			// need to fixup attribute ids to use fastpath match comparison func
			if ( m_iFastPathAttrs )
				m_tState.m_dAttrs[i] = m_dIterators.GetLength();

			m_dIterators.Add ( { CreateColumnarIterator ( m_pColumnar, sAliasedCol.cstr(), sError ), tAttr.m_tLocator } );
			assert ( m_dIterators.Last().m_pIterator.get() );
		}
	}
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchValuesIterator1Single ( VecTraits_T<CSphMatch> & dMatches )
{
	IteratorWithLocator_t & tIterator = m_dIterators[0];

	int iNumValues = dMatches.GetLength();
	util::Span_T<uint32_t> dRowIDs ( m_dRowIDs.Begin(), iNumValues );
	util::Span_T<int64_t> dValues ( m_dValues.Begin(), iNumValues );
	tIterator.m_pIterator->Fetch ( dRowIDs, dValues );
	int64_t * pValue = m_dValues.Begin();

	assert(m_pSorter);
	const CSphMatch * pWorst = m_pSorter->GetWorst();
	if ( pWorst && m_iPushed>=m_iSize )
	{
		SphAttr_t tWorstValue = pWorst->GetAttr ( tIterator.m_tLocator );
		RowID_t tWorstRowID = pWorst->m_tRowID;

		int iNumMatches = 0;
		for ( auto & tMatch : dMatches )
		{
			int64_t tValue = *pValue++;
			if ( SINGLE::IsLess ( tMatch.m_tRowID, tValue, tWorstRowID, tWorstValue ) )
				tMatch.m_tRowID = INVALID_ROWID;
			else
			{
				sphSetRowAttr ( tMatch.m_pDynamic, tIterator.m_tLocator, tValue );
				iNumMatches++;
			}
		}

		m_iPushed += iNumMatches;
	}
	else
	{
		for ( auto & tMatch : dMatches )
			sphSetRowAttr ( tMatch.m_pDynamic, tIterator.m_tLocator, *pValue++ );

		m_iPushed += dMatches.GetLength();
	}
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchValuesIterator1 ( VecTraits_T<CSphMatch> & dMatches )
{
	IteratorWithLocator_t & tIt = m_dIterators[0];

	int iNumValues = dMatches.GetLength();
	util::Span_T<uint32_t> dRowIDs ( m_dRowIDs.Begin(), iNumValues );
	util::Span_T<int64_t> dValues ( m_dValues.Begin(), iNumValues );
	tIt.m_pIterator->Fetch ( dRowIDs, dValues );
	int64_t * pValue = m_dValues.Begin();

	SphAttr_t dValueRow[CSphMatchComparatorState::MAX_ATTRS];
	SphAttr_t dWorstValueRow[CSphMatchComparatorState::MAX_ATTRS];
	memset ( dValueRow, 0, sizeof(dValueRow) );
	memset ( dWorstValueRow, 0, sizeof(dWorstValueRow) );

	assert(m_pSorter);
	const CSphMatch * pWorst = m_pSorter->GetWorst();
	if ( pWorst && m_iPushed>=m_iSize )
	{
		dWorstValueRow[0] = pWorst->GetAttr ( tIt.m_tLocator );

		int iNumMatches = 0;
		for ( auto & tMatch : dMatches )
		{
			dValueRow[0] = *pValue++;
			if ( COMP::IsLess ( tMatch, dValueRow, *pWorst, dWorstValueRow, m_tState ) )
				tMatch.m_tRowID = INVALID_ROWID;
			else
			{
				sphSetRowAttr ( tMatch.m_pDynamic, tIt.m_tLocator, dValueRow[0] );
				iNumMatches++;
			}
		}

		m_iPushed += iNumMatches;
		return;
	}
	else
	{
		for ( auto & tMatch : dMatches )
			sphSetRowAttr ( tMatch.m_pDynamic, tIt.m_tLocator, *pValue++ );

		m_iPushed += dMatches.GetLength();
	}
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchValuesIterator2 ( VecTraits_T<CSphMatch> & dMatches )
{
	IteratorWithLocator_t & tIt0 = m_dIterators[0];
	IteratorWithLocator_t & tIt1 = m_dIterators[1];

	int64_t * pValues0 = m_dValues.Begin();
	int64_t * pValues1 = m_dValues.Begin() + MATCH_BUFFER_SIZE;

	int iNumValues = dMatches.GetLength();
	util::Span_T<uint32_t> dRowIDs ( m_dRowIDs.Begin(), iNumValues );
	util::Span_T<int64_t> dValues0 ( pValues0, iNumValues );
	util::Span_T<int64_t> dValues1 ( pValues1, iNumValues );

	tIt0.m_pIterator->Fetch ( dRowIDs, dValues0 );
	tIt1.m_pIterator->Fetch ( dRowIDs, dValues1 );

	SphAttr_t dValueRow[CSphMatchComparatorState::MAX_ATTRS];
	SphAttr_t dWorstValueRow[CSphMatchComparatorState::MAX_ATTRS];
	memset ( dValueRow, 0, sizeof(dValueRow) );
	memset ( dWorstValueRow, 0, sizeof(dWorstValueRow) );

	assert(m_pSorter);
	const CSphMatch * pWorst = m_pSorter->GetWorst();
	if ( pWorst && m_iPushed>=m_iSize )
	{
		dWorstValueRow[0] = pWorst->GetAttr ( tIt0.m_tLocator );
		dWorstValueRow[1] = pWorst->GetAttr ( tIt1.m_tLocator );

		int iNumMatches = 0;
		ARRAY_FOREACH ( iMatch, dMatches )
		{
			CSphMatch & tMatch = dMatches[iMatch];
			dValueRow[0] = *(pValues0 + iMatch);
			dValueRow[1] = *(pValues1 + iMatch);

			if ( COMP::IsLess ( tMatch, dValueRow, *pWorst, dWorstValueRow, m_tState ) )
				tMatch.m_tRowID = INVALID_ROWID;
			else
			{
				sphSetRowAttr ( tMatch.m_pDynamic, tIt0.m_tLocator, *(pValues0 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt1.m_tLocator, *(pValues1 + iMatch) );
				iNumMatches++;
			}
		}

		m_iPushed += iNumMatches;
		return;
	}

	ARRAY_FOREACH ( iMatch, dMatches )
	{
		CSphMatch & tMatch = dMatches[iMatch];
		sphSetRowAttr ( tMatch.m_pDynamic, tIt0.m_tLocator, *(pValues0 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt1.m_tLocator, *(pValues1 + iMatch) );
	}

	m_iPushed += dMatches.GetLength();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchValuesIterator3 ( VecTraits_T<CSphMatch> & dMatches )
{
	IteratorWithLocator_t & tIt0 = m_dIterators[0];
	IteratorWithLocator_t & tIt1 = m_dIterators[1];
	IteratorWithLocator_t & tIt2 = m_dIterators[2];

	int64_t * pValues0 = m_dValues.Begin();
	int64_t * pValues1 = m_dValues.Begin() + MATCH_BUFFER_SIZE;
	int64_t * pValues2 = m_dValues.Begin() + MATCH_BUFFER_SIZE*2;

	int iNumValues = dMatches.GetLength();
	util::Span_T<uint32_t> dRowIDs ( m_dRowIDs.Begin(), iNumValues );

	util::Span_T<int64_t> dValues0 ( pValues0, iNumValues );
	util::Span_T<int64_t> dValues1 ( pValues1, iNumValues );
	util::Span_T<int64_t> dValues2 ( pValues2, iNumValues );

	tIt0.m_pIterator->Fetch ( dRowIDs, dValues0 );
	tIt1.m_pIterator->Fetch ( dRowIDs, dValues1 );
	tIt2.m_pIterator->Fetch ( dRowIDs, dValues2 );

	SphAttr_t dValueRow[CSphMatchComparatorState::MAX_ATTRS];
	SphAttr_t dWorstValueRow[CSphMatchComparatorState::MAX_ATTRS];
	memset ( dValueRow, 0, sizeof(dValueRow) );
	memset ( dWorstValueRow, 0, sizeof(dWorstValueRow) );

	assert(m_pSorter);
	const CSphMatch * pWorst = m_pSorter->GetWorst();
	if ( pWorst && m_iPushed>=m_iSize )
	{
		dWorstValueRow[0] = pWorst->GetAttr ( tIt0.m_tLocator );
		dWorstValueRow[1] = pWorst->GetAttr ( tIt1.m_tLocator );
		dWorstValueRow[2] = pWorst->GetAttr ( tIt2.m_tLocator );

		int iNumMatches = 0;
		ARRAY_FOREACH ( iMatch, dMatches )
		{
			CSphMatch & tMatch = dMatches[iMatch];
			dValueRow[0] = *(pValues0 + iMatch);
			dValueRow[1] = *(pValues1 + iMatch);
			dValueRow[2] = *(pValues2 + iMatch);

			if ( COMP::IsLess ( tMatch, dValueRow, *pWorst, dWorstValueRow, m_tState ) )
				tMatch.m_tRowID = INVALID_ROWID;
			else
			{
				sphSetRowAttr ( tMatch.m_pDynamic, tIt0.m_tLocator, *(pValues0 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt1.m_tLocator, *(pValues1 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt2.m_tLocator, *(pValues2 + iMatch) );
				iNumMatches++;
			}
		}

		m_iPushed += iNumMatches;
		return;
	}

	ARRAY_FOREACH ( iMatch, dMatches )
	{
		CSphMatch & tMatch = dMatches[iMatch];
		sphSetRowAttr ( tMatch.m_pDynamic, tIt0.m_tLocator, *(pValues0 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt1.m_tLocator, *(pValues1 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt2.m_tLocator, *(pValues2 + iMatch) );
	}

	m_iPushed += dMatches.GetLength();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchValuesIterator4 ( VecTraits_T<CSphMatch> & dMatches )
{
	IteratorWithLocator_t & tIt0 = m_dIterators[0];
	IteratorWithLocator_t & tIt1 = m_dIterators[1];
	IteratorWithLocator_t & tIt2 = m_dIterators[2];
	IteratorWithLocator_t & tIt3 = m_dIterators[3];

	int64_t * pValues0 = m_dValues.Begin();
	int64_t * pValues1 = m_dValues.Begin() + MATCH_BUFFER_SIZE;
	int64_t * pValues2 = m_dValues.Begin() + MATCH_BUFFER_SIZE*2;
	int64_t * pValues3 = m_dValues.Begin() + MATCH_BUFFER_SIZE*3;
	
	int iNumValues = dMatches.GetLength();
	util::Span_T<uint32_t> dRowIDs ( m_dRowIDs.Begin(), iNumValues );

	util::Span_T<int64_t> dValues0 ( pValues0, iNumValues );
	util::Span_T<int64_t> dValues1 ( pValues1, iNumValues );
	util::Span_T<int64_t> dValues2 ( pValues2, iNumValues );
	util::Span_T<int64_t> dValues3 ( pValues3, iNumValues );

	tIt0.m_pIterator->Fetch ( dRowIDs, dValues0 );
	tIt1.m_pIterator->Fetch ( dRowIDs, dValues1 );
	tIt2.m_pIterator->Fetch ( dRowIDs, dValues2 );
	tIt3.m_pIterator->Fetch ( dRowIDs, dValues3 );

	SphAttr_t dValueRow[CSphMatchComparatorState::MAX_ATTRS];
	SphAttr_t dWorstValueRow[CSphMatchComparatorState::MAX_ATTRS];
	memset ( dValueRow, 0, sizeof(dValueRow) );
	memset ( dWorstValueRow, 0, sizeof(dWorstValueRow) );

	assert(m_pSorter);
	const CSphMatch * pWorst = m_pSorter->GetWorst();
	if ( pWorst && m_iPushed>=m_iSize )
	{
		dWorstValueRow[0] = pWorst->GetAttr ( tIt0.m_tLocator );
		dWorstValueRow[1] = pWorst->GetAttr ( tIt1.m_tLocator );
		dWorstValueRow[2] = pWorst->GetAttr ( tIt2.m_tLocator );
		dWorstValueRow[3] = pWorst->GetAttr ( tIt3.m_tLocator );

		int iNumMatches = 0;
		ARRAY_FOREACH ( iMatch, dMatches )
		{
			CSphMatch & tMatch = dMatches[iMatch];
			dValueRow[0] = *(pValues0 + iMatch);
			dValueRow[1] = *(pValues1 + iMatch);
			dValueRow[2] = *(pValues2 + iMatch);
			dValueRow[3] = *(pValues3 + iMatch);

			if ( COMP::IsLess ( tMatch, dValueRow, *pWorst, dWorstValueRow, m_tState ) )
				tMatch.m_tRowID = INVALID_ROWID;
			else
			{
				sphSetRowAttr ( tMatch.m_pDynamic, tIt0.m_tLocator, *(pValues0 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt1.m_tLocator, *(pValues1 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt2.m_tLocator, *(pValues2 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt3.m_tLocator, *(pValues3 + iMatch) );
				iNumMatches++;
			}
		}

		m_iPushed += iNumMatches;
		return;
	}

	ARRAY_FOREACH ( iMatch, dMatches )
	{
		CSphMatch & tMatch = dMatches[iMatch];
		sphSetRowAttr ( tMatch.m_pDynamic, tIt0.m_tLocator, *(pValues0 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt1.m_tLocator, *(pValues1 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt2.m_tLocator, *(pValues2 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt3.m_tLocator, *(pValues3 + iMatch) );
	}

	m_iPushed += dMatches.GetLength();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchValuesIterator5 ( VecTraits_T<CSphMatch> & dMatches )
{
	IteratorWithLocator_t & tIt0 = m_dIterators[0];
	IteratorWithLocator_t & tIt1 = m_dIterators[1];
	IteratorWithLocator_t & tIt2 = m_dIterators[2];
	IteratorWithLocator_t & tIt3 = m_dIterators[3];
	IteratorWithLocator_t & tIt4 = m_dIterators[4];

	int64_t * pValues0 = m_dValues.Begin();
	int64_t * pValues1 = m_dValues.Begin() + MATCH_BUFFER_SIZE;
	int64_t * pValues2 = m_dValues.Begin() + MATCH_BUFFER_SIZE*2;
	int64_t * pValues3 = m_dValues.Begin() + MATCH_BUFFER_SIZE*3;
	int64_t * pValues4 = m_dValues.Begin() + MATCH_BUFFER_SIZE*4;

	int iNumValues = dMatches.GetLength();
	util::Span_T<uint32_t> dRowIDs ( m_dRowIDs.Begin(), iNumValues );

	util::Span_T<int64_t> dValues0 ( pValues0, iNumValues );
	util::Span_T<int64_t> dValues1 ( pValues1, iNumValues );
	util::Span_T<int64_t> dValues2 ( pValues2, iNumValues );
	util::Span_T<int64_t> dValues3 ( pValues3, iNumValues );
	util::Span_T<int64_t> dValues4 ( pValues4, iNumValues );

	tIt0.m_pIterator->Fetch ( dRowIDs, dValues0 );
	tIt1.m_pIterator->Fetch ( dRowIDs, dValues1 );
	tIt2.m_pIterator->Fetch ( dRowIDs, dValues2 );
	tIt3.m_pIterator->Fetch ( dRowIDs, dValues3 );
	tIt4.m_pIterator->Fetch ( dRowIDs, dValues4 );

	SphAttr_t dValueRow[CSphMatchComparatorState::MAX_ATTRS];
	SphAttr_t dWorstValueRow[CSphMatchComparatorState::MAX_ATTRS];
	memset ( dValueRow, 0, sizeof(dValueRow) );
	memset ( dWorstValueRow, 0, sizeof(dWorstValueRow) );

	assert(m_pSorter);
	const CSphMatch * pWorst = m_pSorter->GetWorst();
	if ( pWorst && m_iPushed>=m_iSize )
	{
		dWorstValueRow[0] = pWorst->GetAttr ( tIt0.m_tLocator );
		dWorstValueRow[1] = pWorst->GetAttr ( tIt1.m_tLocator );
		dWorstValueRow[2] = pWorst->GetAttr ( tIt2.m_tLocator );
		dWorstValueRow[3] = pWorst->GetAttr ( tIt3.m_tLocator );
		dWorstValueRow[4] = pWorst->GetAttr ( tIt4.m_tLocator );

		int iNumMatches = 0;
		ARRAY_FOREACH ( iMatch, dMatches )
		{
			CSphMatch & tMatch = dMatches[iMatch];
			dValueRow[0] = *(pValues0 + iMatch);
			dValueRow[1] = *(pValues1 + iMatch);
			dValueRow[2] = *(pValues2 + iMatch);
			dValueRow[3] = *(pValues3 + iMatch);
			dValueRow[4] = *(pValues4 + iMatch);

			if ( COMP::IsLess ( tMatch, dValueRow, *pWorst, dWorstValueRow, m_tState ) )
				tMatch.m_tRowID = INVALID_ROWID;
			else
			{
				sphSetRowAttr ( tMatch.m_pDynamic, tIt0.m_tLocator, *(pValues0 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt1.m_tLocator, *(pValues1 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt2.m_tLocator, *(pValues2 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt3.m_tLocator, *(pValues3 + iMatch) );
				sphSetRowAttr ( tMatch.m_pDynamic, tIt4.m_tLocator, *(pValues4 + iMatch) );
				iNumMatches++;
			}
		}

		m_iPushed += iNumMatches;
		return;
	}

	ARRAY_FOREACH ( iMatch, dMatches )
	{
		CSphMatch & tMatch = dMatches[iMatch];
		sphSetRowAttr ( tMatch.m_pDynamic, tIt0.m_tLocator, *(pValues0 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt1.m_tLocator, *(pValues1 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt2.m_tLocator, *(pValues2 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt3.m_tLocator, *(pValues3 + iMatch) );
		sphSetRowAttr ( tMatch.m_pDynamic, tIt4.m_tLocator, *(pValues4 + iMatch) );
	}

	m_iPushed += dMatches.GetLength();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchValuesMultiIterator ( VecTraits_T<CSphMatch> & dMatches )
{
	int iNumValues = dMatches.GetLength();
	util::Span_T<uint32_t> dRowIDs ( m_dRowIDs.Begin(), iNumValues );
	ARRAY_FOREACH ( i, m_dIterators )
	{
		util::Span_T<int64_t> dValues ( m_dValues.Begin() + i*MATCH_BUFFER_SIZE, iNumValues );
		m_dIterators[i].m_pIterator->Fetch ( dRowIDs, dValues );
	}

	SphAttr_t dValueRow[CSphMatchComparatorState::MAX_ATTRS];
	SphAttr_t dWorstValueRow[CSphMatchComparatorState::MAX_ATTRS];
	memset ( dValueRow, 0, sizeof(dValueRow) );
	memset ( dWorstValueRow, 0, sizeof(dWorstValueRow) );

	assert(m_pSorter);
	const CSphMatch * pWorst = m_pSorter->GetWorst();
	if ( pWorst && m_iPushed>=m_iSize )
	{
		ARRAY_FOREACH ( i, m_dIterators )
			dWorstValueRow[i] = pWorst->GetAttr ( m_dIterators[i].m_tLocator );

		int iNumMatches = 0;
		ARRAY_FOREACH ( iMatch, dMatches )
		{
			CSphMatch & tMatch = dMatches[iMatch];

			int iOffset = 0;
			ARRAY_FOREACH ( i, m_dIterators )
			{
				dValueRow[i] = *(m_dValues.Begin() + iMatch + iOffset);
				iOffset += MATCH_BUFFER_SIZE;
			}

			if ( COMP::IsLess ( tMatch, dValueRow, *pWorst, dWorstValueRow, m_tState ) )
				tMatch.m_tRowID = INVALID_ROWID;
			else
			{
				iOffset = 0;
				ARRAY_FOREACH ( i, m_dIterators )
				{
					sphSetRowAttr ( tMatch.m_pDynamic, m_dIterators[i].m_tLocator, *(m_dValues.Begin() + iMatch + iOffset) );
					iOffset += MATCH_BUFFER_SIZE;
				}

				iNumMatches++;
			}
		}

		m_iPushed += iNumMatches;
		return;
	}

	ARRAY_FOREACH ( iMatch, dMatches )
	{
		CSphMatch & tMatch = dMatches[iMatch];
		int iOffset = 0;
		ARRAY_FOREACH ( i, m_dIterators )
		{
			sphSetRowAttr ( tMatch.m_pDynamic, m_dIterators[i].m_tLocator, *(m_dValues.Begin() + iMatch + iOffset) );
			iOffset += MATCH_BUFFER_SIZE;
		}
	}

	m_iPushed += dMatches.GetLength();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchValuesIteratorGeneric ( VecTraits_T<CSphMatch> & dMatches )
{
	for ( auto & i : m_dIterators )
	{
		int iNumValues = dMatches.GetLength();
		util::Span_T<uint32_t> dRowIDs ( m_dRowIDs.Begin(), iNumValues );
		util::Span_T<int64_t> dValues ( m_dValues.Begin(), iNumValues );
		i.m_pIterator->Fetch ( dRowIDs, dValues );

		int64_t * pValue = m_dValues.Begin();
		for ( auto & tMatch : dMatches )
			sphSetRowAttr ( tMatch.m_pDynamic, i.m_tLocator, *pValue++ );
	}

	assert(m_pSorter);
	const CSphMatch * pWorst = m_pSorter->GetWorst();
	if ( pWorst && m_iPushed>=m_iSize )
	{
		int iNumMatches = 0;
		for ( auto & tMatch : dMatches )
		{
			if ( GENERIC::IsLess ( tMatch, *pWorst, m_tState ) )
				tMatch.m_tRowID = INVALID_ROWID;
			else
				iNumMatches++;
		}

		m_iPushed += iNumMatches;
	}
	else
		m_iPushed += dMatches.GetLength();
}

template <typename GENERIC, typename COMP, typename SINGLE>
void ColumnarProxySorter_T<GENERIC,COMP,SINGLE>::FetchColumnarValues ( VecTraits_T<CSphMatch> & dMatches )
{
	if ( !m_iFastPathAttrs )
	{
		FetchValuesIteratorGeneric(dMatches);
		return;
	}

	if ( m_iFastPathAttrs==1 && m_iFastPathAttrs==m_dIterators.GetLength() )
	{
		FetchValuesIterator1Single(dMatches);
		return;
	}

	switch ( m_dIterators.GetLength() )
	{
	case 1:		FetchValuesIterator1(dMatches); break;
	case 2:		FetchValuesIterator2(dMatches); break;
	case 3:		FetchValuesIterator3(dMatches); break;
	case 4:		FetchValuesIterator4(dMatches); break;
	case 5:		FetchValuesIterator5(dMatches); break;
	default:	FetchValuesMultiIterator(dMatches); break;
	}
}


static bool CanCreateColumnarSorter ( const ISphSchema & tSchema, const CSphMatchComparatorState & tState, bool bNeedFactors, bool bComputeItems, bool bMulti )
{
	// everything precomputed? no need for batched sorter
	if ( !bComputeItems )
		return false;

	// CalcSort is the same for all sorters, so we can't remove an expression from it just for our sorter
	// so no support for multi-sorter case, at least for now
	if ( bMulti )
		return false;

	// can't expose immediate pushed/popped matches, so no luck
	if ( bNeedFactors )
		return false;

	// check that we don't have strings/mvas in presort/prefilter
	// in this case match cloning is slow, so there's no point in spawning proxy sorter
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( tAttr.m_eStage<=SPH_EVAL_PRESORT && tAttr.m_pExpr && sphIsDataPtrAttr ( tAttr.m_eAttrType ) )
			return false;

		if ( tAttr.IsJoined() )
			return false;
	}

	bool bHaveColumnar = false;
	bool bAllColumnar = true;
	for ( int i = 0; i < CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dAttrs[i]==-1 )
			continue;

		const CSphColumnInfo & tAttr = tSchema.GetAttr ( tState.m_dAttrs[i] );
		if ( sphIsDataPtrAttr ( tAttr.m_eAttrType ) )
			return false;

		// all sorter-related columnar attrs should be replaced by expressions at this point
		assert ( !tAttr.IsColumnar() );
		if ( tAttr.IsColumnarExpr() )
			bHaveColumnar = true;
		else
			bAllColumnar = false;
	}

	return bHaveColumnar && bAllColumnar;
}

/////////////////////////////////////////////////////////////////////

template <typename GENERIC>
static ISphMatchSorter * CreateProxySorter ( ISphMatchSorter * pSorter, int iMaxMatches, const ISphSchema & tSchema, const CSphMatchComparatorState & tState, int iSortFastPath, bool bAllInt, bool bAllFloat )
{
	if ( iSortFastPath==0 )
		return new ColumnarProxySorter_T<GENERIC,CompareError_fn,CompareSingleError_fn>	( pSorter, iMaxMatches, iSortFastPath );

	if ( bAllInt )
	{
		switch ( iSortFastPath )
		{
		case 1:
			if ( tState.m_uAttrDesc & 1 )
				return new ColumnarProxySorter_T<GENERIC,CompareInt_fn<1>,CompareSingleInt_fn<true>>  ( pSorter, iMaxMatches, iSortFastPath );
			 else
				return new ColumnarProxySorter_T<GENERIC,CompareInt_fn<1>,CompareSingleInt_fn<false>> ( pSorter, iMaxMatches, iSortFastPath );

		case 2: return new ColumnarProxySorter_T<GENERIC,CompareInt_fn<2>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
		case 3: return new ColumnarProxySorter_T<GENERIC,CompareInt_fn<3>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
		case 4: return new ColumnarProxySorter_T<GENERIC,CompareInt_fn<4>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
		case 5: return new ColumnarProxySorter_T<GENERIC,CompareInt_fn<5>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
		default: break;
		}
	}

	if ( bAllFloat )
	{
		switch ( iSortFastPath )
		{
		case 1:
			if ( tState.m_uAttrDesc & 1 )
				return new ColumnarProxySorter_T<GENERIC,CompareFloat_fn<1>,CompareSingleFloat_fn<true>>	( pSorter, iMaxMatches, iSortFastPath );
			else
				return new ColumnarProxySorter_T<GENERIC,CompareFloat_fn<1>,CompareSingleFloat_fn<false>>	( pSorter, iMaxMatches, iSortFastPath );

		case 2: return new ColumnarProxySorter_T<GENERIC,CompareFloat_fn<2>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
		case 3: return new ColumnarProxySorter_T<GENERIC,CompareFloat_fn<3>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
		case 4: return new ColumnarProxySorter_T<GENERIC,CompareFloat_fn<4>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
		case 5: return new ColumnarProxySorter_T<GENERIC,CompareFloat_fn<5>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
		default: break;
		}
	}

	switch ( iSortFastPath )
	{
	case 1: return new ColumnarProxySorter_T<GENERIC,Compare_fn<1>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
	case 2: return new ColumnarProxySorter_T<GENERIC,Compare_fn<2>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
	case 3: return new ColumnarProxySorter_T<GENERIC,Compare_fn<3>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
	case 4: return new ColumnarProxySorter_T<GENERIC,Compare_fn<4>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
	case 5: return new ColumnarProxySorter_T<GENERIC,Compare_fn<5>,CompareSingleError_fn> ( pSorter, iMaxMatches, iSortFastPath );
	default: break;
	}

	assert ( 0 && "Internal error" );
	return nullptr;
}


ISphMatchSorter * CreateColumnarProxySorter ( ISphMatchSorter * pSorter, int iMaxMatches, const ISphSchema & tSchema, const CSphMatchComparatorState & tState, ESphSortFunc eSortFunc, bool bNeedFactors, bool bComputeItems, bool bMulti )
{
	if ( !CanCreateColumnarSorter ( tSchema, tState, bNeedFactors, bComputeItems, bMulti ) )
		return pSorter;

	for ( int i = 0; i < CSphMatchComparatorState::MAX_ATTRS; i++ )
	{
		if ( tState.m_dAttrs[i]==-1 )
			continue;

		const CSphColumnInfo & tAttr = tSchema.GetAttr ( tState.m_dAttrs[i] );
		assert ( !tAttr.IsColumnar() );

		// proxy sorter will be evaluating this, so no need to evaluate this separately
		if ( tAttr.IsColumnarExpr() && tAttr.m_eStage==SPH_EVAL_PRESORT )
			const_cast<CSphColumnInfo &>(tAttr).m_eStage = SPH_EVAL_SORTER;
	}

	// at this point we have columnar attributes and non-attributes
	// if all sort-bys are attributes of the same type, we can use a fastpath
	int iSortFastPath;
	switch ( eSortFunc )
	{
	case FUNC_GENERIC1:	iSortFastPath = 1; break;
	case FUNC_GENERIC2:	iSortFastPath = 2; break;
	case FUNC_GENERIC3:	iSortFastPath = 3; break;
	case FUNC_GENERIC4:	iSortFastPath = 4; break;
	case FUNC_GENERIC5:	iSortFastPath = 5; break;
	default: iSortFastPath = 0;
	}

	bool bAllInt = true;
	bool bAllFloat = true;
	for ( int i = 0; i < iSortFastPath; i++ )
	{
		if ( tState.m_dAttrs[i]==-1 )
		{
			bAllInt = false;
			bAllFloat = false;
			break;
		}

		const CSphColumnInfo & tAttr = tSchema.GetAttr ( tState.m_dAttrs[i] );
		bool bInt = tAttr.m_eAttrType==SPH_ATTR_INTEGER || tAttr.m_eAttrType==SPH_ATTR_TIMESTAMP || tAttr.m_eAttrType==SPH_ATTR_BOOL || tAttr.m_eAttrType==SPH_ATTR_BIGINT;
		bool bFloat = tAttr.m_eAttrType==SPH_ATTR_FLOAT;
		bAllInt &= bInt;
		bAllFloat &= bFloat;
	}

	switch ( eSortFunc )
	{
	case FUNC_REL_DESC:		return CreateProxySorter<MatchRelevanceLt_fn>	( pSorter, iMaxMatches, tSchema, tState, iSortFastPath, bAllInt, bAllFloat );
	case FUNC_TIMESEGS:		return CreateProxySorter<MatchTimeSegments_fn>	( pSorter, iMaxMatches, tSchema, tState, iSortFastPath, bAllInt, bAllFloat );
	case FUNC_GENERIC1:		return CreateProxySorter<MatchGeneric1_fn>		( pSorter, iMaxMatches, tSchema, tState, iSortFastPath, bAllInt, bAllFloat );
	case FUNC_GENERIC2:		return CreateProxySorter<MatchGeneric2_fn>		( pSorter, iMaxMatches, tSchema, tState, iSortFastPath, bAllInt, bAllFloat );
	case FUNC_GENERIC3:		return CreateProxySorter<MatchGeneric3_fn>		( pSorter, iMaxMatches, tSchema, tState, iSortFastPath, bAllInt, bAllFloat );
	case FUNC_GENERIC4:		return CreateProxySorter<MatchGeneric4_fn>		( pSorter, iMaxMatches, tSchema, tState, iSortFastPath, bAllInt, bAllFloat );
	case FUNC_GENERIC5:		return CreateProxySorter<MatchGeneric5_fn>		( pSorter, iMaxMatches, tSchema, tState, iSortFastPath, bAllInt, bAllFloat );
	case FUNC_EXPR:			return CreateProxySorter<MatchExpr_fn>			( pSorter, iMaxMatches, tSchema, tState, iSortFastPath, bAllInt, bAllFloat );
	default:				return pSorter;
	}

	return pSorter;
}
