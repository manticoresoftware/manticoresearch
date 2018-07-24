//
// Created by alexey on 14.02.18.
//

#include <gtest/gtest.h>

#include "sphinxint.h"
#include "searchdaemon.h"
#include "searchdha.h"


// QueryStatElement_t uses default ctr with inline initializer;
// this test is just to be sure it works correctly
TEST ( functions, QueryStatElement_t_ctr )
{
	QueryStatElement_t tElem;
	ASSERT_EQ ( sizeof(tElem.m_dData), QUERY_STATS_TYPE_TOTAL*sizeof(tElem.m_dData[0]));
	ASSERT_EQ ( tElem.m_uTotalQueries, 0);
	ASSERT_EQ ( tElem.m_dData[QUERY_STATS_TYPE_AVG], 0 );
	ASSERT_EQ ( tElem.m_dData[QUERY_STATS_TYPE_MIN], UINT64_MAX );
	ASSERT_EQ ( tElem.m_dData[QUERY_STATS_TYPE_MAX], 0 );
	ASSERT_EQ ( tElem.m_dData[QUERY_STATS_TYPE_95], 0 );
	ASSERT_EQ ( tElem.m_dData[QUERY_STATS_TYPE_99], 0 );
}

class tstlogger
{
	// test helper log - logs into sLogBuff.
	static void TestLogger ( ESphLogLevel eLevel, const char * sFmt, va_list ap )
	{
		if ( eLevel>=SPH_LOG_DEBUG )
			return;

		const char * lvl = "";
		switch ( eLevel )
		{
		case SPH_LOG_FATAL: lvl = "FATAL: ";
			break;
		case SPH_LOG_WARNING:
		case SPH_LOG_INFO: lvl = "WARNING: ";
			break;
		case SPH_LOG_DEBUG:
		case SPH_LOG_VERBOSE_DEBUG:
		case SPH_LOG_VERY_VERBOSE_DEBUG: lvl = "DEBUG: ";
			break;
		}
		char * pOut = sLogBuff;
		pOut += sprintf ( pOut, "%s", lvl );
		vsprintf ( pOut, sFmt, ap );
	}

protected:
	void setup ()
	{
		sphSetLogger ( TestLogger );
		sLogBuff[0] = '\0';
	}

	static char sLogBuff[1024];
};

char tstlogger::sLogBuff[1024];

// check how ParseAddressPort holds different cases
class T_ParseAddressPort : protected tstlogger, public ::testing::Test
{

protected:
	void SetUp () final
	{
		tstlogger::setup();
	}

	WarnInfo_t tInfo {"tstidx", "tstagent" };

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
		"WARNING: index 'tstidx': agent 'tstagent': portnum expected before 'tail' - Using default IANA 9312 port"
		);
}

// wrong IP, no port, default port - success (we don't resolve addresses here)
TEST_F ( T_ParseAddressPort, wrong_ip_no_port)
{
	ParserTest ( "257.0.0.1|tail", true, "|tail",
		AF_INET, "257.0.0.1", IANA_PORT_SPHINXAPI,
		"WARNING: index 'tstidx': agent 'tstagent': colon and portnum expected before '|tail' - Using default IANA 9312 port"
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
		"WARNING: index 'tstidx': agent 'tstagent': invalid port number near '', - SKIPPING AGENT"
		);
}

