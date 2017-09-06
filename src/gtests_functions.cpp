//
// Created by alexey on 25.08.17.
//

#include <gtest/gtest.h>

#include "sphinxint.h"
#include "json/cJSON.h"
#include <math.h>

// Miscelaneous short functional tests: TDigest, SpanSearch,
// stringbuilder, CJson, TaggedHash, Log2

//////////////////////////////////////////////////////////////////////////


class TDigest : public ::testing::Test
{

protected:
	virtual void TearDown ()
	{
		SafeDelete ( pDigest );
	}

	virtual void SetUp ()
	{
		sphSrand ( 0 );
		pDigest = sphCreateTDigest ();
	}

	TDigest_i * pDigest;
};

TEST_F ( TDigest, simple )
{
	for ( int i = 1; i<=100; i++ )
		pDigest->Add ( i, 1 );

	ASSERT_NEAR ( pDigest->Percentile ( 50 ), 51, 1 );
	ASSERT_NEAR ( pDigest->Percentile ( 95 ), 96, 1 );
	ASSERT_NEAR ( pDigest->Percentile ( 99 ), 100, 1 );
}


TEST_F ( TDigest, dupes )
{
	for ( int j = 0; j<3; j++ )
		for ( int i = 0; i<10000; i++ )
			pDigest->Add ( i / 100 + 1, 1 );

	ASSERT_NEAR ( pDigest->Percentile ( 50 ), 51, 1 );
	ASSERT_NEAR ( pDigest->Percentile ( 95 ), 96, 1 );
	ASSERT_NEAR ( pDigest->Percentile ( 99 ), 100, 1 );
}


TEST_F ( TDigest, compression )
{
	for ( int i = 0; i<10000; i++ )
		pDigest->Add ( i + 1, 1 );

	ASSERT_NEAR ( pDigest->Percentile ( 50 ), 5001, 1.5 );
	ASSERT_NEAR ( pDigest->Percentile ( 95 ), 9501, 1.5 );
	ASSERT_NEAR ( pDigest->Percentile ( 99 ), 9901, 1.5 );
}

//////////////////////////////////////////////////////////////////////////

TEST ( Misc, SpanSearch )
{
	CSphVector<int> dVec;
	dVec.Add ( 1 );
	dVec.Add ( 3 );
	dVec.Add ( 4 );

	ASSERT_EQ ( FindSpan ( dVec, 1, 5 ), 0 );
	ASSERT_EQ ( FindSpan ( dVec, 3, 5 ), 1 );
	ASSERT_EQ ( FindSpan ( dVec, 4, 5 ), 2 );

	dVec.Add ( 15 );
	dVec.Add ( 17 );
	dVec.Add ( 22 );
	dVec.Add ( 23 );

	ASSERT_EQ ( FindSpan ( dVec, 1, 5 ), 0 );
	ASSERT_EQ ( FindSpan ( dVec, 18, 5 ), 4 );
	ASSERT_EQ ( FindSpan ( dVec, 23, 5 ), 6 );
}

//////////////////////////////////////////////////////////////////////////

TEST ( functions, stringbuilder )
{
	StringBuilder_c builder;

	builder += "Hello";

	ASSERT_STREQ ( builder.cstr (), "Hello" );

}

TEST ( functions, strinbguilder_appendf )
{
	StringBuilder_c sRes;
	sRes.Appendf ( "12345678" );
	ASSERT_STREQ ( sRes.cstr (), "12345678" );
	sRes.Appendf ( "this is my rifle this is my gun" );
	ASSERT_STREQ ( sRes.cstr (), "12345678this is my rifle this is my gun" );
	sRes.Appendf ( " int=%d float=%f string=%s", 123, 456.789, "helloworld" );
	ASSERT_STREQ (
		sRes.cstr (), "12345678this is my rifle this is my gun int=123 float=456.789000 string=helloworld" );
}

struct EscapeQuotation_t
{
	static bool IsEscapeChar ( char c )
	{
		return ( c=='\\' || c=='\'' );
	}

	static char GetEscapedChar ( char c )
	{
		return c;
	}
};

using QuotationEscapedBuilder = EscapedStringBuilder_T<EscapeQuotation_t>;

