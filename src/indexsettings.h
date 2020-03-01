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

#ifndef _indexsettings_
#define _indexsettings_

#include "sphinxstd.h"
#include "sphinxutils.h"
#include "fileutils.h"
#include "sphinxexpr.h"

typedef uint64_t SphWordID_t;
STATIC_SIZE_ASSERT ( SphWordID_t, 8 );

class CSphWriter;
class CSphReader;
class FilenameBuilder_i;

enum
{
	// where was TOKENIZER_SBCS=1 once
	TOKENIZER_UTF8 = 2,
	TOKENIZER_NGRAM = 3
};


struct CSphEmbeddedFiles
{
	bool						m_bEmbeddedSynonyms = false;
	bool						m_bEmbeddedStopwords = false;
	bool						m_bEmbeddedWordforms = false;
	CSphSavedFile				m_tSynonymFile;
	StrVec_t					m_dSynonyms;
	CSphVector<CSphSavedFile>	m_dStopwordFiles;
	CSphVector<SphWordID_t>		m_dStopwords;
	StrVec_t					m_dWordforms;
	CSphVector<CSphSavedFile>	m_dWordformFiles;

	void						Reset();
};


class SettingsFormatter_c;

class SettingsWriter_c
{
public:
	virtual			~SettingsWriter_c() {}

	virtual void	Dump ( StringBuilder_c & tBuf, FilenameBuilder_i * pFilenameBuilder ) const;
	virtual void	DumpCfg ( FILE * pFile, FilenameBuilder_i * pFilenameBuilder ) const;
	virtual void	DumpReadable ( FILE * pFile, const CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder ) const;
	virtual void	DumpCreateTable ( StringBuilder_c & tBuf, FilenameBuilder_i * pFilenameBuilder ) const;

protected:
	virtual void	Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * pFilenameBuilder ) const = 0;
};


class CSphTokenizerSettings : public SettingsWriter_c
{
public:
	int				m_iType { TOKENIZER_UTF8 };
	CSphString		m_sCaseFolding;
	int				m_iMinWordLen = 1;
	CSphString		m_sSynonymsFile;
	CSphString		m_sBoundary;
	CSphString		m_sIgnoreChars;
	int				m_iNgramLen = 0;
	CSphString		m_sNgramChars;
	CSphString		m_sBlendChars;
	CSphString		m_sBlendMode;

	void			Setup ( const CSphConfigSection & hIndex, CSphString & sWarning );
	bool			Load ( CSphReader & tReader, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning );

	void			DumpReadable ( FILE * pFile, const CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder ) const override;

protected:
	void			Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * pFilenameBuilder ) const override;
};


class CSphDictSettings : public SettingsWriter_c
{
public:
	CSphString		m_sMorphology;
	CSphString		m_sMorphFields;
	CSphString		m_sStopwords;
	StrVec_t		m_dWordforms;
	int				m_iMinStemmingLen = 1;
	bool			m_bWordDict = true;
	bool			m_bStopwordsUnstemmed = false;
	CSphString		m_sMorphFingerprint;		///< not used for creation; only for a check when loading

	void			Setup ( const CSphConfigSection & hIndex, FilenameBuilder_i * pFilenameBuilder, CSphString & sWarning );
	void			Load ( CSphReader & tReader, CSphEmbeddedFiles & tEmbeddedFiles, CSphString & sWarning );

	void			DumpReadable ( FILE * pFile, const CSphEmbeddedFiles & tEmbeddedFiles, FilenameBuilder_i * pFilenameBuilder ) const override;

protected:
	void			Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * pFilenameBuilder ) const override;
};


class CSphFieldFilterSettings : public SettingsWriter_c
{
public:
	StrVec_t		m_dRegexps;

	bool			Setup  ( const CSphConfigSection & hIndex, CSphString & sWarning );
	void			Load ( CSphReader & tReader );
	void			Save ( CSphWriter & tWriter ) const;

protected:
	void			Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * pFilenameBuilder ) const override;
};


