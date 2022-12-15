//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#include "template_dict_traits.h"

#if WITH_STEMMER
#include <libstemmer.h>
#endif

#include "sphinxstem.h"

#include "word_forms.h"
#include "tokenizer/multiform_container.h"
#include "tokenizer/tokenizer.h"
#include "tokenizer/charset_definition_parser.h"
#include "sphinxint.h"
#include "sphinxjson.h"

static const int MAX_REPORT_LEN = 1024;

CSphString g_sLemmatizerBase;

/// morphology
enum class EMORPH : int {
	STEM_EN,
	STEM_RU_UTF8,
	STEM_CZ,
	STEM_AR_UTF8,
	SOUNDEX,
	METAPHONE_UTF8,
	AOTLEMMER_BASE,
	AOTLEMMER_RU_UTF8 = AOTLEMMER_BASE,
	AOTLEMMER_EN,
	AOTLEMMER_DE_UTF8,
	AOTLEMMER_UK,
	AOTLEMMER_BASE_ALL,
	AOTLEMMER_RU_ALL = AOTLEMMER_BASE_ALL,
	AOTLEMMER_EN_ALL,
	AOTLEMMER_DE_ALL,
	AOTLEMMER_UK_ALL,
	LIBSTEMMER_FIRST,
	LIBSTEMMER_LAST = LIBSTEMMER_FIRST + 64
};


namespace {

void AddStringToReport ( CSphString& sReport, const CSphString& sString, bool bLast )
{
	int iLen = sReport.Length();
	if ( iLen + sString.Length() + 2 > MAX_REPORT_LEN )
		return;

	char* szReport = const_cast<char*> ( sReport.cstr() );
	strcat ( szReport, sString.cstr() ); // NOLINT
	iLen += sString.Length();
	if ( bLast )
		szReport[iLen] = '\0';
	else
	{
		szReport[iLen] = ' ';
		szReport[iLen + 1] = '\0';
	}
}


void ConcatReportStrings ( const CSphTightVector<CSphString>& dStrings, CSphString& sReport )
{
	sReport.Reserve ( MAX_REPORT_LEN );
	*const_cast<char*> ( sReport.cstr() ) = '\0';

	ARRAY_FOREACH ( i, dStrings )
		AddStringToReport ( sReport, dStrings[i], i == dStrings.GetLength() - 1 );
}


void ConcatReportStrings ( const CSphTightVector<CSphNormalForm>& dStrings, CSphString& sReport )
{
	sReport.Reserve ( MAX_REPORT_LEN );
	*const_cast<char*> ( sReport.cstr() ) = '\0';

	ARRAY_FOREACH ( i, dStrings )
		AddStringToReport ( sReport, dStrings[i].m_sForm, i == dStrings.GetLength() - 1 );
}

}// namespace
/////////////////////////////////////////////////////////////////////////////
CSphVector<CSphWordforms*> TemplateDictTraits_c::m_dWordformContainers;

TemplateDictTraits_c::TemplateDictTraits_c() = default;

TemplateDictTraits_c::~TemplateDictTraits_c()
{
#if WITH_STEMMER
	for ( void* pStemmer : m_dStemmers )
		sb_stemmer_delete ( (sb_stemmer*)pStemmer );
#endif

	if ( m_pWordforms )
		--m_pWordforms->m_iRefCount;
}


SphWordID_t TemplateDictTraits_c::FilterStopword ( SphWordID_t uID ) const
{
	if ( !m_iStopwords )
		return uID;

	// OPTIMIZE: binary search is not too good, could do some hashing instead
	SphWordID_t* pStart = m_pStopwords;
	SphWordID_t* pEnd = m_pStopwords + m_iStopwords - 1;
	do
	{
		if ( uID == *pStart || uID == *pEnd )
			return 0;

		if ( uID < *pStart || uID > *pEnd )
			return uID;

		SphWordID_t* pMid = pStart + ( pEnd - pStart ) / 2;
		if ( uID == *pMid )
			return 0;

		if ( uID < *pMid )
			pEnd = pMid;
		else
			pStart = pMid;
	} while ( pEnd - pStart > 1 );

	return uID;
}


int TemplateDictTraits_c::ParseMorphology ( const char* sMorph, CSphString& sMessage )
{
	int iRes = ST_OK;
	for ( const char* sStart = sMorph;; )
	{
		while ( *sStart && ( sphIsSpace ( *sStart ) || *sStart == ',' ) )
			++sStart;
		if ( !*sStart )
			break;

		const char* sWordStart = sStart;
		while ( *sStart && !sphIsSpace ( *sStart ) && *sStart != ',' )
			++sStart;

		if ( sStart > sWordStart )
		{
			switch ( InitMorph ( sWordStart, int ( sStart - sWordStart ), sMessage ) )
			{
			case ST_ERROR: return ST_ERROR;
			case ST_WARNING: iRes = ST_WARNING; break;
			default: break;
			}
		}
	}
	return iRes;
}


