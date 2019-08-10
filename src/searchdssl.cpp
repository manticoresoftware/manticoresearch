//
// Copyright (c) 2017-2019, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxutils.h"
#include "searchdssl.h"

#if !USE_SSL

bool SslInit() { return false; }
void SslDone() {}

bool SetKeys ( const char * pSslCert, const char * pSslKey, const char * pSslCa ) { return false; }
bool IsSslValid () { return false; }


SslClient_i * SslSetup ( SslClient_i * pClient ) { return nullptr; }
void SslFree ( SslClient_i * pClient ) {}
bool SslTick ( SslClient_i * pClient, bool & bWrite, CSphVector<BYTE> & dBuf, int iLen, int iOff, CSphVector<BYTE> & dDecripted ) { return false; }
bool SslSend ( SslClient_i * pClient, CSphVector<BYTE> & dBuf, CSphVector<BYTE> & dDecripted ) { return false; }

#else

#include "openssl/ssl.h"
#include "openssl/err.h"

#define VERBOSE_SSL 0

#if VERBOSE_NETLOOP
	#define sphLogDebugSSL( ... ) sphLogDebugv (__VA_ARGS__)
#else
#if USE_WINDOWS
#pragma warning(disable:4390)
#endif
	#define sphLogDebugSSL( ... )
#endif

static void SslLogError ( const SSL * pSsl, int iRes )
{
	int iErrno = SSL_get_error ( pSsl, iRes );
	if( iErrno!=SSL_ERROR_NONE )
	{
		while ( iErrno!=SSL_ERROR_NONE )
		{
			// FIXME!!! add client info as regular net action reports
			sphWarning ( "ssl error: %d '%s:%s:%s'", iErrno,
				ERR_lib_error_string ( iErrno ), ERR_func_error_string ( iErrno ), ERR_reason_error_string ( iErrno ) );
			iErrno = ERR_get_error();
		}
	}
}

// need by OpenSSL
struct CRYPTO_dynlock_value
{
	CSphMutex m_tLock;
};

static CSphFixedVector<CSphMutex> g_dSslLocks { 0 };
static SSL_CTX * g_pSslCtx = nullptr;
static bool g_bKeysSet = false;

static void fnSslLock ( int iMode, int iLock, const char * , int )
{
	if ( iMode & CRYPTO_LOCK )
		g_dSslLocks[iLock].Lock();
	else
		g_dSslLocks[iLock].Unlock();
}

CRYPTO_dynlock_value * fnSslLockDynCreate ( const char * , int )
{
	CRYPTO_dynlock_value * pLock = new CRYPTO_dynlock_value;
	return pLock;
}

void fnSslLockDyn ( int iMode, CRYPTO_dynlock_value * pLock, const char * , int )
{
	assert ( pLock );
	if ( iMode & CRYPTO_LOCK )
		pLock->m_tLock.Lock();
	else
		pLock->m_tLock.Unlock();
}

void fnSslLockDynDestroy ( CRYPTO_dynlock_value * pLock, const char * , int )
{
	SafeDelete ( pLock );
}

// init SSL library and global context 
bool SslInit()
{
	int iLocks = CRYPTO_num_locks();
	g_dSslLocks.Reset ( iLocks );
	
	CRYPTO_set_locking_callback ( &fnSslLock );
    CRYPTO_set_dynlock_create_callback ( &fnSslLockDynCreate );
	CRYPTO_set_dynlock_lock_callback ( &fnSslLockDyn );
    CRYPTO_set_dynlock_destroy_callback ( &fnSslLockDynDestroy );

    SSL_load_error_strings();
    SSL_library_init();

	const SSL_METHOD * pMode = SSLv23_method();
	g_pSslCtx = SSL_CTX_new (pMode );
	SSL_CTX_set_verify ( g_pSslCtx, SSL_VERIFY_NONE, nullptr );

	return true;
}

