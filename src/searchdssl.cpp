//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "searchdssl.h"

static CSphString g_sSslCert;
static CSphString g_sSslKey;
static CSphString g_sSslCaFileName;

static bool g_bSslForced = false;

void ForceSsl()
{
	g_bSslForced = true;
}

#if WITH_SSL
#include "sphinxstd.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/x509v3.h>

#include <memory>

static BIO_METHOD * BIO_s_coroAsync ( bool bDestroy = false );

static int fnSslError ( const char * pStr, size_t iLen, void * pError )
{
	// trim line ending from string end
	while ( iLen && sphIsSpace ( pStr[iLen-1] ) )
		iLen--;

	if ( pError )
		( (CSphString *)pError )->SetSprintf ( "%.*s", (int)iLen, pStr );
	else
		sphWarning ( "%.*s", (int)iLen, pStr );

	return 1;
}

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

static bool ReadSslData ( CSphString & sBuf, const CSphString & sFileName, CSphString & sError )
{
	CSphAutofile tRd;
	int iFD = tRd.Open ( sFileName, SPH_O_READ, sError );
	if ( iFD<0 )
		return false;

	int iSize = tRd.GetSize();
	if ( iSize<0 )
	{
		sError.SetSprintf ( "%s invalid size %d", sFileName.cstr(), iSize );
		return false;
	}
	sBuf.Reserve ( iSize + CSphString::GetGap() );

	int iGot = sphReadThrottled ( iFD, (char *)sBuf.cstr(), iSize );
	if ( iGot!=iSize )
		return false;

	memset ( (char *)sBuf.cstr() + iSize, 0, CSphString::GetGap() );
	return true;
}

void SetServerSSLKeys ( const CSphString & sSslCert,  const CSphString & sSslKey,  const CSphString & sSslCa )
{
	CSphString sError;

	if ( !sSslCert.IsEmpty() && !ReadSslData ( g_sSslCert, sSslCert, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return;
	}

	if ( !sSslKey.IsEmpty() && !ReadSslData ( g_sSslKey, sSslKey, sError ) )
	{
		sphWarning ( "%s", sError.cstr() );
		return;
	}

	g_sSslCaFileName = sSslCa;
}

static bool IsKeysSet()
{
	return !( g_sSslCert.IsEmpty () && g_sSslKey.IsEmpty () && g_sSslCaFileName.IsEmpty ());
}

// set SSL key, certificate and ca-certificate to global SSL context
static bool SetGlobalKeys ( SSL_CTX * pCtx, CSphString * pError )
{
	if ( !(IsKeysSet()) )
		return false;

	if ( !g_sSslCert.IsEmpty () && SSL_CTX_use_certificate_file ( pCtx, g_sSslCert.cstr (), SSL_FILETYPE_PEM )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, pError );
		return false;
	}

	if ( !g_sSslKey.IsEmpty () && SSL_CTX_use_PrivateKey_file ( pCtx, g_sSslKey.cstr (), SSL_FILETYPE_PEM )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, pError );
		return false;
	}

	if ( !g_sSslCaFileName.IsEmpty () && SSL_CTX_load_verify_locations ( pCtx, g_sSslCaFileName.cstr(), nullptr )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, pError );
		return false;
	}

	// check key and certificate file match
	if ( SSL_CTX_check_private_key( pCtx )!=1 )
	{
		ERR_print_errors_cb ( &fnSslError, pError );
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

	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	ERR_free_strings();
}

using SmartSSL_CTX_t = SharedPtrCustom_t<SSL_CTX>;

// init SSL library and global context by demand
static SmartSSL_CTX_t GetSslCtx ()
{
	static SmartSSL_CTX_t pSslCtx;
	if ( !pSslCtx )
	{
		SSL_load_error_strings();
		SSL_library_init();

		const SSL_METHOD * pMode = TLS_server_method();
		pSslCtx = SmartSSL_CTX_t ( SSL_CTX_new ( pMode ), [] ( SSL_CTX * pCtx )
		{
			sphLogDebugv ( BACKN "~~ Releasing ssl context." NORM );
			BIO_s_coroAsync ( true );
			SslFreeCtx ( pCtx );
		});
		SSL_CTX_set_verify ( pSslCtx, SSL_VERIFY_NONE, nullptr );

		// schedule callback for final shutdown.
		searchd::AddShutdownCb ( [pRefCtx = pSslCtx] {
			sphLogDebugv ( BACKN "~~ Shutdowncb called." NORM );
			pSslCtx = nullptr;
			// pRefCtx will be also deleted going out of scope
		} );
	}
	return pSslCtx;
}

