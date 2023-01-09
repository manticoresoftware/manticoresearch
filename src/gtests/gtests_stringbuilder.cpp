//
// Copyright (c) 2023, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include <gtest/gtest.h>

#include "std/stringbuilder.h"
#include "std/string.h"
#include "std/scoped_comma.h"
#include "sphinxjson.h"


//////////////////////////////////////////////////////////////////////////

// test on usual +=, <<.
TEST ( functions, stringbuilder_hello )
{
	StringBuilder_c builder;

	// += of const char string
	builder += "Hello";

	// << for const char*
	builder << " "
			<< "world!";
	ASSERT_STREQ ( builder.cstr(), "Hello world!" );

	// << for CSphString
	CSphString s ( "I am" );
	builder << s;
	ASSERT_STREQ ( builder.cstr(), "Hello world!I am" );

	// << for vec of chars
	CSphVector<char> dText;
	dText.Add ( 'a' );
	dText.Add ( 'b' );
	dText.Add ( 'c' );
	builder << dText;
	ASSERT_STREQ ( builder.cstr(), "Hello world!I amabc" );

	// AppendChunk of blob
	const char* sText = "text";
	builder.AppendChunk ( { sText, (int)strlen ( sText ) } );
	ASSERT_STREQ ( builder.cstr(), "Hello world!I amabctext" );

	// AppendChunk with quotation
	builder.AppendChunk ( { sText, (int)strlen ( sText ) }, '`' );
	ASSERT_STREQ ( builder.cstr(), "Hello world!I amabctext`text`" );

	// AppendString
	builder.AppendString ( s );
	ASSERT_STREQ ( builder.cstr(), "Hello world!I amabctext`text`I am" );

	// AppendString quoted
	builder.AppendString ( s, '_' );
	ASSERT_STREQ ( builder.cstr(), "Hello world!I amabctext`text`I am_I am_" );
}

// test for scoped-comma modifier.
// comma will automatically append '; ' between ops.
TEST ( functions, stringbuilder_simplescoped )
{
	StringBuilder_c builder;
	auto tComma = ScopedComma_c ( builder, "; " );

	builder += "one";
	builder << "two"
			<< "three";
	builder.Appendf ( "four: %d", 4 );
	builder << "five";
	ASSERT_STREQ ( builder.cstr(), "one; two; three; four: 4; five" );
}

// scoped comma with prefix (will prepend prefix before first op)
// nested comma (will use other behaviour in isolated scope)
TEST ( functions, stringbuilder_scopedprefixed )
{
	StringBuilder_c builder;
	auto tC = ScopedComma_c ( builder, "; ", "List: " );

	builder += "one";
	builder << "two"
			<< "three";
	ASSERT_STREQ ( builder.cstr(), "List: one; two; three" ) << "plain insert into managed";
	{
		auto tI = ScopedComma_c ( builder, ": ", "{", "}" );
		builder << "first"
				<< "second";
		ASSERT_STREQ ( builder.cstr(), "List: one; two; three; {first: second" ) << "nested managed insert";
	}
	ASSERT_STREQ ( builder.cstr(), "List: one; two; three; {first: second}" ) << "nested managed insert terminated";
	builder.Appendf ( "four: %d", 4 );
	builder << "five";
	ASSERT_STREQ ( builder.cstr(), "List: one; two; three; {first: second}; four: 4; five" );
}

// standalone comma. Not necesssary related to stringbuilder, but live alone.
TEST ( functions, stringbuilder_standalone )
{
	StringBuilder_c builder;
	Comma_c tComma ( ", " ); // default is ', '
	builder << tComma << "one";
	builder << tComma << "two";
	builder << tComma << "three";
	ASSERT_STREQ ( builder.cstr(), "one, two, three" );
}

