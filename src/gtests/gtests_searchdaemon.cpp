//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinxint.h"
#include "searchdaemon.h"
#include "searchdha.h"
#include "searchdreplication.h"


// QueryStatElement_t uses default ctr with inline initializer;
// this test is just to be sure it works correctly
TEST ( functions, QueryStatElement_t_ctr )
{
	using namespace QueryStats;
	QueryStatElement_t tElem;
	ASSERT_EQ ( sizeof(tElem.m_dData), TYPE_TOTAL*sizeof(tElem.m_dData[0]));
	ASSERT_EQ ( tElem.m_uTotalQueries, 0);
	ASSERT_EQ ( tElem.m_dData[TYPE_AVG], 0 );
	ASSERT_EQ ( tElem.m_dData[TYPE_MIN], UINT64_MAX );
	ASSERT_EQ ( tElem.m_dData[TYPE_MAX], 0 );
	ASSERT_EQ ( tElem.m_dData[TYPE_95], 0 );
	ASSERT_EQ ( tElem.m_dData[TYPE_99], 0 );
}

class tstlogger
{
	// test helper log - logs into sLogBuff.
	static void TestLogger ( ESphLogLevel eLevel, const char * sFmt, va_list ap )
	{
		if ( eLevel>m_eMaxLevel )
			return;

		if ( m_bOutToStderr )
			vfprintf ( stderr, sFmt, ap);
		else
		{
			const char * lvl = "";
			switch (eLevel) {
			case SPH_LOG_FATAL:
				lvl = "FATAL: ";
				break;
			case SPH_LOG_WARNING:
			case SPH_LOG_INFO:
				lvl = "WARNING: ";
				break;
			case SPH_LOG_DEBUG:
			case SPH_LOG_RPL_DEBUG:
			case SPH_LOG_VERBOSE_DEBUG:
			case SPH_LOG_VERY_VERBOSE_DEBUG:
			default:
				lvl = "DEBUG: ";
				break;
			}
			char * pOut = sLogBuff;
			pOut += sprintf( pOut, "%s", lvl );
			vsprintf( pOut, sFmt, ap );
		}
	}

protected:
	void setup ()
	{
		g_pLogger() = TestLogger;
		sLogBuff[0] = '\0';
	}

	static char sLogBuff[1024];
	static bool m_bOutToStderr;
	static ESphLogLevel m_eMaxLevel;

public:
	static void SetStderrLogger()
	{
		m_eMaxLevel = SPH_LOG_DEBUG;
		m_bOutToStderr = true;
		g_pLogger () = TestLogger;
	}
};

char tstlogger::sLogBuff[1024];
bool tstlogger::m_bOutToStderr = false;
ESphLogLevel tstlogger::m_eMaxLevel = SPH_LOG_INFO;


class CustomLogger_c: protected tstlogger, public ::testing::Test
{
protected:
	void SetUp() override
	{
		tstlogger::setup();
	}
};

class DeathLogger_c: protected tstlogger, public ::testing::Test
{
protected:
	void SetUp() override
	{
		m_bOutToStderr = true;
		tstlogger::setup();
	}
};

void SetStderrLogger()
{
	tstlogger::SetStderrLogger ();
}

// check how ParseAddressPort holds different cases
class T_ParseAddressPort :  public CustomLogger_c
{
protected:

	WarnInfo_c tInfo {"tstidx", "tstagent" };

	void ParserTest ( const char * sInExpr, // incoming line
		bool bExpectedResult,				// expect parsed or not
		const char *sExpectedTail,			// tail of incoming line ret to caller
		int iParsedFamily,					// expected inet family
		const char * sParsedAddress, 		// expected resulting parsed address/path
		int iParsedPort	= -1,				// expected resulting parsed port
		const char * sWarningMessage = ""	// expected warning message, if any
		 )
	{
		const char * pTest = sInExpr;
		AgentDesc_t tFoo;
		bool bResult = ParseAddressPort ( tFoo, &pTest, tInfo );
		EXPECT_EQ ( bResult, bExpectedResult ) << sInExpr;
		EXPECT_STREQ ( sExpectedTail, pTest );
		EXPECT_STREQ ( sWarningMessage, sLogBuff );
		// the rest have no sense to check at all if parsing failed
		if ( !bResult )
			return;

		ASSERT_EQ ( iParsedFamily, tFoo.m_iFamily );
		ASSERT_STREQ ( sParsedAddress, tFoo.m_sAddr.cstr () );
		ASSERT_EQ ( iParsedPort, tFoo.m_iPort );
	}
};


