//
// Copyright (c) 2019-2025, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include <gtest/gtest.h>

#include "threadutils.h"
#include "coroutine.h"
#include "task_dispatcher.h"

#include <atomic>

void SetStderrLogger ();


// here intentionally added initialized lambda capture
// it will always show compiler warning on ancient compilers
TEST ( ThreadPool, movelambda )
{
	CSphString test = "hello";
	CSphString to;
	const char* sOrigHello = test.cstr();
	auto x = [&, line = std::move ( test )] () mutable { to = std::move(line); };
	ASSERT_STREQ ( test.cstr (), NULL ) << "Line is already captured";
	ASSERT_STREQ ( to.cstr (), NULL ) << "line is not yet finally moved";
	x ();
	ASSERT_STREQ ( to.cstr (), "hello" ) << "lambda moved captured line to here";
	ASSERT_EQ ( sOrigHello, to.cstr() ) << "moved value is exactly one from the start";
}

TEST ( ThreadPool, Counter100 )
{
//	SetStderrLogger ();
	auto pPool = Threads::MakeThreadPool ( 4, "tp" );
	auto & tPool = *pPool;
	std::atomic<int> v {0};
	for ( int i=0; i<100; ++i)
		tPool.Schedule ([&] { ++v; }, false);
	tPool.StopAll ();
	ASSERT_EQ ( v, 100 );
}

void Counter100c()
{
	using namespace Threads;
	std::atomic<int> v {0};
	CallCoroutine ( [&] {
	  auto dWaiter = DefferedContinuator();
		for ( int i = 0; i < 100; ++i )
			Coro::Co ( [&] {
				v.fetch_add(1,std::memory_order_relaxed);
			},
					dWaiter );
		WaitForDeffered ( std::move(dWaiter) );
		ASSERT_EQ ( v, 100 );
	} );
}

TEST ( ThreadPool, Counter100c )
{
	for (auto i=0; i<100; ++i)
		Counter100c();
}

const char* SH()
{
	auto pSched = Threads::Coro::CurrentScheduler();
	if (!pSched)
		return "(null)";
	return pSched->Name();
}

void Sleeper(int iMsec, const char* szName)
{
	Threads::Coro::SleepMsec ( iMsec );
	std::cout << szName << " run " << iMsec << "\n";
}

// this test will NOT pass in single-thread (see gtests_globalstate.cpp, if iThreads<2).
// that is because it uses ture sphSleepMsec, which effectively pauses single thread.
TEST ( ThreadPool, WaitForN )
{
	using namespace Threads;
	Threads::CallCoroutine ( [&] {
		int N = 2;
		std::cout << "test started waiting " << N << "th\n";
		auto iIdx = WaitForN ( N, {
			[] {
				Sleeper (1500, "first");
			},
			[] {
				Sleeper ( 500, "second" );
			},
			[] {
				Sleeper ( 2000, "third" );
			}}
		);
		ASSERT_EQ ( iIdx, 0 );
		std::cout << "test finished, idx=" << iIdx;
	});
}

TEST ( ThreadPool, strandr )
{
	g_eLogLevel = SPH_LOG_VERBOSE_DEBUG;
	static const int NUMS = 100;
	CSphVector<int> dRes;
	dRes.Reserve ( NUMS );
	//	SetStderrLogger ();
	auto pSched = Threads::MakeThreadPool ( 1, "tp" );
	auto pRandr = Threads::MakeAloneScheduler ( nullptr );

	((Threads::SchedulerWithBackend_i&) *pRandr).SetBackend (pSched);

	for ( int i = 0; i<NUMS; ++i )
	{
		pRandr->Schedule ( [i,&dRes]
		{
			dRes.Add(i);
			sphSleepMsec ( sphRand () & 15);
		}, false );
		sphSleepMsec ( 10 );
	}

	sphSleepMsec (10);
	pSched->StopAll ();

	ASSERT_EQ ( dRes.GetLength(), NUMS );
	ARRAY_CONSTFOREACH( i, dRes )
	{
		ASSERT_EQ ( i, dRes[i] );
	}
}

