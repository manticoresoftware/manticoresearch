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

#include "aggregate.h"

#include "schema/columninfo.h"

/// aggregate traits for different attribute types
template < typename T >
class AggrFunc_Traits_T : public AggrFunc_i
{
public:
	explicit		AggrFunc_Traits_T ( const CSphAttrLocator & tLocator ) : m_tLocator ( tLocator ) {}

	T				GetValue ( const CSphMatch & tRow );
	void			SetValue ( CSphMatch & tRow, T val );

protected:
	CSphAttrLocator	m_tLocator;
};

template<>
inline DWORD AggrFunc_Traits_T<DWORD>::GetValue ( const CSphMatch & tRow )
{
	return (DWORD) tRow.GetAttr ( m_tLocator );
}

template<>
inline void AggrFunc_Traits_T<DWORD>::SetValue ( CSphMatch & tRow, DWORD val )
{
	tRow.SetAttr ( m_tLocator, val );
}

template<>
inline int64_t AggrFunc_Traits_T<int64_t>::GetValue ( const CSphMatch & tRow )
{
	return tRow.GetAttr ( m_tLocator );
}

template<>
inline void AggrFunc_Traits_T<int64_t>::SetValue ( CSphMatch & tRow, int64_t val )
{
	tRow.SetAttr ( m_tLocator, val );
}

template<>
inline float AggrFunc_Traits_T<float>::GetValue ( const CSphMatch & tRow )
{
	return tRow.GetAttrFloat ( m_tLocator );
}

template<>
inline void AggrFunc_Traits_T<float>::SetValue ( CSphMatch & tRow, float val )
{
	tRow.SetAttrFloat ( m_tLocator, val );
}

template<>
inline double AggrFunc_Traits_T<double>::GetValue ( const CSphMatch & tRow )
{
	return tRow.GetAttrDouble ( m_tLocator );
}

template<>
inline void AggrFunc_Traits_T<double>::SetValue ( CSphMatch & tRow, double val )
{
	tRow.SetAttrDouble ( m_tLocator, val );
}

template < typename T >
class AggrColumnar_Traits_T : public AggrFunc_Traits_T<T>
{
	using BASE = AggrFunc_Traits_T<T>;

public:
	AggrColumnar_Traits_T ( const CSphAttrLocator & tLoc, const CSphString & sAttr )
		: AggrFunc_Traits_T<T> ( tLoc )
		, m_sAttr ( sAttr )
	{}

	void SetColumnar ( columnar::Columnar_i * pColumnar ) final
	{
		if ( pColumnar )
		{
			std::string sError; // FIXME! report errors
			m_pIterator = CreateColumnarIterator ( pColumnar, m_sAttr.cstr(), sError );
			columnar::AttrInfo_t tAttrInfo;
			if ( pColumnar->GetAttrInfo ( m_sAttr.cstr(), tAttrInfo ) )
				m_eType = tAttrInfo.m_eType;
		}
		else
			m_pIterator.reset();
	}

	void Setup ( CSphMatch & tDst, const CSphMatch & tSrc, bool bMerge ) final
	{
		BASE::SetValue ( tDst, FetchValue ( tSrc, bMerge ) );
	}

protected:
	CSphString			m_sAttr;
	common::AttrType_e	m_eType = common::AttrType_e::NONE;
	std::unique_ptr<columnar::Iterator_i> m_pIterator;

	inline T FetchValue ( const CSphMatch & tSrc, bool bMerge )
	{
		if ( bMerge )
			return BASE::GetValue(tSrc);

		if ( m_eType==common::AttrType_e::FLOAT )
			return (T)sphDW2F ( (DWORD)m_pIterator->Get ( tSrc.m_tRowID ) );

		return (T)m_pIterator->Get ( tSrc.m_tRowID );
	}
};

/// SUM() implementation
template < typename T >
class AggrSum_T final : public AggrFunc_Traits_T<T>
{
	using BASE = AggrFunc_Traits_T<T>;
	using BASE::BASE;

public:
	explicit AggrSum_T ( const CSphAttrLocator & tLoc ) : AggrFunc_Traits_T<T> ( tLoc )
	{}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		T tSrcValue = BASE::GetValue(tSrc);
		T tDstValue = BASE::GetValue(tDst);