// simple IP, no port, default port - success
TEST_F ( T_ParseAddressPort, simple_ip_no_port )
{
	ParserTest ( "127.0.0.1:tail", true, ":tail",
		AF_INET, "127.0.0.1", IANA_PORT_SPHINXAPI,
		"WARNING: table 'tstidx': agent 'tstagent': portnum expected before 'tail' - Using default IANA 9312 port"
		);
}

// wrong IP, no port, default port - success (we don't resolve addresses here)
TEST_F ( T_ParseAddressPort, wrong_ip_no_port)
{
	ParserTest ( "257.0.0.1|tail", true, "|tail",
		AF_INET, "257.0.0.1", IANA_PORT_SPHINXAPI,
		"WARNING: table 'tstidx': agent 'tstagent': colon and portnum expected before '|tail' - Using default IANA 9312 port"
		);
}

// any host, explicit port - success
TEST_F ( T_ParseAddressPort, any_host_with_port )
{
	ParserTest ( "my_server:9654:tail", true, ":tail",
		AF_INET, "my_server", 9654);
}

// any host, wrong port - fail
TEST_F ( T_ParseAddressPort, any_host_wrong_port )
{
	ParserTest ( "my_server:96540", false, "",
		AF_INET, "my_server", IANA_PORT_SPHINXAPI,
		"WARNING: table 'tstidx': agent 'tstagent': invalid port number near '', - SKIPPING AGENT"
		);
}

// double host, no port, default port: - success, return first host
TEST_F ( T_ParseAddressPort, double_host_no_port )
{
	ParserTest ( "my_server:my_server2", true, ":my_server2",
		AF_INET, "my_server", IANA_PORT_SPHINXAPI,
		"WARNING: table 'tstidx': agent 'tstagent': portnum expected before 'my_server2' - Using default IANA 9312 port"
		);
}

// host, port, another host - success, return first host
TEST_F ( T_ParseAddressPort, host_port_host2 )
{
	ParserTest ("my_server:1000:my_server2", true, ":my_server2",
		AF_INET, "my_server", 1000);
}

// unix host, port, another host - success, leave all the rest after the host
TEST_F ( T_ParseAddressPort, unixhost_port )
{
	ParserTest ( "/my_server:1000:my_server2", true, ":1000:my_server2",
		AF_UNIX, "/my_server");
}

class T_ConfigureMultiAgent : protected tstlogger, public ::testing::Test
{

protected:
	void SetUp () final
	{
		tstlogger::setup ();
	}

	AgentOptions_t tAgentOptions { false, false, HA_RANDOM, 3, 0 };
	const char * szIndexName = "tstidx";

	MultiAgentDescRefPtr_c ParserTestSimple ( const char * sInExpr, bool bExpectedResult )
	{
		g_bHostnameLookup = true;
		const char * pTest = sInExpr;
		auto pResult = ConfigureMultiAgent ( pTest, "tstidx", tAgentOptions );
		EXPECT_EQ ( pResult!=nullptr, bExpectedResult ) << sInExpr;
		return pResult;
	}

	void ParserTest ( const char * sInExpr // incoming line
		, bool bExpectedResult                // expect parsed or not
		, const char * sWarningMessage = ""    // expected warning message, if any
	)
	{
		MultiAgentDescRefPtr_c pAgent ( ParserTestSimple ( sInExpr, bExpectedResult ) );
		EXPECT_STREQ ( sWarningMessage, sLogBuff );
	}
};