TEST( functions, EscapedStringBuilder )
{
	QuotationEscapedBuilder tBuilder;
	tBuilder.AppendEscaped ( "Hello" );
	ASSERT_STREQ ( tBuilder.cstr (), "Hello" );

	tBuilder.AppendEscaped ( " wo\\rl\'d" );
	ASSERT_STREQ ( tBuilder.cstr (), "Hello wo\\\\rl\\'d" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "wo\\rl\'d", false );
	ASSERT_STREQ ( tBuilder.cstr (), "wo\\rl\'d" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r tab\n here", false, false );
	ASSERT_STREQ ( tBuilder.cstr (), "space\t and\r tab\n here" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r tab\n here" );
	ASSERT_STREQ ( tBuilder.cstr (), "space  and  tab  here" );
}

//////////////////////////////////////////////////////////////////////////

TEST( functions, TaggedHash20_t )
{
	const char * sFIPS = "45f44fd2db02b08b4189abf21e90edd712c9616d *rt_full.ram\n";
	const BYTE bytescheck[HASH20_SIZE] = { 0x45, 0xf4, 0x4f, 0xd2, 0xdb, 0x02, 0xb0, 0x8b, 0x41, 0x89, 0xab, 0xf2
										   , 0x1e, 0x90, 0xed, 0xd7, 0x12, 0xc9, 0x61, 0x6d };
	const char * namecheck = "rt_full.ram";

	TaggedHash20_t tHash ( "HelloFips" );
	CSphString sFips = tHash.ToFIPS ();

	EXPECT_TRUE ( sFips=="" );

	tHash.FromFIPS ( sFIPS );
	ASSERT_TRUE ( tHash.m_sTagName==namecheck );

	ASSERT_TRUE ( 0==memcmp ( tHash.m_dHashValue, bytescheck, HASH20_SIZE ) );

	sFips = tHash.ToFIPS ();
	ASSERT_TRUE ( sFips==sFIPS );

	TaggedHash20_t tHash2 ( namecheck, bytescheck );
	ASSERT_TRUE ( tHash2.ToFIPS ()==sFIPS );

}

//////////////////////////////////////////////////////////////////////////

TEST ( functions, CJson )
{
	struct MyIndex_t
	{
		CSphString m_sName;
		CSphString m_sPath;
	};

	CSphString sResult;

	CSphVector<MyIndex_t> dIndexes;
	dIndexes.Add ( { "test1", "test1_path" } );
	dIndexes.Add ( { "test2", "test2_path" } );
	dIndexes.Add ( { "test3", "test3_path" } );

	{
		cJSON * pRoot = cJSON_CreateObject ();
		ASSERT_TRUE ( pRoot );

		cJSON * pIndexes = cJSON_CreateArray ();
		ASSERT_TRUE ( pIndexes );
		cJSON_AddItemToObject ( pRoot, "indexes", pIndexes );

		for ( auto i : dIndexes )
		{
			cJSON * pIndex = cJSON_CreateObject ();
			ASSERT_TRUE ( pIndex );
			cJSON_AddItemToArray ( pIndexes, pIndex );
			cJSON_AddStringToObject ( pIndex, "name", i.m_sName.cstr () );
			cJSON_AddStringToObject ( pIndex, "path", i.m_sPath.cstr () );
		}

		char * szResult = cJSON_Print ( pRoot );
		sResult.Adopt ( &szResult );
		cJSON_Delete ( pRoot );
	}

	{
		const char * dContents = sResult.cstr ();

		cJSON * pRoot = cJSON_Parse ( dContents );
		EXPECT_TRUE ( pRoot );

		cJSON * pIndexes = cJSON_GetObjectItem ( pRoot, "indexes" );
		EXPECT_TRUE ( pIndexes );

		int iNumIndexes = cJSON_GetArraySize ( pIndexes );
		ASSERT_EQ ( iNumIndexes, dIndexes.GetLength () );

		int iItem = 0;
		for ( auto i : dIndexes )
		{
			cJSON * pIndex = cJSON_GetArrayItem ( pIndexes, iItem++ );
			EXPECT_TRUE ( pIndex );

			cJSON * pJ;
			pJ = cJSON_GetObjectItem ( pIndex, "name" );
			EXPECT_TRUE ( pJ );
			ASSERT_EQ ( i.m_sName, pJ->valuestring );

			pJ = cJSON_GetObjectItem ( pIndex, "path" );
			EXPECT_TRUE ( pJ );
			ASSERT_EQ ( i.m_sPath, pJ->valuestring );
		}
		cJSON_Delete ( pRoot );
	}
}

//////////////////////////////////////////////////////////////////////////
unsigned int nlog2 ( uint64_t x )
{
	x |= ( x >> 1 );
	x |= ( x >> 2 );
	x |= ( x >> 4 );
	x |= ( x >> 8 );
	x |= ( x >> 16 );
	if ( x&0xFFFFFFFF00000000 )
		return 32+ sphBitCount(x>>32);
	return sphBitCount(x&0xFFFFFFFF);
}

TEST ( functions, Log2 )
{
	EXPECT_EQ ( sphLog2 ( 1 ), 1 );
	EXPECT_EQ ( sphLog2 ( 2 ), 2 );
	EXPECT_EQ ( sphLog2 ( 3 ), 2 );
	EXPECT_EQ ( sphLog2 ( 4 ), 3 );
	EXPECT_EQ ( sphLog2 ( 5 ), 3 );
	EXPECT_EQ ( sphLog2 ( 6 ), 3 );
	EXPECT_EQ ( sphLog2 ( 7 ), 3 );
	EXPECT_EQ ( sphLog2 ( 8 ), 4 );
	EXPECT_EQ ( sphLog2 ( 9 ), 4 );
	EXPECT_EQ ( sphLog2 ( 10 ), 4 );
	EXPECT_EQ ( sphLog2 ( 65535 ), 16 );
	EXPECT_EQ ( sphLog2 ( 65536 ), 17 );
	EXPECT_EQ ( sphLog2 ( 0xffffffffUL ), 32 );
	EXPECT_EQ ( sphLog2 ( 0x100000000ULL ), 33 );
	EXPECT_EQ ( sphLog2 ( 0x100000001ULL ), 33 );
	EXPECT_EQ ( sphLog2 ( 0x1ffffffffULL ), 33 );
	EXPECT_EQ ( sphLog2 ( 0x200000000ULL ), 34 );
	EXPECT_EQ ( sphLog2 ( 0xffffffffffffffffULL ), 64 );
	EXPECT_EQ ( sphLog2 ( 0xfffffffffffffffeULL ), 64 );
	EXPECT_EQ ( sphLog2 ( 0xefffffffffffffffULL ), 64 );
	EXPECT_EQ ( sphLog2 ( 0x7fffffffffffffffULL ), 63 );

	EXPECT_EQ ( nlog2 ( 1 ), 1 ) << "emulated";
	EXPECT_EQ ( nlog2 ( 2 ), 2 );
	EXPECT_EQ ( nlog2 ( 3 ), 2 );
	EXPECT_EQ ( nlog2 ( 4 ), 3 );
	EXPECT_EQ ( nlog2 ( 5 ), 3 );
	EXPECT_EQ ( nlog2 ( 6 ), 3 );
	EXPECT_EQ ( nlog2 ( 7 ), 3 );
	EXPECT_EQ ( nlog2 ( 8 ), 4 );
	EXPECT_EQ ( nlog2 ( 9 ), 4 );
	EXPECT_EQ ( nlog2 ( 10 ), 4 );
	EXPECT_EQ ( nlog2 ( 65535 ), 16 );
	EXPECT_EQ ( nlog2 ( 65536 ), 17 );
	EXPECT_EQ ( nlog2 ( 0xffffffffUL ), 32 );
	EXPECT_EQ ( nlog2 ( 0x100000000ULL ), 33 );
	EXPECT_EQ ( nlog2 ( 0x100000001ULL ), 33 );
	EXPECT_EQ ( nlog2 ( 0x1ffffffffULL ), 33 );
	EXPECT_EQ ( nlog2 ( 0x200000000ULL ), 34 );
	EXPECT_EQ ( nlog2 ( 0xffffffffffffffffULL ), 64 );
	EXPECT_EQ ( nlog2 ( 0xfffffffffffffffeULL ), 64 );
	EXPECT_EQ ( nlog2 ( 0xefffffffffffffffULL ), 64 );
	ASSERT_EQ ( sphLog2 ( 0x7fffffffffffffffULL ), 63 );
}

//////////////////////////////////////////////////////////////////////////

CSphMutex g_Mutex1;

void TimedLockTest ( void * )
{
	ASSERT_FALSE ( g_Mutex1.TimedLock ( 1000 ) ) << "timedlock attempt 1";
	ASSERT_FALSE ( g_Mutex1.TimedLock ( 1000 ) ) << "timedlock attempt 2";
	ASSERT_FALSE ( g_Mutex1.TimedLock ( 1000 ) ) << "timedlock attempt 3";
	ASSERT_TRUE ( g_Mutex1.TimedLock ( 1000 ) ) << "timedlock attempt 4";
	ASSERT_TRUE ( g_Mutex1.Unlock () );
}

TEST (functions, Mutex)
{
	SphThread_t th;
	ASSERT_TRUE ( g_Mutex1.Lock () ) << "locked";
	ASSERT_TRUE ( sphThreadCreate ( &th, TimedLockTest, NULL ) ) << "timedlock thread created";
	sphSleepMsec ( 3500 );
	ASSERT_TRUE ( g_Mutex1.Unlock () ) << "unlocked";
	ASSERT_TRUE ( sphThreadJoin ( &th ) ) << "timedlock thread done";
}

//////////////////////////////////////////////////////////////////////////

static int g_iRwlock;
static CSphRwlock g_tRwlock;

void RwlockReader ( void * pArg )
{
	ASSERT_TRUE ( g_tRwlock.ReadLock () );
	sphSleepMsec ( 10 );
	*( int * ) pArg = g_iRwlock;
	ASSERT_TRUE ( g_tRwlock.Unlock () );
}

void RwlockWriter ( void * pArg )
{
	ASSERT_TRUE ( g_tRwlock.WriteLock () );
	g_iRwlock += size_t ( pArg );
	sphSleepMsec ( 3 );
	ASSERT_TRUE ( g_tRwlock.Unlock () );
}

TEST ( functions, RWLock )
{
	ASSERT_TRUE ( g_tRwlock.Init () );

	const int NPAIRS = 10;
	SphThread_t dReaders[NPAIRS];
	SphThread_t dWriters[NPAIRS];
	int iRead[NPAIRS];

	g_iRwlock = 0;
	for ( int i = 0; i<NPAIRS; i++ )
	{
		ASSERT_TRUE ( sphThreadCreate ( &dReaders[i], RwlockReader, ( void * ) &iRead[i] ) );
		ASSERT_TRUE ( sphThreadCreate ( &dWriters[i], RwlockWriter, reinterpret_cast<void *>(1 + i) ) );
	}

	for ( int i = 0; i<NPAIRS; i++ )
	{
		ASSERT_TRUE ( sphThreadJoin ( &dReaders[i] ) );
		ASSERT_TRUE ( sphThreadJoin ( &dWriters[i] ) );
	}

	ASSERT_EQ ( g_iRwlock, NPAIRS * ( 1 + NPAIRS ) / 2 );
	ASSERT_TRUE ( g_tRwlock.Done () );

	int iReadSum = 0;
	for ( int i = 0; i<NPAIRS; i++ )
		iReadSum += iRead[i];

	RecordProperty ( "read_sum", iReadSum );
}

//////////////////////////////////////////////////////////////////////////

void CleanupCallback ( void * pArg )
{
	*( bool * ) pArg = true;
}

void CleanupThread ( void * pArg )
{
	sphThreadOnExit ( CleanupCallback, pArg );
}

TEST ( functions, Cleanup )
{
	const int CLEANUP_COUNT = 10;
	bool bCleanup[CLEANUP_COUNT];
	for ( auto & bClean : bCleanup )
		bClean = false;

	SphThread_t thd[CLEANUP_COUNT];
	for ( int i = 0; i<CLEANUP_COUNT; i++ )
		ASSERT_TRUE ( sphThreadCreate ( &thd[i], CleanupThread, &bCleanup[i] ) );

	for ( auto & th : thd )
		ASSERT_TRUE ( sphThreadJoin ( &th ) );

	for ( auto & bClean : bCleanup )
		ASSERT_TRUE ( bClean );
}

//////////////////////////////////////////////////////////////////////////

TEST ( functions, Hash_simple )
{
	// add and verify a couple keys manually
	CSphHash<int> h;
	int &a = h.Acquire ( 123 );
	ASSERT_FALSE ( a );
	a = 1;
	int &b = h.Acquire ( 234 );
	ASSERT_FALSE ( b);
	b = 2;
	ASSERT_TRUE ( h.Find ( 123 ) );
	ASSERT_EQ ( *h.Find ( 123 ), 1 );
	ASSERT_TRUE ( h.Find ( 234 ) );
	ASSERT_EQ ( *h.Find ( 234 ), 2 );
	ASSERT_FALSE ( h.Find ( 345 ) );

	// add several pairs of colliding keys
	const int DUPES = 8;
	int64_t dupes[DUPES*2] = {
		54309970105, 55904555634,
		54386834629, 61870972983,
		54789062086, 8033211121,
		41888995393, 69125167042,
		18878807922, 3782313558,
		31939787707, 58687170065,
		36013093500, 57976719271,
		35732429300, 67391785901
	};
	for ( int i = 0; i<2 * DUPES; i++ )
	{
		ASSERT_EQ ( h.GetHash ( dupes[i] ), h.GetHash ( dupes[( i >> 1 ) << 1] ) );
		int &x = h.Acquire ( dupes[i] );
		ASSERT_FALSE ( x );
		x = 100 + i;
	}

	// verify that colliding keys hashed differently
	for ( int i = 0; i<2 * DUPES; i++ )
		ASSERT_EQ ( *h.Find ( dupes[i] ), 100 + i );

	// verify that Add() attempts fail
	for ( int i = 0; i<2 * DUPES; i++ )
		ASSERT_FALSE ( h.Add ( dupes[i], 567 ) );

	// delete every 1st colliding key
	for ( int i = 0; i<2 * DUPES; i += 2 )
		h.Delete ( dupes[i] );

	// verify that 1st colliding key got deleted
	for ( int i = 0; i<2 * DUPES; i += 2 )
		ASSERT_FALSE ( h.Find ( dupes[i] ) );

	// verify that 2nd colliding key still works ok
	for ( int i = 1; i<2 * DUPES; i += 2 )
		ASSERT_EQ ( *h.Find ( dupes[i] ), 100 + i );
}

TEST ( functions, HASH_randomized )

// big randomized test
{
	CSphHash<int> h;
	const int NVALS = 996146; // 0.95f out of 1M

	// add N numbers
	sphSrand ( 0 );
	for ( int i = 0; i<NVALS; i++ )
	{
		uint64_t k = sphRand ();
		k = ( k << 32 ) + sphRand ();
		h.Acquire ( k ) = i;
	}

	// verify that everything looks up as expected
	sphSrand ( 0 );
	for ( int i = 0; i<NVALS; i++ )
	{
		uint64_t k = sphRand ();
		k = ( k << 32 ) + sphRand ();
		ASSERT_EQ ( h.Acquire ( k ), i );
	}

	// delete every 3rd number
	sphSrand ( 0 );
	for ( int i = 0; i<NVALS; i++ )
	{
		uint64_t k = sphRand ();
		k = ( k << 32 ) + sphRand ();
		if ( !( i % 3 ) )
			h.Delete ( k );
	}

	// verify that everything looks up as expected
	sphSrand ( 0 );
	for ( int i = 0; i<NVALS; i++ )
	{
		uint64_t k = sphRand ();
		k = ( k << 32 ) + sphRand ();
		if ( i % 3 )
			ASSERT_EQ ( *h.Find ( k ), i );
		else
			ASSERT_FALSE ( h.Find ( k ) );
	}
}

//////////////////////////////////////////////////////////////////////////

TEST ( functions, str_attr_packer_unpacker )
{
	BYTE dBuffer[128];
	int dValues[] = { 16383, 0, 1, 127, 128, 129, 256, 4095, 4096, 4097, 8192, 16383, 16384, 16385, 123456, 4194303
					  , -1 };

	BYTE * pRow = dBuffer;
	for ( int i = 0; dValues[i]>=0; i++ )
		pRow += sphPackStrlen ( pRow, dValues[i] );

	const BYTE * pUnp = dBuffer;
	for ( int i = 0; dValues[i]>=0; i++ )
	{
		int iUnp = sphUnpackStr ( pUnp, &pUnp );
		ASSERT_EQ ( iUnp, dValues[i] );
	}
}

TEST ( functions, string_split )
{
	CSphVector<CSphString> dStr;
	sphSplit ( dStr, "test:me\0off\0", ":" );
	ASSERT_EQ ( dStr.GetLength (), 2 );
	ASSERT_STREQ ( dStr[0].cstr(),"test" );
	ASSERT_STREQ ( dStr[1].cstr(), "me" );
}

//////////////////////////////////////////////////////////////////////////

struct TestAccCmp_fn
{
	typedef DWORD MEDIAN_TYPE;
	typedef DWORD * PTR_TYPE;

	int m_iStride;

	explicit TestAccCmp_fn ( int iStride )
		: m_iStride ( iStride )
	{}

	DWORD Key ( DWORD * pData ) const
	{
		return *pData;
	}

	void CopyKey ( DWORD * pMed, DWORD * pVal ) const
	{
		*pMed = Key ( pVal );
	}

	bool IsLess ( DWORD a, DWORD b ) const
	{
		return a<b;
	}

	void Swap ( DWORD * a, DWORD * b ) const
	{
		for ( int i = 0; i<m_iStride; i++ )
			::Swap ( a[i], b[i] );
	}

	DWORD * Add ( DWORD * p, int i ) const
	{
		return p + i * m_iStride;
	}

	int Sub ( DWORD * b, DWORD * a ) const
	{
		return ( int ) ( ( b - a ) / m_iStride );
	}

	bool IsKeyDataSynced ( const DWORD * pData ) const
	{
		DWORD uKey = *pData;
		DWORD uHash = GenerateKey ( pData );
		return uKey==uHash;
	}

	DWORD GenerateKey ( const DWORD * pData ) const
	{
		return m_iStride>1 ? sphCRC32 ( pData + 1, ( m_iStride - 1 ) * 4 ) : ( *pData );
	}
};


static bool IsSorted ( DWORD * pData, int iCount, const TestAccCmp_fn &fn )
{
	if ( iCount<1 )
		return true;

	const DWORD * pPrev = pData;
	if ( !fn.IsKeyDataSynced ( pPrev ) )
		return false;

	if ( iCount<2 )
		return true;

	for ( int i = 1; i<iCount; ++i )
	{
		const DWORD * pCurr = fn.Add ( pData, i );

		if ( fn.IsLess ( *pCurr, *pPrev ) || !fn.IsKeyDataSynced ( pCurr ) )
			return false;

		pPrev = pCurr;
	}

	return true;
}

void RandomFill ( DWORD * pData, int iCount, const TestAccCmp_fn &fn, bool bChainsaw )
{
	for ( int i = 0; i<iCount; ++i )
	{
		DWORD * pCurr = fn.Add ( pData, i );
		const DWORD * pNext = fn.Add ( pData, i + 1 );

		DWORD * pElem = pCurr;
		DWORD * pChainHill = bChainsaw && ( i % 2 ) ? fn.Add ( pData, i - 1 ) : NULL;
		do
		{
			*pElem = pChainHill ? *pChainHill / 2 : sphRand ();
			++pElem;
			pChainHill = pChainHill ? pChainHill + 1 : pChainHill;
		} while ( pElem!=pNext );

		*pCurr = fn.GenerateKey ( pCurr );
	}
}

void TestStridedSortPass ( int iStride, int iCount )
{
	ASSERT_TRUE ( iStride );
	ASSERT_TRUE  ( iCount );

	DWORD * pData = new DWORD[iCount * iStride];
	ASSERT_TRUE ( pData );

	// checked elements are random
	memset ( pData, 0, sizeof ( DWORD ) * iCount * iStride );
	TestAccCmp_fn fnSort ( iStride );
	RandomFill ( pData, iCount, fnSort, false );

	// crash on sort of mini-arrays
	TestAccCmp_fn fnSortDummy ( 1 );
	DWORD dMini[1] = { 1 };
	sphSort ( dMini, 1, fnSortDummy, fnSortDummy );
	sphSort ( dMini, 0, fnSortDummy, fnSortDummy );
	ASSERT_TRUE ( IsSorted ( dMini, 1, fnSortDummy ) );

	// random sort
	sphSort ( pData, iCount, fnSort, fnSort );
	ASSERT_TRUE ( IsSorted ( pData, iCount, fnSort ) );

	// already sorted sort
	sphSort ( pData, iCount, fnSort, fnSort );
	ASSERT_TRUE ( IsSorted ( pData, iCount, fnSort ) );

	// reverse order sort
	for ( int i = 0; i<iCount; ++i )
	{
		::Swap ( pData[i], pData[iCount - i - 1] );
	}
	sphSort ( pData, iCount, fnSort, fnSort );
	ASSERT_TRUE ( IsSorted ( pData, iCount, fnSort ) );

	// random chainsaw sort
	RandomFill ( pData, iCount, fnSort, true );
	sphSort ( pData, iCount, fnSort, fnSort );
	ASSERT_TRUE ( IsSorted ( pData, iCount, fnSort ) );

	SafeDeleteArray ( pData );
}

TEST ( functions, StridedSort )
{
	SCOPED_TRACE ( "stride 1, count 2" ); TestStridedSortPass ( 1, 2 );
	SCOPED_TRACE ( "stride 3, count 2" ); TestStridedSortPass ( 3, 2 );
	SCOPED_TRACE ( "stride 37, count 2" ); TestStridedSortPass ( 37, 2 );

	// SMALL_THRESH case
	SCOPED_TRACE ( "stride 1, count 30" ); TestStridedSortPass ( 1, 30 );
	SCOPED_TRACE ( "stride 7, count 13" ); TestStridedSortPass ( 7, 13 );
	SCOPED_TRACE ( "stride 113, count 5" ); TestStridedSortPass ( 113, 5 );

	SCOPED_TRACE ( "stride 1, count 1000" ); TestStridedSortPass ( 1, 1000 );
	SCOPED_TRACE ( "stride 5, count 1000" ); TestStridedSortPass ( 5, 1000 );
	SCOPED_TRACE ( "stride 17, count 50" ); TestStridedSortPass ( 17, 50 );
	SCOPED_TRACE ( "stride 31, count 1367" ); TestStridedSortPass ( 31, 1367 );

	SCOPED_TRACE ( "random strides" );
	// rand cases
	for ( int i = 0; i<10; ++i )
	{
		const int iRndStride = sphRand () % 64;
		const int iNrmStride = Max ( iRndStride, 1 );
		const int iRndCount = sphRand () % 1000;
		const int iNrmCount = Max ( iRndCount, 1 );
		TestStridedSortPass ( iNrmStride, iNrmCount );
	}
}

TEST ( functions, StridedSort_regressions )
{
	// regression of uniq vs empty array
	DWORD dUniq[] = { 1, 1, 3, 1 };
	int iCount = sizeof ( dUniq ) / sizeof ( dUniq[0] );
	ASSERT_FALSE ( sphUniq ( dUniq, 0 ) );
	sphSort ( dUniq, iCount );
	ASSERT_EQ ( sphUniq ( dUniq, iCount ), 2);
	ASSERT_EQ ( dUniq[0], 1);
	ASSERT_EQ ( dUniq[1], 3 );

	CSphVector<DWORD> dUniq1;
	dUniq1.Uniq ();
	ASSERT_FALSE ( dUniq1.GetLength () );
	dUniq1.Add ( 1 );
	dUniq1.Add ( 3 );
	dUniq1.Add ( 1 );
	dUniq1.Add ( 1 );
	dUniq1.Uniq ();
	ASSERT_EQ ( dUniq1.GetLength (), 2 );
	ASSERT_EQ ( dUniq1[0], 1 );
	ASSERT_EQ ( dUniq1[1], 3 );
}

//////////////////////////////////////////////////////////////////////////

TEST ( functions, Writer )
{
	const CSphString sTmpWriteout = "__writeout.tmp";
	CSphString sErr;

	static const auto WRITE_OUT_DATA_SIZE = 0x40000;
	BYTE * pData = new BYTE[WRITE_OUT_DATA_SIZE];
	memset ( pData, 0xfe, WRITE_OUT_DATA_SIZE );
	{
		CSphWriter tWrDef;
		tWrDef.OpenFile ( sTmpWriteout, sErr );
		tWrDef.PutBytes ( pData, WRITE_OUT_DATA_SIZE );
		tWrDef.PutByte ( 0xff );
	}
	{
		CSphWriter tWr;
		tWr.SetBufferSize ( WRITE_OUT_DATA_SIZE );
		tWr.OpenFile ( sTmpWriteout, sErr );
		tWr.PutBytes ( pData, WRITE_OUT_DATA_SIZE );
		tWr.PutByte ( 0xff );
	}
	unlink ( sTmpWriteout.cstr () );
	delete[] pData;
}

//////////////////////////////////////////////////////////////////////////

static void TestRebalance_fn ( DWORD * pData, int iLen, int iStride, int iWeights )
{
	ASSERT_FALSE ( iLen % iStride );
	iLen /= iStride;
	CSphFixedVector<int64_t> dTimers ( iWeights );
	CSphFixedVector<WORD> dWeights ( iWeights );
	for ( int i = 0; i<iLen; i++ )
	{
		for ( int j = 0; j<iWeights; j++ )
		{
			dWeights[j] = ( WORD ) pData[i * iStride + j];
			dTimers[j] = pData[i * iStride + iWeights + j];
		}

		RebalanceWeights ( dTimers, dWeights.Begin () );

		for ( int j = 0; j<iWeights; j++ )
		{
			//printf ( "%s%d%s", j>0 ? ", " : "", dWeights[j], j+1==iWeights ? "\n" : "" );
			ASSERT_EQ ( dWeights[j], pData[i * iStride + iWeights * 2 + j] );
		}
	}
}

TEST ( functions, Rebalance )
{
	/* reference captured on live box, ie how it was
	//					old weights,	timers,				new weights
	DWORD dData[] = {	32395, 33139,	228828, 186751,		29082, 36452,
						29082, 36452,	218537, 207608,		28255, 37279,
						28255, 37279,	194305, 214800,		29877, 35657,
						29877, 35657,	190062, 207614,		31318, 34216,
						31318, 34216,	201162, 221708,		32910, 32624,
						32910, 32624,	193441, 247379,		36917, 28617,
						36917, 28617,	194910, 223202,		39080, 26454,
						39080, 26454,	228274, 361018,		45892, 19642,
						45892, 19642,	223009, 275050,		48651, 16883,
						48651, 16883,	205340, 279008,		52202, 13332,
						52202, 13332,	213189, 201466,		51592, 13942,
						51592, 13942,	210235, 197584,		50899, 14635,
						48921, 16613,	207860, 318349,		53641, 11893,
						53641, 11893,	204124, 487120,		59963, 5571,
						59963, 5571,	202851, 412733,		62140, 3394,
	}; */
	//					old weights,	timers,				new weights
	DWORD dData1[] = {	32395, 33139,	228828, 186751,		29449, 36085,
						29082, 36452,	218537, 207608,		31927, 33607,
						28255, 37279,	194305, 214800,		34409, 31125,
						29877, 35657,	190062, 207614,		34213, 31321,
						31318, 34216,	201162, 221708,		34359, 31175,
						32910, 32624,	193441, 247379,		36776, 28758,
						36917, 28617,	194910, 223202,		34984, 30550,
						39080, 26454,	228274, 361018,		40148, 25386,
						45892, 19642,	223009, 275050,		36191, 29343,
						48651, 16883,	205340, 279008,		37751, 27783,
						52202, 13332,	213189, 201466,		31841, 33693,
						51592, 13942,	210235, 197584,		31751, 33783,
						48921, 16613,	207860, 318349,		39647, 25887,
						53641, 11893,	204124, 487120,		46182, 19352,
						59963, 5571,	202851, 412733,		43939, 21595,
	};
	TestRebalance_fn ( dData1, sizeof(dData1) / sizeof(dData1[0]), 6, 2 );

	DWORD dData2[] ={ 0, 1,				0, 18469,			6553, 58981 };
	TestRebalance_fn ( dData2, sizeof(dData2) / sizeof(dData2[0]), 6, 2 );

	DWORD dData3[] ={ 0, 1, 2, 3,		0, 0, 0, 18469,		2184, 2184, 2184, 58981 };
	TestRebalance_fn ( dData3, sizeof ( dData3 ) / sizeof ( dData3[0] ), 12, 4 );

	DWORD dData4[] ={ 0, 1, 2,			7100, 0, 18469,		42603, 6553, 16377 };
	TestRebalance_fn ( dData4, sizeof ( dData4 ) / sizeof ( dData4[0] ), 9, 3 );
}

//////////////////////////////////////////////////////////////////////////

// conversion between degrees and radians
static const double MY_PI = 3.14159265358979323846;
static const double TO_RADD = MY_PI / 180.0;
static const double TO_RADD2 = MY_PI / 360.0;
static const double TO_DEGD = 180.0 / MY_PI;

static inline float GeodistVincenty ( double lat1, double lon1, double lat2, double lon2 )
{
	lat1 *= TO_RADD;
	lon1 *= TO_RADD;
	lat2 *= TO_RADD;
	lon2 *= TO_RADD;
	const double a = 6378137;
	const double b = 6356752.314245;
	double f = ( a - b ) / a;
	double L = lon2 - lon1;
	double u1 = atan ( ( 1 - f ) * tan ( lat1 ) );
	double u2 = atan ( ( 1 - f ) * tan ( lat2 ) );
	double sin_u1 = sin ( u1 );
	double cos_u1 = cos ( u1 );
	double sin_u2 = sin ( u2 );
	double cos_u2 = cos ( u2 );
	double lambda = L;
	double lambda_pi = 2 * MY_PI;
	double sin_sigma = 0, cos_sigma = 0, sigma = 0, cos_sq_alpha = 0, cos2sigma_m = 0;
	while ( fabs ( lambda - lambda_pi )>1e-12 )
	{
		double sin_lambda = sin ( lambda );
		double cos_lambda = cos ( lambda );
		sin_sigma = sqrt ( ( cos_u2 * sin_lambda ) * ( cos_u2 * sin_lambda ) +
			( cos_u1 * sin_u2 - sin_u1 * cos_u2 * cos_lambda ) * ( cos_u1 * sin_u2 - sin_u1 * cos_u2 * cos_lambda ) );
		cos_sigma = sin_u1 * sin_u2 + cos_u1 * cos_u2 * cos_lambda;
		sigma = atan2 ( sin_sigma, cos_sigma );
		double alpha = asin ( cos_u1 * cos_u2 * sin_lambda / sin_sigma );
		cos_sq_alpha = cos ( alpha ) * cos ( alpha );
		cos2sigma_m = cos_sigma - 2 * sin_u1 * sin_u2 / cos_sq_alpha;
		double cc = f / 16 * cos_sq_alpha * ( 4 + f * ( 4 - 3 * cos_sq_alpha ) );
		lambda_pi = lambda;
		lambda = L + ( 1 - cc ) * f * sin ( alpha ) *
			( sigma + cc * sin_sigma * ( cos2sigma_m + cc * cos_sigma * ( -1 + 2 * cos2sigma_m * cos2sigma_m ) ) );
	}
	double usq = cos_sq_alpha * ( a * a - b * b ) / ( b * b );
	double aa = 1 + usq / 16384 * ( 4096 + usq * ( -768 + usq * ( 320 - 175 * usq ) ) );
	double bb = usq / 1024 * ( 256 + usq * ( -128 + usq * ( 74 - 47 * usq ) ) );
	double delta_sigma =
		bb * sin_sigma * ( cos2sigma_m + bb / 4 * ( cos_sigma * ( -1 + 2 * cos2sigma_m * cos2sigma_m ) -
			bb / 6 * cos2sigma_m * ( -3 + 4 * sin_sigma * sin_sigma ) * ( -3 + 4 * cos2sigma_m * cos2sigma_m ) ) );
	double c = b * aa * ( sigma - delta_sigma );
	return ( float ) c;
}

void DestVincenty ( double lat1, double lon1, double brng, double dist, double * lat2, double * lon2 )
{
	double a = 6378137, b = 6356752.3142, f = 1 / 298.257223563; // WGS-84 ellipsiod
	double s = dist;
	double alpha1 = brng * TO_RADD;
	double sinAlpha1 = sin ( alpha1 );
	double cosAlpha1 = cos ( alpha1 );

	double tanU1 = ( 1 - f ) * tan ( lat1 * TO_RADD );
	double cosU1 = 1 / sqrt ( 1 + tanU1 * tanU1 ), sinU1 = tanU1 * cosU1;
	double sigma1 = atan2 ( tanU1, cosAlpha1 );
	double sinAlpha = cosU1 * sinAlpha1;
	double cosSqAlpha = 1 - sinAlpha * sinAlpha;
	double uSq = cosSqAlpha * ( a * a - b * b ) / ( b * b );
	double A = 1 + uSq / 16384 * ( 4096 + uSq * ( -768 + uSq * ( 320 - 175 * uSq ) ) );
	double B = uSq / 1024 * ( 256 + uSq * ( -128 + uSq * ( 74 - 47 * uSq ) ) );

	double sigma = s / ( b * A ), sigmaP = 2 * MY_PI;
	double cos2SigmaM = 0, sinSigma = 0, cosSigma = 0;
	while ( fabs ( sigma - sigmaP )>1e-12 )
	{
		cos2SigmaM = cos ( 2 * sigma1 + sigma );
		sinSigma = sin ( sigma );
		cosSigma = cos ( sigma );
		double deltaSigma = B * sinSigma * ( cos2SigmaM + B / 4 * ( cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) -
			B / 6 * cos2SigmaM * ( -3 + 4 * sinSigma * sinSigma ) * ( -3 + 4 * cos2SigmaM * cos2SigmaM ) ) );
		sigmaP = sigma;
		sigma = s / ( b * A ) + deltaSigma;
	}

	double tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;
	*lat2 = atan2 ( sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1,
		( 1 - f ) * sqrt ( sinAlpha * sinAlpha + tmp * tmp ) );
	double lambda = atan2 ( sinSigma * sinAlpha1, cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1 );
	double C = f / 16 * cosSqAlpha * ( 4 + f * ( 4 - 3 * cosSqAlpha ) );
	double L = lambda - ( 1 - C ) * f * sinAlpha *
		( sigma + C * sinSigma * ( cos2SigmaM + C * cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) ) );
	*lon2 = ( lon1 * TO_RADD + L + 3 * MY_PI );
	while ( *lon2>2 * MY_PI )
		*lon2 -= 2 * MY_PI;
	*lon2 -= MY_PI;
	*lat2 *= TO_DEGD;
	*lon2 *= TO_DEGD;
}

