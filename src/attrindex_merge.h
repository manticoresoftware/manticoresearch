//
// Copyright (c) 2017-2022, Manticore Software LTD (https://manticoresearch.com)
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

#include "sphinx.h"
#include "indexformat.h"

class AttrMerger_c
{
	class Impl_c;
	std::unique_ptr<Impl_c> m_pImpl;

public:
	AttrMerger_c ( MergeCb_c& tMonitor, CSphString& sError, int64_t iTotalDocs );
	~AttrMerger_c();

	bool Prepare ( const CSphIndex * pSrcIndex, const CSphIndex * pDstIndex );
	bool CopyAttributes ( const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap, DWORD uAlive );
	bool FinishMergeAttributes ( const CSphIndex * pDstIndex, BuildHeader_t & tBuildHeader, StrVec_t* pCreatedFiles );
};

bool SiRecreate ( MergeCb_c & tMonitor, const CSphIndex & tIndex, const VecTraits_T<RowID_t> & dRowMap, CSphString & sFile, CSphString & sError );