int TemplateDictTraits_c::InitMorph ( const char* szMorph, int iLength, CSphString& sMessage )
{
	if ( iLength == 0 )
		return ST_OK;

	if ( iLength == 4 && !strncmp ( szMorph, "none", iLength ) )
		return ST_OK;

	if ( iLength == 7 && !strncmp ( szMorph, "stem_en", iLength ) )
	{
		if ( m_dMorph.Contains ( (int)EMORPH::AOTLEMMER_EN ) )
		{
			sMessage.SetSprintf ( "stem_en and lemmatize_en clash" );
			return ST_ERROR;
		}

		if ( m_dMorph.Contains ( (int)EMORPH::AOTLEMMER_EN_ALL ) )
		{
			sMessage.SetSprintf ( "stem_en and lemmatize_en_all clash" );
			return ST_ERROR;
		}

		stem_en_init();
		return AddMorph ( (int)EMORPH::STEM_EN );
	}

	if ( iLength == 7 && !strncmp ( szMorph, "stem_ru", iLength ) )
	{
		if ( m_dMorph.Contains ( (int)EMORPH::AOTLEMMER_RU_UTF8 ) )
		{
			sMessage.SetSprintf ( "stem_ru and lemmatize_ru clash" );
			return ST_ERROR;
		}

		if ( m_dMorph.Contains ( (int)EMORPH::AOTLEMMER_RU_ALL ) )
		{
			sMessage.SetSprintf ( "stem_ru and lemmatize_ru_all clash" );
			return ST_ERROR;
		}

		stem_ru_init();
		return AddMorph ( (int)EMORPH::STEM_RU_UTF8 );
	}

	for ( int j = 0; j < AOT_LENGTH; ++j )
	{
		char buf[20];
		char buf_all[20];
		sprintf ( buf, "lemmatize_%s", AOT_LANGUAGES[j] );		   // NOLINT
		sprintf ( buf_all, "lemmatize_%s_all", AOT_LANGUAGES[j] ); // NOLINT

		if ( iLength == 12 && !strncmp ( szMorph, buf, iLength ) )
		{
			if ( j == AOT_RU && m_dMorph.Contains ( (int)EMORPH::STEM_RU_UTF8 ) )
			{
				sMessage.SetSprintf ( "stem_ru and lemmatize_ru clash" );
				return ST_ERROR;
			}

			if ( j == AOT_EN && m_dMorph.Contains ( (int)EMORPH::STEM_EN ) )
			{
				sMessage.SetSprintf ( "stem_en and lemmatize_en clash" );
				return ST_ERROR;
			}

			// no test for SPH_MORPH_STEM_DE since we doesn't have it.

			if ( m_dMorph.Contains ( static_cast<int> ( EMORPH::AOTLEMMER_BASE_ALL ) + j ) )
			{
				sMessage.SetSprintf ( "%s and %s clash", buf, buf_all );
				return ST_ERROR;
			}

			auto sDictFile = SphSprintf ( "%s/%s.pak", g_sLemmatizerBase.cstr(), AOT_LANGUAGES[j] );
			if ( !sphAotInit ( sDictFile, sMessage, j ) )
				return ST_ERROR;

			if ( j == AOT_UK && !m_tLemmatizer )
				m_tLemmatizer = CreateLemmatizer ( j );

			// add manually instead of AddMorph(), because we need to update that fingerprint
			int iMorph;
			switch ( j )
			{
			case AOT_RU: iMorph = (int)EMORPH::AOTLEMMER_RU_UTF8; break;
			case AOT_DE: iMorph = (int)EMORPH::AOTLEMMER_DE_UTF8; break;
			case AOT_UK: iMorph = (int)EMORPH::AOTLEMMER_UK; break;
			default: iMorph = j + (int)EMORPH::AOTLEMMER_BASE;
			}

			if ( !m_dMorph.Contains ( iMorph ) )
			{
				if ( m_sMorphFingerprint.IsEmpty() )
					m_sMorphFingerprint.SetSprintf ( "%s:%08x", sphAotDictinfo ( j ).first.cstr(), sphAotDictinfo ( j ).second );
				else
					m_sMorphFingerprint.SetSprintf ( "%s;%s:%08x", m_sMorphFingerprint.cstr(), sphAotDictinfo ( j ).first.cstr(), sphAotDictinfo ( j ).second );
				m_dMorph.Add ( iMorph );
			}
			return ST_OK;
		}

		if ( iLength == 16 && !strncmp ( szMorph, buf_all, iLength ) )
		{
			if ( j == AOT_RU && ( m_dMorph.Contains ( (int)EMORPH::STEM_RU_UTF8 ) ) )
			{
				sMessage.SetSprintf ( "stem_ru and lemmatize_ru_all clash" );
				return ST_ERROR;
			}

			if ( m_dMorph.Contains ( (int)EMORPH::AOTLEMMER_BASE + j ) )
			{
				sMessage.SetSprintf ( "%s and %s clash", buf, buf_all );
				return ST_ERROR;
			}

			auto sDictFile = SphSprintf ( "%s/%s.pak", g_sLemmatizerBase.cstr(), AOT_LANGUAGES[j] );
			if ( !sphAotInit ( sDictFile, sMessage, j ) )
				return ST_ERROR;

			if ( j == AOT_UK && !m_tLemmatizer )
				m_tLemmatizer = CreateLemmatizer ( j );

			return AddMorph ( (int)EMORPH::AOTLEMMER_BASE_ALL + j );
		}
	}

	if ( iLength == 7 && !strncmp ( szMorph, "stem_cz", iLength ) )
	{
		stem_cz_init();
		return AddMorph ( (int)EMORPH::STEM_CZ );
	}

	if ( iLength == 7 && !strncmp ( szMorph, "stem_ar", iLength ) )
		return AddMorph ( (int)EMORPH::STEM_AR_UTF8 );

	if ( iLength == 9 && !strncmp ( szMorph, "stem_enru", iLength ) )
	{
		stem_en_init();
		stem_ru_init();
		AddMorph ( (int)EMORPH::STEM_EN );
		return AddMorph ( (int)EMORPH::STEM_RU_UTF8 );
	}

	if ( iLength == 7 && !strncmp ( szMorph, "soundex", iLength ) )
		return AddMorph ( (int)EMORPH::SOUNDEX );

	if ( iLength == 9 && !strncmp ( szMorph, "metaphone", iLength ) )
		return AddMorph ( (int)EMORPH::METAPHONE_UTF8 );

#if WITH_STEMMER
	const int LIBSTEMMER_LEN = 11;
	const int MAX_ALGO_LENGTH = 64;
	if ( iLength > LIBSTEMMER_LEN && iLength - LIBSTEMMER_LEN < MAX_ALGO_LENGTH && !strncmp ( szMorph, "libstemmer_", LIBSTEMMER_LEN ) )
	{
		CSphString sAlgo;
		sAlgo.SetBinary ( szMorph + LIBSTEMMER_LEN, iLength - LIBSTEMMER_LEN );

		sb_stemmer* pStemmer = nullptr;

		pStemmer = sb_stemmer_new ( sAlgo.cstr(), "UTF_8" );

		if ( !pStemmer )
		{
			sMessage.SetSprintf ( "unknown stemmer libstemmer_%s; skipped", sAlgo.cstr() );
			return ST_WARNING;
		}

		AddMorph ( (int)EMORPH::LIBSTEMMER_FIRST + m_dStemmers.GetLength() );
		for ( const auto* pStemmer_c: m_dStemmers )
			if ( pStemmer_c == pStemmer )
			{
				sb_stemmer_delete ( pStemmer );
				return ST_OK;
			}

		m_dStemmers.Add ( pStemmer );
		m_dDescStemmers.Add ( sAlgo );
		return ST_OK;
	}
#endif

	if ( iLength == 11 && !strncmp ( szMorph, "icu_chinese", iLength ) )
		return ST_OK;

	sMessage.SetBinary ( szMorph, iLength );
	sMessage.SetSprintf ( "unknown stemmer %s", sMessage.cstr() );
	return ST_ERROR;
}


