//
// $Id$
//

//
// Copyright (c) 2001-2010, Andrew Aksyonoff
// Copyright (c) 2008-2010, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxutils.h"

#include <locale.h>

const int MAX_STR_LENGTH = 512;

//////////////////////////////////////////////////////////////////////////

BYTE GetWordchar ( const char * & szSet )
{
	if ( *szSet=='\\' )
	{
		if ( !szSet[1] || !szSet[2] || !szSet[3] )
			return 0;

		char szBuf[3];
		memcpy ( szBuf, szSet+2, 2 );
		szBuf[2] = 0;

		char * szStop = NULL;
		int iRes = strtol ( szBuf, &szStop, 16 );
		if ( szStop!=szBuf+2 || iRes<0 || iRes>255 )
			return 0;

		szSet += 4;
		return (BYTE) iRes;
	}

	return *szSet++;
}


bool IsInSet ( BYTE uLetter, const char * szSet )
{
	if ( !szSet )
		return false;

	bool bInvert = ( *szSet=='^' );
	if ( bInvert )
		++szSet;

	const char * szSep = strchr ( szSet, '-' );
	bool bRange = ( szSep!=NULL );

	if ( bRange )
	{
		BYTE uRange1 = GetWordchar ( szSet );
		szSep++;
		BYTE uRange2 = GetWordchar ( szSep );

		if ( uLetter>=Min ( uRange1, uRange2 ) && uLetter<=Max ( uRange1, uRange2 ) )
			return !bInvert;

	} else
	{
		BYTE uChar = 0;
		while ( ( uChar = GetWordchar ( szSet ) )!=0 )
			if ( uChar==uLetter )
				break;

		bool bEnd = !uChar;

		if ( bInvert && bEnd )
			return true;

		if ( !bInvert && !bEnd )
			return true;
	}

	return false;
}


