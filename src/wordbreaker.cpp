//
// $Id$
//

//
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#define PREFER_ALLDICT			0
#define UNKNOWN_WORD_COEFF		3.0f
#define DICT_THRESH				12
#define DEFAULT_DICT			"wordbreaker-dict.txt"

#define DICT_COMPOUND_MIN		6			// keywords longer than this are potential compounds
#define DICT_COMPOUND_THRESH	0.0001f		// penalize a no-split with a rare potential compound (that occurs in less than this percentage)
#define DICT_COMPOUND_COEFF		1.53f		// penalization factor

#include "sphinxstd.h"
#include <math.h>

/// my own isalpha (let's build our own theme park!)
inline int IsAlpha ( int c )
{
	return ( c>='a' && c<='z' );
}

// copied over from sphinxutils; remove at some point
void StrSplit ( CSphVector<CSphString> & dOut, const char * sIn )
{
	if ( !sIn )
		return;

	const char * p = (char*)sIn;
	while ( *p )
	{
		// skip non-alphas
		while ( (*p) && !IsAlpha(*p) )
			p++;
		if ( !(*p) )
			break;

		// this is my next token
		assert ( IsAlpha(*p) );
		const char * sNext = p;
		while ( IsAlpha(*p) )
			p++;
		if ( sNext!=p )
			dOut.Add().SetBinary ( sNext, p-sNext );
	}
}


/// probability descriptor
struct Prob_t
{
	float	m_fProb;	///< log(P(w)) for dict keywords, 0 otherwise (note that P(w)<0 at all times)
	bool	m_bDict;	///< false dor dict keywords, true otherwise

	/// construct a non-dict keyword as default
	Prob_t()
		: m_fProb ( 0.0f )
		, m_bDict ( false )
	{}
};

/// stupid easy unigram model
class LangModel_c
{
private:
	typedef CSphOrderedHash < float, CSphString, CSphStrHashFunc, 1048576 > HashType_t;

	HashType_t *	m_pHash;
	int				m_iTotal;
	float			m_fLogTotal;

public:
	float			m_fLogMin;

	LangModel_c()
	{
		m_pHash = new HashType_t();
		m_fLogMin = logf ( (float)DICT_THRESH );
	}

	~LangModel_c()
	{
		SafeDelete ( m_pHash );
	}

	void SetTotal ( int iTotal )
	{
		m_iTotal = iTotal;
		m_fLogTotal = logf ( (float)iTotal );
	}

	void AddWord ( const char * sWord, int iFreq )
	{
		// we use 1 as a special case
		assert ( iFreq>1 );

		CSphString sVal ( sWord );
		m_pHash->Add ( logf((float)iFreq), sVal );
	}

	Prob_t GetProb ( const char * sKey, int iLen ) const
	{
		Prob_t r;
		CSphString sKey2;
		sKey2.SetBinary ( sKey, iLen );

		// found? P(w) = log(freq/total) = log(freq) - log(total)
		float * pVal = (*m_pHash)(sKey2);
		if ( pVal && *pVal >= m_fLogMin )
		{
			r.m_fProb = *pVal - m_fLogTotal;
			r.m_bDict = true;
		} else
		{
#if 1
			// not found? lets do some tricks
			// we wanna penalize non-dict "words" compared to dict words
			// we wanna penalize super-long "words" compared to shorter options
			// we DO NOT wanna overpenalize (dict + junkN) combo vs (junkM) though
			if ( iLen>20 )
				iLen = 20;
			static float k[21] =
			{
				9.9f, 7.5f, 4.2f, 2.1f, 1.5f, // 0, 1, 2, 3, 4,
				1.2f, 1.1f, 1.0f, 1.1f, 1.2f, // 5, 6, 7, 8, 9,
				1.5f, 1.8f, 2.3f, 2.7f, 3.2f, // 10, 11, 12, 13, 14
				3.6f, 4.0f, 4.4f, 4.8f, 5.3f, // 15, 16, 17, 18, 19
				9.9f // 20
			};
			r.m_fProb = -m_fLogTotal * ( 0.5f + k[iLen] );
#else
			r.m_fProb = -m_fLogTotal * UNKNOWN_WORD_COEFF;
#endif
			r.m_bDict = false;
		}
		return r;
	}