// standalone comma. Not necessary related to stringbuilder, but live alone.
TEST ( functions, stringbuilder_templated )
{
	StringBuilder_c builder ( "," );
	builder.Sprint ( "one", 3, " ", 4.34, "fine" );
	EXPECT_STREQ ( builder.cstr(), "one,3, ,4.340000,fine" );
	const char* szData = "hello";
	builder << szData; // routed as const char*
	const char szData1[] = "hello2";
	builder << szData1; // routed as const char[]
	CSphString sData2 = "hello3";
	builder << sData2.cstr(); // routed as const char*
	builder << "hello4";	  // routed as const char[]
	void* pVoid = nullptr;
	builder << pVoid; // routed as const T*
	int* pInt = nullptr;
	builder << pInt; // routed as const T*
	char* pChar = nullptr;
	builder << pChar; // routed as const char*, and so, will output nothing
	char pCharArr[10] = "fff\0ddd";
	builder << (char*)pCharArr; // routed as const char*
	builder.Sprint ( szData, szData1, sData2.cstr(), "hello4", pVoid, pInt, pChar, (char*)pCharArr );
	builder << pCharArr << "aaa"; // fixme! routed as const char[]. So, tailing "ddd" and "aaa" will NOT be visible, as \0 is inside of pCharArr
	ASSERT_STREQ ( builder.cstr(), "one,3, ,4.340000,fine,hello,hello2,hello3,hello4,0x0000000000000000,0x0000000000000000,fff,hello,hello2,hello3,hello4,0x0000000000000000,0x0000000000000000,fff,fff" );
}

TEST ( functions, StringBuilder_sugar )
{
	static const struct
	{
		const char* name;
		int value;
	} datas[] =
		{
			{ "one", 1 },
			{ "two", 2 },
			{ "three", 3 },
			{ "four", 4 },
			{ "five", 5 },
			{ "six", 6 },
			{ "seven", 7 },
		};

	StringBuilder_c sBuf;
	ScopedComma_c tComma ( sBuf, dJsonObj );
	for ( const auto& data : datas )
	{
		ScopedComma_c ( sBuf, "=" ).Sink() << data.name << data.value;
		sBuf << "dl";
	}
	sBuf.FinishBlocks();

	ASSERT_STREQ ( sBuf.cstr(), "{one=1,dl,two=2,dl,three=3,dl,four=4,dl,five=5,dl,six=6,dl,seven=7,dl}" );
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
	ScopedComma_c lev0 ( builder, ", " );
	{
		ScopedComma_c lev1 ( builder, ", ", "[", "]" );
		builder.StartBlock ( ": ", "(", ")" );
		builder.StartBlock ( ";", "{", "}" );
		ASSERT_STREQ ( builder.cstr(), "one, two, three" ) << "simple blocks do nothing";
		builder << "first"
				<< "second"
				<< "third";
		ASSERT_STREQ ( builder.cstr(), "one, two, three[({first;second;third" ) << "unclosed block";
	}
	// note that only 'lev1' is destroyed, we didn't explicitly finished two nested blocks.
	// but they're finished implicitly
	ASSERT_STREQ ( builder.cstr(), "one, two, three[({first;second;third})]" ) << "closed block";
	builder << "four";

	// note, we doesn't destroy outer comma lev0, but this is not necessary since it doesn't have a suffix.
	ASSERT_STREQ ( builder.cstr(), "one, two, three[({first;second;third})], four" ) << "finished block with tail";
}

// pure StartBlock..FinishBlock test
TEST ( functions, stringbuilder_autoclose )
{
	StringBuilder_c builder ( ": ", "[", "]" );
	// note that there is no ': ' suffixed at the end (since comma only between blocks)
	builder << "one"
			<< "two";
	ASSERT_STREQ ( builder.cstr(), "[one: two" ) << "simple pushed block";

	// starting block doesn't mean any output yet, so content is the same
	builder.StartBlock ( dBracketsComma );
	ASSERT_STREQ ( builder.cstr(), "[one: two" ) << "simple pushed block";

	// note that now ': ' of outer block prepended to the suffix '(' of the current block.
	builder << "abc"
			<< "def";
	ASSERT_STREQ ( builder.cstr(), "[one: two: (abc,def" ) << "simple pushed block 2";

	// finishing block mean that suffix appended, if the state is different from initial
	builder.FinishBlock();
	ASSERT_STREQ ( builder.cstr(), "[one: two: (abc,def)" ) << "simple pushed block 2";
	builder.FinishBlock();
	ASSERT_STREQ ( builder.cstr(), "[one: two: (abc,def)]" ) << "simple pushed block 3";
}