static bool AddCertExtension ( X509 * pCert, int iNid, char * sVal )
{
	X509_EXTENSION * pExt;
	X509V3_CTX tCtx;
	X509V3_set_ctx_nodb ( &tCtx );
	X509V3_set_ctx ( &tCtx, pCert, pCert, nullptr, nullptr, 0 );
	pExt = X509V3_EXT_conf_nid ( nullptr, &tCtx, iNid, sVal );
	if ( !pExt )
		return false;

	X509_add_ext ( pCert, pExt, -1 );
	X509_EXTENSION_free ( pExt );
	return true;
}

static bool GenerateInMemorySslKeys ( CSphString & sSslCert, CSphString & sSslKey, CSphString * pError )
{
	// EVP_PKEY for the private key
	std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> pKey( EVP_PKEY_new(), EVP_PKEY_free );
	if ( !pKey )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	// generate RSA key
	std::unique_ptr<RSA, decltype(&RSA_free)> pRsa ( RSA_new(), RSA_free );
	if ( !pRsa )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	std::unique_ptr<BIGNUM, decltype(&BN_free)> pBne ( BN_new(), BN_free );
	if ( !pBne || !BN_set_word ( pBne.get(), RSA_F4 ) )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	if ( !RSA_generate_key_ex ( pRsa.get(), 2048, pBne.get(), nullptr ) )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	if ( !EVP_PKEY_assign_RSA ( pKey.get(), RSAPrivateKey_dup ( pRsa.get() ) ) )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	// X509 cert
	std::unique_ptr<X509, decltype(&X509_free)> pCert ( X509_new(), X509_free );
	if ( !pCert )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	X509_set_version ( pCert.get(), 2 );
	// timestamp as serial
	ASN1_INTEGER_set ( X509_get_serialNumber ( pCert.get() ), sphMicroTimer() );
	// cert valid from now
	X509_gmtime_adj ( X509_getm_notBefore ( pCert.get() ), 0 );
	// cert valid for 1 year
	X509_gmtime_adj ( X509_getm_notAfter ( pCert.get()), 31536000L );
	X509_set_pubkey ( pCert.get(), pKey.get() );

	// subject and issuer name (self-signed)
	X509_NAME * pName = X509_get_subject_name ( pCert.get() );
	X509_NAME_add_entry_by_txt ( pName, "CN", MBSTRING_ASC, (const BYTE *)"ManticoreSoftwareLTD", -1, -1, 0);
	X509_set_issuer_name ( pCert.get(), pName );

	// extensions for TLS server certificate
	if ( !AddCertExtension ( pCert.get(), NID_subject_alt_name, "IP:127.0.0.1,DNS:localhost" ) || !AddCertExtension ( pCert.get(), NID_basic_constraints, "critical,CA:FALSE" ) || !AddCertExtension ( pCert.get(), NID_key_usage, "critical,digitalSignature,keyEncipherment" ) )
	{
        ERR_print_errors_cb ( &fnSslError, pError );
        return false;
	}

	// sign the cert
	if ( !X509_sign ( pCert.get(), pKey.get(), EVP_sha256() ) )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	// convert key and cert to memory PEM string
	std::unique_ptr<BIO, decltype(&BIO_free)> pKeyBio ( BIO_new ( BIO_s_mem() ), BIO_free );
	if ( !PEM_write_bio_PrivateKey ( pKeyBio.get(), pKey.get(), nullptr, nullptr, 0, nullptr, nullptr ) )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	char * pKeyData = nullptr;
	int iKeyLen = BIO_get_mem_data ( pKeyBio.get(), &pKeyData );

	std::unique_ptr<BIO, decltype(&BIO_free)> pCertBio ( BIO_new ( BIO_s_mem() ), BIO_free );
	if ( !PEM_write_bio_X509 ( pCertBio.get(), pCert.get() ) )
	{
		ERR_print_errors_cb ( &fnSslError, pError );;
		return false;
	}

	char * pCertData = nullptr;
	long iCertLen = BIO_get_mem_data ( pCertBio.get(), &pCertData );

	sSslKey.SetBinary ( pKeyData, iKeyLen );
	sSslCert.SetBinary ( pCertData, iCertLen );

	return true;
}

