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

#include "tokenizer.h"

#include "sphinxstd.h"
#include "sphinxint.h"

#include "charset_definition_parser.h"
#include "lowercaser_impl.h"



/////////////////////////////////////////////////////////////////////////////

bool ISphTokenizer::SetCaseFolding ( const char * sConfig, CSphString & sError )
{
	CSphVector<CSphRemapRange> dRemaps;
	if ( !sphParseCharset ( sConfig, dRemaps, &sError ) )
		return false;

	const int MIN_CODE = 0x21;
	ARRAY_FOREACH ( i, dRemaps )
	{
		CSphRemapRange & tMap = dRemaps[i];
		if ( tMap.m_iStart<MIN_CODE || tMap.m_iStart>= CSphLowercaser::MAX_CODE )
		{
			sphWarning ( "wrong character mapping start specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED", tMap.m_iStart, MIN_CODE, CSphLowercaser::MAX_CODE-1 );
			tMap.m_iStart = Min ( Max ( tMap.m_iStart, MIN_CODE ), CSphLowercaser::MAX_CODE-1 );
		}

		if ( tMap.m_iEnd<MIN_CODE || tMap.m_iEnd>= CSphLowercaser::MAX_CODE)
		{
			sphWarning ( "wrong character mapping end specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED", tMap.m_iEnd, MIN_CODE, CSphLowercaser::MAX_CODE-1 );
			tMap.m_iEnd = Min ( Max ( tMap.m_iEnd, MIN_CODE ), CSphLowercaser::MAX_CODE-1 );
		}

		if ( tMap.m_iRemapStart<MIN_CODE || tMap.m_iRemapStart>= CSphLowercaser::MAX_CODE )
		{
			sphWarning ( "wrong character remapping start specified: U+%x, should be between U+%x and U+%x (inclusive); CLAMPED", tMap.m_iRemapStart, MIN_CODE, CSphLowercaser::MAX_CODE-1 );
			tMap.m_iRemapStart = Min ( Max ( tMap.m_iRemapStart, MIN_CODE ), CSphLowercaser::MAX_CODE-1 );
		}

		int iRemapEnd = tMap.m_iRemapStart+tMap.m_iEnd-tMap.m_iStart;
		if ( iRemapEnd<MIN_CODE || iRemapEnd>= CSphLowercaser::MAX_CODE )
		{
			sphWarning ( "wrong character remapping end specified: U+%x, should be between U+%x and U+%x (inclusive); IGNORED", iRemapEnd, MIN_CODE, CSphLowercaser::MAX_CODE-1 );
			dRemaps.Remove(i);
			--i;
		}
	}

	auto& tLC = StagingLowercaser();
	tLC.Reset ();
	tLC.AddRemaps ( dRemaps );
	return true;
}


void ISphTokenizer::AddPlainChars ( const char* szChars )
{
	StagingLowercaser().AddChars ( szChars );
}


void ISphTokenizer::AddSpecials ( const char * sSpecials )
{
	StagingLowercaser().AddChars ( sSpecials, FLAG_CODEPOINT_SPECIAL );
}


void ISphTokenizer::Setup ( const CSphTokenizerSettings & tSettings )
{
	m_tSettings = tSettings;
}


CSphLowercaser& ISphTokenizer::StagingLowercaser()
{
	if ( !m_pStagingLC )
	{
		LowercaserRefcountedPtr pLC { new CSphLowercaser };
		if ( m_pLC )
			pLC->SetRemap ( m_pLC );
		m_pLC = m_pStagingLC = pLC.Leak();
	}
	return *m_pStagingLC;
}


LowercaserRefcountedConstPtr ISphTokenizer::GetLC() const
{
	assert ( m_pLC );
	m_pStagingLC = nullptr;
	return m_pLC;
}

void ISphTokenizer::SetLC ( LowercaserRefcountedConstPtr rhs )
{
	assert ( !m_pStagingLC && !m_pLC );
	m_pLC = std::move ( rhs );
}

