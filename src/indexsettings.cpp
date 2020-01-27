//
// Copyright (c) 2017-2020, Manticore Software LTD (http://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "indexsettings.h"

#include "sphinxstd.h"
#include "sphinxint.h"
#include "fileutils.h"
#include "sphinxstem.h"
#include "icu.h"
#include "searchdaemon.h"

#if !USE_WINDOWS
	#include <glob.h>
#endif


static const char * sphBigramName ( ESphBigram eType )
{
	switch ( eType )
	{
	case SPH_BIGRAM_ALL:
		return "all";

	case SPH_BIGRAM_FIRSTFREQ:
		return "first_freq";

	case SPH_BIGRAM_BOTHFREQ:
		return "both_freq";

	case SPH_BIGRAM_NONE:
	default:
		return "none";
	}
}


static void DumpKey ( StringBuilder_c & tBuf, const char * sKey, const char * sVal, bool bCond )
{
	if ( bCond )
		tBuf << sKey << " = " << sVal << "\n";
}


static void DumpKey ( StringBuilder_c & tBuf, const char * sKey, int iVal, bool bCond )
{
	if ( bCond )
		tBuf.Appendf ( "%s = %d\n", sKey, iVal );
}


static RtTypedAttr_t g_dRtTypedAttrs[]=
{
	{ SPH_ATTR_INTEGER,		"rt_attr_uint" },
	{ SPH_ATTR_BIGINT,		"rt_attr_bigint" },
	{ SPH_ATTR_TIMESTAMP,	"rt_attr_timestamp" },
	{ SPH_ATTR_BOOL,		"rt_attr_bool" },
	{ SPH_ATTR_FLOAT,		"rt_attr_float" },
	{ SPH_ATTR_STRING,		"rt_attr_string" },
	{ SPH_ATTR_JSON,		"rt_attr_json" },
	{ SPH_ATTR_UINT32SET,	"rt_attr_multi" },
	{ SPH_ATTR_INT64SET,	"rt_attr_multi_64" }
};


int	GetNumRtTypes()
{
	return sizeof(g_dRtTypedAttrs)/sizeof(g_dRtTypedAttrs[0]);
}


const RtTypedAttr_t & GetRtType ( int iType )
{
	return g_dRtTypedAttrs[iType];
}

//////////////////////////////////////////////////////////////////////////

ESphWordpart CSphSourceSettings::GetWordpart ( const char * sField, bool bWordDict )
{
	if ( bWordDict )
		return SPH_WORDPART_WHOLE;

	bool bPrefix = ( m_iMinPrefixLen>0 ) && ( m_dPrefixFields.IsEmpty () || m_dPrefixFields.Contains ( sField ) );
	bool bInfix = ( m_iMinInfixLen>0 ) && ( m_dInfixFields.IsEmpty() || m_dInfixFields.Contains ( sField ) );

	assert ( !( bPrefix && bInfix ) ); // no field must be marked both prefix and infix
	if ( bPrefix )
		return SPH_WORDPART_PREFIX;
	if ( bInfix )
		return SPH_WORDPART_INFIX;
	return SPH_WORDPART_WHOLE;
}

//////////////////////////////////////////////////////////////////////////

void CSphTokenizerSettings::Setup ( const CSphConfigSection & hIndex, CSphString & sWarning )
{
	m_iNgramLen = Max ( hIndex.GetInt ( "ngram_len" ), 0 );

	if ( hIndex ( "ngram_chars" ) )
	{
		if ( m_iNgramLen )
			m_iType = TOKENIZER_NGRAM;
		else
			sWarning = "ngram_chars specified, but ngram_len=0; IGNORED";
	}

	m_sCaseFolding = hIndex.GetStr ( "charset_table" );
	m_iMinWordLen = Max ( hIndex.GetInt ( "min_word_len", 1 ), 1 );
	m_sNgramChars = hIndex.GetStr ( "ngram_chars" );
	m_sSynonymsFile = hIndex.GetStr ( "exceptions" ); // new option name
	m_sIgnoreChars = hIndex.GetStr ( "ignore_chars" );
	m_sBlendChars = hIndex.GetStr ( "blend_chars" );
	m_sBlendMode = hIndex.GetStr ( "blend_mode" );

	// phrase boundaries
	int iBoundaryStep = Max ( hIndex.GetInt ( "phrase_boundary_step" ), -1 );
	if ( iBoundaryStep!=0 )
		m_sBoundary = hIndex.GetStr ( "phrase_boundary" );
}


bool CSphTokenizerSettings::Load ( CSphReader & tReader, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning )
{
	m_iType = tReader.GetByte ();
	if ( m_iType!=TOKENIZER_UTF8 && m_iType!=TOKENIZER_NGRAM )
	{
		sWarning = "can't load an old index with SBCS tokenizer";
		return false;
	}

	m_sCaseFolding = tReader.GetString ();
	m_iMinWordLen = tReader.GetDword ();
	tEmbeddedFiles.m_bEmbeddedSynonyms = false;
	tEmbeddedFiles.m_bEmbeddedSynonyms = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedSynonyms )
	{
		int nSynonyms = (int)tReader.GetDword();
		tEmbeddedFiles.m_dSynonyms.Resize ( nSynonyms );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dSynonyms )
			tEmbeddedFiles.m_dSynonyms[i] = tReader.GetString();
	}

	m_sSynonymsFile = tReader.GetString ();
	ReadFileInfo ( tReader, m_sSynonymsFile.cstr(), false,
		tEmbeddedFiles.m_tSynonymFile, tEmbeddedFiles.m_bEmbeddedSynonyms ? NULL : &sWarning );
	m_sBoundary = tReader.GetString ();
	m_sIgnoreChars = tReader.GetString ();
	m_iNgramLen = tReader.GetDword ();
	m_sNgramChars = tReader.GetString ();
	m_sBlendChars = tReader.GetString ();
	m_sBlendMode = tReader.GetString();

	return true;
}


void CSphTokenizerSettings::Dump ( StringBuilder_c & tBuf ) const
{
	DumpKey ( tBuf, "charset_type",		( m_iType==TOKENIZER_UTF8 || m_iType==TOKENIZER_NGRAM )
		? "utf-8"
		: "unknown tokenizer (deprecated sbcs?)", true );

	DumpKey ( tBuf, "charset_table",	m_sCaseFolding.cstr(),	!m_sCaseFolding.IsEmpty() );
	DumpKey ( tBuf, "min_word_len",		m_iMinWordLen,			m_iMinWordLen>1 );
	DumpKey ( tBuf, "ngram_len",		m_iNgramLen,			m_iNgramLen && !m_sNgramChars.IsEmpty() );
	DumpKey ( tBuf, "ngram_chars",		m_sNgramChars.cstr(),	m_iNgramLen && !m_sNgramChars.IsEmpty() );
	DumpKey ( tBuf, "exceptions",		m_sSynonymsFile.cstr(),	!m_sSynonymsFile.IsEmpty() );
	DumpKey ( tBuf, "phrase_boundary",	m_sBoundary.cstr(),		!m_sBoundary.IsEmpty() );
	DumpKey ( tBuf, "ignore_chars",		m_sIgnoreChars.cstr(),	!m_sIgnoreChars.IsEmpty() );
	DumpKey ( tBuf, "blend_chars",		m_sBlendChars.cstr(),	!m_sBlendChars.IsEmpty() );
	DumpKey ( tBuf, "blend_mode",		m_sBlendMode.cstr(),	!m_sBlendMode.IsEmpty() );
}


