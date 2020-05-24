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

#include "sphinx.h"
#include "sphinxutils.h"
#include "searchdssl.h"

#if !USE_SSL

void SetServerSSLKeys ( CSphVariant *,  CSphVariant *,  CSphVariant * ) {}
bool CheckWeCanUseSSL () { return false; }
bool MakeSecureLayer ( AsyncNetBufferPtr_c & ) { return false; }

#else

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>

#if 0
#define VERBOSE_SSL 0

#if VERBOSE_SSL
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
#endif

// need by OpenSSL
struct CRYPTO_dynlock_value
{
	CSphMutex m_tLock;
};

static CSphFixedVector<CSphMutex> g_dSslLocks { 0 };

static CSphString g_sSslCert;
static CSphString g_sSslKey;
static CSphString g_sSslCa;

void fnSslLock ( int iMode, int iLock, const char * , int )
{
	if ( iMode & CRYPTO_LOCK )
		g_dSslLocks[iLock].Lock();
	else
		g_dSslLocks[iLock].Unlock();
}

CRYPTO_dynlock_value * fnSslLockDynCreate ( const char * , int )
{
	auto * pLock = new CRYPTO_dynlock_value;
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

static BIO_METHOD * BIO_s_coroAsync ( bool bDestroy = false );

#if 0
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

	const SSL_METHOD * pMode = nullptr;
#if HAVE_TLS_SERVER_METHOD
	pMode = TLS_server_method();
#elif HAVE_TLSV1_2_METHOD
	pMode = TLSv1_2_server_method();
#elif HAVE_TLSV1_1_SERVER_METHOD
	pMode = TLSv1_1_server_method();
#else
	pMode = SSLv23_server_method();
#endif
	g_pSslCtx = SSL_CTX_new (pMode );
	SSL_CTX_set_verify ( g_pSslCtx, SSL_VERIFY_NONE, nullptr );

	return true;
}
#endif

static int fnSslError ( const char * pStr, size_t iLen, void * )
{
	// trim line ending from string end
	while ( iLen && sphIsSpace ( pStr[iLen-1] ) )
		iLen--;

	sphWarning ( "%.*s", (int)iLen, pStr );
	return 1;
}

#if 0
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
static bool DoRead ( char * pSrc, int iLen, SslClient_t * pClient, CSphVector<BYTE> & dDecrypted )
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
			int iOff = dDecrypted.GetLength();
			dDecrypted.Resize ( iOff + SSL_STACK_BUF_SIZE );
			iWriteRes = SSL_read ( pClient->m_pSsl, dDecrypted.Begin() + iOff, SSL_STACK_BUF_SIZE );
			sphLogDebugSSL ( "after SSL_read on_read_cb 2 %d", iWriteRes );
			if ( iWriteRes>0 )
			{
				dDecrypted.Resize ( iOff + iWriteRes );
			} else
			{
				dDecrypted.Resize ( iOff );
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
bool SslTick ( SslClient_i * pClient, bool & bWrite, CSphVector<BYTE> & dBuf, int iLen, int iOff, CSphVector<BYTE> & dDecrypted )
{
	SslClient_t * pSession = (SslClient_t *)pClient;
	assert ( pSession );

	if ( !bWrite && !DoRead ((char *)dBuf.Begin() + iOff, iLen, pSession, dDecrypted ) )
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
bool SslSend ( SslClient_i * pClient, CSphVector<BYTE> & dBuf, CSphVector<BYTE> & dDecrypted )
{
	assert ( pClient );
	SslClient_t * pSession = (SslClient_t *)pClient;
	pSession->m_dEncrypt.SwapData ( dDecrypted );
	if ( !SslEncrypt ( pSession ) )
		return false;

	if ( pSession->m_dWrite.GetLength()>0 )
		dBuf.SwapData ( pSession->m_dWrite );

	return true;
}
#endif

#define FBLACK	"\x1b[30m"
#define FRED	"\x1b[31m"
#define FGREEN	"\x1b[32m"
#define FYELLOW	"\x1b[33m"
#define FCYAN	"\x1b[34m"
#define FPURPLE	"\x1b[35m"
#define FBLUE	"\x1b[35m"
#define FWHITE	"\x1b[35m"

#define NORM    "\x1b[0m"

#define FRONT FRED
#define FRONTN FPURPLE
#define BACK FGREEN
#define BACKN FYELLOW
#define SYSN FCYAN

void SetServerSSLKeys ( CSphVariant* pSslCert, CSphVariant* pSslKey, CSphVariant* pSslCa )
{
	if ( pSslCert )
		g_sSslCert = pSslCert->cstr();
	if ( pSslKey )
		g_sSslKey = pSslKey->cstr ();
	if ( pSslCa )
		g_sSslCa = pSslCa->cstr ();
}

static bool IsKeysSet()
{
	return !( g_sSslCert.IsEmpty () && g_sSslKey.IsEmpty () && g_sSslCa.IsEmpty ());
}

// set SSL key, certificate and ca-certificate to global SSL context
static bool SetGlobalKeys ( SSL_CTX * pCtx )
{
	if ( !(IsKeysSet()) )
		return false;

	if ( !g_sSslCert.IsEmpty () && SSL_CTX_use_certificate_file ( pCtx, g_sSslCert.cstr (), SSL_FILETYPE_PEM )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, nullptr );
		return false;
	}

	if ( !g_sSslKey.IsEmpty () && SSL_CTX_use_PrivateKey_file ( pCtx, g_sSslKey.cstr (), SSL_FILETYPE_PEM )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, nullptr );
		return false;
	}

	if ( !g_sSslCa.IsEmpty () && SSL_CTX_load_verify_locations ( pCtx, g_sSslCa.cstr(), nullptr )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, nullptr );
		return false;
	}

	// check key and certificate file match
	if ( SSL_CTX_check_private_key( pCtx )!=1 )
	{
		ERR_print_errors_cb ( &fnSslError, nullptr );
		return false;
	}

	return true;
}

