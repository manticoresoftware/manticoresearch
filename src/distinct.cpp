//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "distinct.h"

template <typename HLL, typename HASH>
static void CopyHashToHLL ( HLL & tHLL, const HASH & tHash )
{
	int64_t i = 0;
	SphAttr_t * pRes;
	while ( ( pRes = tHash.Iterate(i) ) != nullptr )
		tHLL.Add ( (uint64_t)*pRes );
}

/////////////////////////////////////////////////////////////////////
UniqHLLTraits_c::Container_t::Container_t ( const Container_t & tRhs )
	: m_pArray ( tRhs.m_pArray )
	, m_eType ( tRhs.m_eType )
	, m_iHashIdx ( tRhs.m_iHashIdx )
{}


UniqHLLTraits_c::Container_t & UniqHLLTraits_c::Container_t::operator = ( Container_t && tRhs )
{
	m_pArray = std::move ( tRhs.m_pArray );
	m_eType = std::move ( tRhs.m_eType );
	m_iHashIdx = std::move ( tRhs.m_iHashIdx );

	tRhs.m_pArray = nullptr;
	tRhs.m_eType = ContainterType_e::ARRAY;
	tRhs.m_iHashIdx = 0;

	return *this;
}


UniqHLLTraits_c::Container_t & UniqHLLTraits_c::Container_t::operator = ( const Container_t & tRhs )
{
	// we assume that the caller is responsible for managing the copied pointers
	Container_t tNew(tRhs);
	*this = std::move(tNew);
	return *this;
}


int UniqHLLTraits_c::Container_t::Estimate() const
{
	switch ( m_eType )
	{
	case ContainterType_e::ARRAY:				return m_pArray->GetLength();	
	case ContainterType_e::HASH:				return m_pHash->GetLength();
	case ContainterType_e::HLL_DENSE_PACKED:	return int( m_pHLLDensePacked->Estimate() );
	case ContainterType_e::HLL_DENSE_NONPACKED:	return int( m_pHLLDenseNonPacked->Estimate() );
	default:
		assert ( 0 && "Unknown container type" );
		return 0;
	}
}


void UniqHLLTraits_c::Container_t::Reset()
{
	switch ( m_eType )
	{
	case ContainterType_e::ARRAY:				SafeDelete ( m_pArray ); break;
	case ContainterType_e::HASH:				SafeDelete ( m_pHash ); break;
	case ContainterType_e::HLL_DENSE_PACKED:	SafeDelete ( m_pHLLDensePacked ); break;
	case ContainterType_e::HLL_DENSE_NONPACKED:	SafeDelete ( m_pHLLDenseNonPacked ); break;
	default: assert ( 0 && "Unknown container type" ); break;
	}
}


bool UniqHLLTraits_c::Container_t::IsEmpty() const
{
	return m_eType==ContainterType_e::ARRAY && ( !m_pArray || m_pArray->IsEmpty() );
}


UniqHLLTraits_c::UniqHLLTraits_c()
{
	SetAccuracy ( m_iAccuracy );
}


void UniqHLLTraits_c::AddToContainer ( Container_t & tContainer, SphAttr_t tValue )
{
	switch ( tContainer.m_eType )
	{
	case ContainterType_e::ARRAY:
	{
		if ( tContainer.FindInArray(tValue) )
			break;

		if ( tContainer.m_pArray->GetLength() >= SMALL_ARRAY_SIZE )
		{
			ConvertToHash(tContainer);
			AddToContainer ( tContainer, tValue );
		}
		else
			tContainer.m_pArray->Add(tValue);				
	}
	break;

	case ContainterType_e::HASH:
	{
		auto & pHash = tContainer.m_pHash;
		if ( pHash->Find(tValue) )
			break;

		if ( !pHash->IsFull() )
		{
			pHash->Add(tValue);
			break;
		}

		if ( pHash->GetSize()==m_iMaxHashSize )
		{
			if ( m_iAccuracy > NON_PACKED_HLL_THRESH )
				ConvertToHLLDensePacked(tContainer);
			else
				ConvertToHLLDenseNonPacked(tContainer);

			AddToContainer ( tContainer, tValue );
		}
		else
		{
			MoveToLargerHash(tContainer);
			pHash->Add(tValue);
		}			
	}
	break;

	case ContainterType_e::HLL_DENSE_PACKED:
		tContainer.m_pHLLDensePacked->Add ( (uint64_t)tValue );
		break;

	case ContainterType_e::HLL_DENSE_NONPACKED:
		tContainer.m_pHLLDenseNonPacked->Add ( (uint64_t)tValue );
		break;

	default:
		assert ( 0 && "Unknown container type" );
		break;
	}
}


UniqHLLTraits_c::Hash_c * UniqHLLTraits_c::AllocateHash ( int iIdx )
{
	auto & dHashes = m_dUnusedHashes[iIdx];
	if ( dHashes.GetLength() )
	{
		Hash_c * pNew = dHashes.Pop();
		pNew->Clear();
		return pNew;
	}

	return new Hash_c ( 1 << ( iIdx + sphLog2const(MIN_HASH_SIZE) - 1 ) );
}


