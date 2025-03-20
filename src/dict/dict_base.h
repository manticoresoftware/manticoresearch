//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org
//

#pragma once

#include "sphinxstd.h"
#include "sphinxdefs.h"
#include "fileio.h"
#include "indexsettings.h"

class CSphWriter;
class JsonEscapedBuilder;
struct CSphEmbeddedFiles;
struct CSphWordforms;
struct CSphMultiformContainer;
struct DictHeader_t;
struct DictEntry_t;

extern CSphString g_sLemmatizerBase;

/////////////////////////////////////////////////////////////////////////////
// DICTIONARIES
/////////////////////////////////////////////////////////////////////////////

/// abstract word dictionary interface
class CSphDict: public ISphRefcountedMT
{
public:
	enum ST_E : int {
		ST_OK = 0,
		ST_ERROR = 1,
		ST_WARNING = 2,
	};

public:
	/// Get word ID by word, "text" version
	/// may apply stemming and modify word inplace
	/// modified word may become bigger than the original one, so make sure you have enough space in buffer which is pointer by pWord
	/// a general practice is to use char[3*SPH_MAX_WORD_LEN+4] as a buffer
	/// returns 0 for stopwords
	virtual SphWordID_t GetWordID ( BYTE* pWord ) = 0;

	/// get word ID by word, "text" version
	/// may apply stemming and modify word inplace
	/// accepts words with already prepended MAGIC_WORD_HEAD
	/// appends MAGIC_WORD_TAIL
	/// returns 0 for stopwords
	virtual SphWordID_t GetWordIDWithMarkers ( BYTE* pWord ) { return GetWordID ( pWord ); }

	/// get word ID by word, "text" version
	/// does NOT apply stemming
	/// accepts words with already prepended MAGIC_WORD_HEAD_NONSTEMMED
	/// returns 0 for stopwords
	virtual SphWordID_t GetWordIDNonStemmed ( BYTE* pWord ) { return GetWordID ( pWord ); }

	/// get word ID by word, "binary" version
	/// only used with prefix/infix indexing
	/// must not apply stemming and modify anything
	/// filters stopwords on request
	virtual SphWordID_t GetWordID ( const BYTE* pWord, int iLen, bool bFilterStops ) = 0;

	/// apply stemmers to the given word
	virtual void ApplyStemmers ( BYTE* ) const {}

	/// load stopwords from given files
	virtual void LoadStopwords ( const char * sFiles, FilenameBuilder_i * pFilenameBuilder, const TokenizerRefPtr_c & pTokenizer, bool bStripFile ) = 0;

	/// load stopwords from an array
	virtual void LoadStopwords ( const CSphVector<SphWordID_t>& dStopwords ) = 0;

	/// write stopwords to a file
	virtual void WriteStopwords ( Writer_i & tWriter ) const = 0;
	virtual void WriteStopwords ( JsonEscapedBuilder& tOut ) const = 0;

	/// load wordforms from a given list of files
	virtual bool LoadWordforms ( const StrVec_t&, const CSphEmbeddedFiles* pEmbedded, const TokenizerRefPtr_c& pTokenizer, const char* szIndex ) = 0;

	/// write wordforms to a file
	virtual void WriteWordforms ( Writer_i & tWriter ) const = 0;
	virtual void WriteWordforms ( JsonEscapedBuilder& tOut ) const = 0;

	/// get wordforms
	virtual const CSphWordforms* GetWordforms() { return nullptr; }

	/// disable wordforms processing
	virtual void DisableWordforms() {}

	/// set morphology
	/// returns 0 on success, 1 on hard error, 2 on a warning (see ST_xxx constants)
	virtual int SetMorphology ( const char* szMorph, CSphString& sMessage ) = 0;

	/// are there any morphological processors?
	virtual bool HasMorphology() const { return false; }

	/// morphological data fingerprint (lemmatizer filenames and crc32s)
	virtual const CSphString& GetMorphDataFingerprint() const { return m_sMorphFingerprint; }

	/// setup dictionary using settings
	virtual void Setup ( const CSphDictSettings& tSettings ) = 0;

	/// get dictionary settings
	virtual const CSphDictSettings& GetSettings() const = 0;

	/// stopwords file infos
	virtual const CSphVector<CSphSavedFile>& GetStopwordsFileInfos() const = 0;

	/// wordforms file infos
	virtual const CSphVector<CSphSavedFile>& GetWordformsFileInfos() const = 0;

	/// get multiwordforms
	virtual const CSphMultiformContainer* GetMultiWordforms() const = 0;

	/// check what given word is stopword
	virtual bool IsStopWord ( const BYTE* pWord ) const = 0;

public:
	virtual void SetSkiplistBlockSize ( int iSize ) {}

	/// enable actually collecting keywords (needed for stopwords/wordforms loading)
	virtual void HitblockBegin() {}

	/// callback to let dictionary do hit block post-processing
	virtual void HitblockPatch ( CSphWordHit*, int ) const {}

