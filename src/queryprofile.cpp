//
// Copyright (c) 2017-2022, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "queryprofile.h"

QueryProfile_c::QueryProfile_c()
{
	Start ( SPH_QSTATE_TOTAL );
}


ESphQueryState QueryProfile_c::Switch ( ESphQueryState eNew )
{
	int64_t tmNow = sphMicroTimer();
	ESphQueryState eOld = m_eState;
	m_dSwitches [ eOld ]++;
	m_tmTotal [ eOld ] += tmNow - m_tmStamp;
	m_eState = eNew;
	m_tmStamp = tmNow;
	return eOld;
}


void QueryProfile_c::Start ( ESphQueryState eNew )
{
	memset ( m_dSwitches, 0, sizeof(m_dSwitches) );
	memset ( m_tmTotal, 0, sizeof(m_tmTotal) );
	m_eState = eNew;
	m_tmStamp = sphMicroTimer();
	m_iPseudoShards = 1;
	m_iMaxMatches = 0;
}


void QueryProfile_c::AddMetric ( const QueryProfile_c & tData )
{
	// fixme! m.b. invent a way to display data from different profilers with kind of multiplier?
	for ( int i = 0; i<SPH_QSTATE_TOTAL; ++i )
	{
		m_dSwitches[i] += tData.m_dSwitches[i];
		m_tmTotal[i] += tData.m_tmTotal[i];
	}
}


/// stop profiling
void QueryProfile_c::Stop()
{
	Switch ( SPH_QSTATE_TOTAL );
}

//////////////////////////////////////////////////////////////////////////

CSphScopedProfile::CSphScopedProfile ( QueryProfile_c * pProfile, ESphQueryState eNewState )
{
	m_pProfile = pProfile;
	m_eOldState = SPH_QSTATE_UNKNOWN;
	if ( m_pProfile )
		m_eOldState = m_pProfile->Switch ( eNewState );
}


CSphScopedProfile::~CSphScopedProfile()
{
	if ( m_pProfile )
		m_pProfile->Switch ( m_eOldState );
}
