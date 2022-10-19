//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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


// simplest way to test searchd internals - include the source, supress main() function there.
#define SUPRESS_SEARCHD_MAIN 1
#include "searchd.cpp"

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

TEST ( searchd_stuff, prepare_emulation )
{
	CSphQuery tQuery;
	tQuery.m_eMode = SPH_MATCH_ALL;
	PrepareQueryEmulation ( &tQuery );

	ASSERT_EQ ( tQuery.m_eRanker, SPH_RANK_PROXIMITY );
}

class CustomNetloop_c :  public ::testing::Test
{
protected:
	void SetUp () override
	{
		m_pPoll = std::make_unique<NetPooller_c> ( 1000 );
		int64_t tmNow = sphMicroTimer ();
		SetupEvent ( new CSphWakeupEvent, tmNow );
		SetupEvent ( new CSphWakeupEvent, tmNow );
		SetupEvent ( new CSphWakeupEvent, tmNow );
	}

	void TearDown () NO_THREAD_SAFETY_ANALYSIS override
	{
		m_pPoll->ProcessAll( [this] ( NetPollEvent_t * pWork ) NO_THREAD_SAFETY_ANALYSIS {
			m_pPoll->RemoveEvent ( pWork );
			SafeRelease ( pWork );
		} );
	}

	void SetupEvent ( ISphNetAction * pWork, int64_t tmNow ) NO_THREAD_SAFETY_ANALYSIS
	{
		m_pPoll->SetupEvent ( pWork );
	}

	CSphVector<ISphNetAction *> RemoveOutdated ( int iOutdate, int iOutdate2=-1 ) NO_THREAD_SAFETY_ANALYSIS
	{
		CSphVector<ISphNetAction *> dCleanup;
		int ev = -1;
		// remove outdated items on no signals
		m_pPoll->ProcessAll ( [&] ( NetPollEvent_t * pEvent ) NO_THREAD_SAFETY_ANALYSIS {
			++ev;
			if ( ev!=iOutdate && ev!=iOutdate2)
				return;

			m_pPoll->RemoveEvent ( pEvent );
			auto * pWork = (ISphNetAction *) pEvent;
			dCleanup.Add ( pWork );
		} );
		return dCleanup;
	}

	std::unique_ptr<NetPooller_c> m_pPoll;
};

TEST_F ( CustomNetloop_c, test_usual_remove_1st )
{
	int to_del = 0;

	auto dOutdated = RemoveOutdated ( to_del );
	ARRAY_FOREACH ( i, dOutdated ) SafeDelete ( dOutdated[i] );
}

TEST_F ( CustomNetloop_c, test_usual_remove_2nd )
{
	int to_del = 1;

	auto dOutdated = RemoveOutdated ( to_del );
	ARRAY_FOREACH ( i, dOutdated ) SafeDelete ( dOutdated[i] );
}

TEST_F ( CustomNetloop_c, test_usual_remove_3rd )
{
	int to_del = 2;

	auto dOutdated = RemoveOutdated ( to_del );
	ARRAY_FOREACH ( i, dOutdated ) SafeDelete ( dOutdated[i] );
}

TEST_F ( CustomNetloop_c, test_usual_remove_12 )
{
	auto dOutdated = RemoveOutdated ( 0,1 );
	ARRAY_FOREACH ( i, dOutdated ) SafeDelete ( dOutdated[i] );
}

TEST_F ( CustomNetloop_c, test_usual_remove_23 )
{
	auto dOutdated = RemoveOutdated ( 1, 2 );
	ARRAY_FOREACH ( i, dOutdated ) SafeDelete ( dOutdated[i] );
}
