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

#include <gtest/gtest.h>

#include "sphinxint.h"
#include "sphinxutils.h"
#include "json/cJSON.h"
#include <math.h>

// Miscelaneous short functional tests: TDigest, SpanSearch,
// stringbuilder, CJson, TaggedHash, Log2

//////////////////////////////////////////////////////////////////////////


class TDigest : public ::testing::Test
{

protected:
	virtual void TearDown ()
	{
		SafeDelete ( pDigest );
	}

	virtual void SetUp ()
	{
		sphSrand ( 0 );
		pDigest = sphCreateTDigest ();
	}

	TDigest_i * pDigest;
};

TEST_F ( TDigest, simple )
{
	for ( int i = 1; i<=100; i++ )
		pDigest->Add ( i, 1 );

	ASSERT_NEAR ( pDigest->Percentile ( 50 ), 51, 1 );
	ASSERT_NEAR ( pDigest->Percentile ( 95 ), 96, 1 );
	ASSERT_NEAR ( pDigest->Percentile ( 99 ), 100, 1 );
}


TEST_F ( TDigest, dupes )
{
	for ( int j = 0; j<3; j++ )
		for ( int i = 0; i<10000; i++ )
			pDigest->Add ( i / 100 + 1, 1 );

	ASSERT_NEAR ( pDigest->Percentile ( 50 ), 51, 1 );
	ASSERT_NEAR ( pDigest->Percentile ( 95 ), 96, 1 );
	ASSERT_NEAR ( pDigest->Percentile ( 99 ), 100, 1 );
}


TEST_F ( TDigest, compression )
{
	for ( int i = 0; i<10000; i++ )
		pDigest->Add ( i + 1, 1 );

	ASSERT_NEAR ( pDigest->Percentile ( 50 ), 5001, 1.5 );
	ASSERT_NEAR ( pDigest->Percentile ( 95 ), 9501, 1.5 );
	ASSERT_NEAR ( pDigest->Percentile ( 99 ), 9901, 1.5 );
}

//////////////////////////////////////////////////////////////////////////

TEST ( Misc, SpanSearch )
{
	CSphVector<int> dVec;
	dVec.Add ( 1 );
	dVec.Add ( 3 );
	dVec.Add ( 4 );

	ASSERT_EQ ( FindSpan ( dVec, 1, 5 ), 0 );
	ASSERT_EQ ( FindSpan ( dVec, 3, 5 ), 1 );
	ASSERT_EQ ( FindSpan ( dVec, 4, 5 ), 2 );
	ASSERT_EQ ( FindSpan ( dVec, 0, 5 ), -1 );
	ASSERT_EQ ( FindSpan ( dVec, 11, 5 ), 2 );

	dVec.Add ( 15 );
	dVec.Add ( 17 );
	dVec.Add ( 22 );
	dVec.Add ( 23 );

	ASSERT_EQ ( FindSpan ( dVec, 1, 5 ), 0 );
	ASSERT_EQ ( FindSpan ( dVec, 18, 5 ), 4 );
	ASSERT_EQ ( FindSpan ( dVec, 23, 5 ), 6 );
	ASSERT_EQ ( FindSpan ( dVec, 0, 5 ), -1 );
	ASSERT_EQ ( FindSpan ( dVec, 31, 5 ), 6 );
}

//////////////////////////////////////////////////////////////////////////

// test on usual +=, <<.
TEST ( functions, stringbuilder_hello )
{
	StringBuilder_c builder;

	// += of const char string
	builder += "Hello";

	// << for const char*
	builder << " " << "world!";
	ASSERT_STREQ ( builder.cstr (), "Hello world!" );

	// << for CSphString
	CSphString s ("I am");
	builder << s;
	ASSERT_STREQ ( builder.cstr (), "Hello world!I am" );

	// << for vec of chars
	CSphVector<char> dText;
	dText.Add('a'); dText.Add('b'); dText.Add('c');
	builder << dText;
	ASSERT_STREQ ( builder.cstr (), "Hello world!I amabc" );

	// AppendChunk of blob
	const char* sText = "text";
	builder.AppendChunk ( {sText, (int) strlen ( sText )});
	ASSERT_STREQ ( builder.cstr (), "Hello world!I amabctext" );

	// AppendChunk with quotation
	builder.AppendChunk ( {sText, (int) strlen ( sText )}, '`' );
	ASSERT_STREQ ( builder.cstr (), "Hello world!I amabctext`text`" );

	// AppendString
	builder.AppendString (s);
	ASSERT_STREQ ( builder.cstr (), "Hello world!I amabctext`text`I am" );

	// AppendString quoted
	builder.AppendString ( s, '_' );
	ASSERT_STREQ ( builder.cstr (), "Hello world!I amabctext`text`I am_I am_" );
}

// test for scoped-comma modifier.
// comma will automatically append '; ' between ops.
TEST ( functions, stringbuilder_simplescoped )
{
	StringBuilder_c builder;
	auto tComma = ScopedComma_c (builder,"; ");

	builder += "one";
	builder << "two" << "three";
	builder.Appendf ("four: %d", 4);
	builder << "five";
	ASSERT_STREQ ( builder.cstr (), "one; two; three; four: 4; five" );
}

// scoped comma with prefix (will prepend prefix before first op)
// nested comma (will use other behaviour in isolated scope)
TEST ( functions, stringbuilder_scopedprefixed )
{
	StringBuilder_c builder;
	auto tC = ScopedComma_c ( builder, "; ", "List: " );

	builder += "one";
	builder << "two" << "three";
	ASSERT_STREQ ( builder.cstr (), "List: one; two; three" ) << "plain insert into managed";
	{
		auto tI = ScopedComma_c ( builder, ": ", "{", "}" );
		builder << "first" << "second";
		ASSERT_STREQ ( builder.cstr (), "List: one; two; three; {first: second" ) << "nested managed insert";
	}
	ASSERT_STREQ ( builder.cstr (), "List: one; two; three; {first: second}" ) << "nested managed insert terminated";
	builder.Appendf ( "four: %d", 4 );
	builder << "five";
	ASSERT_STREQ ( builder.cstr (), "List: one; two; three; {first: second}; four: 4; five" );
}

// standalone comma. Not necesssary related to stringbuilder, but live alone.
TEST ( functions, stringbuilder_standalone )
{
	StringBuilder_c builder;
	Comma_c tComma; // default is ', '
	builder << tComma << "one";
	builder << tComma << "two";
	builder << tComma << "three";
	ASSERT_STREQ ( builder.cstr (), "one, two, three" );
}

// standalone comma. Not necesssary related to stringbuilder, but live alone.
TEST ( functions, stringbuilder_numprint )
{
	StringBuilder_c builder;
	builder << "sa" << 10 << 10ll;
	ASSERT_STREQ ( builder.cstr(), "sa1010" );
	builder.Clear();

	builder << 1.1f;
	ASSERT_STREQ ( builder.cstr(), "1.100000" );
}


// standalone comma. Not necesssary related to stringbuilder, but live alone.
TEST ( functions, stringbuilder_intprint )
{
	StringBuilder_c builder;
	builder << "sa";
	builder << 10;
	builder << 10ll;
	ASSERT_STREQ ( builder.cstr(), "sa1010" );
}


// many nested scoped commas and 'StartBlock' modifier
// (scoped comma is the same as pair 'StartBlock...FinishBlock')
TEST ( functions, stringbuilder_nested )
{
	StringBuilder_c builder;
	builder << "one, two, three";
	ScopedComma_c lev0 ( builder );
	{
		ScopedComma_c lev1 ( builder, ", ", "[", "]" );
		builder.StartBlock ( ": ", "(", ")" );
		builder.StartBlock ( ";", "{", "}" );
		ASSERT_STREQ ( builder.cstr (), "one, two, three" ) << "simple blocks do nothing";
		builder << "first" << "second" << "third";
		ASSERT_STREQ ( builder.cstr (), "one, two, three[({first;second;third" ) << "unclosed block";
	}
	// note that only 'lev1' is destroyed, we didn't explicitly finished two nested blocks.
	// but they're finished implicitly
	ASSERT_STREQ ( builder.cstr (), "one, two, three[({first;second;third})]" ) << "closed block";
	builder << "four";

	// note, we doesn't destroy outer comma lev0, but this is not necessary since it doesn't have a suffix.
	ASSERT_STREQ ( builder.cstr (), "one, two, three[({first;second;third})], four" ) << "finished block with tail";
}

// pure StartBlock..FinishBlock test
TEST ( functions, stringbuilder_autoclose )
{
	StringBuilder_c builder ( ": ", "[", "]" );
	// note that there is no ': ' suffixed at the end (since comma only between blocks)
	builder << "one" << "two";
	ASSERT_STREQ ( builder.cstr (), "[one: two" ) << "simple pushed block";

	// starting block doesn't mean any output yet, so content is the same
	builder.StartBlock(",","(",")");
	ASSERT_STREQ ( builder.cstr (), "[one: two" ) << "simple pushed block";

	// note that now ': ' of outer block prepended to the suffix '(' of the current block.
	builder << "abc" << "def";
	ASSERT_STREQ ( builder.cstr (), "[one: two: (abc,def" ) << "simple pushed block 2";

	// finishing block mean that suffix appended, if the state is different from initial
	builder.FinishBlock ();
	ASSERT_STREQ ( builder.cstr (), "[one: two: (abc,def)" ) << "simple pushed block 2";
	builder.FinishBlock();
	ASSERT_STREQ ( builder.cstr (), "[one: two: (abc,def)]" ) << "simple pushed block 3";
}

// pure StartBlock..FinishBlock test with one empty block (it outputs nothing)
TEST ( functions, stringbuilder_close_of_empty )
{
	StringBuilder_c builder ( ": ", "[", "]" );

	// note that there is no ': ' suffixed at the end (since comma only between blocks)
	builder << "one" << "two";
	ASSERT_STREQ ( builder.cstr (), "[one: two" ) << "simple pushed block";

	// starting block doesn't output anything by itself, but modify future output
	builder.StartBlock ( ",", "(", ")" );
	ASSERT_STREQ ( builder.cstr (), "[one: two" ) << "started new block";

	// finishing of empty block outputs also nothing
	builder.FinishBlock ();
	ASSERT_STREQ ( builder.cstr (), "[one: two" ) << "finished empty block";

	// finishing non-empty block outputs suffix (and so, doesn't strictly necessary if no suffixes).
	builder.FinishBlock ();
	ASSERT_STREQ ( builder.cstr (), "[one: two]" ) << "final result";
}

// operation 'clear'. Not only wipe content, but also undo any comma state
TEST ( functions, stringbuilder_clear )
{
	StringBuilder_c builder ( ": ", "[", "]" );
	builder << "one" << "two";
	builder.StartBlock ( ",", "(", ")" );
	builder << "abc" << "def";
	builder.Clear();
	ASSERT_STREQ ( builder.cstr (), "" ) << "emtpy";
	builder << "one" << "two";
	ASSERT_STREQ ( builder.cstr (), "onetwo" ) << "nocommas";
	builder.FinishBlocks();
	ASSERT_STREQ ( builder.cstr (), "onetwo" ) << "nocommas";
}

// 'FinishBlocks()' - by default closes ALL opened blocks
TEST ( functions, stringbuilder_twoopenoneclose )
{
	StringBuilder_c builder ( ": ", "[", "]" );
	builder << "one" << "two";
	builder.StartBlock ( ",", "(", ")" );
	builder << "abc" << "def";
	builder.FinishBlocks ();
	ASSERT_STREQ ( builder.cstr (), "[one: two: (abc,def)]" ) << "simple pushed block 3";

}

// simple start/finish blocks manipulation - outputs nothing by alone
TEST ( functions, stringbuilder_finishnoopen )
{
	StringBuilder_c builder ( ":", "[", "]" );
	auto pLev = builder.StartBlock ( ";", "(", ")" );
	builder.StartBlock ( ",", "{", "}" );
	builder.FinishBlocks ( pLev );
	ASSERT_STREQ ( builder.cstr (), "" ) << "nothing outputed";
}

// FinishBlocks() to stored state
TEST ( functions, stringbuilder_ret_to_level )
{
	// outer block
	StringBuilder_c builder ( ":", "[", "]" );
	builder << "exone" << "extwo";

	// middle block - we memorize this state
	auto pLev = builder.StartBlock ( ";", "(", ")" );
	builder << "one" << "two";

	// internal block
	builder.StartBlock ( ",", "{", "}" );
	builder << "three" << "four";
	ASSERT_STREQ ( builder.cstr (), "[exone:extwo:(one;two;{three,four" );

	// finish memorized block and all blocks created after it
	builder.FinishBlocks ( pLev );
	ASSERT_STREQ ( builder.cstr (), "[exone:extwo:(one;two;{three,four})" );

	// it will output into most outer block, since others finished
	builder << "ex3";
	ASSERT_STREQ ( builder.cstr (), "[exone:extwo:(one;two;{three,four}):ex3" );

	// it will finish outer block (and close the bracket).
	builder.FinishBlocks();
	ASSERT_STREQ ( builder.cstr (), "[exone:extwo:(one;two;{three,four}):ex3]" ) << "test complete";
}

// simple test on Appendf
TEST ( functions, strinbguilder_appendf )
{
	StringBuilder_c sRes;
	sRes.Appendf ( "12345678" );
	ASSERT_STREQ ( sRes.cstr (), "12345678" );
	sRes.Appendf ( "this is my rifle this is my gun" );
	ASSERT_STREQ ( sRes.cstr (), "12345678this is my rifle this is my gun" );
	sRes.Appendf ( " int=%d float=%f string=%s", 123, 456.789, "helloworld" );
	ASSERT_STREQ (
		sRes.cstr (), "12345678this is my rifle this is my gun int=123 float=456.789000 string=helloworld" );
}

