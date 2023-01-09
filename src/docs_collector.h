//
// Copyright (c) 2008-2023, Manticore Software LTD (https://manticoresearch.com)
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

#include "searchdaemon.h"

class DocsCollector_c : public ISphNoncopyable
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

public:
	DocsCollector_c ( const CSphQuery& tQuery, bool bJson, const CSphString& sIndex, const cServedIndexRefPtr_c& pDesc, CSphString* pError );
	DocsCollector_c ( DocsCollector_c&& rhs ) noexcept;
	~DocsCollector_c();

	bool GetValuesChunk ( CSphVector<DocID_t>& dValues, int iValues );

	// beware, that slice lives together with this class, and will become undefined once it destroyed.
	VecTraits_T<DocID_t> GetValuesSlice();
};