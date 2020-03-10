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
#include "attribute.h"

#if !USE_WINDOWS
	#include <glob.h>
#endif


static CreateFilenameBuilder_fn g_fnCreateFilenameBuilder = nullptr;

void SetIndexFilenameBuilder ( CreateFilenameBuilder_fn pBuilder )
{
	g_fnCreateFilenameBuilder = pBuilder;
}


CreateFilenameBuilder_fn GetIndexFilenameBuilder()
{
	return g_fnCreateFilenameBuilder;
}


static const char * BigramName ( ESphBigram eType )
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


//////////////////////////////////////////////////////////////////////////

class SettingsFormatter_c
{
public:
				SettingsFormatter_c ( FILE * pFile, const char * szPrefix, const char * szEq, const char * szPostfix, bool bIgnoreConf = false );
				SettingsFormatter_c ( StringBuilder_c & tBuf, const char * szPrefix, const char * szEq, const char * szPostfix, bool bIgnoreConf = false );

	template <typename T>
	void		Add ( const char * szKey, T tVal, bool bCond );

	template <typename T>
	void		AddEmbedded ( const char * szKey, const VecTraits_T<T> & dEmbedded, bool bCond );

private:
	FILE *				m_pFile = nullptr;
	StringBuilder_c *	m_pBuf = nullptr;
	CSphString			m_sPrefix;
	CSphString			m_sEq;
	CSphString			m_sPostfix;
	bool				m_bIgnoreCond = false;
};


SettingsFormatter_c::SettingsFormatter_c ( FILE * pFile, const char * szPrefix, const char * szEq, const char * szPostfix, bool bIgnoreCond )
	: m_pFile		( pFile )
	, m_sPrefix		( szPrefix )
	, m_sEq			( szEq )
	, m_sPostfix	( szPostfix )
	, m_bIgnoreCond	( bIgnoreCond )

{}

SettingsFormatter_c::SettingsFormatter_c ( StringBuilder_c & tBuf, const char * szPrefix, const char * szEq, const char * szPostfix, bool bIgnoreCond )
	: m_pBuf		( &tBuf )
	, m_sPrefix		( szPrefix )
	, m_sEq			( szEq )
	, m_sPostfix	( szPostfix )
	, m_bIgnoreCond	( bIgnoreCond )
{}


template <typename T>
void SettingsFormatter_c::Add ( const char * szKey, T tVal, bool bCond )
{
	if ( !m_bIgnoreCond && !bCond )
		return;

	if ( m_pBuf )
		(*m_pBuf) << m_sPrefix << szKey << m_sEq << tVal << m_sPostfix;

	if ( m_pFile )
	{
		StringBuilder_c tBuilder;
		tBuilder << m_sPrefix << szKey << m_sEq << tVal << m_sPostfix;
		fputs ( tBuilder.cstr(), m_pFile );
	}
}

template <typename T>
void SettingsFormatter_c::AddEmbedded ( const char * szKey, const VecTraits_T<T> & dEmbedded, bool bCond )
{
	CSphString sPlural;
	sPlural.SetSprintf( "%ss", szKey );
	Add ( sPlural.cstr(), bCond ? 1 : 0, true );

	if ( bCond )
	{
		ARRAY_FOREACH ( i, dEmbedded )
		{
			CSphString sName;
			sName.SetSprintf ( "%s [%d]", szKey, i );
			Add ( sName.cstr(), dEmbedded[i], true );
		}
	}
}


//////////////////////////////////////////////////////////////////////////

void SettingsWriter_c::Dump ( StringBuilder_c & tBuf, FilenameBuilder_i * pFilenameBuilder ) const
{
	SettingsFormatter_c tFormatter ( tBuf, "", " = ", "\n" );
	Format ( tFormatter, pFilenameBuilder );
}


void SettingsWriter_c::DumpCfg ( FILE * pFile, FilenameBuilder_i * pFilenameBuilder ) const
{
	SettingsFormatter_c tFormatter ( pFile, "\t", " = ", "\n" );
	Format ( tFormatter, pFilenameBuilder );
}


