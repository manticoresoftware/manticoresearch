//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#ifndef _distinct_
#define _distinct_

#include "sphinxsort.h"
#include "std/openhash.h"

#include "hyperloglog.h"

struct ValueWithCount_t
{
	SphAttr_t	m_tValue;
	int			m_iCount;

	inline bool operator < ( const ValueWithCount_t & rhs ) const
	{
		if ( m_tValue != rhs.m_tValue )
			return m_tValue < rhs.m_tValue;

		return m_iCount > rhs.m_iCount;
	}

	inline bool operator== ( const ValueWithCount_t & rhs ) const
	{
		return m_tValue == rhs.m_tValue;
	}
};


struct ValueWithGroup_t
{
public:
	SphGroupKey_t	m_tGroup;
	SphAttr_t		m_tValue;

	inline bool operator < ( const ValueWithGroup_t & rhs ) const
	{
		if ( m_tGroup!=rhs.m_tGroup ) return m_tGroup<rhs.m_tGroup;
		return m_tValue<rhs.m_tValue;
	}

	inline bool operator == ( const ValueWithGroup_t & rhs ) const
	{
		return m_tGroup==rhs.m_tGroup && m_tValue==rhs.m_tValue;
	}
};

struct ValueWithGroupCount_t
{
public:
	SphGroupKey_t	m_tGroup;
	SphAttr_t		m_tValue;
	int				m_iCount;

	ValueWithGroupCount_t () = default;

	ValueWithGroupCount_t ( SphGroupKey_t uGroup, SphAttr_t uValue, int iCount )
		: m_tGroup ( uGroup )
		, m_tValue ( uValue )
		, m_iCount ( iCount )
	{}

	inline bool operator < ( const ValueWithGroupCount_t & rhs ) const
	{
		if ( m_tGroup!=rhs.m_tGroup ) return m_tGroup<rhs.m_tGroup;
		if ( m_tValue!=rhs.m_tValue ) return m_tValue<rhs.m_tValue;
		return m_iCount>rhs.m_iCount;
	}

	inline bool operator == ( const ValueWithGroupCount_t & rhs ) const
	{
		return m_tGroup==rhs.m_tGroup && m_tValue==rhs.m_tValue;
	}
};

template <typename T>
class UniqGrouped_T : public CSphVector<T>
{
	using BASE = CSphVector<T>;

public:
					UniqGrouped_T () { BASE::Reserve ( 16384 ); }

	void			Add ( const ValueWithGroupCount_t & tValue );
	void			Sort();

	int				CountStart ( SphGroupKey_t & tOutGroup );	///< starting counting distinct values, returns count and group key (0 if empty)
	int				CountNext ( SphGroupKey_t & tOutGroup );	///< continues counting distinct values, returns count and group key (0 if done)
	void			Compact ( VecTraits_T<SphGroupKey_t> & dRemoveGroups );
	void			CopyTo (UniqGrouped_T & dRhs );
	void			Reset() { BASE::Resize(0); }
	void			SetAccuracy ( int iAccuracy ) {}

protected:
	int				m_iCountPos = 0;
	bool			m_bSorted = true;
};

template <typename T>
inline void UniqGrouped_T<T>::Add ( const ValueWithGroupCount_t & tValue )
{
	BASE::Add ( { tValue.m_tGroup, tValue.m_tValue } );
	m_bSorted = false;
}

template <>
inline void UniqGrouped_T<ValueWithGroupCount_t>::Add ( const ValueWithGroupCount_t & tValue )
{
	BASE::Add ( tValue );
	m_bSorted = false;
}

template <typename T>
void UniqGrouped_T<T>::Sort()
{
	if ( m_bSorted )
		return;

	BASE::Sort();
	m_bSorted = true;
}

template <typename T>
int UniqGrouped_T<T>::CountStart ( SphGroupKey_t & tOutGroup )
{
	m_iCountPos = 0;
	return CountNext ( tOutGroup );
}

