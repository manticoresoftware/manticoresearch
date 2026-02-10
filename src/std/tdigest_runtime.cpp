#include "std/tdigest_runtime.h"

#include "attribute.h"

#include <cstring>

TDigestRuntimeState_t::TDigestRuntimeState_t ( double fCompression )
	: m_tDigest ( fCompression )
{}

TDigestRuntimeState_t::TDigestRuntimeState_t ( const TDigestRuntimeState_t & rhs )
	: m_uMagic ( RUNTIME_MAGIC )
	, m_tDigest ( rhs.m_tDigest )
	, m_dPending ( rhs.m_dPending )
{}

TDigestRuntimeState_t & TDigestRuntimeState_t::operator= ( const TDigestRuntimeState_t & rhs )
{
	if ( this==&rhs )
		return *this;

	m_uMagic = RUNTIME_MAGIC;
	m_tDigest = rhs.m_tDigest;
	m_dPending = rhs.m_dPending;
	return *this;
}

TDigestRuntimeAlloc_t sphTDigestRuntimeAllocate ( double fCompression )
{
	BYTE * pPayload = nullptr;
	BYTE * pPacked = sphPackPtrAttr ( (int)sizeof ( TDigestRuntimeState_t ), &pPayload );
	auto * pState = new ( pPayload ) TDigestRuntimeState_t ( fCompression );
	return { pState, pPacked };
}

TDigestRuntimeAlloc_t sphTDigestRuntimeCloneState ( const TDigestRuntimeState_t & tState )
{
	auto tAlloc = sphTDigestRuntimeAllocate ( tState.m_tDigest.GetCompression() );
	tAlloc.m_pState->m_tDigest = tState.m_tDigest;
	tAlloc.m_pState->m_dPending = tState.m_dPending;
	return tAlloc;
}

TDigestRuntimeState_t * sphTDigestRuntimeAccess ( ByteBlob_t dBlob )
{
	if ( !dBlob.first || dBlob.second < (int)sizeof ( TDigestRuntimeState_t ) )
		return nullptr;

	auto * pState = reinterpret_cast<TDigestRuntimeState_t*>( const_cast<BYTE*>( dBlob.first ) );
	return ( pState->m_uMagic==TDigestRuntimeState_t::RUNTIME_MAGIC ) ? pState : nullptr;
}

bool sphIsTDigestRuntimeBlob ( ByteBlob_t dBlob )
{
	return sphTDigestRuntimeAccess ( dBlob )!=nullptr;
}

void sphDestroyTDigestRuntimeBlob ( ByteBlob_t dBlob )
{
	auto * pState = sphTDigestRuntimeAccess ( dBlob );
	if ( !pState )
		return;

	pState->~TDigestRuntimeState_t();
	pState->m_uMagic = 0;
}

BYTE * sphCloneTDigestRuntimeBlob ( ByteBlob_t dBlob )
{
	auto * pState = sphTDigestRuntimeAccess ( dBlob );
	if ( !pState )
		return sphPackPtrAttr ( dBlob );

	auto tAlloc = sphTDigestRuntimeCloneState ( *pState );
	return tAlloc.m_pPacked;
}


void sphTDigestLoadFromBlob ( ByteBlob_t dBlob, TDigest_c & tDigest, double fCompression )
{
	if ( tDigest.GetCompression()!=fCompression )
		tDigest.SetCompression ( fCompression );

	if ( !dBlob.first )
	{
		tDigest.Clear();
		tDigest.SetCompression ( fCompression );
		return;
	}

	const BYTE * pData = dBlob.first;
	int iCount = 0;
	memcpy ( &iCount, pData, sizeof(int) );
	pData += sizeof(int);

	double fMin = 0.0;
	double fMax = 0.0;
	memcpy ( &fMin, pData, sizeof(double) );
	pData += sizeof(double);
	memcpy ( &fMax, pData, sizeof(double) );
	pData += sizeof(double);

	const BYTE * pCentroids = pData;
	tDigest.ImportRaw ( pCentroids, iCount );
	tDigest.SetExtremes ( fMin, fMax, iCount>0 );
}

