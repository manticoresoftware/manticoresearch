// Exact BM25A top-k bound helpers shared by the executor and focused tests.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

inline std::string E1NormalizeTokenSequence ( const char * sText )
{
	std::string sNormalized;
	if ( !sText )
		return sNormalized;
	for ( unsigned char c : std::string_view(sText) )
		if ( c!=' ' && c!='	' && c!='\r' && c!='\n' )
			sNormalized.push_back ( c>='A' && c<='Z' ? char(c-'A'+'a') : char(c) );
	return sNormalized;
}

inline bool E1ExactRankerAdmission ( bool bNamedBM25A, bool bExpressionRanker, const char * sExpression )
{
	return bNamedBM25A || ( bExpressionRanker && E1NormalizeTokenSequence(sExpression)=="1000*bm25a(1.2,0.75,256)" );
}

inline bool E1FixedBM25AGenericFallback ( bool bNamedBM25A, bool bNeedPackedFactors )
{
	return bNamedBM25A && !bNeedPackedFactors;
}

// Keep the query-owned exact heap bounded. Cover practical deep pages whose
// LIMIT+OFFSET slightly exceeds max_matches=2000 while larger pages retain
// generic sorting.
static constexpr int E1_MAX_DYNAMIC_TOPK = 4096;

// Prove that an explicit query mask names every schema field and no field
// outside the schema. The caller handles unscoped queries from parser state.
template <typename MASK>
inline bool E1MaskMatchesExpected ( const MASK & tMask, const MASK & tExpected )
{
	for ( int iWord=0; iWord<MASK::SIZE; ++iWord )
		if ( uint32_t(tMask[iWord])!=uint32_t(tExpected[iWord]) )
			return false;
	return true;
}

template <typename MASK>
inline bool E1ExplicitMaskCoversSchema ( const MASK & tMask, int iFieldCount )
{
	if ( iFieldCount<=0 || iFieldCount>MASK::SIZE*32 )
		return false;
	for ( int iWord=0; iWord<MASK::SIZE; ++iWord )
	{
		const int iFirstField = iWord*32;
		uint32_t uExpected = 0;
		if ( iFirstField<iFieldCount )
		{
			const int iFieldsHere = std::min ( 32, iFieldCount-iFirstField );
			uExpected = iFieldsHere==32 ? UINT32_MAX : ( uint32_t(1)<<iFieldsHere )-1;
		}
		if ( uint32_t(tMask[iWord])!=uExpected )
			return false;
	}
	return true;
}

inline int E1ScopedSingleField ( int iSchemaFields, bool bFullSchemaScope, uint32_t uMask )
{
	if ( bFullSchemaScope || iSchemaFields<2 || !uMask || (uMask&(uMask-1)) )
		return -1;
	const int iField = __builtin_ctz(uMask);
	return iField<iSchemaFields ? iField : -1;
}

inline int E1SameScopedField2 ( int iSchemaFields, bool bLeftFullSchema, uint32_t uLeftMask, bool bRightFullSchema, uint32_t uRightMask )
{
	const int iLeft = E1ScopedSingleField ( iSchemaFields, bLeftFullSchema, uLeftMask );
	const int iRight = E1ScopedSingleField ( iSchemaFields, bRightFullSchema, uRightMask );
	return iLeft>=0 && iLeft==iRight ? iLeft : -1;
}

inline int E1ScopedAnd2Field ( int iSchemaFields, bool bLeftFullSchema, uint32_t uLeftMask, bool bRightFullSchema, uint32_t uRightMask )
{
	return E1SameScopedField2 ( iSchemaFields, bLeftFullSchema, uLeftMask, bRightFullSchema, uRightMask );
}

inline bool E1MixedScopedAnd2Fields ( int iSchemaFields, bool bLeftFullSchema, uint32_t uLeftMask, bool bRightFullSchema, uint32_t uRightMask, int & iLeftField, int & iRightField )
{
	iLeftField = E1ScopedSingleField ( iSchemaFields, bLeftFullSchema, uLeftMask );
	iRightField = E1ScopedSingleField ( iSchemaFields, bRightFullSchema, uRightMask );
	return iLeftField>=0 && iRightField>=0 && iLeftField!=iRightField;
}

inline int E1ScopedOr2Field ( int iSchemaFields, bool bLeftFullSchema, uint32_t uLeftMask, bool bRightFullSchema, uint32_t uRightMask )
{
	return E1SameScopedField2 ( iSchemaFields, bLeftFullSchema, uLeftMask, bRightFullSchema, uRightMask );
}

