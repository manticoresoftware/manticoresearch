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

#include "html_stripper.h"

#include "sphinxutils.h"
#include "sphinxint.h"
#include "tokenizer/tokenizer.h"


/////////////////////////////////////////////////////////////////////////////
// HTML STRIPPER
/////////////////////////////////////////////////////////////////////////////


#if !_WIN32
char* strlwr ( char* s )
{
	while ( *s )
	{
		*s = tolower ( *s );
		s++;
	}
	return s;
}
#endif

static inline int sphIsTag ( int c )
{
	return sphIsAlpha(c) || c=='.' || c==':';
}

static inline int sphIsTagStart ( int c )
{
	return ( c>='a' && c<='z' ) || ( c>='A' && c<='Z' ) || c=='_' || c=='.' || c==':';
}

CSphHTMLStripper::CSphHTMLStripper ( bool bDefaultTags )
{
	if ( bDefaultTags )
	{
		// known inline tags
		const char * dKnown[] =
		{
			"a", "b", "i", "s", "u",
			"basefont", "big", "em", "font", "img",
			"label", "small", "span", "strike", "strong",
			"sub\0", "sup\0", // fix gcc 3.4.3 on solaris10 compiler bug
			"tt"
		};

		m_dTags.Resize ( sizeof(dKnown)/sizeof(dKnown[0]) );
		ARRAY_FOREACH ( i, m_dTags )
		{
			m_dTags[i].m_sTag = dKnown[i];
			m_dTags[i].m_iTagLen = (int) strlen ( dKnown[i] );
			m_dTags[i].m_bInline = true;
		}
	}

	UpdateTags ();
}


int CSphHTMLStripper::GetCharIndex ( int iCh ) const
{
	if ( iCh>='a' && iCh<='z' ) return iCh-'a';
	if ( iCh>='A' && iCh<='Z' ) return iCh-'A';
	if ( iCh=='_' ) return 26;
	if ( iCh==':' ) return 27;
	return -1;
}


void CSphHTMLStripper::UpdateTags ()
{
	m_dTags.Sort ();

	for ( int i=0; i<MAX_CHAR_INDEX; i++ )
	{
		m_dStart[i] = INT_MAX;
		m_dEnd[i] = -1;
	}

	ARRAY_FOREACH ( i, m_dTags )
	{
		int iIdx = GetCharIndex ( m_dTags[i].m_sTag.cstr()[0] );
		if ( iIdx<0 )
			continue;

		m_dStart[iIdx] = Min ( m_dStart[iIdx], i );
		m_dEnd[iIdx] = Max ( m_dEnd[iIdx], i );
	}
}


bool CSphHTMLStripper::SetIndexedAttrs ( const char * sConfig, CSphString & sError )
{
	if ( !sConfig || !*sConfig )
		return true;

	char sTag[256], sAttr[256];

	const char * p = sConfig, * s;
	#define LOC_ERROR(_msg,_pos) { sError.SetSprintf ( "SetIndexedAttrs(): %s near '%s'", _msg, _pos ); return false; }

	while ( *p )
	{
		// skip spaces
		while ( *p && isspace(*p) ) p++;
		if ( !*p ) break;

		// check tag name
		s = p; while ( sphIsTag(*p) ) p++;
		if ( s==p ) LOC_ERROR ( "invalid character in tag name", s );

		// get tag name
		if ( p-s>=(int)sizeof(sTag) ) LOC_ERROR ( "tag name too long", s );
		strncpy ( sTag, s, p-s );
		sTag[p-s] = '\0';

		// skip spaces
		while ( *p && isspace(*p) ) p++;
		if ( *p++!='=' ) LOC_ERROR ( "'=' expected", p-1 );

		// add indexed tag entry, if not there yet
		strlwr ( sTag );

		int iIndexTag = -1;
		ARRAY_FOREACH ( i, m_dTags )
			if ( m_dTags[i].m_sTag==sTag )
		{
			iIndexTag = i;
			break;
		}
		if ( iIndexTag<0 )
		{
			m_dTags.Add();
			m_dTags.Last().m_sTag = sTag;
			m_dTags.Last().m_iTagLen = (int) strlen ( sTag );
			iIndexTag = m_dTags.GetLength()-1;
		}

		m_dTags[iIndexTag].m_bIndexAttrs = true;
		StrVec_t & dAttrs = m_dTags[iIndexTag].m_dAttrs;

		// scan attributes
		while ( *p )
		{
			// skip spaces
			while ( *p && isspace(*p) ) p++;
			if ( !*p ) break;

			// check attr name
			s = p; while ( sphIsTag(*p) ) p++;
			if ( s==p ) LOC_ERROR ( "invalid character in attribute name", s );

			// get attr name
			if ( p-s>=(int)sizeof(sAttr) ) LOC_ERROR ( "attribute name too long", s );
			strncpy ( sAttr, s, p-s );
			sAttr[p-s] = '\0';

			// add attr, if not there yet
			int iAttr;
			for ( iAttr=0; iAttr<dAttrs.GetLength(); iAttr++ )
				if ( dAttrs[iAttr]==sAttr )
					break;

			if ( iAttr==dAttrs.GetLength() )
				dAttrs.Add ( sAttr );

			// skip spaces
			while ( *p && isspace(*p) ) p++;
			if ( !*p ) break;

			// check if there's next attr or tag
			if ( *p==',' ) { p++; continue; } // next attr
			if ( *p==';' ) { p++; break; } // next tag
			LOC_ERROR ( "',' or ';' or end of line expected", p );
		}
	}

	#undef LOC_ERROR

	UpdateTags ();
	return true;
}


