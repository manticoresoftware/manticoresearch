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

/// @file searchdssl.h
/// SSL 

#pragma once


struct SslClient_i
{
	SslClient_i () = default;
	virtual ~SslClient_i() = default;
};

// init SSL library and global context 
bool SslInit();

// free SSL related data
void SslDone();

// set SSL key, certificate and ca-certificate to global SSL context
bool SetKeys ( const char * pSslCert, const char * pSslKey, const char * pSslCa );

// is global SSL context created and keys set
bool IsSslValid ();


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// net action functions

// setup SSL object at net action
SslClient_i * SslSetup ( SslClient_i * pClient );

// free SSL object at net action (on socket close or net action finally remove)
void SslFree ( SslClient_i * pClient );

// handle incoming or outgoing data
bool SslTick ( SslClient_i * pClient, bool & bWrite, CSphVector<BYTE> & dBuf, int iLen, int iOff, CSphVector<BYTE> & dDecrypted );

// encrypt data for sending 
bool SslSend ( SslClient_i * pClient, CSphVector<BYTE> & dBuf, CSphVector<BYTE> & dDecrypted );