TEST_F ( T_ConfigureMultiAgent, wrong_only_options )
{
	ParserTest ( "[only=options]", false,
		"WARNING: table 'tstidx': agent '[only=options]': "
	"one or more hosts/sockets expected before [, - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_syntax )
{
	ParserTest ( "bla|ble|bli:idx[options]haha", false,
			   "WARNING: table 'tstidx': agent 'bla|ble|bli:idx[options]haha': "
	"wrong syntax: expected one or more hosts/sockets, then m.b. []-enclosed options, - SKIPPING AGENT");
}

TEST_F ( T_ConfigureMultiAgent, wrong_unknown_option )
{
	ParserTest ( "bla|ble|bli:idx[conn =pconn, ha_strategy=unknown]", false,
		"WARNING: table 'tstidx': agent 'bla|ble|bli:idx[conn =pconn, ha_strategy=unknown]': "
			"unknown agent option ' ha_strategy=unknown', - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_formatted_option )
{
	ParserTest ( "bla|ble|bli[options]", false,
			   "WARNING: table 'tstidx': agent 'bla|ble|bli[options]': "
	"option options error: option and value must be =-separated pair, - SKIPPING AGENT");
}

// fixme: m.b. parse 1000abc as index name in the case?
TEST_F ( T_ConfigureMultiAgent, fixme_wrong_agent_port )
{
	ParserTest ( "localhost:1000abc", false, "WARNING: table 'tstidx': agent 'localhost:1000abc': "
		"after host/socket expected ':', then table(s), but got 'abc'), - SKIPPING AGENT" );
}

// fixme: m.b. parse 100000abc as index name in the case?
TEST_F ( T_ConfigureMultiAgent, fixme_wrong_portnum )
{
	ParserTest ( "localhost:100000abc", false, "WARNING: table 'tstidx': agent 'localhost:100000abc': "
		"invalid port number near 'abc', - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_idx_name )
{
	ParserTest ( "/localhost:idx,idx=name,idx2", false,
		"WARNING: table 'tstidx': agent '/localhost:idx,idx=name,idx2': "
		"no such table: idx=name, - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_idx_delimited_from_host )
{
	ParserTest ( "localhost:1000 idx", false, "WARNING: table 'tstidx': agent 'localhost:1000 idx': "
		"after host/socket expected ':', then table(s), but got ' idx'), - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_empty_idx )
{
	ParserTest ( "", false, "WARNING: table 'tstidx': agent '': "
		"empty agent definition, - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, agent_couple_mirrors )
{
	ParserTest ( "localhost:1000:idx|localhost:1003:idx1,idx2,idx3", true );
}

TEST_F ( T_ConfigureMultiAgent, agent_host_port_index )
{
	ParserTest ( "localhost:1000:idx", true );
}

TEST_F ( T_ConfigureMultiAgent, agent_ok_only_one_index_and_options )
{
	ParserTest ( "bla|ble|bli:idx[conn=pconn]", true, "WARNING: table 'tstidx': agent 'bla|ble|bli:idx[conn=pconn]': "
		"portnum expected before 'idx' - Using default IANA 9312 port" );
}

TEST_F ( T_ConfigureMultiAgent, agent_ok_options_space_sparsed )
{
	ParserTest ( "bla|ble|bli:idx[conn =pconn, blackhole = 1]", true
				 , "WARNING: table 'tstidx': agent 'bla|ble|bli:idx[conn =pconn, blackhole = 1]': "
			"portnum expected before 'idx' - Using default IANA 9312 port" );
}

TEST_F ( T_ConfigureMultiAgent, fully_configured_3_mirrors )
{
	MultiAgentDescRefPtr_c pAgent (
		ParserTestSimple ( "127.0.0.1|bla:6000:idx|/path[blackhole=1,retry_count=4,conn=pconn]", true ) );

	auto &tAgent = *pAgent;

	ASSERT_EQ ( tAgent.GetLength (), 3);
	ASSERT_EQ ( tAgent.GetRetryLimit(), 4);
	ASSERT_TRUE ( tAgent.IsHA() );

	auto &tFirst = tAgent[0];
	ASSERT_STREQ ( tFirst.m_sIndexes.cstr (), "idx" );
	ASSERT_STREQ ( tFirst.m_sAddr.cstr (), "127.0.0.1" );
	ASSERT_EQ ( tFirst.m_iFamily, AF_INET );
	ASSERT_FALSE ( tFirst.m_bNeedResolve ) << "since plain IP provided, no more resolving necessary";
	ASSERT_EQ ( tFirst.m_uAddr, 16777343 );
	ASSERT_EQ ( tFirst.m_iPort, IANA_PORT_SPHINXAPI );
	ASSERT_TRUE ( tFirst.m_bBlackhole );
	ASSERT_TRUE ( tFirst.m_bPersistent );

	auto &tSecond = tAgent[1];
	ASSERT_STREQ ( tSecond.m_sIndexes.cstr (), "idx" );
	ASSERT_STREQ ( tSecond.m_sAddr.cstr (), "bla" );
	ASSERT_EQ ( tSecond.m_iFamily, AF_INET );
	ASSERT_TRUE ( tSecond.m_bNeedResolve );
	ASSERT_EQ ( tSecond.m_iPort, 6000 );
	ASSERT_TRUE ( tSecond.m_bBlackhole );
	ASSERT_TRUE ( tSecond.m_bPersistent );

	auto &tThird = tAgent[2];
	ASSERT_STREQ ( tThird.m_sIndexes.cstr (), "tstidx" );
	ASSERT_STREQ ( tThird.m_sAddr.cstr (), "/path" );
	ASSERT_EQ ( tThird.m_iFamily, AF_UNIX );
	ASSERT_TRUE ( tThird.m_bBlackhole );
	ASSERT_TRUE ( tThird.m_bPersistent );
}

TEST_F ( T_ConfigureMultiAgent, simple_host )
{
	MultiAgentDescRefPtr_c pAgent ( ParserTestSimple ( "bla", true ));
	auto &tAgent = *pAgent;

	ASSERT_EQ ( tAgent.GetLength (), 1 );
	ASSERT_EQ ( tAgent.GetRetryLimit (), 3 );
	ASSERT_FALSE ( tAgent.IsHA () );

	auto & tMirror = tAgent[0];
	ASSERT_STREQ ( tMirror.m_sIndexes.cstr (), "tstidx" );
	ASSERT_STREQ ( tMirror.m_sAddr.cstr (), "bla" );
	ASSERT_EQ ( tMirror.m_iFamily, AF_INET );
	ASSERT_TRUE ( tMirror.m_bNeedResolve );
	ASSERT_EQ ( tMirror.m_iPort, IANA_PORT_SPHINXAPI );
	ASSERT_FALSE ( tMirror.m_bBlackhole );
	ASSERT_FALSE ( tMirror.m_bPersistent );
}

TEST_F ( T_ConfigureMultiAgent, simple_3_hosts )
{
	MultiAgentDescRefPtr_c pAgent ( ParserTestSimple ( "127.0.0.1|bla|/path", true ) );
	auto &tAgent = *pAgent;

	ASSERT_EQ ( tAgent.GetLength (), 3 );
	ASSERT_EQ ( tAgent.GetRetryLimit (), 9 );
	ASSERT_TRUE ( tAgent.IsHA () );

	auto &tFirst = tAgent[0];
	ASSERT_STREQ ( tFirst.m_sIndexes.cstr (), "tstidx" );
	ASSERT_STREQ ( tFirst.m_sAddr.cstr (), "127.0.0.1" );
	ASSERT_EQ ( tFirst.m_iFamily, AF_INET );
	ASSERT_FALSE ( tFirst.m_bNeedResolve ) << "since plain IP provided, no more resolving necessary";
	ASSERT_EQ ( tFirst.m_uAddr, 16777343 );
	ASSERT_EQ ( tFirst.m_iPort, IANA_PORT_SPHINXAPI );
	ASSERT_FALSE ( tFirst.m_bBlackhole );
	ASSERT_FALSE ( tFirst.m_bPersistent );

	auto &tSecond = tAgent[1];
	ASSERT_STREQ ( tSecond.m_sIndexes.cstr (), "tstidx" );
	ASSERT_STREQ ( tSecond.m_sAddr.cstr (), "bla" );
	ASSERT_EQ ( tSecond.m_iFamily, AF_INET );
	ASSERT_TRUE ( tSecond.m_bNeedResolve );
	ASSERT_EQ ( tSecond.m_iPort, IANA_PORT_SPHINXAPI );
	ASSERT_FALSE ( tSecond.m_bBlackhole );
	ASSERT_FALSE ( tSecond.m_bPersistent );

	auto &tThird = tAgent[2];
	ASSERT_STREQ ( tThird.m_sIndexes.cstr (), "tstidx" );
	ASSERT_STREQ ( tThird.m_sAddr.cstr (), "/path" );
	ASSERT_EQ ( tThird.m_iFamily, AF_UNIX );
	ASSERT_FALSE ( tThird.m_bBlackhole );
	ASSERT_FALSE ( tThird.m_bPersistent );
}

// staging...
// this classes are here only for tests (to avoid recompiling of a big piece in case of experiments)
// the most base class we protect.
class Core_c
{
public:
	explicit Core_c ( int payload ) : m_iPayload (payload) {}
	int m_iPayload = 0;
};

// keeps naked pointer to base class and give it to nobody
class Handler_t : public ISphRefcountedMT, Core_c
{
private:
	friend class HandlerL_t;
	Core_c * ReadLock () const ACQUIRE_SHARED( m_tLock )
	{
		AddRef();
		m_tLock.ReadLock ();
		return ( Core_c * ) this;
	}

	Core_c * WriteLock () const ACQUIRE( m_tLock )
	{
		AddRef();
		m_tLock.WriteLock ();
		return ( Core_c * )this;
	}

	void Unlock () const UNLOCK_FUNCTION( m_tLock )
	{
		m_tLock.Unlock ();
		Release();
	}

	mutable RwLock_t m_tLock;

protected:
	// no manual deletion; lifetime managed by AddRef/Release()
	virtual ~Handler_t () = default;

public:
	Handler_t ( int payload )
		: Core_c { payload }
	{}
};


/// RAII shared read and write lock
class SCOPED_CAPABILITY HandlerL_t : ISphNoncopyable
{
public:
	// by default acquire read (shared) lock
	HandlerL_t ( Handler_t * pLock ) ACQUIRE_SHARED( pLock->m_tLock )
		: m_pCore { pLock->ReadLock () }, m_pLock { pLock }
	{}

	// acquire write (exclusive) lock
	HandlerL_t ( Handler_t * pLock, bool ) ACQUIRE ( pLock->m_tLock )
		: m_pCore { pLock->WriteLock () }, m_pLock { pLock }
	{}

	void Unlock() RELEASE ()
	{
		if ( m_pLock )
			m_pLock->Unlock();
		m_pLock = nullptr;
		m_pCore = nullptr;
	}

	/// unlock on going out of scope
	~HandlerL_t () RELEASE ()
	{
		if ( m_pLock )
			m_pLock->Unlock();
	}

	HandlerL_t ( HandlerL_t	&& rhs ) noexcept ACQUIRE ( rhs.m_pLock->m_tLock )
	: m_pCore { rhs.m_pCore }, m_pLock {rhs.m_pLock}
	{}


	HandlerL_t &operator= ( HandlerL_t &&rhs ) RELEASE () ACQUIRE ( rhs.m_pLock->m_tLock )
	{
		if ( &rhs==this )
			return *this;
		if ( m_pLock )
			m_pLock->Unlock ();
		m_pCore = rhs.m_pCore;
		m_pLock = rhs.m_pLock;
		rhs.m_pCore = nullptr;
		rhs.m_pLock = nullptr;
		return *this;
	}

public:

	Core_c * operator-> () const
	{ return m_pCore; }

	operator bool () const
	{ return m_pCore!=nullptr; }

	operator Core_c * () const
	{ return m_pCore; }

private:
	Core_c * m_pCore = nullptr;
	Handler_t * m_pLock = nullptr;
};

HandlerL_t GetHandler ( Handler_t * pLock )
{
	return HandlerL_t (pLock);
}

TEST ( new_addref_flavour, create_served_index_concept )
{
	int payload = 10;
	int payloadb = 13;

	auto pFoo = new Handler_t ( payload );
	auto pFee = new Handler_t ( payloadb );

	ASSERT_EQ ( pFoo->GetRefcount (), 1);

//	a->ReadLock();

	{
		HandlerL_t a = ( pFoo );
		ASSERT_EQ ( a->m_iPayload, 10 );
//		a = HandlerL_t ( pFee );

//		a.Unlock();
		HandlerL_t b ( pFee, true );
//		dUser->Unlock();
//		ASSERT_STREQ ( dUser->m_sIndexPath.cstr (), "blabla" );
		ASSERT_EQ ( pFoo->GetRefcount (), 2 ) << "one from creation, second from RLocked";
	}
	HandlerL_t b ( pFoo );
	pFoo->Release();
	pFee->Release();
}


class TReadonlyHash_c: public ::testing::Test
{
protected:
	ReadOnlyHash_T<>* pHash = nullptr;
	RefCountedRefPtrGeneric_t pRef;
	RefCountedRefPtrGeneric_t pNullRef;

	void SetUp() override
	{
		pHash = new ReadOnlyHash_T<>;
		pRef = new ISphRefcountedMT();
		ASSERT_TRUE ( pRef->IsLast() ) << "we are the one";
	}

	void TearDown() override
	{
		SafeDelete ( pHash );
		ASSERT_TRUE ( pRef->IsLast() ) << "hash deleted, we a the one";
	}
};


TEST_F ( TReadonlyHash_c, AddUniq )
{
	pHash->Add ( pRef, "hello" );
	ASSERT_EQ ( pRef->GetRefcount(), 2 ) << "one we, second hash";

	pHash->Add ( pRef, "hello" );
	ASSERT_EQ ( pRef->GetRefcount(), 2 ) << "no second addition";

	pHash->Add ( pRef, "world" );
	ASSERT_EQ ( pRef->GetRefcount(), 3 ) << "3 of us";
}

TEST_F ( TReadonlyHash_c, AddOrReplace )
{
	pHash->AddOrReplace ( pRef, "hello" );
	ASSERT_EQ ( pRef->GetRefcount(), 2 ) << "one we, second hash";

	pHash->AddOrReplace ( pRef, "hello" );
	ASSERT_EQ ( pRef->GetRefcount(), 2 ) << "no second addition";

	pHash->AddOrReplace ( pRef, "world" );
	ASSERT_EQ ( pRef->GetRefcount(), 3 ) << "3 of us";
}

TEST_F ( TReadonlyHash_c, Delete )
{
	// prepare
	pHash->Add ( pRef, "hello" );
	pHash->Add ( pRef, "world" );
	EXPECT_EQ ( pRef->GetRefcount(), 3 );
	EXPECT_EQ ( pHash->GetLength(), 2 );

	// the test
	pHash->Delete ( "hello" );
	ASSERT_EQ ( pRef->GetRefcount(), 2 ) << "no second addition";
	ASSERT_EQ ( pHash->GetLength(), 1 );


	pHash->Delete ( "world" );
	ASSERT_TRUE ( pRef->IsLast() ) << "3 of us";
	ASSERT_EQ ( pHash->GetLength(), 0 );
}

TEST_F ( TReadonlyHash_c, GetLength )
{
	pHash->Add ( pNullRef, "hello" );
	EXPECT_EQ ( pHash->GetLength(), 1 );

	pHash->Add ( pNullRef, "world" );
	EXPECT_EQ ( pHash->GetLength(), 2 );

	pHash->Delete ( "hello" );
	ASSERT_EQ ( pHash->GetLength(), 1 );

	pHash->Delete ( "world" );
	ASSERT_EQ ( pHash->GetLength(), 0 );
}

TEST_F ( TReadonlyHash_c, Contains )
{
	pHash->Add ( pNullRef, "hello" );
	pHash->Add ( pRef, "world" );

	EXPECT_EQ ( pRef->GetRefcount(), 2 );

	ASSERT_FALSE ( pHash->Contains ( "foo" ) ) << "foo wasn't inserted";
	ASSERT_TRUE ( pHash->Contains ( "hello" ) ) << "hello has null value";
	ASSERT_TRUE ( pHash->Contains ( "world" ) ) << "world is ok";

	ASSERT_EQ ( pRef->GetRefcount(), 2 ) << "contains does'nt affect ref at all";
}

TEST_F ( TReadonlyHash_c, ReleaseAndClear )
{
	pHash->Add ( pNullRef, "hello" );
	pHash->Add ( pRef, "world" );

	EXPECT_EQ ( pRef->GetRefcount(), 2 );
	EXPECT_EQ ( pHash->GetLength(), 2 );

	pHash->ReleaseAndClear();

	ASSERT_EQ ( pHash->GetLength(), 0 ) << "hash must be anandoned";
	ASSERT_TRUE ( pRef->IsLast() ) << "me was removed";
}

TEST_F ( TReadonlyHash_c, Get )
{
	pHash->Add ( pNullRef, "hello" );
	pHash->Add ( pRef, "world" );
	EXPECT_EQ ( pRef->GetRefcount(), 2 );

	{ // get unexistent
		cRefCountedRefPtrGeneric_t pFoo { pHash->Get ( "foo" ) };
		ASSERT_FALSE ( pFoo ) << "wasn't in hash";
		EXPECT_TRUE ( !pFoo ) << "wasn't in hash";
		EXPECT_TRUE ( pFoo == nullptr ) << "wasn't in hash";
		ASSERT_EQ ( pRef->GetRefcount(), 2 );
	}
	ASSERT_EQ ( pRef->GetRefcount(), 2 );

	{ // get null
		cRefCountedRefPtrGeneric_t pFoo { pHash->Get ( "hello" ) };
		ASSERT_FALSE ( bool ( pFoo ) ) << "null  in hash";
		ASSERT_EQ ( pRef->GetRefcount(), 2 );
	}
	ASSERT_EQ ( pRef->GetRefcount(), 2 );

	{ // get existing
		cRefCountedRefPtrGeneric_t pFoo { pHash->Get ( "world" ) };
		ASSERT_EQ ( pFoo->GetRefcount(), 3 );
		ASSERT_EQ ( pRef->GetRefcount(), 3 );
	}
	ASSERT_EQ ( pRef->GetRefcount(), 2 );
}


bool operator==( const ListenerDesc_t& lhs, const ListenerDesc_t& rhs )
{
	return lhs.m_eProto==rhs.m_eProto
	&& lhs.m_bVIP == rhs.m_bVIP
	&& lhs.m_bReadOnly == rhs.m_bReadOnly
	&& lhs.m_iPort == rhs.m_iPort
	&& lhs.m_iPortsCount == rhs.m_iPortsCount
	&& lhs.m_uIP == rhs.m_uIP
	&& lhs.m_sUnix == rhs.m_sUnix;
}


// simple IP, no port, default port - success
TEST ( ParseListener, simple_ip_no_port )
{
	struct { const char* sSpec; ListenerDesc_t sRes; } dTable[] = {
		{"8.8.8.8:1000",		{ Proto_e::SPHINX, "", 134744072, 1000, 0, false }},
		{"1000",				{ Proto_e::SPHINX, "", 0, 1000, 0, false }},
		{"/linux/host",			{ Proto_e::SPHINX, "/linux/host", 0, 9312, 0, false }},
		{"8.8.8.8:1000-10000",	{ Proto_e::SPHINX, "", 134744072, 1000, 9001, false }},

		{"8.8.8.8:1000:sphinx",		{ Proto_e::SPHINXSE, "", 134744072, 1000, 0, false }},
		{"1000:sphinx",				{ Proto_e::SPHINXSE, "", 0, 1000, 0, false }},
		{"/linux/host:sphinx",		{ Proto_e::SPHINXSE, "/linux/host", 0, 9312, 0, false }},
		{"8.8.8.8:1000-10000:sphinx",	{ Proto_e::SPHINXSE, "", 134744072, 1000, 9001, false }},

		{"8.8.8.8:1000:mysql41",	{ Proto_e::MYSQL41, "", 134744072, 1000, 0, false }},
		{"1000:mysql41",			{ Proto_e::MYSQL41, "", 0, 1000, 0, false }},
		{"/linux/host:mysql41",			{ Proto_e::MYSQL41, "/linux/host", 0, 9312, 0, false }},
		{"8.8.8.8:1000-10000:mysql41",	{ Proto_e::MYSQL41, "", 134744072, 1000, 9001, false }},

		{"8.8.8.8:1000:http",		{ Proto_e::HTTP, "", 134744072, 1000, 0, false }},
		{"1000:http",				{ Proto_e::HTTP, "", 0, 1000, 0, false }},
		{"/linux/host:http",		{ Proto_e::HTTP, "/linux/host", 0, 9312, 0, false }},
		{"8.8.8.8:1000-10000:http",	{ Proto_e::HTTP, "", 134744072, 1000, 9001, false }},

		{"8.8.8.8:1000:replication",	{ Proto_e::REPLICATION, "", 134744072, 1000, 0, false }},
		{"1000:replication",			{ Proto_e::REPLICATION, "", 0, 1000, 0, false }},
		{"/linux/host:replication",		{ Proto_e::REPLICATION, "/linux/host", 0, 9312, 0, false }},
		{"8.8.8.8:1000-10000:replication",	{ Proto_e::REPLICATION, "", 134744072, 1000, 9001, false }},

		{"8.8.8.8:1000:sphinx_vip",		{ Proto_e::SPHINXSE, "", 134744072, 1000, 0, true }},
		{"1000:sphinx_vip",				{ Proto_e::SPHINXSE, "", 0, 1000, 0, true }},
		{"/linux/host:sphinx_vip",		{ Proto_e::SPHINXSE, "/linux/host", 0, 9312, 0, true }},
		{"8.8.8.8:1000-10000:sphinx_vip",	{ Proto_e::SPHINXSE, "", 134744072, 1000, 9001, true }},

		{"8.8.8.8:1000:mysql41_vip",	{ Proto_e::MYSQL41, "", 134744072, 1000, 0, true }},
		{"1000:mysql41_vip",			{ Proto_e::MYSQL41, "", 0, 1000, 0, true }},
		{"/linux/host:mysql41_vip",		{ Proto_e::MYSQL41, "/linux/host", 0, 9312, 0, true }},
		{"8.8.8.8:1000-10000:mysql41_vip",	{ Proto_e::MYSQL41, "", 134744072, 1000, 9001, true }},

		{"8.8.8.8:1000:http_vip",	{ Proto_e::HTTP, "", 134744072, 1000, 0, true }},
		{"1000:http_vip",			{ Proto_e::HTTP, "", 0, 1000, 0, true }},
		{"/linux/host:http_vip",	{ Proto_e::HTTP, "/linux/host", 0, 9312, 0, true }},
		{"8.8.8.8:1000-10000:http_vip",	{ Proto_e::HTTP, "", 134744072, 1000, 9001, true }},

		{"8.8.8.8:1000:replication_vip",	{ Proto_e::REPLICATION, "", 134744072, 1000, 0, true }},
		{"1000:replication_vip",			{ Proto_e::REPLICATION, "", 0, 1000, 0, true }},
		{"/linux/host:replication_vip",		{ Proto_e::REPLICATION, "/linux/host", 0, 9312, 0, true }},
		{"8.8.8.8:1000-10000:replication_vip",	{ Proto_e::REPLICATION, "", 134744072, 1000, 9001, true }},
	};

	for (const auto& sCase : dTable)
	{
#if _WIN32
		if ( sCase.sSpec[0]=='/' ) // skip of UNIX socket cases on Windows
			continue;
#endif
		ListenerDesc_t tDesc = ParseListener( sCase.sSpec );
		EXPECT_TRUE ( tDesc==sCase.sRes ) << sCase.sSpec;
	}
}

TEST_F ( DeathLogger_c, ParseListener_wrong_port )
{
	struct
	{
		const char* sSpec;
		ListenerDesc_t sRes;
	} dTable[] = {
		{ "8.8.8.8:65536", { Proto_e::SPHINX, "", 0, 9306, 0, false }},
		{ "65536", { Proto_e::SPHINX, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536", { Proto_e::SPHINX, "", 0, 1000, 0, false }},

		{ "8.8.8.8:65536:sphinx", { Proto_e::SPHINX, "", 0, 9306, 0, false }},
		{ "65536:sphinx", { Proto_e::SPHINX, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536:sphinx", { Proto_e::SPHINX, "", 0, 9306, 0, false }},

		{ "8.8.8.8:65536:mysql41", { Proto_e::MYSQL41, "", 0, 9306, 0, false }},
		{ "65536:mysql41", { Proto_e::MYSQL41, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536:mysql41", { Proto_e::MYSQL41, "", 0, 9306, 0, false }},

		{ "8.8.8.8:65536:http", { Proto_e::HTTP, "", 0, 9306, 0, false }},
		{ "65536:http", { Proto_e::HTTP, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536:http", { Proto_e::HTTP, "", 0, 9306, 0, false }},

		{ "8.8.8.8:65536:replication", { Proto_e::REPLICATION, "", 0, 9306, 0, false }},
		{ "65536:replication", { Proto_e::REPLICATION, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536:replication", { Proto_e::REPLICATION, "", 0, 9306, 0, false }},

		{ "8.8.8.8:65536:sphinx_vip", { Proto_e::SPHINX, "", 0, 9306, 0, false }},
		{ "65536:sphinx_vip", { Proto_e::SPHINX, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536:sphinx_vip", { Proto_e::SPHINX, "", 0, 9306, 0, false }},

		{ "8.8.8.8:65536:mysql41_vip", { Proto_e::MYSQL41, "", 0, 9306, 0, false }},
		{ "65536:mysql41_vip", { Proto_e::MYSQL41, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536:mysql41_vip", { Proto_e::MYSQL41, "", 0, 9306, 0, false }},

		{ "8.8.8.8:65536:http_vip", { Proto_e::HTTP, "", 0, 9306, 0, false }},
		{ "65536:http_vip", { Proto_e::HTTP, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536:http_vip", { Proto_e::HTTP, "", 0, 9306, 0, false }},

		{ "8.8.8.8:65536:replication_vip", { Proto_e::REPLICATION, "", 0, 9306, 0, false }},
		{ "65536:replication_vip", { Proto_e::REPLICATION, "", 0, 9306, 0, false }},
		{ "8.8.8.8:1000-65536:replication_vip", { Proto_e::REPLICATION, "", 0, 9306, 0, false }},
	};

	for ( const auto& sCase : dTable )
	{
		CSphString sFatal;
		ParseListener ( sCase.sSpec, &sFatal );
		EXPECT_STREQ ( "port 65536 is out of range", sFatal.cstr() );
	}
}