void SettingsWriter_c::DumpReadable ( FILE * pFile, const CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder ) const
{
	SettingsFormatter_c tFormatter ( pFile, "", ": ", "\n", true );
	Format ( tFormatter, pFilenameBuilder );
}


void SettingsWriter_c::DumpCreateTable ( StringBuilder_c & tBuf, FilenameBuilder_i * pFilenameBuilder ) const
{
	SettingsFormatter_c tFormatter ( tBuf, "", "='", "' " );
	Format ( tFormatter, pFilenameBuilder );
}


//////////////////////////////////////////////////////////////////////////

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


static CSphString FormatPath ( const CSphString & sFile, FilenameBuilder_i * pFilenameBuilder )
{
	if ( !pFilenameBuilder || sFile.IsEmpty() )
		return sFile;

	return pFilenameBuilder->GetFullPath(sFile);
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
	tEmbeddedFiles.m_tSynonymFile.Read ( tReader, m_sSynonymsFile.cstr(), false, tEmbeddedFiles.m_bEmbeddedSynonyms ? NULL : &sWarning );
	m_sBoundary = tReader.GetString ();
	m_sIgnoreChars = tReader.GetString ();
	m_iNgramLen = tReader.GetDword ();
	m_sNgramChars = tReader.GetString ();
	m_sBlendChars = tReader.GetString ();
	m_sBlendMode = tReader.GetString();

	return true;
}


void CSphTokenizerSettings::Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * pFilenameBuilder ) const
{
	bool bKnownTokenizer = ( m_iType==TOKENIZER_UTF8 || m_iType==TOKENIZER_NGRAM );
	tOut.Add ( "charset_type",		bKnownTokenizer ? "utf-8" : "unknown tokenizer (deprecated sbcs?)", !bKnownTokenizer );
	tOut.Add ( "charset_table",		m_sCaseFolding,	!m_sCaseFolding.IsEmpty() );
	tOut.Add ( "min_word_len",		m_iMinWordLen,	m_iMinWordLen>1 );
	tOut.Add ( "ngram_len",			m_iNgramLen,	m_iNgramLen && !m_sNgramChars.IsEmpty() );
	tOut.Add ( "ngram_chars",		m_sNgramChars,	m_iNgramLen && !m_sNgramChars.IsEmpty() );
	tOut.Add ( "phrase_boundary",	m_sBoundary,	!m_sBoundary.IsEmpty() );
	tOut.Add ( "ignore_chars",		m_sIgnoreChars,	!m_sIgnoreChars.IsEmpty() );
	tOut.Add ( "blend_chars",		m_sBlendChars,	!m_sBlendChars.IsEmpty() );
	tOut.Add ( "blend_mode",		m_sBlendMode,	!m_sBlendMode.IsEmpty() );

	CSphString sSynonymsFile = FormatPath ( m_sSynonymsFile, pFilenameBuilder );
	tOut.Add ( "exceptions",		sSynonymsFile,	!sSynonymsFile.IsEmpty() );
}


void CSphTokenizerSettings::DumpReadable ( FILE * fp, const CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder ) const
{
	SettingsFormatter_c tFormatter ( fp, "tokenizer-", ": ", "\n", true );
	Format ( tFormatter, pFilenameBuilder );

	tFormatter.AddEmbedded ( "embedded_exception", tEmbeddedFiles.m_dSynonyms, tEmbeddedFiles.m_bEmbeddedSynonyms );
}


//////////////////////////////////////////////////////////////////////////

void CSphDictSettings::Setup ( const CSphConfigSection & hIndex, FilenameBuilder_i * pFilenameBuilder, CSphString & sWarning )
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

		CSphString sWordformFile = FormatPath ( pWordforms->cstr(), pFilenameBuilder );
		StrVec_t dFilesFound = FindFiles ( sWordformFile.cstr() );

		for ( auto & i : dFilesFound )
		{
			if ( pFilenameBuilder )
				StripPath(i);

			m_dWordforms.Add(i);
		}
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
		tEmbeddedFiles.m_dStopwordFiles[i].Read ( tReader, sFile.cstr(), true, tEmbeddedFiles.m_bEmbeddedSynonyms ? NULL : &sWarning );
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
		tEmbeddedFiles.m_dWordformFiles[i].Read ( tReader, m_dWordforms[i].cstr(), false, tEmbeddedFiles.m_bEmbeddedWordforms ? NULL : &sWarning );
	}

	m_iMinStemmingLen = tReader.GetDword ();

	m_bWordDict = ( tReader.GetByte()!=0 );

	m_bStopwordsUnstemmed = ( tReader.GetByte()!=0 );
	m_sMorphFingerprint = tReader.GetString();
}


