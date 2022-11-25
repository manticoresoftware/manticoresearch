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

/// dict traits
class DictProxy_c: public CSphDict
{
public:
	explicit DictProxy_c ( DictRefPtr_c pDict )
		: m_pDict { std::move ( pDict ) }
	{}

	void LoadStopwords ( const char* sFiles, const TokenizerRefPtr_c& pTokenizer, bool bStripFile ) final { m_pDict->LoadStopwords ( sFiles, pTokenizer, bStripFile ); }
	void LoadStopwords ( const CSphVector<SphWordID_t>& dStopwords ) final { m_pDict->LoadStopwords ( dStopwords ); }
	void WriteStopwords ( CSphWriter& tWriter ) const final { m_pDict->WriteStopwords ( tWriter ); }
	void WriteStopwords ( JsonEscapedBuilder& tOut ) const final { m_pDict->WriteStopwords ( tOut ); }
	bool LoadWordforms ( const StrVec_t& dFiles, const CSphEmbeddedFiles* pEmbedded, const TokenizerRefPtr_c& pTokenizer, const char* szIndex ) final { return m_pDict->LoadWordforms ( dFiles, pEmbedded, pTokenizer, szIndex ); }
	void WriteWordforms ( CSphWriter& tWriter ) const final { m_pDict->WriteWordforms ( tWriter ); }
	void WriteWordforms ( JsonEscapedBuilder& tOut ) const final { m_pDict->WriteWordforms ( tOut ); }
	int SetMorphology ( const char* szMorph, CSphString& sMessage ) final { return m_pDict->SetMorphology ( szMorph, sMessage ); }

	SphWordID_t GetWordID ( const BYTE* pWord, int iLen, bool bFilterStops ) final { return m_pDict->GetWordID ( pWord, iLen, bFilterStops ); }

	// that is only one should be overridden
	// SphWordID_t GetWordID ( BYTE* pWord ) override

	SphWordID_t GetWordIDNonStemmed ( BYTE* pWord ) final { return m_pDict->GetWordIDNonStemmed ( pWord ); }
	void Setup ( const CSphDictSettings& ) final {}
	const CSphDictSettings& GetSettings() const final { return m_pDict->GetSettings(); }
	const CSphVector<CSphSavedFile>& GetStopwordsFileInfos() const final { return m_pDict->GetStopwordsFileInfos(); }
	const CSphVector<CSphSavedFile>& GetWordformsFileInfos() const final { return m_pDict->GetWordformsFileInfos(); }
	const CSphMultiformContainer* GetMultiWordforms() const final { return m_pDict->GetMultiWordforms(); }
	const CSphWordforms* GetWordforms() final { return m_pDict->GetWordforms(); }

	bool IsStopWord ( const BYTE* pWord ) const final { return m_pDict->IsStopWord ( pWord ); }
	uint64_t GetSettingsFNV() const final { return m_pDict->GetSettingsFNV(); }

protected:
	DictRefPtr_c m_pDict;
};