static int fnSslError ( const char * pStr, size_t iLen, void * )
{
	// trim line ending from string end
	while ( iLen && sphIsSpace ( pStr[iLen-1] ) )
		iLen--;

	sphWarning ( "%.*s", (int)iLen, pStr );
	return 1;
}

// set SSL key, certificate and ca-certificate to global SSL context
bool SetKeys ( const char * pSslCert, const char * pSslKey, const char * pSslCa )
{
	assert ( g_pSslCtx );
	assert ( pSslCert || pSslKey || pSslCa );
	bool bOk = true;
	g_bKeysSet = false;

	if ( pSslCert && SSL_CTX_use_certificate_file ( g_pSslCtx, pSslCert, SSL_FILETYPE_PEM )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, nullptr );
		bOk = false;
	}

	if ( bOk && pSslKey && SSL_CTX_use_PrivateKey_file ( g_pSslCtx, pSslKey, SSL_FILETYPE_PEM )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, nullptr );
		bOk = false;
	}

	if ( bOk && pSslCa && SSL_CTX_load_verify_locations ( g_pSslCtx, pSslCa, nullptr )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, nullptr );
		bOk = false;
	}

	// check key and certificate file match
	if ( bOk )
	{
		if ( SSL_CTX_check_private_key( g_pSslCtx )!=1 )
		{
			ERR_print_errors_cb ( &fnSslError, nullptr );
			bOk = false;
		} else
		{
			g_bKeysSet = true;
		}
	}

	return ( bOk && g_bKeysSet );
}

// is global SSL context created and keys set
bool IsSslValid ()
{
	return ( g_pSslCtx && g_bKeysSet );
}

// free SSL related data
void SslDone()
{
	if ( !g_pSslCtx )
		return;

	SSL_CTX_free ( g_pSslCtx );
	g_pSslCtx = nullptr;

	CRYPTO_set_locking_callback ( nullptr );
	CRYPTO_set_dynlock_create_callback ( nullptr );
	CRYPTO_set_dynlock_lock_callback ( nullptr );
	CRYPTO_set_dynlock_destroy_callback ( nullptr );

    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_remove_state ( 0 );
    ERR_free_strings();

	g_dSslLocks.Reset ( 0 );
}

#define SSL_STACK_BUF_SIZE 256

struct SslClient_t : public SslClient_i
{
	SSL * m_pSsl = nullptr;

	BIO * m_pRdBio = nullptr; // SSL reads from, code writes to
	BIO * m_pWrBio = nullptr; // SSL writes to, code reads from

	// data ready to write to socket
	// SSL generates this by encrypts of user incoming data or own writes from renegotiation
	CSphVector<BYTE> m_dWrite;

	// data to encrypt by SSL
	CSphVector<BYTE> m_dEncrypt;
};

enum class SslStatus_e
{
	OK,
	WANT_IO,
	FAIL
};

static SslStatus_e SslGetStatus ( const SSL * pSsl, int iRes )
{
	int iState = SSL_get_error ( pSsl, iRes );
	switch ( iState )
	{
		case SSL_ERROR_NONE:
			return SslStatus_e::OK;
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ:
			return SslStatus_e::WANT_IO;
		case SSL_ERROR_ZERO_RETURN:
		case SSL_ERROR_SYSCALL:
		default:
		{
			SslLogError ( pSsl, iState );
			return SslStatus_e::FAIL;
		}
	}
}

// should only be used when SSL requests write 
static void AddEncrypted ( const BYTE * pSrc, int iLen, SslClient_t * pClient )
{
	BYTE * pBuf = pClient->m_dWrite.AddN ( iLen );
	memcpy ( pBuf, pSrc, iLen );
}

