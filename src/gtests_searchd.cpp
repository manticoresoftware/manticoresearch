//
// Created by alexey on 28.09.17.
//

#include <gtest/gtest.h>


// simplest way to test searchd internals - include the source, supress main() function there.
#define SUPRESS_SEARCHD_MAIN 1
#include "searchd.cpp"

// Note that global definitions below may conflict with names from searchd.cpp included above.
// rename your variables here then.
static GuardedHash_c * g_pLocals = nullptr;
static CSphString g_sIndex;
static CSphFixedVector<CSphString> g_dLocals ( 0 );

static volatile bool g_bHas = false;
static CSphString g_sNewPath;
static DWORD g_uIter = 0;
static const auto ITERATIONS = 10000;

static ServedIndexRefPtr_c GetTestLocal ( const CSphString &sName )
{
	auto pObj = (ServedIndex_c *) g_pLocals->Get ( sName );
	ServedIndexRefPtr_c pRes ( pObj );
	return pRes;
}

void ThdSearch ( void * )
{
	for ( auto i = 0; i<ITERATIONS; ++i )
	{
		ARRAY_FOREACH ( i, g_dLocals )
		{
			auto pDesc = GetTestLocal ( g_dLocals[i] );
			bool bGot = pDesc;

			// check that it exists
			if ( !bGot )
				continue;

			g_bHas = g_bHas ^ bGot;
		}
	}
}

void ThdRotate ( void * )
{
	for ( auto i = 0; i<ITERATIONS; ++i )
	{
		g_sNewPath.SetSprintf ( "/tmp/very-long-path/goes/here/new_%u", g_uIter );
		++g_uIter;

		auto pServed = GetTestLocal ( g_sIndex );
		ServedDescWPtr_c dWriteLock ( pServed );

		dWriteLock->m_sNewPath = "";
		dWriteLock->m_sIndexPath = g_sNewPath;
		for ( int i = 0; i<100; i++ )
			g_bHas = g_bHas ^ (!!( i + g_uIter ));
	}
}

TEST ( searchd_stuff, crash_on_exists )
{
	g_pLocals = new GuardedHash_c ();
	g_sIndex = "reader-test-17";
	g_dLocals.Reset ( 30 );
	ARRAY_FOREACH ( i, g_dLocals )
	{
		g_dLocals[i].SetSprintf ( "reader-test-%d", i + 1 );

		ServedDesc_t tDesc;
		tDesc.m_sIndexPath.SetSprintf ( "/tmp/very-long-path/goes/here/%s", g_dLocals[i].cstr () );
		g_pLocals->AddUniq ( new ServedIndex_c (tDesc), g_dLocals[i] );
	}

	SphThread_t th1, th2, th3, th4, th5, thRot;
	sphThreadCreate ( &th1, ThdSearch, nullptr );
	sphThreadCreate ( &th2, ThdSearch, nullptr );
	sphThreadCreate ( &th3, ThdSearch, nullptr );
	sphThreadCreate ( &th4, ThdSearch, nullptr );
	sphThreadCreate ( &th5, ThdSearch, nullptr );

	sphThreadCreate ( &thRot, ThdRotate, nullptr );

	sphThreadJoin ( &th1 );
	sphThreadJoin ( &th2 );
	sphThreadJoin ( &th3 );
	sphThreadJoin ( &th4 );
	sphThreadJoin ( &th5 );
	sphThreadJoin ( &thRot );
}