int TemplateDictTraits_c::AddMorph ( int iMorph )
{
	if ( !m_dMorph.Contains ( iMorph ) )
		m_dMorph.Add ( iMorph );
	return ST_OK;
}


void TemplateDictTraits_c::ApplyStemmers ( BYTE* pWord ) const
{
	// try wordforms
	if ( m_pWordforms && m_pWordforms->ToNormalForm ( pWord, true, m_bDisableWordforms ) )
		return;

	// check length
	if ( m_tSettings.m_iMinStemmingLen <= 1 || sphUTF8Len ( (const char*)pWord ) >= m_tSettings.m_iMinStemmingLen )
	{
		// try stemmers
		for ( int iMorph : m_dMorph )
			if ( StemById ( pWord, iMorph ) )
				break;
	}

	if ( m_pWordforms && m_pWordforms->m_bHavePostMorphNF )
		m_pWordforms->ToNormalForm ( pWord, false, m_bDisableWordforms );
}

const CSphMultiformContainer* TemplateDictTraits_c::GetMultiWordforms() const
{
	return m_pWordforms ? m_pWordforms->m_pMultiWordforms.get() : nullptr;
}

uint64_t TemplateDictTraits_c::GetSettingsFNV() const
{
	auto uHash = (uint64_t)m_pWordforms;

	if ( m_pStopwords )
		uHash = sphFNV64 ( m_pStopwords, m_iStopwords * sizeof ( *m_pStopwords ), uHash );

	uHash = sphFNV64 ( &m_tSettings.m_iMinStemmingLen, sizeof ( m_tSettings.m_iMinStemmingLen ), uHash );
	DWORD uFlags = 0;
	if ( m_tSettings.m_bWordDict )
		uFlags |= 1 << 0;
	if ( m_tSettings.m_bStopwordsUnstemmed )
		uFlags |= 1 << 2;
	uHash = sphFNV64 ( &uFlags, sizeof ( uFlags ), uHash );

	uHash = sphFNV64 ( m_dMorph.Begin(), m_dMorph.GetLength() * sizeof ( m_dMorph[0] ), uHash );
#if WITH_STEMMER
	for ( const CSphString& sDescStemmer : m_dDescStemmers )
		uHash = sphFNV64 ( sDescStemmer.cstr(), sDescStemmer.Length(), uHash );
#endif

	return uHash;
}


DictRefPtr_c TemplateDictTraits_c::CloneBase ( TemplateDictTraits_c* pDict ) const
{
	assert ( pDict );
	pDict->m_tSettings = m_tSettings;
	pDict->m_iStopwords = m_iStopwords;
	pDict->m_pStopwords = m_pStopwords;
	pDict->m_dSWFileInfos = m_dSWFileInfos;
	pDict->m_dWFFileInfos = m_dWFFileInfos;
	pDict->m_pWordforms = m_pWordforms;
	if ( m_pWordforms )
		m_pWordforms->m_iRefCount++;

	pDict->m_dMorph = m_dMorph;
#if WITH_STEMMER
	assert ( m_dDescStemmers.GetLength() == m_dStemmers.GetLength() );
	pDict->m_dDescStemmers = m_dDescStemmers;
	ARRAY_FOREACH ( i, m_dDescStemmers )
	{
		pDict->m_dStemmers.Add ( sb_stemmer_new ( m_dDescStemmers[i].cstr(), "UTF_8" ) );
		assert ( pDict->m_dStemmers.Last() );
	}
#endif
	if ( m_tLemmatizer )
		pDict->m_tLemmatizer = CreateLemmatizer ( AOT_UK );

	return DictRefPtr_c { pDict };
}

bool TemplateDictTraits_c::HasState() const
{
#if !WITH_STEMMER
	return ( (bool)m_tLemmatizer );
#else
	return ( m_dDescStemmers.GetLength() > 0 || m_tLemmatizer );
#endif
}