struct KillListTarget_t
{
	enum
	{
		USE_KLIST	= 1 << 0,
		USE_DOCIDS	= 1 << 1
	};

	CSphString		m_sIndex;
	DWORD			m_uFlags {USE_KLIST|USE_DOCIDS};

	CSphString		Format() const;
};


class KillListTargets_c : public SettingsWriter_c
{
public:
	CSphVector<KillListTarget_t>	m_dTargets;

	bool			Parse ( const CSphString & sTargets, const char * szIndexName, CSphString & sError );

protected:
	void			Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * pFilenameBuilder ) const override;
};

/// wordpart processing type
enum ESphWordpart
{
	SPH_WORDPART_WHOLE		= 0,	///< whole-word
	SPH_WORDPART_PREFIX		= 1,	///< prefix
	SPH_WORDPART_INFIX		= 2		///< infix
};

/// indexing-related source settings
/// NOTE, newly added fields should be synced with CSphSource::Setup()
struct CSphSourceSettings
{
	int		m_iMinPrefixLen = 0;		///< min indexable prefix (0 means don't index prefixes)
	int		m_iMinInfixLen = 0;			///< min indexable infix length (0 means don't index infixes)
	int		m_iMaxSubstringLen = 0;		///< max indexable infix and prefix (0 means don't limit infixes and prefixes)
	int		m_iBoundaryStep = 0;		///< additional boundary word position increment
	bool	m_bIndexExactWords = false;	///< exact (non-stemmed) word indexing flag
	int		m_iOvershortStep = 1;		///< position step on overshort token (default is 1)
	int		m_iStopwordStep = 1;		///< position step on stopword token (default is 1)
	bool	m_bIndexSP = false;			///< whether to index sentence and paragraph delimiters
	bool	m_bIndexFieldLens = false;	///< whether to index field lengths

	StrVec_t m_dPrefixFields;		///< list of prefix fields
	StrVec_t m_dInfixFields;		///< list of infix fields
	StrVec_t m_dStoredFields;		///< list of stored fields
	StrVec_t m_dStoredOnlyFields;	///< list of "fields" that are stored but not indexed

	ESphWordpart GetWordpart ( const char * sField, bool bWordDict );
};


enum class Preprocessor_e
{
	NONE,			///< no preprocessor
	ICU				///< ICU chinese preprocessor
};


enum class Compression_e
{
	NONE,
	LZ4,
	LZ4HC
};


const DWORD DEFAULT_DOCSTORE_BLOCK = 16384;
const int DEFAULT_COMPRESSION_LEVEL = 9;

struct DocstoreSettings_t
{
	Compression_e	m_eCompression		= Compression_e::LZ4;
	int				m_iCompressionLevel	= DEFAULT_COMPRESSION_LEVEL;
	DWORD			m_uBlockSize		= DEFAULT_DOCSTORE_BLOCK;
};


enum ESphHitless
{
	SPH_HITLESS_NONE		= 0,	///< all hits are present
	SPH_HITLESS_SOME		= 1,	///< some of the hits might be omitted (check the flag bit)
	SPH_HITLESS_ALL			= 2		///< no hits in this index
};


enum ESphHitFormat
{
	SPH_HIT_FORMAT_PLAIN	= 0,	///< all hits are stored in hitlist
	SPH_HIT_FORMAT_INLINE	= 1		///< hits can be split and inlined into doclist (aka 9-23)
};


enum ESphBigram
{
	SPH_BIGRAM_NONE			= 0,	///< no bigrams
	SPH_BIGRAM_ALL			= 1,	///< index all word pairs
	SPH_BIGRAM_FIRSTFREQ	= 2,	///< only index pairs where one of the words is in a frequent words list
	SPH_BIGRAM_BOTHFREQ		= 3		///< only index pairs where both words are in a frequent words list
};