bool CSphHTMLStripper::SetRemovedElements ( const char * sConfig, CSphString & )
{
	if ( !sConfig || !*sConfig )
		return true;

	const char * p = sConfig;
	while ( *p )
	{
		// skip separators
		while ( *p && !sphIsTag(*p) ) p++;
		if ( !*p ) break;

		// get tag name
		const char * s = p;
		while ( sphIsTag(*p) ) p++;

		CSphString sTag;
		sTag.SetBinary ( s, int(p-s) );
		sTag.ToLower ();

		// mark it
		int iTag;
		for ( iTag=0; iTag<m_dTags.GetLength(); iTag++ )
			if ( m_dTags[iTag].m_sTag==sTag )
		{
			m_dTags[iTag].m_bRemove = true;
			break;
		}

		if ( iTag==m_dTags.GetLength() )
		{
			m_dTags.Add();
			m_dTags.Last().m_sTag = sTag;
			m_dTags.Last().m_iTagLen = (int) strlen ( sTag.cstr() );
			m_dTags.Last().m_bRemove = true;
		}
	}

	UpdateTags ();
	return true;
}


void CSphHTMLStripper::EnableParagraphs ()
{
	// known block-level elements
	const char * dBlock[] = { "address", "blockquote", "caption", "center",
		"dd", "div", "dl", "dt", "h1", "h2", "h3", "h4", "h5", "li", "menu",
		"ol", "p", "pre", "table", "tbody", "td", "tfoot", "th", "thead",
		"tr", "ul", NULL };

	for ( int iBlock=0; dBlock[iBlock]; iBlock++ )
	{
		const char * sTag = dBlock[iBlock];

		// mark if known already
		int iTag;
		for ( iTag=0; iTag<m_dTags.GetLength(); iTag++ )
			if ( m_dTags[iTag].m_sTag==sTag )
		{
			m_dTags[iTag].m_bPara = true;
			break;
		}

		// add if not known yet
		if ( iTag==m_dTags.GetLength() )
		{
			html_stripper::StripperTag_t& dTag = m_dTags.Add();
			dTag.m_sTag = sTag;
			dTag.m_iTagLen = (int) strlen(sTag);
			dTag.m_bPara = true;
		}
	}

	UpdateTags ();
}


bool CSphHTMLStripper::SetZones ( const char * sZones, CSphString & sError )
{
	// yet another mini parser!
	// index_zones = {tagname | prefix*} [, ...]
	if ( !sZones || !*sZones )
		return true;

	const char * s = sZones;
	while ( *s )
	{
		// skip spaces
		while ( sphIsSpace(*s) )
			s++;
		if ( !*s )
			break;

		// expect ident
		if ( !sphIsTagStart(*s) )
		{
			sError.SetSprintf ( "unexpected char near '%s' in index_zones", s );
			return false;
		}

		// get ident (either tagname or prefix*)
		const char * sTag = s;
		while ( sphIsTag(*s) )
			s++;

		const char * sTagEnd = s;
		bool bPrefix = false;
		if ( *s=='*' )
		{
			s++;
			bPrefix = true;
		}

		// skip spaces
		while ( sphIsSpace(*s) )
			s++;

		// expect eof or comma after ident
		if ( *s && *s!=',' )
		{
			sError.SetSprintf ( "unexpected char near '%s' in index_zones", s );
			return false;
		}
		if ( *s==',' )
			s++;

		// got valid entry, handle it
		html_stripper::StripperTag_t & tTag = m_dTags.Add();
		tTag.m_sTag.SetBinary ( sTag, int(sTagEnd-sTag) );
		tTag.m_iTagLen = (int)( sTagEnd-sTag );
		tTag.m_bZone = true;
		tTag.m_bZonePrefix = bPrefix;
	}

	UpdateTags ();
	return true;
}


