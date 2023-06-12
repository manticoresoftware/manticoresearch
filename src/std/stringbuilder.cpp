//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "stringbuilder.h"

#include "sphinxutils.h"
#include "generics.h"
#include "relimit.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>

//////////////////////////////////////////////////////////////////////////
/// StringBuilder implementation
//////////////////////////////////////////////////////////////////////////
StringBuilder_c::StringBuilder_c ( const char * sDel, const char * sPref, const char * sTerm )
{
	if ( sDel || sPref || sTerm )
		StartBlock ( sDel, sPref, sTerm );
}

StringBuilder_c::~StringBuilder_c()
{
	SafeDeleteArray ( m_szBuffer );
}

StringBuilder_c::StringBuilder_c ( StringBuilder_c&& rhs ) noexcept
{
	Swap (rhs);
}

void StringBuilder_c::Swap ( StringBuilder_c& rhs ) noexcept
{
	::Swap ( m_szBuffer, rhs.m_szBuffer );
	::Swap ( m_iSize, rhs.m_iSize );
	::Swap ( m_iUsed, rhs.m_iUsed );
	::Swap ( m_dDelimiters, rhs.m_dDelimiters );
}

StringBuilder_c & StringBuilder_c::operator= ( StringBuilder_c rhs ) noexcept
{
	Swap (rhs);
	return *this;
}

int StringBuilder_c::StartBlock ( const char * sDel, const char * sPref, const char * sTerm )
{
	m_dDelimiters.Add ( LazyComma_c ( sDel, sPref, sTerm ) );
	return m_dDelimiters.GetLength();
}

int StringBuilder_c::StartBlock ( const StrBlock_t& dBlock )
{
	m_dDelimiters.Add ( LazyComma_c ( dBlock ) );
	return m_dDelimiters.GetLength();
}

int StringBuilder_c::MuteBlock ()
{
	m_dDelimiters.Add ( LazyComma_c() );
	return m_dDelimiters.GetLength ();
}

void StringBuilder_c::FinishBlock ( bool bAllowEmpty ) // finish last pushed block
{
	if ( m_dDelimiters.IsEmpty() )
		return;

	if ( !bAllowEmpty && !m_dDelimiters.Last().Started() )
	{
		AppendChunk ( {"\0", 1} );
		--m_iUsed;
	}

	if ( m_dDelimiters.Last().Started () )
		AppendRawChunk ( m_dDelimiters.Last().m_sSuffix );

	m_dDelimiters.Pop();
}

void StringBuilder_c::FinishBlocks ( int iLevel, bool bAllowEmpty )
{
	while ( !m_dDelimiters.IsEmpty() && iLevel<=m_dDelimiters.GetLength() )
		FinishBlock ( bAllowEmpty );
}

StringBuilder_c & StringBuilder_c::vAppendf ( const char * sTemplate, va_list ap )
{
	if ( !m_szBuffer )
		InitBuffer ();

	assert ( m_szBuffer );
	assert ( m_iUsed<m_iSize );

	auto sComma = Delim();

	while (true)
	{
		int64_t iLeft = m_iSize - m_iUsed;
		if ( sComma.second && sComma.second < iLeft ) // prepend delimiter first...
		{
			if ( sComma.second )
				memcpy ( m_szBuffer + m_iUsed, sComma.first, sComma.second );
			iLeft -= sComma.second;
			m_iUsed += sComma.second;
			sComma = dEmptyStr;
		}

		// try to append
		va_list cp;
		va_copy ( cp, ap );
		int iPrinted = vsnprintf ( m_szBuffer + m_iUsed, iLeft, sTemplate, cp );
		va_end( cp );

		// success? bail
		// note that we check for strictly less, not less or equal
		// that is because vsnprintf does *not* count the trailing zero
		// meaning that if we had N bytes left, and N bytes w/o the zero were printed,
		// we do not have a trailing zero anymore, but vsnprintf succeeds anyway
		if ( iPrinted>=0 && iPrinted<iLeft )
		{
			m_iUsed += iPrinted;
			break;
		}

		// we need more chars!
		// either 256 (happens on Windows; lets assume we need 256 more chars)
		// or get all the needed chars and 64 more for future calls
		GrowEnough ( iPrinted<0 ? 256 : iPrinted + sComma.second );
	}
	return *this;
}

StringBuilder_c & StringBuilder_c::Appendf ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	vAppendf ( sTemplate, ap );
	va_end ( ap );
	return *this;
}

StringBuilder_c & StringBuilder_c::vSprintf ( const char * sTemplate, va_list ap )
{
	if ( !m_szBuffer )
		InitBuffer ();

	assert ( m_iUsed==0 || m_iUsed<m_iSize );

	auto sComma = Delim();

	if ( sComma.second ) // prepend delimiter first...
	{
		GrowEnough ( sComma.second );
		memcpy ( m_szBuffer + m_iUsed, sComma.first, sComma.second );
		m_iUsed += sComma.second;
	}
	sph::vSprintf ( *this, sTemplate, ap );
	return *this;
}