template <typename T>
int UniqGrouped_T<T>::CountNext ( SphGroupKey_t & tOutGroup )
{
	assert ( m_bSorted );
	if ( m_iCountPos>=BASE::GetLength() )
		return 0;

	constexpr bool bNeedCount = std::is_same<T, ValueWithGroupCount_t>::value;

	auto tGroup = BASE::m_pData[m_iCountPos].m_tGroup;
	auto tValue = BASE::m_pData[m_iCountPos].m_tValue;

	int iCount = 1;
	if constexpr ( bNeedCount )
		iCount = BASE::m_pData[m_iCountPos].m_iCount;

	tOutGroup = tGroup;
	m_iCountPos++;
	while ( m_iCountPos<BASE::GetLength() && BASE::m_pData[m_iCountPos].m_tGroup==tGroup )
	{
		if constexpr ( bNeedCount )
		{
			// if we have similar values in both groups, we can safely assume that we have at least one duplicate
			int iAdd = BASE::m_pData[m_iCountPos].m_iCount;
			assert ( iAdd>0 );
			if ( BASE::m_pData[m_iCountPos].m_tValue==tValue )
				iAdd--;

			iCount += iAdd;
		}
		else
		{
			if ( BASE::m_pData[m_iCountPos].m_tValue!=tValue )
				iCount++;
		}

		tValue = BASE::m_pData[m_iCountPos].m_tValue;
		++m_iCountPos;
	}
	return iCount;
}

// works like 'uniq', and also throw away provided values to remove.
template <typename T>
void UniqGrouped_T<T>::Compact ( VecTraits_T<SphGroupKey_t>& dRemoveGroups )
{
	assert ( m_bSorted );
	if ( !BASE::GetLength() )
		return;

	dRemoveGroups.Sort();
	auto * pRemoveGroups = dRemoveGroups.begin ();
	auto iRemoveGroups = dRemoveGroups.GetLength ();

	auto pSrc = BASE::Begin();
	auto pDst = BASE::Begin();
	auto pEnd = BASE::End();

	// skip remove-groups which are not in my list
	while ( iRemoveGroups && (*pRemoveGroups)<pSrc->m_tGroup )
	{
		++pRemoveGroups;
		--iRemoveGroups;
	}

	for ( ; pSrc<pEnd; ++pSrc )
	{
		// check if this entry needs to be removed
		while ( iRemoveGroups && (*pRemoveGroups)<pSrc->m_tGroup )
		{
			++pRemoveGroups;
			--iRemoveGroups;
		}
		if ( iRemoveGroups && pSrc->m_tGroup==*pRemoveGroups )
			continue;

		// check if it's a dupe
		if ( pDst>BASE::Begin() && pDst[-1]==pSrc[0] )
			continue;

		*pDst++ = *pSrc;
	}

	assert ( pDst-BASE::Begin()<=BASE::GetLength() );
	BASE::m_iCount = pDst-BASE::Begin();
}

template <typename T>
void UniqGrouped_T<T>::CopyTo ( UniqGrouped_T & dRhs )
{
	if ( m_bSorted && dRhs.m_bSorted )
	{
		UniqGrouped_T dRes;
		dRes.MergeSorted ( dRhs, *this );
		dRes.m_bSorted = true;
		dRhs = std::move(dRes);
	}
	else
	{
		auto * pNewValues = dRhs.AddN ( BASE::GetLength() );
		memcpy ( pNewValues, BASE::Begin(), BASE::GetLengthBytes64() );
		dRhs.m_bSorted = false;
	}
}

/////////////////////////////////////////////////////////////////////

template <typename T>
class UniqSingle_T : public CSphVector<T>
{
	using BASE = CSphVector<T>;

public:
			UniqSingle_T() { BASE::Reserve ( 16384 ); }

	void	Add ( const ValueWithGroupCount_t & tValue );
	void	Sort();
	void	Compact();
	void	CopyTo ( UniqSingle_T & dRhs );
	void	Reset() { BASE::Resize(0); }
	int		CountDistinct();

protected:
	bool	m_bSorted = true;
};

template <typename T>
inline void UniqSingle_T<T>::Add ( const ValueWithGroupCount_t & tValue )
{
	BASE::Add ( tValue.m_tValue );
	m_bSorted = false;
}

template <>
inline void UniqSingle_T<ValueWithCount_t>::Add ( const ValueWithGroupCount_t & tValue )
{
	BASE::Add ( { tValue.m_tValue, tValue.m_iCount } );
	m_bSorted = false;
}

template <typename T>
void UniqSingle_T<T>::Sort()
{
	if ( m_bSorted )
		return;

	BASE::Sort();
	m_bSorted = true;
}

template <typename T>
void UniqSingle_T<T>::Compact()
{
	BASE::Uniq ( !m_bSorted );
	m_bSorted = true;
}