const BYTE * SkipQuoted ( const BYTE * p )
{
	const BYTE * pMax = p + 512; // 512 bytes should be enough for a reasonable HTML attribute value, right?!
	const BYTE * pProbEnd = NULL; // (most) probable end location in case we don't find a matching quote
	BYTE cEnd = *p++; // either apostrophe or quote

	while ( p<pMax && *p && *p!=cEnd )
	{
		if ( !pProbEnd )
			if ( *p=='>' || *p=='\r' )
				pProbEnd = p;
		p++;
	}

	if ( *p==cEnd )
		return p+1;

	if ( pProbEnd )
		return pProbEnd;

	return p;
}


struct HtmlEntity_t
{
	const char *	m_sName;
	int				m_iCode;
};


static inline DWORD HtmlEntityHash ( const BYTE * str, int len )
{
	static const unsigned short asso_values[] =
	{
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 4,
		6, 22, 1, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 170, 48, 0, 5, 44,
		0, 10, 10, 86, 421, 7, 0, 1, 42, 93,
		41, 421, 0, 5, 8, 14, 421, 421, 5, 11,
		8, 421, 421, 421, 421, 421, 421, 1, 25, 27,
		9, 2, 113, 82, 14, 3, 179, 1, 81, 91,
		12, 0, 1, 180, 56, 17, 5, 31, 60, 7,
		3, 161, 2, 3, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421, 421, 421, 421,
		421, 421, 421, 421, 421, 421, 421
	};

	int hval = len;
	switch ( hval )
	{
		default:	hval += asso_values [ str[4] ];
		case 4:
		case 3:		hval += asso_values [ str[2] ];
		case 2:		hval += asso_values [ str[1]+1 ];
		case 1:		hval += asso_values [ str[0] ];
					break;
	}
	return hval + asso_values [ str[len-1] ];
}


