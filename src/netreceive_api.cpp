//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "netreceive_api.h"

extern int g_iClientTimeoutS; // from searchd.cpp
extern volatile bool g_bMaintenance;
static auto & g_bGotSighup = sphGetGotSighup ();    // we just received SIGHUP; need to log

// mostly repeats HandleClientSphinx
void ApiServe ( AsyncNetBufferPtr_c pBuf )
{
	// non-vip connections in maintainance should be already rejected on accept
	assert  ( !g_bMaintenance || myinfo::IsVIP () );

	myinfo::SetProto ( Proto_e::SPHINX );
	int iCID = myinfo::ConnID();
	const char * sClientIP = myinfo::szClientName();

	// needed to check permission to turn maintenance mode on/off
	auto& tOut = *(NetGenericOutputBuffer_c *) pBuf;
	auto& tIn = *(AsyncNetInputBuffer_c *) pBuf;

	// send handshake
	myinfo::TaskState ( TaskState_e::HANDSHAKE );
	tOut.SendDword ( SPHINX_SEARCHD_PROTO ); // that is handshake
	if ( !tIn.ReadFrom ( 4, true ))
	{
		sphWarning ( "failed to receive API handshake (client=%s(%d), exp=%d, error='%s')",
				sClientIP, iCID, 4, sphSockError ());
		return;
	}
	auto uHandshake = tIn.GetDword();
	sphLogDebugv ( "conn %s(%d): got handshake, major v.%d", sClientIP, iCID, uHandshake );
	if ( uHandshake!=SPHINX_CLIENT_VERSION && uHandshake!=0x01000000UL )
	{
		sphLogDebugv ( "conn %s(%d): got handshake, major v.%d", sClientIP, iCID, uHandshake );
		return;
	}
	if ( !tIn.HasBytes () && !tOut.Flush ())
	{
		sphLogDebugv ( "conn %s(%d): timeout when sending handshake", sClientIP, iCID );
		return;
	}

	bool bPersist = false;
	int iPconnIdleS = 0;

	// main loop for one ore more commands (if persist)
	do
	{
		if ( !tIn.HasBytes ())
			tIn.DiscardProcessed ();

		auto iTimeoutS = bPersist ? 1 : g_iReadTimeoutS; // default 1 vs 5 seconds
		sphLogDebugv ( "conn %s(%d): loop start with timeout %d", sClientIP, iCID, iTimeoutS );
		tIn.SetTimeoutUS ( S2US * iTimeoutS );

		// in "persistent connection" mode, we want interruptible waits
		// so that the worker child could be forcibly restarted
		//
		// currently, the only signal allowed to interrupt this read is SIGTERM
		// letting SIGHUP interrupt causes trouble under query/rotation pressure
		// see sphSockRead() and ReadFrom() for details
		bool bCommand = tIn.ReadFrom ( 8, bPersist );

		if ( !bCommand )
		{
			// on SIGTERM, bail unconditionally and immediately, at all times
			if ( sphInterrupted () )
			{
				sphLogDebugv ( "conn %s(%d): bailing on SIGTERM", sClientIP, iCID );
				break;
			}

			// on SIGHUP vs pconn, bail if a pconn was idle for 1 sec
			if ( bPersist && sphSockPeekErrno ()==ETIMEDOUT )
			{
				sphLogDebugv ( "conn %s(%d): persist + timeout condition", sClientIP, iCID );
				if ( g_bGotSighup )
				{
					sphLogDebugv ( "conn %s(%d): bailing idle pconn on SIGHUP", sClientIP, iCID );
					break;
				}

				// on pconn that was idle for 300 sec (client_timeout), bail
				iPconnIdleS += iTimeoutS;
				bool bClientTimedout = ( iPconnIdleS>=g_iClientTimeoutS );
				if ( bClientTimedout )
					sphLogDebugv ( "conn %s(%d): bailing idle pconn on client_timeout", sClientIP, iCID );
				else
				{
					sphLogDebugv ( "conn %s(%d): timeout, not reached, continue", sClientIP, iCID );
					continue;
				}
			}
			break; // some error, need not to continue.
		}

		iPconnIdleS = 0;

		auto eCommand = (SearchdCommand_e)  tIn.GetWord ();
		auto uVer = tIn.GetWord ();
		auto iReplySize = tIn.GetInt ();
		sphLogDebugv ( "read command %d, version %d, reply size %d", eCommand, uVer, iReplySize );


		bool bBadCommand = ( eCommand>=SEARCHD_COMMAND_WRONG );
		bool bBadLength = ( iReplySize<0 || iReplySize>g_iMaxPacketSize );
		if ( bBadCommand || bBadLength )
		{
			// unknown command, default response header
			if ( bBadLength )
				sphWarning ( "ill-formed client request (length=%d out of bounds)", iReplySize );
			// if command is insane, low level comm is broken, so we bail out
			if ( bBadCommand )
				sphWarning ( "ill-formed client request (command=%d, SEARCHD_COMMAND_TOTAL=%d)", eCommand,
							 SEARCHD_COMMAND_TOTAL );

			SendErrorReply ( tOut, "invalid command (code=%d, len=%d)", eCommand, iReplySize );
			tOut.Flush(); // no need to check return code since we anyway break
			break;
		}

		if ( iReplySize && !tIn.ReadFrom ( iReplySize, true ))
		{
			sphWarning ( "failed to receive API body (client=%s(%d), exp=%d, error='%s')",
					sClientIP, iCID, iReplySize, sphSockError ());
			break;
		}

		auto& tCrashQuery = GlobalCrashQueryGetRef();
		tCrashQuery.m_pQuery = tIn.GetBufferPtr ();
		tCrashQuery.m_iSize = iReplySize;
		tCrashQuery.m_bMySQL = false;
		tCrashQuery.m_uCMD = eCommand;
		tCrashQuery.m_uVer = uVer;

		// special process for 'ping' as immediate answer
		if ( eCommand ==SEARCHD_COMMAND_PING )
		{
			HandleCommandPing ( tOut, uVer, tIn );
			tOut.Flush(); // no need to check return code since we anyway break
			break;
		}

		if ( IsMaxedOut() )
		{
			sphWarning ( "%s", g_sMaxedOutMessage );
			{
				auto tHdr = APIHeader ( tOut, SEARCHD_RETRY );
				tOut.SendString ( g_sMaxedOutMessage );
			}
			tOut.Flush(); // no need to check return code since we anyway break
			break;
		}

		bPersist |= LoopClientSphinx ( eCommand, uVer, iReplySize, tIn, tOut, false );
		if ( !tOut.Flush () )
			break;
	} while (bPersist);

	sphLogDebugv ( "conn %s(%d): exiting", sClientIP, iCID );
}


// Start Sphinx API command/request header
APIBlob_c APIHeader ( ISphOutputBuffer & dBuff, WORD uCommand, WORD uVer )
{
	dBuff.SendWord ( uCommand );
	dBuff.SendWord ( uVer );
	return APIBlob_c ( dBuff );
}

// Sphinx API answer (same as APIHeader, but 2-nd and 3-rd params interchanged. Fixme! Unify.
APIBlob_c APIAnswer ( ISphOutputBuffer & dBuff, WORD uVer, WORD uStatus )
{
	return APIHeader ( dBuff, uStatus, uVer );
}