// check if strandr is really produces sequental work
TEST ( ThreadPool, strandr2 )
{
	using namespace Threads;
	g_eLogLevel = SPH_LOG_VERBOSE_DEBUG;

	static const int NUMS = 100;
	CSphVector<int> dRes;
	dRes.Reserve ( NUMS );

	Threads::CallCoroutine ( [&] {
		auto dWaiter = DefferedRestarter ();
		RoledSchedulerSharedPtr_t pRandr = Threads::MakeAloneScheduler ( Coro::CurrentScheduler () );
		Threads::ScopedScheduler_c customtp { pRandr };
		for ( int i = 0; i<NUMS; ++i )
		{
			// commenting out line below will cause test to fail.
			Coro::Co ( [&,i] {
				sphSleepMsec ( sphRand () % NUMS );
				dRes.Add ( i );
			}, dWaiter );
		}

		WaitForDeffered ( std::move ( dWaiter ) );
	});

	ARRAY_CONSTFOREACH( i, dRes )
	{
		ASSERT_EQ ( i, dRes[i] );
	}
	ASSERT_EQ ( dRes.GetLength (), NUMS );
}

// checks that strandr is re-enterable. I.e. that Coro::Reschedule is NOT cause stack overflow.
// DISABLED because it is manual, otherwise produce many noise
/*TEST ( ThreadPool, DISABLED_strandr_reschedule )
{
	using namespace Threads;
	g_eLogLevel = SPH_LOG_VERBOSE_DEBUG;

	static const int NUMS = 700;

	Threads::CallCoroutine ( [&] {
		RoledSchedulerSharedPtr_t pRandr = Threads::MakeAloneScheduler ( Coro::CurrentScheduler () );
		std::cout << "enter scoped\n";
		{
			Threads::ScopedScheduler_c customtp { pRandr };
			std::cout << "scope entered\n";
			for ( int j = 0; j<NUMS; ++j )
			{
				Coro::Reschedule ();
				std::cout << "rescheduled " << j << "\n";
			}
			std::cout << "done\n";
		}
		std::cout << "strandr escaped\n";
	});
}*/

// from there it is prepatation to the next test...
struct essence_t
{
	int iOwner = -1;
	int iConcurrency = 0;
	int iRefs = 0;
};

struct tstthread_t
{
	Threads::RoledSchedulerSharedPtr_t strandr;
	std::atomic<int> iWorks {0};
};

static const int NWORKERS = 5;
static const int NUMS = 10;

const char* names[NWORKERS] = {"w1","w2","w3","w4","w5"};

void print_owners ( tstthread_t * pthreads )
{
	std::cout << "[";
	for ( int i = 0; i<NWORKERS; ++i )
		std::cout << pthreads[i].iWorks << ":" << ( pthreads[i].strandr ? "valid" : "nullptr" ) << ", ";
	std::cout << "]";
}

int getworker ( int N, tstthread_t* pthreads )
{
	using namespace Threads;
	int iIdx=0;
	int iWorks=1000000;
	std::cout << SH () << " " << N << ": getworker: "; print_owners(pthreads);
	for ( int i = 0; i<NWORKERS; ++i )
	{
		if ( iWorks>pthreads[i].iWorks )
		{
			iWorks = pthreads[i].iWorks;
			iIdx = i;
		}
	}
	if ( !pthreads[iIdx].strandr )
	{
		pthreads[iIdx].strandr = MakeAloneScheduler ( Coro::CurrentScheduler (), names[iIdx] );
		std::cout << " new ";
	}
	std::cout << "give " << iIdx << ":" << iWorks << "\n";
	return iIdx;
}