		if ( tSrcValue )
			BASE::SetValue ( tDst, tDstValue+tSrcValue );
	}
};

template < typename T >
class AggrSumColumnar_T final : public AggrColumnar_Traits_T<T>
{
	using BASE = AggrColumnar_Traits_T<T>;
	using BASE::BASE;

public:
	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		T tSrcValue = BASE::FetchValue ( tSrc, bMerge );
		T tDstValue = BASE::GetValue(tDst);

		if ( tSrcValue )
			BASE::SetValue ( tDst, tDstValue+tSrcValue );
	}
};

/// AVG() implementation
template < typename T >
class AggrAvg_T final : public AggrFunc_Traits_T<T>
{
	CSphAttrLocator m_tCountLoc;
	using AggrFunc_Traits_T<T>::GetValue;
	using AggrFunc_Traits_T<T>::SetValue;

public:
	AggrAvg_T ( const CSphAttrLocator & tLoc, const CSphAttrLocator & tCountLoc )
		: AggrFunc_Traits_T<T> ( tLoc ), m_tCountLoc ( tCountLoc )
	{}

	void Ungroup ( CSphMatch & tDst ) final
	{
		SetValue ( tDst, T ( GetValue ( tDst ) * tDst.GetAttr ( m_tCountLoc ) ) );
	}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		T tSrcValue = GetValue(tSrc);
		T tDstValue = GetValue(tDst);

		if ( bGrouped )
			SetValue ( tDst, tDstValue + T ( tSrcValue*tSrc.GetAttr(m_tCountLoc) ) );
		else if ( tSrcValue )
			SetValue ( tDst, tDstValue+tSrcValue );
	}

	void Finalize ( CSphMatch & tDst ) final
	{
		auto uAttr = tDst.GetAttr ( m_tCountLoc );
		if ( uAttr )
			SetValue ( tDst, T ( GetValue(tDst) / uAttr ) );
	}
};

template < typename T >
class AggrAvgColumnar_T final : public AggrColumnar_Traits_T<T>
{
	using BASE = AggrColumnar_Traits_T<T>;
	using BASE::GetValue;
	using BASE::SetValue;

public:
	AggrAvgColumnar_T ( const CSphAttrLocator & tLoc, const CSphString & sAttr, const CSphAttrLocator & tCountLoc )
		: BASE ( tLoc, sAttr )
		, m_tCountLoc ( tCountLoc )
	{}

	void Ungroup ( CSphMatch & tDst ) final
	{
		SetValue ( tDst, T ( GetValue ( tDst ) * tDst.GetAttr ( m_tCountLoc ) ) );
	}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		T tSrcValue = BASE::FetchValue ( tSrc, bMerge );
		T tDstValue = GetValue(tDst);

		if ( bGrouped )
			SetValue ( tDst, tDstValue + T( tSrcValue*tSrc.GetAttr(m_tCountLoc) ) );
		else if ( tSrcValue )
			SetValue ( tDst, tDstValue + tSrcValue );
	}

	void Finalize ( CSphMatch & tDst ) final
	{
		auto uAttr = tDst.GetAttr ( m_tCountLoc );
		if ( uAttr )
			SetValue ( tDst, T ( GetValue(tDst) / uAttr ) );
	}

private:
	CSphAttrLocator m_tCountLoc;
};

/// MAX() implementation
template < typename T >
class AggrMax_T final : public AggrFunc_Traits_T<T>
{
	using BASE=AggrFunc_Traits_T<T>;
	using BASE::BASE;

public:
	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		T tSrcValue = BASE::GetValue(tSrc);
		T tDstValue = BASE::GetValue(tDst);
		if ( tSrcValue>tDstValue )
			BASE::SetValue ( tDst, tSrcValue );
	}
};