void CSphTokenizerSettings::DumpCfg ( FILE * fp ) const
{
	fprintf ( fp, "\tcharset_type = %s\n", ( m_iType==TOKENIZER_UTF8 || m_iType==TOKENIZER_NGRAM )
		? "utf-8"
		: "unknown tokenizer (deprecated sbcs?)" );

	if ( !m_sCaseFolding.IsEmpty() )
		fprintf ( fp, "\tcharset_table = %s\n", m_sCaseFolding.cstr() );
	if ( m_iMinWordLen>1 )
		fprintf ( fp, "\tmin_word_len = %d\n", m_iMinWordLen );
	if ( m_iNgramLen && !m_sNgramChars.IsEmpty() )
		fprintf ( fp, "\tngram_len = %d\nngram_chars = %s\n", m_iNgramLen, m_sNgramChars.cstr() );
	if ( !m_sSynonymsFile.IsEmpty() )
		fprintf ( fp, "\texceptions = %s\n", m_sSynonymsFile.cstr() );
	if ( !m_sBoundary.IsEmpty() )
		fprintf ( fp, "\tphrase_boundary = %s\n", m_sBoundary.cstr() );
	if ( !m_sIgnoreChars.IsEmpty() )
		fprintf ( fp, "\tignore_chars = %s\n", m_sIgnoreChars.cstr() );
	if ( !m_sBlendChars.IsEmpty() )
		fprintf ( fp, "\tblend_chars = %s\n", m_sBlendChars.cstr() );
	if ( !m_sBlendMode.IsEmpty() )
		fprintf ( fp, "\tblend_mode = %s\n", m_sBlendMode.cstr() );
}


void CSphTokenizerSettings::DumpReadable ( FILE * fp, const CSphEmbeddedFiles & tEmbeddedFiles ) const
{
	fprintf ( fp, "tokenizer-type: %d\n", m_iType );
	fprintf ( fp, "tokenizer-case-folding: %s\n", m_sCaseFolding.cstr() );
	fprintf ( fp, "tokenizer-min-word-len: %d\n", m_iMinWordLen );
	fprintf ( fp, "tokenizer-ngram-chars: %s\n", m_sNgramChars.cstr() );
	fprintf ( fp, "tokenizer-ngram-len: %d\n", m_iNgramLen );
	fprintf ( fp, "tokenizer-exceptions: %s\n", m_sSynonymsFile.cstr() );
	fprintf ( fp, "tokenizer-phrase-boundary: %s\n", m_sBoundary.cstr() );
	fprintf ( fp, "tokenizer-ignore-chars: %s\n", m_sIgnoreChars.cstr() );
	fprintf ( fp, "tokenizer-blend-chars: %s\n", m_sBlendChars.cstr() );
	fprintf ( fp, "tokenizer-blend-mode: %s\n", m_sBlendMode.cstr() );
	fprintf ( fp, "tokenizer-blend-mode: %s\n", m_sBlendMode.cstr() );

	fprintf ( fp, "dictionary-embedded-exceptions: %d\n", tEmbeddedFiles.m_bEmbeddedSynonyms ? 1 : 0 );
	if ( tEmbeddedFiles.m_bEmbeddedSynonyms )
	{
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dSynonyms )
			fprintf ( fp, "\tdictionary-embedded-exception [%d]: %s\n", i, tEmbeddedFiles.m_dSynonyms[i].cstr() );
	}
}

//////////////////////////////////////////////////////////////////////////

void CSphDictSettings::Setup ( const CSphConfigSection & hIndex, CSphString & sWarning )
{
	m_sMorphology = hIndex.GetStr ( "morphology" );
	m_sMorphFields = hIndex.GetStr ( "morphology_skip_fields" );
	m_sStopwords = hIndex.GetStr ( "stopwords" );
	m_iMinStemmingLen = hIndex.GetInt ( "min_stemming_len", 1 );
	m_bStopwordsUnstemmed = hIndex.GetInt ( "stopwords_unstemmed" )!=0;

	for ( CSphVariant * pWordforms = hIndex("wordforms"); pWordforms; pWordforms = pWordforms->m_pNext )
	{
		if ( !pWordforms->cstr() || !*pWordforms->cstr() )
			continue;

		StrVec_t dFilesFound = FindFiles ( pWordforms->cstr() );

		ARRAY_FOREACH ( i, dFilesFound )
			m_dWordforms.Add ( dFilesFound[i] );
	}

	if ( hIndex("dict") )
	{
		m_bWordDict = true; // default to keywords
		if ( hIndex["dict"]=="crc" )
			m_bWordDict = false;
		else if ( hIndex["dict"]!="keywords" )
			sWarning.SetSprintf ( "WARNING: unknown dict=%s, defaulting to keywords\n", hIndex["dict"].cstr() );
	}
}


void CSphDictSettings::Load ( CSphReader & tReader, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning )
{
	m_sMorphology = tReader.GetString();
	m_sMorphFields = tReader.GetString();

	tEmbeddedFiles.m_bEmbeddedStopwords = false;
	tEmbeddedFiles.m_bEmbeddedStopwords = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedStopwords )
	{
		int nStopwords = (int)tReader.GetDword();
		tEmbeddedFiles.m_dStopwords.Resize ( nStopwords );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dStopwords )
			tEmbeddedFiles.m_dStopwords[i] = (SphWordID_t)tReader.UnzipOffset();
	}

	m_sStopwords = tReader.GetString ();
	int nFiles = tReader.GetDword ();

	CSphString sFile;
	tEmbeddedFiles.m_dStopwordFiles.Resize ( nFiles );
	for ( int i = 0; i < nFiles; i++ )
	{
		sFile = tReader.GetString ();
		ReadFileInfo ( tReader, sFile.cstr(), true, tEmbeddedFiles.m_dStopwordFiles[i], tEmbeddedFiles.m_bEmbeddedSynonyms ? NULL : &sWarning );
	}

	tEmbeddedFiles.m_bEmbeddedWordforms = false;
	tEmbeddedFiles.m_bEmbeddedWordforms = !!tReader.GetByte();
	if ( tEmbeddedFiles.m_bEmbeddedWordforms )
	{
		int nWordforms = (int)tReader.GetDword();
		tEmbeddedFiles.m_dWordforms.Resize ( nWordforms );
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dWordforms )
			tEmbeddedFiles.m_dWordforms[i] = tReader.GetString();
	}

	m_dWordforms.Resize ( tReader.GetDword() );

	tEmbeddedFiles.m_dWordformFiles.Resize ( m_dWordforms.GetLength() );
	ARRAY_FOREACH ( i, m_dWordforms )
	{
		m_dWordforms[i] = tReader.GetString();
		ReadFileInfo ( tReader, m_dWordforms[i].cstr(), false,
			tEmbeddedFiles.m_dWordformFiles[i], tEmbeddedFiles.m_bEmbeddedWordforms ? NULL : &sWarning );
	}

	m_iMinStemmingLen = tReader.GetDword ();

	m_bWordDict = ( tReader.GetByte()!=0 );

	m_bStopwordsUnstemmed = ( tReader.GetByte()!=0 );
	m_sMorphFingerprint = tReader.GetString();
}


