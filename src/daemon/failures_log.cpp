//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "failures_log.h"
#include "searchdha.h"


static int CmpString ( const CSphString & a, const CSphString & b )
{
	if ( !a.cstr() && !b.cstr() )
		return 0;

	if ( !a.cstr() || !b.cstr() )
		return a.cstr() ? -1 : 1;

	return strcmp ( a.cstr(), b.cstr() );
}

bool operator == ( const SearchFailure_t & l, const SearchFailure_t & r )
{
	return l.m_sIndex == r.m_sIndex && l.m_sError == r.m_sError && l.m_sParentIndex == r.m_sParentIndex;
}

bool operator < ( const SearchFailure_t & l, const SearchFailure_t & r )
{
	int iRes = CmpString ( l.m_sError.cstr(), r.m_sError.cstr() );
	if ( !iRes )
		iRes = CmpString ( l.m_sParentIndex.cstr(), r.m_sParentIndex.cstr() );
	if ( !iRes )
		iRes = CmpString ( l.m_sIndex.cstr(), r.m_sIndex.cstr() );
	return iRes < 0;
}

void ReportIndexesName ( int iSpanStart, int iSpandEnd, const CSphVector<SearchFailure_t> & dLog, StringBuilder_c & sOut )
{
	int iSpanLen = iSpandEnd - iSpanStart;

	// report distributed index in case all failures are from their locals
	if ( iSpanLen > 1 && !dLog[iSpanStart].m_sParentIndex.IsEmpty()
		&& dLog[iSpanStart].m_sParentIndex == dLog[iSpandEnd - 1].m_sParentIndex )
	{
		auto pDist = GetDistr ( dLog[iSpanStart].m_sParentIndex );
		if ( pDist && pDist->m_dLocal.GetLength() == iSpanLen )
		{
			sOut << "index " << dLog[iSpanStart].m_sParentIndex << ": ";
			return;
		}
	}

	// report only first indexes up to 4
	int iEndReport = (iSpanLen > 4) ? iSpanStart + 3 : iSpandEnd;
	sOut.StartBlock ( ",", "table " );
	for ( int j = iSpanStart; j < iEndReport; ++j )
		sOut << dLog[j].m_sIndex;
	sOut.FinishBlock();

	// add total index count
	if ( iEndReport != iSpandEnd )
		sOut.Sprintf ( " and %d more: ", iSpandEnd - iEndReport );
	else
		sOut += ": ";
}

void SearchFailuresLog_c::Submit ( const CSphString & sIndex, const char * sParentIndex, const char * sError )
{
	SearchFailure_t & tEntry = m_dLog.Add();
	tEntry.m_sParentIndex = sParentIndex;
	tEntry.m_sIndex = sIndex;
	tEntry.m_sError = sError;
}


void SearchFailuresLog_c::SubmitVa ( const char * sIndex, const char * sParentIndex, const char * sTemplate, va_list ap )
{
	StringBuilder_c tError;
	tError.vAppendf ( sTemplate, ap );

	SearchFailure_t & tEntry = m_dLog.Add();
	tEntry.m_sParentIndex = sParentIndex;
	tEntry.m_sIndex = sIndex;
	tError.MoveTo ( tEntry.m_sError );
}

void SearchFailuresLog_c::SubmitEx ( const char * sIndex, const char * sParentIndex, const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	SubmitVa ( sIndex, sParentIndex, sTemplate, ap );
	va_end ( ap );
}

void SearchFailuresLog_c::SubmitEx ( const CSphString & sIndex, const char * sParentIndex, const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	SubmitVa ( sIndex.cstr(), sParentIndex, sTemplate, ap );
	va_end ( ap );
}

void SearchFailuresLog_c::BuildReport ( StringBuilder_c & sReport )
{
	if ( IsEmpty() )
		return;

	// collapse same messages
	m_dLog.Uniq();
	int iSpanStart = 0;
	Comma_c sColon ( { ";\n", 2 } );

	for ( int i = 1; i <= m_dLog.GetLength(); ++i )
	{
		// keep scanning while error text is the same
		if ( i != m_dLog.GetLength() )
			if ( m_dLog[i].m_sError == m_dLog[i - 1].m_sError )
				continue;

		if ( m_dLog[iSpanStart].m_sError.IsEmpty() )
			continue;

		sReport << sColon;

		ReportIndexesName ( iSpanStart, i, m_dLog, sReport );
		sReport << m_dLog[iSpanStart].m_sError;

		// done
		iSpanStart = i;
	}
}
