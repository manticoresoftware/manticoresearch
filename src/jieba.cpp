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

// this file is should build ONLY when 'WITH_JIEBA" defined

#include "jieba.h"

STATIC_ASSERT ( WITH_JIEBA, SHOULD_NOT_BUILD_WIHTOUT_WITH_JIEBA_DEFINITION );

#include "cjkpreprocessor.h"

#include "Jieba.hpp"


class JiebaPreprocessor_c : public CJKPreprocessor_c
{
public:
						JiebaPreprocessor_c ( JiebaMode_e eMode, bool bHMM, const CSphString & sJiebaUserDictPath );

	bool				Init ( CSphString & sError ) override;
	CJKPreprocessor_c * Clone ( const FieldFilterOptions_t * pOptions ) override { return new JiebaPreprocessor_c ( pOptions && pOptions->m_eJiebaMode!=JiebaMode_e::NONE ? pOptions->m_eJiebaMode : m_eMode, m_bHMM, m_sJiebaUserDictPath, m_pJieba ); }

protected:
	void				ProcessBuffer ( const BYTE * pBuffer, int iLength ) override;
	const BYTE *		GetNextToken ( int & iTokenLen ) override;

private:
	std::shared_ptr<cppjieba::Jieba> m_pJieba;
	std::vector<cppjieba::Word>	m_dWords;
	cppjieba::CutContext		m_tCtx;
	int							m_iToken = 0;

	JiebaMode_e					m_eMode;
	bool						m_bHMM = true;
	CSphString					m_sJiebaUserDictPath;

						JiebaPreprocessor_c ( JiebaMode_e eMode, bool bHMM, const CSphString & sJiebaUserDictPath, std::shared_ptr<cppjieba::Jieba> pJieba );
};


JiebaPreprocessor_c::JiebaPreprocessor_c ( JiebaMode_e eMode, bool bHMM, const CSphString & sJiebaUserDictPath )
	: m_eMode ( eMode )
	, m_bHMM ( bHMM )
	, m_sJiebaUserDictPath ( sJiebaUserDictPath )
{}


JiebaPreprocessor_c::JiebaPreprocessor_c ( JiebaMode_e eMode, bool bHMM, const CSphString & sJiebaUserDictPath, std::shared_ptr<cppjieba::Jieba> pJieba )
	: m_pJieba ( pJieba )
	, m_eMode ( eMode )
	, m_bHMM ( bHMM )
	, m_sJiebaUserDictPath ( sJiebaUserDictPath )
{}


bool JiebaPreprocessor_c::Init ( CSphString & sError )
{
	// skip init if reusing existing jieba
	if ( m_pJieba )
		return true;

	CSphString sJiebaPath = GetJiebaDataDir();
	enum class JiebaFiles_e : int
	{
		DICT = 0,
		HMM,
		USER_DICT,
		IDF,
		STOP_WORD,
		TOTAL
	};
	
	CSphString dJiebaFiles[] =
	{
		"jieba.dict.utf8",
		"hmm_model.utf8",
		"user.dict.utf8",
		"idf.utf8",
		"stop_words.utf8"
	};

	for ( auto & i : dJiebaFiles )
		i.SetSprintf ( "%s/%s", sJiebaPath.cstr(), i.cstr() );

	if ( !m_sJiebaUserDictPath.IsEmpty() )
		dJiebaFiles[(int)JiebaFiles_e::USER_DICT] = m_sJiebaUserDictPath;

	for ( auto & i : dJiebaFiles )
		if ( !sphIsReadable ( i.cstr() ) )
		{
			sError.SetSprintf ( "Error initializing Jieba: unable to read '%s'", i.cstr() );
			return false;
		}

	// fixme! jieba responds to load errors with abort() call
	m_pJieba = std::make_shared<cppjieba::Jieba> ( dJiebaFiles[(int)JiebaFiles_e::DICT].cstr(), dJiebaFiles[(int)JiebaFiles_e::HMM].cstr(), dJiebaFiles[(int)JiebaFiles_e::USER_DICT].cstr(), dJiebaFiles[(int)JiebaFiles_e::IDF].cstr(), dJiebaFiles[(int)JiebaFiles_e::STOP_WORD].cstr() );

	return true;
}


void JiebaPreprocessor_c::ProcessBuffer ( const BYTE * pBuffer, int iLength )
{
	m_dWords.resize(0);

	switch ( m_eMode )
	{
	case JiebaMode_e::ACCURATE:
		m_pJieba->Cut ( { (const char*)pBuffer, (size_t)iLength }, m_dWords, m_tCtx, m_bHMM );
		break;

	case JiebaMode_e::FULL:
		m_pJieba->CutAll ( { (const char*)pBuffer, (size_t)iLength }, m_dWords, m_tCtx );
		break;

	case JiebaMode_e::SEARCH:
		m_pJieba->CutForSearch ( { (const char*)pBuffer, (size_t)iLength }, m_dWords, m_tCtx, m_bHMM );
		break;

	default:
		break;
	}

	m_iToken = 0;
}


const BYTE * JiebaPreprocessor_c::GetNextToken ( int & iTokenLen )
{
	if ( (size_t)m_iToken>=m_dWords.size() )
		return nullptr;

	const auto & tWord = m_dWords[m_iToken++];
	iTokenLen = tWord.word.length();
	return (const BYTE*)tWord.word.c_str();
}

//////////////////////////////////////////////////////////////////////////

bool CheckConfigJieba ( CSphIndexSettings & tSettings, CSphString & sError )
{
	return true;
}


bool StrToJiebaMode ( JiebaMode_e & eMode, const CSphString & sValue, CSphString & sError )
{
	if ( sValue=="accurate" )
		eMode = JiebaMode_e::ACCURATE;
	else if ( sValue=="full" )
		eMode = JiebaMode_e::FULL;
	else if ( sValue=="search" )
		eMode = JiebaMode_e::SEARCH;
	else
	{
		sError.SetSprintf ( "unknown jieba mode: %s", sValue.cstr() );
		return false;
	}

	return true;
}


bool SpawnFilterJieba ( std::unique_ptr<ISphFieldFilter> & pFieldFilter, const CSphIndexSettings & tSettings, const CSphTokenizerSettings & tTokSettings, const char * szIndex, FilenameBuilder_i * pFilenameBuilder, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor!=Preprocessor_e::JIEBA )
		return true;

	CSphString sJiebaUserDictPath = tSettings.m_sJiebaUserDictPath;
	if ( !sJiebaUserDictPath.IsEmpty() && pFilenameBuilder )
		sJiebaUserDictPath = pFilenameBuilder->GetFullPath(sJiebaUserDictPath);

	auto pFilterICU = CreateFilterCJK ( std::move ( pFieldFilter ), std::make_unique<JiebaPreprocessor_c> ( tSettings.m_eJiebaMode, tSettings.m_bJiebaHMM, sJiebaUserDictPath ), tTokSettings.m_sBlendChars.cstr(), sError );
	if ( !sError.IsEmpty() )
	{
		sError.SetSprintf ( "table '%s': Error initializing Jieba: %s", szIndex, sError.cstr() );
		return false;
	}

	pFieldFilter = std::move ( pFilterICU );
	return true;
}