void CSphDictSettings::Dump ( StringBuilder_c & tBuf ) const
{
	StringBuilder_c tWordforms;
	for ( const auto & dWordform : m_dWordforms )
		tWordforms << " " << dWordform;

	DumpKey ( tBuf, "dict",					"keywords",				m_bWordDict );
	DumpKey ( tBuf, "morphology",			m_sMorphology.cstr(),	!m_sMorphology.IsEmpty() );
	DumpKey ( tBuf, "stopwords",			m_sStopwords.cstr(),	!m_sStopwords.IsEmpty() );
	DumpKey ( tBuf, "wordforms",			tWordforms.cstr()+1,	m_dWordforms.GetLength()>0 );
	DumpKey ( tBuf, "min_stemming_len",		m_iMinStemmingLen,		m_iMinStemmingLen>1 );
	DumpKey ( tBuf, "stopwords_unstemmed",	1,						m_bStopwordsUnstemmed );
}


void CSphDictSettings::DumpCfg ( FILE * fp ) const
{
	if ( m_bWordDict )
		fprintf ( fp, "\tdict = keywords\n" );
	if ( !m_sMorphology.IsEmpty() )
		fprintf ( fp, "\tmorphology = %s\n", m_sMorphology.cstr() );
	if ( !m_sStopwords.IsEmpty() )
		fprintf ( fp, "\tstopwords = %s\n", m_sStopwords.cstr() );
	if ( m_dWordforms.GetLength() )
	{
		fprintf ( fp, "\twordforms =" );
		for ( const auto & i : m_dWordforms )
			fprintf ( fp, " %s", i.cstr() );
		fprintf ( fp, "\n" );
	}
	if ( m_iMinStemmingLen>1 )
		fprintf ( fp, "\tmin_stemming_len = %d\n", m_iMinStemmingLen );
	if ( m_bStopwordsUnstemmed )
		fprintf ( fp, "\tstopwords_unstemmed = 1\n" );
}


void CSphDictSettings::DumpReadable ( FILE * fp, const CSphEmbeddedFiles & tEmbeddedFiles ) const
{
	fprintf ( fp, "dict: %s\n", m_bWordDict ? "keywords" : "crc" );
	fprintf ( fp, "dictionary-morphology: %s\n", m_sMorphology.cstr() );

	fprintf ( fp, "dictionary-stopwords-file: %s\n", m_sStopwords.cstr() );
	fprintf ( fp, "dictionary-embedded-stopwords: %d\n", tEmbeddedFiles.m_bEmbeddedStopwords ? 1 : 0 );
	if ( tEmbeddedFiles.m_bEmbeddedStopwords )
	{
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dStopwords )
			fprintf ( fp, "\tdictionary-embedded-stopword [%d]: " UINT64_FMT "\n", i, tEmbeddedFiles.m_dStopwords[i] );
	}

	ARRAY_FOREACH ( i, m_dWordforms )
		fprintf ( fp, "dictionary-wordform-file [%d]: %s\n", i, m_dWordforms[i].cstr() );

	fprintf ( fp, "dictionary-embedded-wordforms: %d\n", tEmbeddedFiles.m_bEmbeddedWordforms ? 1 : 0 );
	if ( tEmbeddedFiles.m_bEmbeddedWordforms )
	{
		ARRAY_FOREACH ( i, tEmbeddedFiles.m_dWordforms )
			fprintf ( fp, "\tdictionary-embedded-wordform [%d]: %s\n", i, tEmbeddedFiles.m_dWordforms[i].cstr() );
	}

	fprintf ( fp, "min-stemming-len: %d\n", m_iMinStemmingLen );
	fprintf ( fp, "stopwords-unstemmed: %d\n", m_bStopwordsUnstemmed ? 1 : 0 );
}

//////////////////////////////////////////////////////////////////////////

bool CSphFieldFilterSettings::Setup ( const CSphConfigSection & hIndex, CSphString & sWarning )
{
#if USE_RE2
	// regular expressions
	m_dRegexps.Resize ( 0 );
	for ( CSphVariant * pFilter = hIndex("regexp_filter"); pFilter; pFilter = pFilter->m_pNext )
		m_dRegexps.Add ( pFilter->cstr() );

	return m_dRegexps.GetLength() > 0;
#else
	if ( hIndex ( "regexp_filter" ) )
		sWarning.SetSprintf ( "regexp_filter specified but no regexp support compiled" );

	return false;
#endif
}


void CSphFieldFilterSettings::Load ( CSphReader & tReader )
{
	int nRegexps = tReader.GetDword();
	if ( !nRegexps )
		return;

	m_dRegexps.Resize ( nRegexps );
	for ( auto & i : m_dRegexps )
		i = tReader.GetString();
}


void CSphFieldFilterSettings::Save ( CSphWriter & tWriter ) const
{
	tWriter.PutDword ( m_dRegexps.GetLength() );
	for ( const auto & i : m_dRegexps )
		tWriter.PutString(i);
}


void CSphFieldFilterSettings::Dump ( StringBuilder_c & tBuf ) const
{
	for ( const auto & i : m_dRegexps )
		DumpKey ( tBuf, "regexp_filter", i.cstr(), !i.IsEmpty() );
}


void CSphFieldFilterSettings::DumpCfg ( FILE * pFile ) const
{
	for ( const auto & i : m_dRegexps )
		fprintf ( pFile, "\tregexp_filter = %s\n", i.cstr() );
}


void CSphFieldFilterSettings::DumpReadable ( FILE * pFile ) const
{
	for ( const auto & i : m_dRegexps )
		fprintf ( pFile, "regexp-filter: %s\n", i.cstr() );
}

//////////////////////////////////////////////////////////////////////////

