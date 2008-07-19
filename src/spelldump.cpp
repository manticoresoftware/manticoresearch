//
// $Id: spelldump.cpp 853 2007-10-05 13:58:25Z shodan $
//

#include "sphinx.h"

#include <locale.h>


const int MAX_STR_LENGTH = 512;

//////////////////////////////////////////////////////////////////////////

bool IsInSet ( char cLetter, const char * szSet )
{
	if ( !szSet )
		return false;

	bool bInvert = *szSet == '^';
	if ( bInvert )
		++szSet;

	bool bRange = strlen ( szSet ) == 3 && szSet [1] == '-';

	if ( bRange )
	{
		if ( cLetter >= Min ( szSet [0], szSet [2] ) && cLetter <= Max ( szSet [0], szSet [2] ) )
			return !bInvert;
	}
	else
	{
		while ( *szSet && *szSet != cLetter )
			++szSet;

		bool bEnd = *szSet == '\0';

		if ( bInvert && bEnd )
			return true;

		if ( !bInvert && !bEnd )
			return true;
	}

	return false;
}


bool GetSetMinMax ( const char * szSet, char & cMin, char & cMax )
{
	if ( !szSet || !*szSet )
		return false;

	cMin = *szSet;
	cMax = *szSet;

	while ( *szSet )
	{
		if ( *szSet != '-' )
		{
			if ( *szSet < cMin )
				cMin = *szSet;

			if ( *szSet > cMax )
				cMax = *szSet;
		}
		++szSet;
	}

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

	// string comparator
	struct DictWordLess
	{
		inline bool operator () ( const CISpellDictWord & a, const CISpellDictWord & b )
		{
			if ( a.m_sWord.IsEmpty () )
				return b.m_sWord.IsEmpty () ? false : true;

			if ( b.m_sWord.IsEmpty () )
				return false;

			return strcmp ( a.m_sWord.cstr (), b.m_sWord.cstr () ) < 0;
		}
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
		while ( iPos >= 0 && isspace ( (unsigned char)szWordBuffer [iPos] ) )
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
		}
		else
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

	m_dEntries.Sort ( DictWordLess () );

	return true;
}


void CISpellDict::IterateStart ()
{
	m_iIterator = 0;
}


const CISpellDict::CISpellDictWord * CISpellDict::IterateNext ()
{
	if ( m_iIterator >= m_dEntries.GetLength () )
		return NULL;

	return &m_dEntries [m_iIterator++];
}

//////////////////////////////////////////////////////////////////////////

enum RuleType_e
{
	 RULE_NONE
	,RULE_PREFIXES
	,RULE_SUFFIXES
};


class CISpellAffixRule
{
public:
				CISpellAffixRule () {};
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

	bool bDotCond = m_sCondition == ".";
	
	if ( m_eRule == RULE_SUFFIXES )
	{
		if ( ! bDotCond && ! CheckSuffix ( sWord ) )
			return false;

		if ( ! StripAppendSuffix ( sWord ) )
			return false;
	}
	else
	{
		if ( ! bDotCond && ! CheckPrefix ( sWord ) )
			return false;

		if ( ! StripAppendPrefix ( sWord ) )
			return false;
	}

	return true;
}


