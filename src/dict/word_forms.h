//
// Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
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

#include "fileutils.h"
#include "sphinxstd.h"

struct CSphMultiformContainer;

struct CSphStoredNF
{
	CSphString m_sWord;
	bool m_bAfterMorphology;
};

/// wordforms container
struct CSphWordforms
{
	int m_iRefCount = 0;
	CSphVector<CSphSavedFile> m_dFiles;
	uint64_t m_uTokenizerFNV = 0;
	CSphString m_sIndexName;
	bool m_bHavePostMorphNF = false;
	CSphVector<CSphStoredNF> m_dNormalForms;
	std::unique_ptr<CSphMultiformContainer> m_pMultiWordforms;
	CSphOrderedHash<int, CSphString, CSphStrHashFunc, 1048576> m_hHash;

	~CSphWordforms();

	bool IsEqual ( const CSphVector<CSphSavedFile>& dFiles );
	bool ToNormalForm ( BYTE* pWord, bool bBefore, bool bOnlyCheck ) const;
};