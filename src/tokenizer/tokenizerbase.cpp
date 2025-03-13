//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "tokenizerbase_impl.h"

#include "lowercaser_impl.h"
#include "sphinxdefs.h"
#include "sphinxint.h"
#include "exceptions_trie.h"
#include "fileio.h"

CSphTokenizerBase::CSphTokenizerBase()
{
	m_pAccum = m_sAccum;
}

CSphTokenizerBase::~CSphTokenizerBase()
{
	SafeDelete ( m_pExc );
}


bool CSphTokenizerBase::SetCaseFolding ( const char* sConfig, CSphString& sError )
{
	if ( m_pExc )
	{
		sError = "SetCaseFolding() must not be called after LoadSynonyms()";
		return false;
	}
	m_bHasBlend = false;
	return ISphTokenizer::SetCaseFolding ( sConfig, sError );
}


bool CSphTokenizerBase::SetBlendChars ( const char* sConfig, CSphString& sError )
{
	m_bHasBlend = ISphTokenizer::SetBlendChars ( sConfig, sError );
	return m_bHasBlend;
}


bool CSphTokenizerBase::LoadSynonyms ( const char* sFilename, const CSphEmbeddedFiles* pFiles, StrVec_t& dWarnings, CSphString& sError )
{
	CSphString sWarning;
	ExceptionsTrieGen_c g;
	if ( pFiles )
	{
		m_tSynFileInfo = pFiles->m_tSynonymFile;
		ARRAY_FOREACH ( i, pFiles->m_dSynonyms )
		{
			if ( !g.ParseLine ( const_cast<char*> ( pFiles->m_dSynonyms[i].cstr() ), sWarning ) )
			{
				sWarning.SetSprintf ( "%s line %d: %s", pFiles->m_tSynonymFile.m_sFilename.cstr(), i, sWarning.cstr() );
				dWarnings.Add ( sWarning );
				sphWarning ( "%s", sWarning.cstr() );
			}
		}
	} else
	{
		if ( !sFilename || !*sFilename )
			return true;

		m_tSynFileInfo.Collect ( sFilename );

		CSphAutoreader tReader;
		if ( !tReader.Open ( sFilename, sError ) )
			return false;

		char sBuffer[1024];
		int iLine = 0;
		while ( tReader.GetLine ( sBuffer, sizeof ( sBuffer ) ) >= 0 )
		{
			iLine++;
			if ( !g.ParseLine ( sBuffer, sWarning ) )
			{
				sWarning.SetSprintf ( "%s line %d: %s", sFilename, iLine, sWarning.cstr() );
				dWarnings.Add ( sWarning );
				sphWarning ( "%s", sWarning.cstr() );
			}
		}
	}

	m_pExc = g.Build();
	return true;
}


void CSphTokenizerBase::WriteSynonyms ( Writer_i & tWriter ) const
{
	if ( m_pExc )
		m_pExc->Export ( tWriter );
	else
		tWriter.PutDword ( 0 );
}

void CSphTokenizerBase::WriteSynonyms ( JsonEscapedBuilder & tOut ) const
{
	if ( !m_pExc )
		return;

	tOut.Named ( "synonyms" );
	auto _ = tOut.ArrayW();
	m_pExc->Export ( tOut );
}

void CSphTokenizerBase::CloneBase ( const CSphTokenizerBase* pFrom, ESphTokenizerClone eMode )
{
	m_eMode = eMode;
	m_pExc = nullptr;
	if ( pFrom->m_pExc )
	{
		m_pExc = new ExceptionsTrie_c;
		*m_pExc = *pFrom->m_pExc;
	}
	m_tSettings = pFrom->m_tSettings;
	m_bHasBlend = pFrom->m_bHasBlend;
	m_uBlendVariants = pFrom->m_uBlendVariants;
	m_bBlendSkipPure = pFrom->m_bBlendSkipPure;
	m_bShortTokenFilter = ( m_eMode != SPH_CLONE_INDEX );
	m_bDetectSentences = pFrom->m_bDetectSentences;

	// By default, we operate with read-only refcounted pointer to prepared lowercaser.
	// Any changing operation uses special write-enabled pointer, which is null by default, and also forcibly reset to null if we clone the pointer, in order to protect clone from changes of parent.
	// So, 'just clone' for querying is ok. Clone and add some additional symbols = full clone.

	if ( eMode == pFrom->m_eMode || eMode == SPH_CLONE )
	{
		SetLC ( pFrom->GetLC() );
		m_eMode = pFrom->m_eMode;
		return;
	}

	// assume clones are not compatible between each other. So, clone any kind of query possibly either from the same, either from index, but not from another kind of query.
	assert ( pFrom->m_eMode == SPH_CLONE_INDEX );

	if ( eMode != SPH_CLONE_INDEX )
		m_uBlendVariants = BLEND_TRIM_NONE;

	switch ( eMode )
	{
	case SPH_CLONE_QUERY_WILD_EXACT_JSON:
		SetLC ( pFrom->GetLC()->GetQueryWildExactJsonLC() );
		break;
	case SPH_CLONE_QUERY_WILD_EXACT:
		SetLC ( pFrom->GetLC()->GetQueryWildExactLC() );
		break;
	case SPH_CLONE_QUERY_WILD_JSON:
		SetLC ( pFrom->GetLC()->GetQueryWildJsonLC() );
		break;
	case SPH_CLONE_QUERY_WILD:
		SetLC ( pFrom->GetLC()->GetQueryWildLC() );
		break;
	case SPH_CLONE_QUERY_EXACT_JSON:
		SetLC ( pFrom->GetLC()->GetQueryExactJsonLC() );
		break;
	case SPH_CLONE_QUERY_EXACT:
		SetLC ( pFrom->GetLC()->GetQueryExactLC() );
		break;
	case SPH_CLONE_QUERY_:
		SetLC ( pFrom->GetLC()->GetQuery_LC() );
		break;
	case SPH_CLONE_QUERY:
		SetLC ( pFrom->GetLC()->GetQueryLC() );
		break;
	case SPH_CLONE_INDEX:
	default:
		SetLC ( pFrom->GetLC() );
	}
}

