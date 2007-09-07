//
// $Id$
//

#include "sphinxstd.h"

const int MAX_STR_LENGTH = 512;

#if !USE_WINDOWS
char * strlwr ( char * s )
{
      while ( *s )
      {
              *s = tolower ( *s );
              s++;
      }
      return s;
}
#endif

//////////////////////////////////////////////////////////////////////////

class CISpellDict
{
public:
	struct CISpellDictWord
	{
		CSphString	m_sWord;
		CSphString	m_sFlags;
	};

					~CISpellDict ();

	bool			Load ( const char * szFilename );
	void			IterateStart ();
	const			CISpellDictWord * IterateNext ();

private:
	CSphVector < CISpellDictWord * > m_dEntries;
	int				m_iIterator;

	void			Cleanup ();
};


CISpellDict::~CISpellDict ()
{
	Cleanup ();
}

bool CISpellDict::Load ( const char * szFilename )
{
	if ( ! szFilename )
		return false;

	Cleanup ();

	FILE * pFile = fopen ( szFilename, "rt" );
	if ( ! pFile )
		return false;

	char szWordBuffer [MAX_STR_LENGTH];

	bool bOk = true;

	while ( ! feof ( pFile ) && bOk )
	{
		char * szResult = fgets ( szWordBuffer, MAX_STR_LENGTH, pFile );
		if ( ! szResult && ! feof ( pFile ) )
			bOk = false;
		else
		{
			int iPos = strlen ( szWordBuffer ) - 1;
			while ( iPos >= 0 && isspace ( (unsigned char)szWordBuffer [iPos] ) )
				szWordBuffer [iPos--] = '\0';

			CISpellDictWord * pWord = new CISpellDictWord;
			if ( ! pWord )
				break;

			char * szPosition = strchr ( szWordBuffer, '/' );
			if ( ! szPosition )
				pWord->m_sWord = szResult;
			else
			{
				*szPosition = '\0';
				pWord->m_sWord = szWordBuffer;
				++szPosition;
				char * szFlags = szPosition;
				while ( *szPosition && ! isspace ( (unsigned char)*szPosition ) )
					++szPosition;

				*szPosition = '\0';
				pWord->m_sFlags = szFlags;
			}

			m_dEntries.Add ( pWord );
		}
	}

	fclose ( pFile );
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

	return m_dEntries [m_iIterator++];
}

void CISpellDict::Cleanup ()
{
	for ( int i = 0; i < m_dEntries.GetLength (); ++i )
		delete m_dEntries [i];

	m_dEntries.Reset ();
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
				CISpellAffixRule ( RuleType_e eRule, char cFlag, char * szCondition, char * szStrip, char * szAppend );

	bool		Apply ( CSphString & sWord );
	char		Flag () const;

private:
	RuleType_e	m_eRule;
	char		m_cFlag;
	CSphString	m_sCondition;
	CSphString	m_sStrip;
	CSphString	m_sAppend;

	bool		CheckRange ( char cLetter, const CSphString & sRange ) const;
};


CISpellAffixRule::CISpellAffixRule ( RuleType_e eRule, char cFlag, char * szCondition, char * szStrip, char * szAppend )
	: m_eRule		( eRule )
	, m_cFlag		( cFlag )
	, m_sCondition	( szCondition )
	, m_sStrip		( szStrip )
	, m_sAppend		( szAppend )
{
}

