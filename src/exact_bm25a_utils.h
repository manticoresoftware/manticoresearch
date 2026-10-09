// Exact BM25A top-k bound helpers shared by the executor and focused tests.
#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

enum class E1RankedBoundKind_e : uint8_t
{
	NONE,
	BM25A_RATIO
};

inline const char * E1RankedBoundKindName ( E1RankedBoundKind_e eKind ) noexcept
{
	switch ( eKind )
	{
	case E1RankedBoundKind_e::BM25A_RATIO: return "bm25a_ratio";
	default: return "none";
	}
}

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
	uint64_t m_uRef = 0;
};


inline float E1RoundUp ( float fValue )
{
	return std::nextafter ( fValue, std::numeric_limits<float>::infinity() );
}

inline uint8_t E1EncodeBM25A12_075_256 ( uint32_t uTF, uint32_t uDL, bool bAuthoritative=true ) noexcept
{
	static constexpr uint64_t EXACT_FLOAT_LIMIT = uint64_t(1)<<24;
	if ( !uTF )
		return 0;
	if ( !bAuthoritative || uTF>EXACT_FLOAT_LIMIT || uDL>EXACT_FLOAT_LIMIT )
		return 255;
	const uint64_t uN = 2560ULL*uTF;
	const uint64_t uD = uN+768ULL+9ULL*uDL;
	const uint64_t uX = 254ULL*uN;
	uint64_t uCode = uX/uD + ( uX%uD!=0 );
	const uint64_t uGap = uCode*uD-uX;
	if ( uGap*EXACT_FLOAT_LIMIT < 7ULL*uX )
		++uCode;
	return uint8_t ( std::min<uint64_t>(uCode,254) );
}

inline float E1DecodeBM25A12_075_256 ( uint8_t uCode ) noexcept
{
	return uCode==255 ? std::numeric_limits<float>::infinity()
		: std::nextafter ( float(uCode)/254.0f, std::numeric_limits<float>::infinity() );
}

inline float E1SafeUpperRatioTerm ( uint8_t uCode, float fIDF )
{
	if ( !uCode || fIDF<=0.0f )
		return 0.0f;
	if ( uCode==255 )
		return E1RoundUp(fIDF);
	return E1RoundUp ( E1DecodeBM25A12_075_256(uCode)*fIDF );
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

inline int E1SafeUpperRatioWeight ( uint8_t uCode, float fIDF )
{
	return E1SafeUpperWeight ( E1SafeUpperRatioTerm(uCode,fIDF) );
}

inline bool E1RatioBoundReject ( uint8_t uCode, float fIDF, int iThreshold )
{
	// Saturation is an explicit fail-open escape. Non-positive IDF is outside
	// the persisted-ratio admission contract and must never prune.
	return uCode!=255 && fIDF>0.0f && iThreshold>0
		&& E1SafeUpperRatioWeight(uCode,fIDF)<iThreshold;
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