template <typename T>
void UniqSingle_T<T>::CopyTo ( UniqSingle_T & dRhs )
{
	if ( m_bSorted && dRhs.m_bSorted )
	{
		UniqSingle_T dRes;
		dRes.MergeSorted ( dRhs, *this );
		dRes.m_bSorted = true;
		dRhs = std::move(dRes);
	}
	else
	{
		auto * pNewValues = dRhs.AddN ( BASE::GetLength() );
		memcpy ( pNewValues, BASE::Begin(), BASE::GetLengthBytes64() );
		dRhs.m_bSorted = false;
	}
}

template <typename T>
int UniqSingle_T<T>::CountDistinct()
{
	constexpr bool bNeedCount = std::is_same<T, ValueWithCount_t>::value;

	Sort();
	auto & dArray = *this;
	int iCount = 1;
	if constexpr ( bNeedCount )
		iCount = dArray[0].m_iCount;

	for ( int i = 1, iLen = BASE::GetLength (); i<iLen; ++i )
	{
		if constexpr ( bNeedCount )
		{
			// if we have similar values in both groups, we can safely assume that we have at least one duplicate
			int iAdd = dArray[i].m_iCount;
			assert ( iAdd>0 );
			if ( dArray[i-1]==dArray[i] )
				iAdd--;

			iCount += iAdd;
		}
		else
		{
			if ( dArray[i-1]!=dArray[i] )
				iCount++;
		}
	}

	return iCount;
}


class UniqHLLTraits_c
{
public:
					UniqHLLTraits_c();
					~UniqHLLTraits_c() { Reset(); }

	UniqHLLTraits_c & operator = ( UniqHLLTraits_c && tRhs );

	void			Reset();
	void			SetAccuracy ( int iAccuracy );

protected:
	static const int SMALL_ARRAY_SIZE = 16;
	static const int MIN_HASH_SIZE = 64;
	static const int NON_PACKED_HLL_THRESH = 16;

	using HLLDensePacked_c		= HyperLogLogDense_T<>;
	using HLLDenseNonPacked_c	= HyperLogLogDense_T<uint64_t, FarmHash_T<uint64_t>, RegistersNonPacked_c>;
	using SmallArray_c			= LazyVector_T<SphAttr_t, sph::DefaultRelimit, SMALL_ARRAY_SIZE>;
	using Hash_c				= OpenHashSetFastClear_T<SphAttr_t>;

	enum class ContainterType_e
	{
		ARRAY,
		HASH,
		HLL_DENSE_PACKED,
		HLL_DENSE_NONPACKED,
	};

	struct Container_t
	{
		union
		{
			SmallArray_c *			m_pArray = nullptr;
			Hash_c *				m_pHash;
			HLLDensePacked_c *		m_pHLLDensePacked;
			HLLDenseNonPacked_c *	m_pHLLDenseNonPacked;
		};

		ContainterType_e	m_eType = ContainterType_e::ARRAY;
		int					m_iHashIdx = 0;

		int					Estimate() const;
		void				Reset();
		bool				IsEmpty() const;

		FORCE_INLINE bool FindInArray ( SphAttr_t tValue )
		{
			assert ( m_eType==ContainterType_e::ARRAY );
			for ( auto i : *m_pArray )
				if ( i==tValue )
					return true;

			return false;
		}
	};

	int				m_iAccuracy = 16;

	void			AddToContainer ( Container_t & tContainer, SphAttr_t tValue );

	FORCE_INLINE SmallArray_c *	AllocateArray()
	{
		if ( m_dUnusedArray.GetLength() )
		{
			SmallArray_c * pNew = m_dUnusedArray.Pop();
			pNew->Resize(0);
			return pNew;
		}

		return new SmallArray_c;
	}

	FORCE_INLINE void FreeContainer ( Container_t & tContainer )
	{
		switch ( tContainer.m_eType )
		{
		case ContainterType_e::ARRAY:				m_dUnusedArray.Add ( tContainer.m_pArray ); break;
		case ContainterType_e::HASH:				m_dUnusedHashes[tContainer.m_iHashIdx].Add ( tContainer.m_pHash ); break;
		case ContainterType_e::HLL_DENSE_PACKED:	m_dUnusedHLLDensePacked.Add ( tContainer.m_pHLLDensePacked ); break;
		case ContainterType_e::HLL_DENSE_NONPACKED:	m_dUnusedHLLDenseNonPacked.Add ( tContainer.m_pHLLDenseNonPacked ); break;
		default: assert ( 0 && "Unknown container type" ); break;
		}
	}