static bool SetKeysFromPem ( const CSphString & sSslCert, const CSphString & sSslKey, const CSphString & sSslCaFileName, SSL_CTX * pCtx, CSphString * pError )
{
	// load cert from PEM string
	std::unique_ptr<BIO, decltype(&BIO_free)> pCertBio ( BIO_new_mem_buf ( sSslCert.cstr(), sSslCert.Length() ), BIO_free );
	if ( !pCertBio )
	{
		ERR_print_errors_cb ( &fnSslError, pError );
		return false;
	}

	std::unique_ptr<X509, decltype(&X509_free)> pCertX509 ( PEM_read_bio_X509 ( pCertBio.get(), nullptr, nullptr, nullptr ), X509_free );
	if ( !pCertX509 )
	{
		ERR_print_errors_cb ( &fnSslError, pError );
		return false;
	}

	if ( SSL_CTX_use_certificate ( pCtx, pCertX509.get() )<= 0 )
	{
		ERR_print_errors_cb(&fnSslError, pError);
		return false;
	}

	// load private key from PEM string
	std::unique_ptr<BIO, decltype ( &BIO_free ) > pKeyBio ( BIO_new_mem_buf ( sSslKey.cstr(), sSslKey.Length() ), BIO_free );
	if ( !pKeyBio || SSL_CTX_use_PrivateKey ( pCtx, PEM_read_bio_PrivateKey ( pKeyBio.get(), nullptr, nullptr, nullptr ) )<=0 )
	{
		ERR_print_errors_cb( &fnSslError, pError );
		return false;
	}
	
	// load CA file by the file path
	if ( !sSslCaFileName.IsEmpty() && SSL_CTX_load_verify_locations ( pCtx, sSslCaFileName.cstr(), nullptr )<=0 )
	{
		ERR_print_errors_cb ( &fnSslError, pError );
		return false;
	}

	// check key and cert match
	if ( SSL_CTX_check_private_key ( pCtx )!=1 )
	{
		ERR_print_errors_cb(&fnSslError, pError);
		return false;
	}

	return true;
}

static SmartSSL_CTX_t GetReadySslCtx ( CSphString * pError=nullptr )
{
	auto pCtx = GetSslCtx ();
	if ( !pCtx )
		return pCtx;

	static bool bKeysSet = false;
	if ( bKeysSet )
		return pCtx;

	bKeysSet = true;
	bool bHasKeys = ( !g_sSslCert.IsEmpty() && !g_sSslKey.IsEmpty() );

	// keys provided by the config
	if ( bHasKeys )
	{
		if ( SetKeysFromPem ( g_sSslCert, g_sSslKey, g_sSslCaFileName, pCtx, pError ) )
			return pCtx;

		// if load keys fails and SSL is forced (for auth and Buddy communication) - fatal error
		if ( g_bSslForced && pError )
			pError->SetSprintf ( "failed to load configured SSL keys: %s", pError->cstr() );

		return SmartSSL_CTX_t ( nullptr );
	}

	// no keys in config but SSL is required (for auth and Buddy communication)
	if ( g_bSslForced )
	{
		if ( GenerateInMemorySslKeys ( g_sSslCert, g_sSslKey, pError ) && SetKeysFromPem ( g_sSslCert, g_sSslKey, g_sSslCaFileName, pCtx, pError ) )
		{
			sphLogDebug ( "generated in-memory SSL certificate" );
			//sphSafeInfoWrite ( GetActiveLogFD(), g_sSslCert.cstr(), g_sSslCert.Length() );				
			return pCtx;
		}
		else
		{
			if ( pError )
				pError->SetSprintf ( "failed to generate in-memory SSL keys: %s", pError->cstr() );
		}
		return SmartSSL_CTX_t ( nullptr );
	}

	// no keys provided and SSL is not required - nothing to do
	return SmartSSL_CTX_t ( nullptr );
}