static const int NFUNCS = 3;

float CalcGeofunc ( int iFunc, double * t )
{
	switch ( iFunc )
	{
		case 0: return GeodistSphereDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
		case 1: return GeodistAdaptiveDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
		case 2: return GeodistFlatDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
	}
	return 0;
}

TEST ( functions, DISABLED_geodist )
{
	CSphVector<double> dBench;
	for ( int adist = 10; adist<=10 * 1000 * 1000; adist *= 10 )
		for ( int dist = adist; dist<10 * adist && dist<20 * 1000 * 1000; dist += 2 * adist )
		{
			double avgerr[NFUNCS] = { 0 }, maxerr[NFUNCS] = { 0 };
			int n = 0;
			for ( int lat = -80; lat<=80; lat += 10 )
			{
				for ( int lon = -179; lon<180; lon += 3 )
				{
					for ( int b = 0; b<360; b += 3, n++ )
					{
						double t[4] = { double ( lat ), double ( lon ), 0, 0 };
						DestVincenty ( t[0], t[1], b, dist, t + 2, t + 3 );
						for ( int j = 0; j<4; j++ )
							dBench.Add ( t[j] );
						for ( int f = 0; f<NFUNCS; f++ )
						{
							float fDist = CalcGeofunc ( f, t );
							double err = fabs ( 100 * ( double ( fDist ) - double ( dist ) )
													/ double ( dist ) ); // relative error, in percents
							avgerr[f] += err;
							maxerr[f] = Max ( err, maxerr[f] );
						}
					}
				}
			}
			if ( dist>=1000 )
				printf ( "%5dkm", dist / 1000 );
			else
				printf ( "%6dm", dist );
			for ( int f = 0; f<NFUNCS; f++ )
				printf ( ", f%d %5.2f%% %5.2f%%", f, avgerr[f] / n, maxerr[f] );
			printf ( "\n" );
		}

	const int RUNS = 10;
	float fDist = 0;
	double * tmax = dBench.Begin () + dBench.GetLength ();
	int64_t tm;
	printf ( "%d calls in bench\n", RUNS * dBench.GetLength () );

	tm = sphMicroTimer ();
	for ( int r = 0; r<RUNS; r++ )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistSphereDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	printf ( INT64_FMT" us sphere\n", sphMicroTimer () - tm );

	tm = sphMicroTimer ();
	for ( int r = 0; r<RUNS; r++ )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistFlatDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	printf ( INT64_FMT" us flat\n", sphMicroTimer () - tm );

	tm = sphMicroTimer ();
	for ( int r = 0; r<RUNS; r++ )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistAdaptiveDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	printf ( INT64_FMT" us adaptive\n", sphMicroTimer () - tm );

	printf ( "res %f\n", fDist );

	// coordinates from Wikimapia/Googlemaps
	//
	// distances by Wolfram Alpha (supposedly defaults to Vincenty method)
	// geodistance[{51.5007788, -0.1246771}, {46.2041222, 6.1524349}]
	//
	// 40.6890895, -74.0446899 center of the torch of the Statue of Liberty, New York, USA
	// 40.7041146, -74.0152399 center of The Sphere in Battery Park, New York, USA
	// 40.7643929, -73.9997683 tip of Lockheed A-12 (SR-71) on Intrepid, NY, USA
	// 40.7642578, -73.9994565 tail of Lockheed A-12 (SR-71) on Intrepid, NY, USA
	// 55.7535204, 37.6195371 center of Senatskaya tower, Red Square, Moscow, Russia
	// 51.6606654, 39.1999751 center of Lenin statue, Lenin Square, Voronezh, Russia
	// 49.2055275, -123.2014474 NW corner of Runway 08L-26R, YVR airport, Vancouver, Canada
	// 49.2007563, -123.1596478 NE corner of Runway 08L-26R, YVR airport, Vancouver, Canada
	// 37.6284983, -122.3927365 N corner of L on Runway 10L-28R, SFO airport, San Francisco, USA
	// 37.6137799, -122.3577954 S corner of R on Runway 10L-28R, SFO airport, San Francisco, USA
	// 68.974714, 33.0611873 tip of Lenin icebreaker, Murmansk, Russia
	// -22.9519125, -43.2105616 center of the head of Christ the Redeemer statue, Rio de Janeiro, Brazil
	// 51.5007788, -0.1246771 tip of Big Ben tower, London, England
	// 29.97973, 31.1342695 tip of Pyramid of Cheops, Cairo, Egypt
	// 41.4034549, 2.1741718 tip of the southern tower of Sagrada Familia, Barcelona, Spain
	// 42.6848586, 23.3188623 tip of National Palace of Culture, Sofia, Bulgaria
	// 46.2041222, 6.1524349 center of the fountain in English garden, Geneva, Switzerland
	// 37.8106517, -122.4174678 tip of SS Jeremiah O'Brien, Pier 45, San Francisco, USA
	// 37.8114358, -122.4186279 tail of SS Jeremiah O'Brien, Pier 45, San Francisco, USA
	// 64.1475975, -21.9224185 center of Sun Voyager in Reykjavik, Iceland
	// 63.8079982, -19.5589042 center of Eyjafjallajokull volcano, Iceland
	double dTest[][5] =
	{
		{ 40.7643929, -73.9997683, 40.7642578, -73.9994565, 30.3013 }, // Lockheed A-12 (SR-71) length (30.97m per wiki)
		{ 37.8106517, -122.4174678, 37.8114358, -122.4186279, 134.20 }, // SS Jeremiah O'Brien length ((134.57m per wiki)
		{ 40.6890895, -74.0446899, 40.7041146, -74.0152399, 2996.59 }, // Statue of Liberty to The Sphere
		{ 49.2055275, -123.2014474, 49.2007563, -123.1596478, 3091.96 }, // YVR Runway 08L-26R length (3030m per wiki)
		{ 37.6284983, -122.3927365, 37.6137799, -122.3577954, 3490.54 }, // SFO Runway 10L-28R length (3618m per wiki)
		{ 64.1475975, -21.9224185, 63.8079982, -19.5589042, 121768.14 }, // Reykjavik to Eyjafjallajokull
		{ 55.7535204, 37.6195371, 51.6606654, 39.1999751, 467301.55 }, // Moscow to Voronezh
		{ 51.5007788, -0.1246771, 46.2041222, 6.1524349, 747189.88 }, // London to Geneva
		{ 51.5007788, -0.1246771, 41.4034549, 2.1741718, 1136075.00 }, // London to Barcelona
		{ 51.5007788, -0.1246771, 42.6848586, 23.3188623, 2019138.10 }, // London to Sofia
		{ 51.5007788, -0.1246771, 29.97973, 31.1342695, 3513002.04 }, // London to Cairo
		{ 68.974714, 33.0611873, -22.9519125, -43.2105616, 11833803.11 }, // Murmansk to Rio
		{ 0, 0, 0.5, 179.5, 19936288.579 }, // antipodes, direct Vincenty killer
		// { 0, 0, 0.5, 179.7, 19944127.421 }, // antipodes, inverse Vincenty killer
	};

	for ( int i=0; i<sizeof(dTest)/sizeof(dTest[0]); i++ )
	{
		double * t = dTest[i];
		printf ( "%2d: ref %10.1f", i, t[4] );
		for ( int iFunc=0; iFunc<NFUNCS; iFunc++ )
		{
			float fDist = CalcGeofunc ( iFunc, t );
			printf ( ", f%d %5.2f%%", iFunc, 100*(fDist-t[4])/t[4] );
		}
		printf ( "\n" );
	}
	printf ( "\n" );
}