inline bool E1MixedScopedOr2Fields ( int iSchemaFields, bool bLeftFullSchema, uint32_t uLeftMask, bool bRightFullSchema, uint32_t uRightMask, int & iLeftField, int & iRightField )
{
	iLeftField = E1ScopedSingleField ( iSchemaFields, bLeftFullSchema, uLeftMask );
	iRightField = E1ScopedSingleField ( iSchemaFields, bRightFullSchema, uRightMask );
	return iLeftField>=0 && iRightField>=0 && iLeftField!=iRightField;
}

inline uint32_t E1ScopedExactTF ( uint32_t uAggregateTF, uint32_t uMask, int iField, uint32_t uDecodedTF )
{
	const uint32_t uFieldMask = uint32_t(1)<<iField;
	if ( !(uMask&uFieldMask) )
		return 0;
	return uMask==uFieldMask ? uAggregateTF : uDecodedTF;
}

inline bool E1ScopedAggregateBoundSafe ( uint32_t uScopedTF, uint32_t uAggregateTF )
{
	return uScopedTF<=uAggregateTF;
}

inline int E1RankedTopKCapacity ( int iTopK )
{
	return iTopK>0 && iTopK<=E1_MAX_DYNAMIC_TOPK ? iTopK : 0;
}

inline int E1RankedTopKFromPage ( int iOffset, int iLimit )
{
	if ( iOffset<0 || iLimit<=0 )
		return 0;
	const int64_t iTopK = int64_t(iOffset)+int64_t(iLimit);
	return iTopK<=E1_MAX_DYNAMIC_TOPK ? int(iTopK) : 0;
}

struct E1SelectedMeta_t
{
	uint32_t m_uLocal = 0;
	uint32_t m_uOrdinal = 0;
	uint32_t m_uTF = 0;
	uint32_t m_uMask = 0;
	uint32_t m_uScopedTF = 0;
	uint64_t m_uRef = 0;
};

// Compact reconstruction of persisted ordinal-64 bound runs for one direct
// 64-row word. Two runs suffice because a word contains at most 64 postings.
struct E1DirectBoundWord_t
{
	uint64_t m_dMasks[2] {};
	uint8_t m_dBounds[2] {};
	uint8_t m_uClasses = 0;
};

inline void E1BuildDirectBoundWord ( uint64_t uMembers, uint32_t uOrdinal, const uint8_t * pOrdinalBounds, E1DirectBoundWord_t & tWord )
{
	tWord = {};
	if ( !uMembers )
		return;
	const uint32_t uCount = uint32_t(__builtin_popcountll(uMembers));
	const uint32_t uFirst = std::min ( uCount, 64u-(uOrdinal&63) );
	tWord.m_dBounds[0] = pOrdinalBounds[0];
	if ( uFirst==uCount )
	{
		tWord.m_dMasks[0] = uMembers;
		tWord.m_uClasses = 1;
		return;
	}
	uint32_t uLo = 0, uHi = 64;
	while ( uLo<uHi )
	{
		const uint32_t uMid = (uLo+uHi)/2;
		const uint64_t uPrefix = uMid==64 ? ~uint64_t(0) : (uint64_t(1)<<uMid)-1;
		if ( uint32_t(__builtin_popcountll(uMembers&uPrefix))<uFirst )
			uLo = uMid+1;
		else
			uHi = uMid;
	}
	const uint64_t uPrefix = uLo==64 ? ~uint64_t(0) : (uint64_t(1)<<uLo)-1;
	tWord.m_dMasks[0] = uMembers&uPrefix;
	tWord.m_dMasks[1] = uMembers&~uPrefix;
	tWord.m_dBounds[1] = pOrdinalBounds[1];
	tWord.m_uClasses = 2;
}

inline uint8_t E1DirectBoundForBit ( const E1DirectBoundWord_t & tWord, uint32_t uBit )
{
	const uint64_t uMask = uint64_t(1)<<uBit;
	return tWord.m_dMasks[0]&uMask ? tWord.m_dBounds[0] : tWord.m_dBounds[1];
}

inline float E1RoundUp ( float fValue )
{
	return std::nextafter ( fValue, std::numeric_limits<float>::infinity() );
}

inline float E1SafeUpperTerm ( uint32_t uTF, float fIDF )
{
	if ( !uTF || fIDF<=0.0f )
		return 0.0f;
	const float fTF = float(uTF);
	const float fRatio = E1RoundUp ( fTF/(fTF+0.3f) );
	return E1RoundUp ( fRatio*fIDF );
}