bool ISphTokenizer::AddSpecialsSPZ ( const char* sSpecials, const char* sDirective, CSphString& sError )
{
	for ( int i = 0; sSpecials[i]; ++i )
		if ( m_pLC->ToLower ( sSpecials[i] ) & ( FLAG_CODEPOINT_NGRAM | FLAG_CODEPOINT_BOUNDARY | FLAG_CODEPOINT_IGNORE ) )
		{
			sError.SetSprintf ( "%s requires that character '%c' is not in ngram_chars, phrase_boundary, or ignore_chars", sDirective, sSpecials[i] );
			return false;
		}

	AddSpecials ( sSpecials );
	return true;
}


bool ISphTokenizer::EnableSentenceIndexing ( CSphString& sError )
{
	constexpr char sSpecials[] = { '.', '?', '!', MAGIC_CODE_PARAGRAPH, '\0' };
	if ( !AddSpecialsSPZ ( sSpecials, "index_sp", sError ) )
		return false;

	m_bDetectSentences = true;
	return true;
}


bool ISphTokenizer::EnableZoneIndexing ( CSphString& sError )
{
	constexpr char sSpecials[] = { MAGIC_CODE_ZONE, '\0' };
	return AddSpecialsSPZ ( sSpecials, "index_zones", sError );
}

uint64_t ISphTokenizer::GetSettingsFNV() const
{
	uint64_t uHash = GetLowercaser().GetFNV();

	DWORD uFlags = 0;
	if ( m_bBlendSkipPure )
		uFlags |= 1 << 1;
	if ( m_bShortTokenFilter )
		uFlags |= 1 << 2;
	uHash = sphFNV64 ( &uFlags, sizeof ( uFlags ), uHash );
	uHash = sphFNV64 ( &m_uBlendVariants, sizeof ( m_uBlendVariants ), uHash );

	uHash = sphFNV64 ( &m_tSettings.m_iType, sizeof ( m_tSettings.m_iType ), uHash );
	uHash = sphFNV64 ( &m_tSettings.m_iMinWordLen, sizeof ( m_tSettings.m_iMinWordLen ), uHash );
	uHash = sphFNV64 ( &m_tSettings.m_iNgramLen, sizeof ( m_tSettings.m_iNgramLen ), uHash );

	return uHash;
}

bool ISphTokenizer::RemapCharacters ( const char* sConfig, DWORD uFlags, const char* sSource, bool bCanRemap, CSphString& sError )
{
	// parse
	CSphVector<CSphRemapRange> dRemaps;
	if ( !sphParseCharset ( sConfig, dRemaps, &sError ) )
		return false;

	if ( !m_pLC->CheckRemap ( sError, dRemaps, sSource, bCanRemap ) )
		return false;

	// add mapping
	StagingLowercaser().AddRemaps ( dRemaps, uFlags );
	return true;
}

bool ISphTokenizer::SetBoundary ( const char* sConfig, CSphString& sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_BOUNDARY, "phrase boundary", false, sError );
}

bool ISphTokenizer::SetIgnoreChars ( const char* sConfig, CSphString& sError )
{
	return RemapCharacters ( sConfig, FLAG_CODEPOINT_IGNORE, "ignored", false, sError );
}

bool ISphTokenizer::SetBlendChars ( const char* sConfig, CSphString& sError )
{
	return sConfig ? RemapCharacters ( sConfig, FLAG_CODEPOINT_BLEND, "blend", true, sError ) : false;
}


static bool sphStrncmp ( const char* sCheck, int iCheck, const char* sRef )
{
	return ( iCheck == (int)strlen ( sRef ) && memcmp ( sCheck, sRef, iCheck ) == 0 );
}