static inline int HtmlEntityLookup ( const BYTE * str, int len )
{
	static const unsigned char lengthtable[] =
	{
		0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 3, 3,
		4, 3, 3, 5, 3, 6, 5, 5, 3, 4, 4, 5, 3, 4,
		4, 0, 5, 4, 5, 6, 5, 6, 4, 5, 3, 3, 5, 0,
		0, 0, 0, 6, 0, 5, 5, 0, 5, 6, 6, 3, 0, 3,
		5, 3, 0, 6, 0, 4, 3, 6, 3, 6, 6, 6, 6, 5,
		5, 5, 5, 5, 5, 2, 6, 4, 0, 6, 3, 3, 3, 0,
		4, 5, 4, 4, 4, 3, 7, 4, 3, 6, 2, 3, 6, 4,
		3, 6, 5, 6, 5, 5, 4, 2, 0, 0, 4, 6, 8, 0,
		0, 0, 5, 5, 0, 6, 6, 2, 2, 4, 4, 6, 6, 4,
		4, 5, 6, 2, 3, 4, 6, 5, 0, 2, 0, 0, 6, 6,
		6, 6, 6, 4, 6, 5, 0, 6, 4, 5, 4, 6, 6, 0,
		0, 4, 6, 5, 6, 0, 6, 4, 5, 6, 5, 6, 4, 0,
		3, 6, 0, 4, 4, 4, 5, 4, 6, 0, 4, 4, 6, 5,
		6, 7, 2, 2, 6, 2, 5, 2, 5, 0, 0, 0, 4, 4,
		2, 4, 2, 2, 4, 0, 4, 4, 4, 5, 5, 0, 3, 7,
		5, 0, 5, 6, 5, 0, 6, 0, 6, 0, 4, 6, 4, 6,
		6, 2, 6, 0, 5, 5, 4, 6, 6, 0, 5, 6, 4, 4,
		4, 4, 0, 5, 0, 5, 0, 4, 5, 4, 0, 4, 4, 4,
		0, 0, 0, 4, 0, 0, 0, 5, 6, 5, 3, 0, 0, 6,
		5, 4, 5, 5, 5, 5, 0, 5, 5, 0, 5, 0, 0, 0,
		4, 6, 0, 3, 0, 5, 5, 0, 0, 3, 6, 5, 0, 4,
		0, 0, 0, 0, 5, 7, 5, 3, 5, 3, 0, 0, 6, 0,
		6, 0, 0, 7, 0, 0, 5, 0, 5, 0, 0, 0, 0, 5,
		4, 0, 0, 0, 0, 0, 7, 4, 0, 0, 3, 0, 0, 0,
		3, 0, 6, 0, 0, 7, 5, 5, 0, 3, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 5,
		5, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 4, 6, 0, 0, 0, 0, 0, 0, 0,
		4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		5
	};

	static const struct HtmlEntity_t wordlist[] =
	{
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Rho", 929},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Chi", 935},
		{"phi", 966},
		{"iota", 953},
		{"psi", 968},
		{"int", 8747},
		{"theta", 952},
		{"amp", 38},
		{"there4", 8756},
		{"Theta", 920},
		{"omega", 969},
		{"and", 8743},
		{"prop", 8733},
		{"ensp", 8194},
		{"image", 8465},
		{"not", 172},
		{"isin", 8712},
		{"sdot", 8901},
		{"", 0},
		{"prime", 8242},
		{"prod", 8719},
		{"trade", 8482},
		{"Scaron", 352},
		{"kappa", 954},
		{"thinsp", 8201},
		{"emsp", 8195},
		{"thorn", 254},
		{"eta", 951},
		{"chi", 967},
		{"Kappa", 922},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"scaron", 353},
		{"", 0},
		{"notin", 8713},
		{"ndash", 8211},
		{"", 0},
		{"acute", 180},
		{"otilde", 245},
		{"atilde", 227},
		{"Phi", 934},
		{"", 0},
		{"Psi", 936},
		{"pound", 163},
		{"cap", 8745},
		{"", 0},
		{"otimes", 8855},
		{"", 0},
		{"nbsp", 32},
		{"rho", 961},
		{"ntilde", 241},
		{"eth", 240},
		{"oacute", 243},
		{"aacute", 225},
		{"eacute", 233},
		{"iacute", 237},
		{"nabla", 8711},
		{"Prime", 8243},
		{"ocirc", 244},
		{"acirc", 226},
		{"ecirc", 234},
		{"icirc", 238},
		{"or", 8744},
		{"Yacute", 221},
		{"nsub", 8836},
		{"", 0},
		{"Uacute", 218},
		{"Eta", 919},
		{"ETH", 208},
		{"sup", 8835},
		{"", 0},
		{"supe", 8839},
		{"Ucirc", 219},
		{"sup1", 185},
		{"para", 182},
		{"sup2", 178},
		{"loz", 9674},
		{"omicron", 959},
		{"part", 8706},
		{"cup", 8746},
		{"Ntilde", 209},
		{"Mu", 924},
		{"tau", 964},
		{"uacute", 250},
		{"Iota", 921},
		{"Tau", 932},
		{"rsaquo", 8250},
		{"alpha", 945},
		{"Ccedil", 199},
		{"ucirc", 251},
		{"oline", 8254},
		{"sup3", 179},
		{"nu", 957},
		{"", 0}, {"", 0},
		{"sube", 8838},
		{"Eacute", 201},
		{"thetasym", 977},
		{"", 0}, {"", 0}, {"", 0},
		{"Omega", 937},
		{"Ecirc", 202},
		{"", 0},
		{"lowast", 8727},
		{"iquest", 191},
		{"lt", 60},
		{"gt", 62},
		{"ordm", 186},
		{"euro", 8364},
		{"oslash", 248},
		{"lsaquo", 8249},
		{"zeta", 950},
		{"cong", 8773},
		{"mdash", 8212},
		{"ccedil", 231},
		{"ne", 8800},
		{"sub", 8834},
		{"Zeta", 918},
		{"Lambda", 923},
		{"Gamma", 915},
		{"", 0},
		{"Nu", 925},
		{"", 0}, {"", 0},
		{"ograve", 242},
		{"agrave", 224},
		{"egrave", 232},
		{"igrave", 236},
		{"frac14", 188},
		{"ordf", 170},
		{"Otilde", 213},
		{"infin", 8734},
		{"", 0},
		{"frac12", 189},
		{"beta", 946},
		{"radic", 8730},
		{"darr", 8595},
		{"Iacute", 205},
		{"Ugrave", 217},
		{"", 0}, {"", 0},
		{"harr", 8596},
		{"hearts", 9829},
		{"Icirc", 206},
		{"Oacute", 211},
		{"", 0},
		{"frac34", 190},
		{"cent", 162},
		{"crarr", 8629},
		{"curren", 164},
		{"Ocirc", 212},
		{"brvbar", 166},
		{"sect", 167},
		{"", 0},
		{"ang", 8736},
		{"ugrave", 249},
		{"", 0},
		{"Beta", 914},
		{"uarr", 8593},
		{"dArr", 8659},
		{"asymp", 8776},
		{"perp", 8869},
		{"Dagger", 8225},
		{"", 0},
		{"hArr", 8660},
		{"rang", 9002},
		{"dagger", 8224},
		{"exist", 8707},
		{"Egrave", 200},
		{"Omicron", 927},
		{"mu", 956},
		{"pi", 960},
		{"weierp", 8472},
		{"xi", 958},
		{"clubs", 9827},
		{"Xi", 926},
		{"aring", 229},
		{"", 0}, {"", 0}, {"", 0},
		{"copy", 169},
		{"uArr", 8657},
		{"ni", 8715},
		{"rarr", 8594},
		{"le", 8804},
		{"ge", 8805},
		{"zwnj", 8204},
		{"", 0},
		{"apos", 39},
		{"macr", 175},
		{"lang", 9001},
		{"gamma", 947},
		{"Delta", 916},
		{"", 0},
		{"uml", 168},
		{"alefsym", 8501},
		{"delta", 948},
		{"", 0},
		{"bdquo", 8222},
		{"lambda", 955},
		{"equiv", 8801},
		{"", 0},
		{"Oslash", 216},
		{"", 0},
		{"hellip", 8230},
		{"", 0},
		{"rArr", 8658},
		{"Atilde", 195},
		{"larr", 8592},
		{"spades", 9824},
		{"Igrave", 204},
		{"Pi", 928},
		{"yacute", 253},
		{"", 0},
		{"diams", 9830},
		{"sbquo", 8218},
		{"fnof", 402},
		{"Ograve", 210},
		{"plusmn", 177},
		{"", 0},
		{"rceil", 8969},
		{"Aacute", 193},
		{"ouml", 246},
		{"auml", 228},
		{"euml", 235},
		{"iuml", 239},
		{"", 0},
		{"Acirc", 194},
		{"", 0},
		{"rdquo", 8221},
		{"", 0},
		{"lArr", 8656},
		{"rsquo", 8217},
		{"Yuml", 376},
		{"", 0},
		{"quot", 34},
		{"Uuml", 220},
		{"bull", 8226},
		{"", 0}, {"", 0}, {"", 0},
		{"real", 8476},
		{"", 0}, {"", 0}, {"", 0},
		{"lceil", 8968},
		{"permil", 8240},
		{"upsih", 978},
		{"sum", 8721},
		{"", 0}, {"", 0},
		{"divide", 247},
		{"raquo", 187},
		{"uuml", 252},
		{"ldquo", 8220},
		{"Alpha", 913},
		{"szlig", 223},
		{"lsquo", 8216},
		{"", 0},
		{"Sigma", 931},
		{"tilde", 732},
		{"", 0},
		{"THORN", 222},
		{"", 0}, {"", 0}, {"", 0},
		{"Euml", 203},
		{"rfloor", 8971},
		{"", 0},
		{"lrm", 8206},
		{"", 0},
		{"sigma", 963},
		{"iexcl", 161},
		{"", 0}, {"", 0},
		{"deg", 176},
		{"middot", 183},
		{"laquo", 171},
		{"", 0},
		{"circ", 710},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"frasl", 8260},
		{"epsilon", 949},
		{"oplus", 8853},
		{"yen", 165},
		{"micro", 181},
		{"piv", 982},
		{"", 0}, {"", 0},
		{"lfloor", 8970},
		{"", 0},
		{"Agrave", 192},
		{"", 0}, {"", 0},
		{"Upsilon", 933},
		{"", 0}, {"", 0},
		{"times", 215},
		{"", 0},
		{"cedil", 184},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"minus", 8722},
		{"Iuml", 207},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"upsilon", 965},
		{"Ouml", 214},
		{"", 0}, {"", 0},
		{"rlm", 8207},
		{"", 0}, {"", 0}, {"", 0},
		{"reg", 174},
		{"", 0},
		{"forall", 8704},
		{"", 0}, {"", 0},
		{"Epsilon", 917},
		{"empty", 8709},
		{"OElig", 338},
		{"", 0},
		{"shy", 173},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Aring", 197},
		{"", 0}, {"", 0}, {"", 0},
		{"oelig", 339},
		{"aelig", 230},
		{"", 0},
		{"zwj", 8205},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"sim", 8764},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"yuml", 255},
		{"sigmaf", 962},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"Auml", 196},
		{"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"", 0}, {"", 0}, {"", 0}, {"", 0},
		{"AElig", 198}
	};

	const int MIN_WORD_LENGTH		= 2;
	const int MAX_WORD_LENGTH		= 8;
	const int MAX_HASH_VALUE		= 420;

	if ( len<=MAX_WORD_LENGTH && len>=MIN_WORD_LENGTH )
	{
		int key = HtmlEntityHash ( str, len );
		if ( key<=MAX_HASH_VALUE && key>=0 )
			if ( len==lengthtable[key] )
		{
			const char * s = wordlist[key].m_sName;
			if ( *str==*s && !memcmp ( str+1, s+1, len-1 ) )
				return wordlist[key].m_iCode;
		}
	}
	return 0;
}