static SslStatus_e SslHandshake ( SslClient_t * pClient )
{
	BYTE dTmp[SSL_STACK_BUF_SIZE];
	sphLogDebugSSL ( "at do_ssl_handshake" );

	int iRes = SSL_do_handshake ( pClient->m_pSsl );
	SslStatus_e eState = SslGetStatus ( pClient->m_pSsl, iRes );

	// SSL request to write
	if ( eState!=SslStatus_e::WANT_IO )
		return eState;

	for ( int iRes=1; iRes>0; )
	{
		iRes = BIO_read ( pClient->m_pWrBio, dTmp, sizeof( dTmp ) );
		sphLogDebugSSL ( "after BIO_read do_ssl_handshake %d", iRes );
		if ( iRes>0 )
			AddEncrypted ( dTmp, iRes, pClient );
		else if ( !BIO_should_retry ( pClient->m_pWrBio ) )
		{
			SslLogError ( pClient->m_pSsl, iRes );
			return SslStatus_e::FAIL;
		}
	}

	return eState;
}

// SSL handles incoming data from client
// data then get into SSL for decrypt
static bool DoRead ( char * pSrc, int iLen, SslClient_t * pClient, CSphVector<BYTE> & dDecripted )
{
	BYTE dTmp[SSL_STACK_BUF_SIZE];
	SslStatus_e eState = SslStatus_e::FAIL;

	while ( iLen>0 )
	{
		int iWriteRes = BIO_write ( pClient->m_pRdBio, pSrc, iLen );
		sphLogDebugSSL ( "after BIO_write on_read_cb 1 %d", iWriteRes );

		if ( iWriteRes<=0 )
		{
			eState = SslGetStatus ( pClient->m_pSsl, iWriteRes );
			return ( eState==SslStatus_e::FAIL );
		}

		pSrc += iWriteRes;
		iLen -= iWriteRes;

		if ( !SSL_is_init_finished ( pClient->m_pSsl ) )
		{
			SslStatus_e iHandshakeState = SslHandshake ( pClient );
			sphLogDebugSSL ( "do_ssl_handshake status %d", (int)iHandshakeState );
			if ( iHandshakeState==SslStatus_e::FAIL )
				return false;
			if ( !SSL_is_init_finished ( pClient->m_pSsl ) )
				return true;
		}

		// encrypted data at input BIO
		// can perform actual read of decrypt data

		for ( iWriteRes=1; iWriteRes>0; )
		{
			int iOff = dDecripted.GetLength();
			dDecripted.Resize ( iOff + SSL_STACK_BUF_SIZE );
			iWriteRes = SSL_read ( pClient->m_pSsl, dDecripted.Begin() + iOff, SSL_STACK_BUF_SIZE );
			sphLogDebugSSL ( "after SSL_read on_read_cb 2 %d", iWriteRes );
			if ( iWriteRes>0 )
			{
				dDecripted.Resize ( iOff + iWriteRes );
			} else
			{
				dDecripted.Resize ( iOff );
			}
		}

		eState = SslGetStatus ( pClient->m_pSsl, iWriteRes );

		// SSL request for write, can be due to client asks SSL renegotiation
		if ( eState==SslStatus_e::WANT_IO )
		{
			for ( int iRes=1; iRes>0; )
			{
				iRes = BIO_read ( pClient->m_pWrBio, dTmp, sizeof( dTmp ) );
				sphLogDebugSSL ( "after BIO_read on_read_cb 3 %d", iRes );

				if ( iRes>0 )
					AddEncrypted ( dTmp, iRes, pClient );
				else if ( !BIO_should_retry ( pClient->m_pWrBio ) )
					return false;
			}
		}

		if ( eState==SslStatus_e::FAIL )
			return false;
	}

	return true;
}