struct EscapeQuotation_t
{
	static const char cQuote = '\'';

	inline static bool IsEscapeChar ( char c )
	{
		return ( c=='\\' || c=='\'' );
	}

	inline static char GetEscapedChar ( char c )
	{
		return c;
	}
};

using QuotationEscapedBuilder = EscapedStringBuilder_T<EscapeQuotation_t>;

TEST( functions, EscapedStringBuilder )
{
	QuotationEscapedBuilder tBuilder;
	tBuilder.AppendEscaped ( "Hello" );
	ASSERT_STREQ ( tBuilder.cstr (), "'Hello'" );

	tBuilder.AppendEscaped ( " wo\\rl\'d" );
	ASSERT_STREQ ( tBuilder.cstr (), "'Hello'' wo\\\\rl\\'d'" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "wo\\rl\'d", EscBld::eFixupSpace );
	ASSERT_STREQ ( tBuilder.cstr (), "wo\\rl\'d" );

	// generic const char* with different escapes
	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eNone);
	ASSERT_STREQ ( tBuilder.cstr (), "space\t and\r 'tab'\n here" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace );
	ASSERT_STREQ ( tBuilder.cstr (), "space  and  'tab'  here" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eEscape );
	ASSERT_STREQ ( tBuilder.cstr (), "'space\t and\r \\'tab\\'\n here'" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here" );
	ASSERT_STREQ ( tBuilder.cstr (), "'space  and  \\'tab\\'  here'" );

	// nullptr with different escapes
	tBuilder.Clear ();
	tBuilder.AppendEscaped ( nullptr, EscBld::eNone );
	ASSERT_STREQ ( tBuilder.cstr (), "" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( nullptr, EscBld::eFixupSpace );
	ASSERT_STREQ ( tBuilder.cstr (), "" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( nullptr, EscBld::eEscape );
	ASSERT_STREQ ( tBuilder.cstr (), "''" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( nullptr, EscBld::eAll );
	ASSERT_STREQ ( tBuilder.cstr (), "''" );

	// empty with different escapes
	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "", EscBld::eNone );
	ASSERT_STREQ ( tBuilder.cstr (), "" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "", EscBld::eFixupSpace );
	ASSERT_STREQ ( tBuilder.cstr (), "" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "", EscBld::eEscape );
	ASSERT_STREQ ( tBuilder.cstr (), "''" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "", EscBld::eAll );
	ASSERT_STREQ ( tBuilder.cstr (), "''" );

	// len-defined blob
	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eNone, 10 );
	ASSERT_STREQ ( tBuilder.cstr (), "space\t and" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace, 10 );
	ASSERT_STREQ ( tBuilder.cstr (), "space  and" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eEscape, 10 );
	ASSERT_STREQ ( tBuilder.cstr (), "'space\t and'" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eAll, 10 );
	ASSERT_STREQ ( tBuilder.cstr (), "'space  and'" );

	// zero-len blob
	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eNone, 0 );
	ASSERT_STREQ ( tBuilder.cstr (), "" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace, 0 );
	ASSERT_STREQ ( tBuilder.cstr (), "" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eEscape, 0 );
	ASSERT_STREQ ( tBuilder.cstr (), "''" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eAll, 0 );
	ASSERT_STREQ ( tBuilder.cstr (), "''" );

	// len-defined blob exactly of given len, non z-terminated.
	// (valgrind would check nicely if it even try to touch a byte over allocated buf)
	char * buf = new char[5];
	memcpy (buf, "space", 5);

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( buf, EscBld::eNone, 5 );
	ASSERT_STREQ ( tBuilder.cstr (), "space" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( buf, EscBld::eFixupSpace, 5 );
	ASSERT_STREQ ( tBuilder.cstr (), "space" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( buf, EscBld::eEscape, 5 );
	ASSERT_STREQ ( tBuilder.cstr (), "'space'" );

	tBuilder.Clear ();
	tBuilder.AppendEscaped ( buf, EscBld::eAll, 5 );
	ASSERT_STREQ ( tBuilder.cstr (), "'space'" );

	delete[] buf;
}

void esc_first_comma ( const char* sText, BYTE eKind, const char* sProof )
{
	QuotationEscapedBuilder tBuilder;
	tBuilder.StartBlock ();
	tBuilder << "first";
	tBuilder.AppendEscaped ( sText, eKind );
	ASSERT_STREQ ( tBuilder.cstr (), sProof );
}

TEST( functions, EscapedStringBuilderAndCommas )
{
	// generic const char* with different escapes
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eNone, "first, space\t and\r 'tab'\n here" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace, "first, space  and  'tab'  here" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eEscape , "first, 'space\t and\r \\'tab\\'\n here'" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eAll, "first, 'space  and  \\'tab\\'  here'" );

	// null with different escapes
	esc_first_comma ( nullptr, EscBld::eNone, "first" );
	esc_first_comma ( nullptr, EscBld::eFixupSpace, "first" );
	esc_first_comma ( nullptr, EscBld::eEscape, "first, ''" );
	esc_first_comma ( nullptr, EscBld::eAll, "first, ''" );
}

TEST( functions, JsonNamedEssence )
{
	StringBuilder_c sRes (",","{","}");
	sRes << "hello";
	ASSERT_STREQ ( sRes.cstr (), "{hello" );
	sRes << "world";
	ASSERT_STREQ ( sRes.cstr (), "{hello,world" );
	sRes.AppendName ("bla");
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":" );
	sRes << "foo";
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo" );
	sRes << "bar";
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar" );
	sRes.AppendName("bar").Sprintf("%d",1000);
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000" );
	ScopedComma_c sOne (sRes, ";", "[", "]");
	sRes.AppendName("foo") << "bar";
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":bar" );
	sRes.SkipNextComma ();
	sRes << "baz";
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz" );
	sRes << "end";
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end" );
	sRes.FinishBlock ();
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end]" );
	sRes << "End";
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End" );
	sRes.AppendName("arr");
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":" );
	sRes.StartBlock("|","[","]");
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":" );
	sRes.FinishBlock(false);
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":[]" );
	sRes.AppendName ( "a" ).StartBlock ( "|", "[", "]" );
	sRes << "b";
	sRes.FinishBlock();
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":[],\"a\":[b]" );
	sRes.FinishBlock();
	ASSERT_STREQ ( sRes.cstr (), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":[],\"a\":[b]}" );
}

TEST( functions, EscapedStringBuilderAndSkipCommas )
{
	// generic const char* with different escapes, exclude comma
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eNone | EscBld::eSkipComma
					  , "firstspace\t and\r 'tab'\n here" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace | EscBld::eSkipComma
					  , "firstspace  and  'tab'  here" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eEscape | EscBld::eSkipComma
					  , "first'space\t and\r \\'tab\\'\n here'" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eAll | EscBld::eSkipComma
					  , "first'space  and  \\'tab\\'  here'" );

	// null with different escapes, exclude comma
	esc_first_comma ( nullptr, EscBld::eNone | EscBld::eSkipComma, "first" );
	esc_first_comma ( nullptr, EscBld::eFixupSpace | EscBld::eSkipComma, "first" );
	esc_first_comma ( nullptr, EscBld::eEscape | EscBld::eSkipComma, "first''" );
	esc_first_comma ( nullptr, EscBld::eAll | EscBld::eSkipComma, "first''" );

}


//////////////////////////////////////////////////////////////////////////

TEST( functions, TaggedHash20_t )
{
	const char * sFIPS = "45f44fd2db02b08b4189abf21e90edd712c9616d *rt_full.ram\n";
	const BYTE bytescheck[HASH20_SIZE] = { 0x45, 0xf4, 0x4f, 0xd2, 0xdb, 0x02, 0xb0, 0x8b, 0x41, 0x89, 0xab, 0xf2
										   , 0x1e, 0x90, 0xed, 0xd7, 0x12, 0xc9, 0x61, 0x6d };
	const char * namecheck = "rt_full.ram";

	TaggedHash20_t tHash ( "HelloFips" );
	CSphString sFips = tHash.ToFIPS ();

	EXPECT_TRUE ( sFips=="" );

	tHash.FromFIPS ( sFIPS );
	ASSERT_TRUE ( tHash.m_sTagName==namecheck );

	ASSERT_TRUE ( 0==memcmp ( tHash.m_dHashValue, bytescheck, HASH20_SIZE ) );

	sFips = tHash.ToFIPS ();
	ASSERT_TRUE ( sFips==sFIPS );

	TaggedHash20_t tHash2 ( namecheck, bytescheck );
	ASSERT_TRUE ( tHash2.ToFIPS ()==sFIPS );

}



//////////////////////////////////////////////////////////////////////////
unsigned int nlog2 ( uint64_t x )
{
	x |= ( x >> 1 );
	x |= ( x >> 2 );
	x |= ( x >> 4 );
	x |= ( x >> 8 );
	x |= ( x >> 16 );
	if ( x&0xFFFFFFFF00000000 )
		return 32+ sphBitCount(x>>32);
	return sphBitCount(x&0xFFFFFFFF);
}

TEST ( functions, Log2 )
{
	EXPECT_EQ ( sphLog2 ( 1 ), 1 );
	EXPECT_EQ ( sphLog2 ( 2 ), 2 );
	EXPECT_EQ ( sphLog2 ( 3 ), 2 );
	EXPECT_EQ ( sphLog2 ( 4 ), 3 );
	EXPECT_EQ ( sphLog2 ( 5 ), 3 );
	EXPECT_EQ ( sphLog2 ( 6 ), 3 );
	EXPECT_EQ ( sphLog2 ( 7 ), 3 );
	EXPECT_EQ ( sphLog2 ( 8 ), 4 );
	EXPECT_EQ ( sphLog2 ( 9 ), 4 );
	EXPECT_EQ ( sphLog2 ( 10 ), 4 );
	EXPECT_EQ ( sphLog2 ( 65535 ), 16 );
	EXPECT_EQ ( sphLog2 ( 65536 ), 17 );
	EXPECT_EQ ( sphLog2 ( 0xffffffffUL ), 32 );
	EXPECT_EQ ( sphLog2 ( 0x100000000ULL ), 33 );
	EXPECT_EQ ( sphLog2 ( 0x100000001ULL ), 33 );
	EXPECT_EQ ( sphLog2 ( 0x1ffffffffULL ), 33 );
	EXPECT_EQ ( sphLog2 ( 0x200000000ULL ), 34 );
	EXPECT_EQ ( sphLog2 ( 0xffffffffffffffffULL ), 64 );
	EXPECT_EQ ( sphLog2 ( 0xfffffffffffffffeULL ), 64 );
	EXPECT_EQ ( sphLog2 ( 0xefffffffffffffffULL ), 64 );
	EXPECT_EQ ( sphLog2 ( 0x7fffffffffffffffULL ), 63 );

	EXPECT_EQ ( nlog2 ( 1 ), 1 ) << "emulated";
	EXPECT_EQ ( nlog2 ( 2 ), 2 );
	EXPECT_EQ ( nlog2 ( 3 ), 2 );
	EXPECT_EQ ( nlog2 ( 4 ), 3 );
	EXPECT_EQ ( nlog2 ( 5 ), 3 );
	EXPECT_EQ ( nlog2 ( 6 ), 3 );
	EXPECT_EQ ( nlog2 ( 7 ), 3 );
	EXPECT_EQ ( nlog2 ( 8 ), 4 );
	EXPECT_EQ ( nlog2 ( 9 ), 4 );
	EXPECT_EQ ( nlog2 ( 10 ), 4 );
	EXPECT_EQ ( nlog2 ( 65535 ), 16 );
	EXPECT_EQ ( nlog2 ( 65536 ), 17 );
	EXPECT_EQ ( nlog2 ( 0xffffffffUL ), 32 );
	EXPECT_EQ ( nlog2 ( 0x100000000ULL ), 33 );
	EXPECT_EQ ( nlog2 ( 0x100000001ULL ), 33 );
	EXPECT_EQ ( nlog2 ( 0x1ffffffffULL ), 33 );
	EXPECT_EQ ( nlog2 ( 0x200000000ULL ), 34 );
	EXPECT_EQ ( nlog2 ( 0xffffffffffffffffULL ), 64 );
	EXPECT_EQ ( nlog2 ( 0xfffffffffffffffeULL ), 64 );
	EXPECT_EQ ( nlog2 ( 0xefffffffffffffffULL ), 64 );
	ASSERT_EQ ( sphLog2 ( 0x7fffffffffffffffULL ), 63 );
}

//////////////////////////////////////////////////////////////////////////

CSphMutex g_Mutex1;

void TimedLockTest ( void * )
{
	ASSERT_FALSE ( g_Mutex1.TimedLock ( 1000 ) ) << "timedlock attempt 1";
	ASSERT_FALSE ( g_Mutex1.TimedLock ( 1000 ) ) << "timedlock attempt 2";
	ASSERT_FALSE ( g_Mutex1.TimedLock ( 1000 ) ) << "timedlock attempt 3";
	ASSERT_TRUE ( g_Mutex1.TimedLock ( 1000 ) ) << "timedlock attempt 4";
	ASSERT_TRUE ( g_Mutex1.Unlock () );
}

TEST (functions, Mutex)
{
	SphThread_t th;
	ASSERT_TRUE ( g_Mutex1.Lock () ) << "locked";
	ASSERT_TRUE ( sphThreadCreate ( &th, TimedLockTest, NULL ) ) << "timedlock thread created";
	sphSleepMsec ( 3500 );
	ASSERT_TRUE ( g_Mutex1.Unlock () ) << "unlocked";
	ASSERT_TRUE ( sphThreadJoin ( &th ) ) << "timedlock thread done";
}