void bind_resource ( int N, essence_t* presource, int iResource, tstthread_t * pthreads, int iWorker )
{
	auto& resource = presource[iResource];
	if ( resource.iOwner!=iWorker )
	{
		if ( resource.iOwner!=-1 )
		{
			StringBuilder_c cout;
			std::cout << SH () << " " << N << ": resource owner of " << iResource << " is fiber_" << resource.iOwner << ", wait...\n";
			while ( resource.iOwner!=iWorker && resource.iOwner!=-1 )
				Threads::Coro::Reschedule ();
			std::cout << SH () << " " << N << ": wait of " << iResource << " done, owner is fiber_" << resource.iOwner << ".\n";
		}

		resource.iOwner = iWorker;
	}
	++resource.iRefs;
	if ( resource.iRefs==1 )
		++pthreads[iWorker].iWorks;
	std::cout << SH () << " " << N << ": resource owner of " << iResource << " is fiber_" << iWorker << " (has " << pthreads[iWorker].iWorks << ")\n";
}

void print_refs ( essence_t* presource)
{
	std::cout<<"[";
	for (int i=0; i<NUMS; ++i)
	{
		if ( presource[i].iOwner==-1)
			std::cout << "NONE, ";
		else
			std::cout << presource[i].iOwner << ":" << presource[i].iRefs << ", ";
	}

	std::cout << "]";
}


int release_resource ( int N, essence_t* presource, int i )
{
	auto& resource = presource[i];
	std::cout << SH () << " " << N << ":	release_resource " << i << ":";
	print_refs(presource);
	--resource.iRefs;
	int ires = 0;
	if (!resource.iRefs)
	{
		resource.iOwner = -1;
		ires = 1;
	}
	std::cout << " -> ";
	print_refs ( presource );
	std::cout<<  "\n";
	return ires;
}

// here is test of RT-index chunks/segs dispatcher.
/*
 * We have N resources (chunks/segs) and M threads.
 * We have X tasks which are starte in parallel.
 *
 * 1. For couple of resources in task dispatcher provides one executor from M threads.
 * 2. If one of resources is busy, it provides backoff strategy (yield and resume when state changes) without bunch of
 * empty CPU loops.
 * 3. When executing tasks in parallel we checks that only one executor process resource in time.
 * 4. Finally we check: all tasks processed; all resources released; all threads abandoned.
 *
 * Executing test in single thread will obviously pass it (as only one task executed in time), but in this case we
 * check that the're ho deadlocks - i.e. when we wait for releasing resources, but worker which occupy them, in turn,
 * waits when we release it ourselves (and since there is single thread it will be deadlock).
 *
 * Test is generaly disabled since it is quire long, and also quite noisy. Undisable and run manually, if necessary!
 */