	float GetLogTotal() const
	{
		return -m_fLogTotal;
	}
};


/// compound meta-probability
/// store sum_{w \in words} ( log(P(w)) ) as m_fProb
/// store and_{w \in words} ( m_bDict ) as m_bDict
struct Split_t
{
	CSphVector<int>		m_Pos;			///< split positions
	float				m_fProb;
	bool				m_bAllDict;
	bool				m_bAnyDict;

	Split_t()
		: m_fProb ( 0.0f )
		, m_bAllDict ( false )
		, m_bAnyDict ( false )
	{}

	void Dump ( const char * sWord, const char * sHead=NULL )
	{
		if ( sHead )
			printf ( "%s: ", sHead );
		int iCur = 0;
		ARRAY_FOREACH ( j, m_Pos )
		{
			while ( iCur<m_Pos[j] )
				printf ( "%c", sWord[iCur++] );
			printf ( "|" );
		}
		while ( sWord[iCur] )
			printf ( "%c", sWord[iCur++] );
		printf ( ", %f", m_fProb );
		if ( m_bAllDict )
			printf ( " (all-dict)" );
		else if ( m_bAnyDict )
			printf ( " (some-dict)" );
		printf ( "\n" );
	}

	void AddSplitPos ( const Prob_t & p, int iPos )
	{
		// if current split is empty, new one just takes over
		if ( !m_Pos.GetLength() )
		{
			m_Pos.Add ( iPos );
			m_fProb = p.m_fProb;
			m_bAllDict = p.m_bDict;
			m_bAnyDict = p.m_bDict;
			return;
		}

		// if we have data, we combine it
		assert ( m_Pos.Last() < iPos );
		m_Pos.Add ( iPos );
		m_fProb += p.m_fProb;
		m_bAllDict &= p.m_bDict;
		m_bAnyDict |= p.m_bDict;
	}

	bool operator < ( const Split_t & rhs ) const
	{
#if PREFER_ALLDICT
		if ( m_bAllDict!=rhs.m_bAllDict )
			return m_bAllDict < rhs.m_bAllDict;
#endif
		// do not (!) check anydict flag on an initial empty split
		if ( m_Pos.GetLength() )
			if ( m_bAnyDict!=rhs.m_bAnyDict )
				return m_bAnyDict < rhs.m_bAnyDict;
		return m_fProb < rhs.m_fProb;
	}
};


static LangModel_c g_LM;

void UrlBreakInit ( const char * sDict, bool bVerbose )
{
	FILE * fp = fopen ( sDict, "rb" );
	if ( !fp )
		sphDie ( "failed to open %s", sDict );

	int iEntries = 0, iSumFreqs = 0;
	char sBuf[512];
	sBuf[0] = 0;
	while ( fgets ( sBuf, sizeof(sBuf), fp ) )
	{
		// extract keyword
		char * p = sBuf;
		while ( *p && *p!=' ' )
			p++;
		if ( *p!=' ' )
			sphDie ( "bad freqdict line: no space" );
		*p++ = 0;

		// extract freq
		int iFreq = atoi(p);
		if ( !iFreq )
			sphDie ( "bad freqdict line: zero freq" );
		iSumFreqs += iFreq;

#if 0
		// only keep frequent-enough words
		if ( iFreq < MIN_FREQ )
			continue;
#endif

		// only keep all-latin words
		bool bLatin = true;
		char * s = sBuf;
		while ( *s )
		{
			if ( !( *s>='a' && *s<='z' ) && !( *s>='0' && *s<='9' ) )
			{
				bLatin = false;
				break;
			}
			s++;
		}
		if ( !bLatin )
			continue;

		// hash it
		g_LM.AddWord ( sBuf, iFreq );
		iEntries++;
	}

	fclose ( fp );
	g_LM.SetTotal ( iSumFreqs );
	if ( bVerbose )
		printf ( "kept %d entries, total %d hits\n", iEntries, iSumFreqs );
}