bool CISpellAffixRule::CheckSuffix ( const CSphString & sWord ) const
{
	int iCondI = m_iCondLen - 1;
	for ( int i = m_iWordLen - 1; iCondI >= 0 && i >= 0; --i )
	{
		if ( m_sCondition.cstr () [iCondI] != ']' )
		{
			if ( m_sCondition.cstr () [iCondI] != sWord.cstr () [i] )
				return false;

			--iCondI;
		}
		else
		{
			int iRangeStart = -1;

			for ( int j = iCondI; j >=0 && iRangeStart == -1; --j )
				if ( m_sCondition.cstr () [j] == '[' )
					iRangeStart = j;

			if ( iRangeStart == -1 )
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
		strcat ( szTmp, m_sAppend.cstr () );

	sWord = szTmp;

	return true;
}


bool CISpellAffixRule::CheckPrefix ( const CSphString & sWord ) const
{
	int iCondI = 0;
	for ( int i = 0; iCondI < m_iCondLen && i < m_iWordLen; ++i )
	{
		if ( m_sCondition.cstr () [iCondI] != '[' )
		{
			if ( m_sCondition.cstr () [iCondI] != sWord.cstr () [i] )
				return false;

			++iCondI;
		}
		else
		{
			int iRangeEnd = -1;

			for ( int j = iCondI; j < m_iCondLen && iRangeEnd == -1; ++j )
				if ( m_sCondition.cstr () [j] == ']' )
					iRangeEnd = j;

			if ( iRangeEnd == -1 )
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
	static char szTmp [ MAX_STR_LENGTH];

	if ( !m_sStrip.IsEmpty () )
	{
		const char * Pos = strstr ( sWord.cstr (), m_sStrip.cstr () );
		if ( Pos != sWord.cstr () )
			return false;
	}

	if ( !m_sAppend.IsEmpty () )
		strcpy ( szTmp, m_sAppend.cstr () );

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
	return m_eRule == RULE_PREFIXES;
}

//////////////////////////////////////////////////////////////////////////

class CISpellAffix
{
public:
				CISpellAffix ( const char * szLocale, const char * szCharsetFile  );

	bool		Load ( const char * szFilename );
	CISpellAffixRule * GetRule ( int iRule );
	int			GetNumRules () const;
	bool		HaveCharset () const;
	bool		CheckCrosses () const;

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
	void		AddCharPair ( char cCharL, char cCharU );
	void		Strip ( char * szText );
	char		ToLowerCase ( char cChar );
	void		LoadLocale ();
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


bool CISpellAffix::Load (  const char * szFilename )
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

	char szBuffer	 [MAX_STR_LENGTH];
	char szCondition [MAX_STR_LENGTH];
	char szStrip	 [MAX_STR_LENGTH];
	char szAppend    [MAX_STR_LENGTH];

	RuleType_e eRule = RULE_NONE;
	char cFlag = '\0';
	bool bCrossProduct = false;

	// TODO: parse all .aff character replacement commands
	while ( !feof ( pFile ) )
	{
		char * szResult = fgets ( szBuffer, MAX_STR_LENGTH, pFile );
		if ( !szResult )
			break;

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
				printf ( "WARNING: invalid 'wordchars' statement\n" );
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
				printf ( "WARNING: cannot add to charset: '%s' '%s'\n", szRangeL, szRangeU );

			continue;
		}
		
		if ( !strncasecmp ( szBuffer, "flag", 4 ) )
		{
			if ( eRule == RULE_NONE )
			{
				printf ( "WARNING: 'flag' appears before preffixes or suffixes\n" );
				continue;
			}
			
			char * szStart = szBuffer + 4;
			while ( *szStart && isspace ( (unsigned char) *szStart ) )
				++szStart;

			bCrossProduct = *szStart == '*';

			cFlag = bCrossProduct ? *(szStart + 1) : *(szStart);
			continue;
		}

		if ( eRule == RULE_NONE )
			continue;

		char * szComment = strchr ( szBuffer,'#' );
		if ( szComment )
			*szComment = '\0';
	
		if ( !* szBuffer )
			continue;

		szCondition [0]= '\0';
		szStrip [0]= '\0';
		szAppend [0]= '\0';

		int nFields = sscanf ( szBuffer, "%[^>\n]>%[^,\n],%[^\n]", szCondition, szStrip, szAppend );

		Strip ( szCondition );
		Strip ( szStrip );
		Strip ( szAppend );

		switch ( nFields )
		{
		case 2: // no optional strip-string
			strcpy ( szAppend, szStrip );
			szStrip [0] = '\0';
			break;
		case 3:	// all read
			break;
		default: // invalid repl
			continue;
		}

		CISpellAffixRule Rule ( eRule, cFlag, bCrossProduct, szCondition, szStrip, szAppend );
		m_dRules.Add ( Rule );
	}

	fclose ( pFile );

	bool bHaveCrossPrefix = false;
	for ( int i = 0; i < m_dRules.GetLength () && !bHaveCrossPrefix; ++i )
		if ( m_dRules [i].IsPrefix () && m_dRules [i].IsCrossProduct () )
			bHaveCrossPrefix = true;

	bool bHaveCrossSuffix = false;
	for ( int i = 0; i < m_dRules.GetLength () && !bHaveCrossSuffix; ++i )
		if ( !m_dRules [i].IsPrefix () && m_dRules [i].IsCrossProduct () )
			bHaveCrossSuffix = true;

	m_bCheckCrosses = bHaveCrossPrefix && bHaveCrossSuffix;

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


bool CISpellAffix::HaveCharset () const
{
	for ( int i = 0; i < 256; ++i )
		if ( m_dCharset [i] )
			return true;

	return false;
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

	bool bSetL = iLengthL > 0 && szRangeL [0] == '[' && szRangeL [iLengthL - 1] == ']';
	bool bSetR = iLengthU > 0 && szRangeU [0] == '[' && szRangeU [iLengthU - 1] == ']';

	if ( bSetL != bSetR )
		return false;

	if ( bSetL )
	{
		szRangeL [iLengthL - 1] = '\0';
		szRangeL = szRangeL + 1;

		szRangeU [iLengthU - 1] = '\0';
		szRangeU = szRangeU + 1;

		char cMinL, cMaxL;
		if ( !GetSetMinMax ( szRangeL, cMinL, cMaxL ) )
			return false;

		char cMinU, cMaxU;
		if ( !GetSetMinMax ( szRangeU, cMinU, cMaxU ) )
			return false;

		if ( cMaxU - cMinU != cMaxL - cMinL )
			return false;

		for ( char i = 0; i <= cMaxL - cMinL; ++i )
			if ( IsInSet ( cMinL + i, szRangeL ) && IsInSet ( cMinU + i, szRangeU ) )
				AddCharPair ( cMinL + i, cMinU + i );
	}
	else
	{
		if ( iLengthL != 1 || iLengthU != 1 )
			return false;

		AddCharPair ( *szRangeL, *szRangeU );
	}

	return true;
}


void CISpellAffix::AddCharPair ( char cCharL, char cCharU )
{
	m_dCharset [(BYTE)cCharU] = cCharL;
}


void CISpellAffix::Strip ( char * szText )
{
	char * szIterator1 = szText;
	char * szIterator2 = szText;
	while ( *szIterator1 )
	{
		if ( !isspace ( (unsigned char) *szIterator1 ) && *szIterator1 != '-' )
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
		char cResult = (char)m_LowerCaser.ToLower( (BYTE) cChar );
		return cResult ? cResult : cChar;
	}

	// user-specified code page conversion
	return (char)tolower ( (BYTE)cChar ); // workaround for systems (eg. FreeBSD) which default to signed char. marvelous!
}


void CISpellAffix::LoadLocale ()
{
	if ( HaveCharset () )
	{
		m_bUseDictConversion = true;
		printf ( "Using dictionary-defined character set\n" );
	}
	else
		if ( !m_sCharsetFile.IsEmpty () )
		{
			FILE * pFile = fopen ( m_sCharsetFile.cstr (), "rt" );
			bool bError = pFile == NULL;
			if ( pFile )
			{
				printf ( "Using charater set from '%s'\n", m_sCharsetFile.cstr () );
				
				const int MAX_CHARSET_LENGTH = 4096;
				char szBuffer [MAX_CHARSET_LENGTH];

				char * szResult = fgets ( szBuffer, MAX_CHARSET_LENGTH, pFile );
				if ( szResult )
				{
					CSphCharsetDefinitionParser tParser;
					CSphVector<CSphRemapRange> dRemaps;
					if ( tParser.Parse ( szBuffer, dRemaps ) )
					{
						m_bUseLowerCaser = true;
						m_LowerCaser.AddRemaps ( dRemaps, 0, 0 );
					}
					else
						bError = true;
				}
				else
					bError = true;
			}

			if ( bError )
				printf ( "Error loading character set\n" );
		}
		else
			if ( !m_sLocale.IsEmpty () )
			{
				char dLocaleC[256], dLocaleUser[256];

				setlocale ( LC_ALL, "C" );
				for ( int i=0; i<256; i++ )
					dLocaleC [i] = (char) tolower(i);

				char * szLocale = setlocale ( LC_CTYPE, m_sLocale.cstr() );
				if ( szLocale )
				{
					printf ( "Using user-defined locale (locale=%s)\n", m_sLocale.cstr() );

					for ( int i=0; i<256; i++ )
						dLocaleUser [i] = (char) tolower(i);

					if ( !memcmp ( dLocaleC, dLocaleUser, 256 ) )
						printf ( "WARNING: user-defined locale provides the same case conversion as the default \"C\" locale\n" );
				}
				else
					printf ( "WARNING: could not set user-defined locale for case conversions (locale=%s)\n", m_sLocale.cstr() );
			}
			else
				printf ( "WARNING: no character set specified\n" );
}

//////////////////////////////////////////////////////////////////////////

int main ( int argc, char ** argv )
{
	bool bUseCustomCharset = false;
	CSphString sDict, sAffix, sLocale, sCharsetFile, sResult = "result.txt";

	printf ( "spelldump, an ispell dictionary dumper\n\n" );

	int nArgsStart = 1;

	if ( argc >= 3 )
	{
		if ( ! strcmp ( argv [1], "-c" ) )
		{
			bUseCustomCharset = true;
			sCharsetFile = argv [2];
			nArgsStart += 2;
		}
	}

	switch ( argc - nArgsStart )
	{
		case 4:
			sLocale = argv [nArgsStart + 3];
		case 3:
			sResult = argv [nArgsStart + 2];
		case 2:
			sAffix = argv [nArgsStart + 1];
			sDict = argv [nArgsStart];
			break;
		default:
			printf ( "Usage: spelldump [options] <dictionary> <affix> [result] [locale-name]\n\n"
					 "options:\n"
					 "-c <file> : use case convertion defined in <file>\n" );

			if ( argc==1 )
			{
				printf ( "\n"
					"Examples:\n"
					"spelldump en.dict en.aff\n"
					"spelldump ru.dict ru.aff ru.txt ru_RU.CP1251\n"
					"spelldump ru.dict ru.aff ru.txt .1251\n" );
				exit ( 0 );
			}
			break;
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

	FILE * pResultFile = fopen ( sResult.cstr (), "wt" );
	if ( !pResultFile )
		sphDie ( "Unable to open '%s' for writing\n", sResult.cstr () );

	Dict.IterateStart ();
	const CISpellDict::CISpellDictWord * pWord = NULL;
	int nDone = 0;
	while ( ( pWord = Dict.IterateNext () ) != NULL )
	{
		fprintf ( pResultFile, "%s > %s\n", pWord->m_sWord.cstr (), pWord->m_sWord.cstr () );

		CSphString sWord, sWordForCross;

		if ( ! pWord->m_sFlags.IsEmpty () )
		{
			int iFlagLen = strlen ( pWord->m_sFlags.cstr () );

			for ( int iFlag1 = 0; iFlag1 < iFlagLen; ++iFlag1 )
				for ( int iRule1 = 0; iRule1 < Affix.GetNumRules (); ++iRule1 )
				{
					CISpellAffixRule * pRule1 = Affix.GetRule ( iRule1 );
					if ( pRule1->Flag () != pWord->m_sFlags.cstr () [iFlag1] )
						continue;

					sWord = pWord->m_sWord;

					if ( ! pRule1->Apply ( sWord ) )
						continue;

					fprintf ( pResultFile, "%s > %s\n", sWord.cstr (), pWord->m_sWord.cstr () );

					// apply other rules
					if ( ! Affix.CheckCrosses () )
						continue;

					if ( ! pRule1->IsCrossProduct () )
						continue;

					for ( int iFlag2 = iFlag1 + 1; iFlag2 < iFlagLen; ++iFlag2 )
						for ( int iRule2 = 0; iRule2 < Affix.GetNumRules (); ++iRule2 )
						{
							CISpellAffixRule * pRule2 = Affix.GetRule ( iRule2 );
							if ( ! pRule2->IsCrossProduct () || pRule2->Flag () != pWord->m_sFlags.cstr () [iFlag2] || pRule2->IsPrefix () == pRule1->IsPrefix () )
								continue;

							sWordForCross = sWord;
							if ( pRule2->Apply ( sWordForCross ) )
								fprintf ( pResultFile, "%s > %s\n", sWordForCross.cstr (), pWord->m_sWord.cstr () );
						}
				}
		}

		if ( nDone % 5 == 0 )
			printf ( "\rDictionary words processed: %d", nDone );

		++nDone;
	}

	printf ( "\n" );

	fclose ( pResultFile );

	return 0;
}

//
// $Id: spelldump.cpp 853 2007-10-05 13:58:25Z shodan $
//