CSphString KillListTarget_t::Format() const
{
	CSphString sTarget, sFlags;

	DWORD uMask = KillListTarget_t::USE_KLIST | KillListTarget_t::USE_DOCIDS;
	if ( (m_uFlags & uMask) != uMask )
	{
		if ( m_uFlags & KillListTarget_t::USE_KLIST )
			sFlags=":kl";

		if ( m_uFlags & KillListTarget_t::USE_DOCIDS )
			sFlags=":id";
	} else
		sFlags = "";

	sTarget.SetSprintf ( "%s%s", m_sIndex.cstr(), sFlags.cstr() );

	return sTarget;
}


bool KillListTargets_t::Parse ( const CSphString & sTargets, const char * szIndexName, CSphString & sError )
{
	StrVec_t dIndexes;
	sphSplit ( dIndexes, sTargets.cstr(), " \t," );

	m_dTargets.Resize(dIndexes.GetLength());
	ARRAY_FOREACH ( i, m_dTargets )
	{
		KillListTarget_t & tTarget = m_dTargets[i];
		const char * szTargetName = dIndexes[i].cstr();
		const char * sSplit = strstr ( szTargetName, ":" );
		if ( sSplit )
		{
			CSphString sOptions = sSplit+1;

			if ( sOptions=="kl" )
				tTarget.m_uFlags = KillListTarget_t::USE_KLIST;
			else if ( sOptions=="id" )
				tTarget.m_uFlags = KillListTarget_t::USE_DOCIDS;
			else
			{
				sError.SetSprintf ( "unknown kill list target option near '%s'\n", dIndexes[i].cstr() );
				return false;
			}

			tTarget.m_sIndex = dIndexes[i].SubString ( 0, sSplit-szTargetName );
		}
		else
			tTarget.m_sIndex = szTargetName;

		if ( tTarget.m_sIndex==szIndexName )
		{
			sError.SetSprintf ( "cannot apply kill list to myself: killlist_target=%s\n", sTargets.cstr() );
			return false;
		}
	}

	return true;
}


void KillListTargets_t::Dump ( StringBuilder_c & tBuf ) const
{
	for ( const auto & tTarget : m_dTargets )
		DumpKey ( tBuf, "killlist_target", tTarget.Format().cstr(), true );
}


void KillListTargets_t::DumpCfg ( FILE * pFile ) const
{
	StringBuilder_c sTargets;
	auto tComma = ScopedComma_c ( sTargets, "," );
	for ( const auto & i : m_dTargets )
		sTargets += i.Format().cstr();

	fprintf ( pFile, "\tkilllist_target = %s\n", sTargets.cstr() );
}


void KillListTargets_t::DumpReadable ( FILE * pFile ) const
{
	for ( const auto & i : m_dTargets )
		fprintf ( pFile, "killlist-target: %s %u\n", i.m_sIndex.cstr(), i.m_uFlags );
}

//////////////////////////////////////////////////////////////////////////

void CSphIndexSettings::ParseStoredFields ( const CSphConfigSection & hIndex )
{
	CSphString sFields = hIndex.GetStr ( "stored_fields" );
	sFields.ToLower();
	sphSplit ( m_dStoredFields, sFields.cstr() );
}


bool CSphIndexSettings::ParseDocstoreSettings ( const CSphConfigSection & hIndex, CSphString & sWarning, CSphString & sError )
{
	m_uBlockSize = hIndex.GetSize ( "docstore_block_size", DEFAULT_DOCSTORE_BLOCK );
	m_iCompressionLevel = hIndex.GetInt ( "docstore_compression_level", DEFAULT_COMPRESSION_LEVEL );

	if ( !hIndex.Exists("docstore_compression") )
		return true;

	CSphString sCompression = hIndex["docstore_compression"].cstr();

	if ( sCompression=="none" )
		m_eCompression = Compression_e::NONE;
	else if ( sCompression=="lz4" )
		m_eCompression = Compression_e::LZ4;
	else if ( sCompression=="lz4hc" )
		m_eCompression = Compression_e::LZ4HC;
	else
	{
		sError.SetSprintf ( "unknown compression specified in 'docstore_compression': '%s'\n", sCompression.cstr() ); 
		return false;
	}

	if ( hIndex.Exists("docstore_compression_level") && m_eCompression!=Compression_e::LZ4HC )
		sWarning.SetSprintf ( "docstore_compression_level works only with LZ4HC compression" ); 

	return true;
}


