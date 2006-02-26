//
// $Id$
//

//
// Copyright (c) 2001-2006, Andrew Aksyonoff. All rights reserved.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "sphinx.h"
#include "sphinxexcerpt.h"

/////////////////////////////////////////////////////////////////////////////
// THE EXCERPTS GENERATOR
/////////////////////////////////////////////////////////////////////////////

class ExcerptGen_c
{
public:
							ExcerptGen_c ();
							~ExcerptGen_c () {}
	char *					BuildExcerpt ( const ExcerptQuery_t & q );

public:
	enum Token_e
	{
		TOK_NONE = 0,
		TOK_WORD,
		TOK_NONWORD,
		TOK_SPACE,
	};

	struct Token_t
	{
		Token_e				m_eType;		///< token type
		int					m_iStart;		///< token start (index in codepoints array)
		int					m_iLength;		///< token length (in codepoints)
	};

	struct Region_t
	{
		int					m_iStart;		///< index of the first token to be shown
		int					m_iMatchStart;	///< index of the first token to be highlighted
		int					m_iMatchEnd;	///< index of the last token to be highlighted
		int					m_iEnd;			///< index of the last token to be shown
	};

protected:
	CSphVector<int,8192>	m_dCodes;		///< original source text codepoints
	CSphVector<int,8192>	m_dFolded;		///< folded source text codepoints

	CSphVector<Token_t,1024>m_dTokens;		///< source text tokens
	CSphVector<Token_t,16>	m_dWords;		///< query words tokens

	Token_t					m_tTok;			///< currently decoded token

	CSphVector<BYTE,16384>	m_dResult;		///< result holder
	int						m_iResultLen;	///< result codepoints count

protected:
	template<int L> void	DecodeUtf8 ( const char * sText, CSphVector<Token_t,L> & dBuf );
	template<int L> void	SubmitCodepoint ( CSphVector<Token_t,L> & dBuf, int iCode );
	Token_e					GetCodepointType ( int iCode );
	int						FoldCodepoint ( int iCode );
	bool					TokensMatch ( const Token_t & a, const Token_t & b);
	void					ResultEmit ( int iCode );
	void					ResultEmit ( const char * sLine );
	void					ResultEmit ( const Token_t & sTok );
};

/////////////////////////////////////////////////////////////////////////////

inline bool operator < ( const ExcerptGen_c::Token_t & a, const ExcerptGen_c::Token_t & b )
{
	if ( a.m_iLength==b.m_iLength )
		return a.m_iStart > b.m_iStart;
	return a.m_iLength < b.m_iLength;
};


inline bool operator < ( const ExcerptGen_c::Region_t & a, const ExcerptGen_c::Region_t & b )
{
	return a.m_iStart < b.m_iStart;
};


ExcerptGen_c::ExcerptGen_c ()
{
	m_tTok.m_eType = TOK_NONE;
	m_tTok.m_iStart = -1;
	m_tTok.m_iLength = -1;
}


