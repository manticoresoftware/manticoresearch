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

#include "tokenizer.h"

//////////////////////////////////////////////////////////////////////////
// TOKEN FILTER
//////////////////////////////////////////////////////////////////////////


/// token filter base (boring proxy stuff)
class CSphTokenFilter : public ISphTokenizer
{
protected:
	TokenizerRefPtr_c		m_pTokenizer;

public:
	explicit				CSphTokenFilter ( TokenizerRefPtr_c pTokenizer )						: m_pTokenizer { std::move ( pTokenizer ) } {}
	bool					SetCaseFolding ( const char * sConfig, CSphString & sError ) override	{ return m_pTokenizer->SetCaseFolding ( sConfig, sError ); }
	void					AddPlainChars ( const char * szChars ) override							{ m_pTokenizer->AddPlainChars ( szChars ); }
	void					AddSpecials ( const char * sSpecials ) override							{ m_pTokenizer->AddSpecials ( sSpecials ); }
	bool					SetIgnoreChars ( const char * sIgnored, CSphString & sError ) override	{ return m_pTokenizer->SetIgnoreChars ( sIgnored, sError ); }
	bool					SetNgramChars ( const char * sConfig, CSphString & sError ) override	{ return m_pTokenizer->SetNgramChars ( sConfig, sError ); }
	void					SetNgramLen ( int iLen ) override										{ m_pTokenizer->SetNgramLen ( iLen ); }
	bool					LoadSynonyms ( const char * sFilename, const CSphEmbeddedFiles * pFiles, StrVec_t & dWarnings, CSphString & sError ) override { return m_pTokenizer->LoadSynonyms ( sFilename, pFiles, dWarnings, sError ); }
	void					WriteSynonyms ( CSphWriter & tWriter ) const final						{ return m_pTokenizer->WriteSynonyms ( tWriter ); }
	void 					WriteSynonyms ( JsonEscapedBuilder & tOut ) const final					{ return m_pTokenizer->WriteSynonyms ( tOut ); }
	bool					SetBoundary ( const char * sConfig, CSphString & sError ) override		{ return m_pTokenizer->SetBoundary ( sConfig, sError ); }
	void					Setup ( const CSphTokenizerSettings & tSettings ) override				{ m_pTokenizer->Setup ( tSettings ); }
	const CSphTokenizerSettings &	GetSettings () const override									{ return m_pTokenizer->GetSettings (); }
	const CSphSavedFile &	GetSynFileInfo () const override										{ return m_pTokenizer->GetSynFileInfo (); }
	bool					EnableSentenceIndexing ( CSphString & sError ) override					{ return m_pTokenizer->EnableSentenceIndexing ( sError ); }
	bool					EnableZoneIndexing ( CSphString & sError ) override						{ return m_pTokenizer->EnableZoneIndexing ( sError ); }
	int						SkipBlended () override													{ return m_pTokenizer->SkipBlended(); }
	bool					IsQueryTok() const noexcept override									{ return m_pTokenizer->IsQueryTok(); }


	int						GetCodepointLength ( int iCode ) const final		{ return m_pTokenizer->GetCodepointLength ( iCode ); }
	int						GetMaxCodepointLength () const final				{ return m_pTokenizer->GetMaxCodepointLength(); }

	const char *			GetTokenStart () const override						{ return m_pTokenizer->GetTokenStart(); }
	const char *			GetTokenEnd () const override						{ return m_pTokenizer->GetTokenEnd(); }
	const char *			GetBufferPtr () const override						{ return m_pTokenizer->GetBufferPtr(); }
	const char *			GetBufferEnd () const final							{ return m_pTokenizer->GetBufferEnd (); }
	void					SetBufferPtr ( const char * sNewPtr ) override		{ m_pTokenizer->SetBufferPtr ( sNewPtr ); }
	uint64_t				GetSettingsFNV () const override						{ return m_pTokenizer->GetSettingsFNV(); }

	void					SetBuffer ( const BYTE * sBuffer, int iLength ) override	{ m_pTokenizer->SetBuffer ( sBuffer, iLength ); }
	BYTE *					GetToken () override										{ return m_pTokenizer->GetToken(); }

	bool					WasTokenMultiformDestination ( bool & bHead, int & iDestCount ) const override { return m_pTokenizer->WasTokenMultiformDestination ( bHead, iDestCount ); }
};