// pure StartBlock..FinishBlock test with one empty block (it outputs nothing)
TEST ( functions, stringbuilder_close_of_empty )
{
	StringBuilder_c builder ( ": ", "[", "]" );

	// note that there is no ': ' suffixed at the end (since comma only between blocks)
	builder << "one"
			<< "two";
	ASSERT_STREQ ( builder.cstr(), "[one: two" ) << "simple pushed block";

	// starting block doesn't output anything by itself, but modify future output
	builder.StartBlock ( dBracketsComma );
	ASSERT_STREQ ( builder.cstr(), "[one: two" ) << "started new block";

	// finishing of empty block outputs also nothing
	builder.FinishBlock();
	ASSERT_STREQ ( builder.cstr(), "[one: two" ) << "finished empty block";

	// finishing non-empty block outputs suffix (and so, doesn't strictly necessary if no suffixes).
	builder.FinishBlock();
	ASSERT_STREQ ( builder.cstr(), "[one: two]" ) << "final result";
}

// operation 'clear'. Not only wipe content, but also undo any comma state
TEST ( functions, stringbuilder_clear )
{
	StringBuilder_c builder ( ": ", "[", "]" );
	builder << "one"
			<< "two";
	builder.StartBlock ( dBracketsComma );
	builder << "abc"
			<< "def";
	builder.Clear();
	ASSERT_STREQ ( builder.cstr(), "" ) << "emtpy";
	builder << "one"
			<< "two";
	ASSERT_STREQ ( builder.cstr(), "onetwo" ) << "nocommas";
	builder.FinishBlocks();
	ASSERT_STREQ ( builder.cstr(), "onetwo" ) << "nocommas";
}

// 'FinishBlocks()' - by default closes ALL opened blocks
TEST ( functions, stringbuilder_twoopenoneclose )
{
	StringBuilder_c builder ( ": ", "[", "]" );
	builder << "one"
			<< "two";
	builder.StartBlock ( dBracketsComma );
	builder << "abc"
			<< "def";
	builder.FinishBlocks();
	ASSERT_STREQ ( builder.cstr(), "[one: two: (abc,def)]" ) << "simple pushed block 3";
}

// simple start/finish blocks manipulation - outputs nothing by alone
TEST ( functions, stringbuilder_finishnoopen )
{
	StringBuilder_c builder ( ":", "[", "]" );
	auto pLev = builder.StartBlock ( ";", "(", ")" );
	builder.StartBlock ( dJsonObj );
	builder.FinishBlocks ( pLev );
	ASSERT_STREQ ( builder.cstr(), "" ) << "nothing outputed";
}

// FinishBlocks() to stored state
TEST ( functions, stringbuilder_ret_to_level )
{
	// outer block
	StringBuilder_c builder ( ":", "[", "]" );
	builder << "exone"
			<< "extwo";

	// middle block - we memorize this state
	auto pLev = builder.StartBlock ( ";", "(", ")" );
	builder << "one"
			<< "two";

	// internal block
	builder.StartBlock ( dJsonObj );
	builder << "three"
			<< "four";
	ASSERT_STREQ ( builder.cstr(), "[exone:extwo:(one;two;{three,four" );

	// finish memorized block and all blocks created after it
	builder.FinishBlocks ( pLev );
	ASSERT_STREQ ( builder.cstr(), "[exone:extwo:(one;two;{three,four})" );

	// it will output into most outer block, since others finished
	builder << "ex3";
	ASSERT_STREQ ( builder.cstr(), "[exone:extwo:(one;two;{three,four}):ex3" );

	// it will finish outer block (and close the bracket).
	builder.FinishBlocks();
	ASSERT_STREQ ( builder.cstr(), "[exone:extwo:(one;two;{three,four}):ex3]" ) << "test complete";
}