// free SSL related data
static void SslFreeCtx ( SSL_CTX * pCtx )
{
	if ( !pCtx )
		return;

	SSL_CTX_free ( pCtx );
	pCtx = nullptr;

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

using SmartSSL_CTX_t = SharedPtrCustom_t<SSL_CTX *>;

// init SSL library and global context by demand
static SmartSSL_CTX_t GetSslCtx ()
{
	static SmartSSL_CTX_t pSslCtx;
	if ( !pSslCtx )
	{
		int iLocks = CRYPTO_num_locks();
		g_dSslLocks.Reset ( iLocks );

		CRYPTO_set_locking_callback ( &fnSslLock );
		CRYPTO_set_dynlock_create_callback ( &fnSslLockDynCreate );
		CRYPTO_set_dynlock_lock_callback ( &fnSslLockDyn );
		CRYPTO_set_dynlock_destroy_callback ( &fnSslLockDynDestroy );

		SSL_load_error_strings();
		SSL_library_init();

		const SSL_METHOD * pMode = nullptr;
		#if HAVE_TLS_SERVER_METHOD
		pMode = TLS_server_method ();
		#elif HAVE_TLSV1_2_METHOD
		pMode = TLSv1_2_server_method();
		#elif HAVE_TLSV1_1_SERVER_METHOD
		pMode = TLSv1_1_server_method();
		#else
		pMode = SSLv23_server_method();
		#endif
		pSslCtx = SmartSSL_CTX_t ( SSL_CTX_new ( pMode ), [] ( SSL_CTX *)
		{
			sphLogDebugv ( BACKN "~~ Releasing ssl context." NORM );
			BIO_s_coroAsync ( true );
			SslFreeCtx ( pSslCtx );
		});
		SSL_CTX_set_verify ( pSslCtx, SSL_VERIFY_NONE, nullptr );

		// shedule callback for final shutdown.
		searchd::AddShutdownCb ( [pRefCtx = pSslCtx] {
			sphLogDebugv ( BACKN "~~ Shutdowncb called." NORM );
			pSslCtx = nullptr;
			// pRefCtx will be also deleted going out of scope
		} );
	}
	return pSslCtx;
}

static SmartSSL_CTX_t GetReadySslCtx ()
{
	if ( !IsKeysSet ())
		return SmartSSL_CTX_t ( nullptr );

	auto pCtx = GetSslCtx ();
	if ( !pCtx )
		return pCtx;

	static bool bKeysLoaded = false;

	if ( !bKeysLoaded && SetGlobalKeys ( pCtx ))
		bKeysLoaded = true;

	if ( !bKeysLoaded )
		return SmartSSL_CTX_t ( nullptr );

	return pCtx;
}

// is global SSL context created and keys set
bool CheckWeCanUseSSL ()
{
	static bool bCheckPerformed = false; // to check only once
	static bool bWeCanUseSSL;

	if ( bCheckPerformed )
		return bWeCanUseSSL;

	bCheckPerformed = true;
	bWeCanUseSSL = ( GetReadySslCtx ()!=nullptr );
	return bWeCanUseSSL;
}

// translates AsyncNetBuffer_c to openSSL BIO calls.
class BioAsyncNetAdapter_c
{
	AsyncNetBufferPtr_c m_pBackend;
	NetGenericOutputBuffer_c& m_tOut;
	AsyncNetInputBuffer_c& m_tIn;

public:
	explicit BioAsyncNetAdapter_c ( AsyncNetBufferPtr_c pSource )
		: m_pBackend ( std::move (pSource) )
		, m_tOut ( m_pBackend->Out() )
		, m_tIn ( m_pBackend->In() )
	{}

	int BioRead ( char * pBuf, int iLen )
	{
		sphLogDebugv ( BACK "<< BioBackRead (%p) for %p, %d, in buf %d" NORM, this, pBuf, iLen, m_tIn.HasBytes () );
		if ( !pBuf || iLen<=0 )
			return 0;
		if ( !m_tIn.ReadFrom ( iLen ))
			iLen = -1;
		auto dBlob = m_tIn.PopTail ( iLen );
		if ( IsNull ( dBlob ))
			return 0;

		memcpy ( pBuf, dBlob.first, dBlob.second );
		return dBlob.second;
	}

	int BioWrite ( const char * pBuf, int iLen )
	{
		sphLogDebugv ( BACK ">> BioBackWrite (%p) for %p, %d" NORM, this, pBuf, iLen );
		if ( !pBuf || iLen<=0 )
			return 0;
		m_tOut.SendBytes ( pBuf, iLen );
		return iLen;
	}

	long BioCtrl ( int iCmd, long iNum, void * pPtr)
	{
		long iRes = 0;
		switch ( iCmd )
		{
		case BIO_CTRL_DGRAM_SET_RECV_TIMEOUT: // BIO_CTRL_DGRAM* used for convenience, as something named 'TIMEOUT'
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) set recv tm %lds" NORM, this, iNum );
			m_tIn.SetTimeoutUS ( iNum );
			iRes = 1;
			break;
		case BIO_CTRL_DGRAM_GET_RECV_TIMEOUT:
			iRes = m_tIn.GetTimeoutUS();
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) get recv tm %lds" NORM, this, iRes );
			break;
		case BIO_CTRL_DGRAM_SET_SEND_TIMEOUT:
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) set send tm %lds" NORM, this, iNum );
			m_tOut.SetWTimeoutUS ( iNum );
			iRes = 1;
			break;
		case BIO_CTRL_DGRAM_GET_SEND_TIMEOUT:
			iRes = m_tOut.GetWTimeoutUS();
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) get recv tm %lds" NORM, this, iRes );
			break;
		case BIO_CTRL_FLUSH:
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) flush" NORM, this );
			m_tOut.Flush();
			iRes = 1;
			break;
		case BIO_CTRL_PENDING:
			iRes = Max (0, m_tIn.HasBytes () );
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) read pending, has %ld" NORM, this, iRes );
			break;
		case BIO_CTRL_EOF:
			iRes = m_tIn.GetError() ? 1 : 0;
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) eof, is %ld" NORM, this, iRes );
			break;
		case BIO_CTRL_WPENDING:
			iRes = m_tOut.GetSentCount ();
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) write pending, has %ld" NORM, this, iRes );
			break;
		case BIO_CTRL_PUSH:
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) push %p, ignore" NORM, this, pPtr );
			break;
		case BIO_CTRL_POP:
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) pop %p, ignore" NORM, this, pPtr );
			break;
		default:
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) with %d, %ld, %p" NORM, this, iCmd, iNum, pPtr );
		}

		return iRes;
	}
};

