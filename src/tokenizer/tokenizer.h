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

#pragma once

#include "sphinxstd.h"
#include "lowercaser.h"
#include "indexsettings.h"
#include "fileutils.h"

class CSphWriter;
class CSphSchema;
class FilenameBuilder_i;
struct CSphMultiformContainer;
struct CSphEmbeddedFiles;
enum ESphBigram : BYTE;

/////////////////////////////////////////////////////////////////////////////
/// TOKENIZERS
/////////////////////////////////////////////////////////////////////////////


enum ESphTokenizerClone
{
	SPH_CLONE_INDEX,				///< clone tokenizer and set indexing mode
	SPH_CLONE_QUERY,				///< clone tokenizer and set querying mode
	SPH_CLONE_QUERY_WILD_EXACT_JSON,
	SPH_CLONE_QUERY_WILD_EXACT,
	SPH_CLONE_QUERY_WILD_JSON,
	SPH_CLONE_QUERY_WILD,
	SPH_CLONE_QUERY_EXACT_JSON,
	SPH_CLONE_QUERY_EXACT,
	SPH_CLONE_QUERY_,
	SPH_CLONE,	///< just clone 'as is'
};


enum ESphTokenMorph
{
	SPH_TOKEN_MORPH_RAW,			///< no morphology applied, tokenizer does not handle morphology
	SPH_TOKEN_MORPH_ORIGINAL,		///< no morphology applied, but tokenizer handles morphology
	SPH_TOKEN_MORPH_GUESS			///< morphology applied
};

/// generic tokenizer
class ISphTokenizer : public ISphRefcountedMT
{
	/// trivial dtor - inherited from Refcounted
protected:
	~ISphTokenizer() override = default;

public:
	/// set new translation table
	/// returns true on success, false on failure
	virtual bool					SetCaseFolding ( const char * sConfig, CSphString & sError );

	/// add additional character as valid (with folding to itself)
	virtual void					AddPlainChars ( const char* szChars );

	/// add special chars to translation table
	/// updates lowercaser so that these remap to -1
	virtual void					AddSpecials ( const char * sSpecials );

	/// set ignored characters
	virtual bool					SetIgnoreChars ( const char * sIgnored, CSphString & sError );

	/// set n-gram characters (for CJK n-gram indexing)
	virtual bool					SetNgramChars ( const char *, CSphString & ) { return true; }

	/// set n-gram length (for CJK n-gram indexing)
	virtual void					SetNgramLen ( int ) {}

	/// load synonyms list
	virtual bool					LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, StrVec_t & dWarnings, CSphString & sError ) = 0;

	/// write synonyms to file
	virtual void					WriteSynonyms ( Writer_i & tWriter ) const = 0;
	virtual void 					WriteSynonyms ( JsonEscapedBuilder & tOut ) const = 0;

	/// set phrase boundary chars
	virtual bool					SetBoundary ( const char * sConfig, CSphString & sError );

	/// set blended characters
	virtual bool					SetBlendChars ( const char * sConfig, CSphString & sError );

	/// set blended tokens processing mode
	virtual bool					SetBlendMode ( const char * sMode, CSphString & sError );

	/// setup tokenizer using given settings
	virtual void					Setup ( const CSphTokenizerSettings & tSettings );

	/// save tokenizer settings to a stream
	virtual const CSphTokenizerSettings &	GetSettings () const { return m_tSettings; }

	/// get synonym file info
	virtual const CSphSavedFile &	GetSynFileInfo () const { return m_tSynFileInfo; }