void UrlBreak ( Split_t & tBest, const char * sWord )
{
	const int iLen = strlen(sWord);

	tBest.m_Pos.Resize(0);

	// current partial splits
	// begin with an empty one
	CSphVector<Split_t> dSplits;
	dSplits.Add();

	// our best guess so far
	// begin with a trivial baseline one (ie. no splits at all)
	Prob_t p = g_LM.GetProb ( sWord, iLen );
	tBest.m_Pos.Add ( iLen );
	tBest.m_fProb = p.m_fProb;
	tBest.m_bAllDict = tBest.m_bAnyDict = p.m_bDict;

	if ( iLen>=DICT_COMPOUND_MIN && tBest.m_bAllDict )
	{
		static const float THRESH = logf ( DICT_COMPOUND_THRESH );
		if ( tBest.m_fProb<=THRESH )
			tBest.m_fProb *= DICT_COMPOUND_COEFF;
	}

	// work the current splits
	CSphVector<Split_t> dSplits2;
	while ( dSplits.GetLength() )
	{
		int iWorkedSplits = 0;
		float fPrevBest = tBest.m_fProb;

		ARRAY_FOREACH ( iSplit, dSplits )
		{
			Split_t & s = dSplits[iSplit];

			// filter out splits that were added before (!) a new best guess on the previous iteration
			if ( dSplits[iSplit] < tBest )
				continue;
			iWorkedSplits++;

			int iLast = 0;
			if ( s.m_Pos.GetLength() )
				iLast = s.m_Pos.Last();

			for ( int i=1+iLast; i<iLen; i++ )
			{
				// consider a split at position i
				// it generates a word candidate [iLast,i) and a tail [i,iLen)
				// let's score those
				Prob_t tCand = g_LM.GetProb ( sWord+iLast, i-iLast );
				Prob_t tTail = g_LM.GetProb ( sWord+i, iLen-i );

				// if the current best is all-keywords, the new candidates must be, too
				if ( tBest.m_bAllDict && !tCand.m_bDict )
					continue;

				// compute partial and full split candidates generated by the current guess
				Split_t tPartial = s;
				tPartial.AddSplitPos ( tCand, i );

				Split_t tFull = tPartial;
				tFull.AddSplitPos ( tTail, iLen );

				// check if the full one is our new best full one
				if ( tBest < tFull )
				{
					// FIXME? we do this even when the new split is *not* all-keywords,
					// but the old best split was; is this ever a problem?
					tBest = tFull;
//					tBest.Dump ( sWord, "new-best" );
				}

				// check if the resulting partial split is worth scanning further
				if ( tBest < tPartial )
				{
					dSplits2.Add ( tPartial );
//					dSplits2.Last().Dump ( sWord, "scan-partial" );
				}
			}
		}

		// damage control!
		// if we just processed over 100K candidate splits and got no improvement
		// lets assume that our chances of getting one are kinda low and bail
		if ( iWorkedSplits>=100000 && tBest.m_fProb>=fPrevBest )
			break;

		// keep going
		dSplits.SwapData ( dSplits2 );
		dSplits2.Resize ( 0 );
	}
}


char * Strip ( char * sBuf )
{
	char * p = sBuf;
	while ( *p && isspace(*p) )
		p++;
	char * e = p + strlen(p) - 1;
	while ( e>=p && *e && isspace(*e) )
		*e-- = '\0';

	return p;
}


