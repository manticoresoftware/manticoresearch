// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.

#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>

// Immutable, bulk-built numeric lookup. Inspired by bbtree's contiguous keys
// and wide nodes; this is a native C++ implementation for sorted disk chunks.
// No mutation, node allocation or locking is needed after publication.
class alignas(64) DocidLookupCache_c
{
public:
	struct Deleter_t { void operator() ( DocidLookupCache_c * pCache ) const noexcept; };
	using Ptr_t = std::unique_ptr<DocidLookupCache_c,Deleter_t>;

	static size_t GetAlignment() noexcept;
	static void SetBudget ( size_t nBytes ) noexcept;
	static size_t GetUsed() noexcept;
	static Ptr_t Create ( uint32_t nDocs ) noexcept;
	bool Append ( uint64_t uDocID, uint32_t uRowID ) noexcept;
	bool Seal() noexcept;
	size_t GetLengthBytes() const noexcept { return m_nBytes; }
	static constexpr int BATCH_SIZE = 16;
	static bool IsSparse ( size_t nValues, uint64_t nDocs ) noexcept { return nValues<=nDocs/64; }
	void FindBatch ( const uint64_t * pDocIDs, uint32_t * pRows, size_t nQueries ) const noexcept;

	uint32_t Find ( uint64_t uDocID ) const noexcept
	{
		if ( !m_bSealed || uDocID<m_pKeys[0] || uDocID>m_pKeys[m_nDocs-1] )
			return UINT32_MAX;
		const size_t iNode = m_nFanout==8 ? FindLeaf<8>(uDocID) : FindLeaf<16>(uDocID);
		size_t iFirst = iNode*LEAF_KEYS;
		const uint64_t * pKeys = m_pKeys+iFirst;
		// The directory guarantees that this leaf contains a key >= uDocID.
		// Six conditional offsets find the lower bound in its 64 sorted keys.
		size_t iKey = 0;
		for ( int iStep=LEAF_KEYS/2; iStep; iStep/=2 )
			iKey += pKeys[iKey+iStep-1]<uDocID ? iStep : 0;
		// Avoid an unpredictable hit/miss branch. Redirect misses to the hot
		// first row instead of fetching an unrelated row cache line, then mask.
		const size_t uHitMask = size_t(0)-size_t(pKeys[iKey]==uDocID);
		return m_pRows[(iFirst+iKey)&uHitMask] | uint32_t(~uHitMask);
	}

private:
	static constexpr int LEAF_KEYS = 64;
	static constexpr int MAX_LEVELS = 9; // enough for all u32 document counts with fanout 8
	template <int FANOUT>
#if defined(__GNUC__)
	__attribute__((always_inline))
#endif
	size_t FindLeaf ( uint64_t uDocID ) const noexcept
	{
		size_t iNode = 0;
		for ( int i=m_nLevels-1; i>=0; --i )
		{
			const uint64_t * pKeys = m_dLevels[i].m_pKeys + iNode*FANOUT;
			// Keep the comparison count constant for compiler vectorization.
			size_t iChild = 0;
			for ( int j=0; j<FANOUT; ++j )
				iChild += pKeys[j]<uDocID;
			iNode = iNode*FANOUT+iChild;
		}
		return iNode;
	}

	template <int FANOUT>
	void FindBatchImpl ( const uint64_t * pDocIDs, uint32_t * pRows, size_t nQueries ) const noexcept;
	struct Level_t
	{
		uint64_t * m_pKeys {nullptr};
		size_t m_nKeys {0};
	};
	Level_t m_dLevels[MAX_LEVELS]; // bottom-up arrays of child maxima
	uint64_t * m_pKeys {nullptr};
	uint32_t * m_pRows {nullptr};
	void * m_pAllocation {nullptr};
	size_t m_nBytes {0};
	uint32_t m_nDocs {0};
	uint32_t m_nAdded {0};
	int m_nLevels {0};
	uint8_t m_nFanout {16};
	bool m_bSealed {false};
	DocidLookupCache_c() = default;
};