char * ExcerptGen_c::BuildExcerpt ( const ExcerptQuery_t & q )
{
	DecodeUtf8 ( q.m_sSource.cstr(), m_dTokens );
	DecodeUtf8 ( q.m_sWords.cstr(), m_dWords );

	ARRAY_FOREACH ( i, m_dWords )
		if ( m_dWords[i].m_eType!=TOK_WORD )
			m_dWords.Remove ( i-- );

	m_dResult.Reset ();
	m_iResultLen = 0;

	if ( q.m_iLimit<=0 || q.m_iLimit>m_dCodes.GetLength() )
	{
		ARRAY_FOREACH ( iTok, m_dTokens )
		{
			bool bMatch = false;
			if ( m_dTokens[iTok].m_eType==TOK_WORD )
				ARRAY_FOREACH ( iWord, m_dWords )
					if ( TokensMatch ( m_dTokens[iTok], m_dWords[iWord] ) )
			{
				bMatch = true;
				break;
			}

			if ( bMatch )
			{
				ResultEmit ( q.m_sBeforeMatch.cstr() );
				ResultEmit ( m_dTokens[iTok] );
				ResultEmit ( q.m_sAfterMatch.cstr() );
			} else
			{
				ResultEmit ( m_dTokens[iTok] );
			}
		}

	} else
	{
		Token_t tLast;
		bool bLastMatch = false;

		tLast.m_eType = TOK_NONE;
		tLast.m_iStart = -1;
		tLast.m_iLength = -1;

		CSphVector<Token_t,128> dMatches;

		ARRAY_FOREACH ( iTok, m_dTokens )
		{
			if ( m_dTokens[iTok].m_eType==TOK_SPACE )
				continue;

			bool bMatch = false;
			if ( m_dTokens[iTok].m_eType==TOK_WORD )
				ARRAY_FOREACH ( iWord, m_dWords )
					if ( TokensMatch ( m_dTokens[iTok], m_dWords[iWord] ) )
			{
				bMatch = true;
				break;
			}

			if ( bMatch )
			{
				if ( bLastMatch )
				{
					tLast.m_iLength++;
				} else
				{
					tLast.m_iStart = iTok;
					tLast.m_iLength = 1;
				}
			} else
			{
				if ( bLastMatch )
					dMatches.Add ( tLast );
			}
			bLastMatch = bMatch;
		}

		dMatches.RSort ();

		if ( dMatches.GetLength() )
		{
			// matches found. we want to show the best ones while we can
			int iLen = 0;
			const int AROUND = 3;
			CSphVector<Region_t,16> dToShow;

			// find out what ones we may show while fitting in the limit (more or less)
			for ( int i=0; iLen<q.m_iLimit && i<dMatches.GetLength(); i++ )
			{
				Region_t tReg;
				tReg.m_iStart = dMatches[i].m_iStart;
				tReg.m_iEnd = dMatches[i].m_iStart;
				tReg.m_iMatchStart = tReg.m_iStart;
				tReg.m_iMatchEnd = tReg.m_iStart;

				// scan back AROUND words
				int iScan = AROUND;
				while ( tReg.m_iStart>0 && iScan>0 )
				{
					if ( m_dTokens [ --tReg.m_iStart ].m_eType==TOK_WORD )
						iScan--;
				}

				// scan forward length-1+AROUND words
				iScan = dMatches[i].m_iLength - 1 + AROUND;
				while ( tReg.m_iEnd<m_dTokens.GetLength()-1 && iScan>0 )
				{
					tReg.m_iEnd++;
					if ( iScan>AROUND )
						tReg.m_iMatchEnd++;
					if ( m_dTokens [ tReg.m_iEnd ].m_eType==TOK_WORD )
						iScan--;
				}

				// calc length
				int iAddLen = 0;
				for ( int iPos=tReg.m_iStart; iPos<=tReg.m_iEnd;iPos++ )
				{
					iAddLen += ( m_dTokens[iPos].m_eType == TOK_SPACE && q.m_bRemoveSpaces )
						? 1
						: m_dTokens[iPos].m_iLength;
				}
				if ( iLen+iAddLen>q.m_iLimit )
					continue;

				// add it
				dToShow.Add ( tReg );
				iLen += iAddLen;
			}

			// do show
			dToShow.Sort ();
			int iLastPos = -1;
			ARRAY_FOREACH ( iToShow, dToShow )
			{
				const Region_t & tReg = dToShow[iToShow];

				// if we're not at the very start of the document,
				// or if we're displaying not the first excerpt chunk,
				// add ellipsis to indicate that
				if ( ( iToShow==0 && tReg.m_iStart>0 ) || ( iToShow>0 && tReg.m_iStart>iLastPos+1 ) )
					ResultEmit ( q.m_sChunkSeparator.cstr() );

				// do display match region
				for ( int iPos=tReg.m_iStart; iPos<=tReg.m_iEnd; iPos++ )
				{
					if ( iPos<=iLastPos )
						continue;

					if ( iPos==tReg.m_iMatchStart )
							ResultEmit ( q.m_sBeforeMatch.cstr() );

					if ( m_dTokens[iPos].m_eType == TOK_SPACE && q.m_bRemoveSpaces )
						ResultEmit ( 0x20 );
					else
						ResultEmit ( m_dTokens[iPos] );

					if ( iPos==tReg.m_iMatchEnd )
						ResultEmit ( q.m_sAfterMatch.cstr() );
				}
				iLastPos = tReg.m_iEnd;

				// if we're at the last match, but not the very end of the document, add ellipsis
				if ( iToShow==dToShow.GetLength()-1 && tReg.m_iEnd<m_dTokens.GetLength()-1 )
					ResultEmit ( q.m_sChunkSeparator.cstr() );

			}

		} else
		{
			// no matches found. just show the starting tokens
			int i = 0;
			while ( m_iResultLen+m_dTokens[i].m_iLength < q.m_iLimit )
				ResultEmit ( m_dTokens[i++] );
		}
	}

	// finalize
	m_dCodes.Reset ();
	m_dFolded.Reset ();
	m_dTokens.Reset ();
	m_dWords.Reset ();

	m_dResult.Add ( 0 );
	char * pRes = new char [ m_dResult.GetLength() ];
	memcpy ( pRes, &m_dResult[0], m_dResult.GetLength() );
	m_dResult.Reset ();

	return pRes;
}


