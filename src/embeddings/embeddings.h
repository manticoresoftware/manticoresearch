//
// Copyright (c) 2024, Manticore Software LTD (https://manticoresearch.com)
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
#include "std/string.h"
#include "std/vector.h"

#include <functional>

namespace Embeddings {

	bool LoadLib ( const CSphString& sPath );

	// load model by name, as 'sentence-transformers/multi-qa-MiniLM-L6-cos-v1'
	// returns size of embeddings, and max size of text
	std::pair<int,int> LoadModel ( const CSphString & sPath );

	// make embeddings and copy them to resulting vec
	CSphVector<float> MakeEmbeddings ( const CSphString & sText );

	using fnProcessor = std::function<void ( VecTraits_T<float> )>;

	// make embeddings and process them (in-place, without copying)
	void ProcessEmbeddings ( const CSphString & sText, fnProcessor fnProcess );
}