#if POLLING_EPOLL
// different aspects of epoll internals
TEST ( searchd_stuff, epoll_behaviour )
{
	// create pair of socket and epoll,
	// add 'server' socket to epoll instance,
	// prepare send and receive buffers.
	int dSockets[2] = { -1, -1 };
	auto iRes = socketpair ( AF_LOCAL, SOCK_STREAM, 0, dSockets );
	ASSERT_EQ ( iRes, 0 );

	auto iServer = dSockets[0];
	auto iClient = dSockets[1];

	int iPoll = epoll_create (2);

	epoll_event tEv = { 0 };
	tEv.events = EPOLLIN; // EPOLLOUT : 0 );
	iRes = epoll_ctl ( iPoll, EPOLL_CTL_ADD, iServer, &tEv );
	ASSERT_NE (iRes, -1);

	const char* pSrc = "hello";
	char receiver[10];

	// send 'hello' into client socket
	send ( iClient, pSrc, 6, MSG_NOSIGNAL | MSG_DONTWAIT );

	tEv = {0};
	iRes = epoll_wait ( iPoll, &tEv, 1, 1000);
	ASSERT_EQ (iRes, 1) << "Server socket received " << iRes << " events";

	// receive only 3 bytes (so 3 more left in buf).
	recv ( iServer, receiver, 3, MSG_WAITALL );
	receiver[3] = '\0';
	ASSERT_STREQ ( receiver, "hel");

	// now delete server from subscription.
	tEv = {0};
	iRes = epoll_ctl ( iPoll, EPOLL_CTL_DEL, iServer, &tEv );
	ASSERT_NE ( iRes, -1 );

	// assert that epoll rerturned nothing (3 bytes still there)
	iRes = epoll_wait ( iPoll, &tEv, 1, 500 );
	ASSERT_EQ ( iRes, 0 );

	// substribe server for output
	tEv = {0};
	tEv.events = EPOLLOUT;
	iRes = epoll_ctl ( iPoll, EPOLL_CTL_ADD, iServer, &tEv );

	// ensure it is ready for writing
	tEv = {0};
	iRes = epoll_wait ( iPoll, &tEv, 1, 1000 );
	ASSERT_EQ ( iRes, 1 );

	// send the same 'hello' from other end of the socket.
	send ( iServer, pSrc, 6, MSG_NOSIGNAL | MSG_DONTWAIT );

	// ensure it still ready for writing (6 bytes definitely not fill whole buffer)
	tEv = { 0 };
	iRes = epoll_wait ( iPoll, &tEv, 1, 1000 );
	ASSERT_EQ ( iRes, 1 );
	ASSERT_EQ ( tEv.events, EPOLLOUT );

	// change 'out' to 'in' subscription
	tEv.events = EPOLLIN | EPOLLOUT;
	iRes = epoll_ctl ( iPoll, EPOLL_CTL_MOD, iServer, &tEv );
	ASSERT_NE ( iRes, -1 );

	// ensure we can still read from the sock.
	tEv = { 0 };
	iRes = epoll_wait ( iPoll, &tEv, 1, 1000 );
	ASSERT_EQ ( iRes, 1 );
	ASSERT_EQ ( tEv.events, EPOLLIN | EPOLLOUT );

	// receive rest of originally sent buffer.
	recv ( iServer, receiver, 3, MSG_NOSIGNAL | MSG_WAITALL );
	ASSERT_STREQ ( receiver, "lo" );

	// the gray area: socket still subscribed; we close it
	::close ( iClient );

	// ensure it is still ready for reading - EOF is there!

	tEv = { 0 };
	iRes = epoll_wait ( iPoll, &tEv, 1, 1000 );
	ASSERT_EQ ( iRes, 1 );
	ASSERT_EQ ( tEv.events, EPOLLHUP + EPOLLERR + EPOLLIN + EPOLLOUT);
	// here we have EPOLLHUP + EPOLLER + EPOLLIN + EPOLLOUT

	// it will receive nothing - EOF is here.
	iRes = recv ( iServer, receiver, 10, MSG_NOSIGNAL );
	ASSERT_EQ ( iRes, -1 );

	// now again, it will return 1. Epollerr went out.
	iRes = epoll_wait ( iPoll, &tEv, 1, 1000 );
	ASSERT_EQ ( iRes, 1 );
	ASSERT_EQ ( tEv.events, EPOLLHUP + EPOLLIN + EPOLLOUT );

	// this send should be impossible since other socket is closed.
	iRes = send ( iServer, pSrc, 6, MSG_NOSIGNAL | MSG_DONTWAIT );
	ASSERT_EQ ( iRes, -1 );


	// now again, it will return 1. Epollerr went out.
	iRes = epoll_wait ( iPoll, &tEv, 1, 1000 );
	ASSERT_EQ ( iRes, 1 );
	ASSERT_EQ ( tEv.events, EPOLLHUP + EPOLLIN + EPOLLOUT );

	// now close out side also...
	::close ( iServer );
	tEv = { 0 };
	iRes = epoll_wait ( iPoll, &tEv, 1, 500 );
	ASSERT_EQ ( iRes, 0 );

	// empty epoll
	tEv = {0};
	iRes = epoll_ctl ( iPoll, EPOLL_CTL_DEL, iServer, &tEv );
	ASSERT_EQ ( iRes, -1 ) << "event auto-deleted";

	// ensure that _now_ we have nothing in epoll.
	tEv = { 0 };
	iRes = epoll_wait ( iPoll, &tEv, 1, 500 );
	ASSERT_EQ ( iRes, 0 );

	::close (iPoll);
}

#endif

TEST ( searchd_stuff, iovec_behaviour )
{
	SmartOutputBuffer_t tSrc;
	IOVec_c tIO;

	tSrc.SendDword (0xAAAAAAAA);
	tIO.BuildFrom (tSrc);
	tSrc.StartNewChunk ();
	tSrc.SendDword ( 0xBBBBBBBB);
	tIO.BuildFrom ( tSrc );
	ASSERT_EQ ( tIO.IOSize (), 2);
	tSrc.StartNewChunk();
	tIO.BuildFrom ( tSrc );
	ASSERT_EQ ( tIO.IOSize (), 2 );
	tIO.StepForward (2);
	ASSERT_EQ ( tIO.IOSize (), 2 );
	tIO.StepForward (2);
	ASSERT_EQ ( tIO.IOSize (), 1 );
	tIO.StepForward (4);
	ASSERT_EQ ( tIO.IOSize (), 0 );
}