void CSphDictSettings::Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * pFilenameBuilder ) const
{
	tOut.Add ( "dict",					m_bWordDict ? "keywords" : "crc", !m_bWordDict );
	tOut.Add ( "morphology",			m_sMorphology,		!m_sMorphology.IsEmpty() );
	tOut.Add ( "min_stemming_len",		m_iMinStemmingLen,	m_iMinStemmingLen>1 );
	tOut.Add ( "stopwords_unstemmed",	1,					m_bStopwordsUnstemmed );

	CSphString sStopwordsFile = FormatPath ( m_sStopwords, pFilenameBuilder );
	tOut.Add ( "stopwords",				sStopwordsFile,		!sStopwordsFile.IsEmpty() );

	for ( const auto & i : m_dWordforms )
	{
		CSphString sWordformsFile = FormatPath ( i, pFilenameBuilder );
		tOut.Add ( "wordforms",	sWordformsFile, !sWordformsFile.IsEmpty() );
	}
}


void CSphDictSettings::DumpReadable ( FILE * fp, const CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder ) const
{
	SettingsFormatter_c tFormatter ( fp, "dictionary-", ": ", "\n", true );
	Format ( tFormatter, pFilenameBuilder );

	tFormatter.AddEmbedded ( "embedded_stopword", tEmbeddedFiles.m_dStopwords, tEmbeddedFiles.m_bEmbeddedStopwords );
	tFormatter.AddEmbedded ( "embedded_wordform", tEmbeddedFiles.m_dWordforms, tEmbeddedFiles.m_bEmbeddedWordforms );
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


void CSphFieldFilterSettings::Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * /*pFilenameBuilder*/ ) const
{
	for ( const auto & i : m_dRegexps )
		tOut.Add ( "regexp_filter", i, !i.IsEmpty() );
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


bool KillListTargets_c::Parse ( const CSphString & sTargets, const char * szIndexName, CSphString & sError )
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


void KillListTargets_c::Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * /*pFilenameBuilder*/ ) const
{
	StringBuilder_c sTargets;
	auto tComma = ScopedComma_c ( sTargets, "," );
	for ( const auto & i : m_dTargets )
		sTargets += i.Format().cstr();

	tOut.Add ( "killlist_target", sTargets.cstr(), !sTargets.IsEmpty() );
}

//////////////////////////////////////////////////////////////////////////

void CSphIndexSettings::ParseStoredFields ( const CSphConfigSection & hIndex )
{
	CSphString sFields = hIndex.GetStr ( "stored_fields" );
	sFields.ToLower();
	sphSplit ( m_dStoredFields, sFields.cstr() );
	m_dStoredFields.Uniq();

	sFields = hIndex.GetStr ( "stored_only_fields" );
	sFields.ToLower();
	sphSplit ( m_dStoredOnlyFields, sFields.cstr() );
	m_dStoredOnlyFields.Uniq();
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


void CSphIndexSettings::Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * /*pFilenameBuilder*/ ) const
{
	tOut.Add ( "min_prefix_len",		m_iMinPrefixLen,		m_iMinPrefixLen!=0 );
	tOut.Add ( "min_infix_len",			m_iMinInfixLen,			m_iMinInfixLen!=0 );
	tOut.Add ( "max_substring_len",		m_iMaxSubstringLen,		m_iMaxSubstringLen!=0 );
	tOut.Add ( "index_exact_words",		1,						m_bIndexExactWords );
	tOut.Add ( "html_strip",			1,						m_bHtmlStrip );
	tOut.Add ( "html_index_attrs",		m_sHtmlIndexAttrs,		!m_sHtmlIndexAttrs.IsEmpty() );
	tOut.Add ( "html_remove_elements",	m_sHtmlRemoveElements,	!m_sHtmlRemoveElements.IsEmpty() );
	tOut.Add ( "index_zones",			m_sZones,				!m_sZones.IsEmpty() );
	tOut.Add ( "index_field_lengths",	1,						m_bIndexFieldLens );
	tOut.Add ( "index_sp",				1,						m_bIndexSP );
	tOut.Add ( "phrase_boundary_step",	m_iBoundaryStep,		m_iBoundaryStep!=0 );
	tOut.Add ( "stopword_step",			m_iStopwordStep,		m_iStopwordStep!=1 );
	tOut.Add ( "overshort_step",		m_iOvershortStep,		m_iOvershortStep!=1 );
	tOut.Add ( "bigram_index",			BigramName(m_eBigramIndex), m_eBigramIndex!=SPH_BIGRAM_NONE );
	tOut.Add ( "bigram_freq_words",		m_sBigramWords,			!m_sBigramWords.IsEmpty() );
	tOut.Add ( "index_token_filter",	m_sIndexTokenFilter,	!m_sIndexTokenFilter.IsEmpty() );
}

