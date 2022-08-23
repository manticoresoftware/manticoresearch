//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//


#include <benchmark/benchmark.h>

#include "schema/locator.h"
#include "match.h"

//////////////////////////////////////////////////////////////////////////
static void BM_Locators ( benchmark::State & st )
{
	static const int MAX_ITEMS = 10;
	static const int NUM_MATCHES = 1000;

	CSphRowitem dStatic[MAX_ITEMS];
	CSphRowitem dDynamic[MAX_ITEMS];
	CSphAttrLocator tLoc[NUM_MATCHES];
	CSphMatch tMatch[NUM_MATCHES];

	for ( int i = 0; i<MAX_ITEMS; i++ )
		dStatic[i] = dDynamic[i] = i;

	srand ( 0 );
	for ( int i = 0; i<NUM_MATCHES; ++i )
	{
		tLoc[i].m_iBitCount = 32;
		tLoc[i].m_iBitOffset = 32 * ( rand () % MAX_ITEMS ); // NOLINT
		tLoc[i].m_bDynamic = ( rand () % 2 )==1; // NOLINT
		tMatch[i].m_pStatic = dStatic;
		tMatch[i].m_pDynamic = dDynamic;
	}

	int iSum = 0;
	int j = 0;
	for ( auto _ : st )
	{
		benchmark::DoNotOptimize ( iSum += (int) tMatch[j].GetAttr ( tLoc[j] ) );
		++j;
		if (j>=NUM_MATCHES) j=0;
	}

	// manually cleanup to avoid automatic delete
	for ( auto & m : tMatch )
		m.m_pDynamic = nullptr;
}

BENCHMARK( BM_Locators );
