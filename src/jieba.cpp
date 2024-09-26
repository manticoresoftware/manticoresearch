//
// Copyright (c) 2017-2024, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

#include "jieba.h"

#include "cjkpreprocessor.h"

#include "Jieba.hpp"


#if WITH_JIEBA

class JiebaPreprocessor_c : public CJKPreprocessor_c
{
public:
						JiebaPreprocessor_c ( bool bHMM ) : m_bHMM ( bHMM ) {}

	bool				Init ( CSphString & sError ) override;
	CJKPreprocessor_c * Clone() override { return new JiebaPreprocessor_c(m_bHMM); }

protected:
	void				ProcessBuffer ( const BYTE * pBuffer, int iLength ) override;
	const BYTE *		GetNextToken ( int & iTokenLen ) override;

private:
	std::unique_ptr<cppjieba::Jieba> m_pJieba;
	std::vector<cppjieba::Word>	m_dWords;
	cppjieba::CutContext		m_tCtx;
	int							m_iToken = 0;
	bool						m_bHMM = true;
};


bool JiebaPreprocessor_c::Init ( CSphString & sError )
{
	assert ( !m_pJieba );

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
	{
		i.SetSprintf ( "%s/%s", sJiebaPath.cstr(), i.cstr() );

		if ( !sphIsReadable ( i.cstr() ) )
		{
			sError.SetSprintf ( "Error initializing Jieba: unable to read '%s'", i.cstr() );
			return false;
		}
	}

	// fixme! jieba responds to load errors with abort() call
	m_pJieba = std::make_unique<cppjieba::Jieba> ( dJiebaFiles[(int)JiebaFiles_e::DICT].cstr(), dJiebaFiles[(int)JiebaFiles_e::HMM].cstr(), dJiebaFiles[(int)JiebaFiles_e::USER_DICT].cstr(), dJiebaFiles[(int)JiebaFiles_e::IDF].cstr(), dJiebaFiles[(int)JiebaFiles_e::STOP_WORD].cstr() );

	return true;
}


void JiebaPreprocessor_c::ProcessBuffer ( const BYTE * pBuffer, int iLength )
{
	m_dWords.resize(0);
	m_tCtx.Reset();

	m_pJieba->Cut ( { (const char*)pBuffer, (size_t)iLength }, m_dWords, m_tCtx, m_bHMM );
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


bool SpawnFilterJieba ( std::unique_ptr<ISphFieldFilter> & pFieldFilter, const CSphIndexSettings & tSettings, const CSphTokenizerSettings & tTokSettings, const char * szIndex, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor!=Preprocessor_e::JIEBA )
		return true;

	auto pFilterICU = CreateFilterCJK ( std::move ( pFieldFilter ), std::make_unique<JiebaPreprocessor_c> ( tSettings.m_bJiebaHMM ), tTokSettings.m_sBlendChars.cstr(), sError );
	if ( !sError.IsEmpty() )
	{
		sError.SetSprintf ( "table '%s': Error initializing Jieba: %s", szIndex, sError.cstr() );
		return false;
	}

	pFieldFilter = std::move ( pFilterICU );
	return true;
}

#else

bool CheckConfigJieba ( CSphIndexSettings & tSettings, CSphString & sError )
{
	if ( tSettings.m_ePreprocessor==Preprocessor_e::JIEBA )
	{
		tSettings.m_ePreprocessor = Preprocessor_e::NONE;
		sError.SetSprintf ( "Jieba options specified, but no Jieba support compiled; ignoring" );
		return false;
	}

	return true;
}


bool CheckTokenizerJieba ( CSphIndexSettings &, const CSphTokenizerSettings &, CSphString & )
{
	return true;
}


bool SpawnFilterJieba ( std::unique_ptr<ISphFieldFilter> &, const CSphIndexSettings &, const CSphTokenizerSettings &, const char *, CSphString & )
{
	return true;
}

#endif