//////////////////////////////////////////////////////////////////////////

static int g_iRwlock;
static CSphRwlock g_tRwlock;

void RwlockReader ( void * pArg )
{
	ASSERT_TRUE ( g_tRwlock.ReadLock () );
	sphSleepMsec ( 10 );
	*( int * ) pArg = g_iRwlock;
	ASSERT_TRUE ( g_tRwlock.Unlock () );
}

void RwlockWriter ( void * pArg )
{
	ASSERT_TRUE ( g_tRwlock.WriteLock () );
	g_iRwlock += size_t ( pArg );
	sphSleepMsec ( 3 );
	ASSERT_TRUE ( g_tRwlock.Unlock () );
}

TEST ( functions, RWLock )
{
	ASSERT_TRUE ( g_tRwlock.Init () );

	const int NPAIRS = 10;
	SphThread_t dReaders[NPAIRS];
	SphThread_t dWriters[NPAIRS];
	int iRead[NPAIRS];

	g_iRwlock = 0;
	for ( int i = 0; i<NPAIRS; i++ )
	{
		ASSERT_TRUE ( sphThreadCreate ( &dReaders[i], RwlockReader, ( void * ) &iRead[i] ) );
		ASSERT_TRUE ( sphThreadCreate ( &dWriters[i], RwlockWriter, reinterpret_cast<void *>(1 + i) ) );
	}

	for ( int i = 0; i<NPAIRS; i++ )
	{
		ASSERT_TRUE ( sphThreadJoin ( &dReaders[i] ) );
		ASSERT_TRUE ( sphThreadJoin ( &dWriters[i] ) );
	}

	ASSERT_EQ ( g_iRwlock, NPAIRS * ( 1 + NPAIRS ) / 2 );
	ASSERT_TRUE ( g_tRwlock.Done () );

	int iReadSum = 0;
	for ( int i = 0; i<NPAIRS; i++ )
		iReadSum += iRead[i];

	RecordProperty ( "read_sum", iReadSum );
}

//////////////////////////////////////////////////////////////////////////

CSphAutoEvent g_multievent;
OneshotEvent_c g_oneevent;
volatile int64_t tmNow;

int getms()
{
	return (sphMicroTimer () - tmNow)/1000;
}

void AutoEventest ( void* )
{
	printf("\n%d thread started", getms());
	for ( int i=0; i<5; ++i)
	{
		ASSERT_TRUE ( g_multievent.WaitEvent ()) << "WaitEvent";
		printf ( "\n%d B%d: %d-st event waited", getms (),i+1,i);
	}
}

void AutoEventestTimed ( void* )
{
	printf ( "\n%d B1: started", getms ());
	for ( int i = 0; i<5; ++i )
	{
		ASSERT_TRUE ( g_multievent.WaitEvent (500)) << "WaitEvent";
		printf ( "\n%d B%d: %d-st event waited", getms (), i+2, i );
	}
	ASSERT_FALSE ( g_multievent.WaitEvent ( 200 )) << "WaitEvent";
	printf ( "\n%d B7: one event timed-out", getms () );
	ASSERT_TRUE ( g_multievent.WaitEvent ( 500 )) << "WaitEvent";
	ASSERT_TRUE ( g_multievent.WaitEvent ( 500 )) << "WaitEvent";
	printf ( "\n%d B8: last 2 events succeeded", getms ());
}

// multievent - we can set it N times, and then it may be waited N times also, but N+1 will block
TEST ( functions, MultiAutoEvent )
{
	tmNow = sphMicroTimer ();
	SphThread_t th;

	// set 4 events before event start the thread. Expect, it will be catched.
	printf ( "\n%d A1: set event 4 times a row", getms ());
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();


	// now start the thread, it will receive events
	ASSERT_TRUE ( sphThreadCreate ( &th, AutoEventest, nullptr )) << "autoevent thread created";
	printf ( "\n%d A2: created working thread", getms () );
	// sleep half-a-second and set last event.
	sphSleepMsec ( 500 );
	printf ( "\n%d A3: set event", getms () );
	g_multievent.SetEvent ();

	sphSleepMsec ( 100 );
	ASSERT_TRUE ( sphThreadJoin ( &th )) << "autoevent thread done";
}

TEST ( functions, MultiAutoEventTimed )
{
	tmNow=sphMicroTimer ();
	SphThread_t th;

	// set 4 events before event start the thread. Expect, it will be catched.
	printf ( "\n%d 4 events set", getms () );
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();

	// now start the thread, it will receive events
	ASSERT_TRUE ( sphThreadCreate ( &th, AutoEventestTimed, nullptr ) ) << "autoeventtimed thread created";
	printf ( "\n%d A2: created working thread", getms () );

	// sleep half-a-second and set last event.
	sphSleepMsec ( 100 );
	printf ( "\n%d A3: set event", getms () );
	g_multievent.SetEvent ();
	sphSleepMsec ( 400 );
	printf ( "\n%d A4: set event twice", getms () );
	g_multievent.SetEvent ();
	g_multievent.SetEvent ();
	sphSleepMsec ( 100 );
	ASSERT_TRUE ( sphThreadJoin ( &th ) ) << "autoevent thread done";
}

void OneshotEventTest ( void* )
{
	printf ( "\n%d thread started", getms ());
	for ( int i = 0; i<2; ++i )
	{
		ASSERT_TRUE ( g_oneevent.WaitEvent ()) << "WaitEvent";
		printf ( "\n%d thread %d-st event waited", getms (), i );
	}
}

void OneshotEventTestTimed ( void* )
{
	printf ( "\n%d B1: started", getms ());
	bool bRes=g_oneevent.WaitEvent ( 500 ); ASSERT_TRUE ( bRes ) << "WaitEvent";
	printf ( "\n%d B2: 1-st event returned %s", getms (), bRes?"true":"false" );
	bRes=g_oneevent.WaitEvent ( 500 ); ASSERT_TRUE ( bRes ) << "WaitEvent";
	printf ( "\n%d B3: 2-nd event returned %s", getms (), bRes?"true":"false" );
	bRes=g_oneevent.WaitEvent ( 500 ); ASSERT_TRUE ( bRes ) << "WaitEvent";
	printf ( "\n%d B4: 3-rd event returned %s", getms (), bRes?"true":"false" );
	sphSleepMsec ( 100 );
	bRes=g_oneevent.WaitEvent ( 500 ); ASSERT_FALSE ( bRes ) << "WaitEvent";
	printf ( "\n%d B5: 4-th event returned %s", getms (), bRes?"true":"false" );
}

// oneshot event - we can set it N times, but only once it waited, and then will block.
TEST ( functions, OneshotAutoEvent )
{
	tmNow = sphMicroTimer ();
	SphThread_t th;

	// set 4 events before event start the thread. Expect, it will be catched.
	printf ( "\n%d A1: set event 4 times a row", getms ());
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();

	// now start the thread, it will receive events
	ASSERT_TRUE ( sphThreadCreate ( &th, OneshotEventTest, nullptr )) << "autoevent thread created";
	printf ( "\n%d A2: created working thread", getms ());

	// sleep half-a-second and set last event.
	sphSleepMsec ( 500 );
	printf ( "\n%d A3: set event", getms ());
	g_oneevent.SetEvent ();
	sphSleepMsec ( 100 );
	ASSERT_TRUE ( sphThreadJoin ( &th )) << "autoevent thread done";
}

// oneshot event - we can set it N times, but only once it waited, and then will block.
TEST ( functions, OneshotAutoEventTimed )
{
	tmNow=sphMicroTimer ();
	SphThread_t th;
	
	// set 4 events before event start the thread. Expect, it will be catched.
	printf ( "\n%d A1: set event 4 times a row", getms () );
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();

	// now start the thread, it will receive events
	ASSERT_TRUE ( sphThreadCreate ( &th, OneshotEventTestTimed, nullptr ) ) << "autoevent thread created";
	printf ( "\n%d A2: created working thread", getms () );

	// sleep half-a-second and set last event.
	sphSleepMsec ( 100 );
	printf ( "\n%d A3: set event", getms () );
	g_oneevent.SetEvent ();
	sphSleepMsec ( 400 );
	printf ( "\n%d A4: set event twice", getms () );
	g_oneevent.SetEvent ();
	g_oneevent.SetEvent ();
	sphSleepMsec ( 100 );
	ASSERT_TRUE ( sphThreadJoin ( &th ) ) << "autoevent thread done";
}

//////////////////////////////////////////////////////////////////////////

void CleanupCallback ( void * pArg )
{
	*( bool * ) pArg = true;
}

void CleanupThread ( void * pArg )
{
	sphThreadOnExit ( CleanupCallback, pArg );
}

TEST ( functions, Cleanup )
{
	const int CLEANUP_COUNT = 10;
	bool bCleanup[CLEANUP_COUNT];
	for ( auto & bClean : bCleanup )
		bClean = false;

	SphThread_t thd[CLEANUP_COUNT];
	for ( int i = 0; i<CLEANUP_COUNT; i++ )
		ASSERT_TRUE ( sphThreadCreate ( &thd[i], CleanupThread, &bCleanup[i] ) );

	for ( auto & th : thd )
		ASSERT_TRUE ( sphThreadJoin ( &th ) );

	for ( auto & bClean : bCleanup )
		ASSERT_TRUE ( bClean );
}

//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma warning(push) // store current warning values
#pragma warning(disable:4101)
#endif

TEST ( functions, Hash_simple )
{
	// add and verify a couple keys manually
	OpenHash_T<int, int64_t, HashFunc_Int64_t> h;
	int &a = h.Acquire ( 123 );
	ASSERT_FALSE ( a );
	a = 1;
	int &b = h.Acquire ( 234 );
	ASSERT_FALSE ( b);
	b = 2;
	ASSERT_TRUE ( h.Find ( 123 ) );
	ASSERT_EQ ( *h.Find ( 123 ), 1 );
	ASSERT_TRUE ( h.Find ( 234 ) );
	ASSERT_EQ ( *h.Find ( 234 ), 2 );
	ASSERT_FALSE ( h.Find ( 345 ) );

	// add several pairs of colliding keys
	const int DUPES = 8;
	int64_t dupes[DUPES*2] = {
		54309970105, 55904555634,
		54386834629, 61870972983,
		54789062086, 8033211121,
		41888995393, 69125167042,
		18878807922, 3782313558,
		31939787707, 58687170065,
		36013093500, 57976719271,
		35732429300, 67391785901
	};

	HashFunc_Int64_t tHashFunc;
	for ( int i = 0; i<2 * DUPES; i++ )
	{
		ASSERT_EQ ( tHashFunc.GetHash ( dupes[i] ), tHashFunc.GetHash ( dupes[( i >> 1 ) << 1] ) );
		int &x = h.Acquire ( dupes[i] );
		ASSERT_FALSE ( x );
		x = 100 + i;
	}

	// verify that colliding keys hashed differently
	for ( int i = 0; i<2 * DUPES; i++ )
		ASSERT_EQ ( *h.Find ( dupes[i] ), 100 + i );

	// verify that Add() attempts fail
	for ( int i = 0; i<2 * DUPES; i++ )
		ASSERT_FALSE ( h.Add ( dupes[i], 567 ) );

	// delete every 1st colliding key
	for ( int i = 0; i<2 * DUPES; i += 2 )
		h.Delete ( dupes[i] );

	// verify that 1st colliding key got deleted
	for ( int i = 0; i<2 * DUPES; i += 2 )
		ASSERT_FALSE ( h.Find ( dupes[i] ) );

	// verify that 2nd colliding key still works ok
	for ( int i = 1; i<2 * DUPES; i += 2 )
		ASSERT_EQ ( *h.Find ( dupes[i] ), 100 + i );
}

#ifdef _WIN32
#pragma warning(pop) // restore warnings
#endif

TEST ( functions, HASH_randomized )

// big randomized test
{
	OpenHash_T<int, int64_t, HashFunc_Int64_t> h;
	const int NVALS = 996146; // 0.95f out of 1M

	// add N numbers
	sphSrand ( 0 );
	for ( int i = 0; i<NVALS; i++ )
	{
		uint64_t k = sphRand ();
		k = ( k << 32 ) + sphRand ();
		h.Acquire ( k ) = i;
	}

	// verify that everything looks up as expected
	sphSrand ( 0 );
	for ( int i = 0; i<NVALS; i++ )
	{
		uint64_t k = sphRand ();
		k = ( k << 32 ) + sphRand ();
		ASSERT_EQ ( h.Acquire ( k ), i );
	}

	// delete every 3rd number
	sphSrand ( 0 );
	for ( int i = 0; i<NVALS; i++ )
	{
		uint64_t k = sphRand ();
		k = ( k << 32 ) + sphRand ();
		if ( !( i % 3 ) )
			h.Delete ( k );
	}

	// verify that everything looks up as expected
	sphSrand ( 0 );
	for ( int i = 0; i<NVALS; i++ )
	{
		uint64_t k = sphRand ();
		k = ( k << 32 ) + sphRand ();
		if ( i % 3 )
			ASSERT_EQ ( *h.Find ( k ), i );
		else
			ASSERT_FALSE ( h.Find ( k ) );
	}
}

//////////////////////////////////////////////////////////////////////////