template < typename T >
class AggrMaxColumnar_T final : public AggrColumnar_Traits_T<T>
{
	using BASE = AggrColumnar_Traits_T<T>;
	using BASE::BASE;

public:
	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		T tSrcValue = BASE::FetchValue ( tSrc, bMerge );
		T tDstValue = BASE::GetValue(tDst);
		if ( tSrcValue>tDstValue )
			BASE::SetValue ( tDst, tSrcValue );
	}
};

/// MIN() implementation
template < typename T >
class AggrMin_T final : public AggrFunc_Traits_T<T>
{
	using BASE = AggrFunc_Traits_T<T>;
	using BASE::BASE;

public:
	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		T tSrcValue = BASE::GetValue(tSrc);
		T tDstValue = BASE::GetValue(tDst);
		if ( tSrcValue<tDstValue )
			BASE::SetValue ( tDst, tSrcValue );
	}
};

template < typename T >
class AggrMinColumnar_T final : public AggrColumnar_Traits_T<T>
{
	using BASE = AggrColumnar_Traits_T<T>;
	using BASE::BASE;

public:
	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		T tSrcValue = BASE::FetchValue ( tSrc, bMerge );
		T tDstValue = BASE::GetValue(tDst);
		if ( tSrcValue<tDstValue )
			BASE::SetValue ( tDst, tSrcValue );
	}
};


/// GROUP_CONCAT

/* What is that magic about?
 *
 * In simplest usecase - you have in group matches with 'foo', 'bar' -> group_concat produces 'foo,bar' - no magic.
 *
 * Make a bit complex: one and same sorter processes sequentaly several indexes (chunks), and collects group result.
 * In this case if 1-st chunk gives 'foo', 2-nd gives 'bar', you still can achieve 'foo,bar' naturally, no magic.
 *
 * A bit more complex: several sorters process cloud of chunks in parallel, then merge results.
 * Say, you have 3 chunks, giving 'foo', 'bar' and 'bazz'. Result you've expect is 'foo,bar,bazz'.
 * In parallel with, say, 2 sorters, one processed 1-st and 3-rd chunk, second - middle.
 * One gives you 'foo,bazz', second - 'bar'.
 *
 * What is to do on merge then?
 *
 * Simplest: just merge 'as is'. I.e., return 'foo,bazz,bar' despite the broken order.
 * However that is appropriate only in the narrow case when ordering is not requested. That is *NOT* our way.
 *
 * Each match came from a chunk is tagged with the order num of that chunk.
 * When we have matches from the same chunk, we just group them usual way with no magic, naturally concatenating strings.
 * If all the matches tagged same - we just achieve usual string out of the box, with no magic at all.
 * If into non-empty group came match with another tag, we use this model of blob:
 *
 * '\0', <N> <TAG1> <STRLEN1> chars1 <TAG2> <STRLEN2> chars2 ... <TAGN> <STRLEN> bytesN
 *
 * First \0 marks that the whole blob is special.
 * Each tagged string inside includes concatenated values of the matches from that tag.
 * For described foo-bar-baz in two sorters it will look like:
 *
 * '\0' 2 1 3 foo 3 4 bazz // in the 1-st sorter. 2 chunks, from tag 1 with len 3 'foo', from tag 3 with len 4 'bazz'
 * bar // in the 2-nd sorter. Simple plain 'bar' (tag is not saved here, it is still an attribute of the match itself).
 *
 * Then we can merge results, taking tag for value came from 2-nd sorter from that match itself.
 *
 * '\0' 3 1 3 foo 2 3 bar 3 4 bazz
 *
 * That finally deserializes into expected user string 'foo,bar,bazz'. So, target achieved.
 *
 * One optimization here is that we expect matches with monotonically changing tags. I.e., if we have processed chunk 1,
 * and stay on chunk 3 - then next match will never came with tag 1 or 2, as these numbers already passed.
 * So, no need to 'insert into middle', we always pushes new data to tail of the blob. That makes everything simpler.
 *
 */

// helpers to blob serialization
using BStream_c = TightPackedVec_T<BYTE>;

static BStream_c & operator<< ( BStream_c & dOut, const ByteBlob_t & tData )
{
	dOut.Append ( tData.first, tData.second );
	return dOut;
}