// handle outbound decrypted data to encrypt
// need to pass data into SSL for encrypt
// after SSL generates encrypted data then get into socket write operation
static bool SslEncrypt ( SslClient_t * pClient )
{
	BYTE dTmp[SSL_STACK_BUF_SIZE];
	SslStatus_e eState = SslStatus_e::FAIL;

	if ( !SSL_is_init_finished ( pClient->m_pSsl ) )
		return true;

	while ( pClient->m_dEncrypt.GetLength() )
	{
		int iWriteRes = SSL_write ( pClient->m_pSsl, pClient->m_dEncrypt.Begin(), pClient->m_dEncrypt.GetLength() );
		eState = SslGetStatus ( pClient->m_pSsl, iWriteRes );
		sphLogDebugSSL ( "after SSL_write do_encrypt 1 %d", iWriteRes );

		if ( iWriteRes>0 )
		{
			if ( iWriteRes<pClient->m_dEncrypt.GetLength() )
				memmove ( pClient->m_dEncrypt.Begin(), pClient->m_dEncrypt.Begin() + iWriteRes, pClient->m_dEncrypt.GetLength() - iWriteRes );
			pClient->m_dEncrypt.Resize ( pClient->m_dEncrypt.GetLength() - iWriteRes );

			// SSL output to socket write operation
			for ( iWriteRes=1; iWriteRes>0; )
			{
				iWriteRes = BIO_read ( pClient->m_pWrBio, dTmp, sizeof( dTmp ) );
				sphLogDebugSSL ( "after BIO_read do_encrypt 2 %d", iWriteRes );

				if ( iWriteRes>0 )
				{
					AddEncrypted( dTmp, iWriteRes, pClient );
				} else if ( !BIO_should_retry( pClient->m_pWrBio ) )
				{
					SslLogError ( pClient->m_pSsl, iWriteRes );
					return false;
				}
			}
		}

		if ( eState==SslStatus_e::FAIL )
			return false;

		if ( iWriteRes==0 )
			break;
	}
	return true;
}

// setup SSL object at net action
SslClient_i * SslSetup ( SslClient_i * pClient )
{
	SslClient_t * pSession = (SslClient_t *)pClient;
	if ( !pSession )
	{
		pSession = new SslClient_t();
		pSession->m_pRdBio = BIO_new ( BIO_s_mem() );
		pSession->m_pWrBio = BIO_new ( BIO_s_mem() );
		pSession->m_pSsl = SSL_new ( g_pSslCtx );
		SSL_set_accept_state ( pSession->m_pSsl );
		SSL_set_bio ( pSession->m_pSsl, pSession->m_pRdBio, pSession->m_pWrBio );
	} else
	{
		pSession->m_dWrite.Resize ( 0 );
		pSession->m_dEncrypt.Resize ( 0 );
	}

	return pSession;
}

// free SSL object at net action (on socket close or net action finally remove)
void SslFree ( SslClient_i * pClient )
{
	SslClient_t * pSession = (SslClient_t *)pClient;
	if ( pSession )
	{
		SSL_free ( pSession->m_pSsl );
		SafeDelete ( pClient );
	}
}

// handle incoming or outgoing data
bool SslTick ( SslClient_i * pClient, bool & bWrite, CSphVector<BYTE> & dBuf, int iLen, int iOff, CSphVector<BYTE> & dDecripted )
{
	SslClient_t * pSession = (SslClient_t *)pClient;
	assert ( pSession );

	if ( !bWrite && !DoRead ( (char *)dBuf.Begin() + iOff, iLen, pSession, dDecripted ) )
		return true;

    if ( pSession->m_dEncrypt.GetLength() )
		SslEncrypt( pSession );

	if ( pSession->m_dWrite.GetLength()>0 )
	{
		dBuf.Resize ( pSession->m_dWrite.GetLength() );
		memcpy ( dBuf.Begin(), pSession->m_dWrite.Begin(), pSession->m_dWrite.GetLength() );
		pSession->m_dWrite.Resize ( 0 );
		bWrite = true;
	}

	return false;
}

// encrypt data for sending
bool SslSend ( SslClient_i * pClient, CSphVector<BYTE> & dBuf, CSphVector<BYTE> & dDecripted )
{
	assert ( pClient );
	SslClient_t * pSession = (SslClient_t *)pClient;
	pSession->m_dEncrypt.SwapData ( dDecripted );
	if ( !SslEncrypt ( pSession ) )
		return false;

	if ( pSession->m_dWrite.GetLength()>0 )
		dBuf.SwapData ( pSession->m_dWrite );

	return true;
}

#endif