public:
	/// pass next buffer
	virtual void					SetBuffer ( const BYTE * sBuffer, int iLength ) = 0;

	/// set current index schema (only intended for the token filter plugins)
	virtual bool					SetFilterSchema ( const CSphSchema &, CSphString & ) { return true; }

	/// set per-document options from INSERT
	virtual bool					SetFilterOptions ( const char *, CSphString & ) { return true; }

	/// notify tokenizer that we now begin indexing a field with a given number (only intended for the token filter plugins)
	virtual void					BeginField ( int ) {}

	/// get next token
	virtual BYTE *					GetToken () = 0;

	/// calc codepoint length
	virtual int						GetCodepointLength ( int iCode ) const = 0;

	/// get max codepoint length
	virtual int						GetMaxCodepointLength () const = 0;

	/// enable indexing-time sentence boundary detection, and paragraph indexing
	virtual bool					EnableSentenceIndexing ( CSphString & sError );

	/// enable zone indexing
	virtual bool					EnableZoneIndexing ( CSphString & sError );

	/// enable tokenized multiform tracking
	virtual void					EnableTokenizedMultiformTracking () {}

	/// get last token length, in codepoints
	virtual int						GetLastTokenLen () const { return m_iLastTokenLen; }

	/// get last token boundary flag (true if there was a boundary before the token)
	virtual bool					GetBoundary () { return m_bTokenBoundary; }

	/// get byte offset of the last boundary character
	virtual int						GetBoundaryOffset () { return m_iBoundaryOffset; }

	/// was last token a special one?
	virtual bool					WasTokenSpecial () { return m_bWasSpecial; }

	virtual bool					WasTokenSynonym () const { return m_bWasSynonym; }

	/// get amount of overshort keywords skipped before this token
	virtual int						GetOvershortCount () { return ( !m_bBlended && m_bBlendedPart ? 0 : m_iOvershortCount ); }

	/// get original tokenized multiform (if any); NULL means there was none
	virtual BYTE *					GetTokenizedMultiform () { return nullptr; }

	/// was last token a part of multi-wordforms destination
	/// head parameter might be useful to distinguish between sequence of different multi-wordforms
	virtual bool					WasTokenMultiformDestination ( bool & bHead, int & iDestCount ) const = 0;

	/// check whether this token is a generated morphological guess
	ESphTokenMorph					GetTokenMorph() const { return m_eTokenMorph; }

	virtual bool					TokenIsBlended () const { return m_bBlended; }
	virtual bool					TokenIsBlendedPart () const { return m_bBlendedPart; }
	virtual int						SkipBlended () { return 0; }

public:
	/// spawn a clone of my own
	virtual TokenizerRefPtr_c		Clone ( ESphTokenizerClone eMode ) const = 0;

	/// start buffer point of last token
	virtual const char *			GetTokenStart () const = 0;

	/// end buffer point of last token (exclusive, ie. *GetTokenEnd() is already NOT part of a token!)
	virtual const char *			GetTokenEnd () const = 0;

	/// current buffer ptr
	virtual const char *			GetBufferPtr () const = 0;

	/// buffer end
	virtual const char *			GetBufferEnd () const = 0;

	/// set new buffer ptr (must be within current bounds)
	virtual void					SetBufferPtr ( const char * sNewPtr ) = 0;

	/// get settings hash
	virtual uint64_t				GetSettingsFNV () const;

	/// if I'm cloned in index, or any kind of query mode
	virtual bool					IsQueryTok() const noexcept = 0;

	/// get (readonly) lowercaser
	const CSphLowercaser &			GetLowercaser() const { assert ( m_pLC ); return *m_pLC; }

	/// set the phrase mode for precessing tokens
	virtual void					SetPhraseMode ( bool bPhrase ) { m_bPhrase = bPhrase; }
	virtual bool					IsPhraseMode () const  { return m_bPhrase; }

protected:
	virtual bool					RemapCharacters ( const char * sConfig, DWORD uFlags, const char * sSource, bool bCanRemap, CSphString & sError );
	virtual bool					AddSpecialsSPZ ( const char * sSpecials, const char * sDirective, CSphString & sError );