template <typename NUM>
static BStream_c & operator<< ( BStream_c & dOut, NUM iNum )
{
	sphUnalignedWrite ( dOut.AddN ( sizeof ( NUM ) ), iNum );
	return dOut;
}

// unused for now
/*
template<typename T>
static BStream_c & operator<< ( BStream_c & dOut, const VecTraits_T<T> & tData )
{
	dOut << tData.GetLength ();
	tData.Apply ( [&dOut] ( const T & tChunk ) { dOut << tChunk; } );
	return dOut;
}
*/

static BStream_c & operator<< ( BStream_c & dOut, const VecTraits_T<BYTE> & tData )
{
	return dOut << tData.GetLength () << ByteBlob_t { tData.begin(), tData.GetLength () };
}

// unused for now
/*
static BStream_c & operator<< ( BStream_c & dOut, const CSphString& sData )
{
	return dOut << VecTraits_T<BYTE> ( (BYTE*) const_cast<char*>( sData.cstr() ), sData.Length() );
}

static BStream_c & operator<< ( BStream_c & dOut, const StringBuilder_c & sData )
{
	return dOut << VecTraits_T<BYTE> ( (BYTE*) const_cast<char*>( sData.cstr() ), sData.GetLength () );
}
*/

// helpers to de-serialize
template<typename NUM>
static ByteBlob_t & operator>> ( ByteBlob_t & dIn, NUM & iNum )
{
	assert ( dIn.first );
	assert ( dIn.second>=(int)sizeof (NUM) );
	iNum = sphUnalignedRead ( *(const NUM *) dIn.first );
	dIn.first += sizeof ( NUM );
	dIn.second -= sizeof ( NUM );
	return dIn;
}

static ByteBlob_t & operator>> ( ByteBlob_t & dIn, ByteBlob_t & tData )
{
	assert ( dIn.first );
	assert ( dIn.second>=tData.second );
	tData.first = dIn.first;
	dIn.first += tData.second;
	dIn.second -= tData.second;
	return dIn;
}

// unused for now
/*template<typename T>
static ByteBlob_t & operator>> ( ByteBlob_t & dIn, CSphVector<T> & tData )
{
	int iLen;
	dIn >> iLen;
	tData.Resize ( iLen );
	tData.Apply ( [&dIn] ( T & tChunk ) { dIn >> tChunk; } );
	return dIn;
}
*/

static ByteBlob_t & operator>> ( ByteBlob_t & dIn, VecTraits_T<BYTE> & tData )
{
	int iLen;
	dIn >> iLen;
	ByteBlob_t tChunk { nullptr, iLen };
	dIn >> tChunk;
	tData = tChunk;
	return dIn;
}

// The GROUP_CONCAT() implementation
class AggrConcat_c final : public AggrFunc_i
{
	CSphAttrLocator	m_tLoc;

public:
	explicit AggrConcat_c ( const CSphColumnInfo & tCol )
		: m_tLoc ( tCol.m_tLocator )
	{
		assert ( tCol.m_eAttrType==SPH_ATTR_STRINGPTR );
		assert ( !m_tLoc.IsBlobAttr ()); // otherwise we will fail on fetching data!
	}

	// here we convert back to plain string
	void Finalize ( CSphMatch & tMatch ) final
	{
		auto dSrc = tMatch.FetchAttrData ( m_tLoc, nullptr ); // expect serialized tagged strings

		// empty match
		if ( !dSrc.first )
			return;

		// already grouped match
		if ( *dSrc.first )
			return;

		auto dBlob = dSrc;
		int iSize, iTag, iFinalSize;
		BStream_c dOut;
		VecTraits_T<BYTE> dString;

		BYTE uZero; dBlob >> uZero;
		dBlob >> iSize;
		iFinalSize = dBlob.second - ( iSize * 2 * sizeof ( int ) ) + iSize - 1 + 20; // -tag, -len, +commas-1, +packlen
		dOut.Reserve ( iFinalSize );

		for ( int i=0; i<iSize; ++i )
		{
			if ( i>0 ) dOut << ',';
			dBlob >> iTag >> dString;
			dOut << ByteBlob_t { dString.begin (), dString.GetLength () }; // write raw blob, without length
		}

		// release previous, write converted
		sphDeallocatePacked ( sphPackedBlob ( dSrc ) );
		sphPackPtrAttrInPlace ( dOut );
		tMatch.SetAttr ( m_tLoc, (SphAttr_t) dOut.LeakData () );
	}