TEST ( functions, string_split )
{
	StrVec_t dStr;
	sphSplit ( dStr, "test:me\0off\0", ":" );
	ASSERT_EQ ( dStr.GetLength (), 2 );
	ASSERT_STREQ ( dStr[0].cstr(),"test" );
	ASSERT_STREQ ( dStr[1].cstr(), "me" );

	dStr.Reset();
	sphSplit ( dStr, "  white\tspace\rsplit\ntrying ");
	ASSERT_EQ ( dStr.GetLength (), 4 );
	ASSERT_STREQ ( dStr[0].cstr (), "white" );
	ASSERT_STREQ ( dStr[1].cstr (), "space" );
	ASSERT_STREQ ( dStr[2].cstr (), "split" );
	ASSERT_STREQ ( dStr[3].cstr (), "trying" );

	dStr.Reset();
	sphSplit ( dStr, ":start:finish:", ":" );
	ASSERT_EQ ( dStr.GetLength (), 3 );
	ASSERT_STREQ ( dStr[0].cstr (), "" );
	ASSERT_STREQ ( dStr[1].cstr (), "start" );
	ASSERT_STREQ ( dStr[2].cstr (), "finish" );
}

//////////////////////////////////////////////////////////////////////////

struct TestAccCmp_fn
{
	typedef DWORD MEDIAN_TYPE;
	typedef DWORD * PTR_TYPE;

	int m_iStride;

	explicit TestAccCmp_fn ( int iStride )
		: m_iStride ( iStride )
	{}

	DWORD Key ( DWORD * pData ) const
	{
		return *pData;
	}

	void CopyKey ( DWORD * pMed, DWORD * pVal ) const
	{
		*pMed = Key ( pVal );
	}

	bool IsLess ( DWORD a, DWORD b ) const
	{
		return a<b;
	}

	void Swap ( DWORD * a, DWORD * b ) const
	{
		for ( int i = 0; i<m_iStride; i++ )
			::Swap ( a[i], b[i] );
	}

	DWORD * Add ( DWORD * p, int i ) const
	{
		return p + i * m_iStride;
	}

	int Sub ( DWORD * b, DWORD * a ) const
	{
		return ( int ) ( ( b - a ) / m_iStride );
	}

	bool IsKeyDataSynced ( const DWORD * pData ) const
	{
		DWORD uKey = *pData;
		DWORD uHash = GenerateKey ( pData );
		return uKey==uHash;
	}

	DWORD GenerateKey ( const DWORD * pData ) const
	{
		return m_iStride>1 ? sphCRC32 ( pData + 1, ( m_iStride - 1 ) * 4 ) : ( *pData );
	}
};


static bool IsSorted ( DWORD * pData, int iCount, const TestAccCmp_fn &fn )
{
	if ( iCount<1 )
		return true;

	const DWORD * pPrev = pData;
	if ( !fn.IsKeyDataSynced ( pPrev ) )
		return false;

	if ( iCount<2 )
		return true;

	for ( int i = 1; i<iCount; ++i )
	{
		const DWORD * pCurr = fn.Add ( pData, i );

		if ( fn.IsLess ( *pCurr, *pPrev ) || !fn.IsKeyDataSynced ( pCurr ) )
			return false;

		pPrev = pCurr;
	}

	return true;
}

void RandomFill ( DWORD * pData, int iCount, const TestAccCmp_fn &fn, bool bChainsaw )
{
	for ( int i = 0; i<iCount; ++i )
	{
		DWORD * pCurr = fn.Add ( pData, i );
		const DWORD * pNext = fn.Add ( pData, i + 1 );

		DWORD * pElem = pCurr;
		DWORD * pChainHill = bChainsaw && ( i % 2 ) ? fn.Add ( pData, i - 1 ) : NULL;
		do
		{
			*pElem = pChainHill ? *pChainHill / 2 : sphRand ();
			++pElem;
			pChainHill = pChainHill ? pChainHill + 1 : pChainHill;
		} while ( pElem!=pNext );

		*pCurr = fn.GenerateKey ( pCurr );
	}
}

void TestStridedSortPass ( int iStride, int iCount )
{
	ASSERT_TRUE ( iStride );
	ASSERT_TRUE  ( iCount );

	DWORD * pData = new DWORD[iCount * iStride];
	ASSERT_TRUE ( pData );

	// checked elements are random
	memset ( pData, 0, sizeof ( DWORD ) * iCount * iStride );
	TestAccCmp_fn fnSort ( iStride );
	RandomFill ( pData, iCount, fnSort, false );

	// crash on sort of mini-arrays
	TestAccCmp_fn fnSortDummy ( 1 );
	DWORD dMini[1] = { 1 };
	sphSort ( dMini, 1, fnSortDummy, fnSortDummy );
	sphSort ( dMini, 0, fnSortDummy, fnSortDummy );
	ASSERT_TRUE ( IsSorted ( dMini, 1, fnSortDummy ) );

	// random sort
	sphSort ( pData, iCount, fnSort, fnSort );
	ASSERT_TRUE ( IsSorted ( pData, iCount, fnSort ) );

	// already sorted sort
	sphSort ( pData, iCount, fnSort, fnSort );
	ASSERT_TRUE ( IsSorted ( pData, iCount, fnSort ) );

	// reverse order sort
	for ( int i = 0; i<iCount; ++i )
	{
		::Swap ( pData[i], pData[iCount - i - 1] );
	}
	sphSort ( pData, iCount, fnSort, fnSort );
	ASSERT_TRUE ( IsSorted ( pData, iCount, fnSort ) );

	// random chainsaw sort
	RandomFill ( pData, iCount, fnSort, true );
	sphSort ( pData, iCount, fnSort, fnSort );
	ASSERT_TRUE ( IsSorted ( pData, iCount, fnSort ) );

	SafeDeleteArray ( pData );
}

TEST ( functions, StridedSort )
{
	SCOPED_TRACE ( "stride 1, count 2" ); TestStridedSortPass ( 1, 2 );
	SCOPED_TRACE ( "stride 3, count 2" ); TestStridedSortPass ( 3, 2 );
	SCOPED_TRACE ( "stride 37, count 2" ); TestStridedSortPass ( 37, 2 );

	// SMALL_THRESH case
	SCOPED_TRACE ( "stride 1, count 30" ); TestStridedSortPass ( 1, 30 );
	SCOPED_TRACE ( "stride 7, count 13" ); TestStridedSortPass ( 7, 13 );
	SCOPED_TRACE ( "stride 113, count 5" ); TestStridedSortPass ( 113, 5 );

	SCOPED_TRACE ( "stride 1, count 1000" ); TestStridedSortPass ( 1, 1000 );
	SCOPED_TRACE ( "stride 5, count 1000" ); TestStridedSortPass ( 5, 1000 );
	SCOPED_TRACE ( "stride 17, count 50" ); TestStridedSortPass ( 17, 50 );
	SCOPED_TRACE ( "stride 31, count 1367" ); TestStridedSortPass ( 31, 1367 );

	SCOPED_TRACE ( "random strides" );
	// rand cases
	for ( int i = 0; i<10; ++i )
	{
		const int iRndStride = sphRand () % 64;
		const int iNrmStride = Max ( iRndStride, 1 );
		const int iRndCount = sphRand () % 1000;
		const int iNrmCount = Max ( iRndCount, 1 );
		TestStridedSortPass ( iNrmStride, iNrmCount );
	}
}

TEST ( functions, StridedSort_regressions )
{
	// regression of uniq vs empty array
	DWORD dUniq[] = { 1, 1, 3, 1 };
	int iCount = sizeof ( dUniq ) / sizeof ( dUniq[0] );
	ASSERT_FALSE ( sphUniq ( dUniq, 0 ) );
	sphSort ( dUniq, iCount );
	ASSERT_EQ ( sphUniq ( dUniq, iCount ), 2);
	ASSERT_EQ ( dUniq[0], 1);
	ASSERT_EQ ( dUniq[1], 3 );

	CSphVector<DWORD> dUniq1;
	dUniq1.Uniq ();
	ASSERT_FALSE ( dUniq1.GetLength () );
	dUniq1.Add ( 1 );
	dUniq1.Add ( 3 );
	dUniq1.Add ( 1 );
	dUniq1.Add ( 1 );
	dUniq1.Uniq ();
	ASSERT_EQ ( dUniq1.GetLength (), 2 );
	ASSERT_EQ ( dUniq1[0], 1 );
	ASSERT_EQ ( dUniq1[1], 3 );
}

//////////////////////////////////////////////////////////////////////////

TEST ( functions, Writer )
{
	const CSphString sTmpWriteout = "__writeout.tmp";
	CSphString sErr;

	static const auto WRITE_OUT_DATA_SIZE = 0x40000;
	BYTE * pData = new BYTE[WRITE_OUT_DATA_SIZE];
	memset ( pData, 0xfe, WRITE_OUT_DATA_SIZE );
	{
		CSphWriter tWrDef;
		tWrDef.OpenFile ( sTmpWriteout, sErr );
		tWrDef.PutBytes ( pData, WRITE_OUT_DATA_SIZE );
		tWrDef.PutByte ( 0xff );
	}
	{
		CSphWriter tWr;
		tWr.SetBufferSize ( WRITE_OUT_DATA_SIZE );
		tWr.OpenFile ( sTmpWriteout, sErr );
		tWr.PutBytes ( pData, WRITE_OUT_DATA_SIZE );
		tWr.PutByte ( 0xff );
	}
	unlink ( sTmpWriteout.cstr () );
	delete[] pData;
}

//////////////////////////////////////////////////////////////////////////
struct tstcase { float wold; DWORD utimer; float wnew; };

static void TestRebalance_fn ( tstcase * pData, int iLen, int iStride )
{
	ASSERT_FALSE ( iLen % iStride );
	iLen /= iStride;
	CSphFixedVector<int64_t> dTimers ( iStride );
	CSphFixedVector<float> dWeights ( iStride );
	for ( int i = 0; i<iLen; ++i )
	{
		for ( int j = 0; j<iStride; ++j )
		{
			dWeights[j] = pData[i * iStride + j].wold;
			dTimers[j] = pData[i * iStride + j].utimer;
		}

		RebalanceWeights ( dTimers, dWeights );

		for ( int j = 0; j<iStride; ++j )
		{
			ASSERT_NEAR ( dWeights[j], pData[i * iStride + j].wnew, 0.01)
				<< " \n----dWeights[" << j << "]=" << dWeights[j] << " vs " << pData[i * iStride + j].wnew;
		}
	}
}

TEST ( functions, Rebalance )
{
	//				  old weights, timers,	new weights
	tstcase dData1[] = { {50.5669f, 186751,	55.0625f}, {49.4316f, 228828, 44.9375f},
						 {55.6222f, 207608,	51.2823f}, {44.3763f, 218537, 48.7177f},
						 {56.8841f, 214800,	47.4951f}, {43.1144f, 194305, 52.5049f},
						 {54.4091f, 207614,	47.7932f}, {45.5894f, 190062, 52.2068f},
						 {52.2103f, 221708,	47.5706f}, {47.7882f, 201162, 52.4294f},
						 {49.7810f, 247379,	43.8821f}, {50.2174f, 193441, 56.1179f},
						 {43.6667f, 223202,	46.6167f}, {56.3317f, 194910, 53.3833f},
						 {40.3662f, 361018,	38.7370f}, {59.6323f, 228274, 61.2630f},
						 {29.9718f, 275050,	44.7756f}, {70.0267f, 223009, 55.2244f},
						 {25.7618f, 279008,	42.3951f}, {74.2367f, 205340, 57.6049f},
						 {20.3433f, 201466,	51.4136f}, {79.6551f, 213189, 48.5864f},
						 {21.2741f, 197584,	51.5511f}, {78.7243f, 210235, 48.4489f},
						 {25.3498f, 318349,	39.5014f}, {74.6487f, 207860, 60.4986f},
						 {18.1476f, 487120,	29.5299f}, {81.8509f, 204124, 70.4701f},
						 {08.5008f, 412733,	32.9526f}, {91.4977f, 202851, 67.0474f} };
	TestRebalance_fn ( dData1, sizeof(dData1) / sizeof( tstcase), 2 );

	tstcase dData2[] = { { 0.000000f, 0, 0.00000f }, { 00.0015f, 18469, 100.0000f } };
	TestRebalance_fn ( dData2, sizeof(dData2) / sizeof( tstcase), 2 );

	tstcase dData3[] = { { 0.000000f, 0, 0.00000f }, { 0.0015f, 0, 0.00000f }
						 , { 0.0031f, 0, 0.00000f }, { 0.0046f, 18469, 100.0000f } };
	TestRebalance_fn ( dData3, sizeof ( dData3 ) / sizeof ( tstcase ), 4 );

	tstcase dData4[] = { { 0.000000f, 7100, 72.2320f }, { 0.0015f, 0, 0.0f }, { 0.0031f, 18469, 27.7679f } };
	TestRebalance_fn ( dData4, sizeof ( dData4 ) / sizeof ( tstcase ), 3 );
}

//////////////////////////////////////////////////////////////////////////

// conversion between degrees and radians
static const double MY_PI = 3.14159265358979323846;
static const double TO_RADD = MY_PI / 180.0;
static const double TO_DEGD = 180.0 / MY_PI;

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