static const BYTE * SkipPI ( const BYTE * s )
{
	assert ( s[0]=='<' && s[1]=='?' );
	s += 2;

	const BYTE * pStart = s;
	const BYTE * pMax = s + 256;
	for ( ; s<pMax && *s; s++ )
	{
		// for now, let's just bail whenever we see ">", like Firefox does!!!
		// that covers the valid case, ie. the closing "?>", just as well
		if ( s[0]=='>' )
			return s+1;
	}

	if ( !*s )
		return s;

	// no closing end marker ever found; just skip non-whitespace after "<?" then
	s = pStart;
	while ( s<pMax && *s && !sphIsSpace(*s) )
		s++;
	return s;
}


void CSphHTMLStripper::Strip ( BYTE * sData ) const
{
	if ( !sData )
		return;

	const BYTE * s = sData;
	BYTE * d = sData;
	while (true)
	{
		/////////////////////////////////////
		// scan until eof, or tag, or entity
		/////////////////////////////////////

		while ( *s && *s!='<' && *s!='&' )
		{
			if ( *s>=0x20 )
				*d++ = *s;
			else
				*d++ = ' ';
			s++;
		}
		if ( !*s )
			break;

		/////////////////
		// handle entity
		/////////////////

		if ( *s=='&' )
		{
			if ( s[1]=='#' )
			{
				// handle "&#number;" and "&#xnumber;" forms
				DWORD uCode = 0;
				s += 2;

				bool bHex = ( *s && ( *s=='x' || *s=='X') );
				if ( !bHex )
				{
					while ( isdigit(*s) )
						uCode = uCode*10 + (*s++) - '0';
				} else
				{
					s++;
					while ( *s )
					{
						if ( isdigit(*s) )
							uCode = uCode*16 + (*s++) - '0';
						else if ( *s>=0x41 && *s<=0x46 )
							uCode = uCode*16 + (*s++) - 'A' + 0xA;
						else if ( *s>=0x61 && *s<=0x66 )
							uCode = uCode*16 + (*s++) - 'a' + 0xA;
						else
							break;
					}
				}

				uCode = uCode % 0x110000; // there is no unicode code-points bigger than this value

				if ( uCode<=0x1f || *s!=';' ) // 0-31 are reserved codes
					continue;

				d += sphUTF8Encode ( d, (int)uCode );
				s++;

			} else
			{
				// skip until ';' or max length
				if ( ( s[1]>='a' && s[1]<='z' ) || ( s[1]>='A' && s[1]<='Z' ) )
				{
					const int MAX_ENTITY_LEN = 8;
					const BYTE * sStart = s+1;
					while ( *s && *s!=';' && s-sStart<=MAX_ENTITY_LEN )
						s++;

					if ( *s==';' )
					{
						int iCode = HtmlEntityLookup ( sStart, (int)(s-sStart) );
						if ( iCode>0 )
						{
							// this is a known entity; encode it
							d += sphUTF8Encode ( d, iCode );
							s++;
							continue;
						}
					}

					// rollback
					s = sStart-1;
				}

				// if we're here, it's not an entity; pass the leading ampersand and rescan
				*d++ = *s++;
			}
			continue;
		}

		//////////////
		// handle tag
		//////////////

		assert ( *s=='<' );
		if ( GetCharIndex(s[1])<0 )
		{
			if ( s[1]=='/' )
			{
				// check if it's valid closing tag
				if ( GetCharIndex(s[2])<0 )
				{
					*d++ = *s++;
					continue;
				}

			} else if ( s[1]=='!' )
			{
				if ( s[2]=='-' && s[3]=='-' )
				{
					// it's valid comment; scan until comment end
					s += 4; // skip opening '<!--'
					while ( *s )
					{
						if ( s[0]=='-' && s[1]=='-' && s[2]=='>' )
							break;
						s++;
					}
					if ( !*s )
						break;
					s += 3; // skip closing '-->'
					continue;

				} else if ( isalpha(s[2]) )
				{
					// it's <!doctype> style PI; scan until PI end
					s += 2;
					while ( *s && *s!='>' )
					{
						if ( *s=='\'' || *s=='"' )
						{
							s = SkipQuoted ( s );
							while ( isspace(*s) ) s++;
						} else
						{
							s++;
						}
					}
					if ( *s=='>' )
						s++;
					continue;

				} else
				{
					// it's something malformed; just ignore
					*d++ = *s++;
					continue;
				}

			} else if ( s[1]=='?' )
			{
				// scan until PI end
				s = SkipPI ( s );
				continue;

			} else
			{
				// simply malformed
				*d++ = *s++;
				continue;
			}
		}
		s++; // skip '<'

		//////////////////////////////////////
		// lookup this tag in known tags list
		//////////////////////////////////////

		const html_stripper::StripperTag_t * pTag = nullptr;
		int iZoneNameLen = 0;
		const BYTE * sZoneName = nullptr;
		s = FindTag ( s, &pTag, &sZoneName, &iZoneNameLen );

		/////////////////////////////////////
		// process tag contents
		// index attributes if needed
		// gracefully handle malformed stuff
		/////////////////////////////////////

#define LOC_SKIP_SPACES() { while ( sphIsSpace(*s) ) s++; if ( !*s || *s=='>' ) break; }

		bool bIndexAttrs = ( pTag && pTag->m_bIndexAttrs );
		while ( *s && *s!='>' )
		{
			LOC_SKIP_SPACES();
			if ( sphIsTagStart(*s) )
			{
				// skip attribute name while it's valid
				const BYTE * sAttr = s;
				while ( sphIsTag(*s) )
					s++;

				// blanks or a value after a valid attribute name?
				if ( sphIsSpace(*s) || *s=='=' )
				{
					const int iAttrLen = (int)( s - sAttr );
					LOC_SKIP_SPACES();

					// a valid name but w/o a value; keep scanning
					if ( *s!='=' )
						continue;

					// got value!
					s++;
					LOC_SKIP_SPACES();

					// check attribute name
					// OPTIMIZE! remove linear search
					int iAttr = -1;
					if ( bIndexAttrs )
					{
						for ( iAttr=0; iAttr<pTag->m_dAttrs.GetLength(); iAttr++ )
						{
							auto iLen = (int) strlen ( pTag->m_dAttrs[iAttr].cstr() );
							if ( iLen==iAttrLen && !strncasecmp ( pTag->m_dAttrs[iAttr].cstr(), (const char*)sAttr, iLen ) )
								break;
						}
						if ( iAttr==pTag->m_dAttrs.GetLength() )
							iAttr = -1;
					}

					// process the value
					const BYTE * sVal = s;
					if ( *s=='\'' || *s=='"' )
					{
						// skip quoted value until a matching quote
						s = SkipQuoted ( s );
					} else
					{
						// skip unquoted value until tag end or whitespace
						while ( *s && *s!='>' && !sphIsSpace(*s) )
							s++;
					}

					// if this one is to be indexed, copy it
					if ( iAttr>=0 )
					{
						const BYTE * sMax = s;
						if ( *sVal=='\'' || *sVal=='"' )
						{
							if ( sMax[-1]==sVal[0] )
								sMax--;
							sVal++;
						}
						while ( sVal<sMax )
							*d++ = *sVal++;
						*d++ = ' ';
					}

					// handled the value; keep scanning
					continue;
				}

				// nope, got an invalid character in the sequence (or maybe eof)
				// fall through to an invalid name handler
			}

			// keep skipping until tag end or whitespace
			while ( *s && *s!='>' && !sphIsSpace(*s) )
				s++;
		}

#undef LOC_SKIP_SPACES

		// skip closing angle bracket, if any
		if ( *s )
			s++;

		// unknown tag is done; others might require a bit more work
		if ( !pTag )
		{
			*d++ = ' '; // unknown tags are *not* inline by default
			continue;
		}

		// handle zones
		if ( pTag->m_bZone )
		{
			// should be at tag's end
			assert ( s[0]=='\0' || s[-1]=='>' );

			// emit secret codes
			*d++ = MAGIC_CODE_ZONE;
			for ( int i=0; i<iZoneNameLen; i++ )
				*d++ = (BYTE) tolower ( sZoneName[i] );
			if ( *d )
				*d++ = MAGIC_CODE_ZONE;

			if ( !*s )
				break;
			continue;
		}

		// handle paragraph boundaries
		if ( pTag->m_bPara )
		{
			*d++ = MAGIC_CODE_PARAGRAPH;
			continue;
		}

		// in all cases, the tag must be fully processed at this point
		// not a remove-tag? we're done
		if ( !pTag->m_bRemove )
		{
			if ( !pTag->m_bInline )
				*d++ = ' ';
			continue;
		}

		// sudden eof? bail out
		if ( !*s )
			break;

		// must be a proper remove-tag end, then
		assert ( pTag->m_bRemove && s[-1]=='>' );

		// short-form? we're done
		if ( s[-2]=='/' )
			continue;

		// skip everything until the closing tag
		// FIXME! should we handle insane cases with quoted closing tag within tag?
		while (true)
		{
			while ( *s && ( s[0]!='<' || s[1]!='/' ) ) s++;
			if ( !*s ) break;

			s += 2; // skip </
			if ( strncasecmp ( pTag->m_sTag.cstr(), (const char*)s, pTag->m_iTagLen )!=0 ) continue;
			if ( !sphIsTag ( s[pTag->m_iTagLen] ) )
			{
				s += pTag->m_iTagLen; // skip tag
				if ( *s=='>' ) s++;
				break;
			}
		}

		if ( !pTag->m_bInline ) *d++ = ' ';
	}
	*d++ = '\0';

	// space, paragraph sequences elimination pass
	s = sData;
	d = sData;
	bool bSpaceOut = false;
	bool bParaOut = false;
	bool bZoneOut = false;
	while ( const char c = *s++ )
	{
		assert ( d<=s-1 );

		// handle different character classes
		if ( sphIsSpace(c) )
		{
			// handle whitespace, skip dupes
			if ( !bSpaceOut )
				*d++ = ' ';

			bSpaceOut = true;
			continue;

		} else if ( c==MAGIC_CODE_PARAGRAPH )
		{
			// handle paragraph marker, skip dupes
			if ( !bParaOut && !bZoneOut )
			{
				*d++ = c;
				bParaOut = true;
			}

			bSpaceOut = true;
			continue;

		} else if ( c==MAGIC_CODE_ZONE )
		{
			// zone marker
			// rewind preceding paragraph, if any, it is redundant
			if ( bParaOut )
			{
				assert ( d>sData && d[-1]==MAGIC_CODE_PARAGRAPH );
				d--;
			}

			// copy \4zoneid\4
			*d++ = c;
			while ( *s && *s!=MAGIC_CODE_ZONE )
				*d++ = *s++;

			if ( *s )
				*d++ = *s++;

			// update state
			// no spaces paragraphs allowed
			bSpaceOut = bZoneOut = true;
			bParaOut = false;
			continue;

		} else
		{
			*d++ = c;
			bSpaceOut = bParaOut = bZoneOut = false;
		}
	}
	*d++ = '\0';
}