// is global SSL context created and keys set
bool CheckWeCanUseSSL ( CSphString * pError )
{
	static bool bCheckPerformed = false; // to check only once
	static bool bWeCanUseSSL = false;

	if ( bCheckPerformed )
		return bWeCanUseSSL;

	bCheckPerformed = true;
	bWeCanUseSSL = ( GetReadySslCtx ( pError )!=nullptr );
	return bWeCanUseSSL;
}

// translates AsyncNetBuffer_c to openSSL BIO calls.
class BioAsyncNetAdapter_c
{
	std::unique_ptr<AsyncNetBuffer_c> m_pBackend;
	GenericOutputBuffer_c& m_tOut;
	AsyncNetInputBuffer_c& m_tIn;

public:
	explicit BioAsyncNetAdapter_c ( std::unique_ptr<AsyncNetBuffer_c> pSource )
		: m_pBackend ( std::move (pSource) )
		, m_tOut ( *m_pBackend )
		, m_tIn ( *m_pBackend )
	{}

	int BioRead ( char * pBuf, int iLen )
	{
		sphLogDebugv ( BACK "<< BioBackRead (%p) for %p, %d, in buf %d" NORM, this, pBuf, iLen, m_tIn.HasBytes () );
		if ( !pBuf || iLen<=0 )
			return 0;
		if ( !m_tIn.ReadFrom ( iLen ))
			iLen = -1;
		auto dBlob = m_tIn.PopTail ( iLen );
		if ( IsEmpty ( dBlob ))
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
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) set recv tm %lds" NORM, this, long (iNum / S2US) );
			m_tIn.SetTimeoutUS ( iNum );
			iRes = 1;
			break;
		case BIO_CTRL_DGRAM_GET_RECV_TIMEOUT:
			iRes = (long)m_tIn.GetTimeoutUS();
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) get recv tm %lds" NORM, this, long (iRes / S2US) );
			break;
		case BIO_CTRL_DGRAM_SET_SEND_TIMEOUT:
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) set send tm %lds" NORM, this, long (iNum / S2US) );
			m_tOut.SetWTimeoutUS ( iNum );
			iRes = 1;
			break;
		case BIO_CTRL_DGRAM_GET_SEND_TIMEOUT:
			iRes = (long)m_tOut.GetWTimeoutUS();
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) get recv tm %lds" NORM, this, long (iRes / S2US) );
			break;
		case BIO_CTRL_FLUSH:
			sphLogDebugv ( BACKN "~~ BioBackCtrl (%p) flush" NORM, this );
			iRes = m_tOut.Flush () ? 1 : -1;
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

static BIO * BIO_new_coroAsync ( std::unique_ptr<AsyncNetBuffer_c> pSource )
{
	auto pBio = BIO_new ( BIO_s_coroAsync ());
	BIO_set_data ( pBio, new BioAsyncNetAdapter_c ( std::move ( pSource ) ) );
	BIO_set_init ( pBio, 1 );
	return pBio;
}

using BIOPtr_c = SharedPtrCustom_t<BIO>;

class AsyncSSBufferedSocket_c final : public AsyncNetBuffer_c
{
	BIOPtr_c m_pSslBackend;

	int64_t	m_iSendTotal = 0;
	int64_t m_iReceivedTotal = 0;

	bool SendBuffer ( const VecTraits_T<BYTE> & dData ) final
	{
		assert ( m_pSslBackend );
		CSphScopedProfile tProf ( m_pProfile, SPH_QSTATE_NET_WRITE );
		sphLogDebugv ( FRONT "~~ BioFrontWrite (%p) %d bytes" NORM, (BIO*)m_pSslBackend, dData.GetLength () );
		int iSent = 0;
		if ( !dData.IsEmpty ())
			iSent = BIO_write ( m_pSslBackend, dData.begin (), dData.GetLength () );
		auto iRes = BIO_flush ( m_pSslBackend );
		sphLogDebugv ( FRONT ">> BioFrontWrite (%p) done (%d) %d bytes of %d" NORM, (BIO*)m_pSslBackend, iRes, iSent, dData.GetLength () );
		m_iSendTotal += dData.GetLength();
		return ( iRes>0 );
	}