	void Update ( CSphMatch & tDst, const CSphMatch & tSrc, bool bGrouped, bool bMerge ) final
	{
		ByteBlob_t dSrc = tSrc.FetchAttrData ( m_tLoc, nullptr ); // ok since it is NOT a blob attr
		ByteBlob_t dDst = tDst.FetchAttrData ( m_tLoc, nullptr );

		// empty source? kinda weird, but done!
		if ( !dSrc.first || !dSrc.second )
			return;

		BStream_c dOut;
		if ( !dDst.first )
			dOut << dSrc;
		else if ( *dSrc.first && *dDst.first ) // first byte is a mark: 0 means data packed, another is part of real string.
			AppendStringToString ( dOut, dDst, tDst.m_iTag, dSrc, tSrc.m_iTag );
		else if ( *dSrc.first && !*dDst.first )
			AppendBlobToString ( dOut, dSrc, tSrc.m_iTag, dDst, false );
		else if ( !*dSrc.first && *dDst.first )
			AppendBlobToString ( dOut, dDst, tDst.m_iTag, dSrc, true );
		else // if ( !*dSrc.first && !*dDst.first )
			AppendBlobToBlob ( dOut, dDst, dSrc );

		// Dispose previous packet
		sphDeallocatePacked ( sphPackedBlob ( dDst ) );

		// update saved data
		sphPackPtrAttrInPlace (dOut);
		tDst.SetAttr ( m_tLoc, (SphAttr_t) dOut.LeakData () );
	}

private:

	// merge two simple matches
	static void AppendStringToString ( BStream_c & dOut, const ByteBlob_t & dInDst, int iTagDst, const ByteBlob_t & dInSrc, int iTagSrc )
	{
		if ( iTagDst==iTagSrc ) // plain concat of 2 strings
			dOut << dInDst << ',' << dInSrc;
		else // produce complex match
			dOut << '\0' << int(2)
			<< iTagDst << dInDst.second << dInDst
			<< iTagSrc << dInSrc.second << dInSrc;
	}

	static void WriteCount ( BStream_c& dOut, int iCount )
	{
		// update total num of elements
		int iCurrentLen = dOut.GetLength ();
		dOut.Resize ( 1 ); // since 1-st came '\0' mark
		dOut << iCount;
		dOut.Resize ( iCurrentLen );
	}

	// merge two complex matches
	static void AppendBlobToBlob ( BStream_c& dOut, ByteBlob_t dInDst, ByteBlob_t dInSrc )
	{
		int iOut = 0;
		dOut << '\0' << iOut; // mark of complex and placeholder to num of elems.

		int iSizeSrc = 0, iSizeDst = 0, iTagSrc, iTagDst;
		VecTraits_T<BYTE> dBlobSrc, dBlobDst;

		// read num of elements in both matches
		char cZero;
		dInSrc >> cZero >> iSizeSrc;
		assert ( cZero=='\0' );
		dInDst >> cZero >> iSizeDst;
		assert ( cZero=='\0' );

		auto fnNextSrc = [&] { if (iSizeSrc<=0) iTagSrc = INT_MIN; else {dInSrc >> iTagSrc >> dBlobSrc; --iSizeSrc;} };
		auto fnNextDst = [&] { if (iSizeDst<=0) iTagDst = INT_MIN; else {dInDst >> iTagDst >> dBlobDst; --iSizeDst;} };

		// merge two matches
		fnNextSrc ();
		fnNextDst ();
		while ( iTagSrc!=INT_MIN || iTagDst!=INT_MIN )
		{
			if ( iTagSrc < iTagDst ) {
				dOut << iTagDst << dBlobDst;
				fnNextDst();
			} else if ( iTagDst < iTagSrc ) {
				dOut << iTagSrc << dBlobSrc;
				fnNextSrc();
			} else {
				assert ( iTagSrc!=INT_MAX || iTagDst!=INT_MAX );
				dOut << iTagSrc;
				if ( dBlobDst.IsEmpty() )
					dOut << dBlobSrc;
				else
					dOut << dBlobDst.GetLength() + dBlobSrc.GetLength() + 1
					<< ByteBlob_t ( dBlobDst.begin(), dBlobDst.GetLength() )
					<< ',' << ByteBlob_t ( dBlobSrc.begin(), dBlobSrc.GetLength() );
				fnNextSrc();
				fnNextDst();
			}
			++iOut;
		}

		WriteCount ( dOut, iOut );
	}

