//
// Copyright (c) 2017-2026, Manticore Software LTD (https://manticoresearch.com)
// Copyright (c) 2001-2016, Andrew Aksyonoff
// Copyright (c) 2008-2016, Sphinx Technologies Inc
// All rights reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License. You should have
// received a copy of the GPL license along with this program; if you
// did not, you can find it at http://www.gnu.org/
//

// This file should be built ONLY when 'WITH_JIEBA' is defined.

#include "jieba.h"

static_assert ( WITH_JIEBA, "should not build without WITH_JIEBA definition" );

#include <mutex>
#include <unordered_map>

#include "cjkpreprocessor.h"

#include "Jieba.hpp"

namespace
{
struct JiebaCacheKey_t
{
	int m_iMode = 0;
	bool m_bHMM = true;
	CSphString m_sDictPath;
	CSphString m_sHMMPath;
	CSphString m_sUserDictPath;
	CSphString m_sIdfPath;
	CSphString m_sStopPath;

	bool operator== ( const JiebaCacheKey_t & tOther ) const
	{
		return m_iMode==tOther.m_iMode && m_bHMM==tOther.m_bHMM
			&& m_sDictPath==tOther.m_sDictPath && m_sHMMPath==tOther.m_sHMMPath
			&& m_sUserDictPath==tOther.m_sUserDictPath && m_sIdfPath==tOther.m_sIdfPath
			&& m_sStopPath==tOther.m_sStopPath;
	}
};

struct JiebaCacheKeyHash_t
{
	static const size_t kHashCombine = 0x9e3779b9u; // golden ratio, improves hash mixing

	size_t operator() ( const JiebaCacheKey_t & tKey ) const
	{
		size_t uHash = std::hash<int>{}( tKey.m_iMode );
		uHash ^= std::hash<bool>{}( tKey.m_bHMM ) + kHashCombine + ( uHash<<6 ) + ( uHash>>2 );
		uHash ^= std::hash<std::string>{}( tKey.m_sDictPath.scstr() ) + kHashCombine + ( uHash<<6 ) + ( uHash>>2 );
		uHash ^= std::hash<std::string>{}( tKey.m_sHMMPath.scstr() ) + kHashCombine + ( uHash<<6 ) + ( uHash>>2 );
		uHash ^= std::hash<std::string>{}( tKey.m_sUserDictPath.scstr() ) + kHashCombine + ( uHash<<6 ) + ( uHash>>2 );
		uHash ^= std::hash<std::string>{}( tKey.m_sIdfPath.scstr() ) + kHashCombine + ( uHash<<6 ) + ( uHash>>2 );
		uHash ^= std::hash<std::string>{}( tKey.m_sStopPath.scstr() ) + kHashCombine + ( uHash<<6 ) + ( uHash>>2 );
		return uHash;
	}
};

class JiebaCache_c
{
public:
	std::shared_ptr<cppjieba::Jieba> GetOrCreate ( const JiebaCacheKey_t & tKey, const CSphString dJiebaFiles[] )
	{
		std::lock_guard<std::mutex> tLock ( m_tMutex );
		auto tIt = m_hCache.find ( tKey );
		if ( tIt!=m_hCache.end() )
		{
			if ( auto pExisting = tIt->second.lock() )
				return pExisting;
			m_hCache.erase ( tIt );
		}

		auto pJieba = std::make_shared<cppjieba::Jieba> (
			dJiebaFiles[0].cstr(),
			dJiebaFiles[1].cstr(),
			dJiebaFiles[2].cstr(),
			dJiebaFiles[3].cstr(),
			dJiebaFiles[4].cstr()
		);
		m_hCache.emplace ( tKey, std::weak_ptr<cppjieba::Jieba>(pJieba) );
		return pJieba;
	}

private:
	std::mutex m_tMutex;
	std::unordered_map<JiebaCacheKey_t, std::weak_ptr<cppjieba::Jieba>, JiebaCacheKeyHash_t> m_hCache;
};

JiebaCache_c & GetJiebaCache()
{
	static JiebaCache_c g_tCache;
	return g_tCache;
}
} // namespace


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
	JiebaCacheKey_t tKey;
	tKey.m_iMode = (int)m_eMode;
	tKey.m_bHMM = m_bHMM;
	tKey.m_sDictPath = dJiebaFiles[(int)JiebaFiles_e::DICT];
	tKey.m_sHMMPath = dJiebaFiles[(int)JiebaFiles_e::HMM];
	tKey.m_sUserDictPath = dJiebaFiles[(int)JiebaFiles_e::USER_DICT];
	tKey.m_sIdfPath = dJiebaFiles[(int)JiebaFiles_e::IDF];
	tKey.m_sStopPath = dJiebaFiles[(int)JiebaFiles_e::STOP_WORD];
	m_pJieba = GetJiebaCache().GetOrCreate ( tKey, dJiebaFiles );

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