static inline float GeodistVincenty ( double lat1, double lon1, double lat2, double lon2 )
{
	lat1 *= TO_RADD;
	lon1 *= TO_RADD;
	lat2 *= TO_RADD;
	lon2 *= TO_RADD;
	const double a = 6378137;
	const double b = 6356752.314245;
	double f = ( a - b ) / a;
	double L = lon2 - lon1;
	double u1 = atan ( ( 1 - f ) * tan ( lat1 ) );
	double u2 = atan ( ( 1 - f ) * tan ( lat2 ) );
	double sin_u1 = sin ( u1 );
	double cos_u1 = cos ( u1 );
	double sin_u2 = sin ( u2 );
	double cos_u2 = cos ( u2 );
	double lambda = L;
	double lambda_pi = 2 * MY_PI;
	double sin_sigma = 0, cos_sigma = 0, sigma = 0, cos_sq_alpha = 0, cos2sigma_m = 0;
	while ( fabs ( lambda - lambda_pi )>1e-12 )
	{
		double sin_lambda = sin ( lambda );
		double cos_lambda = cos ( lambda );
		sin_sigma = sqrt ( ( cos_u2 * sin_lambda ) * ( cos_u2 * sin_lambda ) +
			( cos_u1 * sin_u2 - sin_u1 * cos_u2 * cos_lambda ) * ( cos_u1 * sin_u2 - sin_u1 * cos_u2 * cos_lambda ) );
		cos_sigma = sin_u1 * sin_u2 + cos_u1 * cos_u2 * cos_lambda;
		sigma = atan2 ( sin_sigma, cos_sigma );
		double alpha = asin ( cos_u1 * cos_u2 * sin_lambda / sin_sigma );
		cos_sq_alpha = cos ( alpha ) * cos ( alpha );
		cos2sigma_m = cos_sigma - 2 * sin_u1 * sin_u2 / cos_sq_alpha;
		double cc = f / 16 * cos_sq_alpha * ( 4 + f * ( 4 - 3 * cos_sq_alpha ) );
		lambda_pi = lambda;
		lambda = L + ( 1 - cc ) * f * sin ( alpha ) *
			( sigma + cc * sin_sigma * ( cos2sigma_m + cc * cos_sigma * ( -1 + 2 * cos2sigma_m * cos2sigma_m ) ) );
	}
	double usq = cos_sq_alpha * ( a * a - b * b ) / ( b * b );
	double aa = 1 + usq / 16384 * ( 4096 + usq * ( -768 + usq * ( 320 - 175 * usq ) ) );
	double bb = usq / 1024 * ( 256 + usq * ( -128 + usq * ( 74 - 47 * usq ) ) );
	double delta_sigma =
		bb * sin_sigma * ( cos2sigma_m + bb / 4 * ( cos_sigma * ( -1 + 2 * cos2sigma_m * cos2sigma_m ) -
			bb / 6 * cos2sigma_m * ( -3 + 4 * sin_sigma * sin_sigma ) * ( -3 + 4 * cos2sigma_m * cos2sigma_m ) ) );
	double c = b * aa * ( sigma - delta_sigma );
	return ( float ) c;
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

void DestVincenty ( double lat1, double lon1, double brng, double dist, double * lat2, double * lon2 )
{
	double a = 6378137, b = 6356752.3142, f = 1 / 298.257223563; // WGS-84 ellipsiod
	double s = dist;
	double alpha1 = brng * TO_RADD;
	double sinAlpha1 = sin ( alpha1 );
	double cosAlpha1 = cos ( alpha1 );

	double tanU1 = ( 1 - f ) * tan ( lat1 * TO_RADD );
	double cosU1 = 1 / sqrt ( 1 + tanU1 * tanU1 ), sinU1 = tanU1 * cosU1;
	double sigma1 = atan2 ( tanU1, cosAlpha1 );
	double sinAlpha = cosU1 * sinAlpha1;
	double cosSqAlpha = 1 - sinAlpha * sinAlpha;
	double uSq = cosSqAlpha * ( a * a - b * b ) / ( b * b );
	double A = 1 + uSq / 16384 * ( 4096 + uSq * ( -768 + uSq * ( 320 - 175 * uSq ) ) );
	double B = uSq / 1024 * ( 256 + uSq * ( -128 + uSq * ( 74 - 47 * uSq ) ) );

	double sigma = s / ( b * A ), sigmaP = 2 * MY_PI;
	double cos2SigmaM = 0, sinSigma = 0, cosSigma = 0;
	while ( fabs ( sigma - sigmaP )>1e-12 )
	{
		cos2SigmaM = cos ( 2 * sigma1 + sigma );
		sinSigma = sin ( sigma );
		cosSigma = cos ( sigma );
		double deltaSigma = B * sinSigma * ( cos2SigmaM + B / 4 * ( cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) -
			B / 6 * cos2SigmaM * ( -3 + 4 * sinSigma * sinSigma ) * ( -3 + 4 * cos2SigmaM * cos2SigmaM ) ) );
		sigmaP = sigma;
		sigma = s / ( b * A ) + deltaSigma;
	}

	double tmp = sinU1 * sinSigma - cosU1 * cosSigma * cosAlpha1;
	*lat2 = atan2 ( sinU1 * cosSigma + cosU1 * sinSigma * cosAlpha1,
		( 1 - f ) * sqrt ( sinAlpha * sinAlpha + tmp * tmp ) );
	double lambda = atan2 ( sinSigma * sinAlpha1, cosU1 * cosSigma - sinU1 * sinSigma * cosAlpha1 );
	double C = f / 16 * cosSqAlpha * ( 4 + f * ( 4 - 3 * cosSqAlpha ) );
	double L = lambda - ( 1 - C ) * f * sinAlpha *
		( sigma + C * sinSigma * ( cos2SigmaM + C * cosSigma * ( -1 + 2 * cos2SigmaM * cos2SigmaM ) ) );
	*lon2 = ( lon1 * TO_RADD + L + 3 * MY_PI );
	while ( *lon2>2 * MY_PI )
		*lon2 -= 2 * MY_PI;
	*lon2 -= MY_PI;
	*lat2 *= TO_DEGD;
	*lon2 *= TO_DEGD;
}

static const int NFUNCS = 3;

float CalcGeofunc ( int iFunc, double * t )
{
	switch ( iFunc )
	{
		case 0: return GeodistSphereDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
		case 1: return GeodistAdaptiveDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
		case 2: return GeodistFlatDeg ( float(t[0]), float(t[1]), float(t[2]), float(t[3]) ); break;
	}
	return 0;
}

TEST ( functions, DISABLED_geodist )
{
	CSphVector<double> dBench;
	for ( int adist = 10; adist<=10 * 1000 * 1000; adist *= 10 )
		for ( int dist = adist; dist<10 * adist && dist<20 * 1000 * 1000; dist += 2 * adist )
		{
			double avgerr[NFUNCS] = { 0 }, maxerr[NFUNCS] = { 0 };
			int n = 0;
			for ( int lat = -80; lat<=80; lat += 10 )
			{
				for ( int lon = -179; lon<180; lon += 3 )
				{
					for ( int b = 0; b<360; b += 3, n++ )
					{
						double t[4] = { double ( lat ), double ( lon ), 0, 0 };
						DestVincenty ( t[0], t[1], b, dist, t + 2, t + 3 );
						for ( int j = 0; j<4; j++ )
							dBench.Add ( t[j] );
						for ( int f = 0; f<NFUNCS; f++ )
						{
							float fDist = CalcGeofunc ( f, t );
							double err = fabs ( 100 * ( double ( fDist ) - double ( dist ) )
													/ double ( dist ) ); // relative error, in percents
							avgerr[f] += err;
							maxerr[f] = Max ( err, maxerr[f] );
						}
					}
				}
			}
			if ( dist>=1000 )
				printf ( "%5dkm", dist / 1000 );
			else
				printf ( "%6dm", dist );
			for ( int f = 0; f<NFUNCS; f++ )
				printf ( ", f%d %5.2f%% %5.2f%%", f, avgerr[f] / n, maxerr[f] );
			printf ( "\n" );
		}

	const int RUNS = 10;
	float fDist = 0;
	double * tmax = dBench.Begin () + dBench.GetLength ();
	int64_t tm;
	printf ( "%d calls in bench\n", RUNS * dBench.GetLength () );

	tm = sphMicroTimer ();
	for ( int r = 0; r<RUNS; r++ )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistSphereDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	printf ( INT64_FMT" us sphere\n", sphMicroTimer () - tm );

	tm = sphMicroTimer ();
	for ( int r = 0; r<RUNS; r++ )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistFlatDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	printf ( INT64_FMT" us flat\n", sphMicroTimer () - tm );

	tm = sphMicroTimer ();
	for ( int r = 0; r<RUNS; r++ )
		for ( double * t = dBench.Begin (); t<tmax; t += 4 )
			fDist += GeodistAdaptiveDeg ( float ( t[0] ), float ( t[1] ), float ( t[2] ), float ( t[3] ) );
	printf ( INT64_FMT" us adaptive\n", sphMicroTimer () - tm );

	printf ( "res %f\n", fDist );

	// coordinates from Wikimapia/Googlemaps
	//
	// distances by Wolfram Alpha (supposedly defaults to Vincenty method)
	// geodistance[{51.5007788, -0.1246771}, {46.2041222, 6.1524349}]
	//
	// 40.6890895, -74.0446899 center of the torch of the Statue of Liberty, New York, USA
	// 40.7041146, -74.0152399 center of The Sphere in Battery Park, New York, USA
	// 40.7643929, -73.9997683 tip of Lockheed A-12 (SR-71) on Intrepid, NY, USA
	// 40.7642578, -73.9994565 tail of Lockheed A-12 (SR-71) on Intrepid, NY, USA
	// 55.7535204, 37.6195371 center of Senatskaya tower, Red Square, Moscow, Russia
	// 51.6606654, 39.1999751 center of Lenin statue, Lenin Square, Voronezh, Russia
	// 49.2055275, -123.2014474 NW corner of Runway 08L-26R, YVR airport, Vancouver, Canada
	// 49.2007563, -123.1596478 NE corner of Runway 08L-26R, YVR airport, Vancouver, Canada
	// 37.6284983, -122.3927365 N corner of L on Runway 10L-28R, SFO airport, San Francisco, USA
	// 37.6137799, -122.3577954 S corner of R on Runway 10L-28R, SFO airport, San Francisco, USA
	// 68.974714, 33.0611873 tip of Lenin icebreaker, Murmansk, Russia
	// -22.9519125, -43.2105616 center of the head of Christ the Redeemer statue, Rio de Janeiro, Brazil
	// 51.5007788, -0.1246771 tip of Big Ben tower, London, England
	// 29.97973, 31.1342695 tip of Pyramid of Cheops, Cairo, Egypt
	// 41.4034549, 2.1741718 tip of the southern tower of Sagrada Familia, Barcelona, Spain
	// 42.6848586, 23.3188623 tip of National Palace of Culture, Sofia, Bulgaria
	// 46.2041222, 6.1524349 center of the fountain in English garden, Geneva, Switzerland
	// 37.8106517, -122.4174678 tip of SS Jeremiah O'Brien, Pier 45, San Francisco, USA
	// 37.8114358, -122.4186279 tail of SS Jeremiah O'Brien, Pier 45, San Francisco, USA
	// 64.1475975, -21.9224185 center of Sun Voyager in Reykjavik, Iceland
	// 63.8079982, -19.5589042 center of Eyjafjallajokull volcano, Iceland
	double dTest[][5] =
	{
		{ 40.7643929, -73.9997683, 40.7642578, -73.9994565, 30.3013 }, // Lockheed A-12 (SR-71) length (30.97m per wiki)
		{ 37.8106517, -122.4174678, 37.8114358, -122.4186279, 134.20 }, // SS Jeremiah O'Brien length ((134.57m per wiki)
		{ 40.6890895, -74.0446899, 40.7041146, -74.0152399, 2996.59 }, // Statue of Liberty to The Sphere
		{ 49.2055275, -123.2014474, 49.2007563, -123.1596478, 3091.96 }, // YVR Runway 08L-26R length (3030m per wiki)
		{ 37.6284983, -122.3927365, 37.6137799, -122.3577954, 3490.54 }, // SFO Runway 10L-28R length (3618m per wiki)
		{ 64.1475975, -21.9224185, 63.8079982, -19.5589042, 121768.14 }, // Reykjavik to Eyjafjallajokull
		{ 55.7535204, 37.6195371, 51.6606654, 39.1999751, 467301.55 }, // Moscow to Voronezh
		{ 51.5007788, -0.1246771, 46.2041222, 6.1524349, 747189.88 }, // London to Geneva
		{ 51.5007788, -0.1246771, 41.4034549, 2.1741718, 1136075.00 }, // London to Barcelona
		{ 51.5007788, -0.1246771, 42.6848586, 23.3188623, 2019138.10 }, // London to Sofia
		{ 51.5007788, -0.1246771, 29.97973, 31.1342695, 3513002.04 }, // London to Cairo
		{ 68.974714, 33.0611873, -22.9519125, -43.2105616, 11833803.11 }, // Murmansk to Rio
		{ 0, 0, 0.5, 179.5, 19936288.579 }, // antipodes, direct Vincenty killer
		// { 0, 0, 0.5, 179.7, 19944127.421 }, // antipodes, inverse Vincenty killer
	};

	for ( int i=0; i<int(sizeof(dTest)/sizeof(dTest[0])); i++ )
	{
		double * t = dTest[i];
		printf ( "%2d: ref %10.1f", i, t[4] );
		for ( int iFunc=0; iFunc<NFUNCS; iFunc++ )
		{
			float fDist = CalcGeofunc ( iFunc, t );
			printf ( ", f%d %5.2f%%", iFunc, 100*(fDist-t[4])/t[4] );
		}
		printf ( "\n" );
	}
	printf ( "\n" );
}