const BYTE * CSphHTMLStripper::FindTag ( const BYTE * sSrc, const html_stripper::StripperTag_t ** ppTag,
	const BYTE ** ppZoneName, int * pZoneNameLen ) const
{
	assert ( sSrc && ppTag && ppZoneName && pZoneNameLen );
	assert ( sSrc[0]!='/' || sSrc[1]!='\0' );

	const BYTE * sTagName = ( sSrc[0]=='/' ) ? sSrc+1 : sSrc;

	*ppZoneName = sSrc;
	*pZoneNameLen = 0;

	int iIdx = GetCharIndex ( sTagName[0] );
	assert ( iIdx>=0 && iIdx<MAX_CHAR_INDEX );

	if ( iIdx>=0 && m_dEnd[iIdx]>=0 )
	{
		int iStart = m_dStart[iIdx];
		int iEnd = m_dEnd[iIdx];

		for ( int i=iStart; i<=iEnd; i++ )
		{
			int iLen = m_dTags[i].m_iTagLen;
			int iCmp = strncasecmp ( m_dTags[i].m_sTag.cstr(), (const char*)sTagName, iLen );

			// the tags are sorted; so if current candidate is already greater, rest can be skipped
			if ( iCmp>0 )
				break;

			// do we have a match?
			if ( iCmp==0 )
			{
				// got exact match?
				if ( !sphIsTag ( sTagName[iLen] ) )
				{
					*ppTag = m_dTags.Begin() + i;
					sSrc = sTagName + iLen; // skip tag name
					if ( m_dTags[i].m_bZone )
						*pZoneNameLen = int ( sSrc - *ppZoneName );
					break;
				}

				// got wildcard match?
				if ( m_dTags[i].m_bZonePrefix )
				{
					*ppTag = m_dTags.Begin() + i;
					sSrc = sTagName + iLen;
					while ( sphIsTag(*sSrc) )
						sSrc++;
					*pZoneNameLen = int ( sSrc - *ppZoneName );
					break;
				}
			}
		}
	}

	return sSrc;
}

bool CSphHTMLStripper::IsValidTagStart ( int iCh ) const
{
	int i = GetCharIndex ( iCh );
	return ( i>=0 && i<MAX_CHAR_INDEX );
}