void UrlBreakTest ( const char * sTestFile )
{
	// load the test data
	CSphVector < CSphVector<CSphString> > dTests;

	FILE * fp = fopen ( sTestFile, "rb" );
	if ( !fp )
		sphDie ( "failed to open %s", sTestFile );

	char sBuf[1024];
	while ( fgets ( sBuf, sizeof(sBuf), fp ) )
	{
		// strip spaces
		char * p = Strip ( sBuf );

		// ignore empty lines
		if ( !*p )
			continue;

		// ignore comments
		if ( p[0]=='/' && p[1]=='/' )
			continue;

		// parse!
		CSphVector<CSphString> & dTest = dTests.Add();
		StrSplit ( dTest, p );
		if ( dTest.GetLength()<2 )
		{
			if ( dTest.GetLength()==1 )
				printf ( "WARNING: no substrings defined for base %s\n", dTest[0].cstr() );
			dTests.Pop();
		}
	}

	fclose ( fp );

	// self check
	int iNosplit = 0;
	ARRAY_FOREACH ( i, dTests )
	{
		if ( dTests[i][1]==dTests[i][0] )
		{
//			printf ( "WARNING: base %s is the only substring\n", dTests[i][0].cstr() );
			iNosplit++;
		}

		for ( int j=1; j<dTests[i].GetLength(); j++ )
			if ( !strstr ( dTests[i][0].cstr(), dTests[i][j].cstr() ) )
				printf ( "WARNING: substring %s not found in base %s\n", dTests[i][j].cstr(), dTests[i][0].cstr() );
	}
	if ( iNosplit )
		printf ( "total %d nosplits, %.3f of the test suite\n",
			iNosplit, float(iNosplit)/dTests.GetLength() );

	int iTotal = 0;
	int iGood = 0;
	int64_t tmWall = sphMicroTimer();
	ARRAY_FOREACH ( iTest, dTests )
	{
#ifndef NDEBUG
		int64_t tmWord = sphMicroTimer();
#endif
		const char * sWord = dTests[iTest][0].cstr();

		// break into keywords
		Split_t tBest;
		UrlBreak ( tBest, sWord );

		// generate actual strings
		int iCur = 0;
		CSphVector<CSphString> dWords;
		ARRAY_FOREACH ( i, tBest.m_Pos )
		{
			dWords.Add().SetBinary ( sWord+iCur, tBest.m_Pos[i]-iCur );
			iCur = tBest.m_Pos[i];
		}
		if ( sWord[iCur] )
			dWords.Add ( sWord+iCur );

		// check them and compute precision
		bool bGood = true;
		for ( int j=1; j<dTests[iTest].GetLength() && bGood; j++ )
			if ( !dWords.Contains ( dTests[iTest][j] ) )
				bGood = false;

		iTotal++;
		if ( bGood )
			iGood++;

		// debug dump
#ifndef NDEBUG
		if ( !bGood )
		{
			printf ( "%d msec, %s => ", (int)( ( sphMicroTimer() - tmWord )/1000 ), sWord );
			tBest.Dump ( sWord );
		}
#endif
	}

	// results
	printf ( "prec %.3f, wall %d msec, %d good, %d total\n",
		float(iGood)/iTotal, (int)( ( sphMicroTimer() - tmWall )/1000 ), iGood, iTotal );
	printf ( "prec %.3f, %d total w/o nosplits\n", float(iGood)/(iTotal-iNosplit), iTotal-iNosplit );
}


bool UrlBreakIsChar ( int c )
{
	return ( c>='a' && c<='z' );
}


void UrlBreakBench ( const char * sBenchFile )
{
	int64_t tmWall = sphMicroTimer();

	const int MIN_BREAK = 5;

	FILE * fp = fopen ( sBenchFile, "rb" );
	if ( !fp )
		sphDie ( "failed to open %s", sBenchFile );

	Split_t tBest;
	char sBuf[512];
	while ( fgets ( sBuf, sizeof(sBuf), fp ) )
	{
		char * p = sBuf;
		while ( *p )
		{
			while ( *p && !UrlBreakIsChar(*p) )
				p++;
			if ( !*p )
				break;

			char * sUrl = p;
			while ( UrlBreakIsChar(*p) )
				p++;
			if ( p-sUrl < MIN_BREAK )
				continue;
			if ( *p )
				*p++ = '\0';

#if 0
			int64_t tmWord = sphMicroTimer();
#endif
			UrlBreak ( tBest, sUrl );

#if 0
			if ( !tBest.m_bAllDict )
			{
				printf ( "%d usec, %s => ", (int)( (sphMicroTimer()-tmWord) ), sUrl );
				tBest.Dump ( sUrl );
			}
#endif
		}
	}

	fclose ( fp );

	tmWall = sphMicroTimer() - tmWall;
	printf ( "%d msec\n", (int)(tmWall/1000) );
}


