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


template<typename VECTOR>
inline void GetArray ( VECTOR& dBuf, MemoryReader_c& tIn )
{
	int iCount = tIn.GetDword();
	if ( !iCount )
		return;

	dBuf.Resize ( iCount );
	tIn.GetBytes ( dBuf.Begin(), (int)dBuf.GetLengthBytes() );
}

inline void GetArray ( CSphVector<CSphString>& dBuf, MemoryReader_c& tIn )
{
	int iCount = tIn.GetDword();
	if ( !iCount )
		return;

	dBuf.Resize ( iCount );
	for ( CSphString& sVal : dBuf )
		sVal = tIn.GetString();
}

template<typename T>
T GetVal ( MemoryReader_c& tReader )
{
	T tVal;
	tReader.GetBytes ( &tVal, sizeof ( tVal ) );
	return tVal;
}

template<typename T>
T MemoryReader_c::GetVal ()
{
	return ::GetVal<T> (*this);
}

template<typename T>
void MemoryReader_c::GetVal ( T& tVal )
{
	tVal = GetVal<T>();
}

inline SphOffset_t MemoryReader_c::GetOffset()
{
	return GetVal<SphOffset_t> ();
}

inline DWORD MemoryReader_c::GetDword()
{
	return GetVal<DWORD> ();
}

template<typename T>
void PutVal ( MemoryWriter_c& tWriter, T tVal )
{
	tWriter.PutBytes ( (BYTE*)&tVal, sizeof ( tVal ) );
}

template<typename T>
void MemoryWriter_c::PutVal ( T tVal )
{
	::PutVal ( *this, tVal );
}

inline void MemoryWriter_c::PutDword ( DWORD uVal )
{
	PutVal ( uVal );
}

inline void MemoryWriter_c::PutOffset ( SphOffset_t uValue )
{
	PutVal ( uValue );
}

inline void MemoryWriter_c::PutWord ( WORD uVal )
{
	PutVal ( uVal );
}

inline void MemoryWriter_c::PutUint64 ( uint64_t uVal )
{
	PutVal ( uVal );
}


template<typename T>
inline void SaveArray ( const VecTraits_T<T>& dBuf, MemoryWriter_c& tOut )
{
	tOut.PutDword ( dBuf.GetLength() );
	if ( dBuf.GetLength() )
		tOut.PutBytes ( dBuf.Begin(), dBuf.GetLengthBytes() );
}

inline void SaveArray ( const VecTraits_T<CSphString>& dBuf, MemoryWriter_c& tOut )
{
	tOut.PutDword ( dBuf.GetLength() );
	for ( const CSphString& sVal : dBuf )
		tOut.PutString ( sVal );
}