/*TEST ( ThreadPool, DISABLED_strandr3 )
{
	using namespace Threads;

	essence_t resources[NUMS];
	thread_t workers[NWORKERS];

	Threads::CallCoroutine ( [&] {
		auto dWaiter = DefferedRestarter ();
		RoledSchedulerSharedPtr_t pRandr = Threads::MakeAloneScheduler ( Coro::CurrentScheduler (),"SH" );
		std::cout << "started...\n";
		int64_t iMaxTries = 0;

		for ( int i=0; i<1000; ++i)
		{
			Coro::Co ( [&,i] {

				// select couple of resources (random)
				auto a = ( sphRand () % NUMS );
				auto b = a;
				while (b==a)
					b = ( sphRand () % NUMS );
				int iOwner;

				// that is dispatcher. It selects ready worker, or creates new one.
				// of both resources busy, it waits when their state changes and check again in loop.
				{
					// line below organize execution in one 'scheduler' thread.
					Threads::ScopedScheduler_c customtp { pRandr };
					std::cout << SH () << " " << i << ": try to work with " << a << "(" << resources[a].iOwner << ") and " << b << "("
							  << resources[b].iOwner << ")\n";

					bool bHasWorker = false;
					int64_t iLoops = 0;

					// if both resources are free - select or create worker.
					// if one free - bind it to the worker of the second one.
					// of both busy - backoff. Wait until any worker of both is finished and restart selector loop fro scratch.
					while (!bHasWorker)
					{
						auto workera = resources[a].iOwner;
						auto workerb = resources[b].iOwner;
						bHasWorker = true;
						if (workera==-1 || workerb==-1) // one or both are not owned at all - may steal it now!
						{
							if ( workera==-1 && workerb==-1 ) // no both owners, make new one
								iOwner = getworker ( i, workers );
							else // one is not owned - bind to 2-nd.
								iOwner = ( workera==-1 ) ? workerb : workera;
						} else {
							if ( workera==workerb )
								iOwner = workera;
							else {
								bHasWorker = false;
								++iLoops;

								// backoff: schedule 2 tasks, each in the worker of each resource. When any fired,
								// restart the loop.
								// Other way is just Coro::Reschedule(), but that will fire CPU core of dispatcher in wain.
								WaitForN ( 1, {
									[t=workers[workera].strandr] { ScopedScheduler_c _ { t };},
									[t=workers[workerb].strandr] { ScopedScheduler_c _ { t };}
								});
								std::cout << SH () << ": try " << iLoops << " (" << a << " " << b << ")\n";
								continue;
							}
						}
					}

					// here target thread (executor) is selected. We still works in single sheduler thread, so will bind
					// resources to the executor without any locks, since there is no concurrency in current context.
					iMaxTries = Max ( iMaxTries, iLoops );
					std::cout << SH () << " " <<i << ": will work with " << a << "(" << resources[a].iOwner << ") and " << b << "("
						<< resources[b].iOwner << ")";
					if ( iLoops!=0 )
						std::cout << " in " << iLoops << " tries\n";
					else
						std::cout << "\n";

					bind_resource ( i, resources, a, workers, iOwner );
					bind_resource ( i, resources, b, workers, iOwner );

					std::cout << SH () << " " << i <<": -> in fiber_" << iOwner << " (" << workers[iOwner].iWorks << " tasks)";
					print_refs(resources);
					print_owners( workers);
					std::cout<<"\n";
				}

				// here is wild (multi-threaded) context with concurrency.
				// line below might be uncommented for sigh of randomness
//				sphSleepMsec ( 50+sphRand()%100 );


				// from line below we settle in one of the workers. Here is the test of how dispatcher provides us
				// single resource which we will access without concurrency, because there is no intersection between
				// sets of resources among different executors.
				Threads::ScopedScheduler_c customtp { workers[iOwner].strandr };
				std::cout << SH () << " " << i <<":	" << " fiber_" << iOwner << ": " << a << " " << b << "\n";
				++resources[a].iConcurrency;
				++resources[b].iConcurrency;
				ASSERT_EQ ( 1, resources[a].iConcurrency );
				ASSERT_EQ ( 1, resources[b].iConcurrency );
				std::cout << SH () << " " << i << ":	" << "	in use: " << resources[a].iConcurrency << "/" << resources[b].iConcurrency << "\n";
				ASSERT_EQ ( 1, resources[a].iConcurrency );
				ASSERT_EQ ( 1, resources[b].iConcurrency );
				--resources[a].iConcurrency;
				--resources[b].iConcurrency;

				{ // releasing
//					Threads::ScopedScheduler_c customtp { pRandr };
					workers[iOwner].iWorks -= release_resource (i,resources,a) + release_resource(i,resources,b);
					std::cout << SH () << " " << i << ": fiber_" << iOwner << " released "
						<< a << "(" << resources[a].iRefs << ") and "
						<< b << "(" << resources[b].iRefs << "), rest " << workers[iOwner].iWorks << "\n";
				}

			}
			, dWaiter );
		}

		// wait all tasks to finish.
		WaitForDeffered ( std::move ( dWaiter ) );
		std::cout << "finished, "<<  iMaxTries << " max tries\n";

		// check that everything is correctly released.
		for ( auto & resource: resources )
		{
			ASSERT_EQ ( 0, resource.iConcurrency );
			ASSERT_EQ ( -1, resource.iOwner );
			ASSERT_EQ ( 0, resource.iRefs );
		}
		for ( auto & worker : workers )
			ASSERT_EQ ( 0, worker.iWorks );
	} );

}*/