class CSphIndexSettings : public CSphSourceSettings, public DocstoreSettings_t, public SettingsWriter_c
{
public:
	ESphHitFormat	m_eHitFormat = SPH_HIT_FORMAT_PLAIN;
	bool			m_bHtmlStrip = false;
	CSphString		m_sHtmlIndexAttrs;
	CSphString		m_sHtmlRemoveElements;
	CSphString		m_sZones;
	ESphHitless		m_eHitless = SPH_HITLESS_NONE;
	CSphString		m_sHitlessFiles;
	bool			m_bVerbose = false;
	int				m_iEmbeddedLimit = 0;
	SphOffset_t		m_tBlobUpdateSpace {0};
	int				m_iSkiplistBlockSize {32};

	KillListTargets_c m_tKlistTargets;	///< list of indexes to apply killlist to

	ESphBigram		m_eBigramIndex = SPH_BIGRAM_NONE;
	CSphString		m_sBigramWords;
	StrVec_t		m_dBigramWords;

	DWORD			m_uAotFilterMask = 0;			///< lemmatize_XX_all forces us to transform queries on the index level too
	Preprocessor_e	m_ePreprocessor = Preprocessor_e::NONE;

	CSphString		m_sIndexTokenFilter;	///< indexing time token filter spec string (pretty useless for disk, vital for RT)

	bool			Setup ( const CSphConfigSection & hIndex, const char * szIndexName, CSphString & sWarning, CSphString & sError );

protected:
	void			Format ( SettingsFormatter_c & tOut, FilenameBuilder_i * pFilenameBuilder ) const override;

private:
	void			ParseStoredFields ( const CSphConfigSection & hIndex );
	bool			ParseDocstoreSettings ( const CSphConfigSection & hIndex, CSphString & sWarning, CSphString & sError );
};


struct RtTypedAttr_t
{
	ESphAttr		m_eType;
	const char *	m_szName;
};


int						GetNumRtTypes();
const RtTypedAttr_t &	GetRtType ( int iType );


struct CreateTableSettings_t
{
	CSphString					m_sLike;
	bool						m_bIfNotExists = false;
	CSphVector<CSphColumnInfo>	m_dAttrs;
	CSphVector<CSphColumnInfo>	m_dFields;
	CSphVector<NameValueStr_t>	m_dOpts;
};


class IndexSettingsContainer_c
{
public:
	bool			Populate ( const CreateTableSettings_t & tCreateTable );
	bool			Add ( const char * szName, const CSphString & sValue );
	bool			Add ( const CSphString & sName, const CSphString & sValue );
	CSphString		Get ( const CSphString & sName ) const;
	bool			Contains ( const char * szName ) const;
	void			RemoveKeys ( const CSphString & sName );
	bool			AddOption ( const CSphString & sName, const CSphString & sValue );
	StrVec_t 		GetFiles() const;

	const CSphConfigSection &	AsCfg() const;
	const CSphString &			GetError() const { return m_sError; }

private:
	CSphConfigSection m_hCfg;

	StrVec_t		m_dStopwordFiles;
	StrVec_t		m_dExceptionFiles;
	StrVec_t		m_dWordformFiles;
	CSphString		m_sError;
};


class ISphTokenizer;
class CSphDict;
class ISphFieldFilter;
class CSphIndex;

void		SaveTokenizerSettings ( CSphWriter & tWriter, const ISphTokenizer * pTokenizer, int iEmbeddedLimit );
void		SaveDictionarySettings ( CSphWriter & tWriter, const CSphDict * pDict, bool bForceWordDict, int iEmbeddedLimit );

/// try to set dictionary, tokenizer and misc settings for an index (if not already set)
bool		sphFixupIndexSettings ( CSphIndex * pIndex, const CSphConfigSection & hIndex, CSphString & sError, bool bStripFile=false, FilenameBuilder_i * pFilenameBuilder=nullptr );
CSphString	BuildCreateTable ( const CSphString & sName, const CSphIndex * pIndex, const CSphSchema & tSchema );

// daemon-level callback
using CreateFilenameBuilder_fn = FilenameBuilder_i * (*)( const char * szIndex );
void		SetIndexFilenameBuilder ( CreateFilenameBuilder_fn pBuilder );
CreateFilenameBuilder_fn GetIndexFilenameBuilder();

#endif // _indexsettings_