	// merge string and blob. Last bool determines what will came first
	static void AppendBlobToString ( BStream_c & dOut, const ByteBlob_t & dString, int iTagString, ByteBlob_t dBlob, bool bStringFirst=true )
	{
		int iOut;
		char cZero;
		dBlob >> cZero >> iOut;
		assert ( cZero=='\0' );
		dOut << cZero << iOut; // mark of complex and placeholder to num of elems.

		int iTagSrc;
		VecTraits_T<BYTE> dBlobSrc;
		bool bCopied = false;

		// copy elems looking for the place of new one
		for ( int i=0, iOldLen=iOut; i<iOldLen; ++i)
		{
			dBlob >> iTagSrc >> dBlobSrc;
			if ( bCopied )
				dOut << iTagSrc << dBlobSrc;
			else
			{
				if ( !bCopied && iTagString > iTagSrc )
				{
					dOut << iTagString << dString.second << dString << iTagSrc << dBlobSrc;
					++iOut;
					bCopied = true;
				} else if ( !bCopied && iTagString==iTagSrc )
				{
					dOut << iTagString << dString.second + dBlobSrc.GetLength() + 1;
					if ( bStringFirst )
						dOut << dString << ',' << ByteBlob_t ( dBlobSrc.begin(), dBlobSrc.GetLength() );
					else
						dOut << ByteBlob_t ( dBlobSrc.begin(), dBlobSrc.GetLength() ) << ',' << dString;
					bCopied = true;
				} else
					dOut << iTagSrc << dBlobSrc;
			}
		}

		if ( !bCopied )
		{
			dOut << iTagString << dString.second << dString;
			++iOut;
		}

		WriteCount ( dOut, iOut );
	}
};

/////////////////////////////////////////////////////////////////////

static bool GetColumnarCol ( const CSphColumnInfo & tAttr, CSphString & sColumnarCol )
{
	if ( tAttr.m_pExpr && tAttr.m_eStage==SPH_EVAL_SORTER )
	{
		tAttr.m_pExpr->Command ( SPH_EXPR_GET_COLUMNAR_COL, &sColumnarCol );
		return !sColumnarCol.IsEmpty();
	}

	return false;
}


AggrFunc_i * CreateAggrSum ( const CSphColumnInfo & tAttr )
{
	assert ( tAttr.m_eAggrFunc==SPH_AGGR_SUM );

	CSphString sColumnarCol;
	bool bColumnar = GetColumnarCol ( tAttr, sColumnarCol );

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_BOOL:
	case SPH_ATTR_TIMESTAMP:
		if ( bColumnar )
			return new AggrSumColumnar_T<DWORD> ( tAttr.m_tLocator, sColumnarCol );
		
		return new AggrSum_T<DWORD> ( tAttr.m_tLocator );

	case SPH_ATTR_BIGINT:
		if ( bColumnar )
			return new AggrSumColumnar_T<int64_t> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrSum_T<int64_t> ( tAttr.m_tLocator );

	case SPH_ATTR_FLOAT:
		if ( bColumnar )
			return new AggrSumColumnar_T<float> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrSum_T<float> ( tAttr.m_tLocator );

	default:
		assert ( 0 && "internal error: unhandled aggregate type" );
		return nullptr;
	}
}