inline float E1SafeUpperSaturatedTerm ( float fIDF )
{
	return fIDF>0.0f ? E1RoundUp(fIDF) : 0.0f;
}

inline float E1SafeUpperAdd ( float fSum, float fTerm )
{
	return E1RoundUp ( fSum+fTerm );
}

inline int E1SafeUpperWeight ( float fUpperSum )
{
	const float fShifted = E1RoundUp ( fUpperSum+0.5f );
	const float fScaled = E1RoundUp ( 1000.0f*fShifted );
	return fScaled>=float(std::numeric_limits<int>::max()) ? std::numeric_limits<int>::max() : int(fScaled);
}

inline bool E1TieAwareBoundReject ( int iUpperWeight, uint64_t uCandidateRow, int iThreshold, uint64_t uWorstTiedRow )
{
	if ( iThreshold<=0 )
		return false;
	if ( iUpperWeight!=iThreshold )
		return iUpperWeight<iThreshold;
	return uCandidateRow>=uWorstTiedRow;
}

// Query-local exact safe-bound table for the two supported phrase shapes.
// Values outside the compact table use the identical directed-up calculation.
class E1PhraseBoundLookup_c
{
public:
	static constexpr uint32_t MAX_TF = 255;

	void Build ( const float * pIDF, const int * pCanonical, const bool * pPhrase, int iTerms )
	{
		m_iTerms = iTerms;
		for ( int i=0; i<iTerms; ++i )
		{
			m_dIDF[i] = pIDF[i];
			m_dCanonical[i] = pCanonical[i];
			m_dPhrase[i] = pPhrase[i];
		}
		const size_t iSize = iTerms==2 ? MAX_TF+1 : size_t(MAX_TF+1)*(MAX_TF+1);
		m_dWeights.resize(iSize);
		for ( uint32_t uOccurrence=0; uOccurrence<=MAX_TF; ++uOccurrence )
			for ( uint32_t uMandatory=0; uMandatory<=(iTerms==3 ? MAX_TF : 0); ++uMandatory )
				m_dWeights[Index(uOccurrence,uMandatory)] = Calculate(uOccurrence,uMandatory);
	}

	int Weight ( uint32_t uOccurrence, uint32_t uMandatory, bool & bHit ) const
	{
		bHit = uOccurrence<=MAX_TF && ( m_iTerms==2 || uMandatory<=MAX_TF );
		return bHit ? m_dWeights[Index(uOccurrence,uMandatory)] : Calculate(uOccurrence,uMandatory);
	}

private:
	size_t Index ( uint32_t uOccurrence, uint32_t uMandatory ) const
	{
		return m_iTerms==2 ? uOccurrence : size_t(uOccurrence)*(MAX_TF+1)+uMandatory;
	}
	int Calculate ( uint32_t uOccurrence, uint32_t uMandatory ) const
	{
		float fUpper = 0.0f;
		for ( int i=0; i<m_iTerms; ++i )
		{
			const int iNode = m_dCanonical[i];
			const uint32_t uTF = m_dPhrase[iNode] ? uOccurrence : uMandatory;
			fUpper = E1SafeUpperAdd ( fUpper, E1SafeUpperTerm ( uTF, m_dIDF[iNode] ) );
		}
		return E1SafeUpperWeight(fUpper);
	}

	int m_iTerms = 0;
	float m_dIDF[3] {};
	int m_dCanonical[3] {};
	bool m_dPhrase[3] {};
	std::vector<int> m_dWeights;
};

inline int E1PartialUpperWeight ( const uint32_t dExactTF[4], uint32_t uExactMask, const uint8_t dBounds[4], const float dIDF[4], const int dCanonical[4], int iTerms )
{
	float fUpper = 0.0f;
	for ( int iCanonical=0; iCanonical<iTerms; ++iCanonical )
	{
		const int iNode = dCanonical[iCanonical];
		const uint32_t uTF = uExactMask & ( uint32_t(1)<<iNode ) ? dExactTF[iNode] : dBounds[iNode];
		const float fTerm = !( uExactMask & ( uint32_t(1)<<iNode ) ) && uTF==255
			? E1SafeUpperSaturatedTerm ( dIDF[iNode] )
			: E1SafeUpperTerm ( uTF, dIDF[iNode] );
		fUpper = E1SafeUpperAdd ( fUpper, fTerm );
	}
	return E1SafeUpperWeight ( fUpper );
}