/* Custom format specifiers for types:

	'int' values:
	%d - decimal int
	%F - fixed-point int (see notes below about fixed-points)
	%i - skip int. Prints nothing, just skips parameter. Useful for conditional format.

	'DWORD' (uint32) values:
	%u - decimal uint32
	%x - hex uint32

	'int64_t' (signed) values:
	%l - decimal int64
	%D - fixed-point int64
	%t - timespan (see notes below)
	%T - timestamp from now

	'uint64_t' (unsigned) values:
	%U - decimal uint64

	z-terminated string:
	%s - print string, or "(null)"

	etc.:
	%p - print pointer (16 hex digits)
	%f - float (fall-back to standard sprintf)

	Fixed-point ints (both 32 and 64 bits) need precise param to set the decimal point at this place
	Example:	( "%.4F", 999005 ) will output '99.9005'.
				( "%.3D", (int64_t) -10000 ) will output '-10.000'

	Timespan prints time expressed in useconds in human-readable format. It output number with suffix.
	Suffixes are 'us', 'ms', 's', 'm', 'h', 'd', 'w' (usecs, msecs, secs, mins, hours, days, weeks).
	Example:	("%t", 1555555) will print "2s" (2 seconds)
				("%t", 3600000000*24*2) will print "2d" (2 days)

	Timespan may be supplied with precision param, and then it will print up to 7 numbers with suffixes.
	Example:	("%.2t", 1555555) will print "1.56s"
				("%.5t", 1555555) will print "1s 555.56ms"
				("%.8t", 71555555) will print "1m 11s 555ms 555us"
	Timespan also reasonable rounds the value when necessary and doesn't output redundant zeros.
	Example:	("%.7t", 89999994) will print "1m 29s 999.99ms"
				("%.7t", 89999995) will print "1m 30s"

	Timestamp from now - just takes provided value, compares it with sphMicroTimer() output
	and print the timespan difference according to the rules:
		- if given value is in the past, print 'TM ago' (where TM is timespan of the difference)
		- if given value is in the future, print 'in TM'.
		- if given value is exactly now, print the word 'now'.
 */

StringBuilder_c & StringBuilder_c::Sprintf ( const char * sTemplate, ... )
{
	va_list ap;
	va_start ( ap, sTemplate );
	vSprintf ( sTemplate, ap );
	va_end ( ap );
	return *this;
}

BYTE * StringBuilder_c::Leak()
{
	auto pRes = ( BYTE * ) m_szBuffer;
	NewBuffer ();
	return pRes;
}

void StringBuilder_c::MoveTo ( CSphString &sTarget )
{
	sTarget.Adopt ( &m_szBuffer );
	NewBuffer ();
}


void StringBuilder_c::Grow ( int iLen )
{
	assert ( m_iSize<m_iUsed + iLen + GROW_STEP );
	m_iSize = sph::DefaultRelimit::Relimit ( m_iSize, m_iUsed + iLen + GROW_STEP );
	auto * pNew = new char[m_iSize];
	if ( m_szBuffer )
		memcpy ( pNew, m_szBuffer, m_iUsed + 1 );
	::Swap ( pNew, m_szBuffer );
	SafeDeleteArray ( pNew );
}

void StringBuilder_c::InitBuffer()
{
	m_iSize = 256;
	m_szBuffer = new char[m_iSize];
}

void StringBuilder_c::NewBuffer()
{
	InitBuffer();
	Clear ();
}

StringBuilder_c& StringBuilder_c::operator<< ( double fVal )
{
	InitAddPrefix();
	GrowEnough ( 32 );
	m_iUsed += snprintf ( end(), 31, "%f", fVal );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

StringBuilder_c& StringBuilder_c::operator<< ( float fVal )
{
	InitAddPrefix();
	GrowEnough ( 32 );
	m_iUsed += sph::PrintVarFloat ( end(), 31, fVal );
	m_szBuffer[m_iUsed] = '\0';
	return *this;
}

void StringBuilder_c::FtoA ( float fVal )
{
	InitAddPrefix();

	const int MAX_NUMERIC_STR = 64;
	GrowEnough ( MAX_NUMERIC_STR + 1 );

	int iLen = sph::PrintVarFloat ( (char*)m_szBuffer + m_iUsed, MAX_NUMERIC_STR, fVal );
	m_iUsed += iLen;
	m_szBuffer[m_iUsed] = '\0';
}


void StringBuilder_c::DtoA ( double fVal )
{
	InitAddPrefix();

	const int MAX_NUMERIC_STR = 64;
	GrowEnough ( MAX_NUMERIC_STR + 1 );

	int iLen = sph::PrintVarDouble ( (char*)m_szBuffer + m_iUsed, MAX_NUMERIC_STR, fVal );
	m_iUsed += iLen;
	m_szBuffer[m_iUsed] = '\0';
}

//////////////////////////////////////////////////////////////////////////

StringBuilder_c::LazyComma_c::LazyComma_c ( const char * sDelim, const char * sPrefix, const char * sTerm )
	: Comma_c ( sDelim )
{
	if ( sPrefix )
		m_sPrefix = { sPrefix, (int) strlen(sPrefix) };

	if ( sTerm )
		m_sSuffix = { sTerm, (int) strlen(sTerm ) };
}

StringBuilder_c::LazyComma_c::LazyComma_c( const StrBlock_t& dBlock )
	: Comma_c ( std::get<0>(dBlock) )
	, m_sPrefix ( std::get<1>(dBlock) )
	, m_sSuffix ( std::get<2>(dBlock) )
{}


const Str_t& StringBuilder_c::LazyComma_c::RawComma ( const std::function<void ()> & fnAddNext )
{
	if ( m_bSkipNext )
	{
		m_bSkipNext = false;
		return dEmptyStr;
	}

	if ( Started() )
		return m_sComma;

	m_bStarted = true;
	fnAddNext();
	return m_sPrefix;
}


CSphString ConcatWarnings ( StrVec_t & dWarnings )
{
	dWarnings.Uniq();

	StringBuilder_c sRes ( "; " );
	for ( const auto & i : dWarnings )
		sRes << i;

	return sRes.cstr();
}