// simple test on Appendf
TEST ( functions, strinbguilder_appendf )
{
	StringBuilder_c sRes;
	sRes.Appendf ( "12345678" );
	ASSERT_STREQ ( sRes.cstr(), "12345678" );
	sRes.Appendf ( "this is my rifle this is my gun" );
	ASSERT_STREQ ( sRes.cstr(), "12345678this is my rifle this is my gun" );
	sRes.Appendf ( " int=%d float=%f string=%s", 123, 456.789, "helloworld" );
	ASSERT_STREQ (
		sRes.cstr(), "12345678this is my rifle this is my gun int=123 float=456.789000 string=helloworld" );
}

struct EscapeQuotator_t
{
	static constexpr BYTE EscapingSpace ( BYTE c )
	{
		return ( c == '\\' || c == '\'' ) ? 1 : 0;
	}
};


using QuotationEscapedBuilder = EscapedStringBuilder_T<BaseQuotation_T<EscapeQuotator_t>>;

TEST ( functions, EscapedStringBuilder )
{
	QuotationEscapedBuilder tBuilder;
	tBuilder.AppendEscaped ( "Hello" );
	ASSERT_STREQ ( tBuilder.cstr(), "'Hello'" );

	tBuilder.AppendEscaped ( " wo\\rl\'d" );
	ASSERT_STREQ ( tBuilder.cstr(), "'Hello'' wo\\\\rl\\'d'" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "wo\\rl\'d", EscBld::eFixupSpace );
	ASSERT_STREQ ( tBuilder.cstr(), "wo\\rl\'d" );

	// generic const char* with different escapes
	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eNone );
	ASSERT_STREQ ( tBuilder.cstr(), "space\t and\r 'tab'\n here" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace );
	ASSERT_STREQ ( tBuilder.cstr(), "space  and  'tab'  here" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eEscape );
	ASSERT_STREQ ( tBuilder.cstr(), "'space\t and\r \\'tab\\'\n here'" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here" );
	ASSERT_STREQ ( tBuilder.cstr(), "'space  and  \\'tab\\'  here'" );

	// nullptr with different escapes
	tBuilder.Clear();
	tBuilder.AppendEscaped ( nullptr, EscBld::eNone );
	ASSERT_STREQ ( tBuilder.cstr(), "" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( nullptr, EscBld::eFixupSpace );
	ASSERT_STREQ ( tBuilder.cstr(), "" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( nullptr, EscBld::eEscape );
	ASSERT_STREQ ( tBuilder.cstr(), "''" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( nullptr, EscBld::eAll );
	ASSERT_STREQ ( tBuilder.cstr(), "''" );

	// empty with different escapes
	tBuilder.Clear();
	tBuilder.AppendEscaped ( "", EscBld::eNone );
	ASSERT_STREQ ( tBuilder.cstr(), "" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "", EscBld::eFixupSpace );
	ASSERT_STREQ ( tBuilder.cstr(), "" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "", EscBld::eEscape );
	ASSERT_STREQ ( tBuilder.cstr(), "''" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "", EscBld::eAll );
	ASSERT_STREQ ( tBuilder.cstr(), "''" );

	// len-defined blob
	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eNone, 10 );
	ASSERT_STREQ ( tBuilder.cstr(), "space\t and" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace, 10 );
	ASSERT_STREQ ( tBuilder.cstr(), "space  and" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eEscape, 10 );
	ASSERT_STREQ ( tBuilder.cstr(), "'space\t and'" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eAll, 10 );
	ASSERT_STREQ ( tBuilder.cstr(), "'space  and'" );

	// zero-len blob
	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eNone, 0 );
	ASSERT_STREQ ( tBuilder.cstr(), "" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace, 0 );
	ASSERT_STREQ ( tBuilder.cstr(), "" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eEscape, 0 );
	ASSERT_STREQ ( tBuilder.cstr(), "''" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space\t and\r 'tab'\n here", EscBld::eAll, 0 );
	ASSERT_STREQ ( tBuilder.cstr(), "''" );

	// len-defined blob exactly of given len, non z-terminated.
	// (valgrind would check nicely if it even try to touch a byte over allocated buf)
	char* buf = new char[5];
	memcpy ( buf, "space", 5 );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( buf, EscBld::eNone, 5 );
	ASSERT_STREQ ( tBuilder.cstr(), "space" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( buf, EscBld::eFixupSpace, 5 );
	ASSERT_STREQ ( tBuilder.cstr(), "space" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( buf, EscBld::eEscape, 5 );
	ASSERT_STREQ ( tBuilder.cstr(), "'space'" );

	tBuilder.Clear();
	tBuilder.AppendEscaped ( buf, EscBld::eAll, 5 );
	ASSERT_STREQ ( tBuilder.cstr(), "'space'" );

	delete[] buf;
}

TEST ( functions, EscapedStringBuilderbounds )
{
	QuotationEscapedBuilder tBuilder;

	// len-defined blob exactly of given len, non z-terminated.
	// (valgrind would check nicely if it even try to touch a byte over allocated buf)

	tBuilder.Clear();
	tBuilder.AppendEscaped ( "space", EscBld::eAll, 5 );
	ASSERT_STREQ ( tBuilder.cstr(), "'space'" );
}

void esc_first_comma ( const char* sText, BYTE eKind, const char* sProof )
{
	QuotationEscapedBuilder tBuilder;
	tBuilder.StartBlock();
	tBuilder << "first";
	tBuilder.AppendEscaped ( sText, eKind );
	ASSERT_STREQ ( tBuilder.cstr(), sProof ) << (int)eKind;
}

TEST ( functions, EscapedStringBuilderAndCommas )
{
	// generic const char* with different escapes
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eNone, "first, space\t and\r 'tab'\n here" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace, "first, space  and  'tab'  here" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eEscape, "first, 'space\t and\r \\'tab\\'\n here'" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eAll, "first, 'space  and  \\'tab\\'  here'" );

	// null with different escapes
	esc_first_comma ( nullptr, EscBld::eNone, "first" );
	esc_first_comma ( nullptr, EscBld::eFixupSpace, "first" );
	esc_first_comma ( nullptr, EscBld::eEscape, "first, ''" );
	esc_first_comma ( nullptr, EscBld::eAll, "first, ''" );
}

TEST ( functions, JsonNamedEssence )
{
	StringBuilder_c sRes ( ",", "{", "}" );
	sRes << "hello";
	ASSERT_STREQ ( sRes.cstr(), "{hello" );
	sRes << "world";
	ASSERT_STREQ ( sRes.cstr(), "{hello,world" );
	sRes.AppendName ( "bla" );
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":" );
	sRes << "foo";
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo" );
	sRes << "bar";
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar" );
	sRes.AppendName ( "bar" ).Sprintf ( "%d", 1000 );
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000" );
	ScopedComma_c sOne ( sRes, ";", "[", "]" );
	sRes.AppendName ( "foo" ) << "bar";
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":bar" );
	sRes.SkipNextComma();
	sRes << "baz";
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz" );
	sRes << "end";
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end" );
	sRes.FinishBlock();
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end]" );
	sRes << "End";
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End" );
	sRes.AppendName ( "arr" );
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":" );
	sRes.StartBlock ( "|", "[", "]" );
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":" );
	sRes.FinishBlock ( false );
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":[]" );
	sRes.AppendName ( "a" ).StartBlock ( "|", "[", "]" );
	sRes << "b";
	sRes.FinishBlock();
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":[],\"a\":[b]" );
	sRes.FinishBlock();
	ASSERT_STREQ ( sRes.cstr(), "{hello,world,\"bla\":foo,bar,\"bar\":1000,[\"foo\":barbaz;end],End,\"arr\":[],\"a\":[b]}" );
}

