//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "ints.h"
#include "mm.h"
#include "mem.h"
#include "string.h"

#include <cassert>


namespace detail {
#if SPH_ALLOCS_PROFILER
inline void MemStatMMapAdd ( int64_t iSize ) { sphMemStatMMapAdd(iSize); }
inline void MemStatMMapDel ( int64_t iSize ) { sphMemStatMMapDel(iSize); }
#else
inline void MemStatMMapAdd ( int64_t ) {}
inline void MemStatMMapDel ( int64_t ) {}
#endif
};

template<typename T, bool SHARED>
CSphLargeBuffer<T,SHARED>::~CSphLargeBuffer()
{
	this->Reset();
}

/// allocate storage
template<typename T, bool SHARED>
bool CSphLargeBuffer<T, SHARED>::Alloc ( int64_t iEntries, CSphString& sError )
{
	assert ( !this->GetWritePtr() );

	int64_t uCheck = sizeof ( T );
	uCheck *= iEntries;

	int64_t iLength = (size_t)uCheck;
	if ( uCheck != iLength )
	{
		sError.SetSprintf ( "impossible to mmap() over 4 GB on 32-bit system" );
		return false;
	}

	auto* pData = (T*)mmalloc ( iLength, Mode_e::RW, SHARED ? Share_e::ANON_SHARED : Share_e::ANON_PRIVATE );
	if ( !mmapvalid ( pData ) )
	{
		if ( iLength > (int64_t)0x7fffffffUL )
			sError.SetSprintf ( "mmap() failed: %s (length=" INT64_FMT " is over 2GB, impossible on some 32-bit systems)",
				strerror ( errno ),
				iLength );
		else
			sError.SetSprintf ( "mmap() failed: %s (length=" INT64_FMT ")", strerror ( errno ), iLength );
		return false;
	}
	mmadvise ( pData, iLength, Advise_e::NODUMP );
	if ( !SHARED )
		mmadvise ( pData, iLength, Advise_e::NOFORK );

	detail::MemStatMMapAdd ( iLength );

	assert ( pData );
	this->Set ( pData, iEntries );
	return true;
}


/// deallocate storage
template<typename T, bool SHARED>
void CSphLargeBuffer<T, SHARED>::Reset()
{
	this->MemUnlock();

	if ( !this->GetWritePtr() )
		return;

	Debug ( int iRes = ) mmfree ( this->GetWritePtr(), this->GetLengthBytes() );
	assert ( !iRes && "munmap() failed" );

	detail::MemStatMMapDel ( this->GetLengthBytes() );
	this->Set ( NULL, 0 );
}