template<int L> void ExcerptGen_c::DecodeUtf8 ( const char * sText, CSphVector<Token_t,L> & dBuf )
{
	BYTE * pCur = (BYTE*) sText;
	while ( *pCur )
	{
		BYTE v = *pCur++;

		if ( v<128 )
		{
			SubmitCodepoint ( dBuf, v );
			continue;
		}

		// get number of bytes
		int iBytes = 0;
		while ( v & 0x80 )
		{
			iBytes++;
			v <<= 1;
		}

		// check for valid number of bytes
		if ( iBytes<2 || iBytes>4 )
			continue;

		int iCode = ( v>>iBytes );
		iBytes--;
		do
		{
			if ( !(*pCur) )
			{
				SubmitCodepoint ( dBuf, 0 );
				return; // EOF
			}
			if ( ((*pCur) & 0xC0)!=0x80 )
				break; // broken code

			iCode = ( iCode<<6 ) + ( (*pCur) & 0x3F );
			iBytes--;
		} while ( iBytes );

		// return code point if there were no errors
		// ignore and continue scanning otherwise
		if ( !iBytes )
			SubmitCodepoint ( dBuf, iCode );
	}
	SubmitCodepoint ( dBuf, 0 );
}


template<int L> void ExcerptGen_c::SubmitCodepoint ( CSphVector<Token_t,L> & dBuf, int iCode )
{
	// find out its type
	Token_e eType = GetCodepointType ( iCode );

	// add the codepoint
	int iPos = m_dCodes.GetLength ();
	m_dCodes.Add ( iCode );
	m_dFolded.Add ( FoldCodepoint ( iCode ) );

	// do tokenizing
	if ( m_tTok.m_eType==eType )
	{
		// type did not change, continue accumulating
		m_tTok.m_iLength++;

	} else
	{
		// type changed, do flush last one
		if ( m_tTok.m_eType!=TOK_NONE )
			dBuf.Add ( m_tTok );

		m_tTok.m_eType = eType;
		m_tTok.m_iStart = iPos;
		m_tTok.m_iLength = 1;

		// emit terminating token
		if ( eType==TOK_NONE )
			dBuf.Add ( m_tTok );
	}
}


ExcerptGen_c::Token_e ExcerptGen_c::GetCodepointType ( int iCode )
{
	//!COMMIT
	if ( !iCode ) return TOK_NONE;
	if ( iCode>=256 ) return TOK_NONWORD;
	if ( isspace(iCode) ) return TOK_SPACE;
	if ( iCode>='a' && iCode<='z' ) return TOK_WORD;
	if ( iCode>='A' && iCode<='Z' ) return TOK_WORD;
	return TOK_NONWORD;
}


int ExcerptGen_c::FoldCodepoint ( int iCode )
{
	//!COMMIT
	if ( iCode>='A' && iCode<='Z' ) return iCode-'A'+'a';
	return iCode;
}


bool ExcerptGen_c::TokensMatch ( const Token_t & a, const Token_t & b )
{
	if ( a.m_iLength!=b.m_iLength )
		return false;

	for ( int i=0; i<a.m_iLength; i++ )
		if ( m_dFolded [ a.m_iStart+i ] != m_dFolded [ b.m_iStart+i] )
			return false;

	return true;
}


void ExcerptGen_c::ResultEmit ( int iCode )
{
	if ( iCode<=0x7f )
	{
		m_dResult.Add ( BYTE(iCode) );

	} else if ( iCode<=0x7ff )
	{
		m_dResult.Add ( 0xc0 | BYTE( iCode>>6) );
		m_dResult.Add ( 0x80 | BYTE( iCode&0x3f ) );

	} else if ( iCode<=0xffff )
	{
		m_dResult.Add ( 0xe0 | BYTE( iCode>>12) );
		m_dResult.Add ( 0x80 | BYTE( (iCode>>6)&0x3f ) );
		m_dResult.Add ( 0x80 | BYTE( iCode&0x3f ) );

	} else
	{
		m_dResult.Add ( 0xf0 | BYTE( iCode>>18) );
		m_dResult.Add ( 0x80 | BYTE( (iCode>>12)&0x3f ) );
		m_dResult.Add ( 0x80 | BYTE( (iCode>>6)&0x3f ) );
		m_dResult.Add ( 0x80 | BYTE( iCode&0x3f ) );
	}
	m_iResultLen++;
}


void ExcerptGen_c::ResultEmit ( const char * sLine )
{
	while ( *sLine )
	{
		assert ( (*(BYTE*)sLine)<128 );
		ResultEmit ( (int)*sLine++ );
	}
}


void ExcerptGen_c::ResultEmit ( const Token_t & sTok )
{
	for ( int i=0; i<sTok.m_iLength; i++ )
		ResultEmit ( m_dCodes [ i+sTok.m_iStart ] );
}

/////////////////////////////////////////////////////////////////////////////

char * sphBuildExcerpt ( const ExcerptQuery_t & q )
{
	ExcerptGen_c tGen;
	return tGen.BuildExcerpt ( q );
}

//
// $Id$
//