uint64_t CSphTokenizerBase::GetSettingsFNV() const noexcept
{
	uint64_t uHash = ISphTokenizer::GetSettingsFNV();

	DWORD uFlags = 0;
	if ( m_bHasBlend )
		uFlags |= 1 << 0;
	uHash = sphFNV64 ( &uFlags, sizeof ( uFlags ), uHash );

	return uHash;
}


void CSphTokenizerBase::SetBufferPtr ( const char* sNewPtr )
{
	assert ( (const BYTE*)sNewPtr >= m_pBuffer && (const BYTE*)sNewPtr <= m_pBufferMax );
	m_pCur = Min ( m_pBufferMax, Max ( m_pBuffer, (const BYTE*)sNewPtr ) );
	m_iAccum = 0;
	m_pAccum = m_sAccum;
	m_pTokenStart = m_pTokenEnd = nullptr;
	m_pBlendStart = m_pBlendEnd = nullptr;
}

/// adjusts blending magic when we're about to return a token (any token)
/// returns false if current token should be skipped, true otherwise
bool CSphTokenizerBase::BlendAdjust ( const BYTE* pCur )
{
	// check if all we got is a bunch of blended characters (pure-blended case)
	if ( m_bBlended && !m_bNonBlended )
	{
		// we either skip this token, or pretend it was normal
		// in both cases, clear the flag
		m_bBlended = false;

		// do we need to skip it?
		if ( m_bBlendSkipPure )
		{
			m_pBlendStart = NULL;
			return false;
		}
	}
	m_bNonBlended = false;

	// adjust buffer pointers
	if ( m_bBlended && m_pBlendStart )
	{
		// called once per blended token, on processing start
		// at this point, full blended token is in the accumulator
		// and we're about to return it
		m_pCur = m_pBlendStart;
		m_pBlendEnd = pCur;
		m_pBlendStart = nullptr;
		m_bBlendedPart = true;
	} else if ( pCur >= m_pBlendEnd )
	{
		// tricky bit, as at this point, token we're about to return
		// can either be a blended subtoken, or the next one
		m_bBlendedPart = m_pTokenStart && ( m_pTokenStart < m_pBlendEnd );
		m_pBlendEnd = nullptr;
		m_pBlendStart = nullptr;
	} else if ( !m_pBlendEnd )
	{
		// we aren't re-parsing blended; so clear the "blended subtoken" flag
		m_bBlendedPart = false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

static inline bool IsCapital ( int iCh )
{
	return iCh >= 'A' && iCh <= 'Z';
}

static inline bool IsWhitespace ( BYTE c )
{
	return ( c == '\0' || c == ' ' || c == '\t' || c == '\r' || c == '\n' );
}

static inline bool IsBoundary ( BYTE c, bool bPhrase )
{
	// FIXME? sorta intersects with specials
	// then again, a shortened-down list (more strict syntax) is reasonble here too
	return IsWhitespace ( c ) || c == '"' || ( !bPhrase && ( c == '(' || c == ')' || c == '|' ) );
}

int CSphTokenizerBase::CodepointArbitrationI ( int iCode )
{
	if ( !m_bDetectSentences )
		return iCode;

	// detect sentence boundaries
	// FIXME! should use charset_table (or add a new directive) and support languages other than English
	int iSymbol = iCode & MASK_CODEPOINT;
	if ( iSymbol == '?' || iSymbol == '!' )
	{
		// definitely a sentence boundary
		return MAGIC_CODE_SENTENCE | FLAG_CODEPOINT_SPECIAL;
	}

	if ( iSymbol == '.' )
	{
		// inline dot ("in the U.K and"), not a boundary
		bool bInwordDot = ( sphIsAlpha ( m_pCur[0] ) || ( m_pCur[0] & 0x80 ) == 0x80 // IsAlpha to consider UTF8 chars
							|| m_pCur[0] == ',' );

		// followed by a small letter or an opening paren, not a boundary
		// FIXME? might want to scan for more than one space
		// Yoyodine Inc. exists ...
		// Yoyodine Inc. (the company) ..
		bool bInphraseDot = ( sphIsSpace ( m_pCur[0] )
							  && ( ( 'a' <= m_pCur[1] && m_pCur[1] <= 'z' )
									  || ( m_pCur[1] == '(' && 'a' <= m_pCur[2] && m_pCur[2] <= 'z' ) ) );

		// preceded by something that looks like a middle name, opening first name, salutation
		bool bMiddleName = false;
		switch ( m_iAccum )
		{
		case 1:
			// 1-char capital letter
			// example: J. R. R. Tolkien, who wrote Hobbit ...
			// example: John D. Doe ...
			bMiddleName = IsCapital ( m_pCur[-2] );
			break;
		case 2:
			// 2-char token starting with a capital
			if ( IsCapital ( m_pCur[-3] ) )
			{
				// capital+small
				// example: Known as Mr. Doe ...
				if ( !IsCapital ( m_pCur[-2] ) )
					bMiddleName = true;

				// known capital+capital (MR, DR, MS)
				if (
						( m_pCur[-3] == 'M' && m_pCur[-2] == 'R' ) || ( m_pCur[-3] == 'M' && m_pCur[-2] == 'S' ) || ( m_pCur[-3] == 'D' && m_pCur[-2] == 'R' ) )
					bMiddleName = true;
			}
			break;
		case 3:
			// preceded by a known 3-byte token (MRS, DRS)
			// example: Survived by Mrs. Doe ...
			if ( ( m_sAccum[0] == 'm' || m_sAccum[0] == 'd' ) && m_sAccum[1] == 'r' && m_sAccum[2] == 's' )
				bMiddleName = true;
			break;
		}

		if ( !bInwordDot && !bInphraseDot && !bMiddleName )
		{
			// sentence boundary
			return MAGIC_CODE_SENTENCE | FLAG_CODEPOINT_SPECIAL;
		} else
		{
			// just a character
			if ( ( iCode & MASK_FLAGS ) == FLAG_CODEPOINT_SPECIAL )
				return 0; // special only, not dual? then in this context, it is a separator
			else
				return iCode & ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL ); // perhaps it was blended, so return the original code
		}
	}

	// pass-through
	return iCode;
}


int CSphTokenizerBase::CodepointArbitrationQ ( int iCode, bool bWasEscaped, BYTE uNextByte )
{
	if ( iCode & FLAG_CODEPOINT_NGRAM )
		return iCode; // ngrams are handled elsewhere

	int iSymbol = iCode & MASK_CODEPOINT;

	// codepoints can't be blended and special at the same time
	if ( ( iCode & FLAG_CODEPOINT_BLEND ) && ( iCode & FLAG_CODEPOINT_SPECIAL ) )
	{
		bool bBlend =
				bWasEscaped ||															// escaped characters should always act as blended
				( m_bPhrase && !sphIsModifier ( iSymbol ) && iSymbol != '"' ) ||		// non-modifier special inside phrase
				( m_iAccum && ( iSymbol == '@' || iSymbol == '/' || iSymbol == '-' ) ); // some specials in the middle of a token

		// clear special or blend flags
		iCode &= bBlend
					   ? ~( FLAG_CODEPOINT_DUAL | FLAG_CODEPOINT_SPECIAL )
					   : ~( FLAG_CODEPOINT_DUAL | FLAG_CODEPOINT_BLEND );
	}

	// escaped specials are not special
	// dash and dollar inside the word are not special (however, single opening modifier is not a word!)
	// non-modifier specials within phrase are not special
	bool bDashInside = ( m_iAccum && iSymbol == '-' && !( m_iAccum == 1 && sphIsModifier ( m_sAccum[0] ) ) );
	if ( iCode & FLAG_CODEPOINT_SPECIAL )
		if ( bWasEscaped
				|| bDashInside
				|| ( m_iAccum && iSymbol == '$' && !IsBoundary ( uNextByte, m_bPhrase ) )
				|| ( m_bPhrase && iSymbol != '"' && !sphIsModifier ( iSymbol ) ) )
		{
			if ( iCode & FLAG_CODEPOINT_DUAL )
				iCode &= ~( FLAG_CODEPOINT_SPECIAL | FLAG_CODEPOINT_DUAL );
			else
				iCode = 0;
		}

	// if we didn't remove special by now, it must win
	if ( iCode & FLAG_CODEPOINT_DUAL )
	{
		assert ( iCode & FLAG_CODEPOINT_SPECIAL );
		iCode = iSymbol | FLAG_CODEPOINT_SPECIAL;
	}

	// ideally, all conflicts must be resolved here
	// well, at least most
	assert ( sphBitCount ( iCode & MASK_FLAGS ) <= 1 );
	return iCode;
}
