//
// Created by alexey on 28.09.17.
//

#include <gtest/gtest.h>


// simplest way to test searchd internals - include the source, supress main() function there.
#define SUPRESS_SEARCHD_MAIN 1
#include "searchd.cpp"

// Note that global definitions below may conflict with names from searchd.cpp included above.
// rename your variables here then.
static IndexHash_c * g_pLocals = nullptr;
static CSphString g_sIndex;
static CSphFixedVector<CSphString> g_dLocals ( 0 );

static volatile bool g_bHas = false;
static CSphString g_sNewPath;
static DWORD g_uIter = 0;
static const auto ITERATIONS = 10000;

void ThdSearch ( void * )
{
	for ( auto i = 0; i<ITERATIONS; ++i )
	{
		ARRAY_FOREACH ( i, g_dLocals )
		{
			ServedDesc_t tDesc;
			bool bGot = g_pLocals->Exists ( g_dLocals[i], &tDesc );
			bool bEnabled = ( bGot && tDesc.m_bEnabled );

			// check that it exists
			if ( !bGot )
				continue;

			if ( bEnabled )
				g_bHas = g_bHas ^ bEnabled;
		}
	}
}

void ThdRotate ( void * )
{
	for ( auto i = 0; i<ITERATIONS; ++i )
	{
		g_sNewPath.SetSprintf ( "/tmp/very-long-path/goes/here/new_%u", g_uIter );
		g_uIter++;

		ServedIndex_c * pServed = g_pLocals->GetWlockedEntry ( g_sIndex );

		pServed->m_sNewPath = "";
		pServed->m_sIndexPath = g_sNewPath;
		for ( int i = 0; i<100; i++ )
			g_bHas = g_bHas ^ (!!( i + g_uIter ));

		pServed->Unlock ();
	}
}

TEST ( searchd_stuff, crash_on_exists )
{
	g_pLocals = new IndexHash_c ();
	g_sIndex = "reader-test-17";
	g_dLocals.Reset ( 30 );
	ARRAY_FOREACH ( i, g_dLocals )
	{
		g_dLocals[i].SetSprintf ( "reader-test-%d", i + 1 );

		ServedDesc_t tDesc;
		tDesc.m_sIndexPath.SetSprintf ( "/tmp/very-long-path/goes/here/%s", g_dLocals[i].cstr () );
		g_pLocals->Add ( tDesc, g_dLocals[i] );
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