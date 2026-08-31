#pragma once

#include "std/blobs.h"
#include "std/tdigest.h"
#include "std/vector.h"

struct TDigestRuntimeState_t
{
	static constexpr uint32_t RUNTIME_MAGIC = 0x54444947; // 'TDIG'

	uint32_t	m_uMagic { RUNTIME_MAGIC };
	TDigest_c	m_tDigest;
	CSphVector<double> m_dPending;

	explicit TDigestRuntimeState_t ( double fCompression );
	TDigestRuntimeState_t ( const TDigestRuntimeState_t & rhs );
	TDigestRuntimeState_t & operator= ( const TDigestRuntimeState_t & rhs );
};

struct TDigestRuntimeAlloc_t
{
	TDigestRuntimeState_t *	m_pState = nullptr;
	BYTE *					m_pPacked = nullptr;
};

TDigestRuntimeAlloc_t	sphTDigestRuntimeAllocate ( double fCompression );
TDigestRuntimeAlloc_t	sphTDigestRuntimeCloneState ( const TDigestRuntimeState_t & tState );

TDigestRuntimeState_t *	sphTDigestRuntimeAccess ( ByteBlob_t dBlob );
bool					sphIsTDigestRuntimeBlob ( ByteBlob_t dBlob );
void					sphDestroyTDigestRuntimeBlob ( ByteBlob_t dBlob );
BYTE *					sphCloneTDigestRuntimeBlob ( ByteBlob_t dBlob );

/// Utility helpers shared between TDigest aggregation paths
void sphTDigestLoadFromBlob ( ByteBlob_t dBlob, TDigest_c & tDigest, double fCompression );
