//
// Copyright (c) 2017-2022, Manticore Software LTD (http://manticoresearch.com)
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

#include "tokenizer/tokenizer.h"
#include "sphinxint.h"


struct ISphQueryFilter
{
	TokenizerRefPtr_c m_pTokenizer;
	CSphDict* m_pDict = nullptr;
	const CSphIndexSettings* m_pSettings = nullptr;
	GetKeywordsSettings_t m_tFoldSettings;

	virtual ~ISphQueryFilter() = default;

	void GetKeywords ( CSphVector<CSphKeywordInfo>& dKeywords, const ExpansionContext_t& tCtx );
	virtual void AddKeywordStats ( BYTE* sWord, const BYTE* sTokenized, int iQpos, CSphVector<CSphKeywordInfo>& dKeywords ) = 0;
};

struct CSphTemplateQueryFilter: public ISphQueryFilter
{
	void AddKeywordStats ( BYTE* sWord, const BYTE* sTokenized, int iQpos, CSphVector<CSphKeywordInfo>& dKeywords ) final;
};

class ISphQwordSetup;
class ISphQword;

struct CSphPlainQueryFilter : public ISphQueryFilter
{
	const ISphQwordSetup *	m_pTermSetup;
	ISphQword *				m_pQueryWord;

	void AddKeywordStats ( BYTE * sWord, const BYTE * sTokenized, int iQpos, CSphVector <CSphKeywordInfo> & dKeywords ) override;
};