//////////////////////////////////////////////////////////////////////////

static StrVec_t SplitArg ( const CSphString & sValue, StrVec_t & dFiles )
{
	StrVec_t dValues = sphSplit ( sValue.cstr(), sValue.Length(), " \t," );
	for ( auto & i : dValues )
	{
		dFiles.Add ( i.Trim() );
		StripPath(i);
	}

	return dValues;
}


bool IndexSettingsContainer_c::AddOption ( const CSphString & sName, const CSphString & sValue )
{
	if ( sName=="type" && sValue=="pq" )
	{
		CSphString sNewValue = "percolate";
		return Add ( sName, sNewValue );
	}

	if ( sName=="stopwords" )
	{
		RemoveKeys(sName);
		m_dStopwordFiles.Reset();

		StrVec_t dValues = SplitArg ( sValue, m_dStopwordFiles );

		// m_dStopwordFiles now holds the files with olds paths
		// there's a hack in stopword loading code that modifies the folder to pre-installed
		// let's use this hack here and copy that file too so that we're fully standalone
		for ( auto & i : m_dStopwordFiles )
			if ( !sphIsReadable(i) )
			{
				CSphString sFilename = i;
				sFilename.SetSprintf ( "%s/stopwords/%s", FULL_SHARE_DIR, StripPath(sFilename).cstr() );
				if ( sphIsReadable ( sFilename.cstr() ) )
					i = sFilename;
			}

		StringBuilder_c sNewValue = " ";
		for ( auto & i : dValues )
			sNewValue << i;

		return Add ( sName, sNewValue.cstr() );
	}

	if ( sName=="exceptions" )
	{
		RemoveKeys(sName);
		m_dExceptionFiles.Reset();

		StrVec_t dValues = SplitArg ( sValue, m_dExceptionFiles );
		if ( dValues.GetLength()>1 )
		{
			m_sError = "'exceptions' options only supports a single file";
			return false;
		}

		return Add ( sName, dValues[0] );
	}

	if ( sName=="wordforms" )
	{
		RemoveKeys(sName);
		m_dWordformFiles.Reset();

		StrVec_t dValues = SplitArg ( sValue, m_dWordformFiles );
		for ( auto & i : dValues )
			Add ( sName, i );

		return true;
	}

	return Add ( sName, sValue );
}


void IndexSettingsContainer_c::RemoveKeys ( const CSphString & sName )
{
	m_hCfg.Delete(sName);
}


bool IndexSettingsContainer_c::Populate ( const CreateTableSettings_t & tCreateTable )
{
	StringBuilder_c sStoredFields(",");
	StringBuilder_c sStoredOnlyFields(",");
	for ( const auto & i : tCreateTable.m_dFields )
	{
		Add ( "rt_field", i.m_sName );

		DWORD uFlags = i.m_uFieldFlags;
		if ( !uFlags )
			uFlags = CSphColumnInfo::FIELD_INDEXED | CSphColumnInfo::FIELD_STORED;

		if ( uFlags==CSphColumnInfo::FIELD_STORED )
			sStoredOnlyFields << i.m_sName;
		else if ( uFlags & CSphColumnInfo::FIELD_STORED )
			sStoredFields << i.m_sName;
	}

	if ( sStoredFields.GetLength() )
		Add ( "stored_fields", sStoredFields.cstr() );

	if ( sStoredOnlyFields.GetLength() )
		Add ( "stored_only_fields", sStoredOnlyFields.cstr() );

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
		if ( !AddOption ( i.m_sName, i.m_sValue ) )
			return false;

	if ( !Contains("type") )
		Add ( "type", "rt" );


	bool bDistributed = Get("type")=="distributed";
	if ( !bDistributed )
		Add ( "embedded_limit", "0" );

	SetDefaults();

	return true;
}