UniqHLLTraits_c::HLLDensePacked_c * UniqHLLTraits_c::AllocateHLLDensePacked()
{
	if ( m_dUnusedHLLDensePacked.GetLength() )
	{
		HLLDensePacked_c * pNew = m_dUnusedHLLDensePacked.Pop();
		pNew->Clear();
		return pNew;
	}

	return new HLLDensePacked_c ( m_iAccuracy );
}


UniqHLLTraits_c::HLLDenseNonPacked_c * UniqHLLTraits_c::AllocateHLLDenseNonPacked()
{
	if ( m_dUnusedHLLDenseNonPacked.GetLength() )
	{
		HLLDenseNonPacked_c * pNew = m_dUnusedHLLDenseNonPacked.Pop();
		pNew->Clear();
		return pNew;
	}

	return new HLLDenseNonPacked_c ( m_iAccuracy );
}


void UniqHLLTraits_c::ConvertToHash ( Container_t & tContainer )
{
	assert ( tContainer.m_eType==ContainterType_e::ARRAY );

	Hash_c * pHash = AllocateHash(0);
	assert(pHash);

	for ( const auto & i : *tContainer.m_pArray )
		pHash->Add(i);

	FreeContainer ( tContainer );

	tContainer.m_pHash = pHash;
	tContainer.m_iHashIdx = 0;
	tContainer.m_eType = ContainterType_e::HASH;
}


void UniqHLLTraits_c::ConvertToHLLDensePacked ( Container_t & tContainer )
{
	assert ( tContainer.m_eType==ContainterType_e::HASH );

	HLLDensePacked_c * pHLL = AllocateHLLDensePacked();
	assert ( pHLL && tContainer.m_pHash );

	CopyHashToHLL ( *pHLL, *tContainer.m_pHash );
	FreeContainer ( tContainer );

	tContainer.m_pHLLDensePacked = pHLL;
	tContainer.m_eType = ContainterType_e::HLL_DENSE_PACKED;
}


void UniqHLLTraits_c::ConvertToHLLDenseNonPacked ( Container_t & tContainer )
{
	assert ( tContainer.m_eType==ContainterType_e::HASH );

	HLLDenseNonPacked_c * pHLL = AllocateHLLDenseNonPacked();
	assert ( pHLL && tContainer.m_pHash );

	CopyHashToHLL ( *pHLL, *tContainer.m_pHash );
	FreeContainer ( tContainer );

	tContainer.m_pHLLDenseNonPacked = pHLL;
	tContainer.m_eType = ContainterType_e::HLL_DENSE_NONPACKED;
}


void UniqHLLTraits_c::MoveToLargerHash ( Container_t & tContainer )
{
	int & iIdx = tContainer.m_iHashIdx;
	Hash_c * pNewHash = AllocateHash ( iIdx + 1 );
	tContainer.m_pHash->MoveTo ( *pNewHash );
	m_dUnusedHashes[iIdx].Add ( tContainer.m_pHash );
	tContainer.m_pHash = pNewHash;
	iIdx++;
}


UniqHLLTraits_c & UniqHLLTraits_c::operator = ( UniqHLLTraits_c && tRhs )
{
	m_dUnusedArray = std::move ( tRhs.m_dUnusedArray );
	m_dUnusedHashes = std::move ( tRhs.m_dUnusedHashes );
	m_dUnusedHLLDensePacked = std::move ( tRhs.m_dUnusedHLLDensePacked );
	m_dUnusedHLLDenseNonPacked = std::move ( tRhs.m_dUnusedHLLDenseNonPacked );
	m_iMaxHashSize = std::move ( tRhs.m_iMaxHashSize );

	return *this;
}


void UniqHLLTraits_c::Reset()
{
	for ( auto i : m_dUnusedArray )
		SafeDelete(i);

	for ( auto & i : m_dUnusedHashes )
		for ( auto j : i )
			SafeDelete(j);

	for ( auto i : m_dUnusedHLLDensePacked )
		SafeDelete(i);

	for ( auto i : m_dUnusedHLLDenseNonPacked )
		SafeDelete(i);

	m_dUnusedArray.Resize(0);
	m_dUnusedHashes.Resize(0);
	m_dUnusedHLLDensePacked.Resize(0);
	m_dUnusedHLLDenseNonPacked.Resize(0);
}


void UniqHLLTraits_c::SetAccuracy ( int iAccuracy )
{
	m_iAccuracy = iAccuracy;
	m_iMaxHashSize = 1 << ( m_iAccuracy - 4 );

	for ( auto & i : m_dUnusedHashes )
		for ( auto j : i )
			SafeDelete(j);

	m_dUnusedHashes.Resize ( sphLog2 ( m_iMaxHashSize ) - sphLog2const ( MIN_HASH_SIZE ) + 1 );
}

