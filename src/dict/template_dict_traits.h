//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "dict_base.h"

class LemmatizerTrait_i;

struct TemplateDictTraits_c: DictStub_c
{
	TemplateDictTraits_c();

protected:
	~TemplateDictTraits_c() override;

public:
	void LoadStopwords ( const char* sFiles, const TokenizerRefPtr_c& pTokenizer, bool bStripFile ) final;
	void LoadStopwords ( const CSphVector<SphWordID_t>& dStopwords ) final;
	void WriteStopwords ( CSphWriter& tWriter ) const final;
	void WriteStopwords ( JsonEscapedBuilder& tOut ) const final;
	bool LoadWordforms ( const StrVec_t& dFiles, const CSphEmbeddedFiles* pEmbedded, const TokenizerRefPtr_c& pTokenizer, const char* szIndex ) final;
	void WriteWordforms ( CSphWriter& tWriter ) const final;
	void WriteWordforms ( JsonEscapedBuilder& tOut ) const final;
	const CSphWordforms* GetWordforms() final { return m_pWordforms; }
	void DisableWordforms() final { m_bDisableWordforms = true; }
	int SetMorphology ( const char* szMorph, CSphString& sMessage ) final;
	bool HasMorphology() const final;
	void ApplyStemmers ( BYTE* pWord ) const final;
	const CSphMultiformContainer* GetMultiWordforms() const final;
	uint64_t GetSettingsFNV() const final;
	static void SweepWordformContainers ( const CSphVector<CSphSavedFile>& dFiles );

protected:
	CSphVector<int> m_dMorph;
#if WITH_STEMMER
	CSphVector<void*> m_dStemmers; // avoid to poison namespace with sb_stemmer*, use void*
	StrVec_t m_dDescStemmers;
#endif
	std::unique_ptr<LemmatizerTrait_i> m_tLemmatizer;

	int m_iStopwords = 0;		   ///< stopwords count
	SphWordID_t* m_pStopwords = nullptr; ///< stopwords ID list
	CSphFixedVector<SphWordID_t> m_dStopwordContainer {0};

protected:
	int ParseMorphology ( const char* szMorph, CSphString& sError );
	SphWordID_t FilterStopword ( SphWordID_t uID ) const; ///< filter ID against stopwords list
	DictRefPtr_c CloneBase ( TemplateDictTraits_c* pDict ) const;
	bool HasState() const final;

	bool m_bDisableWordforms = false;

private:
	CSphWordforms* m_pWordforms = nullptr;
	static CSphVector<CSphWordforms*> m_dWordformContainers;

	CSphWordforms* GetWordformContainer ( const CSphVector<CSphSavedFile>& dFileInfos, const StrVec_t* pEmbeddedWordforms, const TokenizerRefPtr_c& pTokenizer, const char* szIndex );
	CSphWordforms* LoadWordformContainer ( const CSphVector<CSphSavedFile>& dFileInfos, const StrVec_t* pEmbeddedWordforms, const TokenizerRefPtr_c& pTokenizer, const char* szIndex );

	int InitMorph ( const char* szMorph, int iLength, CSphString& sError );
	int AddMorph ( int iMorph ); ///< helper that always returns ST_OK
	bool StemById ( BYTE* pWord, int iStemmer ) const;
	void AddWordform ( CSphWordforms* pContainer, char* sBuffer, int iLen, const TokenizerRefPtr_c& pTokenizer, const char* szFile, const CSphVector<int>& dBlended, int iFileId );
};