bool CISpellAffixRule::Apply ( CSphString & sWord )
{
	if ( m_sCondition.IsEmpty () )
		return true;

	if ( sWord.IsEmpty () )
		return false;

	bool bFits = true;

	if ( m_eRule == RULE_SUFFIXES )
	{
		int iWordLen = strlen ( sWord.cstr () );
		if ( m_sCondition != "." )
		{
			int iCondI = strlen ( m_sCondition.cstr () ) - 1;
			for ( int i = iWordLen - 1; iCondI >= 0 && i >= 0 && bFits; --i )
			{
				if ( m_sCondition.cstr () [iCondI] != ']' )
				{
					if ( m_sCondition.cstr () [iCondI] != sWord.cstr () [i] )
						bFits = false;
				}
				else
				{
					int iRangeStart = -1;
					
					for ( int j = iCondI; j >=0 && iRangeStart == -1; --j )
						if ( m_sCondition.cstr () [j] == '[' )
							iRangeStart = j;

					if ( iRangeStart == -1 )
						bFits = false;
					else
					{
						if ( ! CheckRange ( sWord.cstr () [i], m_sCondition.SubString ( iRangeStart + 1, iCondI - iRangeStart - 1 ) ) )
							bFits = false;
						iCondI = iRangeStart - 1;
					}
				}
			}
		}

		if ( ! bFits )
			return false;

		char szTmp [MAX_STR_LENGTH];
		int nCopy = iWordLen;

		if ( ! m_sStrip.IsEmpty () )
		{
			const char * Pos = strstr ( sWord.cstr (), m_sStrip.cstr () );
			nCopy = Pos - sWord.cstr ();
			if ( nCopy != iWordLen - (int)strlen ( m_sStrip.cstr () ) )
				return false;
		}

		strncpy ( szTmp, sWord.cstr (), nCopy );
		szTmp [nCopy] = '\0';

		if ( ! m_sAppend.IsEmpty () )
			strcat ( szTmp, m_sAppend.cstr () );
		
		sWord = szTmp;
	}
	else
	{
		// TODO
	}

	return true;
}

char CISpellAffixRule::Flag () const
{
	return m_cFlag;
}

bool CISpellAffixRule::CheckRange ( char cLetter, const CSphString & sRange ) const
{
	if ( sRange.IsEmpty () )
		return false;

	const char * szRange = sRange.cstr ();

	bool bInvert = *szRange == '^';
	if ( bInvert )
		++szRange;

	while ( *szRange && *szRange != cLetter )
		++szRange;

	bool bEnd = *szRange == '\0';

	if ( bInvert && bEnd )
		return true;

	if ( ! bInvert && ! bEnd )
		return true;

	return false;
}

//////////////////////////////////////////////////////////////////////////
class CISpellAffix
{
public:
				~CISpellAffix ();

	bool		Load ( const char * szFilename );
	CISpellAffixRule * GetRule ( int iRule );
	int			GetNumRules () const;

private:
	CSphVector < CISpellAffixRule * > m_dRules;

	void		Strip ( char * szText );
	void		Cleanup ();
};

CISpellAffix::~CISpellAffix ()
{
	Cleanup ();
}

