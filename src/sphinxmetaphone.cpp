//
// $Id$
//

//
// Copyright (c) 2001-2015, Andrew Aksyonoff
// Copyright (c) 2008-2015, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxint.h"


struct CurrentWord_t
{
	BYTE *	pWord;
	int		iLength;
	int		iLengthPadded;
};


static bool IsVowel ( BYTE c )
{
	return c=='A' || c=='E' || c=='I' || c=='O' || c=='U' || c=='Y';
}


static bool SlavoGermanic ( BYTE * pString )
{
	// OPTIMIZE!
	char * szWord = (char *) pString;

	if ( strstr ( szWord, "W" ) )
		return true;

	if ( strstr ( szWord, "K" ) )
		return true;

	if ( strstr ( szWord, "CZ" ) )
		return true;

	if ( strstr ( szWord, "WITZ" ) )
		return true;

	return false;
}


static bool StrAt ( const CurrentWord_t & Word, int iStart, int iLength, const char * szStr1 )
{
	if ( iStart<0 || iStart>=Word.iLengthPadded )
		return false;

	char * szPos = (char *)Word.pWord + iStart;
	return !strncmp ( szPos, szStr1, iLength );
}


static bool StrAt ( const CurrentWord_t & Word, int iStart, int iLength, const char * szStr1, const char * szStr2 )
{
	if ( iStart<0 || iStart>=Word.iLengthPadded )
		return false;

	char * szPos = (char *)Word.pWord + iStart;
	return !strncmp ( szPos, szStr1, iLength ) || !strncmp ( szPos, szStr2, iLength );
}


static bool StrAt ( const CurrentWord_t & Word, int iStart, int iLength, const char * szStr1, const char * szStr2, const char * szStr3 )
{
	if ( iStart<0 || iStart>=Word.iLengthPadded )
		return false;

	char * szPos = (char *)Word.pWord + iStart;
	return !strncmp ( szPos, szStr1, iLength ) || !strncmp ( szPos, szStr2, iLength ) || !strncmp ( szPos, szStr3, iLength );
}


static bool StrAt ( const CurrentWord_t & Word, int iStart, int iLength, const char * szStr1,
	const char * szStr2, const char * szStr3, const char * szStr4 )
{
	if ( iStart<0 || iStart>=Word.iLengthPadded )
		return false;

	char * szPos = (char *)Word.pWord + iStart;
	return !strncmp ( szPos, szStr1, iLength ) || !strncmp ( szPos, szStr2, iLength ) || !strncmp ( szPos, szStr3, iLength )
		|| !strncmp ( szPos, szStr4, iLength );
}

static bool StrAt ( const CurrentWord_t & Word, int iStart, int iLength, const char * szStr1,
	const char * szStr2, const char * szStr3, const char * szStr4, const char * szStr5 )
{
	if ( iStart<0 || iStart>=Word.iLengthPadded )
		return false;

	char * szPos = (char *)Word.pWord + iStart;
	return !strncmp ( szPos, szStr1, iLength ) || !strncmp ( szPos, szStr2, iLength ) || !strncmp ( szPos, szStr3, iLength )
		|| !strncmp ( szPos, szStr4, iLength ) || !strncmp ( szPos, szStr5, iLength );
}



static void MetaphAdd ( BYTE * pPrimary, BYTE * pSecondary, const char * szAddPrimary, const char * szAddSecondary )
{
	strcat ( (char*)pPrimary, szAddPrimary ); // NOLINT
	strcat ( (char*)pSecondary, szAddSecondary ); // NOLINT
}

#define ADD_RET(prim,sec,adv)\
{\
	MetaphAdd ( sPrimary, sSecondary, prim, sec );\
	return (adv);\
}

#define ADD(prim,sec)\
	MetaphAdd ( sPrimary, sSecondary, prim, sec )