bool CSphIndexSettings::Setup ( const CSphConfigSection & hIndex, const char * szIndexName, CSphString & sWarning, CSphString & sError )
{
	// misc settings
	m_iMinPrefixLen = Max ( hIndex.GetInt ( "min_prefix_len" ), 0 );
	m_iMinInfixLen = Max ( hIndex.GetInt ( "min_infix_len" ), 0 );
	m_iMaxSubstringLen = Max ( hIndex.GetInt ( "max_substring_len" ), 0 );
	m_iBoundaryStep = Max ( hIndex.GetInt ( "phrase_boundary_step" ), -1 );
	m_bIndexExactWords = hIndex.GetInt ( "index_exact_words" )!=0;
	m_iOvershortStep = Min ( Max ( hIndex.GetInt ( "overshort_step", 1 ), 0 ), 1 );
	m_iStopwordStep = Min ( Max ( hIndex.GetInt ( "stopword_step", 1 ), 0 ), 1 );
	m_iEmbeddedLimit = hIndex.GetSize ( "embedded_limit", 16384 );
	m_bIndexFieldLens = hIndex.GetInt ( "index_field_lengths" )!=0;
	m_sIndexTokenFilter = hIndex.GetStr ( "index_token_filter" );
	m_tBlobUpdateSpace = hIndex.GetSize64 ( "attr_update_reserve", 131072 );

	if ( !m_tKlistTargets.Parse ( hIndex.GetStr ( "killlist_target" ), szIndexName, sError ) )
		return false;

	// prefix/infix fields
	CSphString sFields;

	sFields = hIndex.GetStr ( "prefix_fields" );
	sFields.ToLower();
	sphSplit ( m_dPrefixFields, sFields.cstr() );

	sFields = hIndex.GetStr ( "infix_fields" );
	sFields.ToLower();
	sphSplit ( m_dInfixFields, sFields.cstr() );

	ParseStoredFields(hIndex);

	if ( m_iMinPrefixLen==0 && m_dPrefixFields.GetLength()!=0 )
	{
		sWarning = "WARNING: min_prefix_len=0, prefix_fields ignored\n";
		m_dPrefixFields.Reset();
	}

	if ( m_iMinInfixLen==0 && m_dInfixFields.GetLength()!=0 )
	{
		sWarning = "WARNING: min_infix_len=0, infix_fields ignored\n";
		m_dInfixFields.Reset();
	}

	// the only way we could have both prefixes and infixes enabled is when specific field subsets are configured
	if ( m_iMinInfixLen>0 && m_iMinPrefixLen>0
		&& ( !m_dPrefixFields.GetLength() || !m_dInfixFields.GetLength() ) )
	{
		sError.SetSprintf ( "prefixes and infixes can not both be enabled on all fields" );
		return false;
	}

	m_dPrefixFields.Uniq();
	m_dInfixFields.Uniq();
	m_dStoredFields.Uniq();

	for ( const auto & sField : m_dPrefixFields )
		if ( m_dInfixFields.Contains(sField) )
		{
			sError.SetSprintf ( "field '%s' marked both as prefix and infix", sField.cstr() );
			return false;
		}

	if ( m_iMaxSubstringLen && m_iMaxSubstringLen<m_iMinInfixLen )
	{
		sError.SetSprintf ( "max_substring_len=%d is less than min_infix_len=%d", m_iMaxSubstringLen, m_iMinInfixLen );
		return false;
	}

	if ( m_iMaxSubstringLen && m_iMaxSubstringLen<m_iMinPrefixLen )
	{
		sError.SetSprintf ( "max_substring_len=%d is less than min_prefix_len=%d", m_iMaxSubstringLen, m_iMinPrefixLen );
		return false;
	}

	if ( !ParseDocstoreSettings ( hIndex, sWarning, sError ) )
		return false;

	CSphString sIndexType = hIndex.GetStr ( "dict", "keywords" );

	bool bWordDict = true;
	if ( sIndexType=="crc" )
		bWordDict = false;
	else if ( sIndexType!="keywords" )
	{
		sError.SetSprintf ( "index '%s': unknown dict=%s; only 'keywords' or 'crc' values allowed", szIndexName, sIndexType.cstr() );
		return false;
	}

	if ( hIndex("type") && hIndex["type"]=="rt" && ( m_iMinInfixLen>0 || m_iMinPrefixLen>0 ) && !bWordDict )
	{
		sError.SetSprintf ( "RT indexes support prefixes and infixes with only dict=keywords" );
		return false;
	}

	if ( bWordDict && m_iMaxSubstringLen>0 )
	{
		sError.SetSprintf ( "max_substring_len can not be used with dict=keywords" );
		return false;
	}

	// html stripping
	if ( hIndex ( "html_strip" ) )
	{
		m_bHtmlStrip = hIndex.GetInt ( "html_strip" )!=0;
		m_sHtmlIndexAttrs = hIndex.GetStr ( "html_index_attrs" );
		m_sHtmlRemoveElements = hIndex.GetStr ( "html_remove_elements" );
	}

	// hit format
	// TODO! add the description into documentation.
	m_eHitFormat = SPH_HIT_FORMAT_INLINE;
	if ( hIndex("hit_format") )
	{
		if ( hIndex["hit_format"]=="plain" )		m_eHitFormat = SPH_HIT_FORMAT_PLAIN;
		else if ( hIndex["hit_format"]=="inline" )	m_eHitFormat = SPH_HIT_FORMAT_INLINE;
		else
			sWarning.SetSprintf ( "WARNING: unknown hit_format=%s, defaulting to inline\n", hIndex["hit_format"].cstr() );
	}

	// hit-less indices
	if ( hIndex("hitless_words") )
	{
		const CSphString & sValue = hIndex["hitless_words"].strval();
		if ( sValue=="all" )
			m_eHitless = SPH_HITLESS_ALL;
		else
		{
			m_eHitless = SPH_HITLESS_SOME;
			m_sHitlessFiles = sValue;
		}
	}

	// sentence and paragraph indexing
	m_bIndexSP = ( hIndex.GetInt ( "index_sp" )!=0 );
	m_sZones = hIndex.GetStr ( "index_zones" );

	// bigrams
	m_eBigramIndex = SPH_BIGRAM_NONE;
	if ( hIndex("bigram_index") )
	{
		CSphString s = hIndex["bigram_index"].strval();
		s.ToLower();
		if ( s=="all" )
			m_eBigramIndex = SPH_BIGRAM_ALL;
		else if ( s=="first_freq" )
			m_eBigramIndex = SPH_BIGRAM_FIRSTFREQ;
		else if ( s=="both_freq" )
			m_eBigramIndex = SPH_BIGRAM_BOTHFREQ;
		else
		{
			sError.SetSprintf ( "unknown bigram_index=%s (must be all, first_freq, or both_freq)", s.cstr() );
			return false;
		}
	}

	m_sBigramWords = hIndex.GetStr ( "bigram_freq_words" );
	m_sBigramWords.Trim();

	bool bEmptyOk = m_eBigramIndex==SPH_BIGRAM_NONE || m_eBigramIndex==SPH_BIGRAM_ALL;
	if ( bEmptyOk!=m_sBigramWords.IsEmpty() )
	{
		sError.SetSprintf ( "bigram_index=%s, bigram_freq_words must%s be empty", hIndex["bigram_index"].cstr(),
			bEmptyOk ? "" : " not" );
		return false;
	}

	// aot
	StrVec_t dMorphs;
	sphSplit ( dMorphs, hIndex.GetStr ( "morphology" ) );

	m_uAotFilterMask = 0;
	for ( int j=0; j<AOT_LENGTH; ++j )
	{
		char buf_all[20];
		sprintf ( buf_all, "lemmatize_%s_all", AOT_LANGUAGES[j] ); //NOLINT
		ARRAY_FOREACH ( i, dMorphs )
			if ( dMorphs[i]==buf_all )
			{
				m_uAotFilterMask |= (1UL) << j;
				break;
			}
	}

	m_ePreprocessor = dMorphs.Contains ( "icu_chinese" ) ? Preprocessor_e::ICU : Preprocessor_e::NONE;

	if ( !sphCheckConfigICU ( *this, sError ) )
		return false;

	// all good
	return true;
}


void CSphIndexSettings::Dump ( StringBuilder_c & tBuf ) const
{
	DumpKey ( tBuf, "min_prefix_len",		m_iMinPrefixLen,				m_iMinPrefixLen!=0 );
	DumpKey ( tBuf, "min_infix_len",		m_iMinInfixLen,					m_iMinInfixLen!=0 );
	DumpKey ( tBuf, "max_substring_len",	m_iMaxSubstringLen,				m_iMaxSubstringLen!=0 );
	DumpKey ( tBuf, "index_exact_words",	1,								m_bIndexExactWords );
	DumpKey ( tBuf, "html_strip",			1,								m_bHtmlStrip );
	DumpKey ( tBuf, "html_index_attrs",		m_sHtmlIndexAttrs.cstr(),		!m_sHtmlIndexAttrs.IsEmpty() );
	DumpKey ( tBuf, "html_remove_elements", m_sHtmlRemoveElements.cstr(),	!m_sHtmlRemoveElements.IsEmpty() );
	DumpKey ( tBuf, "index_zones",			m_sZones.cstr(),				!m_sZones.IsEmpty() );
	DumpKey ( tBuf, "index_field_lengths",	1,								m_bIndexFieldLens );
	DumpKey ( tBuf, "index_sp",				1,								m_bIndexSP );
	DumpKey ( tBuf, "phrase_boundary_step",	m_iBoundaryStep,				m_iBoundaryStep!=0 );
	DumpKey ( tBuf, "stopword_step",		m_iStopwordStep,				m_iStopwordStep!=1 );
	DumpKey ( tBuf, "overshort_step",		m_iOvershortStep,				m_iOvershortStep!=1 );
	DumpKey ( tBuf, "bigram_index", sphBigramName ( m_eBigramIndex ),		m_eBigramIndex!=SPH_BIGRAM_NONE );
	DumpKey ( tBuf, "bigram_freq_words",	m_sBigramWords.cstr(),			!m_sBigramWords.IsEmpty() );
	DumpKey ( tBuf, "index_token_filter",	m_sIndexTokenFilter.cstr(),		!m_sIndexTokenFilter.IsEmpty() );
}


