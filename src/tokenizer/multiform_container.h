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

// that is internal header shared between tokenizer and dict implementations

#pragma once

#include "sphinxstd.h"

struct CSphNormalForm
{
	CSphString m_sForm;
	int m_iLengthCP;
};

struct CSphMultiform
{
	int m_iFileId;
	CSphTightVector<CSphNormalForm> m_dNormalForm;
	CSphTightVector<CSphString> m_dTokens;
};


struct CSphMultiforms
{
	int m_iMinTokens;
	int m_iMaxTokens;
	CSphVector<CSphMultiform*> m_pForms; // OPTIMIZE? blobify?
};


struct CSphMultiformContainer
{
	int m_iMaxTokens = 0;
	using CSphMultiformHash = CSphOrderedHash<CSphMultiforms*, CSphString, CSphStrHashFunc, 131072> ;
	CSphMultiformHash m_Hash;
};
