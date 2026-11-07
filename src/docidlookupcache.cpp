// Copyright (c) 2026, Manticore Software LTD (https://manticoresearch.com)
// Distributed under the GNU General Public License, version 3 or later.

#include "docidlookupcache.h"
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <new>
#if defined(__APPLE__)
#include <sys/sysctl.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

namespace
{
std::atomic<size_t> g_nLimit {0};
std::atomic<size_t> g_nUsed {0};

bool Reserve ( size_t nBytes ) noexcept
{
	size_t nUsed = g_nUsed.load(std::memory_order_relaxed);
	for (;;)
	{
		size_t nLimit = g_nLimit.load(std::memory_order_relaxed);
		if ( nBytes>nLimit || nUsed>nLimit-nBytes )
			return false;
		if ( g_nUsed.compare_exchange_weak ( nUsed, nUsed+nBytes, std::memory_order_relaxed ) )
			return true;
	}
}
}

size_t DocidLookupCache_c::GetAlignment() noexcept
{
	// Detect once when creating caches, never on the lookup path.
	static const size_t nAlignment = []() noexcept -> size_t {
		size_t nLine = 64;
#if defined(__APPLE__)
		size_t nLength = sizeof(nLine);
		if ( sysctlbyname ( "hw.cachelinesize", &nLine, &nLength, nullptr, 0 )!=0 )
			nLine = 64;
#elif defined(__linux__) && defined(_SC_LEVEL1_DCACHE_LINESIZE)
		long iLine = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
		if ( iLine>0 ) nLine = size_t(iLine);
#endif
		return nLine>=64 && nLine<=256 && !(nLine&(nLine-1)) ? nLine : 64;
	}();
	return nAlignment;
}

void DocidLookupCache_c::SetBudget ( size_t nBytes ) noexcept
{
	g_nLimit.store ( nBytes, std::memory_order_relaxed );
}

size_t DocidLookupCache_c::GetUsed() noexcept
{
	return g_nUsed.load(std::memory_order_relaxed);
}

DocidLookupCache_c::Ptr_t DocidLookupCache_c::Create ( uint32_t nDocs ) noexcept
{
	if ( !nDocs )
		return {};
	// uint64 arithmetic keeps the layout calculation safe on 32-bit targets.
	uint64_t nSlots = (uint64_t(nDocs)+LEAF_KEYS-1)/LEAF_KEYS*LEAF_KEYS;
	const size_t nAlignment = GetAlignment();
	// Keep a directory node within one cache line on the tested 64/128-byte
	// CPUs. Lookup selects a fixed-width specialization, not an inner-loop
	// runtime comparison count. Larger reported lines retain fanout 16.
	const size_t nFanout = nAlignment==64 ? 8 : 16;
	const size_t nHandleBytes = (sizeof(DocidLookupCache_c)+nAlignment-1)&~(nAlignment-1);
	uint64_t nBytes = nHandleBytes+nAlignment-1 + nSlots*(sizeof(uint64_t)+sizeof(uint32_t));
	uint64_t nLevelKeys[MAX_LEVELS];
	int nLevels = 0;
	for ( uint64_t nChildren=nSlots/LEAF_KEYS; nChildren>1; nChildren=(nChildren+nFanout-1)/nFanout )
	{
		if ( nLevels==MAX_LEVELS )
			return {};
		nLevelKeys[nLevels++] = nChildren;
		nBytes += (nChildren+nFanout-1)/nFanout*nFanout*sizeof(uint64_t);
	}
	if ( nBytes>SIZE_MAX || !Reserve((size_t)nBytes) )
		return {};
	void * pAllocation = malloc((size_t)nBytes);
	if ( !pAllocation )
	{
		g_nUsed.fetch_sub ( (size_t)nBytes, std::memory_order_relaxed );
		return {};
	}
	void * pAligned = pAllocation;
	size_t nSpace = (size_t)nBytes;
	std::align ( nAlignment, (size_t)nBytes-(nAlignment-1), pAligned, nSpace );
	auto * pCache = new (pAligned) DocidLookupCache_c;
	pCache->m_pAllocation = pAllocation;
	pCache->m_nBytes = (size_t)nBytes;
	pCache->m_nDocs = nDocs;
	pCache->m_nLevels = nLevels;
	pCache->m_nFanout = uint8_t(nFanout);
	pCache->m_pKeys = reinterpret_cast<uint64_t *>(static_cast<char *>(pAligned)+nHandleBytes);
	pCache->m_pRows = reinterpret_cast<uint32_t *>(pCache->m_pKeys+nSlots);
	auto * pLevelKeys = reinterpret_cast<uint64_t *>(pCache->m_pRows+nSlots);
	for ( int i=0; i<nLevels; ++i )
	{
		pCache->m_dLevels[i] = { pLevelKeys, (size_t)nLevelKeys[i] };
		pLevelKeys += (nLevelKeys[i]+nFanout-1)/nFanout*nFanout;
	}
	return Ptr_t(pCache);
}