TEST ( functions, EscapedStringBuilderAndSkipCommas )
{
	// generic const char* with different escapes, exclude comma
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eNone | EscBld::eSkipComma, "firstspace\t and\r 'tab'\n here" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eFixupSpace | EscBld::eSkipComma, "firstspace  and  'tab'  here" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eEscape | EscBld::eSkipComma, "first'space\t and\r \\'tab\\'\n here'" );
	esc_first_comma ( "space\t and\r 'tab'\n here", EscBld::eAll | EscBld::eSkipComma, "first'space  and  \\'tab\\'  here'" );

	// null with different escapes, exclude comma
	esc_first_comma ( nullptr, EscBld::eNone | EscBld::eSkipComma, "first" );
	esc_first_comma ( nullptr, EscBld::eFixupSpace | EscBld::eSkipComma, "first" );
	esc_first_comma ( nullptr, EscBld::eEscape | EscBld::eSkipComma, "first''" );
	esc_first_comma ( nullptr, EscBld::eAll | EscBld::eSkipComma, "first''" );
}


TEST ( functions, JsonEscapedBuilder_sugar )
{
	JsonEscapedBuilder tOut;

	// scoped name
	{
		auto _ = tOut.Array();
		auto tNamed = tOut.Named ( "test1" );
		tOut << "one"
			 << "two";
		tOut.AppendEscaped ( "blabla" );
	};
	EXPECT_STREQ ( tOut.cstr(), "[\"test1\":onetwo\"blabla\"]" );

	// scoped immediate name
	tOut.Clear();
	{
		auto _ = tOut.Array();
		tOut.Named ( "test1" ).Sink() << "one"
									  << "two";
		tOut.AppendEscaped ( "blabla" );
	};
	EXPECT_STREQ ( tOut.cstr(), "[\"test1\":onetwo,\"blabla\"]" );

	// block name
	tOut.Clear();
	{
		auto _ = tOut.Array();
		tOut.NamedBlock ( "test1" );
		tOut << "one"
			 << "two";
		tOut.AppendEscaped ( "blabla" );
		tOut.FinishBlock();
	}
	EXPECT_STREQ ( tOut.cstr(), "[\"test1\":onetwo\"blabla\"]" );

	// scoped object
	tOut.Clear();
	{
		auto tObj = tOut.Object();
		tOut.Named ( "val1" ).Sink() << 1;
		tOut.Named ( "val2" ).Sink() << 2;
	}
	EXPECT_STREQ ( tOut.cstr(), "{\"val1\":1,\"val2\":2}" );

	// scoped immediate object
	tOut.Clear();
	( tOut.Object().Sink().AppendName ( "val1" ) << 1 ).AppendName ( "val2" ) << 2;
	EXPECT_STREQ ( tOut.cstr(), "{\"val1\":1,\"val2\":2}" );

	// block object
	tOut.Clear();
	tOut.ObjectBlock();
	tOut.Named ( "val1" ).Sink() << 1;
	tOut.Named ( "val2" ).Sink() << 2;
	tOut.FinishBlocks();
	EXPECT_STREQ ( tOut.cstr(), "{\"val1\":1,\"val2\":2}" );

	// scoped array
	tOut.Clear();
	{
		auto tObj = tOut.Array();
		tOut << 1 << 2 << 3 << 4;
	}
	EXPECT_STREQ ( tOut.cstr(), "[1,2,3,4]" );

	// scoped immediate array
	tOut.Clear();
	tOut.Array().Sink() << 1 << 2 << 3 << 4;
	EXPECT_STREQ ( tOut.cstr(), "[1,2,3,4]" );

	// block array
	tOut.Clear();
	{
		auto _ = tOut.Array();
		tOut << 1 << 2 << 3 << 4;
	}
	EXPECT_STREQ ( tOut.cstr(), "[1,2,3,4]" );

	// scoped immediate warray
	tOut.Clear();
	tOut.ArrayW().Sink() << 1 << 2 << 3 << 4;
	EXPECT_STREQ ( tOut.cstr(), "[\n1,\n2,\n3,\n4\n]" );

	// control characters immediate warray
	tOut.Clear();
	tOut.AppendEscaped ( "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f", EscBld::eAll, 16 );
	EXPECT_STREQ ( tOut.cstr(), "\"\\u0000\\u0001\\u0002\\u0003\\u0004\\u0005\\u0006\\u0007\\b\\t\\n\\u000b\\f\\r\\u000e\\u000f\"" );

	tOut.Clear();
	tOut.AppendEscaped ( "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f" );
	EXPECT_STREQ ( tOut.cstr(), "\"\\u0010\\u0011\\u0012\\u0013\\u0014\\u0015\\u0016\\u0017\\u0018\\u0019\\u001a\\u001b\\u001c\\u001d\\u001e\\u001f\"" );
}