	/// resolve temporary hit block wide wordid (!) back to keyword
	virtual const char* HitblockGetKeyword ( SphWordID_t ) { return nullptr; }

	/// check current memory usage
	virtual int HitblockGetMemUse() { return 0; }

	/// hit block dismissed
	virtual void HitblockReset() {}

public:
	/// begin creating dictionary file, setup any needed internal structures
	virtual void DictBegin ( CSphAutofile& tTempDict, CSphAutofile& tDict, int iDictLimit );

	/// begin creating dictionary file, assuming sorted entries came (when processing ready dicts like merge, add/delete field, etc.)
	virtual void SortedDictBegin ( CSphAutofile& tDict, int iDictLimit, int iInfixCodepointBytes );

	/// add next keyword entry to final dict
	virtual void DictEntry ( const DictEntry_t& tEntry );

	/// flush last entry
	virtual void DictEndEntries ( SphOffset_t iDoclistOffset );

	/// end indexing, store dictionary and checkpoints
	virtual bool DictEnd ( DictHeader_t* pHeader, int iMemLimit, CSphString& sError );

	/// check whether there were any errors during indexing
	virtual bool DictIsError() const;

public:
	/// check whether this dict is stateful (when it comes to lookups)
	virtual bool HasState() const { return false; }

	/// make a clone
	virtual DictRefPtr_c Clone() const { return nullptr; }

	/// get settings hash
	virtual uint64_t GetSettingsFNV() const = 0;

protected:
	CSphString m_sMorphFingerprint;
};

using DictRefPtr_c = CSphRefcountedPtr<CSphDict>;


class DictStub_c: public CSphDict
{
protected:
	CSphVector<CSphSavedFile> m_dSWFileInfos;
	CSphVector<CSphSavedFile> m_dWFFileInfos;
	CSphDictSettings m_tSettings;

public:
	SphWordID_t GetWordID ( BYTE* ) override { return 0; }
	SphWordID_t GetWordID ( const BYTE*, int, bool ) override { return 0; };
	void LoadStopwords ( const char * , FilenameBuilder_i * , const TokenizerRefPtr_c& , bool ) override {};
	void LoadStopwords ( const CSphVector<SphWordID_t>& ) override {};
	void WriteStopwords ( Writer_i & ) const override {};
	void WriteStopwords ( JsonEscapedBuilder& ) const override {};
	bool LoadWordforms ( const StrVec_t&, const CSphEmbeddedFiles*, const TokenizerRefPtr_c&, const char* ) override { return false; };
	void WriteWordforms ( Writer_i & ) const override {};
	void WriteWordforms ( JsonEscapedBuilder& ) const override {};
	int SetMorphology ( const char*, CSphString& ) override { return ST_OK; }
	void Setup ( const CSphDictSettings& tSettings ) override { m_tSettings = tSettings; };
	const CSphDictSettings& GetSettings() const override { return m_tSettings; }
	const CSphVector<CSphSavedFile>& GetStopwordsFileInfos() const override { return m_dSWFileInfos; }
	const CSphVector<CSphSavedFile>& GetWordformsFileInfos() const override { return m_dWFFileInfos; }
	const CSphMultiformContainer* GetMultiWordforms() const override { return nullptr; };
	bool IsStopWord ( const BYTE* ) const override { return false; };
	uint64_t GetSettingsFNV() const override { return 0; };
};


void SetupDictionary ( DictRefPtr_c& pDict, const CSphDictSettings& tSettings, const CSphEmbeddedFiles* pFiles, const TokenizerRefPtr_c& pTokenizer, const char* szIndex, bool bStripFile, FilenameBuilder_i* pFilenameBuilder, CSphString& sError );

/// returns pDict, if stateless. Or it's clone, if not
DictRefPtr_c GetStatelessDict ( const DictRefPtr_c& pDict );

/// CRC32/FNV64 dictionary factory
DictRefPtr_c sphCreateDictionaryCRC ( const CSphDictSettings& tSettings, const CSphEmbeddedFiles* pFiles, const TokenizerRefPtr_c& pTokenizer, const char* szIndex, bool bStripFile, int iSkiplistBlockSize, FilenameBuilder_i* pFilenameBuilder, CSphString& sError );

/// keyword-storing dictionary factory
DictRefPtr_c sphCreateDictionaryKeywords ( const CSphDictSettings& tSettings, const CSphEmbeddedFiles* pFiles, const TokenizerRefPtr_c& pTokenizer, const char* szIndex, bool bStripFile, int iSkiplistBlockSize, FilenameBuilder_i* pFilenameBuilder, CSphString& sError );

/// exact dict
void SetupExactDict ( DictRefPtr_c& pDict );

/// star dict old format (pre v8)
void SetupStarDictOld ( DictRefPtr_c& pDict );

/// star dict v8
void SetupStarDictV8 ( DictRefPtr_c& pDict, bool bInfixes = true );

/// clear wordform cache
void sphShutdownWordforms();

void SetupLemmatizerBase();