void DocidLookupCache_c::Deleter_t::operator() ( DocidLookupCache_c * pCache ) const noexcept
{
	if ( !pCache )
		return;
	const size_t nBytes = pCache->m_nBytes;
	void * pAllocation = pCache->m_pAllocation;
	pCache->~DocidLookupCache_c();
	free(pAllocation);
	g_nUsed.fetch_sub ( nBytes, std::memory_order_relaxed );
}

bool DocidLookupCache_c::Append ( uint64_t uDocID, uint32_t uRowID ) noexcept
{
	if ( m_bSealed || m_nAdded==m_nDocs || ( m_nAdded && uDocID<=m_pKeys[m_nAdded-1] ) )
		return false;
	m_pKeys[m_nAdded] = uDocID;
	m_pRows[m_nAdded++] = uRowID;
	return true;
}

bool DocidLookupCache_c::Seal() noexcept
{
	if ( m_nAdded!=m_nDocs )
		return false;
	if ( m_bSealed )
		return true;
	size_t nSlots = (size_t(m_nDocs)+LEAF_KEYS-1)/LEAF_KEYS*LEAF_KEYS;
	std::fill ( m_pKeys+m_nDocs, m_pKeys+nSlots, UINT64_MAX );
	std::fill ( m_pRows+m_nDocs, m_pRows+nSlots, UINT32_MAX );
	const size_t nFanout = size_t(m_nFanout);
	for ( int i=0; i<m_nLevels; ++i )
	{
		auto & tLevel = m_dLevels[i];
		for ( size_t j=0; j<tLevel.m_nKeys; ++j )
			tLevel.m_pKeys[j] = i
				? m_dLevels[i-1].m_pKeys[std::min ( (j+1)*nFanout, m_dLevels[i-1].m_nKeys )-1]
				: m_pKeys[std::min ( (j+1)*LEAF_KEYS, size_t(m_nDocs) )-1];
		size_t nPadded = (tLevel.m_nKeys+nFanout-1)/nFanout*nFanout;
		std::fill ( tLevel.m_pKeys+tLevel.m_nKeys, tLevel.m_pKeys+nPadded, UINT64_MAX );
	}
	m_bSealed = true;
	return true;
}

template <int FANOUT>
void DocidLookupCache_c::FindBatchImpl ( const uint64_t * pDocIDs, uint32_t * pRows, size_t nQueries ) const noexcept
{
	size_t iBase = 0;
	if ( m_bSealed && nQueries>=BATCH_SIZE )
	{
		for ( ; nQueries-iBase>=BATCH_SIZE; iBase+=BATCH_SIZE )
		{
			// Each call owns its scratch; both directory specializations use
			// stack arrays without reserving storage for every OS thread.
			uint64_t dIDs[BATCH_SIZE];
			size_t dOutput[BATCH_SIZE], dNode[BATCH_SIZE] {}, dKey[BATCH_SIZE] {};
			// Interleave independent probes to expose memory parallelism.
			size_t nActive = 0;
			for ( int j=0; j<BATCH_SIZE; ++j )
			{
				uint64_t uID = pDocIDs[iBase+j];
				pRows[iBase+j] = UINT32_MAX;
				if ( uID>=m_pKeys[0] && uID<=m_pKeys[m_nDocs-1] )
				{
					dIDs[nActive] = uID;
					dOutput[nActive++] = iBase+j;
				}
			}
			for ( int i=m_nLevels-1; i>=0; --i )
				for ( size_t j=0; j<nActive; ++j )
				{
					const uint64_t * pKeys = m_dLevels[i].m_pKeys+dNode[j]*FANOUT;
					size_t iChild = 0;
					for ( int k=0; k<FANOUT; ++k )
						iChild += pKeys[k]<dIDs[j];
					dNode[j] = dNode[j]*FANOUT+iChild;
				}
			for ( int iStep=LEAF_KEYS/2; iStep; iStep/=2 )
				for ( size_t j=0; j<nActive; ++j )
					dKey[j] += m_pKeys[dNode[j]*LEAF_KEYS+dKey[j]+iStep-1]<dIDs[j] ? iStep : 0;
			for ( size_t j=0; j<nActive; ++j )
			{
				size_t iKey = dNode[j]*LEAF_KEYS+dKey[j];
				// As in Find, avoid a random row-array fetch for a missing ID.
				const size_t uHitMask = size_t(0)-size_t(m_pKeys[iKey]==dIDs[j]);
				pRows[dOutput[j]] = m_pRows[iKey&uHitMask] | uint32_t(~uHitMask);
			}
		}
	}
	// Small inputs and partial final tiles keep the scalar path. Find also
	// supplies INVALID for an unsealed cache without touching its key arrays.
	for ( ; iBase<nQueries; ++iBase )
		pRows[iBase] = Find(pDocIDs[iBase]);
}

void DocidLookupCache_c::FindBatch ( const uint64_t * pDocIDs, uint32_t * pRows, size_t nQueries ) const noexcept
{
	if ( m_nFanout==8 )
		FindBatchImpl<8> ( pDocIDs, pRows, nQueries );
	else
		FindBatchImpl<16> ( pDocIDs, pRows, nQueries );
}
