//
// $Id$
//

#include "sphinxstd.h"

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

			return strcmp ( a.m_sWord.cstr (), b.m_sWord.cstr () ) == -1;
		}
	};


	bool			Load ( const char * szFilename );
	void			IterateStart ();
	const			CISpellDictWord * IterateNext ();

private:
	CSphVector < CISpellDictWord, 131072 > m_dEntries;
	int				m_iIterator;
};


bool CISpellDict::Load ( const char * szFilename )
{
	if ( !szFilename )
		return false;

	m_dEntries.Reset ();

	FILE * pFile = fopen ( szFilename, "rt" );
	if ( !pFile )
		return false;

	char szWordBuffer [MAX_STR_LENGTH];

	bool bOk = true;

	while ( !feof ( pFile ) && bOk )
	{
		char * szResult = fgets ( szWordBuffer, MAX_STR_LENGTH, pFile );
		if ( !szResult && !feof ( pFile ) )
			bOk = false;
		else
		{
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
	}

	fclose ( pFile );

	if ( bOk )
		m_dEntries.Sort ( DictWordLess () );

	return bOk;
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
				CISpellAffixRule ( RuleType_e eRule, char cFlag, char * szCondition, char * szStrip, char * szAppend );

	bool		Apply ( CSphString & sWord );
	char		Flag () const;

private:
	RuleType_e	m_eRule;
	char		m_cFlag;
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


CISpellAffixRule::CISpellAffixRule ( RuleType_e eRule, char cFlag, char * szCondition, char * szStrip, char * szAppend )
	: m_eRule		( eRule )
	, m_cFlag		( cFlag )
	, m_sCondition	( szCondition )
	, m_sStrip		( szStrip )
	, m_sAppend		( szAppend )
	, m_iWordLen	( 0 )
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
		{
			printf ( "Warning: can't append suffix to '%s'. Wrong flags?\n", sWord.cstr () );
			return false;
		}
	}
	else
	{
		if ( ! bDotCond && ! CheckPrefix ( sWord ) )
			return false;

		if ( ! StripAppendPrefix ( sWord ) )
		{
			printf ( "Warning: can't append prefix to '%s'. Wrong flags?\n", sWord.cstr () );
			return false;
		}
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

//////////////////////////////////////////////////////////////////////////

class CISpellAffix
{
public:
				CISpellAffix ( const char * szLocale );

	bool		Load ( const char * szFilename );
	CISpellAffixRule * GetRule ( int iRule );
	int			GetNumRules () const;
	bool		HaveCharset () const;

private:
	CSphVector < CISpellAffixRule > m_dRules;

	char		m_dCharset [256];
	bool		m_bFirstCaseConv;
	CSphString	m_sLocale;

	bool		AddToCharset ( char * szRangeL, char * szRangeU );
	void		AddCharPair ( char cCharL, char cCharU );
	void		Strip ( char * szText );
	char		ToLowerCase ( char cChar );
	void		LoadLocale ();
};


CISpellAffix::CISpellAffix ( const char * szLocale )
	: m_sLocale ( szLocale )
{
}


bool CISpellAffix::Load (  const char * szFilename )
{
	if ( !szFilename )
		return false;

	m_dRules.Reset ();
	memset ( m_dCharset, 0, sizeof ( m_dCharset ) );
	m_bFirstCaseConv = true;

	FILE * pFile = fopen ( szFilename, "rt" );
	if ( !pFile )
		return false;

	char szBuffer	 [MAX_STR_LENGTH];
	char szCondition [MAX_STR_LENGTH];
	char szStrip	 [MAX_STR_LENGTH];
	char szAppend    [MAX_STR_LENGTH];

	bool bOk = true;
	RuleType_e eRule = RULE_NONE;
	char cFlag = '\0';

	// TODO: parse all .aff character replacement commands
	while ( !feof ( pFile ) && bOk )
	{
		char * szResult = fgets ( szBuffer, MAX_STR_LENGTH, pFile );
		if ( !szResult && !feof ( pFile ) )
		{
			bOk = false;
			break;
		}

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
				printf ( "Warning: invalid 'wordchars' statement\n" );
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
				printf ( "Warning: cannot add to charset: '%s' '%s'\n", szRangeL, szRangeU );

			continue;
		}
		
		if ( !strncasecmp ( szBuffer, "flag", 4 ) )
		{
			if ( eRule == RULE_NONE )
			{
				printf ( "Warning: 'flag' appears before preffixes or suffixes\n" );
				continue;
			}
			
			char * szStart = szBuffer + 4;
			while ( *szStart && isspace ( (unsigned char) *szStart ) )
				++szStart;

			if ( *szStart != '*' )
			{
				bOk = false;
				break;
			}

			cFlag = *(szStart + 1);
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

		CISpellAffixRule Rule ( eRule, cFlag, szCondition, szStrip, szAppend );
		m_dRules.Add ( Rule );
	}

	fclose ( pFile );

	return bOk;
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

		for ( char i = 0; i < cMaxL - cMinL; ++i )
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
	m_dCharset [(BYTE)cCharL] = cCharU;
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
	if ( m_dCharset [(BYTE) cChar] )
		return m_dCharset [(BYTE) cChar];

	// user-defined code page conversion
	return (char)tolower ( cChar );
}


void CISpellAffix::LoadLocale ()
{
	if ( HaveCharset () )
		printf ( "Using dictionary-defined character set\n" );
	else
		if ( ! m_sLocale.IsEmpty () )
		{
			char szLocaleBuffer [MAX_STR_LENGTH];
			sprintf ( szLocaleBuffer, ".%s", m_sLocale.cstr () );
			char * szLocale = setlocale( LC_ALL, szLocaleBuffer );
			if ( szLocale )
				printf ( "Using user-defined character set: '%s'\n", szLocale );
			else
				printf ( "Warning: could not load specified code page\n" );
		}
		else
			printf ( "Warning: no character set specified\n" );
}

//////////////////////////////////////////////////////////////////////////

int main ( int argc, char ** argv )
{
	CSphString sDict, sAffix, sLocale, sResult = "result.txt";

	printf ( "spelldump, an ispell dictionary dumper\n\n" );

	switch ( argc )
	{
		case 5:
			sLocale = argv [4];
		case 4:
			sResult = argv [3];
		case 3:
			sDict = argv [1];
			sAffix = argv [2];
			break;
		default:
			printf ( "Usage: spelldump <dictionary> <affix> [result] [codepage_number]\n" );
			if ( argc==1 )
				exit ( 0 );
			break;
	}

	printf ( "Loading dictionary...\n" );
	CISpellDict Dict;
	if ( !Dict.Load ( sDict.cstr () ) )
		sphDie ( "Error loading dictionary file '%s'\n", sDict.IsEmpty () ? "" : sDict.cstr () );

	printf ( "Loading affix file...\n" );
	CISpellAffix Affix ( sLocale.cstr () );
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

		if ( ! pWord->m_sFlags.IsEmpty () )
		{
			for ( unsigned int i = 0; i < strlen ( pWord->m_sFlags.cstr () ); ++i )
			{
				for ( int j = 0; j < Affix.GetNumRules (); ++j )
				{
					CISpellAffixRule * pRule = Affix.GetRule ( j );
					if ( pRule->Flag () == pWord->m_sFlags.cstr () [i] )
					{
						CSphString sWord = pWord->m_sWord;
						if ( pRule->Apply ( sWord ) )
							fprintf ( pResultFile, "%s > %s\n", sWord.cstr (), pWord->m_sWord.cstr () );
					}
				}
			}
		}

		if ( nDone % 5 == 0 )
			printf ( "\rDictionary words processed: %d", nDone );

		++nDone;
	}

	fclose ( pResultFile );

	return 0;
}

//
// $Id$
//
