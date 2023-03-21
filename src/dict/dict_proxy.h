//
// Copyright (c) 2017-2023, Manticore Software LTD (https://manticoresearch.com)
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

	// that is only one should be overridden
	SphWordID_t GetWordID ( BYTE* pWord ) override { return m_pDict->GetWordID ( pWord ); }
	SphWordID_t GetWordIDWithMarkers ( BYTE* pWord ) final { return m_pDict->GetWordIDWithMarkers ( pWord ); }
	SphWordID_t GetWordIDNonStemmed ( BYTE* pWord ) final { return m_pDict->GetWordIDNonStemmed ( pWord ); }
	SphWordID_t GetWordID ( const BYTE* pWord, int iLen, bool bFilterStops ) final { return m_pDict->GetWordID ( pWord, iLen, bFilterStops ); }
	void ApplyStemmers ( BYTE* pWord ) const final { m_pDict->ApplyStemmers ( pWord ); }
	void LoadStopwords ( const char* sFiles, const TokenizerRefPtr_c& pTokenizer, bool bStripFile ) final { m_pDict->LoadStopwords ( sFiles, pTokenizer, bStripFile ); }
	void LoadStopwords ( const CSphVector<SphWordID_t>& dStopwords ) final { m_pDict->LoadStopwords ( dStopwords ); }
	void WriteStopwords ( Writer_i & tWriter ) const final { m_pDict->WriteStopwords ( tWriter ); }
	void WriteStopwords ( JsonEscapedBuilder& tOut ) const final { m_pDict->WriteStopwords ( tOut ); }
	bool LoadWordforms ( const StrVec_t& dFiles, const CSphEmbeddedFiles* pEmbedded, const TokenizerRefPtr_c& pTokenizer, const char* szIndex ) final { return m_pDict->LoadWordforms ( dFiles, pEmbedded, pTokenizer, szIndex ); }
	void WriteWordforms ( Writer_i & tWriter ) const final { m_pDict->WriteWordforms ( tWriter ); }
	void WriteWordforms ( JsonEscapedBuilder& tOut ) const final { m_pDict->WriteWordforms ( tOut ); }
	const CSphWordforms* GetWordforms() final { return m_pDict->GetWordforms(); }
	void DisableWordforms() final { m_pDict->DisableWordforms(); }
	int SetMorphology ( const char* szMorph, CSphString& sMessage ) final { return m_pDict->SetMorphology ( szMorph, sMessage ); }
	bool HasMorphology() const override{ return m_pDict->HasMorphology(); }
	const CSphString& GetMorphDataFingerprint() const override { return m_pDict->GetMorphDataFingerprint(); }
	void Setup ( const CSphDictSettings& ) final {} /// notice!
	const CSphDictSettings& GetSettings() const final { return m_pDict->GetSettings(); }
	const CSphVector<CSphSavedFile>& GetStopwordsFileInfos() const final { return m_pDict->GetStopwordsFileInfos(); }
	const CSphVector<CSphSavedFile>& GetWordformsFileInfos() const final { return m_pDict->GetWordformsFileInfos(); }
	const CSphMultiformContainer* GetMultiWordforms() const final { return m_pDict->GetMultiWordforms(); }
	bool IsStopWord ( const BYTE* pWord ) const final { return m_pDict->IsStopWord ( pWord ); }

public:
	void SetSkiplistBlockSize ( int iSize ) final { m_pDict->SetSkiplistBlockSize ( iSize ); }
	void HitblockBegin() final { m_pDict->HitblockBegin(); }
	void HitblockPatch ( CSphWordHit* pHits, int iHits ) const final { m_pDict->HitblockPatch ( pHits, iHits ); }
	const char* HitblockGetKeyword ( SphWordID_t tWordID ) final { return m_pDict->HitblockGetKeyword ( tWordID ); }
	int HitblockGetMemUse() final { return m_pDict->HitblockGetMemUse(); }
	void HitblockReset() final { m_pDict->HitblockReset(); }

public:
	void DictBegin ( CSphAutofile& tTempDict, CSphAutofile& tDict, int iDictLimit ) final { m_pDict->DictBegin ( tTempDict, tDict, iDictLimit ); };
	void DictEntry ( const DictEntry_t& tEntry ) final { m_pDict->DictEntry ( tEntry ); };
	void DictEndEntries ( SphOffset_t iDoclistOffset ) final { m_pDict->DictEndEntries ( iDoclistOffset ); };
	bool DictEnd ( DictHeader_t* pHeader, int iMemLimit, CSphString& sError ) final { return m_pDict->DictEnd ( pHeader, iMemLimit, sError ); };
	bool DictIsError() const final { return m_pDict->DictIsError(); };

public:
	bool HasState() const final {
		return m_pDict->HasState();
	}
	DictRefPtr_c Clone() const final {
		return DictRefPtr_c { new DictProxy_c ( m_pDict->Clone() ) };
	}
	uint64_t GetSettingsFNV() const final { return m_pDict->GetSettingsFNV(); }

protected:
	DictRefPtr_c m_pDict;
};