// parsing size - number with possible suffixes k, m, g, t.
TEST (functions, size_parser)
{
	// upper case suffixes
	ASSERT_EQ ( 1024, sphGetSize64 ( "1K" ) );
	ASSERT_EQ ( 1024 * 1024, sphGetSize64 ( "1M" ) );
	ASSERT_EQ ( 1024 * 1024 * 1024, sphGetSize64 ( "1G" ) );
	ASSERT_EQ ( 1024ULL * 1024 * 1024 * 1024, sphGetSize64 ( "1T" ) );

	// lower case suffixes;
	// Untouched sError on success;
	char * sError = nullptr;
	ASSERT_EQ ( 1, sphGetSize64 ( "1", &sError ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 1024, sphGetSize64 ( "1k", &sError ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 1024 * 1024, sphGetSize64 ( "1m", &sError ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 1024 * 1024 * 1024, sphGetSize64 ( "1g", &sError ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 1024ULL * 1024 * 1024 * 1024, sphGetSize64 ( "1t", &sError ) );
	ASSERT_EQ ( sError, nullptr );

	// empty and null input strings
	ASSERT_EQ ( 11, sphGetSize64 ( "", &sError, 11 ) );
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 12, sphGetSize64 ( nullptr, &sError, 12 ) );
	ASSERT_EQ ( sError, nullptr );

	// error handle for non-numeric
	ASSERT_EQ ( -1, sphGetSize64 ( "abc", &sError ) );
	ASSERT_STREQ (sError,"abc");

	// error handle for numeric, but unknown suffix (=non-numeric)
	ASSERT_EQ ( -1, sphGetSize64 ( "10z", &sError ) );
	ASSERT_STREQ ( sError, "z" );
}

// parsing time - number with possible suffixes us, ms, s, m, h, d, w
TEST ( functions, sphGetTime64 )
{
	static const struct
	{ int64_t tm; const char* str; } models[] = {
		{ 1, "1us" }, { 2, "2Usm" }, { 3, "3uS" }, { 4, "4US" }, // useconds
		{ 1000, "1ms" }, { 2000, "2Ms" }, { 3000, "3mS" }, { 4000, "4MS" },// milliseconds
		{ 1000000, "1" }, { 2000000, "2s" }, { 3000000, "3S" }, // seconds
		{ 60000000, "1m" }, { 120000000, "2M" }, // minutes
		{ 3600000000, "1h" }, { 36000000000, "10H" }, // hours
		{ 24ULL * 3600000000, "1D" }, { 48ULL * 3600000000, "2d" }, // days
		{ 7ULL * 24 * 3600000000, "1W" }, { 14ULL * 24 * 3600000000, "2w" }, // weeks
	};

	for ( const auto& model : models )
		EXPECT_EQ ( model.tm, sphGetTime64 (model.str) ) << "for " << model.tm << " and " << model.str;
}

// Untouched sError on success;
TEST ( functions, sphGetTime64_nullerror )
{
	static const struct
	{ int64_t tm; const char* str; } models[] = {
		{ 1000000, "1" }, { 2, "2us" }, { 1000000, "1s" }, { 60000000, "1m" },
		{ 3600000000, "1h" }, { 24ULL * 3600000000, "1d" }, { 7ULL * 24 * 3600000000, "1w" },
	};

	char* sError = nullptr;
	for ( const auto& model : models ) {
		EXPECT_EQ ( model.tm, sphGetTime64 ( model.str, &sError )) << "for " << model.tm << " and " << model.str;
		EXPECT_EQ ( sError, nullptr ) << "for " << model.tm << " and " << model.str;
	}
}

// empty and null input strings
TEST ( functions, sphGetTime64_defaults )
{
	char* sError = nullptr;
	ASSERT_EQ ( 11, sphGetTime64 ( "", &sError, 11 ));
	ASSERT_EQ ( sError, nullptr );
	ASSERT_EQ ( 12, sphGetTime64 ( nullptr, &sError, 12 ));
	ASSERT_EQ ( sError, nullptr );
}

// processing errors
TEST ( functions, sphGetTime64_errors )
{
	static const struct
	{ int64_t res; const char* str; const char* err;} models[] = {
		{ -1, "abc", "abc" }, // error handle for non-numeric
		{ -1, "10z", "z" }, // error handle for numeric, but unknown suffix (=non-numeric)
	};

	char* sError = nullptr;
	for ( const auto& model : models ) {
		EXPECT_EQ ( model.res, sphGetTime64 ( model.str, &sError ));
		EXPECT_STREQ ( sError, model.err ) << "for " << model.res << " and " << model.str << " err " << model.err;
	}
}

TEST ( functions, hashmap_iterations )
{
	struct
	{
		int iVal;
		const char * sKey;
	} tstvalues[] =
		{ {   1, "one" }
		  , { 2, "two" }
		  , { 3, "three" }
		  , { 4, "four" } };

	SmallStringHash_T<int> tHash;
	for ( auto &test: tstvalues )
		tHash.Add ( test.iVal, test.sKey );

	auto i = 0;
	for ( tHash.IterateStart (); tHash.IterateNext (); )
	{
		EXPECT_STREQ ( tHash.IterateGetKey ().cstr (), tstvalues[i].sKey );
		EXPECT_EQ ( tHash.IterateGet (), tstvalues[i].iVal );
		++i;
	}
}

TEST ( functions, vector )
{
	CSphVector<int> dVec;
	dVec.Add(1);
	dVec.Add(2);
	auto & dv = dVec.Add();
	dv = 3;
	dVec.Add(4);
	dVec.Add ( 5 );
	dVec.Add ( 6 );
	dVec.Add ( 7 );
	dVec.RemoveValue (2);
	dVec.Add ( 8 );
	dVec.Add ( 9 );
	dVec.RemoveValue ( 9);
	dVec.Add ( 9 );
	dVec.Add ( 10);
	dVec.RemoveValue ( 10 );
	ASSERT_EQ (dVec.GetLength (),8);
}

TEST ( functions, vector_slice )
{
	CSphVector<int> dVec;
	auto dSlice0 = dVec.Slice();
	ASSERT_TRUE ( dSlice0.IsEmpty() );
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	dVec.Add ( 3 );
	
	auto dSlice1 = dVec.Slice(-1,20);
	ASSERT_EQ ( dSlice1.begin(), dVec.begin());
	ASSERT_EQ ( dSlice1.GetLength (), 3 );
	
	auto dSlice2 = dVec.Slice (10,10);
	ASSERT_TRUE ( dSlice2.IsEmpty() );
	
	auto dSlice3 = dVec.Slice(1);
	ASSERT_EQ ( dSlice3.GetLength(),2);
	ASSERT_EQ ( dSlice3[0], 2);
	ASSERT_EQ ( dSlice3[1], 3 );
	ASSERT_EQ ( dSlice3.begin(), &dVec[1]);
	
	auto dSlice4 = dVec.Slice(1,1);
	ASSERT_EQ ( dSlice4.GetLength(), 1);
	ASSERT_EQ ( dSlice4.begin (), &dVec[1] );
}

TEST ( functions, sphSplit )
{
	StrVec_t dParts;
	sphSplit ( dParts, "a:b,c_", ":,_");
	ASSERT_EQ ( dParts.GetLength (), 3 );
	ASSERT_STREQ ( dParts[0].cstr (), "a" );
	ASSERT_STREQ ( dParts[1].cstr (), "b" );
	ASSERT_STREQ ( dParts[2].cstr (), "c" );

	dParts.Reset();
	sphSplit ( dParts, "a:", ":" );
	
	ASSERT_EQ ( dParts.GetLength (), 1 );
	ASSERT_STREQ ( dParts[0].cstr (), "a" );

	dParts.Reset ();
	sphSplit ( dParts, ":a", ":" );

	ASSERT_EQ ( dParts.GetLength (), 2 );
	ASSERT_STREQ ( dParts[0].cstr (), "" );
	ASSERT_STREQ ( dParts[1].cstr (), "a" );
}

// as we found g++ (GCC) 4.8.5 20150623 (Red Hat 4.8.5-28) works strange with curly initializer of refs
TEST ( functions, curledref )
{
	CSphString sProof="abc";
	const CSphString &sTest { sProof };
	const CSphString &sTestc ( sProof );
	ASSERT_TRUE ( &sProof==&sTestc ) << "curly brackets";
	ASSERT_TRUE ( &sProof==&sTest ) << "figured brackets";
}

TEST ( functions, valgrind_use )
{
	BYTE* VARIABLE_IS_NOT_USED pLeak = new BYTE[100];
	ASSERT_TRUE (true) << "intended leak";
}

TEST ( functions, int64_le )
{
	DWORD pMva[2] = {0x01020304, 0x05060708};

	// expression from MVA_UPSIZE
	auto iTest = ( int64_t ) ( ( uint64_t ) pMva[0] | ( ( ( uint64_t ) pMva[1] ) << 32 ) );
	auto iTest2 = MVA_UPSIZE ( pMva );
	ASSERT_EQ ( iTest, iTest2 );

#if USE_LITTLE_ENDIAN
	auto iTestLE = *( int64_t * ) pMva;
	ASSERT_EQ ( iTest, iTestLE ) << "little endian allows simplify";
#endif
}

TEST ( functions, FindLastNumeric )
{
	static const char * sNum1 = "12345";
	ASSERT_EQ ( sNum1, sphFindLastNumeric (sNum1,5 ));

	static const char * sNum2 = "1234 ";
	ASSERT_EQ ( sNum2+5, sphFindLastNumeric ( sNum2, 5 ) );

	static const char * sNum3 = "12 34";
	ASSERT_EQ ( sNum3 + 3, sphFindLastNumeric ( sNum3, 5 ) );
}
const char* sPattern="DeadBeefDeadBeefDeadBeefDeadBeefDeadBeefDeadBeefDeadBeefDeadBeef";

TEST ( functions, DISABLED_bench_allocator_linear )
{
	static const DWORD uTries = 10000000;

	struct chunk_t {
		BYTE * pChunk;
		BYTE uSize;
	};

	CSphVector<chunk_t> dChunks;
	dChunks.Resize (uTries);
	auto iTimeSpan = -sphMicroTimer ();
	BYTE iAllocate = 1;
	for ( auto & chunk : dChunks)
	{
		chunk.uSize = iAllocate;
		chunk.pChunk = sphAllocateSmall ( iAllocate );
		memcpy ( chunk.pChunk, sPattern, iAllocate );
		++iAllocate;
		if ( iAllocate > MAX_SMALL_OBJECT_SIZE )
			iAllocate = 1;
	}
	for ( auto &chunk : dChunks )
		sphDeallocateSmall (chunk.pChunk, chunk.uSize);
	iTimeSpan += sphMicroTimer();
	auto uReserved = sphGetSmallReservedSize ();
	std::cout << "Took " << iTimeSpan << " uSec, reserved " << uReserved << " bytes.\n";
	ASSERT_EQ ( sphGetSmallAllocatedSize (), 0 );
}

TEST ( functions, DISABLED_bench_allocator_linear64 )
{
	static const DWORD uTries = 1000;

	struct chunk_t
	{
		BYTE * pChunk;
		BYTE uSize;
	};

	CSphVector<chunk_t> dChunks;
	dChunks.Resize ( uTries );
	auto iTimeSpan = -sphMicroTimer ();
	BYTE iAllocate = 64;
	for ( auto &chunk : dChunks )
	{
		chunk.uSize = iAllocate;
		chunk.pChunk = sphAllocateSmall ( iAllocate );
		memcpy ( chunk.pChunk, sPattern, iAllocate );
//		iAllocate++;
		if ( iAllocate>MAX_SMALL_OBJECT_SIZE )
			iAllocate = 1;
	}
	for ( auto &chunk : dChunks )
		sphDeallocateSmall ( chunk.pChunk, chunk.uSize );
	iTimeSpan += sphMicroTimer ();
	auto uReserved = sphGetSmallReservedSize ();
	std::cout << "Took " << iTimeSpan << " uSec, reserved " << uReserved << " bytes.\n";
	ASSERT_EQ ( sphGetSmallAllocatedSize (), 0 );
}

TEST ( functions, DISABLED_bench_allocator_small )
{
	static const DWORD uTries = 10000000;
	static const DWORD uLoops = uTries/MAX_SMALL_OBJECT_SIZE;

	CSphVector<BYTE*> dChunks;
	dChunks.Resize ( MAX_SMALL_OBJECT_SIZE );
	auto iTimeSpan = -sphMicroTimer ();

	for ( DWORD j=0; j<uLoops; ++j)
	{
		ARRAY_FOREACH ( i, dChunks )
		{
			dChunks[i] = sphAllocateSmall ( MAX_SMALL_OBJECT_SIZE - i );
			memcpy ( dChunks[i], sPattern, MAX_SMALL_OBJECT_SIZE - i );
		}

		ARRAY_FOREACH ( i, dChunks )
			sphDeallocateSmall ( dChunks[i], MAX_SMALL_OBJECT_SIZE - i );
	}

	iTimeSpan += sphMicroTimer ();
	auto uReserved = sphGetSmallReservedSize ();
	std::cout << uLoops << " loops took " << iTimeSpan << " uSec, reserved " << uReserved << " bytes.\n";
	ASSERT_EQ ( sphGetSmallAllocatedSize (), 0 );
}

TEST ( functions, UItoA_ItoA )
{
	using namespace sph;

	char sBuf[50];
	memset (sBuf, 255, 50);

	int iLen = UItoA (sBuf, (DWORD)50);
	sBuf[iLen]='\0';
	ASSERT_STREQ ( "50", sBuf);

	iLen = ItoA ( sBuf, 50, 10, 0, 4);
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "0050", sBuf );

	iLen = ItoA ( sBuf, 50, 10, 4 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "  50", sBuf );

	iLen = ItoA ( sBuf, 50, 10, 6, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "   050", sBuf );

	iLen = ItoA ( sBuf, 50, 10, 6, 3, '_' );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "___050", sBuf );

	iLen = ItoA<int64_t> ( sBuf, 0xFFFFFFFFFFFFFFFFll );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-1", sBuf );

	iLen = ItoA<int64_t> ( sBuf, 0x8000000000000000ll );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-9223372036854775808", sBuf );

	iLen = ItoA ( sBuf, 0x7FFFFFFFFFFFFFFFll );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "9223372036854775807", sBuf );

	iLen = ItoA ( sBuf, -9223372036854775807 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-9223372036854775807", sBuf );

	sBuf[ItoA ( sBuf, -9223372036854775807 )] = '\0';
	ASSERT_STREQ ( "-9223372036854775807", sBuf );

	iLen = ItoA ( sBuf, 9223372036854775807 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "9223372036854775807", sBuf );

	iLen = ItoA<int64_t> ( sBuf, 0xFFFFFFFFFFFFFFFFll, 16 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-1", sBuf );

	iLen = ItoA<int64_t> ( sBuf, 0x8000000000000000ll, 16 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-8000000000000000", sBuf );
}

TEST ( functions, IFtoA )
{
	using namespace sph;

	char sBuf[50];
	memset ( sBuf, 255, 50 );

	int iLen = IFtoA ( sBuf, 50000, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "50.000", sBuf );

	iLen = IFtoA ( sBuf, -50000, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-50.000", sBuf );

	iLen = IFtoA ( sBuf, -1, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "-0.001", sBuf );

	iLen = IFtoA ( sBuf, 1, 3 );
	sBuf[iLen] = '\0';
	ASSERT_STREQ ( "0.001", sBuf );
}


void test_mysprintf ( const char* sFmt, int64_t iNum, const char* sResult)
{
	using namespace sph;
	char sBuf[50];
	memset ( sBuf, 255, 50 );
	sph::Sprintf ( sBuf, sFmt, iNum );
	ASSERT_STREQ ( sBuf, sResult ) << " (on fmt " << sFmt << ")";
}

void test_sprintf ( const char * sFmt, int64_t iNum )
{
	char sBuf[50];
	sprintf ( sBuf, sFmt, iNum );
	test_mysprintf ( sFmt, iNum, sBuf );
}

void test_sphintf_for ( int64_t iNum )
{
	test_sprintf ( "%d", iNum );
	test_sprintf ( "%0d", iNum );
	test_sprintf ( "%4d", iNum );
	test_sprintf ( "%04d", iNum );
	test_sprintf ( "%.4d", iNum );
	test_sprintf ( "%0.4d", iNum );
	test_sprintf ( "%9.3d", iNum );
	test_sprintf ( "%09.3d", iNum );
}

/*
 * Different helpers to investigate how copy/move would work
 * Run test functions.trainer to investigate what is finally happens.
 */
struct train_c
{
	int m_x = 0;

	train_c() { std::cout << "\n-CTR train default 0 " << this; }
	train_c(int x) : m_x (x) { std::cout << "\n-CTR train_c(x) " << m_x << " " << this; }

	train_c(const train_c& c) : m_x(c.m_x) { std::cout << "\n-COPY train ctr "
	<< m_x << " " << this << " from " << c.m_x << " " << &c;}
	train_c(train_c&& c) : m_x(c.m_x) { c.m_x = 0; std::cout << "\n-MOVE train ctr "
	<< m_x << " " << this << " from " << c.m_x << " " << &c;}

	train_c& operator= (const train_c& c) { m_x = c.m_x; std::cout << "\n-COPY train ="
		<< m_x << " " << this << " from " << &c; return *this;}
	train_c& operator= ( train_c&& c ) { m_x = c.m_x; c.m_x = 0; std::cout << "\n-MOVE train ="
		<< m_x << " " << this << " from " << &c; return *this;}

	~train_c() { std::cout << "\n-DTR train " << m_x << " " << this; m_x = 0;}
};

struct helper_c
{
	int pad = 0;
	train_c m_h;

	helper_c() { std::cout << "\nHELPER default " << this; }
//	helper_c( train_c c ) : m_h { std::move(c) } {
//		std::cout << "\nHELPER " << this << " from " << &c << " " << &m_h << " " << m_h.m_x; }

	template <typename TRAIN_C>
	helper_c ( TRAIN_C&& c ): m_h { std::forward<TRAIN_C> ( c ) }
	{
		std::cout << "\nHELPER_TT " << this << " from " << &c << " " << &m_h << " " << m_h.m_x;
	}

	~helper_c() { std::cout << "\n~HELPER " << this; }
};

template <typename TRAIN_C>
helper_c* make_helper ( TRAIN_C&& c )
{
	std::cout << "\n====>  called make_helper with " << &c;
	return new helper_c ( std::forward<TRAIN_C>(c) );
}

TEST ( functions, trainer )
{
	std::cout << "\n\n==>  usual pass";
	{
		train_c a (10);
		auto* foo = make_helper (a);
		std::cout << "\n==>  made foo " << foo->m_h.m_x << " a is " << a.m_x;
		delete foo;
	}

	std::cout << "\n\n==>  indirect ctr";
	auto fee = make_helper (11);
	std::cout << "\n==>  made fee " << fee->m_h.m_x;
	delete fee;

	std::cout << "\n\n==>  direct ctr";
	auto bar = make_helper ( train_c (12) );
	std::cout << "\n==>  made fee " << bar->m_h.m_x;
	delete bar;
}

helper_c* make_helper_byval( train_c c )
{
	std::cout << "\n====>  called make_helper_byval with " << &c;
	return new helper_c( std::move( c ));
}

TEST ( functions, trainer_by_val )
{
	std::cout << "\n\n==>  usual pass";
	{
		train_c a( 10 );
		auto* foo = make_helper_byval( a );
		std::cout << "\n==>  made foo " << foo->m_h.m_x << " a is " << a.m_x;
		delete foo;
	}

	std::cout << "\n\n==>  indirect ctr";
	auto fee = make_helper_byval( 11 );
	std::cout << "\n==>  made fee " << fee->m_h.m_x;
	delete fee;

	std::cout << "\n\n==>  direct ctr";
	auto bar = make_helper_byval( train_c( 12 ));
	std::cout << "\n==>  made fee " << bar->m_h.m_x;
	delete bar;
}

TEST ( functions, sph_Sprintf )
{
	test_sphintf_for ( 0 );
	test_sphintf_for ( 50 );
	test_sphintf_for ( -50 );
	test_sphintf_for ( 10000 );
	test_sphintf_for ( -10000 );

	int iNum = -10000;
	test_mysprintf ( "%l", iNum, "-10000" ); // %l is our specific for 64-bit signed
	test_mysprintf ( "%0l", iNum, "-10000" );
	test_mysprintf ( "%4l", iNum, "-10000" );
	test_mysprintf ( "%04l", iNum, "-10000" );
	test_mysprintf ( "%.4l", iNum, "-10000" );
	test_mysprintf ( "%0.4l", iNum, "-10000" );
	test_mysprintf ( "%9.3l", iNum, "   -10000" );
	test_mysprintf ( "%09.3l", iNum, "   -10000" );


	test_mysprintf ( "%l", -100000000000000, "-100000000000000" ); // %l is our specific for 64-bit signed

	// my own fixed-point nums
	test_mysprintf ( "%.3D", iNum, "-10.000");
	test_mysprintf ( "%.9D", iNum, "-0.000010000" );

	test_mysprintf ( "%.3F", iNum, "-10.000" );
	test_mysprintf ( "%.5F", iNum, "-0.10000" );

	iNum = 10000;
	test_mysprintf ( "%U", iNum, "10000" ); // %U is our specific for 64-bit signed
	test_mysprintf ( "%0U", iNum, "10000" );
	test_mysprintf ( "%4U", iNum, "10000" );
	test_mysprintf ( "%04U", iNum, "10000" );
	test_mysprintf ( "%.4U", iNum, "10000" );
	test_mysprintf ( "%0.4U", iNum, "10000" );
	test_mysprintf ( "%9.3U", iNum, "    10000" );
	test_mysprintf ( "%09.3U", iNum, "    10000" );

	// fallback to stardard %f
	using namespace sph;
	char sBuf[50];
	memset ( sBuf, 255, 50 );
	sph::Sprintf ( sBuf, "%03.2f", 99.9911 );
	ASSERT_STREQ ( sBuf, "99.99" );

	// strings output
	sph::Sprintf ( sBuf, "%s", "hello");
	ASSERT_STREQ ( sBuf, "hello" );
	sph::Sprintf ( sBuf, "%-s", "hello" );
	ASSERT_STREQ ( sBuf, "hello" );
	sph::Sprintf ( sBuf, "%10s", "hello" );
	ASSERT_STREQ ( sBuf, "     hello" );
	sph::Sprintf ( sBuf, "%-10s", "hello" );
	ASSERT_STREQ ( sBuf, "hello     " );
	sph::Sprintf ( sBuf, "%-10.3s", "hello" );
	ASSERT_STREQ ( sBuf, "hel       " );
	sph::Sprintf ( sBuf, "%10.3s", "hello" );
	ASSERT_STREQ ( sBuf, "       hel" );

	sph::Sprintf ( sBuf, "Hello %l, %d world!", -100000000000000, -2000000000 );
	ASSERT_STREQ ( sBuf, "Hello -100000000000000, -2000000000 world!" );
}

// sph::Sprintf into StringBuilder_c
TEST ( functions, sph_Sprintf_to_builder )
{
	using namespace sph;
	StringBuilder_c sBuf;
	Sprintf ( sBuf, "%-10s", "hello" );
	EXPECT_STREQ ( sBuf.cstr(), "hello     " );
	sBuf.Clear();

	Sprintf ( sBuf, "%03.2f", 99.9911 );
	EXPECT_STREQ ( sBuf.cstr(), "99.99" );
	sBuf.Clear ();

	sph::Sprintf ( sBuf, "Hello %d, %l world!", -2000000000, -100000000000000 );
	ASSERT_STREQ ( sBuf.cstr(), "Hello -2000000000, -100000000000000 world!" );

	sph::Sprintf ( sBuf, "Hi!" );
	ASSERT_STREQ ( sBuf.cstr (), "Hello -2000000000, -100000000000000 world!Hi!" );
	sBuf.Clear();

	Sprintf ( sBuf, "%09.3d", -10000 );
	EXPECT_STREQ ( sBuf.cstr(), "   -10000" );
	sBuf.Clear ();

	Sprintf ( sBuf, "%.3D", (int64_t) -10000 );
	EXPECT_STREQ ( sBuf.cstr (), "-10.000" );
	sBuf.Clear ();

	Sprintf ( sBuf, "%.9D", -10000ll );
	ASSERT_STREQ ( sBuf.cstr (), "-0.000010000" );
	sBuf.Clear ();

	sBuf.StartBlock ( ",", "{", "}" );
	sBuf.Sprintf ( "%d %d %d", 1, -1, 100);
	sBuf.Sprintf ( "%d %d %d", 2, -2, 200 );
	sBuf.FinishBlock ();
	ASSERT_STREQ ( sBuf.cstr (), "{1 -1 100,2 -2 200}" );

	sBuf.Sprintf ( " %.3F, %.6F", 999500, -1400932 );
	ASSERT_STREQ ( sBuf.cstr (), "{1 -1 100,2 -2 200} 999.500, -1.400932" );

	sBuf.Sprintf ( " %.3F", 999005 );
	ASSERT_STREQ ( sBuf.cstr (), "{1 -1 100,2 -2 200} 999.500, -1.400932 999.005" );
}

TEST ( functions, sph_Sprintf_regression_on_empty_buf )
{
	StringBuilder_c sBuf;
	sBuf.Sprintf ( "%.3F", 10 );
	ASSERT_STREQ ( sBuf.cstr (), "0.010" );
}

TEST ( functions, sph_Sprintf_inttimespans )
{
	StringBuilder_c sBuf;

	static const struct {int64_t tm; const char* res;} models[] =
	{
		{ 4,			"4us" },
		{ 5000,			"5ms" },
		{ 6000000,		"6s" },
		{ 120000000,			"2m" },
		{ 3600000000,			"1h" },
		{ 3600000000ULL*24*2,	"2d" },
		{ 3600000000ULL*24*7*2,	"2w" },
	};

	for ( const auto& model : models )
	{
		sBuf.Sprintf ( "%t", model.tm );
		EXPECT_STREQ ( sBuf.cstr (), model.res ) << "for " << model.tm << " with %t";
		sBuf.Clear ();
	}
}

TEST ( functions, sph_Sprintf_fractimespans_round )
{
	StringBuilder_c sBuf;
	static const struct {int64_t tm; const char* fmt; const char* res;} models[] =
		{
			// us rounding
			{ 999, "%t", "999us" },
			{ 999, "%.1t", "999us" },
			{ 999, "%.2t", "999us" },
			{ 999, "%.3t", "999us" },

			// ms rounding
			{ 1559, "%t", "2ms" },
			{ 1559, "%.1t", "1.6ms" },
			{ 1559, "%.2t", "1.56ms" },
			{ 1559, "%.3t", "1ms 559us" },

			// s rounding
			{ 1555555, "%t", "2s" },
			{ 1555555, "%.1t", "1.6s" },
			{ 1555555, "%.2t", "1.56s" },
			{ 1555555, "%.3t", "1s 556ms" },
			{ 1555555, "%.4t", "1s 555.6ms" },
			{ 1555555, "%.5t", "1s 555.56ms" },
			{ 1999995, "%.5t", "2s" },
			{ 1555555, "%.6t", "1s 555ms 555us" },

			// m rounding
			{ 71555555, "%t", "1m" },
			{ 71555555, "%.1t", "1.2m" },
			{ 71555555, "%.2t", "1m 12s" },
			{ 71555555, "%.3t", "1m 11.6s" },
			{ 71555555, "%.4t", "1m 11.56s" },
			{ 71555555, "%.5t", "1m 11s 556ms" },
			{ 71555555, "%.6t", "1m 11s 555.6ms" },
			{ 71555555, "%.7t", "1m 11s 555.56ms" },
			{ 71555555, "%.8t", "1m 11s 555ms 555us" },
			{ 89999999, "%.7t", "1m 30s" },
			{ 89999994, "%.7t", "1m 29s 999.99ms" },
			{ 89999995, "%.7t", "1m 30s" },
			{ 90999999, "%.7t", "1m 31s" },
		};

	for ( const auto& model : models )
	{
		sBuf.Sprintf ( model.fmt, model.tm );
		EXPECT_STREQ ( sBuf.cstr (), model.res ) << "for " << model.tm << " with " << model.fmt;
		sBuf.Clear ();
	}
}

TEST ( functions, sph_Sprintf_fractimezero )
{
	StringBuilder_c sBuf;

	sBuf.Sprintf ( "%t", 0 );
	ASSERT_STREQ ( sBuf.cstr (), "0us" );
	sBuf.Clear ();

	sBuf.Sprintf ( "%.3t", 0 );
	ASSERT_STREQ ( sBuf.cstr (), "0us" );
	sBuf.Clear ();
}

TEST ( functions, DISABLED_bench_Sprintf )
{
	char sBuf[40];
	auto uLoops = 10000000;

	auto iTimeSpan = -sphMicroTimer ();
	for ( auto i=0; i<uLoops; ++i )
		sph::Sprintf ( sBuf, "%d", 1000000 );
	iTimeSpan += sphMicroTimer ();
	std::cout << "\n" << uLoops << " of sph::sprintf took " << iTimeSpan << " uSec";

	iTimeSpan = -sphMicroTimer ();
	for ( auto i = 0; i<uLoops; ++i )
		sprintf ( sBuf, "%d", 1000000 );
	iTimeSpan += sphMicroTimer ();
	std::cout << "\n" << uLoops << " of sprintf took " << iTimeSpan << " uSec\n";

	ASSERT_EQ ( sphGetSmallAllocatedSize (), 0 );
}

TEST ( functions, DISABLED_bench_builder_Appendf_vs_Sprintf )
{
	auto uLoops = 1000000;

	const char * sFieldFmt = R"({"field":%d, "lcs":%u, "hit_count":%u, "word_count":%u, "tf_idf":%d, "min_idf":%d, )"
				R"("max_idf":%d, "sum_idf":%d, "min_hit_pos":%d, "min_best_span_pos":%d, "exact_hit":%u, )"
				R"("max_window_hits":%d, "min_gaps":%d, "exact_order":%u, "lccs":%d, "wlccs":%f, "atc":%f})";

	StringBuilder_c sBuf;

	auto iTimeSpan = -sphMicroTimer ();
	for ( auto i = 0; i<uLoops; ++i )
	{
		sBuf.Appendf ( sFieldFmt, 3, 23, 23465, 234, 234, 4346,
			345345, 3434535, 345, 54, 1,
			23, 5, 0, 34, .345f, .234f );
		sBuf.Clear();
	}
	iTimeSpan += sphMicroTimer ();
	std::cout << "\n" << uLoops << " of Appendf took " << iTimeSpan << " uSec";

	iTimeSpan = -sphMicroTimer ();
	for ( auto i = 0; i<uLoops; ++i )
	{
		sBuf.Sprintf ( sFieldFmt, 3, 23, 23465, 234, 234, 4346,
			345345, 3434535, 345, 54, 1,
			23, 5, 0, 34, .345f, .234f );
		sBuf.Clear();
	}
	iTimeSpan += sphMicroTimer ();
	std::cout << "\n" << uLoops << " of Sprintf took " << iTimeSpan << " uSec\n";

	ASSERT_EQ ( sphGetSmallAllocatedSize (), 0 );
}

TEST ( functions, DISABLED_bench_builder_Appendf_vs_Sprintf_ints )
{
	auto uLoops = 1000000;

	const char * sFieldFmt = R"({"field":%d, "lcs":%u, "hit_count":%u, "word_count":%u, "tf_idf":%d, "min_idf":%d, )"
			 R"("max_idf":%d, "sum_idf":%d, "min_hit_pos":%d, "min_best_span_pos":%d, "exact_hit":%u, )"
			 R"("max_window_hits":%d, "min_gaps":%d, "exact_order":%u, "lccs":%d, "wlccs":%d, "atc":%d})";

	StringBuilder_c sBuf;

	auto iTimeSpan = -sphMicroTimer ();
	for ( auto i = 0; i<uLoops; ++i )
	{
		sBuf.Appendf ( sFieldFmt, 3, 23, 23465, 234, 234, 4346, 345345, 3434535, 345, 54, 1, 23, 5, 0, 34, 45
					   , 234 );
		sBuf.Clear ();
	}
	iTimeSpan += sphMicroTimer ();
	std::cout << "\n" << uLoops << " of Appendf took " << iTimeSpan << " uSec";

	iTimeSpan = -sphMicroTimer ();
	for ( auto i = 0; i<uLoops; ++i )
	{
		sBuf.Sprintf ( sFieldFmt, 3, 23, 23465, 234, 234, 4346, 345345, 3434535, 345, 54, 1, 23, 5, 0, 34, 45
					   , 234 );
		sBuf.Clear ();
	}
	iTimeSpan += sphMicroTimer ();
	std::cout << "\n" << uLoops << " of Sprintf took " << iTimeSpan << " uSec\n";

	ASSERT_EQ ( sphGetSmallAllocatedSize (), 0 );
}

TEST ( functions, VectorEx )
{
	using namespace sph;
	CSphTightVector<int> dTVec;
	CSphVector<int> dVec;
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	auto &dv = dVec.Add ();
	dv = 3;
	dVec.Add ( 4 );
	dVec.Add ( 5 );
	dVec.Add ( 6 );
	dVec.Add ( 7 );
	dVec.RemoveValue ( 2 );
	dVec.Add ( 8 );
	dVec.Add ( 9 );
	dVec.RemoveValue ( 9 );
	dVec.Add ( 9 );
	dVec.Add ( 10 );
	dVec.RemoveValue ( 10 );
	ASSERT_EQ ( dVec.GetLength (), 8 );
	dTVec.Add(30);
	dTVec.Add(20);
	dVec.Append ( dTVec );
	ASSERT_EQ ( dVec.GetLength (), 10 );
//	dVec.SwapData (dTVec);
	LazyVector_T<int> dLVec;
	dLVec.Add(4);
	dLVec.Add(5);
	ASSERT_EQ ( dLVec.GetLength (), 2 );
	dTVec.Append (dLVec);
	ASSERT_EQ ( dTVec.GetLength (), 4 );
	int* VARIABLE_IS_NOT_USED pData = dTVec.LeakData();
}

TEST ( functions, VectorCopyMove )
{
	using vec = CSphVector<int>;
	vec dVec;
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	dVec.Add ( 3 );
	dVec.Add ( 4 );
	dVec.Add ( 5 );
	vec dCopy ( dVec ); // copy c-tr
	vec dCopy2; // default c-tr
	dCopy2 = dVec; // copy c-tr dVec to tmp, then swap dCopy2 with tmp; then d-tr of empty tmp.
	vec dMove ( std::move ( dCopy )); // move c-tr
	vec dMove2; // default ctr
	dMove2 = std::move ( dCopy2 ); // move ctr dCopy2 to tmp, swap dMove2 with tmp; dtr empty tmp.
}

TEST ( functions, LazyVectorCopyMove )
{
	using vec = LazyVector_T<int>;
	vec dVec;
	dVec.Add ( 1 );
	dVec.Add ( 2 );
	dVec.Add ( 3 );
	dVec.Add ( 4 );
	dVec.Add ( 5 );
	// vec dCopy ( dVec ); // will not compile since copy c-tr is deleted
	vec dCopy;
	dCopy.Append(dVec);
}

#ifdef _WIN32
#pragma warning(push) // store current warning values
#pragma warning(disable:4101)
#endif

TEST ( functions, wider_and_widest )
{
	WIDER<BYTE,DWORD>::T VARIABLE_IS_NOT_USED dw;
	ASSERT_EQ ( sizeof ( dw ), sizeof ( DWORD ) );
	
	WIDER<double,char>::T VARIABLE_IS_NOT_USED dbl;
	ASSERT_EQ ( sizeof ( dbl ), sizeof ( double ) );
	
	WIDEST<char,BYTE,WORD,double>::T VARIABLE_IS_NOT_USED dbl2;
	ASSERT_EQ ( sizeof ( dbl ), sizeof ( double ) );

	WIDEST<char *, BYTE, WORD, float>::T VARIABLE_IS_NOT_USED pchar;
	ASSERT_EQ ( sizeof ( pchar ), sizeof ( char* ) );
}

#ifdef _WIN32
#pragma warning(pop) // restore warnings
#endif

TEST ( functions, warner_c )
{
	Warner_c sMsg;

	// output two errors - expect ,-separated
	sMsg.Err("Error 1");
	sMsg.Err("Error 2");
	ASSERT_STREQ ( sMsg.sError(), "Error 1, Error 2");

	// formatted output
	sMsg.Clear();
	sMsg.Err("Error %d", 10);
	ASSERT_STREQ ( sMsg.sError (), "Error 10" );

	// finalized combo output for errors only
	CSphString sFinal;
	sMsg.MoveAllTo (sFinal);
	ASSERT_STREQ ( sFinal.cstr (), "ERRORS: Error 10" );

	// finalized combo output for warnings only
	sMsg.Warn ( "msg 1" );
	sMsg.Warn ( "msg %d", 2 );
	sMsg.MoveAllTo ( sFinal );
	ASSERT_STREQ ( sFinal.cstr (), "WARNINGS: msg 1, msg 2" );

	// output two warnings (same as with error - expected ,-separated)
	sMsg.Warn ( "msg 1" );
	sMsg.Warn ( "msg %d", 2 );
	ASSERT_STREQ ( sMsg.sWarning (), "msg 1, msg 2" );

	// finalized combo output of both errors and warnings
	sMsg.Err ( "Error %d", 10 );
	sMsg.MoveAllTo ( sFinal );
	ASSERT_STREQ ( sFinal.cstr (), "ERRORS: Error 10; WARNINGS: msg 1, msg 2" );
}

// testing our priority queue
TEST ( functions, CSphQueue )
{
	int iMin = 1000;
	CSphQueue<int, SphLess_T<int> > qQ ( 10 );
	for ( auto iVal : { 89, 5, 4, 8, 4, 3, 1, 5, 4, 2 } )
	{
		qQ.Push ( iVal );
		iMin = Min ( iMin, iVal );
		ASSERT_EQ ( qQ.Root (), iMin ) << "min elem always on root";
	}

	ASSERT_EQ ( qQ.GetLength (), 10 ); ASSERT_EQ ( qQ.Root(), 1); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 9 ); ASSERT_EQ ( qQ.Root (), 2 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 8 ); ASSERT_EQ ( qQ.Root (), 3 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 7 ); ASSERT_EQ ( qQ.Root (), 4 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 6 ); ASSERT_EQ ( qQ.Root (), 4 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 5 ); ASSERT_EQ ( qQ.Root (), 4 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 4 ); ASSERT_EQ ( qQ.Root (), 5 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 3 ); ASSERT_EQ ( qQ.Root (), 5 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 2 ); ASSERT_EQ ( qQ.Root (), 8 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 1 ); ASSERT_EQ ( qQ.Root (), 89 ); qQ.Pop();
	ASSERT_EQ ( qQ.GetLength (), 0 );

	qQ.Push(1000);
	ASSERT_EQ ( qQ.Root (), 1000 ) << "pushed 1000 to empty, it is root now";

	qQ.Push ( 100 );
	ASSERT_EQ ( qQ.Root (), 100 ) << "pushed 100 over 1000, it became root now";

}


TEST ( functions, path )
{
	CSphString sSrc1 ( "/home/build/test/data/pq2" );
	CSphString sPath1 = GetPathOnly ( sSrc1 );
	ASSERT_STREQ ( sPath1.cstr(), "/home/build/test/data/" );

	CSphString sSrc2 ( "home/pq2" );
	CSphString sPath2 = GetPathOnly ( sSrc2 );
	ASSERT_STREQ ( sPath2.cstr(), "home/" );

	CSphString sSrc3 ( "/pq2" );
	CSphString sPath3 = GetPathOnly ( sSrc3 );
	ASSERT_STREQ ( sPath3.cstr(), "/" );

	CSphString sSrc4 ( "/home/pq2" );
	CSphString sPath4 = GetPathOnly ( sSrc4 );
	ASSERT_STREQ ( sPath4.cstr(), "/home/" );

	CSphString sSrc5 ( "/home/build/" );
	CSphString sPath5 = GetPathOnly ( sSrc5 );
	ASSERT_STREQ ( sPath5.cstr(), "/home/build/" );

	CSphString sSrc6 ( "home/build/" );
	CSphString sPath6 = GetPathOnly ( sSrc6 );
	ASSERT_STREQ ( sPath6.cstr(), "home/build/" );

	CSphString sSrc11 ( "/home/pq2" );
	CSphString sFile11 = GetBaseName ( sSrc11 );
	ASSERT_STREQ ( sFile11.cstr(), "pq2" );

	CSphString sSrc12 ( "home/pq2" );
	CSphString sFile12 = GetBaseName ( sSrc12 );
	ASSERT_STREQ ( sFile12.cstr(), "pq2" );

	CSphString sSrc13 ( "pq2" );
	CSphString sFile13 = GetBaseName ( sSrc13 );
	ASSERT_STREQ ( sFile13.cstr(), "pq2" );

	CSphString sSrc14 ( "/pq2" );
	CSphString sFile14 = GetBaseName ( sSrc14 );
	ASSERT_STREQ ( sFile14.cstr(), "pq2" );
}