//
// Created by alexey on 25.08.17.
//

#include <gtest/gtest.h>

#include "sphinxint.h"


// Miscelaneous tests mostly processing texts with many test cases: HTML Stripper, levenstein,
// wildcards, expr parser,

TEST( Text, HTMLStriper )
{
	const char * sTests[][4] =
	{
		// source-data, index-attrs, remove-elements, expected-results
		{ "<?xml broken piece of shit/>should be indexed", "", "", "should be indexed" },
		{ "<?xml crazily=\"broken>shit\">still should be indexed", "", "", "shit\">still should be indexed" },
		{ "MOVING? HATE TO PACK ?HIRE A TRUCK WE\'LL DO THE REST! CLICK HERE<?<?<?", "", "", "MOVING? HATE TO PACK ?HIRE A TRUCK WE'LL DO THE REST! CLICK HERE" },
		{ "<html>trivial test</html>", "", "", " trivial test " },
		{ "<html>lets <img src=\"g/smth.jpg\" alt=\"nice picture\">index attrs</html>", "img=alt", "", " lets nice picture index attrs " },
		{ "<html>   lets  also<script> whatever here; a<b</script>remove scripts", "", "script, style", " lets also remove scripts" },
		{ "testing in<b><font color='red'>line</font> ele<em>men</em>ts", "", "", "testing inline elements" },
		{ "testing non<p>inline</h1>elements", "", "", "testing non inline elements" },
		{ "testing&nbsp;entities&amp;stuff", "", "", "testing entities&stuff" },
		{ "testing &#1040;&#1041;&#1042; utf encoding", "", "", "testing \xD0\x90\xD0\x91\xD0\x92 utf encoding" },
		{ "testing <1 <\" <\x80 <\xe0 <\xff </3 malformed tags", "", "", "testing <1 <\" <\x80 <\xe0 <\xff </3 malformed tags" },
		{ "testing comm<!--comm-->ents", "", "", "testing comments" },
		{ "&lt; &gt; &thetasym; &somethingverylong; &the", "", "", "< > \xCF\x91 &somethingverylong; &the" },
		{ "testing <img src=\"g/smth.jpg\" alt=\"nice picture\" rel=anotherattr junk=throwaway>inline tags vs attr indexing", "img=alt,rel", "", "testing nice picture anotherattr inline tags vs attr indexing" },
		{ "this <?php $code = \"must be stripped\"; ?> away", "", "", "this away" },
		{ "<a href=\"http://www.com\">content1</a>", "a=title", "", "content1" },
		{ "<a href=\"http://www.com\" title=\"my test title\">content2</a>", "a=title", "", "my test title content2" },
		{ "testing <img src=\"g/smth.jpg\" alt=\"nice picture\" rel=anotherattr junk=\"throwaway\">inline tags vs attr indexing", "img=alt,rel", "", "testing nice picture anotherattr inline tags vs attr indexing" },
		{ "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"><html>test</html>", "", "", " test " },
		{ "<!smth \"that>can<break\"><html>test</html>", "", "", " test " },
		{ "<TABLE CLASS=\"MSONORMALTABLE\" STYLE=\"BORDER-COLLAPSE: COLLAPSE; MARGIN-LEFT: ID=\"TABLE76\"><TR><TD>ohai</TD></TR></TABLE>", "", "", " ohai " },
		{ "ohai2<table class", "", "", "ohai2 " },
		{ "ohai<table class>3", "", "", "ohai 3" },
		{ "ohai<table class >4", "", "", "ohai 4" },
		{ "ohai<table class =>5", "", "", "ohai 5" },
		{ "ohai<table class =\"smth><tr><td>6</td></tr></table> some more content", "", "", "ohai 6 some more content" },
		{ "ohai<table nowrap class=\"a>b\">7", "", "", "ohai 7" },
		{ "ohai<table nowrap class =\"a>b\">8", "", "", "ohai 8" },
		{ "ohai<table nowrap class= \"a>b\">9", "", "", "ohai 9" },
		{ "ohai<table now rap class=\"a>b\">10", "", "", "ohai 10" },
		{ "ohai<table class = \"smth><tr><td>6</td><td class=\"test\">11</td></tr></table> gimme more", "", "", "ohai 11 gimme more" },
		{ "<P ALIGN=\"LEFT STYLE=\"MARGIN:0IN 0IN .0001PT;TEXT-ALIGN:LEFT;\"><B><FONT SIZE=\"2\" FACE=\"TIMES NEW ROMAN\" STYLE=\"FONT-SIZE:10.0PT;FONT-WEIGHT:BOLD;\">Commission File Number: 333-155507", "", "", " Commission File Number: 333-155507" },
		{ "<TD NOWRAP ALIGN=RIGHT STYLE=\"BORDER-BOTTOM: #000000 1PX SOLID; BORDER-TOP: #000000 1PX SOLID;\"\"><B>SGX", "", "", " SGX" },
		{ "tango & cash", "", "", "tango & cash" },
		{ "<font CLASS=\"MSONORMALTABLE\" STYLE=\"BORDER-COLLAPSE: COLLAPSE; MARGIN-LEFT: ID=\"TABLE76\">ahoy\"mate", "font=zzz", "", "ahoy\"mate" },
		{ "ahoy<font class =>2", "font=zzz", "", "ahoy2" },
		{ "ahoy<font class =\"smth><b>3</b></font>there", "font=zzz", "", "ahoy3there" },
		{ "ahoy<font nowrap class=\"a>b\">4", "font=zzz", "", "ahoy4" },
		{ "ahoy<font now rap class=\"a>b\">5", "font=zzz", "", "ahoy5" },
		{ "ahoy<font class = \"smth><b><i>6</i><b class=\"test\">seven</b></i></font>eight", "font=zzz", "", "ahoyseveneight" },
		{ "testing &#xC0; &#x2116; &#x0116;1 numbers utf encoding", "", "", "testing \xC3\x80 \xE2\x84\x96 \xC4\x96\x31 numbers utf encoding" }
	};

	int nTests = sizeof ( sTests ) / sizeof ( sTests[0] );
	for ( auto iTest = 0; iTest<nTests; ++iTest )
	{
		CSphString sError;
		CSphHTMLStripper tStripper ( true );
		ASSERT_TRUE ( tStripper.SetIndexedAttrs ( sTests[iTest][1], sError ) );
		ASSERT_TRUE ( tStripper.SetRemovedElements ( sTests[iTest][2], sError ) );

		CSphString sBuf ( sTests[iTest][0] );
		tStripper.Strip ( ( BYTE * ) sBuf.cstr () );
		ASSERT_STREQ ( sBuf.cstr (), sTests[iTest][3] ) << "test " << 1+iTest << "/" << nTests;
	}
}



//////////////////////////////////////////////////////////////////////////

static int ProxyLevenshtein ( const char * sA, const char * sB )
{
	int iLenA = strlen ( sA );
	int iLenB = strlen ( sB );
	return sphLevenshtein ( sA, iLenA, sB, iLenB );
}

TEST ( Text, Levenshtein )
{
	ASSERT_EQ ( ProxyLevenshtein ( "a", "b" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "ab", "ac" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "ac", "bc" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "abc", "axc" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "kitten", "sitting" ), 3 );
	ASSERT_EQ ( ProxyLevenshtein ( "xabxcdxxefxgx", "1ab2cd34ef5g6" ), 6 );
	ASSERT_EQ ( ProxyLevenshtein ( "cat", "cow" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "xabxcdxxefxgx", "abcdefg" ), 6 );
	ASSERT_EQ ( ProxyLevenshtein ( "javawasneat", "scalaisgreat" ), 7 );
	ASSERT_EQ ( ProxyLevenshtein ( "example", "samples" ), 3 );
	ASSERT_EQ ( ProxyLevenshtein ( "sturgeon", "urgently" ), 6 );
	ASSERT_EQ ( ProxyLevenshtein ( "levenshtein", "frankenstein" ), 6 );
	ASSERT_EQ ( ProxyLevenshtein ( "distance", "difference" ), 5 );
	ASSERT_EQ ( ProxyLevenshtein ( "abc", "xyz" ), 3 );
	ASSERT_EQ ( ProxyLevenshtein ( "abc", "a" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "a", "abc" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "abc", "c" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "c", "abc" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "cake", "drake" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "drake", "cake" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "saturday", "sunday" ), 3 );
	ASSERT_EQ ( ProxyLevenshtein ( "sunday", "saturday" ), 3 );
	ASSERT_EQ ( ProxyLevenshtein ( "book", "back" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "dog", "fog" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "foq", "fog" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "fvg", "fog" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "encyclopedia", "encyclopediaz" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "encyclopediz", "encyclopediaz" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "chukumwong", "ckwong" ), 4 );
	ASSERT_EQ ( ProxyLevenshtein ( "ckwong", "chukumwong" ), 4 );

	ASSERT_EQ ( ProxyLevenshtein ( "folden", "older" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "folden", "melden" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "folden", "scolded" ), 3 );
	ASSERT_EQ ( ProxyLevenshtein ( "goldin", "holding" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "goldin", "soldier" ), 3 );
	ASSERT_EQ ( ProxyLevenshtein ( "helden", "hielden" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "helden", "sheldon" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "helena", "helens" ), 1 );
	ASSERT_EQ ( ProxyLevenshtein ( "helena", "helllena" ), 2 );
	ASSERT_EQ ( ProxyLevenshtein ( "helga", "belgrave" ), 4 );
	ASSERT_EQ ( ProxyLevenshtein ( "helga", "anhel" ), 4 );
}


//////////////////////////////////////////////////////////////////////////

TEST ( Wildcards, simple )
{
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "abc" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "?bc" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "a?c" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "ab?" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abc", "?ab" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abac", "a*c" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abac", "a*?c" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abac", "a*??c" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abac", "a?*?c" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abac", "a*???c" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abac", "a?a?" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abac", "a?a??" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abac", "a??a" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abracadabra", "a*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abracadabra", "a*a" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abracadabra", "a*c" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abracadabra", "?b*r?" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abracadabra", "?b*r*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abracadabra", "?b*r*r*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abracadabra", "*a*a*a*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abracadabra", "*a*a*a*a*a*" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "a", "a*a?" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abracadabra", "*a*a*a*a*a?" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "car", "car%" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "cars", "car%" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "card", "car%" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "carded", "car%" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "abc%" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abcd", "abc%" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abcde", "abc%" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "ab", "a%b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "acb", "a%b" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "acdb", "a%b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "a%bc" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abbc", "a%bc" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abbbc", "a%bc" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "ab", "a%%b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "axb", "a%%b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "axyb", "a%%b" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "axyzb", "a%%b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "a*b", "a?b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "a*b", "a*b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "a*b", "a\\*b" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "acb", "a\\*b" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "acdeb", "a\\*b" ) );
}

TEST ( Wildcards, recursive_slow )
{
	// new cases recursive slow cases
	ASSERT_FALSE ( sphWildcardMatch ( "-----this-li", "-*-*-*-" ) );
	ASSERT_FALSE (
		sphWildcardMatch ( "---------------------------------this-li", "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" ) );
	ASSERT_TRUE (
		sphWildcardMatch ( "---------------------------------this-li-", "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-" ) );
	ASSERT_TRUE (
		sphWildcardMatch ( "---------------------------------this-li", "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*i" ) );
	ASSERT_TRUE (
		sphWildcardMatch ( "---------------------------------this-li", "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*" ) );
	ASSERT_TRUE (
		sphWildcardMatch ( "---------------------------------this-li", "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*s-*i" ) );
	ASSERT_FALSE (
		sphWildcardMatch ( "---------------------------------this-li", "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-s-*i" ) );
	ASSERT_FALSE (
		sphWildcardMatch ( "---------------------------------this-li", "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*x-*i" ) );
	ASSERT_FALSE (
		sphWildcardMatch ( "--------------------------this-li--p---", "-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-z-*-" ) );
}

TEST ( Wildcards, repeating_character_sequences )
{
	// cases with repeating character sequences
	ASSERT_TRUE ( sphWildcardMatch ( "abcccd", "*ccd" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "mississipissippi", "*issip*ss*" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "xxxx*zzzzzzzzy*f", "xxxx*zzy*fffff" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "xxxx*zzzzzzzzy*f", "xxx*zzy*f" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "xxxxzzzzzzzzyf", "xxxx*zzy*fffff" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "xxxxzzzzzzzzyf", "xxxx*zzy*f" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "xyxyxyzyxyz", "xy*z*xyz" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "mississippi", "*sip*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "xyxyxyxyz", "xy*xyz" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "mississippi", "mi*sip*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "ababac", "*abac*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "ababac", "*abac*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "aaazz", "a*zz*" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "a12b12", "*12*23" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "a12b12", "a12b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "a12b12", "*12*12*" ) );
}

TEST ( Wildcards, in_same_string )
{
	// wildcard in the tame string
	ASSERT_TRUE ( sphWildcardMatch ( "*", "*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "a*abab", "a*b" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "a*r", "a*" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "a*ar", "a*aar" ) );
}

TEST ( Wildcards, Double )
{
	// double wildcard
	ASSERT_TRUE ( sphWildcardMatch ( "XYXYXYZYXYz", "XY*Z*XYz" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "missisSIPpi", "*SIP*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "mississipPI", "*issip*PI" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "xyxyxyxyz", "xy*xyz" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "miSsissippi", "mi*sip*" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "miSsissippi", "mi*Sip*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abAbac", "*Abac*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abAbac", "*Abac*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "aAazz", "a*zz*" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "A12b12", "*12*23" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "a12B12", "*12*12*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "oWn", "*oWn*" ) );
}

TEST ( Wildcards, mixed )
{
	// mixed wildcard
	ASSERT_TRUE ( sphWildcardMatch ( "a", "*?" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "ab", "*?" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "*?" ) );
}

TEST ( Wildcards, false_positives )
{
	// wildcard false positives
	ASSERT_FALSE ( sphWildcardMatch ( "a", "??" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "ab", "?*?" ) );
	// due to loop just right after case '*'
	// skip all the extra stars and question marks
	// this case has opposite result
	ASSERT_FALSE ( sphWildcardMatch ( "ab", "*?*?*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "?**?*?" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abc", "?**?*&?" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abcd", "?b*??" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abcd", "?a*??" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abcd", "?**?c?" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abcd", "?**?d?" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abcde", "?*b*?*d*?" ) );
}

TEST ( Wildcards, single_char_match )
{
	// single char match
	ASSERT_TRUE ( sphWildcardMatch ( "bLah", "bL?h" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "bLaaa", "bLa?" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "bLah", "bLa?" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "bLaH", "?Lah" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "bLaH", "?LaH" ) );
}

TEST ( Wildcards, many )
{
	// many wildcard
	ASSERT_TRUE (
		sphWildcardMatch ( "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab"
						   , "a*a*a*a*a*a*aa*aaa*a*a*b" ) );
	ASSERT_TRUE ( sphWildcardMatch (
		"abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab"
		, "*a*b*ba*ca*a*aa*aaa*fa*ga*b*" ) );
	ASSERT_FALSE ( sphWildcardMatch (
		"abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab"
		, "*a*b*ba*ca*a*x*aaa*fa*ga*b*" ) );
	ASSERT_FALSE ( sphWildcardMatch (
		"abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab"
		, "*a*b*ba*ca*aaaa*fa*ga*gggg*b*" ) );
	ASSERT_TRUE ( sphWildcardMatch (
		"abababababababababababababababababababaacacacacacacacadaeafagahaiajakalaaaaaaaaaaaaaaaaaffafagaagggagaaaaaaaab"
		, "*a*b*ba*ca*aaaa*fa*ga*ggg*b*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "aaabbaabbaab", "*aabbaa*a*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*", "a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "aaaaaaaaaaaaaaaaa", "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "aaaaaaaaaaaaaaaa", "*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*a*" ) );
	ASSERT_FALSE ( sphWildcardMatch (
		"abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn"
		, "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*" ) );
	ASSERT_TRUE ( sphWildcardMatch (
		"abc*abcd*abcde*abcdef*abcdefg*abcdefgh*abcdefghi*abcdefghij*abcdefghijk*abcdefghijkl*abcdefghijklm*abcdefghijklmn"
		, "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abc*abcd*abcd*abc*abcd", "abc*abc*abc*abc*abc" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc*abcd*abcd*abc*abcd*abcd*abc*abcd*abc*abc*abcd"
								, "abc*abc*abc*abc*abc*abc*abc*abc*abc*abc*abcd" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "abc", "********a********b********c********" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "********a********b********c********", "abc" ) );
	ASSERT_FALSE ( sphWildcardMatch ( "abc", "********a********b********b********" ) );
	ASSERT_TRUE ( sphWildcardMatch ( "*abc*", "***a*b*c***" ) );
}

//////////////////////////////////////////////////////////////////////////

TEST ( Text, expression_parser )
{
	CSphColumnInfo tCol;
	tCol.m_eAttrType = SPH_ATTR_INTEGER;

	CSphSchema tSchema;
	tCol.m_sName = "aaa";
	tSchema.AddAttr ( tCol, false );
	tCol.m_sName = "bbb";
	tSchema.AddAttr ( tCol, false );
	tCol.m_sName = "ccc";
	tSchema.AddAttr ( tCol, false );

	auto * pRow = new CSphRowitem[tSchema.GetRowSize ()];
	for ( auto i = 0; i<tSchema.GetRowSize (); ++i )
		pRow[i] = 1 + i;

	CSphMatch tMatch;
	tMatch.m_uDocID = 123;
	tMatch.m_iWeight = 456;
	tMatch.m_pStatic = pRow;

	struct ExprTest_t
	{
		const char * m_sExpr;
		float m_fValue;
	};
	ExprTest_t dTests[] =
		{
			{   "ccc/2"                       , 1.5f }
			, { "1*2*3*4*5*6*7*8*9*10"        , 3628800.0f }
			, { "aaa+bbb*sin(0)*ccc"          , 1.0f }
			, { "if(pow(sqrt(2),2)=2,123,456)", 123.0f }
			, { "if(2<2,3,4)"                 , 4.0f }
			, { "if(2>=2,3,4)"                , 3.0f }
			, { "pow(7,5)"                    , 16807.f }
			, { "sqrt(3)"                     , 1.7320508f }
			, { "log2((2+2)*(2+2))"           , 4.0f }
			, { "min(3,15)"                   , 3.0f }
			, { "max(3,15)"                   , 15.0f }
			, { "if(3<15,bbb,ccc)"            , 2.0f }
			, { "@id+@weight"                 , 579.0f }
			, { "abs(-3-ccc)"                 , 6.0f }
			, { "(aaa+bbb)*(ccc-aaa)"         , 6.0f }
			, { "(((aaa)))"                   , 1.0f }
			, { "aaa-bbb*ccc"                 , -5.0f }
			, { " aaa    -\tbbb *\t\t\tccc "  , -5.0f }
			, { "bbb+123*aaa"                 , 125.0f }
			, { "2.000*2e+1+2"                , 42.0f }
			, { "3<5"                         , 1.0f }
			, { "1 + 2*3 > 4*4"               , 0.0f }
			, { "aaa/-bbb"                    , -0.5f, }
			, { "-10*-10"                     , 100.0f }
			, { "aaa+-bbb*-5"                 , 11.0f }
			, { "-aaa>-bbb"                   , 1.0f }
			, { "1-aaa+2-3+4"                 , 3.0f }
			, { "bbb/1*2/6*3"                 , 2.0f }
			, { "(aaa+bbb)/sqrt(3)/sqrt(3)"   , 1.0f }
			, { "aaa-bbb-2"                   , -3.0f }
			, { "ccc/2*4/bbb"                 , 3.0f }
			, { "(2+(aaa*bbb))+3"             , 7.0f }
		};

	for ( auto &dTest: dTests )
	{
		CSphString sError;
		CSphScopedPtr<ISphExpr> pExpr ( sphExprParse ( dTest.m_sExpr, tSchema, NULL, NULL, sError, NULL ) );
		ASSERT_TRUE ( pExpr.Ptr () ) << sError.cstr ();
		ASSERT_FLOAT_EQ ( dTest.m_fValue, pExpr->Eval ( tMatch ) );
	}

	SafeDeleteArray ( pRow );
}

//////////////////////////////////////////////////////////////////////////

TEST ( Text, ArabicStemmer )
{
	// a few words, cross-verified using NLTK implementation
	const char * dTests[] =
	{
		"\xd8\xb0\xd9\x87\xd8\xa8\xd8\xaa\0", "\xd8\xb0\xd9\x87\xd8\xa8\0",
		"\xd8\xa7\xd9\x84\xd8\xb7\xd8\xa7\xd9\x84\xd8\xa8\xd8\xa9\0", "\xd8\xb7\xd9\x84\xd8\xa8\0",
		"\xd8\xa7\xd9\x84\xd8\xb5\xd8\xba\xd9\x8a\xd8\xb1\xd8\xa9\0", "\xd8\xb5\xd8\xba\xd8\xb1\0",
		"\xd8\xa7\xd9\x84\xd9\x89\0", "\xd8\xa7\xd9\x84\xd9\x89\0",
		"\xd8\xa7\xd9\x84\xd9\x85\xd8\xaf\xd8\xb1\xd8\xb3\xd8\xa9\0", "\xd8\xaf\xd8\xb1\xd8\xb3\0",
		"\xd9\x88\xd8\xaf\xd8\xb1\xd8\xb3\xd8\xaa\0", "\xd8\xaf\xd8\xb1\xd8\xb3\0",
		"\xd8\xa7\xd9\x84\xd8\xaf\xd8\xb1\xd9\x88\xd8\xb3\0", "\xd8\xaf\xd8\xb1\xd8\xb3\0",
		"\xd8\xac\xd9\x85\xd9\x8a\xd8\xb9\xd9\x87\xd8\xa7\0", "\xd8\xac\xd9\x85\xd8\xb9\0",
		"\xd9\x88\xd8\xad\xd9\x8a\xd9\x86\0", "\xd9\x88\xd8\xad\xd9\x86\0",
		// "\xd8\xac\xd8\xa7\xd8\xa1\0", "\xd8\xac\xd8\xa7\xd8\xa1\0",
		"\xd9\x88\xd9\x82\xd8\xaa\0", "\xd9\x88\xd9\x82\xd8\xaa\0",
		// "\xd8\xa7\xd9\x84\xd8\xa7\xd8\xae\xd8\xaa\xd8\xa8\xd8\xa7\xd8\xb1\0", "\xd8\xae\xd8\xa8\xd8\xb1\0",
		"\xd9\x86\xd8\xac\xd8\xad\xd8\xaa\0", "\xd9\x86\xd8\xac\xd8\xad\0",
		"\xd8\xb7\xd8\xa7\xd9\x84\xd8\xa8\xd8\xaa\xd9\x86\xd8\xa7\0", "\xd8\xb7\xd9\x84\xd8\xa8\0",
		"\xd8\xa8\xd8\xa7\xd9\x85\xd8\xaa\xd9\x8a\xd8\xa7\xd8\xb2\0", "\xd9\x85\xd9\x8a\xd8\xb2\0",
		"\xd8\xa7\xd9\x84\xd9\x85\xd8\xaf\xd8\xa7\xd8\xb1\xd8\xb3\0", "\xd8\xaf\xd8\xb1\xd8\xb3\0",
		"\xd9\x84\xd9\x87\xd8\xa7\0", "\xd9\x84\xd9\x87\xd8\xa7\0",
		"\xd8\xaf\xd9\x88\xd8\xb1\0", "\xd8\xaf\xd9\x88\xd8\xb1\0",
		"\xd9\x83\xd8\xa8\xd9\x8a\xd8\xb1\0", "\xd9\x83\xd8\xa8\xd8\xb1\0",
		"\xd9\x81\xd9\x8a\0", "\xd9\x81\xd9\x8a\0",
		"\xd8\xaa\xd8\xb9\xd9\x84\xd9\x8a\xd9\x85\0", "\xd8\xb9\xd9\x84\xd9\x85\0",
		"\xd8\xa7\xd8\xa8\xd9\x86\xd8\xa7\xd9\x8a\xd9\x86\xd8\xa7\0", "\xd8\xa8\xd9\x86\xd9\x8a\0",
		// "\xd8\xa7\xd9\x84\xd8\xa7\xd8\xad\xd8\xa8\xd8\xa7\xd8\xa1\0", "\xd8\xad\xd8\xa8\xd8\xa1\0",
	};

	for ( int i = 0; i<int ( sizeof ( dTests ) / sizeof ( dTests[0] ) ); i += 2 )
	{
		char sBuf[64];
		snprintf ( sBuf, sizeof ( sBuf ), "%s", dTests[i] );
		stem_ar_utf8 ( ( BYTE * ) sBuf );
		ASSERT_STREQ ( sBuf, dTests[i + 1] );
	}

	char sTest1[16] = "\xD9\x80\xD9\x80\xD9\x80\xD9\x80\0abcdef";
	char sRef1[16] = "\0\0\0\0\0\0\0\0\0abcdef";

	stem_ar_utf8 ( ( BYTE * ) sTest1 );
	ASSERT_FALSE ( memcmp ( sTest1, sRef1, sizeof ( sTest1 ) ) );


	char sTest2[] = "\xd8\xa7\xd9\x84\xd8\xb7\xd8\xa7\xd9\x84\xd8\xa8\xd8\xa9\0";
	char sRef2[] = "\xd8\xb7\xd9\x84\xd8\xa8\0";
	CSphTightVector<BYTE> dTest22;
	dTest22.Resize ( sizeof ( sTest2 ) );
	for ( int i = 0; i<10; i++ )
	{
		dTest22.Resize ( dTest22.GetLength () * 2 );
		int iOff = dTest22.GetLength () - sizeof ( sTest2 );
		memcpy ( dTest22.Begin () + iOff, sTest2, sizeof ( sTest2 ) );
		stem_ar_utf8 ( dTest22.Begin () + iOff );
		ASSERT_FALSE ( memcmp ( dTest22.Begin () + iOff, sRef2, sizeof ( sRef2 ) ) );
	}
}

//////////////////////////////////////////////////////////////////////////

TEST ( Text, cvs_source )
{
	int iWriteStride = 7;
	const char * dTest[] = {
		"1,\"a,b \"\" c\",\"d \"\"a\"\" c\",\"the\tdox\n fox\",tmp,tmp,tmp,11\n",
		"a,b \" c", "d \"a\" c", "the\tdox\n fox", "tmp", "tmp", "tmp",

		"2,\"abc, defghijk. \"Lmnopqrs, \"tuv,\"\" wxyz.\",...,tmp,tmp,tmp,11\n",
		"abc, defghijk. Lmnopqrs", " tuv,\" wxyz.", "...", "tmp", "tmp", "tmp",

		"3,\",\",\"\",tmp,tmp,tmp,tmp,11\n",
		",", "", "tmp", "tmp", "tmp", "tmp",

		"4,\"Sup, \"\"puper\"\", duper\",tmp,tmp,tmp,tmp,tmp,11\n",
		"Sup, \"puper\", duper", "tmp", "tmp", "tmp", "tmp", "tmp",

		"5,\"Sup, \"\"puper\"\" duper\",tmp,tmp,tmp,tmp,tmp,11\n",
		"Sup, \"puper\" duper", "tmp", "tmp", "tmp", "tmp", "tmp",

		"6,\"Sup, \"\"puper\"\"\","",tmp,tmp,tmp,tmp,11\n",
		"Sup, \"puper\"", "", "tmp", "tmp", "tmp", "tmp",

		"7,\"Sup, \"\"puper, duper\"\"\",,tmp,tmp,tmp,tmp,11\n",
		"Sup, \"puper, duper\"", "", "tmp", "tmp", "tmp", "tmp",

		"8,cool,so far,\"Sup\n extra, duper,\",tmp,tmp,tmp,11\n",
		"cool", "so far", "Sup\n extra, duper,", "tmp", "tmp", "tmp",

		"9,//\\\\match//\\\\,//\\\\double//\\\\,//\\\\escape//\\\\,tmp,tmp,tmp,11\n",
		"//\\match//\\", "//\\double//\\", "//\\escape//\\", "tmp", "tmp", "tmp",

		"10,ma\\\"tch,me,ten\\\"der,tmp,tmp,tmp,11\n",
		"ma\"tch", "me", "ten\"der", "tmp", "tmp", "tmp",

		"11,"
		"test fest \\\" best"
		",tmp,tmp,tmp,tmp,tmp,11\n",
		"test fest \" best",
		"tmp", "tmp", "tmp", "tmp", "tmp",

		"12,"
		"\"test fest \\\" be\"st\""
		",tmp,tmp,tmp,tmp,tmp,11\n",
		"test fest \\ be\"st\"",
		"tmp", "tmp", "tmp", "tmp", "tmp",

		"13,"
		"\"test fest, be\"st of, the\""
		",tmp,tmp,tmp,tmp,11\n",
		"test fest, best of", " the\"",
		"tmp", "tmp", "tmp", "tmp",

		"14,"
		"\"test fest, best of, th\"e"
		",tmp,tmp,tmp,tmp,tmp,11\n",
		"test fest, best of, the",
		"tmp", "tmp", "tmp", "tmp", "tmp",

		"15,"
		"\"test fest\\, best of th\"e"
		",tmp,tmp,tmp,tmp,tmp,11\n",
		"test fest\\, best of the",
		"tmp", "tmp", "tmp", "tmp", "tmp",

		"16,"
		"test \"fest\\, best of th\"e"
		",tmp,tmp,tmp,tmp,11\n",
		"test \"fest", " best of th\"e",
		"tmp", "tmp", "tmp", "tmp",

		NULL
	};

	const char * cvs_tmpfile = "__libsphinxtestcvs.tmp";
	// write csv file
	FILE * fp = fopen ( cvs_tmpfile, "wb" );
	for ( int iTest = 0; dTest[iTest]!=NULL; iTest += iWriteStride )
		fwrite ( dTest[iTest], 1, strlen ( dTest[iTest] ), fp );
	fclose ( fp );

	// open csv pipe
	fp = fopen ( cvs_tmpfile, "rb" );

	// make config for 6 fields and attribute
	CSphConfigSection tConf;
	ASSERT_TRUE ( tConf.Add ( CSphVariant ( "f0", 0 ), "csvpipe_field" ) );
	CSphVariant &tTail = tConf["csvpipe_field"];
	tTail.m_pNext = new CSphVariant ( "f1", 1 );
	tTail.m_pNext->m_pNext = new CSphVariant ( "f2", 2 );
	tTail.m_pNext->m_pNext->m_pNext = new CSphVariant ( "f3", 3 );
	tTail.m_pNext->m_pNext->m_pNext->m_pNext = new CSphVariant ( "f4", 4 );
	tTail.m_pNext->m_pNext->m_pNext->m_pNext->m_pNext = new CSphVariant ( "f5", 5 );
	ASSERT_TRUE ( tConf.Add ( CSphVariant ( "gid", 6 ), "csvpipe_attr_uint" ) );

	// setup source
	CSphSource_Document * pCSV = ( CSphSource_Document * ) sphCreateSourceCSVpipe ( &tConf, fp, "csv", false );
	CSphString sError;
	ASSERT_TRUE ( pCSV->Connect ( sError ) );
	ASSERT_TRUE ( pCSV->IterateStart ( sError ) );

	// verify that config matches to source schema
	CSphSchema tSchema;
	ASSERT_TRUE ( pCSV->UpdateSchema ( &tSchema, sError ) );
	int iColumns = tSchema.GetFieldsCount();

	// check parsed fields
	for ( int iTest = 1;; )
	{
		BYTE ** pFields = pCSV->NextDocument ( sError );
		ASSERT_TRUE ( pFields || pCSV->m_tDocInfo.m_uDocID==0 );
		if ( pCSV->m_tDocInfo.m_uDocID==0 )
			break;

		for ( int i = 0; i<iColumns; i++ )
		{
			CSphString sTmp ( ( const char * ) pFields[i] );
			ASSERT_STREQ ( sTmp.cstr(), dTest[iTest + i] );
		}

		iTest += iWriteStride;
	}

	// clean up, fp will be closed automatically in CSphSource_BaseSV::Disconnect()
	SafeDelete ( pCSV );
	unlink ( cvs_tmpfile );
}