void TemplateDictTraits_c::LoadStopwords ( const char* sFiles, const TokenizerRefPtr_c& pTokenizer, bool bStripFile )
{
	assert ( !m_pStopwords );
	assert ( !m_iStopwords );

	// tokenize file list
	if ( !sFiles || !*sFiles )
		return;

	m_dSWFileInfos.Resize ( 0 );

	TokenizerRefPtr_c pTokenizerClone = pTokenizer->Clone ( SPH_CLONE_INDEX );
	CSphFixedVector<char> dList ( 1 + (int)strlen ( sFiles ) );
	strcpy ( dList.Begin(), sFiles ); // NOLINT

	char* pCur = dList.Begin();
	char* sName = nullptr;

	CSphVector<SphWordID_t> dStop;

	while ( true )
	{
		// find next name start
		while ( *pCur && ( isspace ( *pCur ) || *pCur == ',' ) )
			pCur++;
		if ( !*pCur )
			break;
		sName = pCur;

		// find next name end
		while ( *pCur && !( isspace ( *pCur ) || *pCur == ',' ) )
			pCur++;
		if ( *pCur )
			*pCur++ = '\0';

		CSphString sFileName = sName;
		bool bGotFile = sphIsReadable ( sFileName );
		if ( !bGotFile )
		{
			if ( bStripFile )
			{
				StripPath ( sFileName );
				bGotFile = sphIsReadable ( sFileName );
			}
			if ( !bGotFile )
			{
				if ( !bStripFile )
					StripPath ( sFileName );
				sFileName.SetSprintf ( "%s/stopwords/%s", GET_FULL_SHARE_DIR(), sFileName.cstr() );
				bGotFile = sphIsReadable ( sFileName );
			}
		}

		CSphFixedVector<BYTE> dBuffer ( 0 );
		CSphSavedFile tInfo;
		tInfo.Collect ( sFileName.cstr() );

		// need to store original name to compatible with original behavior of load order
		// from path defined; from tool CWD; from SHARE_DIR
		tInfo.m_sFilename = sName;
		m_dSWFileInfos.Add ( tInfo );

		if ( !bGotFile )
		{
			StringBuilder_c sError;
			sError.Appendf ( "failed to load stopwords from either '%s' or '%s'", sName, sFileName.cstr() );
			if ( bStripFile )
				sError += ", current work directory";
			sphWarn ( "%s", sError.cstr() );
			continue;
		}

		// open file
		FILE* fp = fopen ( sFileName.cstr(), "rb" );
		if ( !fp )
		{
			sphWarn ( "failed to load stopwords from '%s'", sFileName.cstr() );
			continue;
		}

		struct_stat st = { 0 };
		if ( fstat ( fileno ( fp ), &st ) == 0 )
			dBuffer.Reset ( st.st_size );
		else
		{
			fclose ( fp );
			sphWarn ( "stopwords: failed to get file size for '%s'", sFileName.cstr() );
			continue;
		}

		// tokenize file
		int iLength = (int)fread ( dBuffer.Begin(), 1, (size_t)st.st_size, fp );

		BYTE* pToken;
		pTokenizerClone->SetBuffer ( dBuffer.Begin(), iLength );
		while ( ( pToken = pTokenizerClone->GetToken() ) != nullptr )
			if ( m_tSettings.m_bStopwordsUnstemmed )
				dStop.Add ( GetWordIDNonStemmed ( pToken ) );
			else
				dStop.Add ( GetWordID ( pToken ) );

		// close file
		fclose ( fp );
	}

	// sort stopwords
	dStop.Uniq();

	// store IDs
	if ( dStop.GetLength() )
	{
		m_dStopwordContainer.Reset ( dStop.GetLength() );
		ARRAY_FOREACH ( i, dStop )
			m_dStopwordContainer[i] = dStop[i];

		m_iStopwords = m_dStopwordContainer.GetLength();
		m_pStopwords = m_dStopwordContainer.Begin();
	}
}


void TemplateDictTraits_c::LoadStopwords ( const CSphVector<SphWordID_t>& dStopwords )
{
	m_dStopwordContainer.Reset ( dStopwords.GetLength() );
	ARRAY_FOREACH ( i, dStopwords )
		m_dStopwordContainer[i] = dStopwords[i];

	m_iStopwords = m_dStopwordContainer.GetLength();
	m_pStopwords = m_dStopwordContainer.Begin();
}


void TemplateDictTraits_c::WriteStopwords ( CSphWriter& tWriter ) const
{
	tWriter.PutDword ( (DWORD)m_iStopwords );
	for ( int i = 0; i < m_iStopwords; ++i )
		tWriter.ZipOffset ( m_pStopwords[i] );
}

void TemplateDictTraits_c::WriteStopwords ( JsonEscapedBuilder& tOut ) const
{
	if ( !m_iStopwords )
		return;

	tOut.Named ( "stopwords_list" );
	auto _ = tOut.Array();
	for ( int i = 0; i < m_iStopwords; ++i )
		tOut << cast2signed ( m_pStopwords[i] );
}


void TemplateDictTraits_c::SweepWordformContainers ( const CSphVector<CSphSavedFile>& dFiles )
{
	for ( int i = 0; i < m_dWordformContainers.GetLength(); )
	{
		CSphWordforms* WC = m_dWordformContainers[i];
		if ( WC->m_iRefCount == 0 && !WC->IsEqual ( dFiles ) )
		{
			delete WC;
			m_dWordformContainers.Remove ( i );
		} else
			++i;
	}
}

CSphWordforms* TemplateDictTraits_c::GetWordformContainer ( const CSphVector<CSphSavedFile>& dFileInfos, const StrVec_t* pEmbedded, const TokenizerRefPtr_c& pTokenizer, const char* szIndex )
{
	uint64_t uTokenizerFNV = pTokenizer->GetSettingsFNV();
	for ( CSphWordforms *pContainer : m_dWordformContainers )
		if ( pContainer->IsEqual ( dFileInfos ) )
		{
			if ( uTokenizerFNV == pContainer->m_uTokenizerFNV )
				return pContainer;

			CSphTightVector<CSphString> dErrorReport;
			for ( const auto& j : dFileInfos )
				dErrorReport.Add ( j.m_sFilename );

			CSphString sAllFiles;
			ConcatReportStrings ( dErrorReport, sAllFiles );
			sphWarning ( "table '%s': wordforms file '%s' is shared with table '%s', but tokenizer settings are different",	szIndex,	sAllFiles.cstr(), pContainer->m_sIndexName.cstr() );
		}

	CSphWordforms* pContainer = LoadWordformContainer ( dFileInfos, pEmbedded, pTokenizer, szIndex );
	if ( pContainer )
		m_dWordformContainers.Add ( pContainer );

	return pContainer;
}