static int ProcessCode ( int iCode, int iCur, CurrentWord_t & Word, BYTE * sPrimary, BYTE * sSecondary )
{
	BYTE * pWord = Word.pWord;

	// codepoints, not bytes
	int iAdvance = 1;

	// bytes, not codepoints
	int iLast = Word.iLength - 1;

	switch ( iCode )
	{
	case 'A':
	case 'E':
	case 'I':
	case 'O':
	case 'U':
	case 'Y':
		// all init vowels now map to 'A'
		if ( !iCur )
			ADD ( "A", "A" );
		break;

	case 'B':
		// "-mb", e.g", "dumb", already skipped over...
		ADD_RET ( "P", "P", ( pWord[iCur+1]=='B' ) ? 2 : 1 )

	case 0xC7:
	case 0xE7:
		ADD_RET ( "S", "S", 1 )

	case 'C':
		// various germanic
		if ( iCur > 1 && !IsVowel ( pWord[iCur-2] ) && StrAt ( Word, iCur-1, 3, "ACH" )
			&& ( pWord[iCur+2]!='I' && ( pWord[iCur+2]!='E' || StrAt ( Word, iCur-2, 6, "BACHER", "MACHER" ) ) ) )
			ADD_RET ( "K", "K", 2 )

		// special case 'caesar'
		if ( iCur==0 && StrAt ( Word, 0, 6, "CAESAR" ) )
			ADD_RET ( "S", "S", 2 )

		// italian 'chianti'
		if ( StrAt ( Word, iCur, 4, "CHIA" ) )
			ADD_RET ( "K", "K", 2 )

		if ( StrAt ( Word, iCur, 2, "CH" ) )
		{
			// find 'michael'
			if ( iCur > 0 && StrAt ( Word, iCur, 4, "CHAE" ) )
				ADD_RET ( "K", "X", 2 )

			// greek roots e.g. 'chemistry', 'chorus'
			if ( iCur==0
				&& ( StrAt ( Word, iCur+1, 5, "HARAC", "HARIS" )
					|| StrAt ( Word, iCur+1, 3, "HOR", "HYM", "HIA", "HEM" ) )
				&& !StrAt ( Word, 0, 5, "CHORE" ) )
			{
				ADD_RET ( "K", "K", 2 )
			}

			// germanic, greek, or otherwise 'ch' for 'kh' sound
			if ( ( StrAt ( Word, 0, 4, "VAN ", "VON " ) || StrAt ( Word, 0, 3, "SCH" ) )
				// 'architect but not 'arch', 'orchestra', 'orchid'
				|| StrAt ( Word, iCur-2, 6, "ORCHES", "ARCHIT", "ORCHID" )
				|| StrAt ( Word, iCur+2, 1, "T", "S" )
				|| ( ( StrAt ( Word, iCur-1, 1, "A", "O", "U", "E" ) || iCur==0 ) // e.g., 'wachtler', 'wechsler', but not 'tichner'
					&& ( StrAt ( Word, iCur+2, 1, "L", "R", "N", "M" ) || StrAt ( Word, iCur+2, 1, "B", "H", "F", "V" )
						|| StrAt ( Word, iCur+2, 1, "W", " " ) ) ) )
			{
				ADD ( "K", "K" );
			} else
			{
				if ( iCur > 0 )
				{
					if ( StrAt ( Word, 0, 2, "MC" ) ) // e.g., "McHugh"
						ADD ( "K", "K" );
					else
						ADD ( "X", "K" );
				} else
					ADD ( "X", "X" );
			}

			return 2;
		}

		// e.g, 'czerny'
		if ( StrAt ( Word, iCur, 2, "CZ" ) && !StrAt ( Word, iCur-2, 4, "WICZ" ) )
			ADD_RET ( "S", "X", 2 )

		// e.g., 'focaccia'
		if ( StrAt ( Word, iCur+1, 3, "CIA" ) )
			ADD_RET ( "X", "X", 3 )

		// double 'C', but not if e.g. 'McClellan'
		if ( StrAt ( Word, iCur, 2, "CC" ) && !( iCur==1 && pWord[0]=='M' ) )
		{
			// 'bellocchio' but not 'bacchus'
			if ( StrAt ( Word, iCur+2, 1, "I", "E", "H" ) && !StrAt ( Word, iCur+2, 2, "HU" ) )
			{
				// 'accident', 'accede' 'succeed'
				if ( ( iCur==1 && pWord[iCur-1]=='A' ) || StrAt ( Word, iCur-1, 5, "UCCEE", "UCCES" ) )
					ADD_RET ( "KS", "KS", 2 )
				else // 'bacci', 'bertucci', other italian
					ADD_RET ( "X", "X", 2 )
			} else // Pierce's rule
				ADD_RET ( "K", "K", 2 )
		}

		if ( StrAt ( Word, iCur, 2, "CK", "CG", "CQ" ) )
			ADD_RET ( "K", "K", 2 )

		if ( StrAt ( Word, iCur, 2, "CI", "CE", "CY" ) )
		{
			// italian vs. english
			if ( StrAt ( Word, iCur, 3, "CIO", "CIE", "CIA" ) )
				ADD_RET ( "S", "X", 2 )
			else
				ADD_RET ( "S", "S", 2 )
		}

		// else
		ADD ( "K", "K" );

		// name sent in 'mac caffrey', 'mac gregor
		if ( StrAt ( Word, iCur+1, 2, " C", " Q", " G" ) )
			return 3;
		else
		{
			if ( StrAt ( Word, iCur+1, 1, "C", "K", "Q" ) && !StrAt ( Word, iCur+1, 2, "CE", "CI" ) )
				return 2;
		}
		break;

	case 'D':
		if ( StrAt ( Word, iCur, 2, "DG" ) )
		{
			if ( StrAt ( Word, iCur+2, 1, "I", "E", "Y" ) ) // e.g. 'edge'
				ADD_RET	( "J", "J", 3 )
			else // e.g. 'edgar'
				ADD_RET	( "TK", "TK", 2 )
		}

		if ( StrAt ( Word, iCur, 2, "DT", "DD" ) )
			ADD_RET ( "T", "T", 2 )

		// else
		ADD_RET ( "T", "T", 1 )

	case 'F':
		ADD_RET ( "F", "F", pWord[iCur+1]=='F' ? 2 : 1 )

	case 'G':
		if ( pWord[iCur+1]=='H' )
		{
			if ( iCur > 0 && !IsVowel ( pWord[iCur-1] ) )
				ADD_RET ( "K", "K", 2 )

			if ( iCur < 3 )
			{
				// 'ghislane', ghiradelli
				if ( iCur==0 )
				{
					if ( pWord[iCur+2]=='I' )
						ADD_RET ( "J", "J", 2 )
					else
						ADD_RET ( "K", "K", 2 )
				}
			}

			// Parker's rule (with some further refinements) - e.g., 'hugh'
			if ( ( iCur > 1 && StrAt ( Word, iCur-2, 1, "B", "H", "D" ) )
				|| ( iCur > 2 && StrAt ( Word, iCur-3, 1, "B", "H", "D" ) ) // e.g., 'bough'
				|| ( iCur > 3 && StrAt ( Word, iCur-4, 1, "B", "H" ) ) ) // e.g., 'broughton'
				return 2;
			else
			{
				// e.g., 'laugh', 'McLaughlin', 'cough', 'gough', 'rough', 'tough'
				if ( iCur > 2 && pWord[iCur-1]=='U' && StrAt ( Word, iCur-3, 1, "C", "G", "L", "R", "T" ) )
					ADD ( "F", "F" );
				else
					if ( iCur > 0 && pWord[iCur-1]!='I' )
						ADD ( "K", "K" );

				return 2;
			}
		}

		if ( pWord[iCur+1]=='N' )
		{
			if ( iCur==1 && IsVowel ( pWord[0] ) && !SlavoGermanic ( pWord ) )
				ADD ( "KN", "N" );
			else	// not e.g. 'cagney'
				if ( !StrAt ( Word, iCur+2, 2, "EY" ) && pWord[iCur+1]!='Y' && !SlavoGermanic ( pWord ) )
					ADD ( "N", "KN" );
				else
					ADD ( "KN", "KN" );

			return 2;
		}

		// 'tagliaro'
		if ( StrAt ( Word, iCur+1, 2, "LI" ) && !SlavoGermanic ( pWord ) )
			ADD_RET ( "KL", "L", 2 )

		// -ges-,-gep-,-gel-, -gie- at beginning
		if ( iCur==0 && ( pWord[iCur+1]=='Y' || StrAt ( Word, iCur+1, 2, "ES", "EP", "EB", "EL" )
			|| StrAt ( Word, iCur+1, 2, "EY", "IB", "IL", "IN" ) || StrAt ( Word, iCur+1, 2, "IE", "EI", "ER" ) ) )
				ADD_RET ( "K", "J", 2 )

		// -ger-, -gy-
		if ( ( StrAt ( Word, iCur+1, 2, "ER" ) || pWord[iCur+1]=='Y' ) && !StrAt ( Word, 0, 6, "DANGER", "RANGER", "MANGER" )
			&& !StrAt ( Word, iCur-1, 1, "E", "I" ) && !StrAt ( Word, iCur-1, 3, "RGY", "OGY" ) )
			ADD_RET ( "K", "J", 2 )

		// italian e.g, 'biaggi'
		if ( StrAt ( Word, iCur+1, 1, "E", "I", "Y" ) || StrAt ( Word, iCur-1, 4, "AGGI", "OGGI" ) )
		{
			// obvious germanic
			if ( StrAt ( Word, 0, 4, "VAN ", "VON " ) || StrAt ( Word, 0, 3, "SCH" ) || StrAt ( Word, iCur+1, 2, "ET" ) )
				ADD ( "K", "K" );
			else
			{
				// always soft if french ending
				if ( StrAt ( Word, iCur+1, 4, "IER " ) )
					ADD ( "J", "J" );
				else
					ADD ( "J", "K" );
			}

			return 2;
		}

		ADD_RET ( "K", "K", pWord[iCur+1]=='G' ? 2 : 1 )

	case 'H':
		// only keep if first & before vowel or btw. 2 vowels
		if ( ( iCur==0 || IsVowel ( pWord[iCur-1] ) ) && IsVowel ( pWord[iCur+1] ) )
			ADD_RET ( "H", "H", 2 )
		break; // also takes care of 'HH'

	case 'J':
		// obvious spanish, 'jose', 'san jacinto'
		if ( StrAt ( Word, iCur, 4, "JOSE" ) || StrAt ( Word, 0, 4, "SAN " ) )
		{
			if ( ( iCur==0 && pWord[iCur+4]==' ' ) || StrAt ( Word, 0, 4, "SAN " ) )
				ADD_RET ( "H", "H", 1 )
			else
				ADD_RET ( "J", "H", 1 )
		}

		if ( iCur==0 && !StrAt ( Word, iCur, 4, "JOSE" ) )
			ADD ( "J", "A" );	// Yankelovich/Jankelowicz
		else
		{
			// spanish pron. of e.g. 'bajador'
			if ( ( iCur>0 && IsVowel ( pWord[iCur-1] ) )&& !SlavoGermanic ( pWord ) && ( pWord[iCur+1]=='A' || pWord[iCur+1]=='O' ) )
				ADD ( "J", "H" );
			else
			{
				if ( iCur==iLast )
					ADD ( "J", "" );
				else
					if ( !StrAt ( Word, iCur+1, 1, "L", "T", "K", "S" )
						&& !StrAt ( Word, iCur+1, 1, "N", "M", "B", "Z" )
						&& !StrAt ( Word, iCur-1, 1, "S", "K", "L" ) )
					{
						ADD ( "J", "J" );
					}
			}
		}

		if ( pWord[iCur+1]=='J' ) // it could happen!
			return 2;

		break;

	case 'K':
		ADD_RET ( "K", "K", pWord[iCur+1]=='K' ? 2 : 1 )

	case 'L':
		if ( pWord[iCur+1]=='L' )
		{
			// spanish e.g. 'cabrillo', 'gallegos'
			if ( ( iCur==iLast-2 && StrAt ( Word, iCur-1, 4, "ILLO", "ILLA", "ALLE" ) )
				|| ( ( StrAt ( Word, iLast - 1, 2, "AS", "OS" ) || StrAt ( Word, iLast, 1, "A", "O" ) ) && StrAt ( Word, iCur-1, 4, "ALLE" ) ) )
				ADD_RET ( "L", "", 2 )

			iAdvance = 2;
		}

		ADD ( "L", "L" );
		break;

	case 'M':
		ADD ( "M", "M" );

		// 'dumb','thumb'
		if ( ( StrAt ( Word, iCur-1, 3, "UMB" ) && ( iCur+1==iLast || StrAt ( Word, iCur+2, 2, "ER" ) ) ) || pWord[iCur+1]=='M' )
			return 2;

		break;

	case 'N':
		ADD_RET ( "N", "N", pWord[iCur+1]=='N' ? 2 : 1 )

	case 0xD1:
	case 0xF1:
		ADD_RET ( "N", "N", 1 )

	case 'P':
		if ( pWord[iCur+1]=='H' )
			ADD_RET ( "F", "F", 2 )

		// also account for "campbell", "raspberry"
		ADD_RET ( "P", "P", StrAt ( Word, iCur+1, 1, "P", "B" ) ? 2 : 1 )

	case 'Q':
		ADD_RET ( "K", "K", pWord[iCur+1]=='Q' ? 2 : 1 )

	case 'R':
		// french e.g. 'rogier', but exclude 'hochmeier'
		if ( iCur==iLast && !SlavoGermanic ( pWord ) && StrAt ( Word, iCur-2, 2, "IE" ) && !StrAt ( Word, iCur-4, 2, "ME", "MA" ) )
			ADD ( "", "R" );
		else
			ADD ( "R", "R" );

		return pWord[iCur+1]=='R' ? 2 : 1;

	case 'S':
		// special cases 'island', 'isle', 'carlisle', 'carlysle'
		if ( StrAt ( Word, iCur-1, 3, "ISL", "YSL" ) )
			return 1;

		// special case 'sugar-'
		if ( iCur==0 && StrAt ( Word, iCur, 5, "SUGAR" ) )
			ADD_RET ( "X", "S", 1 )

		if ( StrAt ( Word, iCur, 2, "SH" ) )
		{
			// germanic
			if ( StrAt ( Word, iCur+1, 4, "HEIM", "HOEK", "HOLM", "HOLZ" ) )
				ADD_RET ( "S", "S", 2 )
			else
				ADD_RET ( "X", "X", 2 )
		}

		// italian & armenian
		if ( StrAt ( Word, iCur, 3, "SIO", "SIA" ) || StrAt ( Word, iCur, 4, "SIAN" ) )
		{
			if ( !SlavoGermanic ( pWord ) )
				ADD_RET ( "S", "X", 3 )
			else
				ADD_RET ( "S", "S", 3 )
		}

		// german & anglicisations, e.g. 'smith' match 'schmidt', 'snider' match 'schneider'
		// also, -sz- in slavic language altho in hungarian it is pronounced 's'
		if ( ( iCur==0 && StrAt ( Word, iCur+1, 1, "M", "N", "L", "W" ) ) || StrAt ( Word, iCur+1, 1, "Z" ) )
			ADD_RET ( "S", "X", StrAt ( Word, iCur+1, 1, "Z" ) ? 2 : 1 )

		if ( StrAt ( Word, iCur, 2, "SC" ) )
		{
			// Schlesinger's rule
			if ( pWord[iCur+2]=='H' )
			{
				if ( StrAt ( Word, iCur+3, 2, "OO", "ER", "EN", "UY" )
					|| StrAt ( Word, iCur+3, 2, "ED", "EM" ) ) // dutch origin, e.g. 'school', 'schooner'
				{
					// 'schermerhorn', 'schenker'
					if ( StrAt ( Word, iCur+3, 2, "ER", "EN" ) )
						ADD_RET ( "X", "SK", 3 )
					else
						ADD_RET ( "SK", "SK", 3 )
				} else
				{
					if ( iCur==0 && !IsVowel ( pWord[3] ) && pWord[3]!='W' )
						ADD_RET ( "X", "S", 3 )
					else
						ADD_RET ( "X", "X", 3 )
				}
			}

			if ( StrAt ( Word, iCur+2, 1, "I", "E", "Y" ) )
				ADD_RET ( "S", "S", 3 )

			// else
			ADD_RET ( "SK", "SK", 3 )
		}

		// french e.g. 'resnais', 'artois'
		if ( iCur==iLast && StrAt ( Word, iCur-2, 2, "AI", "OI" ) )
			ADD ( "", "S" );
		else
			ADD ( "S", "S" );

		return StrAt ( Word, iCur+1, 1, "S", "Z" ) ? 2 : 1;

	case 'T':
		if ( StrAt ( Word, iCur, 4, "TION" ) )
			ADD_RET ( "X", "X", 3 )

		if ( StrAt ( Word, iCur, 3, "TIA", "TCH" ) )
			ADD_RET ( "X", "X", 3 )

		if ( StrAt ( Word, iCur, 2, "TH" ) || StrAt ( Word, iCur, 3, "TTH" ) )
		{
			// special case 'thomas', 'thames' or germanic
			if ( StrAt ( Word, iCur+2, 2, "OM", "AM" ) || StrAt ( Word, 0, 4, "VAN ", "VON " ) || StrAt ( Word, 0, 3, "SCH" ) )
				ADD_RET ( "T", "T", 2 )
			else
				ADD_RET ( "0", "T", 2 ) // yes, zero
		}

		ADD_RET ( "T", "T", StrAt ( Word, iCur+1, 1, "T", "D" ) ? 2 : 1 )

	case 'V':
		ADD_RET ( "F", "F", pWord[iCur+1]=='V' ? 2 : 1 )

	case 'W':
		// can also be in middle of word
		if ( StrAt ( Word, iCur, 2, "WR" ) )
			ADD_RET ( "R", "R", 2 )

		if ( iCur==0 && ( IsVowel ( pWord[iCur+1] ) || StrAt ( Word, iCur, 2, "WH" ) ) )
		{
			// Wasserman should match Vasserman
			if ( IsVowel ( pWord[iCur+1] ) )
				ADD ( "A", "F" );
			else	// need Uomo to match Womo
				ADD ( "A", "A" );
		}

		// Arnow should match Arnoff
		if ( ( iCur==iLast && iCur > 0 && IsVowel ( pWord[iCur-1] ) ) || StrAt ( Word, iCur-1, 5, "EWSKI", "EWSKY", "OWSKI", "OWSKY" )
			|| StrAt ( Word, 0, 3, "SCH" ) )
			ADD_RET ( "", "F", 1 )

		// polish e.g. 'filipowicz'
		if ( StrAt ( Word, iCur, 4, "WICZ", "WITZ" ) )
			ADD_RET ( "TS", "FX", 4 )
		break;

	case 'X':
		// french e.g. breaux
		if ( !( iCur==iLast && ( StrAt ( Word, iCur-3, 3, "IAU", "EAU" ) || StrAt ( Word, iCur-2, 2, "AU", "OU" ) ) ) )
			ADD ( "KS", "KS" );

		return ( pWord[iCur+1]=='C' || pWord[iCur+1]=='X' ) ? 2 : 1;

	case 'Z':
		// chinese pinyin e.g. 'zhao'
		if ( pWord[iCur+1]=='H' )
			ADD_RET ( "J", "J", 2 )
		else
			if ( StrAt ( Word, iCur+1, 2, "ZO", "ZI", "ZA" ) || ( SlavoGermanic ( pWord ) && ( iCur > 0 && pWord[iCur-1]!='T' ) ) )
				MetaphAdd ( sPrimary, sSecondary, "S", "TS" );
			else
				MetaphAdd ( sPrimary, sSecondary, "S", "S" );

		return pWord[iCur+1]=='Z' ? 2 : 1;
	}

	return iAdvance;
}