bool GetSetMinMax ( const char * szSet, BYTE & uMin, BYTE & uMax )
{
	if ( !szSet || !*szSet )
		return false;

	uMin = GetWordchar ( szSet );
	uMax = uMin;

	BYTE uChar;
	while ( ( uChar = GetWordchar ( szSet ) )!=0 )
		if ( uChar!='-' )
		{
			uMin = Min ( uMin, uChar );
			uMax = Max ( uMax, uChar );
		}

	if ( !uMin || !uMax )
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////

class CISpellDict
{
public:
	struct CISpellDictWord
	{
		CSphString	m_sWord;
		CSphString	m_sFlags;
	};

	bool			Load ( const char * szFilename );
	void			IterateStart ();
	const			CISpellDictWord * IterateNext ();

private:
	CSphVector < CISpellDictWord > m_dEntries;
	int				m_iIterator;
};


bool CISpellDict::Load ( const char * szFilename )
{
	if ( !szFilename )
		return false;

	m_dEntries.Reset ();
	m_dEntries.Reserve ( 131072 );

	FILE * pFile = fopen ( szFilename, "rt" );
	if ( !pFile )
		return false;

	char szWordBuffer [MAX_STR_LENGTH];

	while ( !feof ( pFile ) )
	{
		char * szResult = fgets ( szWordBuffer, MAX_STR_LENGTH, pFile );
		if ( !szResult )
			break;

		int iPos = strlen ( szWordBuffer ) - 1;
		while ( iPos>=0 && isspace ( (unsigned char)szWordBuffer[iPos] ) )
			szWordBuffer [iPos--] = '\0';

		CISpellDictWord Word;

		char * szPosition = strchr ( szWordBuffer, '/' );
		if ( !szPosition )
		{
			szPosition = szWordBuffer;
			while ( *szPosition && !isspace ( (unsigned char)*szPosition ) )
				++szPosition;

			*szPosition = '\0';
			Word.m_sWord = szWordBuffer;

		} else
		{
			*szPosition = '\0';
			Word.m_sWord = szWordBuffer;
			++szPosition;
			char * szFlags = szPosition;
			while ( *szPosition && !isspace ( (unsigned char)*szPosition ) )
				++szPosition;

			*szPosition = '\0';
			Word.m_sFlags = szFlags;
		}

		m_dEntries.Add ( Word );
	}

	fclose ( pFile );

	return true;
}


void CISpellDict::IterateStart ()
{
	m_iIterator = 0;
}


const CISpellDict::CISpellDictWord * CISpellDict::IterateNext ()
{
	if ( m_iIterator>=m_dEntries.GetLength() )
		return NULL;

	return &m_dEntries [m_iIterator++];
}

//////////////////////////////////////////////////////////////////////////

enum RuleType_e
{
	RULE_NONE,
	RULE_PREFIXES,
	RULE_SUFFIXES
};


class CISpellAffixRule
{
public:
				CISpellAffixRule () {}
				CISpellAffixRule ( RuleType_e eRule, char cFlag, bool bCrossProduct, char * szCondition, char * szStrip, char * szAppend );

	bool		Apply ( CSphString & sWord );
	char		Flag () const;
	bool		IsCrossProduct () const;
	bool		IsPrefix () const;

private:
	RuleType_e	m_eRule;
	char		m_cFlag;
	bool		m_bCrossProduct;
	CSphString	m_sCondition;
	CSphString	m_sStrip;
	CSphString	m_sAppend;

	int			m_iWordLen;
	int			m_iCondLen;
	int			m_iStripLen;
	int			m_iAppendLen;

	bool		CheckSuffix ( const CSphString & sWord ) const;
	bool		CheckPrefix ( const CSphString & sWord ) const;
	bool		StripAppendSuffix ( CSphString & sWord ) const;
	bool		StripAppendPrefix ( CSphString & sWord ) const;
};


CISpellAffixRule::CISpellAffixRule ( RuleType_e eRule, char cFlag, bool bCrossProduct, char * szCondition, char * szStrip, char * szAppend )
	: m_eRule			( eRule )
	, m_cFlag			( cFlag )
	, m_bCrossProduct	( bCrossProduct )
	, m_sCondition		( szCondition )
	, m_sStrip			( szStrip )
	, m_sAppend			( szAppend )
	, m_iWordLen		( 0 )
{
	m_iCondLen	= szCondition ? strlen ( szCondition ) : 0;
	m_iStripLen = szStrip ? strlen ( szStrip ) : 0;
	m_iAppendLen = szAppend ? strlen ( szAppend ) : 0;
}


bool CISpellAffixRule::Apply ( CSphString & sWord )
{
	if ( m_sCondition.IsEmpty () )
		return true;

	if ( sWord.IsEmpty () )
		return false;

	m_iWordLen = strlen ( sWord.cstr () );

	bool bDotCond = ( m_sCondition=="." );
	if ( m_eRule==RULE_SUFFIXES )
	{
		if ( !bDotCond && !CheckSuffix ( sWord ) )
			return false;

		if ( !StripAppendSuffix ( sWord ) )
			return false;
	} else
	{
		if ( !bDotCond && !CheckPrefix ( sWord ) )
			return false;

		if ( !StripAppendPrefix ( sWord ) )
			return false;
	}
	return true;
}


bool CISpellAffixRule::CheckSuffix ( const CSphString & sWord ) const
{
	int iCondI = m_iCondLen-1;
	for ( int i=m_iWordLen-1; iCondI>=0 && i>=0; --i )
	{
		if ( m_sCondition.cstr()[iCondI]!=']' )
		{
			if ( m_sCondition.cstr()[iCondI]!=sWord.cstr()[i] )
				return false;

			--iCondI;

		} else
		{
			int iRangeStart = -1;

			for ( int j=iCondI; j>=0 && iRangeStart==-1; --j )
				if ( m_sCondition.cstr()[j]=='[' )
					iRangeStart = j;

			if ( iRangeStart==-1 )
				return false;
			else
			{
				if ( !IsInSet ( sWord.cstr () [i], m_sCondition.SubString ( iRangeStart + 1, iCondI - iRangeStart - 1 ).cstr () ) )
					return false;

				iCondI = iRangeStart - 1;
			}
		}
	}

	return true;
}


bool CISpellAffixRule::StripAppendSuffix ( CSphString & sWord ) const
{
	static char szTmp [ MAX_STR_LENGTH];

	if ( !m_sStrip.IsEmpty () )
	{
		if ( m_iWordLen < m_iStripLen )
			return false;

		if ( strncmp ( sWord.cstr () + m_iWordLen - m_iStripLen, m_sStrip.cstr (), m_iStripLen ) )
			return false;
	}

	strncpy ( szTmp, sWord.cstr (), m_iWordLen - m_iStripLen );
	szTmp [m_iWordLen - m_iStripLen] = '\0';

	if ( !m_sAppend.IsEmpty () )
		strcat ( szTmp, m_sAppend.cstr () ); // NOLINT

	sWord = szTmp;

	return true;
}


bool CISpellAffixRule::CheckPrefix ( const CSphString & sWord ) const
{
	int iCondI = 0;
	for ( int i = 0; iCondI < m_iCondLen && i < m_iWordLen; ++i )
	{
		if ( m_sCondition.cstr()[iCondI]!='[' )
		{
			if ( m_sCondition.cstr()[iCondI]!=sWord.cstr()[i] )
				return false;

			++iCondI;

		} else
		{
			int iRangeEnd = -1;

			for ( int j=iCondI; j<m_iCondLen && iRangeEnd==-1; ++j )
				if ( m_sCondition.cstr()[j]==']' )
					iRangeEnd = j;

			if ( iRangeEnd==-1 )
				return false;
			else
			{
				if ( !IsInSet ( sWord.cstr () [i], m_sCondition.SubString ( iCondI + 1, iRangeEnd - iCondI - 1 ).cstr () ) )
					return false;
			}
		}
	}

	return true;
}


bool CISpellAffixRule::StripAppendPrefix ( CSphString & sWord ) const
{
	static char szTmp [MAX_STR_LENGTH];

	if ( !m_sStrip.IsEmpty () )
	{
		const char * Pos = strstr ( sWord.cstr (), m_sStrip.cstr () );
		if ( Pos!=sWord.cstr() )
			return false;
	}

	if ( !m_sAppend.IsEmpty () )
		strcpy ( szTmp, m_sAppend.cstr() ); // NOLINT

	strncpy ( szTmp + m_iAppendLen, sWord.cstr () + m_iStripLen, m_iWordLen - m_iStripLen );
	szTmp [m_iWordLen - m_iStripLen + m_iAppendLen] = '\0';

	sWord = szTmp;

	return true;
}


char CISpellAffixRule::Flag () const
{
	return m_cFlag;
}

bool CISpellAffixRule::IsCrossProduct () const
{
	return m_bCrossProduct;
}

bool CISpellAffixRule::IsPrefix () const
{
	return m_eRule==RULE_PREFIXES;
}

//////////////////////////////////////////////////////////////////////////

enum AffixFormat_e
{
	AFFIX_FORMAT_ISPELL		= 0,
	AFFIX_FORMAT_MYSPELL	= 1,
	AFFIX_FORMAT_UNKNOWN
};

const char * AffixFormatName[] =
{
	"ISpell",
	"MySpell"
};

class CISpellAffix
{
public:
						CISpellAffix ( const char * szLocale, const char * szCharsetFile );

	bool				Load ( const char * szFilename );
	CISpellAffixRule *	GetRule ( int iRule );
	int					GetNumRules () const;
	bool				CheckCrosses () const;

private:
	CSphVector < CISpellAffixRule > m_dRules;

	char		m_dCharset [256];
	bool		m_bFirstCaseConv;
	CSphString	m_sLocale;
	CSphString	m_sCharsetFile;
	bool		m_bCheckCrosses;
	CSphLowercaser m_LowerCaser;
	bool		m_bUseLowerCaser;
	bool		m_bUseDictConversion;

	bool		AddToCharset ( char * szRangeL, char * szRangeU );
	void		AddCharPair ( BYTE uCharL, BYTE uCharU );
	void		Strip ( char * szText );
	char		ToLowerCase ( char cChar );
	void		LoadLocale ();

	AffixFormat_e		DetectFormat ( FILE * );
	bool				LoadISpell ( FILE * );
	bool				LoadMySpell ( FILE * );
};


CISpellAffix::CISpellAffix ( const char * szLocale, const char * szCharsetFile )
	: m_bFirstCaseConv	( true )
	, m_sLocale			( szLocale )
	, m_sCharsetFile	( szCharsetFile )
	, m_bCheckCrosses	( false )
	, m_bUseLowerCaser	( false )
	, m_bUseDictConversion	( false )
{
}


AffixFormat_e CISpellAffix::DetectFormat ( FILE * pFile )
{
	char sBuffer [MAX_STR_LENGTH];

	while ( !feof ( pFile ) )
	{
		char * sLine = fgets ( sBuffer, MAX_STR_LENGTH, pFile );
		if ( !sLine )
			break;

		if ( !strncmp ( sLine, "SFX", 3 ) ) return AFFIX_FORMAT_MYSPELL;
		if ( !strncmp ( sLine, "PFX", 3 ) ) return AFFIX_FORMAT_MYSPELL;
		if ( !strncmp ( sLine, "REP", 3 ) ) return AFFIX_FORMAT_MYSPELL;

		if ( !strncasecmp ( sLine, "prefixes", 8 ) )	return AFFIX_FORMAT_ISPELL;
		if ( !strncasecmp ( sLine, "suffixes", 8 ) )	return AFFIX_FORMAT_ISPELL;
		if ( !strncasecmp ( sLine, "flag", 4 ) )		return AFFIX_FORMAT_ISPELL;
	}

	return AFFIX_FORMAT_UNKNOWN;
}


bool CISpellAffix::Load ( const char * szFilename )
{
	if ( !szFilename )
		return false;

	m_dRules.Reset ();
	memset ( m_dCharset, 0, sizeof ( m_dCharset ) );
	m_bFirstCaseConv = true;
	m_bUseLowerCaser = false;
	m_bUseDictConversion = false;
	m_LowerCaser.Reset ();

	FILE * pFile = fopen ( szFilename, "rt" );
	if ( !pFile )
		return false;

	bool bResult = false;
	AffixFormat_e eFormat = DetectFormat ( pFile );
	if ( eFormat==AFFIX_FORMAT_UNKNOWN )
		printf ( "Failed to detect affix file format\n" );
	else
	{
		fseek ( pFile, SEEK_SET, 0 );
		printf ( "Using %s affix file format\n", AffixFormatName[eFormat] );
		switch ( eFormat )
		{
			case AFFIX_FORMAT_MYSPELL:	bResult = LoadMySpell ( pFile ); break;
			case AFFIX_FORMAT_ISPELL:	bResult = LoadISpell ( pFile ); break;
			case AFFIX_FORMAT_UNKNOWN:	break;
		}
	}
	fclose ( pFile );

	bool bHaveCrossPrefix = false;
	for ( int i = 0; i < m_dRules.GetLength () && !bHaveCrossPrefix; i++ )
		if ( m_dRules[i].IsPrefix() && m_dRules[i].IsCrossProduct() )
			bHaveCrossPrefix = true;

	bool bHaveCrossSuffix = false;
	for ( int i = 0; i < m_dRules.GetLength () && !bHaveCrossSuffix; i++ )
		if ( !m_dRules[i].IsPrefix() && m_dRules[i].IsCrossProduct() )
			bHaveCrossSuffix = true;

	m_bCheckCrosses = bHaveCrossPrefix && bHaveCrossSuffix;

	return bResult;
}


bool CISpellAffix::LoadISpell ( FILE * pFile )
{
	char szBuffer [ MAX_STR_LENGTH ];
	char szCondition [ MAX_STR_LENGTH ];
	char szStrip [ MAX_STR_LENGTH ];
	char szAppend [ MAX_STR_LENGTH ];

	RuleType_e eRule = RULE_NONE;
	char cFlag = '\0';
	bool bCrossProduct = false;
	int iLine = 0;

	// TODO: parse all .aff character replacement commands
	while ( !feof ( pFile ) )
	{
		char * szResult = fgets ( szBuffer, MAX_STR_LENGTH, pFile );
		if ( !szResult )
			break;
		iLine++;

		if ( !strncasecmp ( szBuffer, "prefixes", 8 ) )
		{
			eRule = RULE_PREFIXES;
			continue;
		}

		if ( !strncasecmp ( szBuffer, "suffixes", 8 ) )
		{
			eRule = RULE_SUFFIXES;
			continue;
		}

		if ( !strncasecmp ( szBuffer, "wordchars", 9 ) )
		{
			char * szStart = szBuffer + 9;
			while ( *szStart && isspace ( (unsigned char) *szStart ) )
				++szStart;

			char * szRangeL = szStart;
			while ( *szStart && !isspace ( (unsigned char) *szStart ) )
				++szStart;

			if ( !*szStart )
			{
				printf ( "WARNING: Line %d: invalid 'wordchars' statement\n", iLine );
				continue;
			}

			*szStart = '\0';
			++szStart;

			while ( *szStart && isspace ( (unsigned char) *szStart ) )
				++szStart;

			char * szRangeU = szStart;

			while ( *szStart && !isspace ( (unsigned char) *szStart ) )
				++szStart;

			*szStart = '\0';

			if ( !AddToCharset ( szRangeL, szRangeU ) )
				printf ( "WARNING: Line %d: cannot add to charset: '%s' '%s'\n", iLine, szRangeL, szRangeU );

			continue;
		}

		if ( !strncasecmp ( szBuffer, "flag", 4 ) )
		{
			if ( eRule==RULE_NONE )
			{
				printf ( "WARNING: Line %d: 'flag' appears before preffixes or suffixes\n", iLine );
				continue;
			}

			char * szStart = szBuffer + 4;
			while ( *szStart && isspace ( (unsigned char) *szStart ) )
				++szStart;

			bCrossProduct = ( *szStart=='*' );

			cFlag = bCrossProduct ? *(szStart + 1) : *(szStart);
			continue;
		}

		if ( eRule==RULE_NONE )
			continue;

		char * szComment = strchr ( szBuffer, '#' );
		if ( szComment )
			*szComment = '\0';

		if ( !* szBuffer )
			continue;

		szCondition[0] = '\0';
		szStrip[0] = '\0';
		szAppend[0] = '\0';

		int nFields = sscanf ( szBuffer, "%[^>\n]>%[^,\n],%[^\n]", szCondition, szStrip, szAppend ); // NOLINT

		Strip ( szCondition );
		Strip ( szStrip );
		Strip ( szAppend );

		switch ( nFields )
		{
		case 2: // no optional strip-string
			strcpy ( szAppend, szStrip ); // NOLINT
			szStrip[0] = '\0';
			break;
		case 3:	// all read
			break;
		default: // invalid repl
			continue;
		}

		CISpellAffixRule Rule ( eRule, cFlag, bCrossProduct, szCondition, szStrip, szAppend );
		m_dRules.Add ( Rule );
	}

	return true;
}


bool CISpellAffix::LoadMySpell ( FILE * pFile )
{
	char sBuffer	[MAX_STR_LENGTH];
	char sCondition	[MAX_STR_LENGTH];
	char sRemove	[MAX_STR_LENGTH];
	char sAppend	[MAX_STR_LENGTH];

	RuleType_e eRule = RULE_NONE;
	BYTE cFlag = 0;
	BYTE cCombine = 0;
	int iCount = 0, iLine = 0;
	const char * sMode = 0;

	while ( !feof ( pFile ) )
	{
		char * sLine = fgets ( sBuffer, MAX_STR_LENGTH, pFile );
		if ( !sLine )
			break;
		++iLine;

		// prefix and suffix rules
		RuleType_e eNewRule = RULE_NONE;
		if ( !strncmp ( sLine, "PFX", 3 ) )
		{
			eNewRule = RULE_PREFIXES;
			sMode = "prefix";

		} else if ( !strncmp ( sLine, "SFX", 3 ) )
		{
			eNewRule = RULE_SUFFIXES;
			sMode = "suffix";
		}

		if ( eNewRule!=RULE_NONE )
		{
			sLine += 3;
			while ( *sLine && isspace ( (unsigned char) *sLine ) )
				++sLine;

			if ( eNewRule!=eRule ) // new rule header
			{
				if ( iCount )
					printf ( "WARNING: Line %d: Premature end of entries.\n", iLine );

				if ( sscanf ( sLine, "%c %c %d", &cFlag, &cCombine, &iCount )!=3 ) // NOLINT
					printf ( "WARNING; Line %d: Malformed %s header\n", iLine, sMode );

				eRule = eNewRule;

			} else // current rule continued
			{
				*sRemove = *sAppend = 0;
				char cNewFlag;
				if ( sscanf ( sLine, "%c %s %s %s", &cNewFlag, sRemove, sAppend, sCondition )==4 ) // NOLINT
				{
					if ( cNewFlag!=cFlag )
						printf ( "WARNING: Line %d: Flag character mismatch\n", iLine );

					if ( *sRemove=='0' && *(sRemove + 1)==0 ) *sRemove = 0;
					if ( *sAppend=='0' && *(sAppend + 1)==0 ) *sAppend = 0;

					CISpellAffixRule Rule ( eRule, cFlag, cCombine=='Y', sCondition, sRemove, sAppend );
					m_dRules.Add ( Rule );

				} else
					printf ( "WARNING: Line %d: Malformed %s rule\n", iLine, sMode );

				if ( !--iCount ) eRule = RULE_NONE;
			}
			continue;
		}
	}
	return true;
}


CISpellAffixRule * CISpellAffix::GetRule ( int iRule )
{
	return &m_dRules [iRule];
}


int CISpellAffix::GetNumRules () const
{
	return m_dRules.GetLength ();
}


bool CISpellAffix::CheckCrosses () const
{
	return m_bCheckCrosses;
}


bool CISpellAffix::AddToCharset ( char * szRangeL, char * szRangeU )
{
	if ( !szRangeL || !szRangeU )
		return false;

	int iLengthL = strlen ( szRangeL );
	int iLengthU = strlen ( szRangeU );

	bool bSetL = ( iLengthL>0 && szRangeL[0]=='[' && szRangeL[iLengthL-1]==']' );
	bool bSetR = ( iLengthU>0 && szRangeU[0]=='[' && szRangeU[iLengthU-1]==']' );

	if ( bSetL!=bSetR )
		return false;

	if ( bSetL )
	{
		szRangeL [iLengthL - 1] = '\0';
		szRangeL = szRangeL + 1;

		szRangeU [iLengthU - 1] = '\0';
		szRangeU = szRangeU + 1;

		BYTE uMinL, uMaxL;
		if ( !GetSetMinMax ( szRangeL, uMinL, uMaxL ) )
			return false;

		BYTE uMinU, uMaxU;
		if ( !GetSetMinMax ( szRangeU, uMinU, uMaxU ) )
			return false;

		if ( ( uMaxU - uMinU )!=( uMaxL - uMinL ) )
			return false;

		for ( BYTE i=0; i<=( uMaxL - uMinL ); ++i )
			if ( IsInSet ( uMinL + i, szRangeL ) && IsInSet ( uMinU + i, szRangeU ) )
				AddCharPair ( uMinL + i, uMinU + i );
	} else
	{
		if ( iLengthL > 4 || iLengthU > 4 )
			return false;

		const char * szL = szRangeL;
		const char * szU = szRangeU;
		AddCharPair ( GetWordchar(szL), GetWordchar(szU) );
	}

	m_bUseDictConversion = true;

	return true;
}


void CISpellAffix::AddCharPair ( BYTE uCharL, BYTE uCharU )
{
	m_dCharset [uCharU] = uCharL;
}


void CISpellAffix::Strip ( char * szText )
{
	char * szIterator1 = szText;
	char * szIterator2 = szText;
	while ( *szIterator1 )
	{
		if ( !isspace ( (unsigned char) *szIterator1 ) && *szIterator1!='-' )
		{
			*szIterator2 = *szIterator1;
			++szIterator2;
		}
		++szIterator1;
	}

	*szIterator2 = '\0';

	while ( *szText )
	{
		*szText = ToLowerCase ( *szText );
		++szText;
	}
}


char CISpellAffix::ToLowerCase ( char cChar )
{
	if ( m_bFirstCaseConv )
	{
		LoadLocale ();
		m_bFirstCaseConv = false;
	}

	// dictionary conversion
	if ( m_bUseDictConversion )
		return m_dCharset [(BYTE) cChar] ? m_dCharset [(BYTE) cChar] : cChar;

	// user-defined character mapping
	if ( m_bUseLowerCaser )
	{
		char cResult = (char)m_LowerCaser.ToLower ( (BYTE) cChar );
		return cResult ? cResult : cChar;
	}

	// user-specified code page conversion
	return (char)tolower ( (BYTE)cChar ); // workaround for systems (eg. FreeBSD) which default to signed char. marvelous!
}


void CISpellAffix::LoadLocale ()
{
	if ( m_bUseDictConversion )
		printf ( "Using dictionary-defined character set\n" );
	else
		if ( !m_sCharsetFile.IsEmpty () )
		{
			FILE * pFile = fopen ( m_sCharsetFile.cstr (), "rt" );
			if ( pFile )
			{
				printf ( "Using charater set from '%s'\n", m_sCharsetFile.cstr () );

				const int MAX_CHARSET_LENGTH = 4096;
				char szBuffer [MAX_CHARSET_LENGTH];

				char * szResult = fgets ( szBuffer, MAX_CHARSET_LENGTH, pFile );
				if ( szResult )
				{
					CSphVector<CSphRemapRange> dRemaps;
					if ( sphParseCharset ( szBuffer, dRemaps ) )
					{
						m_bUseLowerCaser = true;
						m_LowerCaser.AddRemaps ( dRemaps, 0 );
					} else
					{
						printf ( "Failed to parse charset from '%s'\n", m_sCharsetFile.cstr() );
					}
				} else
				{
					printf ( "Failed to read charset from '%s'\n", m_sCharsetFile.cstr() );
				}

				fclose ( pFile );

			} else
			{
				printf ( "Failed to open '%s'\n", m_sCharsetFile.cstr() );
			}

		} else
		{
			if ( !m_sLocale.IsEmpty () )
			{
				char dLocaleC[256], dLocaleUser[256];

				setlocale ( LC_ALL, "C" );
				for ( int i=0; i<256; i++ )
					dLocaleC[i] = (char) tolower(i);

				char * szLocale = setlocale ( LC_CTYPE, m_sLocale.cstr() );
				if ( szLocale )
				{
					printf ( "Using user-defined locale (locale=%s)\n", m_sLocale.cstr() );

					for ( int i=0; i<256; i++ )
						dLocaleUser[i] = (char) tolower(i);

					if ( !memcmp ( dLocaleC, dLocaleUser, 256 ) )
						printf ( "WARNING: user-defined locale provides the same case conversion as the default \"C\" locale\n" );
				} else
					printf ( "WARNING: could not set user-defined locale for case conversions (locale=%s)\n", m_sLocale.cstr() );
			} else
				printf ( "WARNING: no character set specified\n" );
		}
}

//////////////////////////////////////////////////////////////////////////

enum OutputMode_e
{
	M_DEBUG,
	M_DUPLICATES,
	M_LAST,
	M_EXACT_OR_LONGEST,
	M_DEFAULT = M_EXACT_OR_LONGEST
};

const char * dModeName[] =
{
	"debug",
	"duplicates",
	"last"
};

struct MapInfo_t
{
	CSphString	m_sWord;
	char		m_sRules[3];
};

struct WordLess
{
	inline bool IsLess ( const char * a, const char * b ) const
	{
		return strcoll ( a, b ) < 0;
	}
};

typedef CSphOrderedHash < CSphVector<MapInfo_t>, CSphString, CSphStrHashFunc, 100000, 13 > WordMap_t;

static void EmitResult ( WordMap_t & tMap , const CSphString & sFrom, const CSphString & sTo, char cRuleA = 0, char cRuleB = 0 )
{
	if ( !tMap.Exists(sFrom) )
		tMap.Add ( CSphVector<MapInfo_t>(), sFrom );

	MapInfo_t tInfo;
	tInfo.m_sWord = sTo;
	tInfo.m_sRules[0] = cRuleA;
	tInfo.m_sRules[1] = cRuleB;
	tInfo.m_sRules[2] = 0;
	tMap[sFrom].Add ( tInfo );
}

int main ( int iArgs, char ** dArgs )
{
	OutputMode_e eMode = M_DEFAULT;
	bool bUseCustomCharset = false;
	CSphString sDict, sAffix, sLocale, sCharsetFile, sResult = "result.txt";

	printf ( "spelldump, an ispell dictionary dumper\n\n" );

	int i = 1;
	for ( ; i < iArgs; i++ )
	{
		if ( !strcmp ( dArgs[i], "-c" ) )
		{
			if ( ++i==iArgs ) break;
			bUseCustomCharset = true;
			sCharsetFile = dArgs[i];

		} else if ( !strcmp ( dArgs[i], "-m" ) )
		{
			if ( ++i==iArgs ) break;
			char * sMode = dArgs[i];

			if ( !strcmp ( sMode, "debug" ) )		{ eMode = M_DEBUG; continue; }
			if ( !strcmp ( sMode, "duplicates" ) )	{ eMode = M_DUPLICATES; continue; }
			if ( !strcmp ( sMode, "last" ) )		{ eMode = M_LAST; continue; }
			if ( !strcmp ( sMode, "default" ) )		{ eMode = M_DEFAULT; continue; }

			printf ( "Unrecognized mode: %s\n", sMode );
			return 1;

		} else
			break;
	}

	switch ( iArgs - i )
	{
		case 4:
			sLocale = dArgs[i + 3];
		case 3:
			sResult = dArgs[i + 2];
		case 2:
			sAffix = dArgs[i + 1];
			sDict = dArgs[i];
			break;
		default:
			printf ( "Usage: spelldump [options] <dictionary> <affix> [result] [locale-name]\n\n"
				"Options:\n"
				"-c <file>\tuse case convertion defined in <file>\n"
				"-m <mode>\toutput (conflict resolution) mode:\n"
				"\t\tdefault - try to guess the best way to resolve a conflict\n"
				"\t\tlast - choose last entry\n"
				"\t\tdebug - dump all mappings (with rules)\n"
				"\t\tduplicates - dump duplicate mappings only (with rules)\n" );
			if ( iArgs>1 )
			{
				printf ( "\n"
					"Examples:\n"
					"spelldump en.dict en.aff\n"
					"spelldump ru.dict ru.aff ru.txt ru_RU.CP1251\n"
					"spelldump ru.dict ru.aff ru.txt .1251\n" );
			}
			return 1;
	}

	printf ( "Loading dictionary...\n" );
	CISpellDict Dict;
	if ( !Dict.Load ( sDict.cstr () ) )
		sphDie ( "Error loading dictionary file '%s'\n", sDict.IsEmpty () ? "" : sDict.cstr () );

	printf ( "Loading affix file...\n" );
	CISpellAffix Affix ( sLocale.cstr (), bUseCustomCharset ? sCharsetFile.cstr () : NULL );
	if ( !Affix.Load ( sAffix.cstr () ) )
		sphDie ( "Error loading affix file '%s'\n", sAffix.IsEmpty () ? "" : sAffix.cstr () );

	if ( sResult.IsEmpty () )
		sphDie ( "No result file specified\n" );

	FILE * pFile = fopen ( sResult.cstr (), "wt" );
	if ( !pFile )
		sphDie ( "Unable to open '%s' for writing\n", sResult.cstr () );

	if ( eMode!=M_DEFAULT )
		printf ( "Output mode: %s\n", dModeName[eMode] );

	Dict.IterateStart ();
	WordMap_t tWordMap;
	const CISpellDict::CISpellDictWord * pWord = NULL;
	int nDone = 0;
	while ( ( pWord = Dict.IterateNext () )!=NULL )
	{
		EmitResult ( tWordMap, pWord->m_sWord, pWord->m_sWord );

		if ( ( ++nDone % 10 )==0 )
		{
			printf ( "\rDictionary words processed: %d", nDone );
			fflush ( stdout );
		}

		if ( pWord->m_sFlags.IsEmpty() )
			continue;

		CSphString sWord, sWordForCross;
		int iFlagLen = strlen ( pWord->m_sFlags.cstr () );
		for ( int iFlag1 = 0; iFlag1 < iFlagLen; ++iFlag1 )
			for ( int iRule1 = 0; iRule1 < Affix.GetNumRules (); ++iRule1 )
			{
				CISpellAffixRule * pRule1 = Affix.GetRule ( iRule1 );
				if ( pRule1->Flag()!=pWord->m_sFlags.cstr()[iFlag1] )
					continue;

				sWord = pWord->m_sWord;

				if ( !pRule1->Apply ( sWord ) )
					continue;

				EmitResult ( tWordMap, sWord, pWord->m_sWord, pRule1->Flag() );

				// apply other rules
				if ( !Affix.CheckCrosses() )
					continue;

				if ( !pRule1->IsCrossProduct() )
					continue;

				for ( int iFlag2 = iFlag1 + 1; iFlag2 < iFlagLen; ++iFlag2 )
					for ( int iRule2 = 0; iRule2 < Affix.GetNumRules (); ++iRule2 )
					{
						CISpellAffixRule * pRule2 = Affix.GetRule ( iRule2 );
						if ( !pRule2->IsCrossProduct () || pRule2->Flag()!=pWord->m_sFlags.cstr()[iFlag2] ||
							pRule2->IsPrefix()==pRule1->IsPrefix() )
							continue;

						sWordForCross = sWord;
						if ( pRule2->Apply ( sWordForCross ) )
							EmitResult ( tWordMap, sWordForCross, pWord->m_sWord, pRule1->Flag(), pRule2->Flag() );
					}
			}
	}
	printf ( "\rDictionary words processed: %d\n", nDone );

	// output

	CSphVector<const char *> dKeys;
	tWordMap.IterateStart();
	while ( tWordMap.IterateNext() )
		dKeys.Add ( tWordMap.IterateGetKey().cstr() );
	dKeys.Sort ( WordLess() );

	ARRAY_FOREACH ( iKey, dKeys )
	{
		const CSphVector<MapInfo_t> & dWords = tWordMap[dKeys[iKey]];
		const char * sKey = dKeys[iKey];

		switch ( eMode )
		{
			case M_LAST:
				fprintf ( pFile, "%s > %s\n", sKey, dWords.Last().m_sWord.cstr() );
				break;

			case M_EXACT_OR_LONGEST:
			{
				int iMatch = 0;
				int iLength = 0;

				ARRAY_FOREACH ( i, dWords )
				{
					if ( dWords[i].m_sWord==sKey )
					{
						iMatch = i;
						break;
					}

					int iWordLength = strlen ( dWords[i].m_sWord.cstr() );
					if ( iWordLength>iLength )
					{
						iLength = iWordLength;
						iMatch = i;
					}
				}

				fprintf ( pFile, "%s > %s\n", sKey, dWords[iMatch].m_sWord.cstr() );
				break;
			}

			case M_DUPLICATES:
				if ( dWords.GetLength()==1 ) break;
			case M_DEBUG:
				ARRAY_FOREACH ( i, dWords )
					fprintf ( pFile, "%s > %s %s/%d\n", sKey, dWords[i].m_sWord.cstr(),
						dWords[i].m_sRules, dWords.GetLength() );
				break;
		}
	}

	fclose ( pFile );

	return 0;
}

//
// $Id$
//