inline bool E1AdmitAfterExactTF ( int iExactNode, uint32_t uExactTF, const uint8_t dBounds[4], const float dIDF[4], const int dCanonical[4], int iTerms, int iThreshold, uint64_t uCandidateRow, uint64_t uWorstTiedRow )
{
	uint32_t dExactTF[4] {};
	dExactTF[iExactNode] = uExactTF;
	return !E1TieAwareBoundReject ( E1PartialUpperWeight ( dExactTF, uint32_t(1)<<iExactNode, dBounds, dIDF, dCanonical, iTerms ), uCandidateRow, iThreshold, uWorstTiedRow );
}

// A direct AND word has at most two compact ordinal-64 bound runs per term.
// Refine the candidate mask one term at a time so AND4 stays bounded at 2^4
// small classes instead of building an unbounded Cartesian data structure.
static constexpr uint32_t E1_AND_MAX_BOUND_CLASSES = 16;

struct E1AndBoundClass_t
{
	uint64_t m_uMask = 0;
	float m_fUpper = 0.0f;
};

inline uint64_t E1AdmitAndBoundMasks ( uint64_t uCandidates, const uint64_t dTermClassMasks[4][2], const uint8_t dTermClassBounds[4][2], const uint32_t dTermClassCounts[4], const float dIDF[4], const int dCanonical[4], int iTerms, uint64_t uFirstRow, int iThreshold, uint64_t uWorstTiedRow, uint64_t & uRejectedRows, uint64_t & uTieRejectedRows, uint64_t & uRejectedClasses, uint64_t * pClassCombinations=nullptr )
{
	E1AndBoundClass_t dCurrent[E1_AND_MAX_BOUND_CLASSES], dNext[E1_AND_MAX_BOUND_CLASSES];
	uint32_t uCurrent = 1;
	dCurrent[0].m_uMask = uCandidates;
	for ( int iCanonical=0; iCanonical<iTerms; ++iCanonical )
	{
		const int iNode = dCanonical[iCanonical];
		uint32_t uNext = 0;
		for ( uint32_t iClass=0; iClass<uCurrent; ++iClass )
			for ( uint32_t iTermClass=0; iTermClass<dTermClassCounts[iNode]; ++iTermClass )
			{
				const uint64_t uRows = dCurrent[iClass].m_uMask & dTermClassMasks[iNode][iTermClass];
				if ( !uRows )
					continue;
				if ( pClassCombinations )
					++*pClassCombinations;
				if ( uNext>=E1_AND_MAX_BOUND_CLASSES )
					return uCandidates; // fail open; never prune from an incomplete refinement
				const uint8_t uBound = dTermClassBounds[iNode][iTermClass];
				const float fTerm = uBound==255 ? E1SafeUpperSaturatedTerm(dIDF[iNode]) : E1SafeUpperTerm(uBound,dIDF[iNode]);
				dNext[uNext++] = { uRows, E1SafeUpperAdd ( dCurrent[iClass].m_fUpper, fTerm ) };
			}
		if ( !uNext )
			return uCandidates; // malformed/missing coverage: conservative admission
		for ( uint32_t i=0; i<uNext; ++i )
			dCurrent[i] = dNext[i];
		uCurrent = uNext;
	}

	uint64_t uAdmitted = 0;
	for ( uint32_t iClass=0; iClass<uCurrent; ++iClass )
	{
		const uint64_t uRows = dCurrent[iClass].m_uMask;
		const int iUpperWeight = E1SafeUpperWeight ( dCurrent[iClass].m_fUpper );
		if ( iThreshold<=0 || iUpperWeight>iThreshold )
		{
			uAdmitted |= uRows;
			continue;
		}
		if ( iUpperWeight<iThreshold || uWorstTiedRow<=uFirstRow )
		{
			const uint64_t uCount = uint64_t(__builtin_popcountll(uRows));
			uRejectedRows += uCount;
			uTieRejectedRows += iUpperWeight==iThreshold ? uCount : 0;
			++uRejectedClasses;
			continue;
		}
		const uint32_t uWinningRows = uint32_t ( uWorstTiedRow-uFirstRow>=64 ? 64 : uWorstTiedRow-uFirstRow );
		const uint64_t uWinningMask = uWinningRows==64 ? ~uint64_t(0) : ( uint64_t(1)<<uWinningRows )-1;
		const uint64_t uWinning = uRows & uWinningMask;
		const uint64_t uRejected = uRows & ~uWinningMask;
		uAdmitted |= uWinning;
		uRejectedRows += uint64_t(__builtin_popcountll(uRejected));
		uTieRejectedRows += uint64_t(__builtin_popcountll(uRejected));
		if ( !uWinning )
			++uRejectedClasses;
	}
	return uAdmitted;
}