	int ReadFromBackend ( int iNeed, int iSpace, bool ) final
	{
		assert ( iNeed <= iSpace );
		auto dBuf = AllocateBuffer ( iSpace );
		int iHaveSpace = dBuf.GetLength();
		auto pBuf = dBuf.begin();

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
		m_iReceivedTotal += iGotTotal;
		return iGotTotal;
	}

public:
	explicit AsyncSSBufferedSocket_c ( BIOPtr_c pSslFrontend )
		: m_pSslBackend ( std::move ( pSslFrontend ) )
	{}

	void SetWTimeoutUS ( int64_t iTimeoutUS ) final
	{
		BIO_ctrl ( m_pSslBackend, BIO_CTRL_DGRAM_SET_SEND_TIMEOUT, (long)iTimeoutUS, nullptr );
	}

	int64_t GetWTimeoutUS () const final
	{
		return BIO_ctrl ( m_pSslBackend, BIO_CTRL_DGRAM_GET_SEND_TIMEOUT, 0, nullptr );
	}

	void SetTimeoutUS ( int64_t iTimeoutUS ) final
	{
		BIO_ctrl ( m_pSslBackend, BIO_CTRL_DGRAM_SET_RECV_TIMEOUT, (long)iTimeoutUS, nullptr );
	}

	int64_t GetTimeoutUS () const final
	{
		return BIO_ctrl ( m_pSslBackend, BIO_CTRL_DGRAM_GET_RECV_TIMEOUT, 0, nullptr );
	}

	int64_t GetTotalSent () const final
	{
		return m_iSendTotal;
	}

	int64_t GetTotalReceived() const final
	{
		return m_iReceivedTotal;
	}
};

bool MakeSecureLayer ( std::unique_ptr<AsyncNetBuffer_c>& pSource )
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
	pSource = std::make_unique<AsyncSSBufferedSocket_c> ( std::move ( pFrontEnd ) );
	return true;
}

static bool SslError ( CSphString & sError, const char * sEmptyMsg=nullptr )
{
	int iCode = 0;
	char sBuf[256];

	while ( ( iCode=ERR_get_error() )!=0 )
	{
		ERR_error_string_n ( iCode, sBuf, sizeof ( sBuf ) );
		if ( !sError.IsEmpty() )
			sError.SetSprintf ( "%s; %s", sError.cstr(), sBuf );
		else
			sError.SetSprintf ( "%s", sBuf );
	}
	if ( sError.IsEmpty() )
	{
		if ( sEmptyMsg )
			sError = sEmptyMsg;
		else
			sError = "OpenSSL error (no details)";
	}

	return false;
}

static const int GCM_TAG_LEN = 16;
static const BYTE ALGO_VER = 1;
static const int AES_GCM_KEY_SIZE = 32;

