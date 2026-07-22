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

#include <gtest/gtest.h>

#include "searchdssl.h"

#if WITH_SSL

#include <openssl/bio.h>
#include <openssl/opensslv.h>

#if ( OPENSSL_VERSION_NUMBER < 0x1010000fL )
#define BIO_set_shutdown(pBio,CODE) pBio->shutdown = CODE
#define BIO_get_shutdown(pBio) pBio->shutdown
#define BIO_set_init(pBio,CODE) pBio->init = CODE
#define BIO_set_data(pBio,DATA) pBio->ptr = DATA
#define BIO_get_data(pBio) pBio->ptr
#define BIO_get_new_index() (24)
#define BIO_meth_set_create(pMethod,pFn) pMethod->create = pFn
#define BIO_meth_set_destroy(pMethod,pFn) pMethod->destroy = pFn
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
#endif

struct ScriptedBioState_t
{
	std::vector<int> m_dReads;
	int m_iReadCalls = 0;
	int m_iCopiedBytes = 0;

	explicit ScriptedBioState_t ( std::initializer_list<int> dReads )
		: m_dReads ( dReads )
	{}
};

static int ScriptedBioCreate ( BIO * pBio )
{
	BIO_set_shutdown ( pBio, BIO_CLOSE );
	BIO_set_init ( pBio, 0 );
	BIO_set_data ( pBio, nullptr );
	BIO_clear_flags ( pBio, ~0 );
	return 1;
}

static int ScriptedBioDestroy ( BIO * pBio )
{
	if ( !pBio )
		return 0;

	delete (std::shared_ptr<ScriptedBioState_t> *) BIO_get_data ( pBio );
	BIO_set_data ( pBio, nullptr );
	BIO_set_init ( pBio, 0 );
	BIO_clear_flags ( pBio, ~0 );
	return 1;
}

static int ScriptedBioRead ( BIO * pBio, char * pBuf, int iNum )
{
	auto ppState = (std::shared_ptr<ScriptedBioState_t> *) BIO_get_data ( pBio );
	if ( !ppState || !pBuf || iNum<=0 )
		return 0;

	auto & tState = **ppState;
	int iResult = 0;
	if ( tState.m_iReadCalls<(int)tState.m_dReads.size() )
		iResult = tState.m_dReads[tState.m_iReadCalls];
	tState.m_iReadCalls++;

	if ( iResult<=0 )
		return iResult;

	int iToCopy = std::min ( iResult, iNum );
	memset ( pBuf, 'x', iToCopy );
	tState.m_iCopiedBytes += iToCopy;
	return iToCopy;
}

static int ScriptedBioWrite ( BIO *, const char *, int iNum )
{
	return iNum;
}

static long ScriptedBioCtrl ( BIO *, int iCmd, long, void * )
{
	switch ( iCmd )
	{
	case BIO_CTRL_FLUSH:
		return 1;
	case BIO_CTRL_PENDING:
	case BIO_CTRL_WPENDING:
	case BIO_CTRL_EOF:
	default:
		return 0;
	}
}

static BIO_METHOD * ScriptedBioMethod ()
{
	static BIO_METHOD * pMethod = nullptr;
	if ( !pMethod )
	{
		pMethod = BIO_meth_new ( BIO_get_new_index () | BIO_TYPE_SOURCE_SINK, "scripted test bio" );
		BIO_meth_set_create ( pMethod, ScriptedBioCreate );
		BIO_meth_set_destroy ( pMethod, ScriptedBioDestroy );
		BIO_meth_set_read ( pMethod, ScriptedBioRead );
		BIO_meth_set_write ( pMethod, ScriptedBioWrite );
		BIO_meth_set_ctrl ( pMethod, ScriptedBioCtrl );
	}

	return pMethod;
}

static BIO * MakeScriptedBio ( std::shared_ptr<ScriptedBioState_t> pState )
{
	auto pBio = BIO_new ( ScriptedBioMethod() );
	BIO_set_data ( pBio, new std::shared_ptr<ScriptedBioState_t> ( std::move ( pState ) ) );
	BIO_set_init ( pBio, 1 );
	return pBio;
}

TEST ( SslBioRead, NegativeReadDoesNotSpinOrAccountBytes )
{
	auto pState = std::make_shared<ScriptedBioState_t> ( std::initializer_list<int>{ -1, 0 } );
	auto pBuf = MakeSslTestBuffer ( MakeScriptedBio ( pState ) );

	EXPECT_FALSE ( pBuf->ReadFrom ( 1 ) );
	EXPECT_EQ ( pState->m_iReadCalls, 1 );
	EXPECT_EQ ( pState->m_iCopiedBytes, 0 );
	EXPECT_EQ ( pBuf->GetTotalReceived(), 0 );
}

TEST ( SslBioRead, PartialReadBeforeNegativeReadKeepsAccounting )
{
	auto pState = std::make_shared<ScriptedBioState_t> ( std::initializer_list<int>{ 2, -1, 0 } );
	auto pBuf = MakeSslTestBuffer ( MakeScriptedBio ( pState ) );

	EXPECT_FALSE ( pBuf->ReadFrom ( 3 ) );
	EXPECT_EQ ( pState->m_iReadCalls, 2 );
	EXPECT_EQ ( pState->m_iCopiedBytes, 2 );
	EXPECT_EQ ( pBuf->HasBytes(), 2 );
	EXPECT_EQ ( pBuf->GetTotalReceived(), 2 );
}

#endif // WITH_SSL