bool IndexSettingsContainer_c::Add ( const char * szName, const CSphString & sValue )
{
	// same behavior as an ordinary config parser
	m_hCfg.AddEntry ( szName, sValue.cstr() );
	return true;
}


bool IndexSettingsContainer_c::Add ( const CSphString & sName, const CSphString & sValue )
{
	return Add ( sName.cstr(), sValue );
}


CSphString IndexSettingsContainer_c::Get ( const CSphString & sName ) const
{
	if ( !Contains ( sName.cstr() ) )
		return "";

	return m_hCfg[sName].strval();
}


bool IndexSettingsContainer_c::Contains ( const char * szName ) const
{
	return m_hCfg.Exists(szName);
}


StrVec_t IndexSettingsContainer_c::GetFiles() const
{
	StrVec_t dFiles;
	for ( const auto & i : m_dStopwordFiles )
		dFiles.Add(i);

	for ( const auto & i : m_dExceptionFiles )
		dFiles.Add(i);

	for ( const auto & i : m_dWordformFiles )
		dFiles.Add(i);

	return dFiles;
}


const CSphConfigSection & IndexSettingsContainer_c::AsCfg() const
{
	return m_hCfg;
}

// TODO: read defaults from file or predefined templates
static std::pair<const char* , const char *> g_dIndexSettingsDefaults[] =
{
	{ "charset_table", "non_cjk" }
};

void IndexSettingsContainer_c::SetDefaults()
{
	for ( const auto & tItem : g_dIndexSettingsDefaults )
	{
		if ( !m_hCfg.Exists ( tItem.first ) )
			Add ( tItem.first, tItem.second );
	}
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
		tWriter.PutString ( tSettings.m_dWordforms[i] );
		WriteFileInfo ( tWriter, dWFFileInfos[i] );
	}

	tWriter.PutDword ( tSettings.m_iMinStemmingLen );
	tWriter.PutByte ( tSettings.m_bWordDict || bForceWordDict );
	tWriter.PutByte ( tSettings.m_bStopwordsUnstemmed );
	tWriter.PutString ( pDict->GetMorphDataFingerprint() );
}

//////////////////////////////////////////////////////////////////////////

static void AddWarning ( StrVec_t & dWarnings, const CSphString & sWarning )
{
	if ( !sWarning.IsEmpty() )
		dWarnings.Add(sWarning);
}


