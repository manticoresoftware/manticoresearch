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

#include <gtest/gtest.h>

#include "sphinxint.h"
#include "sphinxutils.h"
#include "json/cJSON.h"
#include "threadutils.h"
#include <cmath>
#include "histogram.h"
#include "conversion.h"
#include "digest_sha1.h"

// Miscelaneous short functional tests: TDigest, SpanSearch,
// stringbuilder, CJson, TaggedHash, Log2

//////////////////////////////////////////////////////////////////////////


class TDigest : public ::testing::Test
{

protected:
	void SetUp() override
	{
		sphSrand ( 0 );
		pDigest = sphCreateTDigest ();
	}

	std::unique_ptr<TDigest_i> pDigest;
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
	ASSERT_EQ ( FindSpan ( dVec, 0, 5 ), -1 );
	ASSERT_EQ ( FindSpan ( dVec, 11, 5 ), 2 );

	dVec.Add ( 15 );
	dVec.Add ( 17 );
	dVec.Add ( 22 );
	dVec.Add ( 23 );

	ASSERT_EQ ( FindSpan ( dVec, 1, 5 ), 0 );
	ASSERT_EQ ( FindSpan ( dVec, 18, 5 ), 4 );
	ASSERT_EQ ( FindSpan ( dVec, 23, 5 ), 6 );
	ASSERT_EQ ( FindSpan ( dVec, 0, 5 ), -1 );
	ASSERT_EQ ( FindSpan ( dVec, 31, 5 ), 6 );
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

TEST ( functions, SHA1_hashing )
{
	auto sData = FromSz ( "bla-bla-bla" );
	auto sHash = CalcSHA1 ( sData.first, sData.second );
	ASSERT_STREQ ( sHash.cstr(), "1d537ba3814495b5be2c8f6537e4bd6764fcc9b4");
}


//////////////////////////////////////////////////////////////////////////
unsigned int nlog2 ( uint64_t x )
{
	x |= ( x >> 1 ) | 1;
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
	EXPECT_EQ ( sphLog2 ( 0 ), 1 );
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

	EXPECT_EQ ( nlog2 ( 0 ), 1 ) << "emulated";
	EXPECT_EQ ( nlog2 ( 1 ), 1 );
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

TEST ( functions, sphCalcZippedLen )
{
	EXPECT_EQ ( sphCalcZippedLen ( 0 ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 1 ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 2 ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 4 ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 8 ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x10 ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x20 ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x40 ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x7F ), 1 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x80 ), 2 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0100 ), 2 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0200 ), 2 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0400 ), 2 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0800 ), 2 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x1000 ), 2 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x2000 ), 2 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x3FFF ), 2 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x4000 ), 3 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x8000 ), 3 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00010000 ), 3 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00020000 ), 3 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00040000 ), 3 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00080000 ), 3 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00100000 ), 3 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x001FFFFF ), 3 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00200000 ), 4 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00400000 ), 4 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00800000 ), 4 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x01000000 ), 4 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x02000000 ), 4 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x04000000 ), 4 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x08000000 ), 4 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0FFFFFFF ), 4 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x10000000 ), 5 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x20000000 ), 5 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x40000000 ), 5 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x80000000 ), 5 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000000100000000 ), 5 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000000200000000 ), 5 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000000400000000 ), 5 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00000007FFFFFFFF ), 5 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000000800000000 ), 6 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000001000000000 ), 6 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000002000000000 ), 6 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000004000000000 ), 6 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000008000000000 ), 6 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000010000000000 ), 6 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000020000000000 ), 6 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x000003FFFFFFFFFF ), 6 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000040000000000 ), 7 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000080000000000 ), 7 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000100000000000 ), 7 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000200000000000 ), 7 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000400000000000 ), 7 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0000800000000000 ), 7 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0001000000000000 ), 7 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0001FFFFFFFFFFFF ), 7 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0002000000000000 ), 8 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0004000000000000 ), 8 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0008000000000000 ), 8 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0010000000000000 ), 8 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0020000000000000 ), 8 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0040000000000000 ), 8 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0080000000000000 ), 8 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x00FFFFFFFFFFFFFF ), 8 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0100000000000000 ), 9 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0200000000000000 ), 9 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0400000000000000 ), 9 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x0800000000000000 ), 9 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x1000000000000000 ), 9 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x2000000000000000 ), 9 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x4000000000000000 ), 9 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x7FFFFFFFFFFFFFFF ), 9 );
	EXPECT_EQ ( sphCalcZippedLen ( 0x8000000000000000 ), 10 );
	EXPECT_EQ ( sphCalcZippedLen ( 0xFFFFFFFFFFFFFFFF ), 10 );
}

//////////////////////////////////////////////////////////////////////////

CSphMutex g_Mutex1;

void TimedLockTest ()
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
	ASSERT_TRUE ( Threads::Create ( &th, TimedLockTest ) ) << "timedlock thread created";
	sphSleepMsec ( 3500 );
	ASSERT_TRUE ( g_Mutex1.Unlock () ) << "unlocked";
	ASSERT_TRUE ( Threads::Join ( &th ) ) << "timedlock thread done";
}

//////////////////////////////////////////////////////////////////////////

class TZip: public ::testing::Test
{
protected:
	void SetUp() override
	{
		uint64_t uBase = 0xDEADBEAF12345678;
		for ( auto i = 0; i < 65; ++i )
		{
			uint64_t c64 = uBase >> i;
			dValues64.Add ( c64 );
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE64.Add ( b ); }, c64 );
			ZipValueLE ( [this] ( BYTE b ) mutable { dBufLE64.Add ( b ); }, c64 );

			DWORD c32 = c64 & 0xFFFFFFFF;
			dValues32.Add ( c32 );
			ZipValueBE ( [this] ( BYTE b ) mutable { dBufBE.Add ( b ); }, c32 );
			ZipValueLE ( [this] ( BYTE b ) mutable { dBufLE.Add ( b ); }, c32 );
		}
	}

	CSphVector<DWORD> dValues32;
	CSphVector<uint64_t> dValues64;
	CSphVector<BYTE> dBufBE;
	CSphVector<BYTE> dBufLE;
	CSphVector<BYTE> dBufBE64;
	CSphVector<BYTE> dBufLE64;
};

TEST_F ( TZip, BE32 )
{
	const BYTE* pBuf = dBufBE.begin();
	for ( auto& ref : dValues32 )
	{
		auto val = UnzipIntBE ( pBuf );
		ASSERT_EQ ( ref, val );
	}
}

TEST_F ( TZip, BE64 )
{
	const BYTE* pBuf = dBufBE64.begin();
	for ( auto& ref : dValues64 )
	{
		auto val = UnzipOffsetBE ( pBuf );
		ASSERT_EQ ( ref, val );
	}
}

TEST_F ( TZip, LE32 )
{
	const BYTE* pBuf = dBufLE.begin();
	for ( auto& ref : dValues32 )
	{
		auto val = UnzipIntLE ( pBuf );
		ASSERT_EQ ( ref, val );
	}
}

TEST_F ( TZip, LE64 )
{
	const BYTE* pBuf = dBufLE64.begin();
	for ( auto& ref : dValues64 )
	{
		auto val = UnzipOffsetLE ( pBuf );
		ASSERT_EQ ( ref, val );
	}
}

//////////////////////////////////////////////////////////////////////////

static int g_iRwlock;
static RwLock_t g_tRwlock;

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
	g_iRwlock += static_cast<int>(reinterpret_cast<intptr_t>(pArg));
	sphSleepMsec ( 3 );
	ASSERT_TRUE ( g_tRwlock.Unlock () );
}

TEST ( functions, RWLock )
{
	const int NPAIRS = 10;
	SphThread_t dReaders[NPAIRS];
	SphThread_t dWriters[NPAIRS];
	int dRead[NPAIRS];

	g_iRwlock = 0;
	for ( int i = 0; i<NPAIRS; i++ )
	{
		ASSERT_TRUE ( Threads::Create ( &dReaders[i], [&,i] { RwlockReader ( &dRead[i] );} ));
		ASSERT_TRUE ( Threads::Create ( &dWriters[i], [&,i] { RwlockWriter ( reinterpret_cast<void *>(static_cast<intptr_t>(1 + i) ) );} ));
	}

	for ( int i = 0; i<NPAIRS; i++ )
	{
		ASSERT_TRUE ( Threads::Join ( &dReaders[i] ) );
		ASSERT_TRUE ( Threads::Join ( &dWriters[i] ) );
	}

	ASSERT_EQ ( g_iRwlock, NPAIRS * ( 1 + NPAIRS ) / 2 );

	int iReadSum = 0;
	for ( int i : dRead )
		iReadSum += i;

	RecordProperty ( "read_sum", iReadSum );
}

//////////////////////////////////////////////////////////////////////////