UniqHLLTraits_c::Container_t & UniqHLL_c::Get ( SphGroupKey_t tGroup )
{
	int iLen = m_hGroups.GetLength();
	UniqHLLTraits_c::Container_t & tCont = m_hGroups.Acquire ( tGroup );

	// need reset value in case of the Container_t just added 
	if ( iLen!=m_hGroups.GetLength() )
		tCont = UniqHLLTraits_c::Container_t();

	return tCont;
}

template <typename T>
void CopyContainerTo ( SphGroupKey_t tGroup, const UniqHLLTraits_c::Container_t & tFrom, T & tRhs )
{
	if ( tFrom.IsEmpty() )
		return;

	UniqHLLTraits_c::Container_t & tTo = tRhs.Get ( tGroup );
	if ( tTo.IsEmpty() )
		tTo.m_pArray = tRhs.AllocateArray();

	if ( tFrom.m_eType==UniqHLLTraits_c::ContainterType_e::ARRAY )
	{
		for ( auto i : *tFrom.m_pArray )
			tRhs.Add ( { tGroup, i, 1 } );

	} else if ( tFrom.m_eType==UniqHLLTraits_c::ContainterType_e::HASH )
	{
		int64_t i = 0;
		SphAttr_t * pRes;
		while ( ( pRes = tFrom.m_pHash->Iterate(i) ) != nullptr )
			tRhs.Add ( { tGroup, *pRes, 1 } );

	} else
	{
		if ( tTo.m_eType==UniqHLLTraits_c::ContainterType_e::ARRAY )
			tRhs.ConvertToHash ( tTo );

		if ( tTo.m_eType==UniqHLLTraits_c::ContainterType_e::HASH )
		{
			assert ( tRhs.m_iAccuracy==tRhs.m_iAccuracy );

			if ( tRhs.m_iAccuracy > UniqHLLTraits_c::NON_PACKED_HLL_THRESH )
			{
				tRhs.ConvertToHLLDensePacked ( tTo );
				tTo.m_pHLLDensePacked->Merge ( *tFrom.m_pHLLDensePacked );
			}
			else
			{
				tRhs.ConvertToHLLDenseNonPacked ( tTo );
				tTo.m_pHLLDenseNonPacked->Merge ( *tFrom.m_pHLLDenseNonPacked );
			}
		}
	}
}


/////////////////////////////////////////////////////////////////////
UniqHLL_c &	UniqHLL_c::operator = ( UniqHLL_c && tRhs )
{
	BASE::operator = ( std::move(tRhs) );

	m_hGroups = std::move ( tRhs.m_hGroups );
	m_iHashIterator = std::move ( tRhs.m_iHashIterator );

	return *this;
}


int UniqHLL_c::CountStart ( SphGroupKey_t & tOutGroup )
{
	m_iHashIterator = 0;
	return CountNext ( tOutGroup );
}


int UniqHLL_c::CountNext ( SphGroupKey_t & tOutGroup )
{
	auto tRes = m_hGroups.Iterate ( m_iHashIterator );
	if ( !tRes.second )
		return 0;

	tOutGroup = tRes.first;
	assert ( tRes.second );
	return tRes.second->Estimate();
}


void UniqHLL_c::Compact ( VecTraits_T<SphGroupKey_t> & dRemoveGroups )
{
	for ( auto i : dRemoveGroups )
	{
		Container_t tContainer = m_hGroups.FindAndDelete(i);
		FreeContainer(tContainer);
	}
}


void UniqHLL_c::Reset()
{
	int64_t iIterator = 0;
	std::pair<SphGroupKey_t, Container_t*> tRes;
	while ( ( tRes = m_hGroups.Iterate ( iIterator ) ).second )
		tRes.second->Reset();

	m_hGroups.Reset(0);

	BASE::Reset();
}


void UniqHLL_c::CopyTo ( UniqHLL_c & tRhs ) const
{
	int64_t iIterator = 0;
	std::pair<SphGroupKey_t, Container_t*> tRes;
	while ( ( tRes = m_hGroups.Iterate ( iIterator ) ).second )
	{
		// can not move Container_t into dRhs as move invalidates this
		CopyContainerTo ( tRes.first, *tRes.second, tRhs );
	}
}

/////////////////////////////////////////////////////////////////////////////

UniqHLLSingle_c & UniqHLLSingle_c::operator = ( UniqHLLSingle_c && tRhs )
{
	BASE::operator = ( std::move(tRhs) );

	m_tContainer = std::move ( tRhs.m_tContainer );

	return *this;
}


void UniqHLLSingle_c::CopyTo ( UniqHLLSingle_c & tRhs ) const
{
	CopyContainerTo ( 0, m_tContainer, tRhs );
}


void UniqHLLSingle_c::Reset()
{
	m_tContainer.Reset();
	BASE::Reset();
}