protected:

	static const BYTE				BLEND_TRIM_NONE		= 1;
	static const BYTE				BLEND_TRIM_HEAD		= 2;
	static const BYTE				BLEND_TRIM_TAIL		= 4;
	static const BYTE				BLEND_TRIM_BOTH		= 8;
	static const BYTE				BLEND_TRIM_ALL		= 16;

private:
	LowercaserRefcountedConstPtr			m_pLC;						///< my lowercaser
	mutable std::atomic<CSphLowercaser*>	m_pStagingLC {nullptr};		///< preparing my lowercaser.

protected:
	CSphLowercaser &				StagingLowercaser();
	LowercaserRefcountedConstPtr	GetLC() const;
	void							SetLC ( LowercaserRefcountedConstPtr rhs) ;

protected:
	int								m_iLastTokenLen = 0;		///< last token length, in codepoints
	bool							m_bTokenBoundary = false;	///< last token boundary flag (true after boundary codepoint followed by separator)
	bool							m_bBoundary = false;		///< boundary flag (true immediately after boundary codepoint)
	int								m_iBoundaryOffset = 0;		///< boundary character offset (in bytes)
	bool							m_bWasSpecial = false;		///< special token flag
	bool							m_bWasSynonym = false;		///< last token is a synonym token
	int								m_iOvershortCount = 0;		///< skipped overshort tokens count
	ESphTokenMorph					m_eTokenMorph {SPH_TOKEN_MORPH_RAW}; ///< whether last token was a generated morphological guess

	bool							m_bBlended = false;			///< whether last token (as in just returned from GetToken()) was blended
	bool							m_bNonBlended = true;		///< internal, whether there were any normal chars in that blended token
	bool							m_bBlendedPart = false;		///< whether last token is a normal subtoken of a blended token
	bool							m_bBlendAdd = false;		///< whether we have more pending blended variants (of current accumulator) to return
	BYTE							m_uBlendVariants {BLEND_TRIM_NONE};	///< mask of blended variants as requested by blend_mode (see BLEND_TRIM_xxx flags)
	BYTE							m_uBlendVariantsPending = 0;///< mask of pending blended variants (we clear bits as we return variants)
	bool							m_bBlendSkipPure = false;	///< skip purely blended tokens

	bool							m_bShortTokenFilter = false;///< short token filter flag
	bool							m_bDetectSentences = false;	///< should we detect sentence boundaries?

	CSphTokenizerSettings			m_tSettings;				///< tokenizer settings
	CSphSavedFile					m_tSynFileInfo;				///< synonyms file info
	bool							m_bPhrase = false;
};

using TokenizerRefPtr_c = CSphRefcountedPtr<ISphTokenizer>;

namespace Tokenizer {

/// create a tokenizer using the given settings
TokenizerRefPtr_c		Create ( const CSphTokenizerSettings & tSettings, const CSphEmbeddedFiles * pFiles, FilenameBuilder_i * pFilenameBuilder, StrVec_t & dWarnings, CSphString & sError );

/// add multiform filter upon given tokenizer
void AddToMultiformFilterTo ( TokenizerRefPtr_c& pTokenizer, const CSphMultiformContainer* pContainer );

/// add bigram filter upon given tokenizer
void AddBigramFilterTo ( TokenizerRefPtr_c& pTokenizer, ESphBigram eBigramIndex, const CSphString& sBigramWords, CSphString& sError );

/// create a plugin filter
/// sSspec is a library, name, and options specification string, eg "myplugins.dll:myfilter1:arg1=123"
void AddPluginFilterTo ( TokenizerRefPtr_c& pTokenizer, const CSphString & sSpec, CSphString & sError );

namespace Detail {

	/// create UTF-8 tokenizer
	TokenizerRefPtr_c CreateUTF8Tokenizer ( bool bDefaultCharset = true );

	/// create UTF-8 tokenizer with n-grams support (for CJK n-gram indexing)
	TokenizerRefPtr_c CreateUTF8NgramTokenizer ( bool bDefaultCharset = true );

} // namespace Detail
} // namespace Tokenizer