CSphAutoEvent g_multievent;
OneshotEvent_c g_oneevent;
volatile int64_t tmNow;

int getms()
{
	return (sphMicroTimer () - tmNow)/1000;
}

void AutoEventest ()
{
	printf("\n%d thread started", getms());
	for ( int i=0; i<5; ++i)
	{
		ASSERT_TRUE ( g_multievent.WaitEvent ()) << "WaitEvent";
		printf ( "\n%d B%d: %d-st event waited", getms (),i+1,i);
	}
}

void AutoEventestTimed ()
{
	printf ( "\n%d B1: started", getms ());
	for ( int i = 0; i<5; ++i )
	{
		ASSERT_TRUE ( g_multievent.WaitEvent (500)) << "WaitEvent";
		printf ( "\n%d B%d: %d-st event waited", getms (), i+2, i );
	}
	ASSERT_FALSE ( g_multievent.WaitEvent ( 200 )) << "WaitEvent";
	printf ( "\n%d B7: one event timed-out", getms () );
	ASSERT_TRUE ( g_multievent.WaitEvent ( 500 )) << "WaitEvent";
	ASSERT_TRUE ( g_multievent.WaitEvent ( 500 )) << "WaitEvent";
	printf ( "\n%d B8: last 2 events succeeded", getms ());
}

// multievent - we can set it N times, and then it may be waited N times also, but N+1 will block
TEST ( functions, MultiAutoEvent )
{
	tmNow = sphMicroTimer ();
	SphThread_t th;

	// set 4 events before event start the thread. Expect, it will be catched.
	printf ( "\n%d A1: set event 4 times a row", getms ());
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();


	// now start the thread, it will receive events
	ASSERT_TRUE ( Threads::Create ( &th, AutoEventest )) << "autoevent thread created";
	printf ( "\n%d A2: created working thread", getms () );
	// sleep half-a-second and set last event.
	sphSleepMsec ( 500 );
	printf ( "\n%d A3: set event", getms () );
	g_multievent.SetEvent ();

	sphSleepMsec ( 100 );
	ASSERT_TRUE ( Threads::Join ( &th )) << "autoevent thread done";
}

TEST ( functions, MultiAutoEventTimed )
{
	tmNow=sphMicroTimer ();
	SphThread_t th;

	// set 4 events before event start the thread. Expect, it will be catched.
	printf ( "\n%d 4 events set", getms () );
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();

	// now start the thread, it will receive events
	ASSERT_TRUE ( Threads::Create ( &th, AutoEventestTimed ) ) << "autoeventtimed thread created";
	printf ( "\n%d A2: created working thread", getms () );

	// sleep half-a-second and set last event.
	sphSleepMsec ( 100 );
	printf ( "\n%d A3: set event", getms () );
	g_multievent.SetEvent ();
	sphSleepMsec ( 400 );
	printf ( "\n%d A4: set event twice", getms () );
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	sphSleepMsec ( 100 );
	ASSERT_TRUE ( Threads::Join ( &th ) ) << "autoevent thread done";
}

void OneshotEventTest()
{
	printf ( "\n%d thread started", getms ());
	for ( int i = 0; i<2; ++i )
	{
		ASSERT_TRUE ( g_oneevent.WaitEvent ()) << "WaitEvent";
		printf ( "\n%d thread %d-st event waited", getms (), i );
	}
}

void OneshotEventTestTimed()
{
	printf ( "\n%d B1: started", getms ());
	bool bRes=g_oneevent.WaitEvent ( 500 ); ASSERT_TRUE ( bRes ) << "WaitEvent";
	printf ( "\n%d B2: 1-st event returned %s", getms (), bRes?"true":"false" );
	bRes=g_oneevent.WaitEvent ( 500 ); ASSERT_TRUE ( bRes ) << "WaitEvent";
	printf ( "\n%d B3: 2-nd event returned %s", getms (), bRes?"true":"false" );
	bRes=g_oneevent.WaitEvent ( 500 ); ASSERT_TRUE ( bRes ) << "WaitEvent";
	printf ( "\n%d B4: 3-rd event returned %s", getms (), bRes?"true":"false" );
	sphSleepMsec ( 100 );
	bRes=g_oneevent.WaitEvent ( 500 ); ASSERT_FALSE ( bRes ) << "WaitEvent";
	printf ( "\n%d B5: 4-th event returned %s", getms (), bRes?"true":"false" );
}

// oneshot event - we can set it N times, but only once it waited, and then will block.
TEST ( functions, OneshotAutoEvent )
{
	tmNow = sphMicroTimer ();
	SphThread_t th;

	// set 4 events before event start the thread. Expect, it will be catched.
	printf ( "\n%d A1: set event 4 times a row", getms ());
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();

	// now start the thread, it will receive events
	ASSERT_TRUE ( Threads::Create ( &th, OneshotEventTest )) << "autoevent thread created";
	printf ( "\n%d A2: created working thread", getms ());

	// sleep half-a-second and set last event.
	sphSleepMsec ( 500 );
	printf ( "\n%d A3: set event", getms ());
	g_oneevent.SetEvent ();
	sphSleepMsec ( 100 );
	ASSERT_TRUE ( Threads::Join ( &th )) << "autoevent thread done";
}

// oneshot event - we can set it N times, but only once it waited, and then will block.
/*TEST ( functions, DISABLED_OneshotAutoEventTimed )
{
	tmNow=sphMicroTimer ();
	SphThread_t th;
	
	// set 4 events before event start the thread. Expect, it will be catched.
	printf ( "\n%d A1: set event 4 times a row", getms () );
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();

	// now start the thread, it will receive events
	ASSERT_TRUE ( Threads::Create ( &th, OneshotEventTestTimed ) ) << "autoevent thread created";
	printf ( "\n%d A2: created working thread", getms () );

	// sleep half-a-second and set last event.
	sphSleepMsec ( 100 );
	printf ( "\n%d A3: set event", getms () );
	g_oneevent.SetEvent ();
	sphSleepMsec ( 400 );
	printf ( "\n%d A4: set event twice", getms () );
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	sphSleepMsec ( 100 );
	ASSERT_TRUE ( Threads::Join ( &th ) ) << "autoevent thread done";
}*/

//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma warning(push) // store current warning values
#pragma warning(disable:4101)
#endif