void CSphIndexSettings::DumpCfg ( FILE * fp ) const
{
	if ( m_iMinPrefixLen )
		fprintf ( fp, "\tmin_prefix_len = %d\n", m_iMinPrefixLen );
	if ( m_iMinInfixLen )
		fprintf ( fp, "\tmin_prefix_len = %d\n", m_iMinInfixLen );
	if ( m_iMaxSubstringLen )
		fprintf ( fp, "\tmax_substring_len = %d\n", m_iMaxSubstringLen );
	if ( m_bIndexExactWords )
		fprintf ( fp, "\tindex_exact_words = %d\n", m_bIndexExactWords ? 1 : 0 );
	if ( m_bHtmlStrip )
		fprintf ( fp, "\thtml_strip = 1\n" );
	if ( !m_sHtmlIndexAttrs.IsEmpty() )
		fprintf ( fp, "\thtml_index_attrs = %s\n", m_sHtmlIndexAttrs.cstr() );
	if ( !m_sHtmlRemoveElements.IsEmpty() )
		fprintf ( fp, "\thtml_remove_elements = %s\n", m_sHtmlRemoveElements.cstr() );
	if ( !m_sZones.IsEmpty() )
		fprintf ( fp, "\tindex_zones = %s\n", m_sZones.cstr() );
	if ( m_bIndexFieldLens )
		fprintf ( fp, "\tindex_field_lengths = 1\n" );
	if ( m_bIndexSP )
		fprintf ( fp, "\tindex_sp = 1\n" );
	if ( m_iBoundaryStep!=0 )
		fprintf ( fp, "\tphrase_boundary_step = %d\n", m_iBoundaryStep );
	if ( m_iStopwordStep!=1 )
		fprintf ( fp, "\tstopword_step = %d\n", m_iStopwordStep );
	if ( m_iOvershortStep!=1 )
		fprintf ( fp, "\tovershort_step = %d\n", m_iOvershortStep );
	if ( m_eBigramIndex!=SPH_BIGRAM_NONE )
		fprintf ( fp, "\tbigram_index = %s\n", sphBigramName ( m_eBigramIndex ) );
	if ( !m_sBigramWords.IsEmpty() )
		fprintf ( fp, "\tbigram_freq_words = %s\n", m_sBigramWords.cstr() );
	if ( !m_sIndexTokenFilter.IsEmpty() )
		fprintf ( fp, "\tindex_token_filter = %s\n", m_sIndexTokenFilter.cstr() );
}


void CSphIndexSettings::DumpReadable ( FILE * fp ) const
{
	fprintf ( fp, "min-prefix-len: %d\n", m_iMinPrefixLen );
	fprintf ( fp, "min-infix-len: %d\n", m_iMinInfixLen );
	fprintf ( fp, "max-substring-len: %d\n", m_iMaxSubstringLen );
	fprintf ( fp, "exact-words: %d\n", m_bIndexExactWords ? 1 : 0 );
	fprintf ( fp, "html-strip: %d\n", m_bHtmlStrip ? 1 : 0 );
	fprintf ( fp, "html-index-attrs: %s\n", m_sHtmlIndexAttrs.cstr() );
	fprintf ( fp, "html-remove-elements: %s\n", m_sHtmlRemoveElements.cstr() );
	fprintf ( fp, "index-zones: %s\n", m_sZones.cstr() );
	fprintf ( fp, "index-field-lengths: %d\n", m_bIndexFieldLens ? 1 : 0 );
	fprintf ( fp, "index-sp: %d\n", m_bIndexSP ? 1 : 0 );
	fprintf ( fp, "phrase-boundary-step: %d\n", m_iBoundaryStep );
	fprintf ( fp, "stopword-step: %d\n", m_iStopwordStep );
	fprintf ( fp, "overshort-step: %d\n", m_iOvershortStep );
	fprintf ( fp, "bigram-index: %s\n", sphBigramName ( m_eBigramIndex ) );
	fprintf ( fp, "bigram-freq-words: %s\n", m_sBigramWords.cstr() );
	fprintf ( fp, "index-token-filter: %s\n", m_sIndexTokenFilter.cstr() );
}

//////////////////////////////////////////////////////////////////////////

bool CreateTableSettings_t::Load ( CSphReader & tReader )
{
	m_bIfNotExists = !!tReader.GetByte();
	m_dFields.Resize ( tReader.GetDword() );
	for ( auto & i : m_dFields )
	{
		i.m_sName = tReader.GetString();
		i.m_uFieldFlags = tReader.GetDword();
	}

	m_dAttrs.Resize( tReader.GetDword() );
	for ( auto & i : m_dAttrs )
	{
		i.m_sName = tReader.GetString();
		i.m_eAttrType = (ESphAttr)tReader.GetDword();
	}

	m_dOpts.Resize ( tReader.GetDword() );
	for ( auto & i : m_dOpts )
	{
		i.m_sName = tReader.GetString();
		i.m_sValue = tReader.GetString();
	}

	return !tReader.GetErrorFlag();
}


void CreateTableSettings_t::Save ( CSphWriter & tWriter ) const
{
	tWriter.PutByte ( m_bIfNotExists ? 1 : 0  );
	tWriter.PutDword ( m_dFields.GetLength() );
	for ( const auto & i : m_dFields )
	{
		tWriter.PutString ( i.m_sName );
		tWriter.PutDword ( i.m_uFieldFlags );
	}

	tWriter.PutDword ( m_dAttrs.GetLength() );
	for ( const auto & i : m_dAttrs )
	{
		tWriter.PutString ( i.m_sName );
		tWriter.PutDword ( i.m_eAttrType );
	}

	tWriter.PutDword ( m_dOpts.GetLength() );
	for ( const auto & i : m_dOpts )
	{
		tWriter.PutString ( i.m_sName );
		tWriter.PutString ( i.m_sValue );
	}
}