#if ( OPENSSL_VERSION_NUMBER < 0x1010000fL)

#define BIO_set_shutdown(pBio,CODE) pBio->shutdown = CODE
#define BIO_get_shutdown(pBio) pBio->shutdown
#define BIO_set_init(pBio,CODE) pBio->init = CODE
#define BIO_set_data(pBio,DATA) pBio->ptr = DATA
#define BIO_get_data(pBio) pBio->ptr
#define BIO_meth_free(pMethod) delete pMethod
#define BIO_get_new_index() (24)
#define BIO_meth_set_create(pMethod,pFn) pMethod->create = pFn
#define BIO_meth_set_destroy(pMethod, pFn ) pMethod->destroy = pFn
#define BIO_meth_set_read(pMethod,pFn) pMethod->bread = pFn
#define BIO_meth_set_write(pMethod,pFn) pMethod->bwrite = pFn
#define BIO_meth_set_ctrl(pMethod,pFn) pMethod->ctrl = pFn

inline static BIO_METHOD * BIO_meth_new ( int iType, const char * szName )
{
	auto pMethod = new BIO_METHOD;
	memset ( pMethod, 0, sizeof ( BIO_METHOD ) );
	pMethod->type = iType;
	pMethod->name = szName;
	return pMethod;
}

#endif // OPENSSL_VERSON_NUMBER dependent code