TEST ( ThreadPool, CoroPromiceFutureConcept )
{
	using namespace Threads;
	CallCoroutine ( [&] {
	volatile int iData;
	auto fnCoro = MakeCoroExecutor ( [&iData]() {
		iData = 1;
		Coro::Yield_();
		iData = 2;
		Coro::Yield_();
		iData = 10;
		Coro::Yield_();
		iData = 16;
	} );

	auto fnCondition = [fnCoro = std::move ( fnCoro ), &iData] ( int& iData2 ) -> bool {
		bool bRes = !fnCoro();
		iData2 = iData;
		return bRes;
	};

	int iCheck;
	ASSERT_TRUE ( fnCondition ( iCheck ) );
	ASSERT_EQ ( iCheck, 1 );
	ASSERT_TRUE ( fnCondition ( iCheck ) );
	ASSERT_EQ ( iCheck, 2 );
	ASSERT_TRUE ( fnCondition ( iCheck ) );
	ASSERT_EQ ( iCheck, 10 );
	ASSERT_FALSE ( fnCondition ( iCheck ) );
	ASSERT_EQ ( iCheck, 16 );
	});
}

TEST ( Dispatcher, Trivial )
{
	auto pDispatcher = Dispatcher::MakeTrivial(6, 3);

	auto pWork1 = pDispatcher->MakeSource();
	int iJob = -1;
	ASSERT_TRUE ( pWork1->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 0 );

	// wasn't consumed, same value
	ASSERT_TRUE ( pWork1->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 0 );

	iJob = -1;
	ASSERT_TRUE ( pWork1->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 1 );

	auto pWork2 = pDispatcher->MakeSource();
	auto pWork3 = pDispatcher->MakeSource();

	// we can make source over initial concurrency, since dispatcher is trivial
	auto pWork4 = pDispatcher->MakeSource();

	iJob = -1;
	ASSERT_TRUE ( pWork2->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 2 );

	iJob = -1;
	ASSERT_TRUE ( pWork3->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 3 );

	iJob = -1;
	ASSERT_TRUE ( pWork4->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 4 );

	iJob = -1;
	ASSERT_TRUE ( pWork1->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 5 );

	// should work as iJob wasn't consumed
	ASSERT_TRUE ( pWork3->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 5 );

	iJob = -1;
	// all jobs are done
	ASSERT_FALSE ( pWork3->FetchTask ( iJob ) );
}

TEST ( Dispatcher, RoundRobin_batch_1 )
{
	auto pDispatcher = Dispatcher::MakeRoundRobin ( 8, 2 );

	auto pFIRST = pDispatcher->MakeSource();
	int iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 0 );

	// wasn't consumed, same value
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 0 );

	iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 2 );

	iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 4 );

	auto pSECOND = pDispatcher->MakeSource();
	auto pTHIRD = pDispatcher->MakeSource();

	iJob = -1;
	ASSERT_TRUE ( pSECOND->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 1 );

	iJob = -1;
	ASSERT_TRUE ( pSECOND->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 3 );

	iJob = -1;
	ASSERT_TRUE ( pSECOND->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 5 );

	// this one is empty, will fail
	iJob = -1;
	ASSERT_FALSE ( pTHIRD->FetchTask ( iJob ) );

	iJob = -1;
	ASSERT_TRUE ( pSECOND->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 7 );

	// this one is done
	iJob = -1;
	ASSERT_FALSE ( pSECOND->FetchTask ( iJob ) );

	iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 6 );

	// this one is also done
	iJob = -1;
	ASSERT_FALSE ( pFIRST->FetchTask ( iJob ) );
}