void UrlBreakSplit ()
{
	char sBuf[1024];
	char sSpace[] = " ";
	while ( !feof ( stdin ) )
	{
		// read next one
		if ( !fgets ( sBuf, sizeof(sBuf), stdin ) )
			continue;

		char * pMax = sBuf + strlen(sBuf);
		char * p = sBuf;

		while ( p<pMax )
		{
			while ( p<pMax && !IsAlpha(*p) )
				p++;
			if ( p>=pMax )
				break;

			char * sWord = p;
			while ( p<pMax && IsAlpha(*p) )
				p++;
			*p = '\0';

			// break this token into keywords
			Split_t tBest;
			UrlBreak ( tBest, sWord );

			// generate actual strings
			int iCur = 0;
			CSphVector<CSphString> dWords;
			ARRAY_FOREACH ( i, tBest.m_Pos )
			{
				fwrite ( sWord+iCur, 1, tBest.m_Pos[i]-iCur, stdout );
				fwrite ( sSpace, 1, 1, stdout );
				iCur = tBest.m_Pos[i];
			}
			if ( sWord[iCur] )
				printf ( "%s ", sWord+iCur );
		}

		// all done
		printf ( "\n" );
		fflush ( stdout );
	}
}


int main ( int argc, char ** argv )
{
	const char * sDict = DEFAULT_DICT;

	if ( argc<2 )
	{
		printf (
			"wordbreaker, a tool to split compounds (eg URL parts) into individual words\n"
			"\n"
			"Usage: wordbreaker <COMMAND> [OPTIONS]\n"
			"\n"
			"Commands are:\n"
			"test <TESTFILE>\t\tdo a splitting precision test on TESTFILE\n"
			"bench <TESTFILE>\tdo a splitting performance benchmark on TESTFILE\n"
			"split\t\t\tdo splitting (read from stdin, split, print to stdout)\n"
			"\n"
			"Options are:\n"
			"--dict <FILENAME>\tuse FILENAME as a frequency dictionary\n"
			"\t\t\t(default is " DEFAULT_DICT ")\n"
		);
		return 0;
	}

	enum
	{
		CMD_NONE,
		CMD_TEST,
		CMD_BENCH,
		CMD_SPLIT
	} eCommand = CMD_NONE;
	const char * sFile = "";

	for ( int i=1; i<argc; i++ )
	{
		if ( !strcmp ( argv[i], "test") )
		{
			if ( eCommand!=CMD_NONE )
				sphDie ( "you must specify exactly one command" );
			if ( ++i>=argc )
				sphDie ( "test requires an argument" );

			eCommand = CMD_TEST;
			sFile = argv[i];

		} else if ( !strcmp ( argv[i], "bench") )
		{
			if ( eCommand!=CMD_NONE )
				sphDie ( "you must specify exactly one command" );
			if ( ++i>=argc )
				sphDie ( "bench requires an argument" );

			eCommand = CMD_BENCH;
			sFile = argv[i];

		} else if ( !strcmp ( argv[i], "split") )
		{
			if ( eCommand!=CMD_NONE )
				sphDie ( "you must specify exactly one command" );

			eCommand = CMD_SPLIT;

		} else if ( !strcmp ( argv[i], "--dict") )
		{
			if ( ++i>=argc )
				sphDie ( "--dict requires an argument" );

			sDict = argv[i];

		} else
		{
			sphDie ( "unknown switch: %s", argv[i] );
		}
	}

	switch ( eCommand )
	{
		case CMD_NONE:
			sphDie ( "no command given" );
		case CMD_TEST:
			UrlBreakInit ( sDict, true );
			UrlBreakTest ( sFile );
			break;
		case CMD_BENCH:
			UrlBreakInit ( sDict, true );
			UrlBreakBench ( sFile );
			break;
		case CMD_SPLIT:
			UrlBreakInit ( sDict, false );
			UrlBreakSplit ();
			break;
	}
	return 0;
}

//
// $Id$
//