#include <openssl/rand.h>
bool EncryptGCM ( const VecTraits_T<BYTE> & dRawData, const CSphString & sUser, const VecTraits_T<BYTE> & dEncKey, const BYTE * pSrcNonce, int iHeadGap, CSphVector<BYTE> & dDstData, CSphString & sError )
{
	assert ( dEncKey.GetLength()==AES_GCM_KEY_SIZE );
	assert ( dRawData.GetLength() );

	int iNameLen = sUser.Length();
	dDstData.Reserve ( iHeadGap + 1 + iNameLen + 4 + GCM_NONCE_LEN + GCM_TAG_LEN + dRawData.GetLength() );

	{
		// dst got stealed
		ISphOutputBuffer tWr ( dDstData );
		tWr.Rewind ( iHeadGap );
		tWr.SendByte ( ALGO_VER );
		tWr.SendString ( Str_t ( sUser.cstr(), iNameLen ) );
		// swap back dst
		tWr.m_dBuf.SwapData ( dDstData );
	}

	BYTE * pNonce = dDstData.AddN ( GCM_NONCE_LEN + GCM_TAG_LEN + dRawData.GetLength() );
	BYTE * pTag = pNonce + GCM_NONCE_LEN;
	BYTE * pCipher = pTag + GCM_TAG_LEN;

	memcpy ( pNonce, pSrcNonce, GCM_NONCE_LEN );
	
	EVP_CIPHER_CTX * pCtx = EVP_CIPHER_CTX_new();
	if ( !pCtx )
		return SslError ( sError );

	AT_SCOPE_EXIT([&pCtx] { if ( pCtx ) EVP_CIPHER_CTX_free(pCtx); } );

	if ( !EVP_EncryptInit_ex ( pCtx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr ) )
		return SslError ( sError );
	
	if ( !EVP_CIPHER_CTX_ctrl ( pCtx, EVP_CTRL_GCM_SET_IVLEN, GCM_NONCE_LEN, nullptr ) )
		return SslError ( sError );
	
	if ( !EVP_EncryptInit_ex ( pCtx, nullptr, nullptr, dEncKey.Begin(), pNonce ) )
		return SslError ( sError );

	// AAD part authenticated but not encrypted goes prior to the data
	if ( iNameLen )
	{
		int iAadLen = 0;
		if ( !EVP_EncryptUpdate ( pCtx, nullptr, &iAadLen, (const BYTE *)sUser.cstr(), iNameLen ) )
			return SslError ( sError );
	}

	int iLen = 0;
	if ( !EVP_EncryptUpdate ( pCtx, pCipher, &iLen, dRawData.Begin(), dRawData.GetLength() ) )
		return SslError ( sError );

	int iCiphLen = iLen;
	// GCM do not pad but lets call for correctness
	if ( !EVP_EncryptFinal_ex ( pCtx, pCipher + iLen, &iLen ) )
		return SslError ( sError );
	
	iCiphLen += iLen;
	
	if ( !EVP_CIPHER_CTX_ctrl ( pCtx, EVP_CTRL_GCM_GET_TAG, GCM_TAG_LEN, pTag ) )
		return SslError ( sError );

	return true;
}

bool DecryptGCM ( const VecTraits_T<BYTE> & dEncryptedData, CSphVector<BYTE> & dDstData, CSphString & sUser, GcmUserKey_i & tKey, CSphString & sError )
{
	const int iMinExtraLen = ( GCM_NONCE_LEN + GCM_TAG_LEN + 1 + 4 );
	if ( dEncryptedData.GetLength()<iMinExtraLen )
	{
		sError.SetSprintf ( "invalid encrypted length, got %d, expected %d", dEncryptedData.GetLength(), iMinExtraLen );
		return false;
	}

	const BYTE * pBase = nullptr;
	int iGotVer = 0;
	{
		InputBuffer_c tRd ( dEncryptedData );
		iGotVer = tRd.GetByte();
		sUser = tRd.GetString();
		pBase = tRd.GetBufferPtr();

		if ( tRd.GetError() )
		{
			sError = tRd.GetErrorMessage();
			return false;
		}
	}

	const BYTE * pNonce = pBase;
	const BYTE * pTag = pNonce + GCM_NONCE_LEN;
	const BYTE * pCipher = pTag + GCM_TAG_LEN;
	const int iCipherLen = dEncryptedData.End() - pCipher;
	if ( iCipherLen<0 )
	{
		sError.SetSprintf ( "invalid encrypted data length, got %d, encrypted length %d", iCipherLen, dEncryptedData.GetLength() );
		return false;
	}

	if ( iGotVer!=ALGO_VER )
	{
		sError.SetSprintf ( "invalid encrypted version, got %d, expected %d", iGotVer, int( ALGO_VER ) );
		return false;
	}

	if ( !tKey.ValidateNonceReplay ( pNonce, sError ) )
		return false;

	auto dDecryptionKey = tKey.Get ( sUser, sError );
	if ( !dDecryptionKey )
		return false;

	assert ( dDecryptionKey->GetLength()==AES_GCM_KEY_SIZE );

	EVP_CIPHER_CTX * pCtx = EVP_CIPHER_CTX_new();
	if ( !pCtx )
		return SslError ( sError );

	AT_SCOPE_EXIT([&pCtx] { if ( pCtx ) EVP_CIPHER_CTX_free(pCtx); } );

	if ( !EVP_DecryptInit_ex ( pCtx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr ) )
		return SslError ( sError );
	
	if ( !EVP_CIPHER_CTX_ctrl ( pCtx, EVP_CTRL_GCM_SET_IVLEN, GCM_NONCE_LEN, nullptr ) )
		return SslError ( sError );
	
	if ( !EVP_DecryptInit_ex ( pCtx, nullptr, nullptr, dDecryptionKey->Begin(), pNonce ) )
		return SslError ( sError );

	// AAD part must be identical to encryption side
	if ( !sUser.IsEmpty())
	{
		int iAadLen = 0;
		if ( !EVP_DecryptUpdate ( pCtx, nullptr, &iAadLen, (const BYTE *)sUser.cstr(), (int)sUser.Length() ) )
			return SslError ( sError );
	}

	dDstData.Resize ( iCipherLen );
	int iLen = 0;
	if ( !EVP_DecryptUpdate ( pCtx, dDstData.Begin(), &iLen, pCipher, iCipherLen ) )
		return SslError ( sError );

	if ( !EVP_CIPHER_CTX_ctrl ( pCtx, EVP_CTRL_GCM_SET_TAG, GCM_TAG_LEN, (void *)pTag ) )
		return SslError ( sError );
	
	int iDataLen = iLen;
	int iRes = EVP_DecryptFinal_ex ( pCtx, dDstData.Begin() + iLen, &iLen );
	if ( iRes<=0 )
		return SslError ( sError, "GCM authentication failed (bad tag)" );
	
	iDataLen += iLen;
	dDstData.Resize ( iDataLen );
	return true;
}