TEST ( Dispatcher, RoundRobin_batch_2 )
{
	auto pDispatcher = Dispatcher::MakeRoundRobin ( 8, 2, 3 );

	auto pFIRST = pDispatcher->MakeSource();
	int iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 0 );

	// wasn't consumed, same value
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 0 );

	iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 1 );

	// defer first source and create 2 more
	auto pSECOND = pDispatcher->MakeSource();
	auto pTHIRD = pDispatcher->MakeSource();

	iJob = -1;
	ASSERT_TRUE ( pSECOND->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 3 );

	iJob = -1;
	ASSERT_TRUE ( pSECOND->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 4 );

	// now query 3-rd, it is empty, will fail
	iJob = -1;
	ASSERT_FALSE ( pTHIRD->FetchTask ( iJob ) );

	// finish with 2-nd. It has only jobs 3-5 out of 8.
	iJob = -1;
	ASSERT_TRUE ( pSECOND->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 5 );

	// this one is done
	iJob = -1;
	ASSERT_FALSE ( pSECOND->FetchTask ( iJob ) );

	// finish 1-st reader. It has 3 more: 2, then 6 and 7

	iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 2 );

	iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 6 );

	iJob = -1;
	ASSERT_TRUE ( pFIRST->FetchTask ( iJob ) );
	ASSERT_EQ ( iJob, 7 );

	// this one is finally done also
	iJob = -1;
	ASSERT_FALSE ( pFIRST->FetchTask ( iJob ) );
}

struct CheckDispatch { const char* szTemplate; int iConc; int iBatch; };

static CheckDispatch dChecks[] = {
	{ nullptr, 0, 0 },
	{ "", 0, 0 },
	{ " ", 0, 0 },
	{ "/", 0, 0 },
	{ "0", 0, 0 },
	{ "*", 0, 0 },
	{ "0/", 0, 0 },
	{ "*/", 0, 0 },
	{ "/0", 0, 0 },
	{ "/*", 0, 0 },
	{ "*/*", 0, 0 },
	{ "*/0", 0, 0 },
	{ "0/0", 0, 0 },
	{ "0/*", 0, 0 },
	{ "13", 13, 0 },
	{ "13/", 13, 0 },
	{ "13/*", 13, 0 },
	{ "13/0", 13, 0 },
	{ "/3", 0, 3 },
	{ "0/3", 0, 3 },
	{ "*/3", 0, 3 },
	{ "13/3", 13, 3 },
	{ " 13/3", 13, 3 },
	{ "13 /3", 13, 3 },
	{ "13/ 3", 13, 3 },
	{ "13/3 ", 13, 3 },
	{ " 13 /3", 13, 3 },
	{ " 13/ 3", 13, 3 },
	{ " 13/3 ", 13, 3 },
	{ " 13 / 3", 13, 3 },
	{ " 13 /3 ", 13, 3 },
	{ " 13 / 3 ", 13, 3 },
};

void Check ( const char* szTemplate, int iConc, int iBatch )
{
	auto tVal = Dispatcher::ParseTemplate ( szTemplate );
	ASSERT_EQ ( tVal.concurrency, iConc ) << szTemplate;
	ASSERT_EQ ( tVal.batch, iBatch ) << szTemplate;
}

TEST ( Dispatcher, ParseOne )
{
	for ( const auto& tCheck : dChecks )
		Check ( tCheck.szTemplate, tCheck.iConc, tCheck.iBatch );
}

void CheckTwo ( const char* szTemplate, int iConcx, int iBatchx, int iConcy, int iBatchy )
{
	auto tVal = Dispatcher::ParseTemplates ( szTemplate );
	ASSERT_EQ ( tVal.first.concurrency, iConcx ) << szTemplate;
	ASSERT_EQ ( tVal.first.batch, iBatchx ) << szTemplate;
	ASSERT_EQ ( tVal.second.concurrency, iConcy ) << szTemplate;
	ASSERT_EQ ( tVal.second.batch, iBatchy ) << szTemplate;
}

TEST ( Dispatcher, ParseCouple )
{
	CheckTwo ( nullptr, 0, 0, 0, 0 );
	CheckTwo ( "", 0, 0, 0, 0 );
	for ( const auto& x : dChecks )
		for ( const auto& y : dChecks )
		{
			StringBuilder_c sTmp;
			sTmp << x.szTemplate << '+' << y.szTemplate;
			CheckTwo(sTmp.cstr(),x.iConc,x.iBatch,y.iConc,y.iBatch);
		}
}
