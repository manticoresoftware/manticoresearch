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
#include "sphinxdefs.h"

class ExceptionsTrie_c;

class CSphTokenizerBase: public ISphTokenizer
{
public:
	CSphTokenizerBase();

	bool SetCaseFolding ( const char* sConfig, CSphString& sError ) final;
	bool LoadSynonyms ( const char* sFilename, const CSphEmbeddedFiles* pFiles, StrVec_t& dWarnings, CSphString& sError ) final;
	void WriteSynonyms ( Writer_i & tWriter ) const final;
	void WriteSynonyms ( JsonEscapedBuilder & tOut ) const final;
	void CloneBase ( const CSphTokenizerBase* pFrom, ESphTokenizerClone eMode );

	const char* GetTokenStart() const final
	{
		return (const char*)m_pTokenStart;
	}
	const char* GetTokenEnd() const final
	{
		return (const char*)m_pTokenEnd;
	}
	const char* GetBufferPtr() const final
	{
		return (const char*)m_pCur;
	}
	const char* GetBufferEnd() const final
	{
		return (const char*)m_pBufferMax;
	}
	void SetBufferPtr ( const char* sNewPtr ) final;
	uint64_t GetSettingsFNV() const final;

	bool SetBlendChars ( const char* sConfig, CSphString& sError ) final;
	bool WasTokenMultiformDestination ( bool&, int& ) const final
	{
		return false;
	}

	bool IsQueryTok() const noexcept final
	{
		return m_eMode != SPH_CLONE_INDEX;
	}
protected:
	~CSphTokenizerBase() override;

	bool BlendAdjust ( const BYTE* pPosition );
	int CodepointArbitrationI ( int iCodepoint );
	int CodepointArbitrationQ ( int iCodepoint, bool bWasEscaped, BYTE uNextByte );

protected:
	const BYTE* m_pBuffer = nullptr;	 ///< my buffer
	const BYTE* m_pBufferMax = nullptr;	 ///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	const BYTE* m_pCur = nullptr;		 ///< current position
	const BYTE* m_pTokenStart = nullptr; ///< last token start point
	const BYTE* m_pTokenEnd = nullptr;	 ///< last token end point

	BYTE m_sAccum[3 * SPH_MAX_WORD_LEN + 3]; ///< folded token accumulator
	BYTE* m_pAccum = nullptr;				 ///< current accumulator position
	int m_iAccum = 0;						 ///< boundary token size

	BYTE m_sAccumBlend[3 * SPH_MAX_WORD_LEN + 3]; ///< blend-acc, an accumulator copy for additional blended variants
	int m_iBlendNormalStart = 0;				  ///< points to first normal char in the accumulators (might be NULL)
	int m_iBlendNormalEnd = 0;					  ///< points just past (!) last normal char in the accumulators (might be NULL)

	ExceptionsTrie_c* m_pExc = nullptr; ///< exceptions trie, if any

	bool m_bHasBlend = false;
	const BYTE* m_pBlendStart = nullptr;
	const BYTE* m_pBlendEnd = nullptr;

	ESphTokenizerClone m_eMode { SPH_CLONE_INDEX };
};