bool MakeApiKdf ( const ByteBlob_t & tSalt, const ByteBlob_t & tPwd, CSphFixedVector<BYTE> & dRes, CSphString & sError )
{
	if ( IsEmpty ( tSalt ) )
	{
		sError = "can not generate API key from an empty salt";
		return false;
	}

	if ( IsEmpty ( tPwd ) )
	{
		sError = "can not generate API key from an empty password_hash";
		return false;
	}

	dRes.Reset ( 32 ); // AES-256 key length 32 bytes
	const int PBKDF2_ITER_COUNT = 100000; // high iteration count

	int iRes = PKCS5_PBKDF2_HMAC ( (const char *)tPwd.first, tPwd.second, tSalt.first, tSalt.second, PBKDF2_ITER_COUNT, EVP_sha256(), dRes.GetLength(), dRes.Begin() );
	if ( iRes!=1 )
	{
		SslError ( sError, "PBKDF2 API key derivation from hash failed" );
		return false;
	}

	return true;
}

bool MakeRandBuf ( VecTraits_T<BYTE> & dRes, CSphString & sError )
{
	if ( RAND_bytes ( dRes.Begin(), dRes.GetLength() )==1 )
		return true;
	else
		return SslError ( sError );
}

#else

static bool NoSslSupport ( CSphString & sError )
{
	sError = "daemon built without SSL support";
	return false;
}

bool EncryptGCM ( const VecTraits_T<BYTE> & , const CSphString & , const VecTraits_T<BYTE> & , const BYTE * , int , CSphVector<BYTE> & , CSphString & sError )
{
	return NoSslSupport ( sError );
}

bool DecryptGCM ( const VecTraits_T<BYTE> & , CSphVector<BYTE> & , CSphString & , GcmUserKey_i & , CSphString & sError )
{
	return NoSslSupport ( sError );
}

bool MakeApiKdf ( const ByteBlob_t & , const ByteBlob_t & , CSphFixedVector<BYTE> & , CSphString & sError )
{
	return NoSslSupport ( sError );
}

bool MakeRandBuf ( VecTraits_T<BYTE> & , CSphString & sError )
{
	return NoSslSupport ( sError );
}

#endif

const CSphString & GetSslCert()
{
	return g_sSslCert;
}

const CSphString & GetSslKey()
{
	return g_sSslKey;
}