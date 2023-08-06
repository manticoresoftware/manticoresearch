//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2011-2016, Andrew Aksyonoff
// Copyright (c) 2011-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "pseudosharding.h"
#include <math.h>


void DistributeThreadsOverIndexes ( IntVec_t & dThreads, const CSphVector<SplitData_t> & dSplitData, int iConcurrency )
{
	dThreads.Resize ( dSplitData.GetLength() );
	dThreads.Fill(1);

	int64_t iTotalMetric = 0;
	for ( auto & i : dSplitData )
		iTotalMetric += i.m_iMetric;

	// ignore indexes with thread cap==1; they won't get more that 1 thread
	int iThreadsUsed = dSplitData.count_of ( []( auto & i ){ return i.m_iThreadCap==1; } );

	// split remaining threads between left indexes (and apply the thread cap)
	int iThreadsLeft = iConcurrency-iThreadsUsed;
	ARRAY_FOREACH ( i, dSplitData )
	{
		const SplitData_t & tSD = dSplitData[i];
		int & iThreads = dThreads[i];

		if ( tSD.m_iThreadCap==1 )
			continue;

		assert ( tSD.m_iMetric>=0 );
		iThreads = Max ( (int)round ( float(tSD.m_iMetric) / iTotalMetric * iThreadsLeft ), 1 );
		if ( tSD.m_iThreadCap > 1 )
			iThreads = Min ( iThreads, tSD.m_iThreadCap );
	}

	int iCappedThreads = 0;
	int iNonCappedThreads = 0;
	ARRAY_FOREACH ( i, dSplitData )
		if ( dSplitData[i].m_iThreadCap >= 1 )
			iCappedThreads += dThreads[i];
		else
			iNonCappedThreads += dThreads[i];

	// might get negative due to rounding
	iThreadsLeft = Max ( iConcurrency-iCappedThreads, 0 );
	ARRAY_FOREACH ( i, dSplitData )
		if ( dSplitData[i].m_iThreadCap < 1 )
			dThreads[i] = Max ( (int)round ( iThreadsLeft * dThreads[i]/iNonCappedThreads ), 1 );
}


int CalcMaxThreadsPerIndex ( int iConcurrency, int iNumIndexes )
{
	return iNumIndexes<iConcurrency ? ( iConcurrency-iNumIndexes ) + 1 : 1;
}