// double host, no port, default port: - success, return first host
TEST_F ( T_ParseAddressPort, double_host_no_port )
{
	ParserTest ( "my_server:my_server2", true, ":my_server2",
		AF_INET, "my_server", IANA_PORT_SPHINXAPI,
		"WARNING: index 'tstidx': agent 'tstagent': portnum expected before 'my_server2' - Using default IANA 9312 port"
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

	MultiAgentDesc_c* ParserTestSimple ( const char * sInExpr, bool bExpectedResult )
	{
		g_bHostnameLookup = true;
		const char * pTest = sInExpr;
		auto pResult = ConfigureMultiAgent ( pTest, "tstidx", tAgentOptions );
		EXPECT_EQ ( (bool)pResult, bExpectedResult ) << sInExpr;
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
		"WARNING: index 'tstidx': agent '[only=options]': "
	"one or more hosts/sockets expected before [, - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_syntax )
{
	ParserTest ( "bla|ble|bli:idx[options]haha", false,
			   "WARNING: index 'tstidx': agent 'bla|ble|bli:idx[options]haha': "
	"wrong syntax: expected one or more hosts/sockets, then m.b. []-enclosed options, - SKIPPING AGENT");
}

TEST_F ( T_ConfigureMultiAgent, wrong_unknown_option )
{
	ParserTest ( "bla|ble|bli:idx[conn =pconn, ha_strategy=unknown]", false,
		"WARNING: index 'tstidx': agent 'bla|ble|bli:idx[conn =pconn, ha_strategy=unknown]': "
			"unknown agent option ' ha_strategy=unknown', - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_formatted_option )
{
	ParserTest ( "bla|ble|bli[options]", false,
			   "WARNING: index 'tstidx': agent 'bla|ble|bli[options]': "
	"option options error: option and value must be =-separated pair, - SKIPPING AGENT");
}

// fixme: m.b. parse 1000abc as index name in the case?
TEST_F ( T_ConfigureMultiAgent, fixme_wrong_agent_port )
{
	ParserTest ( "localhost:1000abc", false, "WARNING: index 'tstidx': agent 'localhost:1000abc': "
		"after host/socket expected ':', then index(es), but got 'abc'), - SKIPPING AGENT" );
}

// fixme: m.b. parse 100000abc as index name in the case?
TEST_F ( T_ConfigureMultiAgent, fixme_wrong_portnum )
{
	ParserTest ( "localhost:100000abc", false, "WARNING: index 'tstidx': agent 'localhost:100000abc': "
		"invalid port number near 'abc', - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_idx_name )
{
	ParserTest ( "/localhost:idx,idx-name,idx2", false,
		"WARNING: index 'tstidx': agent '/localhost:idx,idx-name,idx2': "
		"no such index: idx-name, - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_idx_delimited_from_host )
{
	ParserTest ( "localhost:1000 idx", false, "WARNING: index 'tstidx': agent 'localhost:1000 idx': "
		"after host/socket expected ':', then index(es), but got ' idx'), - SKIPPING AGENT" );
}

TEST_F ( T_ConfigureMultiAgent, wrong_empty_idx )
{
	ParserTest ( "", false, "WARNING: index 'tstidx': agent '': "
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
	ParserTest ( "bla|ble|bli:idx[conn=pconn]", true, "WARNING: index 'tstidx': agent 'bla|ble|bli:idx[conn=pconn]': "
		"portnum expected before 'idx' - Using default IANA 9312 port" );
}

TEST_F ( T_ConfigureMultiAgent, agent_ok_options_space_sparsed )
{
	ParserTest ( "bla|ble|bli:idx[conn =pconn, blackhole = 1]", true
				 , "WARNING: index 'tstidx': agent 'bla|ble|bli:idx[conn =pconn, blackhole = 1]': "
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

	mutable CSphRwlock m_tLock;

protected:
	// no manual deletion; lifetime managed by AddRef/Release()
	virtual ~Handler_t ()
	{
		m_tLock.Done ();
	};

public:
	Handler_t ( int payload )
		: Core_c { payload }
	{
		Verify ( m_tLock.Init () );
	}
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

TEST ( T_IndexHash, served_hash_and_getter )
{
	auto pHash = new GuardedHash_c;
	ServedDesc_t tDesc;
	pHash->AddUniq ( new ServedIndex_c ( tDesc ), "hello" );
	pHash->AddUniq ( new ServedIndex_c ( tDesc ), "world" );

	{
		auto pHello = GetServed ( "hello", pHash );
		auto pWorld = GetServed ( "hello", pHash );
		ServedDescRPtr_c pIdx ( pHello );
//		auto pIdx2 = ServedIndexScPtr_c ( pWorld );
//		pIdx  = std::move (pIdx2); // must release prev. mutex and acquire new one
//		ServedIndexScPtr_c pIdxConcurrent ( pHello ); //warning: acquiring mutex 'pHello.().m_tLock' that is already held
	}

	auto pFoo = GetServed ( "world", pHash );
	pFoo = GetServed ( "hello", pHash );

	ASSERT_EQ ( pFoo->GetRefcount (), 2 ) << "1 from creation, +1 from GetServed";
	{
		auto pFee = GetServed ( "hello", pHash );
		ASSERT_EQ ( pFoo->GetRefcount (), 3 ) << "+1 from GetServed";
	}
	ASSERT_EQ ( pFoo->GetRefcount (), 2 ) << "temporary destroyed";

	// try to add with the same key
	auto pFee = new ServedIndex_c ( tDesc );
	ASSERT_FALSE ( pHash->AddUniq ( pFee, "hello" ) ) << "dupes not alowed";

	// try to replace existing one
	pHash->AddOrReplace ( pFee, "hello" );
	ASSERT_EQ ( pFoo->GetRefcount (), 1 ) << "we're the only owner now";
	ASSERT_EQ ( pFee->GetRefcount (), 1 ) << "hash owns the var, we just have a weak ptr";
	delete pHash;
	ASSERT_EQ ( pFoo->GetRefcount (), 1 ) << "we're the only owner now";
}

TEST ( T_IndexHash, ensure_right_refcounting )
{
	auto pHash = new GuardedHash_c;
	ServedDesc_t tDesc;
	pHash->AddUniq ( new ServedIndex_c ( tDesc ), "hello" );


	{
		ServedDescRPtr_c pIdx ( GetServed ( "hello", pHash ) );
//		pIdx = ServedIndexScPtr_c ( GetServed ( "world", pHash ) );
	}

	auto pFoo = GetServed ( "world", pHash );
	pFoo = GetServed ( "hello", pHash );

	ASSERT_EQ ( pFoo->GetRefcount (), 2 ) << "1 from creation, +1 from GetServed";
	{
		auto pFee = GetServed ( "hello", pHash );
		ASSERT_EQ ( pFoo->GetRefcount (), 3 ) << "+1 from GetServed";
	}
	ASSERT_EQ ( pFoo->GetRefcount (), 2 ) << "temporary destroyed";

	// try to add with the same key
	auto pFee = new ServedIndex_c ( tDesc );
	ASSERT_FALSE ( pHash->AddUniq ( pFee, "hello" ) ) << "dupes not alowed";

	// try to replace existing one
	pHash->AddOrReplace ( pFee, "hello" );
	ASSERT_EQ ( pFoo->GetRefcount (), 1 ) << "we're the only owner now";
	ASSERT_EQ ( pFee->GetRefcount (), 1 ) << "hash owns the var, we just have a weak ptr";
	delete pHash;
	ASSERT_EQ ( pFoo->GetRefcount (), 1 ) << "we're the only owner now";
}


TEST ( T_IndexHash, served_pointer_manipulations )
{
	auto pHash = new GuardedHash_c;
	ServedDesc_t tDesc;
	pHash->AddUniq ( new ServedIndex_c ( tDesc ), "hello" );

	using DistrPtrs_t = VecRefPtrs_t<const ServedIndex_c *>;

	{
		DistrPtrs_t dVec;

		{
			dVec.Reset ();
			dVec.Resize ( 1 );
			dVec.ZeroMem ();
			ASSERT_TRUE ( pHash->Contains ( "hello" ) );
			dVec[0] = GetServed ("hello", pHash);
			dVec[0]->AddRef ();
		}
		delete pHash;
//		auto & x = dVec[0];
	}
}