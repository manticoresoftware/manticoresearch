#include "std/tdigest_runtime.h"

#include "attribute.h"

#include <cassert>
#include <cstring>
#include <limits>
#include <memory>

namespace
{

constexpr uint8_t TDIGEST_RUNTIME_BLOB_VERSION = 1;

struct TDigestRuntimeBlobHeader_t
{
	uint8_t m_uVersion = TDIGEST_RUNTIME_BLOB_VERSION;
	uint8_t m_uPad = 0;
	uint8_t m_uReserved[2] = { 0, 0 };
};

constexpr size_t RUNTIME_ALIGN = alignof ( TDigestRuntimeState_t );
static_assert ( RUNTIME_ALIGN <= std::numeric_limits<uint8_t>::max(), "runtime alignment must fit into pad byte" );

constexpr size_t RuntimePayloadReservation ()
{
	return sizeof ( TDigestRuntimeBlobHeader_t )
		+ sizeof ( TDigestRuntimeState_t )
		+ ( RUNTIME_ALIGN - 1 );
}

TDigestRuntimeState_t * AccessLegacyRuntime ( ByteBlob_t dBlob )
{
	if ( dBlob.second < (int)sizeof ( TDigestRuntimeState_t ) )
		return nullptr;

	auto * pState = reinterpret_cast<TDigestRuntimeState_t*>( const_cast<BYTE*>( dBlob.first ) );
	return ( pState && pState->m_uMagic==TDigestRuntimeState_t::RUNTIME_MAGIC ) ? pState : nullptr;
}

const TDigestRuntimeBlobHeader_t * GetRuntimeHeader ( ByteBlob_t dBlob )
{
	if ( dBlob.second < (int)sizeof ( TDigestRuntimeBlobHeader_t ) )
		return nullptr;

	const auto * pHeader = reinterpret_cast<const TDigestRuntimeBlobHeader_t*>( dBlob.first );
	if ( pHeader->m_uVersion!=TDIGEST_RUNTIME_BLOB_VERSION )
		return nullptr;

	if ( pHeader->m_uPad > ( RUNTIME_ALIGN - 1 ) )
		return nullptr;

	const size_t uRequired = sizeof ( TDigestRuntimeBlobHeader_t )
		+ pHeader->m_uPad
		+ sizeof ( TDigestRuntimeState_t );

	if ( dBlob.second < (int)uRequired )
		return nullptr;

	return pHeader;
}

TDigestRuntimeState_t * AccessHeaderRuntime ( ByteBlob_t dBlob )
{
	const auto * pHeader = GetRuntimeHeader ( dBlob );
	if ( !pHeader )
		return nullptr;

	const BYTE * pStateStart = reinterpret_cast<const BYTE*>( pHeader + 1 );
	const BYTE * pStateBytes = pStateStart + pHeader->m_uPad;

	return const_cast<TDigestRuntimeState_t*>( reinterpret_cast<const TDigestRuntimeState_t*>( pStateBytes ) );
}

TDigestRuntimeState_t * AccessRuntimeState ( ByteBlob_t dBlob )
{
	if ( auto * pLegacy = AccessLegacyRuntime ( dBlob ) )
		return pLegacy;
	return AccessHeaderRuntime ( dBlob );
}

}

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
	constexpr size_t uReservation = RuntimePayloadReservation ();
	BYTE * pPacked = sphPackPtrAttr ( (int)uReservation, &pPayload );

	auto * pHeader = new ( pPayload ) TDigestRuntimeBlobHeader_t();
	BYTE * pStateArena = reinterpret_cast<BYTE*>( pHeader + 1 );
	size_t uSpace = uReservation - sizeof ( TDigestRuntimeBlobHeader_t );
	void * pAligned = pStateArena;

	bool bAligned = std::align ( RUNTIME_ALIGN, sizeof ( TDigestRuntimeState_t ), pAligned, uSpace );
	assert ( bAligned && "Failed to align TDigest runtime state" );

	size_t uPad = reinterpret_cast<BYTE*>( pAligned ) - pStateArena;
	assert ( uPad <= std::numeric_limits<uint8_t>::max() );
	pHeader->m_uPad = (uint8_t)uPad;

	auto * pState = new ( pAligned ) TDigestRuntimeState_t ( fCompression );
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
	if ( !dBlob.first )
		return nullptr;

	return AccessRuntimeState ( dBlob );
}

bool sphIsTDigestRuntimeBlob ( ByteBlob_t dBlob )
{
	if ( !dBlob.first )
		return false;

	if ( GetRuntimeHeader ( dBlob ) )
		return true;

	if ( dBlob.second < (int)sizeof ( TDigestRuntimeState_t ) )
		return false;

	uint32_t uMagic = 0;
	memcpy ( &uMagic, dBlob.first, sizeof ( uMagic ) );
	return uMagic==TDigestRuntimeState_t::RUNTIME_MAGIC;
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

