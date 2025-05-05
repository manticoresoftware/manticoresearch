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

#include "logger.h"
#include "fileutils.h"
#include "searchdaemon.h"
#include "std/string.h"

#include <sys/stat.h>

static int g_iHttpLogFile = -1;
static CSphString g_sHttpLogFile;


bool HttpLogEnabled () noexcept
{
	return g_iHttpLogFile != -1;
}

void ReopenHttpLog ()
{
	if ( g_iHttpLogFile < 0 || isatty ( g_iHttpLogFile ) )
		return;


	int iFD = ::open ( g_sHttpLogFile.cstr(), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
	if ( iFD < 0 )
	{
		sphWarning ( "failed to reopen http log file '%s': %s", g_sHttpLogFile.cstr (), strerrorm ( errno ) );
	} else
	{
		::close ( g_iHttpLogFile );
		g_iHttpLogFile = iFD;
		ChangeLogFileMode ( g_iHttpLogFile );
		sphInfo ( "http log reopened" );
	}
}

void SetupHttpLog ( CSphString sHttpLog )
{
	FixPathAbsolute ( sHttpLog );
	g_iHttpLogFile = ::open ( sHttpLog.cstr (), O_CREAT | O_RDWR | O_APPEND, S_IREAD | S_IWRITE );
	if ( g_iHttpLogFile<0 )
		sphWarning ( "failed to open http log file '%s': %s", sHttpLog.cstr (), strerrorm ( errno ) );
	else
	{
		ChangeLogFileMode ( g_iHttpLogFile );
		g_sHttpLogFile = std::move( sHttpLog );
	}
}

static void logRec ( const StringBuilder_c& sData )
{
	sphSeek ( g_iHttpLogFile, 0, SEEK_END );
	sphWrite ( g_iHttpLogFile, sData.cstr (), sData.GetLength () );
}

void logHttpOutput ( VecTraits_T<BYTE> dData, int iReqID )
{
	StringBuilder_c tBuf;
	tBuf << "[";
	sphFormatCurrentTime ( tBuf );
	tBuf << "] [Request-ID: " << iReqID << "] - Outgoing HTTP Response:\n";
	auto iLine = tBuf.GetLength();
	for ( auto i = 1; i<iLine; ++i )
		tBuf << ">";
	tBuf << "\n";
	if ( dData.IsEmpty () )
		tBuf << "<empty response>";
	else
		tBuf << Str_t { dData };
	tBuf << "\n\n";
	logRec ( tBuf );
}

void logHttpInput ( VecTraits_T<BYTE> sData, int iReqID, int64_t iTimestamp )
{
	StringBuilder_c tBuf;
	tBuf << "[";
	sphFormatTime ( iTimestamp, tBuf );
	tBuf << "] [Request-ID: " << iReqID << "] - Incoming HTTP Request:\n";
	auto iLine = tBuf.GetLength ();
	for ( auto i = 1; i<iLine; ++i )
		tBuf << "<";
	tBuf << "\n";
	if ( sData.IsEmpty () )
		tBuf << "<empty request>";
	else
		tBuf.AppendRawChunk ( sData );
	tBuf << "\n\n";
	logRec ( tBuf );
}