static int MyBioCreate ( BIO * pBio )
{
	sphLogDebugv ( BACKN "~~ MyBioCreate called with %p" NORM, pBio );
	BIO_set_shutdown ( pBio, BIO_CLOSE );
	BIO_set_init ( pBio, 0 ); // without it write, read will not be called
	BIO_set_data ( pBio, nullptr );
	BIO_clear_flags ( pBio, ~0 );
	return 1;
}

static int MyBioDestroy ( BIO * pBio )
{
	sphLogDebugv ( BACKN "~~ MyBioDestroy called with %p" NORM, pBio );
	if ( !pBio )
		return 0;
	auto pAdapter = ( BioAsyncNetAdapter_c*) BIO_get_data ( pBio );
	assert ( pAdapter );
	SafeDelete ( pAdapter );
	if ( BIO_get_shutdown ( pBio ) )
	{
		BIO_clear_flags ( pBio, ~0 );
		BIO_set_init ( pBio, 0 );
	}
	return 1;
}

static int MyBioWrite ( BIO * pBio, const char * cBuf, int iNum )
{
	auto pAdapter = (BioAsyncNetAdapter_c *) BIO_get_data ( pBio );
	assert ( pAdapter );
	return pAdapter->BioWrite ( cBuf, iNum );
}

static int MyBioRead ( BIO * pBio, char * cBuf, int iNum )
{
	auto pAdapter = (BioAsyncNetAdapter_c *) BIO_get_data ( pBio );
	assert ( pAdapter );
	return pAdapter->BioRead ( cBuf, iNum );
}

static long MyBioCtrl ( BIO * pBio, int iCmd, long iNum, void * pPtr )
{
	auto pAdapter = (BioAsyncNetAdapter_c *) BIO_get_data ( pBio );
	assert ( pAdapter );
	return pAdapter->BioCtrl ( iCmd, iNum, pPtr );
}

static BIO_METHOD * BIO_s_coroAsync ( bool bDestroy )
{
	static BIO_METHOD * pMethod = nullptr;
	if ( bDestroy && pMethod )
	{
		sphLogDebugv ( FRONT "~~ BIO_s_coroAsync (%d)" NORM, !!bDestroy );
		BIO_meth_free ( pMethod );
		pMethod = nullptr;
	} else if ( !bDestroy && !pMethod )
	{
		sphLogDebugv ( FRONT "~~ BIO_s_coroAsync (%d)" NORM, !!bDestroy );
		pMethod = BIO_meth_new ( BIO_get_new_index () | BIO_TYPE_DESCRIPTOR | BIO_TYPE_SOURCE_SINK, "async sock coroutine" );
		BIO_meth_set_create ( pMethod, MyBioCreate );
		BIO_meth_set_destroy ( pMethod, MyBioDestroy );
		BIO_meth_set_read ( pMethod, MyBioRead );
		BIO_meth_set_write ( pMethod, MyBioWrite );
		BIO_meth_set_ctrl ( pMethod, MyBioCtrl );
	}
	return pMethod;
}

static BIO * BIO_new_coroAsync ( AsyncNetBufferPtr_c pSource )
{
	auto pBio = BIO_new ( BIO_s_coroAsync ());
	BIO_set_data ( pBio, new BioAsyncNetAdapter_c ( std::move ( pSource ) ) );
	BIO_set_init ( pBio, 1 );
	return pBio;
}

using BIOPtr_c = SharedPtrCustom_t<BIO *>;