bool sphFixupIndexSettings ( CSphIndex * pIndex, const CSphConfigSection & hIndex, bool bStripPath, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings, CSphString & sError )
{
	bool bTokenizerSpawned = false;

	if ( !pIndex->GetTokenizer () )
	{
		CSphTokenizerSettings tSettings;
		CSphString sWarning;
		tSettings.Setup ( hIndex, sWarning );
		AddWarning ( dWarnings, sWarning );

		TokenizerRefPtr_c pTokenizer { ISphTokenizer::Create ( tSettings, nullptr, pFilenameBuilder, dWarnings, sError ) };
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
		tSettings.Setup ( hIndex, pFilenameBuilder, sWarning );
		AddWarning ( dWarnings, sWarning );

		pDict = sphCreateDictionaryCRC ( tSettings, nullptr, pIndex->GetTokenizer (), pIndex->GetName(), bStripPath, pIndex->GetSettings().m_iSkiplistBlockSize, pFilenameBuilder, sError );
		if ( !pDict )
			return false;

		pIndex->SetDictionary ( pDict );
	}

	if ( bTokenizerSpawned )
	{
		TokenizerRefPtr_c pOldTokenizer { pIndex->LeakTokenizer () };
		TokenizerRefPtr_c pMultiTokenizer { ISphTokenizer::CreateMultiformFilter ( pOldTokenizer, pIndex->GetDictionary ()->GetMultiWordforms () ) };
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
		bool bSetupOk = tFilterSettings.Setup ( hIndex, sError );

		// treat warnings as errors
		if ( !sError.IsEmpty() )
			return false;

		if ( bSetupOk )
		{
			CSphString sWarning;
			pFieldFilter = sphCreateRegexpFilter ( tFilterSettings, sWarning );
			AddWarning ( dWarnings, sWarning );
		}

		CSphString sWarning;
		sphSpawnFilterICU ( pFieldFilter, pIndex->GetSettings(), pIndex->GetTokenizer()->GetSettings(), pIndex->GetName(), sWarning );
		AddWarning ( dWarnings, sWarning );

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
		dWarnings.Add ( "no morphology, index_exact_words=1 has no effect, ignoring" );
	}

	if ( pDict->GetSettings().m_bWordDict && pDict->HasMorphology() &&
		( tSettings.m_iMinPrefixLen || tSettings.m_iMinInfixLen || !pDict->GetSettings().m_sMorphFields.IsEmpty() ) && !tSettings.m_bIndexExactWords )
	{
		tSettings.m_bIndexExactWords = true;
		pIndex->Setup ( tSettings );
		dWarnings.Add ( "dict=keywords and prefixes and morphology enabled, forcing index_exact_words=1" );
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


CSphString BuildCreateTable ( const CSphString & sName, const CSphIndex * pIndex, const CSphSchema & tSchema )
{
	assert ( pIndex );

	StringBuilder_c sRes;
	sRes << "CREATE TABLE " << sName << " (\n";

	bool bHasAttrs = false;
	for ( int i = 0; i < tSchema.GetAttrsCount(); i++ )
	{
		const CSphColumnInfo & tAttr = tSchema.GetAttr(i);
		if ( sphIsInternalAttr ( tAttr.m_sName ) || tAttr.m_sName==sphGetDocidName() )
			continue;

		if ( bHasAttrs )
			sRes << ",\n";

		sRes << tAttr.m_sName << " " << GetAttrTypeName(tAttr);
		bHasAttrs = true;
	}

	for ( int i = 0; i < tSchema.GetFieldsCount(); i++ )
	{
		const CSphColumnInfo & tField = tSchema.GetField(i);

		if ( i || bHasAttrs )
			sRes << ",\n";

		sRes << tField.m_sName << " text";
		if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_INDEXED )
			sRes << " indexed";

		if ( tField.m_uFieldFlags & CSphColumnInfo::FIELD_STORED )
			sRes << " stored";
	}

	sRes << "\n)";

	StringBuilder_c tBuf;

	CSphScopedPtr<FilenameBuilder_i> pFilenameBuilder ( g_fnCreateFilenameBuilder ? g_fnCreateFilenameBuilder ( pIndex->GetName() ) : nullptr );
	pIndex->GetSettings().DumpCreateTable ( tBuf, pFilenameBuilder.Ptr() );

	CSphFieldFilterSettings tFieldFilter;
	pIndex->GetFieldFilterSettings ( tFieldFilter );
	tFieldFilter.DumpCreateTable ( tBuf, pFilenameBuilder.Ptr() );

	KillListTargets_c tKlistTargets;
	// fixme! handle errors
	CSphString sError;
	if ( !pIndex->LoadKillList ( nullptr, tKlistTargets, sError ) )
		tKlistTargets.m_dTargets.Reset();

	tKlistTargets.DumpCreateTable ( tBuf, pFilenameBuilder.Ptr() );

	if ( pIndex->GetTokenizer() )
		pIndex->GetTokenizer()->GetSettings().DumpCreateTable ( tBuf, pFilenameBuilder.Ptr() );

	if ( pIndex->GetDictionary() )
		pIndex->GetDictionary()->GetSettings().DumpCreateTable ( tBuf, pFilenameBuilder.Ptr() );

	if ( tBuf.GetLength() )
		sRes << " " << tBuf.cstr();

	CSphString sResult = sRes.cstr();
	return sResult;
}