void TemplateDictTraits_c::AddWordform ( CSphWordforms* pContainer, char* sBuffer, int iLen, const TokenizerRefPtr_c& pTokenizer, const char* szFile, const CSphVector<int>& dBlended, int iFileId )
{
	StrVec_t dTokens;

	bool bSeparatorFound = false;
	bool bAfterMorphology = false;

	// parse the line
	pTokenizer->SetBuffer ( (BYTE*)sBuffer, iLen );

	bool bFirstToken = true;
	bool bStopwordsPresent = false;
	bool bCommentedWholeLine = false;

	BYTE* pFrom = nullptr;
	while ( ( pFrom = pTokenizer->GetToken() ) != nullptr )
	{
		if ( *pFrom == '#' )
		{
			bCommentedWholeLine = bFirstToken;
			break;
		}

		if ( *pFrom == '~' && bFirstToken )
		{
			bAfterMorphology = true;
			bFirstToken = false;
			continue;
		}

		bFirstToken = false;

		if ( *pFrom == '>' )
		{
			bSeparatorFound = true;
			break;
		}

		if ( *pFrom == '=' && *pTokenizer->GetBufferPtr() == '>' )
		{
			pTokenizer->GetToken();
			bSeparatorFound = true;
			break;
		}

		if ( GetWordID ( pFrom, (int)strlen ( (const char*)pFrom ), true ) )
			dTokens.Add ( (const char*)pFrom );
		else
			bStopwordsPresent = true;
	}

	if ( !dTokens.GetLength() )
	{
		if ( !bCommentedWholeLine )
			sphWarning ( "table '%s': all source tokens are stopwords (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( !bSeparatorFound )
	{
		sphWarning ( "table '%s': no wordform separator found (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	BYTE* pTo = pTokenizer->GetToken();
	if ( !pTo )
	{
		sphWarning ( "table '%s': no destination token found (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( *pTo == '#' )
	{
		sphWarning ( "table '%s': misplaced comment (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	CSphVector<CSphNormalForm> dDestTokens;
	bool bFirstDestIsStop = !GetWordID ( pTo, (int)strlen ( (const char*)pTo ), true );
	CSphNormalForm& tForm = dDestTokens.Add();
	tForm.m_sForm = (const char*)pTo;
	tForm.m_iLengthCP = pTokenizer->GetLastTokenLen();

	// what if we have more than one word in the right part?
	const BYTE* pDestToken;
	while ( ( pDestToken = pTokenizer->GetToken() ) != nullptr )
	{
		bool bStop = ( !GetWordID ( pDestToken, (int)strlen ( (const char*)pDestToken ), true ) );
		if ( !bStop )
		{
			CSphNormalForm& tNewForm = dDestTokens.Add();
			tNewForm.m_sForm = (const char*)pDestToken;
			tNewForm.m_iLengthCP = pTokenizer->GetLastTokenLen();
		}

		bStopwordsPresent |= bStop;
	}

	// we can have wordforms with 1 destination token that is a stopword
	if ( dDestTokens.GetLength() > 1 && bFirstDestIsStop )
		dDestTokens.Remove ( 0 );

	if ( !dDestTokens.GetLength() )
	{
		sphWarning ( "table '%s': destination token is a stopword (wordform='%s', file='%s'). IGNORED.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		return;
	}

	if ( bStopwordsPresent )
		sphWarning ( "table '%s': wordform contains stopwords (wordform='%s'). Fix your wordforms file '%s'.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );

	// we disabled all blended, so we need to filter them manually
	bool bBlendedPresent = false;
	if ( dBlended.GetLength() )
		for ( const auto& tDestToken : dDestTokens )
		{
			int iCode;
			const BYTE* pBuf = (const BYTE*)tDestToken.m_sForm.cstr();
			while ( ( iCode = sphUTF8Decode ( pBuf ) ) > 0 && !bBlendedPresent )
				bBlendedPresent = ( dBlended.BinarySearch ( iCode ) != nullptr );
		}

	if ( bBlendedPresent )
		sphWarning ( "invalid mapping (destination contains blended characters) (wordform='%s'). Fix your wordforms file '%s'.", sBuffer, szFile );

	if ( bBlendedPresent && dDestTokens.GetLength() > 1 )
	{
		sphWarning ( "blended characters are not allowed with multiple destination tokens (wordform='%s', file='%s'). IGNORED.", sBuffer, szFile );
		return;
	}

	if ( dTokens.GetLength() > 1 || dDestTokens.GetLength() > 1 )
	{
		auto pMultiWordform = std::make_unique<CSphMultiform>();
		pMultiWordform->m_iFileId = iFileId;
		pMultiWordform->m_dNormalForm.Resize ( dDestTokens.GetLength() );
		ARRAY_FOREACH ( i, dDestTokens )
			pMultiWordform->m_dNormalForm[i] = dDestTokens[i];

		for ( int i = 1; i < dTokens.GetLength(); ++i )
			pMultiWordform->m_dTokens.Add ( dTokens[i] );

		if ( !pContainer->m_pMultiWordforms )
			pContainer->m_pMultiWordforms = std::make_unique<CSphMultiformContainer>();

		CSphMultiforms** ppWordforms = pContainer->m_pMultiWordforms->m_Hash ( dTokens[0] );
		if ( ppWordforms )
		{
			auto* pWordforms = *ppWordforms;
			for ( const auto& pStoredMF : pWordforms->m_pForms )
			{
				if ( pStoredMF->m_dTokens.GetLength() == pMultiWordform->m_dTokens.GetLength() )
				{
					bool bSameTokens = true;
					ARRAY_FOREACH_COND ( iToken, pStoredMF->m_dTokens, bSameTokens )
						if ( pStoredMF->m_dTokens[iToken] != pMultiWordform->m_dTokens[iToken] )
							bSameTokens = false;

					if ( bSameTokens )
					{
						CSphString sStoredTokens, sStoredForms;
						ConcatReportStrings ( pStoredMF->m_dTokens, sStoredTokens );
						ConcatReportStrings ( pStoredMF->m_dNormalForm, sStoredForms );
						sphWarning ( "table '%s': duplicate wordform found - overridden ( current='%s', old='%s %s > %s' ). Fix your wordforms file '%s'.",
							pContainer->m_sIndexName.cstr(),
							sBuffer,
							dTokens[0].cstr(),
							sStoredTokens.cstr(),
							sStoredForms.cstr(),
							szFile );

						pStoredMF->m_dNormalForm.Resize ( pMultiWordform->m_dNormalForm.GetLength() );
						ARRAY_FOREACH ( iForm, pMultiWordform->m_dNormalForm )
							pStoredMF->m_dNormalForm[iForm] = pMultiWordform->m_dNormalForm[iForm];

						pStoredMF->m_iFileId = iFileId;

						pMultiWordform.reset();
						break; // otherwise, we crash next turn
					}
				}
			}

			if ( pMultiWordform )
			{
				pWordforms->m_iMinTokens = Min ( pWordforms->m_iMinTokens, pMultiWordform->m_dTokens.GetLength() );
				pWordforms->m_iMaxTokens = Max ( pWordforms->m_iMaxTokens, pMultiWordform->m_dTokens.GetLength() );
				pWordforms->m_pForms.Add ( pMultiWordform.release() );

				// sort forms by files and length
				// but do not sort if we're loading embedded
				if ( iFileId >= 0 )
					pWordforms->m_pForms.Sort ( Lesser ( [] ( const CSphMultiform* pA, const CSphMultiform* pB ) {
						assert ( pA && pB );
						return ( pA->m_iFileId == pB->m_iFileId ) ? pA->m_dTokens.GetLength() > pB->m_dTokens.GetLength() : pA->m_iFileId > pB->m_iFileId;
					} ) );

				pContainer->m_pMultiWordforms->m_iMaxTokens = Max ( pContainer->m_pMultiWordforms->m_iMaxTokens, pWordforms->m_iMaxTokens );
			}
		} else
		{
			auto pNewWordforms = std::make_unique<CSphMultiforms>();
			pNewWordforms->m_iMinTokens = pMultiWordform->m_dTokens.GetLength();
			pNewWordforms->m_iMaxTokens = pMultiWordform->m_dTokens.GetLength();
			pNewWordforms->m_pForms.Add ( pMultiWordform.release() );
			pContainer->m_pMultiWordforms->m_iMaxTokens = Max ( pContainer->m_pMultiWordforms->m_iMaxTokens, pNewWordforms->m_iMaxTokens );
			pContainer->m_pMultiWordforms->m_Hash.Add ( pNewWordforms.release(), dTokens[0] );
		}

		// let's add destination form to regular wordform to keep destination from being stemmed
		// FIXME!!! handle multiple destination tokens and ~flag for wordforms
		if ( !bAfterMorphology && dDestTokens.GetLength() == 1 && !pContainer->m_hHash.Exists ( dDestTokens[0].m_sForm ) )
		{
			CSphStoredNF tStoredForm;
			tStoredForm.m_sWord = dDestTokens[0].m_sForm;
			tStoredForm.m_bAfterMorphology = bAfterMorphology;
			pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			if ( !pContainer->m_dNormalForms.GetLength()
				 || pContainer->m_dNormalForms.Last().m_sWord != dDestTokens[0].m_sForm
				 || pContainer->m_dNormalForms.Last().m_bAfterMorphology != bAfterMorphology )
				pContainer->m_dNormalForms.Add ( tStoredForm );

			pContainer->m_hHash.Add ( pContainer->m_dNormalForms.GetLength() - 1, dDestTokens[0].m_sForm );
		}
	} else
	{
		if ( bAfterMorphology )
		{
			BYTE pBuf[16 + 3 * SPH_MAX_WORD_LEN];
			memcpy ( pBuf, dTokens[0].cstr(), dTokens[0].Length() + 1 );
			ApplyStemmers ( pBuf );
			dTokens[0] = (char*)pBuf;
		}

		// check wordform that source token is a new token or has same destination token
		int* pRefTo = pContainer->m_hHash ( dTokens[0] );
		assert ( !pRefTo || ( *pRefTo >= 0 && *pRefTo < pContainer->m_dNormalForms.GetLength() ) );
		if ( pRefTo )
		{
			// replace with a new wordform
			if ( pContainer->m_dNormalForms[*pRefTo].m_sWord != dDestTokens[0].m_sForm || pContainer->m_dNormalForms[*pRefTo].m_bAfterMorphology != bAfterMorphology )
			{
				CSphStoredNF& tRefTo = pContainer->m_dNormalForms[*pRefTo];
				sphWarning ( "table '%s': duplicate wordform found - overridden ( current='%s', old='%s%s > %s' ). Fix your wordforms file '%s'.",
					pContainer->m_sIndexName.cstr(),
					sBuffer,
					tRefTo.m_bAfterMorphology ? "~" : "",
					dTokens[0].cstr(),
					tRefTo.m_sWord.cstr(),
					szFile );

				tRefTo.m_sWord = dDestTokens[0].m_sForm;
				tRefTo.m_bAfterMorphology = bAfterMorphology;
				pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			} else
				sphWarning ( "table '%s': duplicate wordform found ( '%s' ). Fix your wordforms file '%s'.", pContainer->m_sIndexName.cstr(), sBuffer, szFile );
		} else
		{
			CSphStoredNF tStoredForm;
			tStoredForm.m_sWord = dDestTokens[0].m_sForm;
			tStoredForm.m_bAfterMorphology = bAfterMorphology;
			pContainer->m_bHavePostMorphNF |= bAfterMorphology;
			if ( !pContainer->m_dNormalForms.GetLength()
				 || pContainer->m_dNormalForms.Last().m_sWord != dDestTokens[0].m_sForm
				 || pContainer->m_dNormalForms.Last().m_bAfterMorphology != bAfterMorphology )
				pContainer->m_dNormalForms.Add ( tStoredForm );

			pContainer->m_hHash.Add ( pContainer->m_dNormalForms.GetLength() - 1, dTokens[0] );
		}
	}
}


CSphWordforms* TemplateDictTraits_c::LoadWordformContainer ( const CSphVector<CSphSavedFile>& dFileInfos, const StrVec_t* pEmbeddedWordforms, const TokenizerRefPtr_c& pTokenizer, const char* szIndex )
{
	// allocate it
	auto pContainer = std::make_unique<CSphWordforms>();
	pContainer->m_dFiles = dFileInfos;
	pContainer->m_uTokenizerFNV = pTokenizer->GetSettingsFNV();
	pContainer->m_sIndexName = szIndex;

	TokenizerRefPtr_c pMyTokenizer = pTokenizer->Clone ( SPH_CLONE_INDEX );
	const CSphTokenizerSettings& tSettings = pMyTokenizer->GetSettings();
	CSphVector<int> dBlended;

	// get a list of blend chars and set add them to the tokenizer as simple chars
	if ( tSettings.m_sBlendChars.Length() )
	{
		StringBuilder_c sNewCharset;
		sNewCharset << tSettings.m_sCaseFolding;

		CSphVector<CSphRemapRange> dRemaps;
		if ( sphParseCharset ( tSettings.m_sBlendChars.cstr(), dRemaps ) )
			for ( const auto& dRemap : dRemaps )
				for ( int j = dRemap.m_iStart; j <= dRemap.m_iEnd; ++j )
				{
					sNewCharset << ", " << (char)j;
					dBlended.Add ( j );
				}

		// sort dBlended for binary search
		dBlended.Sort();

		CSphString sError;
		pMyTokenizer->SetCaseFolding ( sNewCharset.cstr(), sError );

		// disable blend chars
		pMyTokenizer->SetBlendChars ( nullptr, sError );
	}

	// add wordform-specific specials
	pMyTokenizer->AddSpecials ( "#=>~" );

	if ( pEmbeddedWordforms )
	{
		CSphTightVector<CSphString> dFilenames;
		dFilenames.Resize ( dFileInfos.GetLength() );
		ARRAY_FOREACH ( i, dFileInfos )
			dFilenames[i] = dFileInfos[i].m_sFilename;

		CSphString sAllFiles;
		ConcatReportStrings ( dFilenames, sAllFiles );

		for ( auto& sWordForm : ( *pEmbeddedWordforms ) )
			AddWordform ( pContainer.get(), const_cast<char*> ( sWordForm.cstr() ), sWordForm.Length(), pMyTokenizer, sAllFiles.cstr(), dBlended, -1 );
	} else
	{
		char sBuffer[6 * SPH_MAX_WORD_LEN + 512]; // enough to hold 2 UTF-8 words, plus some whitespace overhead

		ARRAY_FOREACH ( i, dFileInfos )
		{
			CSphAutoreader rdWordforms;
			const char* szFile = dFileInfos[i].m_sFilename.cstr();
			CSphString sError;
			if ( !rdWordforms.Open ( szFile, sError ) )
			{
				sphWarning ( "table '%s': %s", szIndex, sError.cstr() );
				return nullptr;
			}

			int iLen;
			while ( ( iLen = rdWordforms.GetLine ( sBuffer, sizeof ( sBuffer ) ) ) >= 0 )
				AddWordform ( pContainer.get(), sBuffer, iLen, pMyTokenizer, szFile, dBlended, i );
		}
	}

	return pContainer.release();
}


bool TemplateDictTraits_c::LoadWordforms ( const StrVec_t& dFiles, const CSphEmbeddedFiles* pEmbedded, const TokenizerRefPtr_c& pTokenizer, const char* szIndex )
{
	if ( pEmbedded )
	{
		m_dWFFileInfos.Resize ( pEmbedded->m_dWordformFiles.GetLength() );
		ARRAY_FOREACH ( i, m_dWFFileInfos )
			m_dWFFileInfos[i] = pEmbedded->m_dWordformFiles[i];
	} else
	{
		m_dWFFileInfos.Reserve ( dFiles.GetLength() );
		CSphSavedFile tFile;
		for ( const auto& sFile : dFiles )
			if ( !sFile.IsEmpty() )
			{
				if ( tFile.Collect ( sFile.cstr() ) )
					m_dWFFileInfos.Add ( tFile );
				else
					sphWarning ( "table '%s': wordforms file '%s' not found", szIndex, sFile.cstr() );
			}
	}

	if ( !m_dWFFileInfos.GetLength() )
		return false;

	SweepWordformContainers ( m_dWFFileInfos );

	m_pWordforms = GetWordformContainer ( m_dWFFileInfos, pEmbedded ? &( pEmbedded->m_dWordforms ) : nullptr, pTokenizer, szIndex );
	if ( m_pWordforms )
	{
		++m_pWordforms->m_iRefCount;
		if ( m_pWordforms->m_bHavePostMorphNF && !m_dMorph.GetLength() )
			sphWarning ( "table '%s': wordforms contain post-morphology normal forms, but no morphology was specified", szIndex );
	}

	return !!m_pWordforms;
}


void TemplateDictTraits_c::WriteWordforms ( CSphWriter& tWriter ) const
{
	if ( !m_pWordforms )
	{
		tWriter.PutDword ( 0 );
		return;
	}

	int nMultiforms = 0;
	if ( m_pWordforms->m_pMultiWordforms )
		for ( const auto& tMF : m_pWordforms->m_pMultiWordforms->m_Hash )
			nMultiforms += tMF.second ? tMF.second->m_pForms.GetLength() : 0;

	tWriter.PutDword ( m_pWordforms->m_hHash.GetLength() + nMultiforms );
	for ( const auto& tForm : m_pWordforms->m_hHash )
	{
		CSphString sLine;
		sLine.SetSprintf ( "%s%s > %s", m_pWordforms->m_dNormalForms[tForm.second].m_bAfterMorphology ? "~" : "", tForm.first.cstr(), m_pWordforms->m_dNormalForms[tForm.second].m_sWord.cstr() );
		tWriter.PutString ( sLine );
	}

	if ( m_pWordforms->m_pMultiWordforms )
	{
		for ( const auto& tMultiForm : m_pWordforms->m_pMultiWordforms->m_Hash )
		{
			CSphMultiforms* pMF = tMultiForm.second;
			if ( !pMF )
				continue;

			for ( const auto& i : pMF->m_pForms )
			{
				CSphString sLine, sTokens, sForms;
				ConcatReportStrings ( i->m_dTokens, sTokens );
				ConcatReportStrings ( i->m_dNormalForm, sForms );

				sLine.SetSprintf ( "%s %s > %s", tMultiForm.first.cstr(), sTokens.cstr(), sForms.cstr() );
				tWriter.PutString ( sLine );
			}
		}
	}
}

void TemplateDictTraits_c::WriteWordforms ( JsonEscapedBuilder& tOut ) const
{
	if ( !m_pWordforms )
		return;

	bool bHaveData = ( m_pWordforms->m_hHash.GetLength() != 0 );

	using HASHIT = std::pair<CSphString, CSphMultiforms*>;
	auto& pMulti = m_pWordforms->m_pMultiWordforms; // shortcut
	if ( pMulti )
		bHaveData |= ::any_of ( pMulti->m_Hash, [] ( const HASHIT& tMF ) { return tMF.second && !tMF.second->m_pForms.IsEmpty(); } );

	if ( !bHaveData )
		return;

	tOut.Named ( "word_forms" );
	auto _ = tOut.ArrayW();

	if ( m_pWordforms->m_hHash.GetLength() )
		for ( const auto& tForm : m_pWordforms->m_hHash )
		{
			CSphString sLine;
			sLine.SetSprintf ( "%s%s > %s", m_pWordforms->m_dNormalForms[tForm.second].m_bAfterMorphology ? "~" : "", tForm.first.cstr(), m_pWordforms->m_dNormalForms[tForm.second].m_sWord.cstr() );
			tOut.FixupSpacedAndAppendEscaped ( sLine.cstr() );
		}

	if ( !pMulti )
		return;

	for ( const HASHIT& tForms : pMulti->m_Hash )
	{
		if ( !tForms.second )
			continue;

		for ( const CSphMultiform* pMF : tForms.second->m_pForms )
		{
			CSphString sLine, sTokens, sForms;
			ConcatReportStrings ( pMF->m_dTokens, sTokens );
			ConcatReportStrings ( pMF->m_dNormalForm, sForms );
			sLine.SetSprintf ( "%s %s > %s", tForms.first.cstr(), sTokens.cstr(), sForms.cstr() );
			tOut.FixupSpacedAndAppendEscaped ( sLine.cstr() );
		}
	}
}


int TemplateDictTraits_c::SetMorphology ( const char* szMorph, CSphString& sMessage )
{
	m_dMorph.Reset();
#if WITH_STEMMER
	for ( void* pStemmer : m_dStemmers )
		sb_stemmer_delete ( (sb_stemmer*)pStemmer );
	m_dStemmers.Reset();
#endif

	if ( !szMorph )
		return ST_OK;

	CSphString sOption = szMorph;
	sOption.ToLower();

	CSphString sError;
	int iRes = ParseMorphology ( sOption.cstr(), sMessage );
	if ( iRes == ST_WARNING && sMessage.IsEmpty() )
		sMessage.SetSprintf ( "invalid morphology option %s; skipped", sOption.cstr() );
	return iRes;
}


bool TemplateDictTraits_c::HasMorphology() const
{
	return ( m_dMorph.GetLength() > 0 );
}


/// common id-based stemmer
bool TemplateDictTraits_c::StemById ( BYTE* pWord, int iStemmer ) const
{
	char szBuf[MAX_KEYWORD_BYTES];

	// safe quick strncpy without (!) padding and with a side of strlen
	char* p = szBuf;
	char* pMax = szBuf + sizeof ( szBuf ) - 1;
	BYTE* pLastSBS = nullptr;
	while ( *pWord && p < pMax )
	{
		pLastSBS = ( *pWord ) < 0x80 ? pWord : pLastSBS;
		*p++ = *pWord++;
	}
	int iLen = int ( p - szBuf );
	*p = '\0';
	pWord -= iLen;

	switch ( (EMORPH)iStemmer )
	{
	case EMORPH::STEM_EN:
		stem_en ( pWord, iLen );
		break;

	case EMORPH::STEM_RU_UTF8:
		// skip stemming in case of SBC at the end of the word
		if ( pLastSBS && ( pLastSBS - pWord + 1 ) >= iLen )
			break;

		// stem only UTF8 tail
		if ( !pLastSBS )
		{
			stem_ru_utf8 ( (WORD*)pWord );
		} else
		{
			stem_ru_utf8 ( (WORD*)( pLastSBS + 1 ) );
		}
		break;

	case EMORPH::STEM_CZ:
		stem_cz ( pWord );
		break;

	case EMORPH::STEM_AR_UTF8:
		stem_ar_utf8 ( pWord );
		break;

	case EMORPH::SOUNDEX:
		stem_soundex ( pWord );
		break;

	case EMORPH::METAPHONE_UTF8:
		stem_dmetaphone ( pWord );
		break;

	case EMORPH::AOTLEMMER_RU_UTF8:
		sphAotLemmatizeRuUTF8 ( pWord );
		break;

	case EMORPH::AOTLEMMER_EN:
		sphAotLemmatize ( pWord, AOT_EN );
		break;

	case EMORPH::AOTLEMMER_DE_UTF8:
		sphAotLemmatizeDeUTF8 ( pWord );
		break;

	case EMORPH::AOTLEMMER_UK:
		sphAotLemmatizeUk ( pWord, m_tLemmatizer.get() );
		break;

	case EMORPH::AOTLEMMER_RU_ALL:
	case EMORPH::AOTLEMMER_EN_ALL:
	case EMORPH::AOTLEMMER_DE_ALL:
	case EMORPH::AOTLEMMER_UK_ALL:
		// do the real work somewhere else
		// this is mostly for warning suppressing and making some features like
		// index_exact_words=1 vs expand_keywords=1 work
		break;

	default:
#if WITH_STEMMER
		if ( iStemmer >= (int)EMORPH::LIBSTEMMER_FIRST && iStemmer < (int)EMORPH::LIBSTEMMER_LAST )
		{
			auto* pStemmer = (sb_stemmer*)m_dStemmers[iStemmer - (int)EMORPH::LIBSTEMMER_FIRST];
			assert ( pStemmer );

			const sb_symbol* sStemmed = sb_stemmer_stem ( pStemmer, (sb_symbol*)pWord, (int)strlen ( (const char*)pWord ) );
			int iStemmedLen = sb_stemmer_length ( pStemmer );

			memcpy ( pWord, sStemmed, iStemmedLen );
			pWord[iStemmedLen] = '\0';
		} else
			return false;

		break;
#else
		return false;
#endif
	}

	return strcmp ( (char*)pWord, szBuf ) != 0;
}


void sphShutdownWordforms()
{
	CSphVector<CSphSavedFile> dEmptyFiles;
	TemplateDictTraits_c::SweepWordformContainers ( dEmptyFiles );
}


void SetupLemmatizerBase()
{
	g_sLemmatizerBase = GET_FULL_SHARE_DIR();
}