TEST ( functions, Hash_simple )
{
	// add and verify a couple keys manually
	OpenHashTable_T<int64_t, int> h;
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

	HashFunc_Int64_t tHashFunc;
	for ( int i = 0; i<2 * DUPES; i++ )
	{
		ASSERT_EQ ( tHashFunc.GetHash ( dupes[i] ), tHashFunc.GetHash ( dupes[( i >> 1 ) << 1] ) );
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

#ifdef _WIN32
#pragma warning(pop) // restore warnings
#endif

TEST ( functions, HASH_randomized )

// big randomized test
{
	OpenHashTable_T<int64_t, int> h;
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

TEST ( functions, string_split )
{
	StrVec_t dStr;
	sphSplit ( dStr, "test:me\0off\0", ":" );
	ASSERT_EQ ( dStr.GetLength (), 2 );
	ASSERT_STREQ ( dStr[0].cstr(),"test" );
	ASSERT_STREQ ( dStr[1].cstr(), "me" );

	dStr.Reset();
	sphSplit ( dStr, "  white\tspace\rsplit\ntrying ");
	ASSERT_EQ ( dStr.GetLength (), 4 );
	ASSERT_STREQ ( dStr[0].cstr (), "white" );
	ASSERT_STREQ ( dStr[1].cstr (), "space" );
	ASSERT_STREQ ( dStr[2].cstr (), "split" );
	ASSERT_STREQ ( dStr[3].cstr (), "trying" );

	dStr.Reset();
	sphSplit ( dStr, ":start:finish:", ":" );
	ASSERT_EQ ( dStr.GetLength (), 3 );
	ASSERT_STREQ ( dStr[0].cstr (), "" );
	ASSERT_STREQ ( dStr[1].cstr (), "start" );
	ASSERT_STREQ ( dStr[2].cstr (), "finish" );
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
		tWrDef.CloseFile();
	}
	{
		CSphWriter tWr;
		tWr.SetBufferSize ( WRITE_OUT_DATA_SIZE );
		tWr.OpenFile ( sTmpWriteout, sErr );
		tWr.PutBytes ( pData, WRITE_OUT_DATA_SIZE );
		tWr.PutByte ( 0xff );
		tWr.CloseFile();
	}
	unlink ( sTmpWriteout.cstr () );
	delete[] pData;
}

//////////////////////////////////////////////////////////////////////////
struct tstcase { float wold; DWORD utimer; float wnew; };

static void TestRebalance_fn ( tstcase * pData, int iLen, int iStride )
{
	ASSERT_FALSE ( iLen % iStride );
	iLen /= iStride;
	CSphFixedVector<int64_t> dTimers ( iStride );
	CSphFixedVector<float> dWeights ( iStride );
	for ( int i = 0; i<iLen; ++i )
	{
		for ( int j = 0; j<iStride; ++j )
		{
			dWeights[j] = pData[i * iStride + j].wold;
			dTimers[j] = pData[i * iStride + j].utimer;
		}

		RebalanceWeights ( dTimers, dWeights );

		for ( int j = 0; j<iStride; ++j )
		{
			ASSERT_NEAR ( dWeights[j], pData[i * iStride + j].wnew, 0.01)
				<< " \n----dWeights[" << j << "]=" << dWeights[j] << " vs " << pData[i * iStride + j].wnew;
		}
	}
}

TEST ( functions, Rebalance )
{
	//				  old weights, timers,	new weights
	tstcase dData1[] = { {50.5669f, 186751,	55.0625f}, {49.4316f, 228828, 44.9375f},
						 {55.6222f, 207608,	51.2823f}, {44.3763f, 218537, 48.7177f},
						 {56.8841f, 214800,	47.4951f}, {43.1144f, 194305, 52.5049f},
						 {54.4091f, 207614,	47.7932f}, {45.5894f, 190062, 52.2068f},
						 {52.2103f, 221708,	47.5706f}, {47.7882f, 201162, 52.4294f},
						 {49.7810f, 247379,	43.8821f}, {50.2174f, 193441, 56.1179f},
						 {43.6667f, 223202,	46.6167f}, {56.3317f, 194910, 53.3833f},
						 {40.3662f, 361018,	38.7370f}, {59.6323f, 228274, 61.2630f},
						 {29.9718f, 275050,	44.7756f}, {70.0267f, 223009, 55.2244f},
						 {25.7618f, 279008,	42.3951f}, {74.2367f, 205340, 57.6049f},
						 {20.3433f, 201466,	51.4136f}, {79.6551f, 213189, 48.5864f},
						 {21.2741f, 197584,	51.5511f}, {78.7243f, 210235, 48.4489f},
						 {25.3498f, 318349,	39.5014f}, {74.6487f, 207860, 60.4986f},
						 {18.1476f, 487120,	29.5299f}, {81.8509f, 204124, 70.4701f},
						 {08.5008f, 412733,	32.9526f}, {91.4977f, 202851, 67.0474f} };
	TestRebalance_fn ( dData1, sizeof(dData1) / sizeof( tstcase), 2 );

	tstcase dData2[] = { { 0.000000f, 0, 0.00000f }, { 00.0015f, 18469, 100.0000f } };
	TestRebalance_fn ( dData2, sizeof(dData2) / sizeof( tstcase), 2 );

	tstcase dData3[] = { { 0.000000f, 0, 0.00000f }, { 0.0015f, 0, 0.00000f }
						 , { 0.0031f, 0, 0.00000f }, { 0.0046f, 18469, 100.0000f } };
	TestRebalance_fn ( dData3, sizeof ( dData3 ) / sizeof ( tstcase ), 4 );

	tstcase dData4[] = { { 0.000000f, 7100, 72.2320f }, { 0.0015f, 0, 0.0f }, { 0.0031f, 18469, 27.7679f } };
	TestRebalance_fn ( dData4, sizeof ( dData4 ) / sizeof ( tstcase ), 3 );
}

//////////////////////////////////////////////////////////////////////////
// parsing size - number with possible suffixes k, m, g, t.
TEST (functions, size_parser)
{
	// upper case suffixes
	ASSERT_EQ ( 1024, sphGetSize64 ( "1K" ) );
	ASSERT_EQ ( 1024 * 1024, sphGetSize64 ( "1M" ) );
	ASSERT_EQ ( 1024 * 1024 * 1024, sphGetSize64 ( "1G" ) );
	ASSERT_EQ ( 1024ULL * 1024 * 1024 * 1024, sphGetSize64 ( "1T" ) );

	// lower case suffixes;
	// Untouched sError on success;
	char * sError = nullptr;
	ASSERT_EQ ( 1, sphGetSize64 ( "1", &sError ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 1024, sphGetSize64 ( "1k", &sError ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 1024 * 1024, sphGetSize64 ( "1m", &sError ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 1024 * 1024 * 1024, sphGetSize64 ( "1g", &sError ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 1024ULL * 1024 * 1024 * 1024, sphGetSize64 ( "1t", &sError ) );
	ASSERT_EQ ( sError, nullptr );

	// empty and null input strings
	ASSERT_EQ ( 11, sphGetSize64 ( "", &sError, 11 ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 12, sphGetSize64 ( nullptr, &sError, 12 ) );
	ASSERT_EQ ( sError, nullptr );

	// error handle for non-numeric
	ASSERT_EQ ( -1, sphGetSize64 ( "abc", &sError ) );
	ASSERT_STREQ (sError,"abc");

	// error handle for numeric, but unknown suffix (=non-numeric)
	ASSERT_EQ ( -1, sphGetSize64 ( "10z", &sError ) );
	ASSERT_STREQ ( sError, "z" );
}

// parsing time - number with possible suffixes us, ms, s, m, h, d, w
TEST ( functions, sphGetTime64 )
{
	static const struct
	{ int64_t tm; const char* str; } models[] = {
		{ 1, "1us" }, { 2, "2Usm" }, { 3, "3uS" }, { 4, "4US" }, // useconds
		{ 1000, "1ms" }, { 2000, "2Ms" }, { 3000, "3mS" }, { 4000, "4MS" },// milliseconds
		{ 1000000, "1" }, { 2000000, "2s" }, { 3000000, "3S" }, // seconds
		{ 60000000, "1m" }, { 120000000, "2M" }, // minutes
		{ 3600000000, "1h" }, { 36000000000, "10H" }, // hours
		{ 24ULL * 3600000000, "1D" }, { 48ULL * 3600000000, "2d" }, // days
		{ 7ULL * 24 * 3600000000, "1W" }, { 14ULL * 24 * 3600000000, "2w" }, // weeks
	};

	for ( const auto& model : models )
		EXPECT_EQ ( model.tm, sphGetTime64 (model.str) ) << "for " << model.tm << " and " << model.str;
}

// Untouched sError on success;
TEST ( functions, sphGetTime64_nullerror )
{
	static const struct
	{ int64_t tm; const char* str; } models[] = {
		{ 1000000, "1" }, { 2, "2us" }, { 1000000, "1s" }, { 60000000, "1m" },
		{ 3600000000, "1h" }, { 24ULL * 3600000000, "1d" }, { 7ULL * 24 * 3600000000, "1w" },
	};

	char* sError = nullptr;
	for ( const auto& model : models ) {
		EXPECT_EQ ( model.tm, sphGetTime64 ( model.str, &sError )) << "for " << model.tm << " and " << model.str;
		EXPECT_EQ ( sError, nullptr ) << "for " << model.tm << " and " << model.str;
	}
}

// empty and null input strings
TEST ( functions, sphGetTime64_defaults )
{
	char* sError = nullptr;
	ASSERT_EQ ( 11, sphGetTime64 ( "", &sError, 11 ));
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 12, sphGetTime64 ( nullptr, &sError, 12 ));
	ASSERT_EQ ( sError, nullptr );
}

// processing errors
TEST ( functions, sphGetTime64_errors )
{
	static const struct
	{ int64_t res; const char* str; const char* err;} models[] = {
		{ -1, "abc", "abc" }, // error handle for non-numeric
		{ -1, "10z", "z" }, // error handle for numeric, but unknown suffix (=non-numeric)
	};

	char* sError = nullptr;
	for ( const auto& model : models ) {
		EXPECT_EQ ( model.res, sphGetTime64 ( model.str, &sError ));
		EXPECT_STREQ ( sError, model.err ) << "for " << model.res << " and " << model.str << " err " << model.err;
	}
}

TEST ( functions, hashmap_iterations )
{
	struct
	{
		int iVal;
		const char * sKey;
	} tstvalues[] =
		{ {   1, "one" }
		  , { 2, "two" }
		  , { 3, "three" }
		  , { 4, "four" } };

	SmallStringHash_T<int> tHash;
	for ( auto &test: tstvalues )
		tHash.Add ( test.iVal, test.sKey );

	auto i = 0;
	for ( const auto& tValue : tHash )
	{
		EXPECT_STREQ ( tValue.first.cstr (), tstvalues[i].sKey );
		EXPECT_EQ ( tValue.second, tstvalues[i].iVal );
		++i;
	}

	// test ranged-for iterations
	i = 0;
	for ( const auto& mp : tHash )
	{
		EXPECT_STREQ ( mp.first.cstr (), tstvalues[i].sKey );
		EXPECT_EQ ( mp.second, tstvalues[i].iVal );
		++i;
	}
}

TEST ( functions, vector )
{
	CSphVector<int> dVec;
	dVec.Add(1);
	dVec.Add(2);
	auto & dv = dVec.Add();
	dv = 3;
	dVec.Add(4);
	dVec.Add ( 5 );
	dVec.Add ( 6 );
	dVec.Add ( 7 );
	dVec.RemoveValue (2);
	dVec.Add ( 8 );
	dVec.Add ( 9 );
	dVec.RemoveValue ( 9);
	dVec.Add ( 9 );
	dVec.Add ( 10);
	dVec.RemoveValue ( 10 );
	ASSERT_EQ (dVec.GetLength (),8);
}

TEST ( functions, vector_slice )
{
	CSphVector<int> dVec;
	auto dSlice0 = dVec.Slice();
	ASSERT_TRUE ( dSlice0.IsEmpty() );
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	dVec.Add ( 3 );
	
	auto dSlice1 = dVec.Slice(-1,20);
	ASSERT_EQ ( dSlice1.begin(), dVec.begin());
	ASSERT_EQ ( dSlice1.GetLength (), 3 );
	
	auto dSlice2 = dVec.Slice (10,10);
	ASSERT_TRUE ( dSlice2.IsEmpty() );
	
	auto dSlice3 = dVec.Slice(1);
	ASSERT_EQ ( dSlice3.GetLength(),2);
	ASSERT_EQ ( dSlice3[0], 2);
	ASSERT_EQ ( dSlice3[1], 3 );
	ASSERT_EQ ( dSlice3.begin(), &dVec[1]);
	
	auto dSlice4 = dVec.Slice(1,1);
	ASSERT_EQ ( dSlice4.GetLength(), 1);
	ASSERT_EQ ( dSlice4.begin (), &dVec[1] );
}

TEST ( functions, vector2pair_and_pair2mva )
{
	CSphVector<DWORD> dVec;
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	dVec.Add ( 3 );
	ASSERT_EQ ( dVec.GetLength(), 3 );

	ByteBlob_t dBlob { dVec };

	ASSERT_EQ ( dBlob.first, (const BYTE*)dVec.begin() );
	ASSERT_EQ ( dBlob.second, 12 ) << "3 DWORDS are 12 bytes";

	dVec.Add ( 4 );
	ByteBlob_t dMva {dVec};

	std::pair<int64_t *, int> dTest {dVec};
	ASSERT_EQ ( dTest.first, (const int64_t *) dVec.begin ());
	ASSERT_EQ ( dTest.second, 2 ) << "4 DWORDS are 2 int64s";

	VecTraits_T<DWORD> dMva32 { dMva };
	ASSERT_EQ ( dMva32.GetLength (), 4 ) << "mva32 from 16 bytes contains 4 values";
	ASSERT_EQ ( dMva32.begin(), dVec.begin() );

	VecTraits_T<int64_t> dMva64 {dMva};
	ASSERT_EQ ( dMva64.GetLength (), 2 ) << "mva64 from 16 bytes contains 2 values";
}

TEST ( functions, sphSplit )
{
	StrVec_t dParts;
	sphSplit ( dParts, "a:b,c_", ":,_");
	ASSERT_EQ ( dParts.GetLength (), 3 );
	ASSERT_STREQ ( dParts[0].cstr (), "a" );
	ASSERT_STREQ ( dParts[1].cstr (), "b" );
	ASSERT_STREQ ( dParts[2].cstr (), "c" );

	dParts.Reset();
	sphSplit ( dParts, "a:", ":" );
	
	ASSERT_EQ ( dParts.GetLength (), 1 );
	ASSERT_STREQ ( dParts[0].cstr (), "a" );

	dParts.Reset ();
	sphSplit ( dParts, ":a", ":" );

	ASSERT_EQ ( dParts.GetLength (), 2 );
	ASSERT_STREQ ( dParts[0].cstr (), "" );
	ASSERT_STREQ ( dParts[1].cstr (), "a" );
}

// as we found g++ (GCC) 4.8.5 20150623 (Red Hat 4.8.5-28) works strange with curly initializer of refs
TEST ( functions, curledref )
{
	CSphString sProof="abc";
	const CSphString &sTest { sProof };
	const CSphString &sTestc ( sProof );
	ASSERT_TRUE ( &sProof==&sTestc ) << "curly brackets";
	ASSERT_TRUE ( &sProof==&sTest ) << "figured brackets";
}

TEST ( functions, valgrind_use )
{
	BYTE* VARIABLE_IS_NOT_USED pLeak = new BYTE[100];
	ASSERT_TRUE (true) << "intended leak";
}

TEST ( functions, int64_le )
{
	union {
		DWORD pMva[2] = {0x01020304, 0x05060708};
		int64_t Mva64;
	} u;

	// expression from MVA_UPSIZE
	auto iTest = ( int64_t ) ( ( uint64_t ) u.pMva[0] | ( ( ( uint64_t ) u.pMva[1] ) << 32 ) );
	auto iTest2 = MVA_UPSIZE ( u.pMva );
	ASSERT_EQ ( iTest, iTest2 );

#if USE_LITTLE_ENDIAN
	auto iTestLE = u.Mva64;
	ASSERT_EQ ( iTest, iTestLE ) << "little endian allows simplify";
#endif
}

TEST ( functions, FindLastNumeric )
{
	static const char * sNum1 = "12345";
	ASSERT_EQ ( sNum1, sphFindLastNumeric (sNum1,5 ));

	static const char * sNum2 = "1234 ";
	ASSERT_EQ ( sNum2+5, sphFindLastNumeric ( sNum2, 5 ) );

	static const char * sNum3 = "12 34";
	ASSERT_EQ ( sNum3 + 3, sphFindLastNumeric ( sNum3, 5 ) );
}

/*
 * Different helpers to investigate how copy/move would work
 * Run test functions.trainer to investigate what is finally happens.
 */
struct train_c
{
	int m_x = 0;

	train_c() { std::cout << "\n-CTR train default 0 " << this; }
	train_c(int x) : m_x (x) { std::cout << "\n-CTR train_c(x) " << m_x << " " << this; }

	train_c(const train_c& c) : m_x(c.m_x) { std::cout << "\n-COPY train ctr "
	<< m_x << " " << this << " from " << c.m_x << " " << &c;}
	train_c(train_c&& c) : m_x(c.m_x) { c.m_x = 0; std::cout << "\n-MOVE train ctr "
	<< m_x << " " << this << " from " << c.m_x << " " << &c;}

	train_c& operator= (const train_c& c) { m_x = c.m_x; std::cout << "\n-COPY train ="
		<< m_x << " " << this << " from " << &c; return *this;}
	train_c& operator= ( train_c&& c ) { m_x = c.m_x; c.m_x = 0; std::cout << "\n-MOVE train ="
		<< m_x << " " << this << " from " << &c; return *this;}

	~train_c() { std::cout << "\n-DTR train " << m_x << " " << this; m_x = 0;}
};

struct helper_c
{
	int pad = 0;
	train_c m_h;

	helper_c() { std::cout << "\nHELPER default " << this; }
//	helper_c( train_c c ) : m_h { std::move(c) } {
//		std::cout << "\nHELPER " << this << " from " << &c << " " << &m_h << " " << m_h.m_x; }

	template <typename TRAIN_C>
	helper_c ( TRAIN_C&& c ): m_h { std::forward<TRAIN_C> ( c ) }
	{
		std::cout << "\nHELPER_TT " << this << " from " << &c << " " << &m_h << " " << m_h.m_x;
	}

	~helper_c() { std::cout << "\n~HELPER " << this; }
};

template <typename TRAIN_C>
helper_c* make_helper ( TRAIN_C&& c )
{
	std::cout << "\n====>  called make_helper with " << &c;
	return new helper_c ( std::forward<TRAIN_C>(c) );
}

TEST ( functions, trainer )
{
	std::cout << "\n\n==>  usual pass";
	{
		train_c a (10);
		auto* foo = make_helper (a);
		std::cout << "\n==>  made foo " << foo->m_h.m_x << " a is " << a.m_x;
		delete foo;
	}

	std::cout << "\n\n==>  indirect ctr";
	auto fee = make_helper (11);
	std::cout << "\n==>  made fee " << fee->m_h.m_x;
	delete fee;

	std::cout << "\n\n==>  direct ctr";
	auto bar = make_helper ( train_c (12) );
	std::cout << "\n==>  made fee " << bar->m_h.m_x;
	delete bar;
}

helper_c* make_helper_byval( train_c c )
{
	std::cout << "\n====>  called make_helper_byval with " << &c;
	return new helper_c( std::move( c ));
}

TEST ( functions, trainer_by_val )
{
	std::cout << "\n\n==>  usual pass";
	{
		train_c a( 10 );
		auto* foo = make_helper_byval( a );
		std::cout << "\n==>  made foo " << foo->m_h.m_x << " a is " << a.m_x;
		delete foo;
	}

	std::cout << "\n\n==>  indirect ctr";
	auto fee = make_helper_byval( 11 );
	std::cout << "\n==>  made fee " << fee->m_h.m_x;
	delete fee;

	std::cout << "\n\n==>  direct ctr";
	auto bar = make_helper_byval( train_c( 12 ));
	std::cout << "\n==>  made fee " << bar->m_h.m_x;
	delete bar;
}


TEST ( functions, VectorEx )
{
	using namespace sph;
	CSphTightVector<int> dTVec;
	CSphVector<int> dVec;
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	auto &dv = dVec.Add ();
	dv = 3;
	dVec.Add ( 4 );
	dVec.Add ( 5 );
	dVec.Add ( 6 );
	dVec.Add ( 7 );
	dVec.RemoveValue ( 2 );
	dVec.Add ( 8 );
	dVec.Add ( 9 );
	dVec.RemoveValue ( 9 );
	dVec.Add ( 9 );
	dVec.Add ( 10 );
	dVec.RemoveValue ( 10 );
	ASSERT_EQ ( dVec.GetLength (), 8 );
	dTVec.Add(30);
	dTVec.Add(20);
	dVec.Append ( dTVec );
	ASSERT_EQ ( dVec.GetLength (), 10 );
//	dVec.SwapData (dTVec);
	LazyVector_T<int> dLVec;
	dLVec.Add(4);
	dLVec.Add(5);
	ASSERT_EQ ( dLVec.GetLength (), 2 );
	dTVec.Append (dLVec);
	ASSERT_EQ ( dTVec.GetLength (), 4 );
	int* VARIABLE_IS_NOT_USED pData = dTVec.LeakData();
}

TEST ( functions, VectorCopyMove )
{
	using vec = CSphVector<int>;
	vec dVec;
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	dVec.Add ( 3 );
	dVec.Add ( 4 );
	dVec.Add ( 5 );
	vec dCopy ( dVec ); // copy c-tr
	vec dCopy2; // default c-tr
	dCopy2 = dVec; // copy c-tr dVec to tmp, then swap dCopy2 with tmp; then d-tr of empty tmp.
	vec dMove ( std::move ( dCopy )); // move c-tr
	vec dMove2; // default ctr
	dMove2 = std::move ( dCopy2 ); // move ctr dCopy2 to tmp, swap dMove2 with tmp; dtr empty tmp.
}

TEST ( functions, LazyVectorCopyMove )
{
	using vec = LazyVector_T<int>;
	vec dVec;
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	dVec.Add ( 3 );
	dVec.Add ( 4 );
	dVec.Add ( 5 );
	// vec dCopy ( dVec ); // will not compile since copy c-tr is deleted
	vec dCopy;
	dCopy.Append(dVec);
}

TEST ( functions, BitVec_managing )
{
	{
		CSphBitvec foo ( 10 );
		CSphBitvec bar ( 1000 );
		foo.BitSet ( 9 );
		bar.BitSet ( 900 );
		bar.Swap ( foo );
		ASSERT_TRUE ( foo.BitGet ( 900 ) );
		ASSERT_TRUE ( bar.BitGet ( 9 ) );
	}
	{
		CSphBitvec baz;
		CSphBitvec fee ( 100 );
		fee.BitSet ( 90 );
		baz = std::move ( fee );
		ASSERT_TRUE ( baz.BitGet ( 90 ) );
	}
	{
		CSphBitvec baz;
		CSphBitvec fee ( 1000 );
		fee.BitSet ( 90 );
		baz = std::move ( fee );
		ASSERT_TRUE ( baz.BitGet ( 90 ) );
	}
}

TEST ( functions, warner_c )
{
	Warner_c sMsg;

	// output two errors - expect ,-separated
	sMsg.Err("Error 1");
	sMsg.Err("Error 2");
	ASSERT_STREQ ( sMsg.sError(), "Error 1, Error 2");

	// formatted output
	sMsg.Clear();
	sMsg.Err("Error %d", 10);
	ASSERT_STREQ ( sMsg.sError (), "Error 10" );

	// finalized combo output for errors only
	CSphString sFinal;
	sMsg.MoveAllTo (sFinal);
	ASSERT_STREQ ( sFinal.cstr (), "ERRORS: Error 10" );

	// finalized combo output for warnings only
	sMsg.Warn ( "msg 1" );
	sMsg.Warn ( "msg %d", 2 );
	sMsg.MoveAllTo ( sFinal );
	ASSERT_STREQ ( sFinal.cstr (), "WARNINGS: msg 1, msg 2" );

	// output two warnings (same as with error - expected ,-separated)
	sMsg.Warn ( "msg 1" );
	sMsg.Warn ( "msg %d", 2 );
	ASSERT_STREQ ( sMsg.sWarning (), "msg 1, msg 2" );

	// finalized combo output of both errors and warnings
	sMsg.Err ( "Error %d", 10 );
	sMsg.MoveAllTo ( sFinal );
	ASSERT_STREQ ( sFinal.cstr (), "ERRORS: Error 10; WARNINGS: msg 1, msg 2" );
}

// testing our priority queue
TEST ( functions, CSphQueue )
{
	int iMin = 1000;
	CSphQueue<int, SphLess_T<int> > qQ ( 10 );
	for ( auto iVal : { 89, 5, 4, 8, 4, 3, 1, 5, 4, 2 } )
	{
		qQ.Push ( iVal );
		iMin = Min ( iMin, iVal );
		ASSERT_EQ ( qQ.Root (), iMin ) << "min elem always on root";
	}

	ASSERT_EQ ( qQ.GetLength (), 10 ); ASSERT_EQ ( qQ.Root(), 1); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 9 ); ASSERT_EQ ( qQ.Root (), 2 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 8 ); ASSERT_EQ ( qQ.Root (), 3 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 7 ); ASSERT_EQ ( qQ.Root (), 4 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 6 ); ASSERT_EQ ( qQ.Root (), 4 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 5 ); ASSERT_EQ ( qQ.Root (), 4 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 4 ); ASSERT_EQ ( qQ.Root (), 5 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 3 ); ASSERT_EQ ( qQ.Root (), 5 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 2 ); ASSERT_EQ ( qQ.Root (), 8 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 1 ); ASSERT_EQ ( qQ.Root (), 89 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 0 );

	qQ.Push(1000);
	ASSERT_EQ ( qQ.Root (), 1000 ) << "pushed 1000 to empty, it is root now";

	qQ.Push ( 100 );
	ASSERT_EQ ( qQ.Root (), 100 ) << "pushed 100 over 1000, it became root now";

}


TEST ( functions, path )
{
	CSphString sSrc1 ( "/home/build/test/data/pq2" );
	CSphString sPath1 = GetPathOnly ( sSrc1 );
	ASSERT_STREQ ( sPath1.cstr(), "/home/build/test/data/" );

	CSphString sSrc2 ( "home/pq2" );
	CSphString sPath2 = GetPathOnly ( sSrc2 );
	ASSERT_STREQ ( sPath2.cstr(), "home/" );

	CSphString sSrc3 ( "/pq2" );
	CSphString sPath3 = GetPathOnly ( sSrc3 );
	ASSERT_STREQ ( sPath3.cstr(), "/" );

	CSphString sSrc4 ( "/home/pq2" );
	CSphString sPath4 = GetPathOnly ( sSrc4 );
	ASSERT_STREQ ( sPath4.cstr(), "/home/" );

	CSphString sSrc5 ( "/home/build/" );
	CSphString sPath5 = GetPathOnly ( sSrc5 );
	ASSERT_STREQ ( sPath5.cstr(), "/home/build/" );

	CSphString sSrc6 ( "home/build/" );
	CSphString sPath6 = GetPathOnly ( sSrc6 );
	ASSERT_STREQ ( sPath6.cstr(), "home/build/" );

	CSphString sSrc11 ( "/home/pq2" );
	CSphString sFile11 = GetBaseName ( sSrc11 );
	ASSERT_STREQ ( sFile11.cstr(), "pq2" );

	CSphString sSrc12 ( "home/pq2" );
	CSphString sFile12 = GetBaseName ( sSrc12 );
	ASSERT_STREQ ( sFile12.cstr(), "pq2" );

	CSphString sSrc13 ( "pq2" );
	CSphString sFile13 = GetBaseName ( sSrc13 );
	ASSERT_STREQ ( sFile13.cstr(), "pq2" );

	CSphString sSrc14 ( "/pq2" );
	CSphString sFile14 = GetBaseName ( sSrc14 );
	ASSERT_STREQ ( sFile14.cstr(), "pq2" );
}

TEST ( functions, IsTriviallyCopyable )
{
	EXPECT_TRUE ( IS_TRIVIALLY_COPYABLE ( DWORD ) ) << "DWORD";
	EXPECT_TRUE ( IS_TRIVIALLY_COPYABLE ( DWORD[] ) ) << "DWORD[]";
	ASSERT_TRUE ( IS_TRIVIALLY_COPYABLE ( DWORD* ) ) << "DWORD*";
	ASSERT_FALSE ( IS_TRIVIALLY_COPYABLE ( CSphFixedVector<DWORD> ) ) << "CSphFixedVector<DWORD>";
	ASSERT_FALSE ( IS_TRIVIALLY_COPYABLE ( CSphString )) << "CSphString";
}

int iCountCtr = 0;
int iCountDtr = 0;

struct NonDefaultCtr_t
{
	int& m_iFoo; // this effectively disables default c-tr
	int m_iNum;
	NonDefaultCtr_t ( int & iFoo, int iNum=0 ) : m_iFoo ( iFoo ), m_iNum ( iNum )
	{
		++iCountCtr;
	}

	~NonDefaultCtr_t ()
	{
		++iCountDtr;
	}
};

TEST ( functions, RawVector )
{
	iCountCtr = iCountDtr = 0;
	RawVector_T<NonDefaultCtr_t> testv;
	ASSERT_EQ ( iCountCtr, 0 );
	ASSERT_EQ ( iCountDtr, 0 );

	EXPECT_FALSE ( IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE ( NonDefaultCtr_t )) << "NonDefaultCtr_t";

	// testv.Reserve (100); //< will not compile since Reserve() may reallocate which needs copy ctr

	testv.Reserve_static ( 1000 );
	ASSERT_EQ ( iCountCtr, 0 ) << "nothing constructed";
	ASSERT_EQ ( iCountDtr, 0 ) << "nothing destructed";

	int foo;
	for ( int i=0; i<100; ++i )
		testv.Emplace_back(foo,i);
	ASSERT_EQ ( iCountCtr, 100 );
	ASSERT_EQ ( iCountDtr, 0 ) << "nothing destructed";

	// testv.Resize(500); //< will not compile, since Resize() may imply Reserve

	// Shrink down to 10 elems. It implies that 90 elems will be destructed, and nothing new added
	testv.Shrink ( 10 );
	ASSERT_EQ ( iCountCtr, 100 );
	ASSERT_EQ ( iCountDtr, 90 );
	ASSERT_EQ ( testv.GetLength(), 10);
	for ( auto& elem : testv )
		ASSERT_EQ ( &elem.m_iFoo, &foo );

	// add 10 another elems. It implies that 10 new will be constructed, nothing deleted
	int bar;
	for ( int i = 0; i<10; ++i )
		testv.Emplace_back ( bar, i );
	ASSERT_EQ ( iCountCtr, 110 );
	ASSERT_EQ ( iCountDtr, 90 );

	// ensure that 1-st 10 elemst refers to foo, second 10 - to bar
	for ( int i = 0; i<10; ++i )
		ASSERT_EQ ( &testv[i].m_iFoo, &foo );
	for ( int i = 10; i<20; ++i )
		ASSERT_EQ ( &testv[i].m_iFoo, &bar );
}

int iCountMoving = 0;

struct TrivialStructure_t
{
	int m_iNum = -1;

	TrivialStructure_t ( int iNum = -1 ) : m_iNum ( iNum )
	{
		++iCountCtr;
	}

	TrivialStructure_t ( const TrivialStructure_t& ) = default;
	TrivialStructure_t ( TrivialStructure_t && rhs ) noexcept { ++iCountMoving; Swap ( rhs ); }
	TrivialStructure_t & operator= ( TrivialStructure_t rhs ) noexcept { Swap(rhs); return *this;  }

	void Swap ( TrivialStructure_t & rhs ) noexcept
	{
		::Swap ( m_iNum, rhs.m_iNum );
	}

	~TrivialStructure_t ()
	{
		++iCountDtr;
	}
};

template<typename T>
using RawTrivialVector_T = sph::Vector_T<T, sph::DefaultCopy_T<T>, sph::DefaultRelimit, sph::RawStorage_T<T>>;

TEST ( functions, RawTrivialVector )
{
	EXPECT_FALSE ( IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE ( TrivialStructure_t )) << "TrivialStructure_t";

	iCountCtr = iCountDtr = 0;
	RawTrivialVector_T<TrivialStructure_t> testv;
	ASSERT_EQ ( iCountCtr, 0 );
	ASSERT_EQ ( iCountDtr, 0 );

	// reserve of empty - relocates nothing
	testv.Reserve ( 100 );
	ASSERT_EQ ( iCountCtr, 0 ) << "nothing constructed";
	ASSERT_EQ ( iCountDtr, 0 ) << "nothing destructed";

	// static reserve of empty - destroys/create nothing
	testv.Reserve_static ( 1000 );
	ASSERT_EQ ( iCountCtr, 0 ) << "nothing constructed";
	ASSERT_EQ ( iCountDtr, 0 ) << "nothing destructed";

	// explicitly construct 100 elems
	for ( int i=0; i<100; ++i )
		testv.Emplace_back(55);
	ASSERT_EQ ( iCountCtr, 100 );
	ASSERT_EQ ( iCountDtr, 0 ) << "nothing destructed";

	// resize to add 100 more elems and expect they just default c-tred
	testv.Resize(200);
	ASSERT_EQ ( iCountMoving, 0 );
	ASSERT_EQ ( iCountCtr, 200 );
	ASSERT_EQ ( iCountDtr, 0 );

	// check that 1-st 100 elems are c-tred, and another are default c-tred
	for ( int i = 0; i<100; ++i )
		ASSERT_EQ ( testv[i].m_iNum, 55 );
	for ( int i = 100; i<200; ++i )
		ASSERT_EQ ( testv[i].m_iNum, -1 );

	// Shrink down to 10 elems. It implies that 190 elems will be destructed, and nothing new added
	testv.Shrink ( 10 );
	ASSERT_EQ ( iCountCtr, 200 );
	ASSERT_EQ ( iCountDtr, 190 );
	ASSERT_EQ ( testv.GetLength(), 10);

	// add 10 another elems. It implies that 10 new will be constructed, nothing deleted
	for ( int i = 0; i<10; ++i )
		testv.Emplace_back ( i );
	ASSERT_EQ ( iCountCtr, 210 );
	ASSERT_EQ ( iCountDtr, 190 );
	ASSERT_EQ ( iCountMoving, 0 );

	/* RemoveFast.
	 * Swap inside implies triple move/destruct:
	 * 	T temp = std::move ( v1 ); // 1-st move
	 *	v1 = std::move ( v2 ); // destruct of v1, then 2-nd move
	 *  v2 = std::move ( temp ); // destruct of v2, then 3-rd move
	 *  } // destruct of temp
	 *
	 *  Then removing last elem implies 4-th destruct. Totally: 3 moving, 4 destructs
	 *  (usual vec in preallocated storage will have 3/3 here since removing doesn't destroy objects.)
	 */
	iCountCtr = iCountDtr = 0;
	testv.RemoveFast(1);
	ASSERT_EQ ( iCountCtr, 0 );
	ASSERT_EQ ( iCountDtr, 4 );
	ASSERT_EQ ( iCountMoving, 3 );

}

TEST ( functions, SharedPtr )
{
	SharedPtr_t<int> pFoo;
	ASSERT_FALSE ( bool(pFoo) );
	{
		SharedPtr_t<int> pBar { new int };
		*pBar = 10;
		pFoo = pBar;
		ASSERT_EQ ( *pFoo, 10 );
	}
	auto b = new int;
	*b = 20;
	ASSERT_EQ ( *pFoo, 10 );
	pFoo = b;
	ASSERT_EQ ( *pFoo, 20 );
}

TEST ( functions, SharedPtrCompare )
{
	SharedPtr_t<int> pFoo;
	ASSERT_EQ ( pFoo, nullptr );
	ASSERT_TRUE ( pFoo == nullptr );
	SharedPtr_t<int> pBar { new int };
	*pBar = 10;
	pFoo = pBar;
	auto pBaz = pFoo;
	ASSERT_EQ ( *pFoo, 10 );
	ASSERT_EQ ( pFoo, pBar );
	ASSERT_EQ ( pFoo, pBaz );
	ASSERT_EQ ( pBaz, pBar );
	ASSERT_TRUE ( pFoo == pBar );
	ASSERT_TRUE ( pFoo == pBaz );
	ASSERT_TRUE ( pBaz == pBar );
}

void pr (const VecTraits_T<DWORD>& dData, int a=-1, int b=-1)
{
	return;
	for ( auto i=0,len=dData.GetLength(); i<len; ++i)
		if (i==a)
			printf ("/%d, ", dData[i]);
		else if (i==b)
			printf ( "%d\\, ", dData[i] );
		else
			printf ( "%d, ", dData[i] );
	printf ( "(%d/%d)\n", b-a+1, dData.GetLength ());
}

int iCompared;

int make_partition (DWORD iPivot, int iNeedElems, VecTraits_T<DWORD> dData )
{
//	printf ( "iPivot=%d, need %d, has %d\n", iPivot, iNeedElems, dData.GetLength() );
	int iPass = 0;
	auto cmp = Lesser ( [] ( int a, int b ) {
		++iCompared;
		return a>b;
	} );

	--iNeedElems;
	int a=0;
	int b=dData.GetLength()-1;
	while (true)
	{
		int i=a;
		int j=b;
//		pr ( dData, i, j );
		while (i<=j)
		{
			while (cmp.IsLess (dData[i],iPivot))
				++i;
			while (cmp.IsLess ( iPivot, dData[j]))
				--j;
			if ( i<=j ) {
				::Swap(dData[i],dData[j]);
//				pr ( dData, i, j );
				++i;
				--j;
			}
		}

//		printf ( "i=%d, j=%d, Di=%d, Dj=%d\n", i, j, dData[i], dData[j] );

		if ( iNeedElems == j )
			break;

		if ( iNeedElems < j)
			b = j;  // too many elems aquired; continue with left part
		else
			a = i;  // too less elems aquired; continue with right part

//		iPivot = dData[(a+b)/2];
		iPivot = dData[( a*3+b ) / 4]; // ( a*(COEF-1)+b)/COEF
//		printf ( "a=%d, b=%d, pivot=%d\n", a,b,iPivot );
		++iPass;
	}

//	printf ( "partitioning completed in %d passes, %d comparisions, new pivot %d\n", iPass, iCompared, iPivot );
	return iPivot;
}

int lazy_partition ( VecTraits_T<DWORD>& dData,int iPivot, int COEFF )
{
	auto iElems = dData.GetLength();
	int N = iElems/COEFF;

	if ( iPivot<0 )
	{
		auto iPivotIndex = N / COEFF+1;
		iPivot = dData[iPivotIndex];
//		printf ("1-st pass\n");
	}

	return make_partition (iPivot,N,dData);
}

bool CheckData ( VecTraits_T<DWORD> & dData, int COEFF )
{
	auto iElems = dData.GetLength ();
	auto N = iElems / COEFF;

	DWORD val = 0xFFFFFFFF;
	for ( auto i=0; i<N; ++i)
		val = Min(val,dData[i]);

	for ( auto i=N;i<N * ( COEFF-1 ); ++i)
		if (dData[i]>val)
		{
			printf ("%d-f elem %d misplaced (%d)", i, dData[i], val);
			return false;
		}

	return true;
}

TEST ( functions, partition_random )
{
	const auto N = 1000;

	const auto COEFF = 4;
	const auto PASSES = 10000;
	const auto LIMIT = 1000000000;

	CSphVector<DWORD> dValues;
	dValues.Reserve ( N*COEFF );

	for ( auto i=0; i<N; ++i)
		dValues.Add(sphRand()% LIMIT);
	dValues.Sort( Lesser ( [] ( int a, int b ) { return a>b; } ));

	for ( auto i=0; i<N*( COEFF-1); ++i)
		dValues.Add ( sphRand ()% LIMIT);

	pr(dValues,0,N-1);

	iCompared = 0;
	// let's begin
	int iPivot = -1;

	for ( auto i=0; i<PASSES; ++i)
	{
		for (auto j=N;j<N*(COEFF-1);++j)
			dValues[j] = sphRand()% LIMIT;
		iPivot = lazy_partition ( dValues, iPivot, COEFF );
		ASSERT_TRUE ( CheckData ( dValues, COEFF )) << "failed on " << i << " pass.";
	}
//	printf ( "After partitioning\n" );
	pr ( dValues, 0, N-1 );

	printf ("\n avg %f comparisions per pass of %d elems\n", float(iCompared)/float(PASSES), dValues.GetLength());
//	ASSERT_STREQ ( nullptr, "1.100000" );
}

TEST ( functions, partition_monoasc )
{
	const auto COEFF = 4;

	CSphVector<DWORD> dValues;

	for (auto i=0; i<100; ++i)
		dValues.Add(1);

	for ( auto i = 0; i<300; ++i )
		dValues.Add ( 2 );

	pr ( dValues, 0, 99 );
	lazy_partition ( dValues, -1, COEFF );
	pr ( dValues, 0, 99 );
	ASSERT_TRUE ( CheckData ( dValues, COEFF ));
}

TEST ( functions, partition_monodesc )
{
	const auto COEFF = 4;

	CSphVector<DWORD> dValues;

	for ( auto i = 0; i<100; ++i )
		dValues.Add ( 2 );

	for ( auto i = 0; i<300; ++i )
		dValues.Add ( 1 );

	pr ( dValues, 0, 99 );
	lazy_partition ( dValues, -1, COEFF );
	pr ( dValues, 0, 99 );
	ASSERT_TRUE ( CheckData ( dValues, COEFF ));
}

TEST ( functions, partition_ascending )
{
	const auto COEFF = 4;

	CSphVector<DWORD> dValues;

	for ( auto i = 0; i<400; ++i )
		dValues.Add ( i );

	pr ( dValues, 0, 99 );
	lazy_partition ( dValues, -1, COEFF );
	pr ( dValues, 0, 99 );
	ASSERT_TRUE ( CheckData ( dValues, COEFF ));
}


TEST ( functions, partition_descending )
{
	const auto COEFF = 4;

	CSphVector<DWORD> dValues;

	for ( auto i = 0; i<400; ++i )
		dValues.Add ( 1000-i );

	pr ( dValues, 0, 99 );
	lazy_partition ( dValues, -1, COEFF );
	pr ( dValues, 0, 99 );
	ASSERT_TRUE ( CheckData ( dValues, COEFF ));
}

static CSphString GetHist ( const Histogram_i * pHist )
{
	StringBuilder_c tOut;
	pHist->Dump ( tOut );
	const char * pFull = tOut.cstr();

	const char * sDel = strchr ( pFull, '\n' );
	int iLen = tOut.GetLength() - ( sDel - pFull ) + 1;

	CSphString sDump;
	sDump.SetBinary ( sDel+1, iLen );

	return sDump;
}

static const float g_dHistSrc[] = {0.0f,41.0f,50.0f,54.0f,60.0f,61.0f,63.0f,64.0f,65.0f,67.0f,68.0f,69.0f,71.0f,72.0f,73.0f,74.0f,75.0f,76.0f,77.0f,78.0f,79.0f,80.0f,81.0f,
	83.0f,84.0f,96.0f,107.0f,143.0f,147.0f,148.0f,149.0f,150.0f,151.0f,152.0f,153.0f,154.0f,155.0f,156.0f,157.0f,158.0f,159.0f,160.0f,162.0f,165.0f,
	166.0f,167.0f,168.0f,169.0f,170.0f,171.0f,175.0f,178.0f,180.0f,181.0f,182.0f,183.0f,184.0f,185.0f,186.0f,188.0f,189.0f,190.0f,192.0f,193.0f,
	195.0f,197.0f,198.0f};

struct HistCase_t
{
	int m_iLoop = 0;
	int m_iSize = 0;
	const char * m_sRef = nullptr;
};

static std::unique_ptr<Histogram_i> PopulateHist ( const HistCase_t & tCase, bool bFinalize = true )
{
	std::unique_ptr<Histogram_i> pHist = CreateHistogram ( "dyn", SPH_ATTR_FLOAT, tCase.m_iSize );

	for ( int i=0; i<tCase.m_iLoop; i++ )
	{
		for ( float fVal : g_dHistSrc )
		{
			SphAttr_t tVal = sphF2DW ( fVal );
			pHist->Insert ( tVal );
		}
	}

	if ( bFinalize )
		pHist->Finalize();

	return pHist;
}

TEST ( functions, histogram )
{
	HistCase_t dCases[] = {
		{1, 35, R"(values:35
0.000,1;41.000,1;50.000,1;54.000,1;60.500,2;64.000,3;68.000,3;72.500,4;75.500,2;77.500,2;80.000,3;83.500,2;96.000,1;107.000,1;143.000,1;147.500,2;149.500,2;151.500,2;153.500,2;155.500,2;157.500,2;159.500,2;162.000,1;165.500,2;167.500,2;170.000,3;175.000,1;178.000,1;180.500,2;182.500,2;185.000,3;189.000,3;192.500,2;195.000,1;197.500,2)"},
		{2, 35, R"(values:35
0.000,2;41.000,2;50.000,2;54.000,2;60.500,4;64.000,6;68.000,6;72.286,7;75.200,5;77.500,4;80.000,6;83.500,4;96.000,2;107.000,2;143.000,2;147.500,4;149.500,4;151.500,4;153.500,4;155.500,4;157.500,4;159.500,4;162.000,2;165.500,4;167.500,4;170.000,6;175.000,2;178.000,2;180.500,4;182.500,4;185.000,6;189.000,6;192.500,4;195.000,2;197.500,4)"},
		{1, 65, R"(values:65
0.000,1;41.000,1;50.000,1;54.000,1;60.500,2;63.500,2;65.000,1;67.000,1;68.000,1;69.000,1;71.000,1;72.000,1;73.000,1;74.000,1;75.000,1;76.000,1;77.000,1;78.000,1;79.000,1;80.000,1;81.000,1;83.000,1;84.000,1;96.000,1;107.000,1;143.000,1;147.000,1;148.000,1;149.000,1;150.000,1;151.000,1;152.000,1;153.000,1;154.000,1;155.000,1;156.000,1;157.000,1;158.000,1;159.000,1;160.000,1;162.000,1;165.000,1;166.000,1;167.000,1;168.000,1;169.000,1;170.000,1;171.000,1;175.000,1;178.000,1;180.000,1;181.000,1;182.000,1;183.000,1;184.000,1;185.000,1;186.000,1;188.000,1;189.000,1;190.000,1;192.000,1;193.000,1;195.000,1;197.000,1;198.000,1)"},
		{1, 70, R"(values:67
0.000,1;41.000,1;50.000,1;54.000,1;60.000,1;61.000,1;63.000,1;64.000,1;65.000,1;67.000,1;68.000,1;69.000,1;71.000,1;72.000,1;73.000,1;74.000,1;75.000,1;76.000,1;77.000,1;78.000,1;79.000,1;80.000,1;81.000,1;83.000,1;84.000,1;96.000,1;107.000,1;143.000,1;147.000,1;148.000,1;149.000,1;150.000,1;151.000,1;152.000,1;153.000,1;154.000,1;155.000,1;156.000,1;157.000,1;158.000,1;159.000,1;160.000,1;162.000,1;165.000,1;166.000,1;167.000,1;168.000,1;169.000,1;170.000,1;171.000,1;175.000,1;178.000,1;180.000,1;181.000,1;182.000,1;183.000,1;184.000,1;185.000,1;186.000,1;188.000,1;189.000,1;190.000,1;192.000,1;193.000,1;195.000,1;197.000,1;198.000,1)"},
		{20, 15, R"(values:15
0.000,20;41.000,20;52.000,40;62.600,100;70.587,138;79.115,182;96.000,20;107.000,20;143.000,20;150.126,143;157.656,157;168.000,140;176.585,41;183.057,140;192.780,159)"}
	};

	for ( const HistCase_t & tCase : dCases )
	{
		std::unique_ptr<Histogram_i> pHist = PopulateHist ( tCase );
		ASSERT_STREQ( GetHist ( pHist.get() ).cstr(), tCase.m_sRef );
	}

	// estimate of merged values
	{
		HistCase_t tCase;
		tCase.m_iLoop = 1;
		tCase.m_iSize = 10;
		std::unique_ptr<Histogram_i> pHist = PopulateHist ( tCase, false );
		for ( int i=0; i<20; i++)
			pHist->Insert ( sphF2DW ( 10.0f ) );

		pHist->Finalize();

		CSphFilterSettings tFilter;
		tFilter.m_eType = SPH_FILTER_FLOATRANGE;
		tFilter.m_fMinValue = 0.0f;
		tFilter.m_fMaxValue = 10.0f;

		HistogramRset_t tRes;
		pHist->EstimateRsetSize ( tFilter, tRes );
		ASSERT_EQ( tRes.m_iTotal, 20 );
	}
}

TEST ( functions, field_mask )
{
	FieldMask_t foo;
	foo.Assign32(0x55555555);
	ASSERT_TRUE ( foo.Test ( 6 ) );
	ASSERT_TRUE ( foo.Test ( 30 ) );
	foo.DeleteBit (15);
	ASSERT_EQ (foo.GetMask32(),0x2AAAD555);

	// test that removing a high bit works
	foo.Set (250);
	foo.DeleteBit(249);
	ASSERT_TRUE ( foo.Test (249));

	// test that removing DWORD-edge bit carries edge correct
	foo.Set(224);
	foo.DeleteBit(223);
	ASSERT_TRUE ( foo.Test ( 223 ) );

}

class foo
{
public:
	void bar() {};
	static void bar_static() {}
};

TEST ( functions, static_trait )
{
	ASSERT_TRUE ( std::is_member_function_pointer<decltype ( &foo::bar )>::value );
	ASSERT_FALSE ( std::is_member_function_pointer<decltype ( &foo::bar_static )>::value );
}

template<typename T>
class RefCountedTestVec_T final : public ISphRefcountedMT, public LazyVector_T<T>
{
protected:
	~RefCountedTestVec_T () final { }

public:
	RefCountedTestVec_T () = default;
};

template <typename T>
using DataVecRefPtr_t = CSphRefcountedPtr<RefCountedTestVec_T<T> >;

template <typename T>
using ConstDataVecRefPtr_t = CSphRefcountedPtr<const RefCountedTestVec_T<T> >;

struct DataVecMutable_c : ISphNoncopyable
{
public:
	ConstDataVecRefPtr_t<int> & m_tOwner;

	DataVecRefPtr_t<int> m_tNewData;
	// shortcuts
	RefCountedTestVec_T<int> & m_dData;

	explicit DataVecMutable_c ( ConstDataVecRefPtr_t<int>& tData )
			: m_tOwner { tData }
			, m_tNewData { new RefCountedTestVec_T<int> () }
			, m_dData { *m_tNewData }
	{
		m_dData.Reserve ( tData->GetLength() );
		for ( const auto & i : *tData )
			m_dData.Add(i);
	}

	~DataVecMutable_c ()
	{
		m_tOwner = m_tNewData.Leak();
	}
};

TEST ( functions, mutate_via_ref )
{
	// original immutable data
	DataVecRefPtr_t<int> origData { new RefCountedTestVec_T<int> };
	origData->Add ( 1 );
	origData->Add ( 2 );
	origData->Add ( 3 );
	ASSERT_EQ ( origData->GetRefcount (), 1 );

	// make const snapshot of orig data
	ConstDataVecRefPtr_t<int> refData;
	refData = origData.Leak();

	ASSERT_EQ ( refData->GetLength (), 3 );
	ASSERT_EQ ( ( *refData )[0], 1 );
	ASSERT_EQ ( ( *refData )[1], 2 );
	ASSERT_EQ ( ( *refData )[2], 3 );
	ASSERT_EQ ( refData->GetRefcount(), 1 );

	ASSERT_EQ ( refData->GetRefcount (), 1 );
	auto prevRefData = refData;
	ASSERT_EQ ( refData->GetRefcount (), 2 );

	// make mutable snapshot
	{
		DataVecMutable_c foo { refData };
		ASSERT_EQ ( refData->GetRefcount (), 2 );
		ASSERT_EQ ( foo.m_tNewData->GetRefcount (), 1 );
		ASSERT_EQ ( foo.m_tNewData->GetLength (), 3 );
		ASSERT_EQ ( foo.m_dData.GetLength (), 3 );
		ASSERT_EQ ( &foo.m_dData, foo.m_tNewData );

		// mutate mutable data
		foo.m_dData.Resize(4);
		foo.m_dData[0] = 10;
		foo.m_dData[3] = 42;

		// check that orig (ref) is not changed
		ASSERT_EQ ( refData->GetLength (), 3 );
		ASSERT_EQ ( ( *refData )[0], 1 );
		ASSERT_EQ ( ( *refData )[1], 2 );
		ASSERT_EQ ( ( *refData )[2], 3 );

		// check that mutation is changed
		ASSERT_EQ ( foo.m_tNewData->GetLength (), 4 );
		ASSERT_EQ ( foo.m_dData.GetLength (), 4 );
		ASSERT_EQ ( foo.m_dData[0], 10 );
		ASSERT_EQ ( foo.m_dData[1], 2 );
		ASSERT_EQ ( foo.m_dData[2], 3 );
		ASSERT_EQ ( foo.m_dData[3], 42 );
	}

	// check that prev data is still unchanged (i.e. 'another reader')
	ASSERT_EQ ( prevRefData->GetLength (), 3 );
	ASSERT_EQ ( ( *prevRefData )[0], 1 );
	ASSERT_EQ ( ( *prevRefData )[1], 2 );
	ASSERT_EQ ( ( *prevRefData )[2], 3 );
	ASSERT_EQ ( prevRefData->GetRefcount (), 1 );

	// check that ref data is now pointee to new values provided by mutator
	ASSERT_EQ ( refData->GetLength (), 4 );
	ASSERT_EQ ( ( *refData )[0], 10 );
	ASSERT_EQ ( ( *refData )[1], 2 );
	ASSERT_EQ ( ( *refData )[2], 3 );
	ASSERT_EQ ( ( *refData )[3], 42 );
	ASSERT_EQ ( refData->GetRefcount (), 1 );

}