bool ISphTokenizer::SetBlendMode ( const char* sMode, CSphString& sError )
{
	if ( !sMode || !*sMode )
	{
		m_uBlendVariants = BLEND_TRIM_NONE;
		m_bBlendSkipPure = false;
		return true;
	}

	m_uBlendVariants = 0;
	const char* p = sMode;
	while ( *p )
	{
		while ( !sphIsAlpha ( *p ) )
			p++;
		if ( !*p )
			break;

		const char* sTok = p;
		while ( sphIsAlpha ( *p ) )
			p++;

		int iLen = int ( p - sTok );
		if ( sphStrncmp ( sTok, iLen, "trim_none" ) )
			m_uBlendVariants |= BLEND_TRIM_NONE;
		else if ( sphStrncmp ( sTok, iLen, "trim_head" ) )
			m_uBlendVariants |= BLEND_TRIM_HEAD;
		else if ( sphStrncmp ( sTok, iLen, "trim_tail" ) )
			m_uBlendVariants |= BLEND_TRIM_TAIL;
		else if ( sphStrncmp ( sTok, iLen, "trim_both" ) )
			m_uBlendVariants |= BLEND_TRIM_BOTH;
		else if ( sphStrncmp ( sTok, iLen, "trim_all" ) )
			m_uBlendVariants |= BLEND_TRIM_ALL;
		else if ( sphStrncmp ( sTok, iLen, "skip_pure" ) )
			m_bBlendSkipPure = true;
		else
		{
			sError.SetSprintf ( "unknown blend_mode option near '%s'", sTok );
			return false;
		}
	}

	if ( !m_uBlendVariants )
	{
		sError.SetSprintf ( "blend_mode must define at least one variant to index" );
		m_uBlendVariants = BLEND_TRIM_NONE;
		m_bBlendSkipPure = false;
		return false;
	}
	return true;
}

TokenizerRefPtr_c Tokenizer::Create ( const CSphTokenizerSettings & tSettings, const CSphEmbeddedFiles * pFiles, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings, CSphString & sError )
{
	TokenizerRefPtr_c pResult;
	TokenizerRefPtr_c pTokenizer;

	switch ( tSettings.m_iType )
	{
	case TOKENIZER_UTF8:	pTokenizer = Tokenizer::Detail::CreateUTF8Tokenizer ( tSettings.m_sCaseFolding.IsEmpty() ); break;
	case TOKENIZER_NGRAM:	pTokenizer = Tokenizer::Detail::CreateUTF8NgramTokenizer ( tSettings.m_sCaseFolding.IsEmpty() ); break;
	default:
		sError.SetSprintf ( "failed to create tokenizer (unknown charset type '%d')", tSettings.m_iType );
		return pResult;
	}

	pTokenizer->Setup ( tSettings );

	if ( !tSettings.m_sCaseFolding.IsEmpty () && !pTokenizer->SetCaseFolding ( tSettings.m_sCaseFolding.cstr (), sError ) )
	{
		sError.SetSprintf ( "'charset_table': %s", sError.cstr() );
		return pResult;
	}

	CSphString sSynonymsFile = tSettings.m_sSynonymsFile;
	if ( !sSynonymsFile.IsEmpty() )
	{
		if ( pFilenameBuilder )
			sSynonymsFile = pFilenameBuilder->GetFullPath(sSynonymsFile);

		if ( !pTokenizer->LoadSynonyms ( sSynonymsFile.cstr(), pFiles && pFiles->m_bEmbeddedSynonyms ? pFiles : nullptr, dWarnings, sError ) )
		{
			sError.SetSprintf ( "'synonyms': %s", sError.cstr() );
			return pResult;
		}
	}

	if ( !tSettings.m_sBoundary.IsEmpty () && !pTokenizer->SetBoundary ( tSettings.m_sBoundary.cstr (), sError ) )
	{
		sError.SetSprintf ( "'phrase_boundary': %s", sError.cstr() );
		return pResult;
	}

	if ( !tSettings.m_sIgnoreChars.IsEmpty () && !pTokenizer->SetIgnoreChars ( tSettings.m_sIgnoreChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'ignore_chars': %s", sError.cstr() );
		return pResult;
	}

	if ( !tSettings.m_sBlendChars.IsEmpty () && !pTokenizer->SetBlendChars ( tSettings.m_sBlendChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'blend_chars': %s", sError.cstr() );
		return pResult;
	}

	if ( !pTokenizer->SetBlendMode ( tSettings.m_sBlendMode.cstr (), sError ) )
	{
		sError.SetSprintf ( "'blend_mode': %s", sError.cstr() );
		return pResult;
	}

	pTokenizer->SetNgramLen ( tSettings.m_iNgramLen );

	if ( !tSettings.m_sNgramChars.IsEmpty () && !pTokenizer->SetNgramChars ( tSettings.m_sNgramChars.cstr (), sError ) )
	{
		sError.SetSprintf ( "'ngram_chars': %s", sError.cstr() );
		return pResult;
	}

	pResult = std::move (pTokenizer);
	return pResult;
}