bool CISpellAffix::Load (  const char * szFilename )
{
	if ( ! szFilename )
		return false;

	Cleanup ();

	FILE * pFile = fopen ( szFilename, "rt" );
	if ( ! pFile )
		return false;

	char szBuffer [MAX_STR_LENGTH];
	char szCondition  [MAX_STR_LENGTH];
	char szStrip  [MAX_STR_LENGTH];
	char szAppend  [MAX_STR_LENGTH];

	bool bOk = true;
	RuleType_e eRule = RULE_NONE;
	char cFlag = '\0';

	// TODO: parse all .aff character replacement commands
	while ( ! feof ( pFile ) && bOk )
	{
		char * szResult = fgets ( szBuffer, MAX_STR_LENGTH, pFile );
		if ( ! szResult && ! feof ( pFile ) )
		{
			bOk = false;
			break;
		}

		if ( ! strncasecmp ( szBuffer, "prefixes", 8 ) )
		{
			eRule = RULE_PREFIXES;
			continue;
		}

		if ( ! strncasecmp ( szBuffer, "suffixes", 8 ) )
		{
			eRule = RULE_SUFFIXES;
			continue;
		}
		
		if ( ! strncasecmp ( szBuffer, "flag", 4 ) )
		{
			if ( eRule == RULE_NONE )
			{
				printf ( "Warning: \"flag\" appears before preffixes or suffixes" );
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
	
		if ( ! * szBuffer )
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

		CISpellAffixRule * pRule = new CISpellAffixRule ( eRule, cFlag, szCondition, szStrip, szAppend );
		if ( ! pRule )
			bOk = false;
		else
			m_dRules.Add ( pRule );
	}

	fclose ( pFile );

	return bOk;
}

CISpellAffixRule * CISpellAffix::GetRule ( int iRule )
{
	return m_dRules [iRule];
}

int CISpellAffix::GetNumRules () const
{
	return m_dRules.GetLength ();
}

void CISpellAffix::Strip ( char * szText )
{
	char * szIterator1 = szText;
	char * szIterator2 = szText;
	while ( *szIterator1 )
	{
		if ( ! isspace ( (unsigned char) *szIterator1 ) && *szIterator1 != '-' )
		{
			*szIterator2 = *szIterator1;
			++szIterator2;
		}
		++szIterator1;
	}

	*szIterator2 = '\0';

	// FIXME, !COMMIT
	// use a user-defined or dictionary charset in here
	strlwr ( szText );

	char Upper [] = "¿¡¬√ƒ≈®∆«»… ÀÃÕŒœ–—“”‘’÷◊ÿŸ⁄€‹›ﬁﬂ";
	char Lower [] = "‡·‚„‰Â∏ÊÁËÈÍÎÏÌÓÔÒÚÛÙıˆ˜¯˘˙˚¸˝˛ˇ";

	for ( unsigned int i = 0; i < strlen ( szText ); ++i )
	{
		for ( unsigned int j = 0; j < strlen ( Upper); ++j )
			if ( szText [i] == Upper [j] )
				szText [i] = Lower [j];
	}
}

void CISpellAffix::Cleanup ()
{
	for ( int i = 0; i < m_dRules.GetLength (); ++i )
		delete m_dRules [i];

	m_dRules.Reset ();
}

//////////////////////////////////////////////////////////////////////////

int main ( int argc, char ** argv )
{
	CSphString sDict, sAffix, sResult = "result.txt";

	printf ( "spelldump, an ispell dictionary dumper\n\n" );

	switch ( argc )
	{
		case 4:
			sResult = argv [3];
		case 3:
			sDict = argv [1];
			sAffix = argv [2];
			break;
		default:
			printf ( "Usage: spelldump <dictionary> <affix> [result]\n" );
			if ( argc==1 )
				exit ( 0 );
			break;
	}

	CISpellDict Dict;
	if ( ! Dict.Load ( sDict.cstr () ) )
	{
		printf ( "Error loading dictionary file '%s'\n", sDict.IsEmpty () ? "" : sDict.cstr () );
		return 1;
	}

	CISpellAffix Affix;
	if ( ! Affix.Load ( sAffix.cstr () ) )
	{
		printf ( "Error loading affix file '%s'\n", sAffix.IsEmpty () ? "" : sAffix.cstr () );
		return 1;
	}

	if ( sResult.IsEmpty () )
	{
		printf ( "No result file specified\n" );
		return 1;
	}

	FILE * pResultFile = fopen ( sResult.cstr (), "wt" );
	if ( ! pResultFile )
	{
		printf ( "Unable to open '%s' for writing\n", sResult.cstr () );
		return 1;
	}

	Dict.IterateStart ();
	const CISpellDict::CISpellDictWord * pWord = NULL;
	int nDone = 0;
	while ( ( pWord = Dict.IterateNext () ) != NULL )
	{
		if ( pWord->m_sFlags.IsEmpty () )
			fprintf ( pResultFile, "%s > %s\n", pWord->m_sWord.cstr (), pWord->m_sWord.cstr () );
		else
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

		printf ( "\rDictionary words processed: %d", ++nDone );
	}

	fclose ( pResultFile );

	return 0;
}

//
// $Id$
//
