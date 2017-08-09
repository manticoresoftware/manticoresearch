//
// Created by alexey on 09.08.17.
//

#include <gtest/gtest.h>

#include "sphinxint.h"

TEST (SHATests, stringbuilder) {
	StringBuilder_c builder;

	builder += "Hello";
	
	ASSERT_STREQ ( builder.cstr (), "Hello");

}


struct EscapeQuotation_t
{
	static bool IsEscapeChar ( char c )
	{
		return ( c=='\\' || c=='\'' );
	}

	static char GetEscapedChar ( char c )
	{
		return c;
	}
};


using QuotationEscapedBuilder = EscapedStringBuilder_T<EscapeQuotation_t>;

TEST( SHATests, EscapedBuildertest )
{
	QuotationEscapedBuilder tBuilder;
	tBuilder.AppendEscaped ("Hello");
	ASSERT_STREQ ( tBuilder.cstr (), "Hello" );

	tBuilder.AppendEscaped (" wo\\rl\'d");
	ASSERT_STREQ ( tBuilder.cstr(), "Hello wo\\\\rl\\'d");
}

TEST( SHATests, TaggedHash20_t )
{
	const char * sFIPS = "45f44fd2db02b08b4189abf21e90edd712c9616d *rt_full.ram\n";
	const BYTE bytescheck[HASH20_SIZE] = { 0x45, 0xf4, 0x4f, 0xd2, 0xdb, 0x02, 0xb0, 0x8b, 0x41, 0x89, 0xab, 0xf2
										   , 0x1e, 0x90, 0xed, 0xd7, 0x12, 0xc9, 0x61, 0x6d };
	const char * namecheck = "rt_full.ram";

	TaggedHash20_t tHash ( "HelloFips" );
	CSphString sFips = tHash.ToFIPS ();

	EXPECT_TRUE ( sFips == "" );

	tHash.FromFIPS ( sFIPS );
	ASSERT_TRUE ( tHash.m_sTagName==namecheck );

	ASSERT_TRUE ( 0 == memcmp ( tHash.m_dHashValue, bytescheck, HASH20_SIZE ) );

	sFips = tHash.ToFIPS ();
	ASSERT_TRUE ( sFips==sFIPS);

	TaggedHash20_t tHash2 ( namecheck, bytescheck );
	ASSERT_TRUE ( tHash2.ToFIPS ()== sFIPS );

}