//////////////////////////////////////////////////////////////////////////

void IndexSettingsContainer_c::Populate ( const CreateTableSettings_t & tCreateTable )
{
	StringBuilder_c sStoredFields(",");
	for ( const auto & i : tCreateTable.m_dFields )
	{
		Add ( "rt_field", i.m_sName );

		DWORD uFlags = i.m_uFieldFlags;
		if ( !uFlags )
			uFlags = CSphColumnInfo::FIELD_INDEXED | CSphColumnInfo::FIELD_STORED;

		if ( uFlags & CSphColumnInfo::FIELD_STORED )
			sStoredFields << i.m_sName;
	}

	if ( sStoredFields.GetLength() )
		Add ( "stored_fields", sStoredFields.cstr() );

	for ( const auto & i : tCreateTable.m_dAttrs )
		for ( const auto & j : g_dRtTypedAttrs )
			if ( i.m_eAttrType==j.m_eType )
			{
				CSphString sValue;
				if ( i.m_eAttrType==SPH_ATTR_INTEGER && i.m_tLocator.m_iBitCount!=-1 )
					sValue.SetSprintf ( "%s:%d", i.m_sName.cstr(), i.m_tLocator.m_iBitCount );
				else
					sValue = i.m_sName;

				Add ( j.m_szName, sValue );
				break;
			}

	for ( const auto & i : tCreateTable.m_dOpts )
		Add ( i.m_sName.cstr(), i.m_sValue );
}


void IndexSettingsContainer_c::Add ( const char * szName, const CSphString & sValue )
{
	// same behavior as an ordinary config parser
	m_hCfg.AddEntry ( szName, sValue.cstr() );
}


const CSphConfigSection & IndexSettingsContainer_c::AsCfg() const
{
	return m_hCfg;
}

//////////////////////////////////////////////////////////////////////////

static void WriteFileInfo ( CSphWriter & tWriter, const CSphSavedFile & tInfo )
{
	tWriter.PutOffset ( tInfo.m_uSize );
	tWriter.PutOffset ( tInfo.m_uCTime );
	tWriter.PutOffset ( tInfo.m_uMTime );
	tWriter.PutDword ( tInfo.m_uCRC32 );
}


/// gets called from and MUST be in sync with RtIndex_c::SaveDiskHeader()!
/// note that SaveDiskHeader() occasionaly uses some PREVIOUS format version!
void SaveTokenizerSettings ( CSphWriter & tWriter, const ISphTokenizer * pTokenizer, int iEmbeddedLimit )
{
	assert ( pTokenizer );

	const CSphTokenizerSettings & tSettings = pTokenizer->GetSettings ();
	tWriter.PutByte ( tSettings.m_iType );
	tWriter.PutString ( tSettings.m_sCaseFolding.cstr() );
	tWriter.PutDword ( tSettings.m_iMinWordLen );

	bool bEmbedSynonyms = ( iEmbeddedLimit>0 && pTokenizer->GetSynFileInfo ().m_uSize<=(SphOffset_t)iEmbeddedLimit );
	tWriter.PutByte ( bEmbedSynonyms ? 1 : 0 );
	if ( bEmbedSynonyms )
		pTokenizer->WriteSynonyms ( tWriter );

	tWriter.PutString ( tSettings.m_sSynonymsFile.cstr() );
	WriteFileInfo ( tWriter, pTokenizer->GetSynFileInfo () );
	tWriter.PutString ( tSettings.m_sBoundary.cstr() );
	tWriter.PutString ( tSettings.m_sIgnoreChars.cstr() );
	tWriter.PutDword ( tSettings.m_iNgramLen );
	tWriter.PutString ( tSettings.m_sNgramChars.cstr() );
	tWriter.PutString ( tSettings.m_sBlendChars.cstr() );
	tWriter.PutString ( tSettings.m_sBlendMode.cstr() );
}


/// gets called from and MUST be in sync with RtIndex_c::SaveDiskHeader()!
/// note that SaveDiskHeader() occasionaly uses some PREVIOUS format version!
void SaveDictionarySettings ( CSphWriter & tWriter, const CSphDict * pDict, bool bForceWordDict, int iEmbeddedLimit )
{
	assert ( pDict );
	const CSphDictSettings & tSettings = pDict->GetSettings ();

	tWriter.PutString ( tSettings.m_sMorphology.cstr() );
	tWriter.PutString ( tSettings.m_sMorphFields.cstr() );

	const CSphVector <CSphSavedFile> & dSWFileInfos = pDict->GetStopwordsFileInfos ();
	SphOffset_t uTotalSize = 0;
	ARRAY_FOREACH ( i, dSWFileInfos )
		uTotalSize += dSWFileInfos[i].m_uSize;

	// embed only in case it allowed
	bool bEmbedStopwords = ( iEmbeddedLimit>0 && uTotalSize<=(SphOffset_t)iEmbeddedLimit );
	tWriter.PutByte ( bEmbedStopwords ? 1 : 0 );
	if ( bEmbedStopwords )
		pDict->WriteStopwords ( tWriter );

	tWriter.PutString ( tSettings.m_sStopwords.cstr() );
	tWriter.PutDword ( dSWFileInfos.GetLength () );
	ARRAY_FOREACH ( i, dSWFileInfos )
	{
		tWriter.PutString ( dSWFileInfos[i].m_sFilename.cstr() );
		WriteFileInfo ( tWriter, dSWFileInfos[i] );
	}

	const CSphVector <CSphSavedFile> & dWFFileInfos = pDict->GetWordformsFileInfos ();
	uTotalSize = 0;
	ARRAY_FOREACH ( i, dWFFileInfos )
		uTotalSize += dWFFileInfos[i].m_uSize;

	bool bEmbedWordforms = uTotalSize<=(SphOffset_t)iEmbeddedLimit;
	tWriter.PutByte ( bEmbedWordforms ? 1 : 0 );
	if ( bEmbedWordforms )
		pDict->WriteWordforms ( tWriter );

	tWriter.PutDword ( dWFFileInfos.GetLength() );
	ARRAY_FOREACH ( i, dWFFileInfos )
	{
		tWriter.PutString ( dWFFileInfos[i].m_sFilename.cstr() );
		WriteFileInfo ( tWriter, dWFFileInfos[i] );
	}

	tWriter.PutDword ( tSettings.m_iMinStemmingLen );
	tWriter.PutByte ( tSettings.m_bWordDict || bForceWordDict );
	tWriter.PutByte ( tSettings.m_bStopwordsUnstemmed );
	tWriter.PutString ( pDict->GetMorphDataFingerprint() );
}

//////////////////////////////////////////////////////////////////////////