class AsyncSSBufferedSocket_c final : public AsyncNetBuffer_c, protected AsyncNetInputBuffer_c, protected NetGenericOutputBuffer_c
{
	BIOPtr_c m_pSslBackend;

public:
	explicit AsyncSSBufferedSocket_c ( BIOPtr_c pSslFrontend )
		: m_pSslBackend ( std::move ( pSslFrontend ) )
	{}

	AsyncNetInputBuffer_c & In () final { return *this; }
	NetGenericOutputBuffer_c & Out () final { return *this; }

	void SendBuffer ( const VecTraits_T<BYTE> & dData ) final
	{
		assert ( m_pSslBackend );
		CSphScopedProfile tProf ( m_pProfile, SPH_QSTATE_NET_WRITE );
		sphLogDebugv ( FRONT "~~ BioFrontWrite (%p) %d bytes" NORM,
				(BIO*)m_pSslBackend, dData.GetLength () );
		int iSent = 0;
		if ( !dData.IsEmpty ())
			iSent = BIO_write ( m_pSslBackend, dData.begin (), dData.GetLength () );
		auto iRes = BIO_flush ( m_pSslBackend );
		sphLogDebugv ( FRONT ">> BioFrontWrite (%p) done (%d) %d bytes of %d" NORM,
				(BIO*)m_pSslBackend, iRes, iSent, dData.GetLength () );
	}

	int ReadFromBackend ( int iNeed, int iHaveSpace, bool ) final
	{
		assert ( iNeed <= iHaveSpace );
		auto pBuf = AddN(0);

		int iGotTotal = 0;
		while ( iNeed>0 )
		{
			auto iPending = BIO_pending( m_pSslBackend );
			if ( !iPending && BIO_eof ( m_pSslBackend ))
			{
				sphLogDebugv ( FRONT "~~ BIO_eof on frontend. Bailing" NORM );
				return -1;
			}
			auto iCanRead = Max ( iNeed, Min ( iHaveSpace, iPending ));
			sphLogDebugv ( FRONT "~~ BioReadFront %d..%d, can %d, pending %d" NORM,
					iNeed, iHaveSpace, iCanRead, iPending );
			int iGot = BIO_read ( m_pSslBackend, pBuf, iCanRead );
			sphLogDebugv ( FRONT "<< BioReadFront (%p) done %d from %d..%d" NORM,
					(BIO *) m_pSslBackend, iGot, iNeed, iHaveSpace );
			pBuf += iGot;
			iGotTotal += iGot;
			iNeed -= iGot;
			iHaveSpace -= iGot;
			if ( !iGot )
			{
				sphLogDebugv ( FRONT "<< BioReadFront (%p) breaking on %d" NORM,
						(BIO *) m_pSslBackend, iGotTotal );
				break;
			}
		}
		return iGotTotal;
	}

	void SetWTimeoutUS ( int64_t iTimeoutUS ) final
	{
		BIO_ctrl ( m_pSslBackend, BIO_CTRL_DGRAM_SET_SEND_TIMEOUT, iTimeoutUS, nullptr );
	};

	int64_t GetWTimeoutUS () const final
	{
		return BIO_ctrl ( m_pSslBackend, BIO_CTRL_DGRAM_GET_SEND_TIMEOUT, 0, nullptr );
	}

	void SetTimeoutUS ( int64_t iTimeoutUS ) final
	{
		BIO_ctrl ( m_pSslBackend, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, iTimeoutUS, nullptr );
	}

	int64_t GetTimeoutUS () const final
	{
		return BIO_ctrl ( m_pSslBackend, BIO_CTRL_DGRAM_GET_RECV_TIMEOUT, 0, nullptr );
	}
};

bool MakeSecureLayer ( AsyncNetBufferPtr_c& pSource )
{
	auto pCtx = GetReadySslCtx();
	if ( !pCtx )
		return false;

	BIOPtr_c pFrontEnd ( BIO_new_ssl ( pCtx, 0 ), [pCtx] (BIO * pBio) {
		BIO_free_all ( pBio );
	} );
	SSL * pSSL = nullptr;

	BIO_get_ssl ( pFrontEnd, &pSSL );
	SSL_set_mode ( pSSL, SSL_MODE_AUTO_RETRY );
	BIO_push ( pFrontEnd, BIO_new_coroAsync ( std::move ( pSource ) ) );
	pSource = new AsyncSSBufferedSocket_c ( std::move ( pFrontEnd ) );
	return true;
}

#endif