void stem_dmetaphone ( BYTE * pWord )
{
	BYTE	sOriginal [3*SPH_MAX_WORD_LEN+3];
	BYTE	sPrimary [3*SPH_MAX_WORD_LEN+3];
	BYTE	sSecondary [ 3*SPH_MAX_WORD_LEN+3 ];
	int		iLength = strlen ( (const char *)pWord );
	memcpy ( sOriginal, pWord, iLength + 1 );
	sPrimary[0] = '\0';
	sSecondary[0] = '\0';

	BYTE * pStart = sOriginal;
	while ( *pStart )
	{
		if ( *pStart>='a' && *pStart<='z' )
			*pStart = (BYTE) toupper ( *pStart );

		++pStart;
	}

	strcat ( (char *) sOriginal, "          " ); // NOLINT

	int iAdvance = 0;

	CurrentWord_t Word;
	Word.pWord = sOriginal;
	Word.iLength = iLength;
	Word.iLengthPadded = strlen ( (const char *)sOriginal );

	// skip these when at start of word
	if ( StrAt ( Word, 0, 2, "GN", "KN", "PN", "WR", "PS" ) )
		iAdvance = 1;

	// Initial 'X' is pronounced 'Z' e.g. 'Xavier'
	if ( sOriginal[0]=='X' )
	{
		ADD ( "S", "S" );	// 'Z' maps to 'S'
		iAdvance = 1;
	}

	const BYTE * pPtr = sOriginal;
	const BYTE * pLastPtr = sOriginal;
	int iCode = -1;

	iCode = sphUTF8Decode ( pPtr );

	while ( iCode!=0 )
	{
		int iCur = pLastPtr-sOriginal;
		if ( iCur>=iLength )
			break;

		for ( int i = 0; i < iAdvance; ++i )
		{
			pLastPtr = pPtr;
			iCode = sphUTF8Decode ( pPtr );
		}

		if ( iCode<=0 )
			break;

		// unknown code: don't copy, just return
		if ( iCode>128 && iCode!=0xC7 && iCode!=0xE7 && iCode!=0xD1 && iCode!=0xF1 )
			return;

		iAdvance = ProcessCode ( iCode, pLastPtr-sOriginal, Word, sPrimary, sSecondary );
	}

	if ( !pWord[0] || sPrimary [0] )
		strcpy ( (char*)pWord, (char*)sPrimary ); // NOLINT

	// TODO: handle secondary too
}

//
// $Id$
//