	void			ConvertToHash ( Container_t & tContainer );
	void			ConvertToHLLDensePacked ( Container_t & tContainer );
	void			ConvertToHLLDenseNonPacked ( Container_t & tContainer );

	template <typename T>
	void CopyContainerTo ( SphGroupKey_t tGroup, const Container_t & tFrom, Container_t & tTo, T & tRhs )
	{
		if ( tFrom.IsEmpty() )
			return;

		if ( tTo.IsEmpty() )
		{
			tTo.Reset();
			tTo = tFrom;
			tTo.m_eType = ContainterType_e::ARRAY;
			tTo.m_pArray = nullptr;
		}
		else
		{
			if ( tFrom.m_eType==ContainterType_e::ARRAY )
			{
				for ( auto i : *tFrom.m_pArray )
					tRhs.Add ( { tGroup, i, 1 } );
			}
			else if ( tFrom.m_eType==ContainterType_e::HASH )
			{
				int64_t i = 0;
				SphAttr_t * pRes;
				while ( ( pRes = tFrom.m_pHash->Iterate(i) ) != nullptr )
					tRhs.Add ( { tGroup, *pRes, 1 } );
			}
			else
			{
				if ( tTo.m_eType==ContainterType_e::ARRAY )
					tRhs.ConvertToHash ( tTo );

				if ( tTo.m_eType==ContainterType_e::HASH )
				{
					assert ( m_iAccuracy==tRhs.m_iAccuracy );

					if ( m_iAccuracy > NON_PACKED_HLL_THRESH )
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
	}

private:
	CSphVector<SmallArray_c *>			m_dUnusedArray;
	CSphVector<CSphVector<Hash_c *>>	m_dUnusedHashes;
	CSphVector<HLLDensePacked_c *>		m_dUnusedHLLDensePacked;
	CSphVector<HLLDenseNonPacked_c *>	m_dUnusedHLLDenseNonPacked;

	int						m_iMaxHashSize = 0;

	HLLDensePacked_c *		AllocateHLLDensePacked();
	HLLDenseNonPacked_c *	AllocateHLLDenseNonPacked();
	Hash_c *				AllocateHash ( int iIdx );
	void					MoveToLargerHash ( Container_t & tContainer );
};

class UniqHLL_c : public UniqHLLTraits_c
{
	using BASE = UniqHLLTraits_c;

public:
					~UniqHLL_c() { Reset(); }

	UniqHLL_c &		operator = ( UniqHLL_c && tRhs );

	FORCE_INLINE void Add ( const ValueWithGroupCount_t & tValue );
	void			Sort() {}
	int				CountStart ( SphGroupKey_t & tOutGroup );
	int				CountNext ( SphGroupKey_t & tOutGroup );
	void			Compact ( VecTraits_T<SphGroupKey_t>& dRemoveGroups );
	void			Reset();
	void			CopyTo ( UniqHLL_c & tRhs );

private:
	OpenHashTable_T<SphGroupKey_t, Container_t> m_hGroups;
	int64_t			m_iHashIterator = 0;
};


void UniqHLL_c::Add ( const ValueWithGroupCount_t & tValue )
{
	Container_t * pContainer = m_hGroups.Find ( tValue.m_tGroup );
	if ( pContainer )
		AddToContainer ( *pContainer, tValue.m_tValue );
	else
	{
		Container_t tContainer;
		tContainer.m_pArray = AllocateArray();
		tContainer.m_pArray->Add ( tValue.m_tValue );
		m_hGroups.Add ( tValue.m_tGroup, tContainer );
	}
}

class UniqHLLSingle_c : public UniqHLLTraits_c
{
	using BASE = UniqHLLTraits_c;

public:
				UniqHLLSingle_c() { m_tContainer.m_pArray = AllocateArray(); }
				~UniqHLLSingle_c() { Reset(); }

	UniqHLLSingle_c & operator = ( UniqHLLSingle_c && tRhs );

	inline void	Add ( const ValueWithGroupCount_t & tValue ) { AddToContainer ( m_tContainer, tValue.m_tValue ); }
	void		Compact() {}
	int			CountDistinct() { return m_tContainer.Estimate(); }
	void		CopyTo ( UniqHLLSingle_c & tRhs );
	void		Reset();

private:
	Container_t	m_tContainer;
};

#endif // _distinct_