bool sphFixupIndexSettings ( CSphIndex * pIndex, const CSphConfigSection & hIndex, CSphString & sError, /*bool bTemplateDict, */bool bStripPath )
{
	bool bTokenizerSpawned = false;
	//bool bTemplateDict = false; // was param, but was never used

	if ( !pIndex->GetTokenizer () )
	{
		CSphTokenizerSettings tSettings;
		CSphString sWarning;
		tSettings.Setup ( hIndex, sWarning );
		if ( !sWarning.IsEmpty() )
			sphWarning ( "index '%s': %s", pIndex->GetName(), sWarning.cstr() );

		TokenizerRefPtr_c pTokenizer { ISphTokenizer::Create ( tSettings, nullptr, sError ) };
		if ( !pTokenizer )
			return false;

		bTokenizerSpawned = true;
		pIndex->SetTokenizer ( pTokenizer );
	}

	if ( !pIndex->GetDictionary () )
	{
		DictRefPtr_c pDict;
		CSphDictSettings tSettings;
		CSphString sWarning;
		tSettings.Setup ( hIndex, sWarning );
		if ( !sWarning.IsEmpty() )
			sphWarning ( "index '%s': %s", pIndex->GetName(), sWarning.cstr() );

		pDict = sphCreateDictionaryCRC ( tSettings, nullptr, pIndex->GetTokenizer (), pIndex->GetName(), bStripPath, pIndex->GetSettings().m_iSkiplistBlockSize, sError );

		if ( !pDict )
		{
			sphWarning ( "index '%s': %s", pIndex->GetName(), sError.cstr() );
			return false;
		}

		pIndex->SetDictionary ( pDict );
	}

	if ( bTokenizerSpawned )
	{
		TokenizerRefPtr_c pOldTokenizer { pIndex->LeakTokenizer () };
		TokenizerRefPtr_c pMultiTokenizer
		{ ISphTokenizer::CreateMultiformFilter ( pOldTokenizer, pIndex->GetDictionary ()->GetMultiWordforms () ) };
		pIndex->SetTokenizer ( pMultiTokenizer );
	}

	pIndex->SetupQueryTokenizer();

	if ( !pIndex->IsStripperInited () )
	{
		CSphIndexSettings tSettings = pIndex->GetSettings ();

		if ( hIndex ( "html_strip" ) )
		{
			tSettings.m_bHtmlStrip = hIndex.GetInt ( "html_strip" )!=0;
			tSettings.m_sHtmlIndexAttrs = hIndex.GetStr ( "html_index_attrs" );
			tSettings.m_sHtmlRemoveElements = hIndex.GetStr ( "html_remove_elements" );
		}
		tSettings.m_sZones = hIndex.GetStr ( "index_zones" );

		pIndex->Setup ( tSettings );
	}

	if ( !pIndex->GetFieldFilter() )
	{
		FieldFilterRefPtr_c pFieldFilter;
		CSphFieldFilterSettings tFilterSettings;
		if ( tFilterSettings.Setup ( hIndex, sError ) )
			pFieldFilter = sphCreateRegexpFilter ( tFilterSettings, sError );

		if ( !sError.IsEmpty() )
			sphWarning ( "index '%s': %s", pIndex->GetName(), sError.cstr() );

		if ( !sphSpawnFilterICU ( pFieldFilter, pIndex->GetSettings(), pIndex->GetTokenizer()->GetSettings(), pIndex->GetName(), sError ) )
			sphWarning ( "index '%s': %s", pIndex->GetName(), sError.cstr() );

		pIndex->SetFieldFilter ( pFieldFilter );
	}

	// exact words fixup, needed for RT indexes
	// cloned from indexer, remove somehow?
	DictRefPtr_c pDict { pIndex->GetDictionary() };
	SafeAddRef ( pDict );
	assert ( pDict );

	CSphIndexSettings tSettings = pIndex->GetSettings ();
	bool bNeedExact = ( pDict->HasMorphology() || pDict->GetWordformsFileInfos().GetLength() );
	if ( tSettings.m_bIndexExactWords && !bNeedExact )
	{
		tSettings.m_bIndexExactWords = false;
		pIndex->Setup ( tSettings );
		fprintf ( stdout, "WARNING: no morphology, index_exact_words=1 has no effect, ignoring\n" );
	}

	if ( pDict->GetSettings().m_bWordDict && pDict->HasMorphology() &&
		( tSettings.m_iMinPrefixLen || tSettings.m_iMinInfixLen || !pDict->GetSettings().m_sMorphFields.IsEmpty() ) && !tSettings.m_bIndexExactWords )
	{
		tSettings.m_bIndexExactWords = true;
		pIndex->Setup ( tSettings );
		fprintf ( stdout, "WARNING: dict=keywords and prefixes and morphology enabled, forcing index_exact_words=1\n" );
	}

	pIndex->PostSetup();
	return true;
}


static RtTypedAttr_t g_dTypeNames[] =
{
	{ SPH_ATTR_INTEGER,		"integer" },
	{ SPH_ATTR_BIGINT,		"bigint" },
	{ SPH_ATTR_FLOAT,		"float" },
	{ SPH_ATTR_BOOL,		"bool" },
	{ SPH_ATTR_UINT32SET,	"multi" },
	{ SPH_ATTR_INT64SET,	"multi64" },
	{ SPH_ATTR_JSON,		"json" },
	{ SPH_ATTR_STRING,		"string" },
	{ SPH_ATTR_TIMESTAMP,	"timestamp" }
};


static CSphString GetAttrTypeName ( const CSphColumnInfo & tAttr )
{
	if ( tAttr.m_eAttrType==SPH_ATTR_INTEGER && tAttr.m_tLocator.m_iBitCount!=32 )
	{
		CSphString sRes;
		sRes.SetSprintf( "bit(%d)", tAttr.m_tLocator.m_iBitCount );
		return sRes;
	}

	for ( const auto & i : g_dTypeNames )
		if ( tAttr.m_eAttrType==i.m_eType )
			return i.m_szName;

	assert ( 0 && "Internal error: unknown attr type" );
	return "";
}


CSphString BuildCreateTable ( const CSphString & sIndex, const CSphSchema & tSchema )
{
	StringBuilder_c sRes;
	sRes << "CREATE TABLE " << sIndex << " (\n";

	bool bHasAttrs = false;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( sphIsInternalAttr ( tAttr.m_sName ) )
			continue;

		if ( i )
			sRes << ",\n";

		sRes << tAttr.m_sName << " " << GetAttrTypeName(tAttr);
		bHasAttrs = true;
	}

	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
	{
		const CSphColumnInfo & tField = tSchema.GetField(i);

		if ( i || bHasAttrs )
			sRes << ",\n";

		sRes << tField.m_sName << " field";
		if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED )
			sRes << " indexed";

		if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
			sRes << " stored";
	}

	sRes << "\n)";

	CSphString sResult = sRes.cstr();
	return sResult;
}
