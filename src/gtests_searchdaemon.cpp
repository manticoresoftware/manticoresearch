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
	virtual void SetUp ()
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
		AgentDesc_c tFoo;
		bool bResult = ParseAddressPort ( &tFoo, &pTest, tInfo );
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
	virtual void SetUp ()
	{
		tstlogger::setup ();
	}

	AgentOptions_t tAgentOptions { false, false, HA_RANDOM, 3, 1 };
	const char * szIndexName = "tstidx";

	void ParserTestImpl ( MultiAgentDesc_t& tAgent, const char * sInExpr
		,bool bExpectedResult
		,const char * sWarningMessage
	)
	{
		g_bHostnameLookup = true;
		const char * pTest = sInExpr;
		bool bResult = ConfigureMultiAgent ( tAgent, pTest, "tstidx", tAgentOptions );
		EXPECT_EQ ( bResult, bExpectedResult ) << sInExpr;
		EXPECT_STREQ ( sWarningMessage, sLogBuff );
	}

	void ParserTest ( const char * sInExpr // incoming line
		, bool bExpectedResult                // expect parsed or not
		, const char * sWarningMessage = ""    // expected warning message, if any
	)
	{
		MultiAgentDesc_t tAgent;
		return ParserTestImpl ( tAgent, sInExpr, bExpectedResult, sWarningMessage );
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