AggrFunc_i * CreateAggrAvg ( const CSphColumnInfo & tAttr, const CSphAttrLocator & tCount )
{
	assert ( tAttr.m_eAggrFunc==SPH_AGGR_AVG );

	CSphString sColumnarCol;
	bool bColumnar = GetColumnarCol ( tAttr, sColumnarCol );

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_BOOL:
	case SPH_ATTR_TIMESTAMP:
		if ( bColumnar )
			return new AggrAvgColumnar_T<DWORD> ( tAttr.m_tLocator, sColumnarCol, tCount );

		return new AggrAvg_T<DWORD> ( tAttr.m_tLocator, tCount );

	case SPH_ATTR_BIGINT:
		if ( bColumnar )
			return new AggrAvgColumnar_T<int64_t> ( tAttr.m_tLocator, sColumnarCol, tCount );

		return new AggrAvg_T<int64_t> ( tAttr.m_tLocator, tCount );

	case SPH_ATTR_FLOAT:
		if ( bColumnar )
			return new AggrAvgColumnar_T<float> ( tAttr.m_tLocator, sColumnarCol, tCount );

		return new AggrAvg_T<float> ( tAttr.m_tLocator, tCount );

	case SPH_ATTR_DOUBLE:
		if ( bColumnar )
			return new AggrAvgColumnar_T<double> ( tAttr.m_tLocator, sColumnarCol, tCount );

		return new AggrAvg_T<double> ( tAttr.m_tLocator, tCount );

	default:
		assert ( 0 && "internal error: unhandled aggregate type" );
		return nullptr;
	}
}


AggrFunc_i * CreateAggrMin ( const CSphColumnInfo & tAttr )
{
	assert ( tAttr.m_eAggrFunc==SPH_AGGR_MIN );

	CSphString sColumnarCol;
	bool bColumnar = GetColumnarCol ( tAttr, sColumnarCol );

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_BOOL:
	case SPH_ATTR_TIMESTAMP:
		if ( bColumnar )
			return new AggrMinColumnar_T<DWORD> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrMin_T<DWORD> ( tAttr.m_tLocator );

	case SPH_ATTR_BIGINT:
		if ( bColumnar )
			return new AggrMinColumnar_T<int64_t> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrMin_T<int64_t> ( tAttr.m_tLocator );

	case SPH_ATTR_FLOAT:
		if ( bColumnar )
			return new AggrMinColumnar_T<float> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrMin_T<float> ( tAttr.m_tLocator );

	case SPH_ATTR_DOUBLE:
		if ( bColumnar )
			return new AggrMinColumnar_T<double> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrMin_T<double> ( tAttr.m_tLocator );

	default:
		assert ( 0 && "internal error: unhandled aggregate type" );
		return nullptr;
	}
}


AggrFunc_i * CreateAggrMax ( const CSphColumnInfo & tAttr )
{
	assert ( tAttr.m_eAggrFunc==SPH_AGGR_MAX );

	CSphString sColumnarCol;
	bool bColumnar = GetColumnarCol ( tAttr, sColumnarCol );

	switch ( tAttr.m_eAttrType )
	{
	case SPH_ATTR_INTEGER:
	case SPH_ATTR_BOOL:
	case SPH_ATTR_TIMESTAMP:
		if ( bColumnar )
			return new AggrMaxColumnar_T<DWORD> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrMax_T<DWORD> ( tAttr.m_tLocator );

	case SPH_ATTR_BIGINT:
		if ( bColumnar )
			return new AggrMaxColumnar_T<int64_t> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrMax_T<int64_t> ( tAttr.m_tLocator );

	case SPH_ATTR_FLOAT:
		if ( bColumnar )
			return new AggrMaxColumnar_T<float> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrMax_T<float> ( tAttr.m_tLocator );

	case SPH_ATTR_DOUBLE:
		if ( bColumnar )
			return new AggrMaxColumnar_T<double> ( tAttr.m_tLocator, sColumnarCol );

		return new AggrMax_T<double> ( tAttr.m_tLocator );

	default:
		assert ( 0 && "internal error: unhandled aggregate type" );
		return nullptr;
	}
}

/////////////////////////////////////////////////////////////////////

AggrFunc_i * CreateAggrConcat ( const CSphColumnInfo & tAttr )
{
	return new AggrConcat_c(tAttr);
}
