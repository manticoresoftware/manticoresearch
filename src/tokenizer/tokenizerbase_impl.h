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

#pragma once

#include "tokenizer.h"
#include "sphinxdefs.h"

class ExceptionsTrie_c;

class CSphTokenizerBase: public ISphTokenizer
{
public:
	explicit CSphTokenizerBase ( int iTokenBytes = SPH_LEGACY_TOKEN_BYTES );

	bool SetCaseFolding ( const char* sConfig, CSphString& sError ) final;
	bool LoadSynonyms ( const char* sFilename, const CSphEmbeddedFiles* pFiles, StrVec_t& dWarnings, CSphString& sError ) final;
	void WriteSynonyms ( Writer_i & tWriter ) const final;
	void WriteSynonyms ( JsonEscapedBuilder & tOut ) const final;
	void CloneBase ( const CSphTokenizerBase* pFrom, ESphTokenizerClone eMode );

	const char* GetTokenStart() const noexcept final
	{
		return (const char*)m_pTokenStart;
	}
	const char* GetTokenEnd() const noexcept final
	{
		return (const char*)m_pTokenEnd;
	}
	const char* GetBufferPtr() const noexcept final
	{
		return (const char*)m_pCur;
	}
	const char* GetBufferEnd() const noexcept final
	{
		return (const char*)m_pBufferMax;
	}
	void SetBufferPtr ( const char* sNewPtr ) final;
	uint64_t GetSettingsFNV() const noexcept final;
	int GetMaxTokenBytes () const noexcept final { return m_iTokenBytes; }
	int GetOversizedTokenCount () const noexcept final { return m_iOversizedTokenCount; }
	int ResetOversizedTokenCount () noexcept final { int iSkipped = m_iOversizedTokenCount; m_iOversizedTokenCount = 0; return iSkipped; }

	bool SetBlendChars ( const char* sConfig, CSphString& sError ) final;
	bool WasTokenMultiformDestination ( bool&, int& ) const noexcept final
	{
		return false;
	}
	bool TokenIsBlendedHead () const noexcept final { return m_bBlendedHead; }

	bool IsQueryTok() const noexcept final
	{
		return m_eMode != SPH_CLONE_INDEX;
	}
protected:
	~CSphTokenizerBase() override;

	bool BlendAdjust ( const BYTE* pPosition );
	int CodepointArbitrationI ( int iCodepoint );
	int CodepointArbitrationQ ( int iCodepoint, bool bWasEscaped, BYTE uNextByte );
	BYTE * Accum() noexcept { return m_dAccum.Begin(); }
	const BYTE * Accum() const noexcept { return m_dAccum.Begin(); }
	BYTE * AccumBlend() noexcept { return m_dAccumBlend.Begin(); }
	const BYTE * AccumBlend() const noexcept { return m_dAccumBlend.Begin(); }
	int AccumCapacity() const noexcept { return m_iTokenBytes+1; }
	int AccumBufferCapacity() const noexcept { return m_dAccum.GetLength(); }
	int AccumBytes() const noexcept { return (int)( m_pAccum - Accum() ); }
	void ResetAccum() noexcept { m_iAccum = 0; m_pAccum = Accum(); }
	bool ConsumeLastTokenOverLimit() noexcept;

protected:
	const BYTE* m_pBuffer = nullptr;	 ///< my buffer
	const BYTE* m_pBufferMax = nullptr;	 ///< max buffer ptr, exclusive (ie. this ptr is invalid, but every ptr below is ok)
	const BYTE* m_pCur = nullptr;		 ///< current position
	const BYTE* m_pTokenStart = nullptr; ///< last token start point
	const BYTE* m_pTokenEnd = nullptr;	 ///< last token end point

	CSphFixedVector<BYTE> m_dAccum;		///< folded token accumulator
	CSphFixedVector<BYTE> m_dAccumBlend;	///< blend-acc, an accumulator copy for additional blended variants
	BYTE* m_pAccum = nullptr;				///< current accumulator position
	int m_iAccum = 0;						///< boundary token size, codepoints
	int m_iTokenBytes = SPH_LEGACY_TOKEN_BYTES;
	int m_iTokenCodepoints = SPH_MAX_WORD_LEN;
	bool m_bSkipOverLimit = false;
	bool m_bAccumOverLimit = false;
	bool m_bLastTokenOverLimit = false;
	int m_iOversizedTokenCount = 0;

	int m_iBlendNormalStart = 0;				  ///< points to first normal char in the accumulators (might be NULL)
	int m_iBlendNormalEnd = 0;					  ///< points just past (!) last normal char in the accumulators (might be NULL)

	ExceptionsTrie_c* m_pExc = nullptr; ///< exceptions trie, if any

	bool m_bHasBlend = false;
	const BYTE* m_pBlendStart = nullptr;
	const BYTE* m_pBlendEnd = nullptr;
	bool m_bBlendedHead = false;

	ESphTokenizerClone m_eMode { SPH_CLONE_INDEX };
};
